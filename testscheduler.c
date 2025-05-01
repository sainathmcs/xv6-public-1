#include "types.h"
#include "user.h"
#include "stat.h"
//#include <fcntl.h>

struct CommandInfo {
    char *cmd;      // Command string
    char *args[10]; // Command arguments
    int PID;
    int execTime;   // Execution time (job length) in ticks (hypothetical)
};

//Hypothetical function to measure execution time of a command
int measureExecTime(char *args[]) {
    //int startTime, endTime;
    int pid = fork();
    int jobLength;
    jobLength = sjf_job_length(pid);
    if (pid == 0) {

    int fd = open("tmpfile", 0x002 | 0x200); // Using numeric values directly as an example
    if (fd >= 0) {
        close(1); // Close stdout
        dup(fd);  // Duplicate the file descriptor to stdout
        close(2); // Close stderr
        dup(fd);  // Duplicate the file descriptor to stderr
    }

        exec(args[0], args);
        exit();
    } else { // Assume uptime() gives system uptime in ticks
        wait();
    }
    printf(1, "job %d has job length %d\n", pid, jobLength);
    return jobLength;
}


// Simple bubble sort to sort commands based on execution time
void sortCommands(struct CommandInfo *commands, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (commands[j].execTime > commands[j + 1].execTime) {
                struct CommandInfo temp = commands[j];
                commands[j] = commands[j + 1];
                commands[j + 1] = temp;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    struct CommandInfo commands[5]; // Hypothetically assuming a maximum of 5 commands
    int cmdIndex = 0;
    int argIndex = 0;
    int tat = 0;
    int res = 0;
    int completion_times[3];
    // Initial parsing and execution to measure job lengths
    for (int i = 1; i < argc && cmdIndex < 5; i++) {
        if (strcmp(argv[i], "-") == 0 || i == argc - 1) {
            if (i == argc - 1) { // Last command case
                commands[cmdIndex].args[argIndex] = argv[i];
                argIndex++;
            }

            commands[cmdIndex].args[argIndex] = 0; // Null terminate args array
            commands[cmdIndex].execTime = measureExecTime(commands[cmdIndex].args);
            cmdIndex++;
            argIndex = 0;
        } else {
            commands[cmdIndex].args[argIndex++] = argv[i];
        }
    }

    // Sort commands based on measured execution times
    sortCommands(commands, cmdIndex);

    // Execute commands in sorted order
    int arraival_time = uptime();
    for (int i = 0; i < cmdIndex; i++) {
        int pid = fork();
        int start_time = uptime();
        if (pid == 0) {
            exec(commands[i].args[0], commands[i].args);

            exit();
        }
        //waitAndGetProcTimes(pid);
        //execution_time = ticks_running(pid);
        wait();
        int end_time = uptime();
        //execution_time = ticks_running(pid);
        printf(1,"----------- Process Times -----------\n");
        //cprintf("Name: %s\n", p->name);
        //cprintf("PID: %d\n", p->pid);
        //cprintf("arrival time: 0");
        completion_times[i] = end_time;
        printf(1, "arraival time : %d ticks", arraival_time);
        printf(1,"execution start Time: %d ticks\n", start_time);
        printf(1,"execution end Time: %d ticks\n", end_time);
        printf(1, "executiom Time: %d ticks\n", end_time - start_time);
        printf(1,"turnaround time : %d ticks\n", end_time - arraival_time);
        printf(1,"response time: %d ticks\n", start_time - arraival_time);
        printf(1,"------------------------------------\n");
        tat = tat + end_time - arraival_time;
        res = res + start_time - arraival_time;

        //printf(1," joblegth of %d is %d", commands[i].execTime);
    }
    printf(1, " average turn around time is : %d", tat/3);
    printf(1, " average resposne time is : %d", res/3);
    int sum1 = 0;
    int sum2 = 0;
    int min = 0;
    int max = completion_times[0];
    for(int i = 0; i < 3; i++){
        sum1 += completion_times[i];
        sum2 += completion_times[i] * completion_times[i];
        if (completion_times[i] < min){
            min = completion_times[i];
        }
        if (completion_times[i] > max){
            max = completion_times[i];
        }

    }
    printf(1, " jain fairness metrics is : %d / %d", sum1, sum2);
    printf(1, "min max ratio is : %d  / %d", min, max);

    exit();
}
