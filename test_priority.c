#include "types.h"
#include "user.h"
#include "stat.h"
#include "param.h"

#define MAX_COMMANDS 3

struct CommandInfo {
    char *command;
    int priority; // Priority for the command
    char *argv[10]; // Arguments for the command
    int arrivalTime; // Time when the command is scheduled to run
    int startTime; // Time when the command actually starts running
    int endTime; // Time when the command completes
};

void executeAndSetPriority(struct CommandInfo commands[], int numCommands) {
    int globalStartTime = uptime();
    int totalTime = 0, totalTurnaroundTime = 0, totalResponseTime = 0;

    for (int i = 0; i < numCommands; i++) {
        commands[i].arrivalTime = uptime() - globalStartTime;
        int pid = fork();
        if (pid == 0) {
            // Child process
            commands[i].startTime = uptime() - globalStartTime; // Capture start time
            exec(commands[i].command, commands[i].argv);
            printf(2, "exec %s failed\n", commands[i].command);
            exit();
        } else if (pid > 0) {
            // Assuming set_sched_priority is a function to set priority for each command/process
            set_sched_priority(pid, commands[i].priority);
        } else {
            printf(2, "Fork failed\n");
        }
    }

    // Wait for all child processes to complete and capture their end times
    for (int i = 0; i < numCommands; i++) {
        wait();
        commands[i].endTime = uptime() - globalStartTime;
        totalTime += commands[i].endTime;
        totalTurnaroundTime += commands[i].endTime - commands[i].arrivalTime;
        totalResponseTime += commands[i].startTime - commands[i].arrivalTime;
    }

    // Calculating averages
    float avgTurnaroundTime = (float)totalTurnaroundTime / numCommands;
    float avgResponseTime = (float)totalResponseTime / numCommands;
    float throughput = (float)numCommands / totalTime;

    // Print turnaround and response times
    for (int i = 0; i < numCommands; i++) {
        printf(1, "%s - Arrival Time: %d, Start Time: %d, End Time: %d\n",
               commands[i].command, commands[i].arrivalTime, commands[i].startTime, commands[i].endTime);
    }

    // Print average metrics
    printf(1, "Average Turnaround Time: %d\nAverage Response Time: %d\nThroughput: %d\n",
           (int)avgTurnaroundTime, (int)avgResponseTime, (int)throughput);
}

int main(void) {
    struct CommandInfo commands[MAX_COMMANDS] = {
        {"ls", LOW_PRIORITY, {"ls", 0}, 0, 0, 0},
        {"echo", HIGH_PRIORITY, {"echo", "Hello, world!", 0}, 0, 0, 0},
        {"wc", MEDIUM_PRIORITY, {"wc", "README", 0}, 0, 0, 0}
    };

    executeAndSetPriority(commands, MAX_COMMANDS);
    exit();
}
