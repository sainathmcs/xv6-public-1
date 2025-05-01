// #include "types.h"
// #include "user.h"
// #include "stat.h"

// struct CommandInfo {
//     char *cmd;      // Command string
//     char *args[10]; // Command arguments
// };

// // Function to execute a command and measure its execution time
// int executeCommand(char *args[]) {
//     int start_time = uptime(); // Record start time
//     int pid = fork();
//     if (pid == 0) {
//         exec(args[0], args);
//         exit();
//     } else {
//         wait();
//         int end_time = uptime(); // Record end time
//         return end_time - start_time;
//     }
// }

// int main(int argc, char *argv[]) {
//     struct CommandInfo commands[5]; // Hypothetically assuming a maximum of 5 commands
//     int cmdIndex = 0;
//     int argIndex = 0;
//     int tat = 0;
//     int res = 0;
//     int completion_times[5];
//     int start_time;
//     int end_time;
//     int arraival_time;

//     // Parse command-line arguments and store commands
//     for (int i = 1; i < argc && cmdIndex < 5; i++) {
//         if (strcmp(argv[i], "-") == 0 || i == argc - 1) {
//             if (i == argc - 1) {
//                 commands[cmdIndex].args[argIndex] = argv[i];
//                 argIndex++;
//             }
//             commands[cmdIndex].args[argIndex] = 0; // Null terminate args array
//             cmdIndex++;
//             argIndex = 0;
//         } else {
//             commands[cmdIndex].args[argIndex++] = argv[i];
//         }
//     }

//     // Execute commands one by one
//     int arraival_time = uptime();
//     for (int i = 0; i < cmdIndex; i++) { 
//         int pid = fork();
//         int start_time = uptime();
//         if (pid == 0) {
//             exec(args[0], args);
//             exit();
//         } else {
//             wait();
//             int end_time = uptime(); // Record end time
//             r
//     }
//         printf(1,"----------- Process Times -----------\n");
//         completion_times[i] = end_time;
//         printf(1, "arraival time : %d ticks", arraival_time);
//         printf(1,"execution start Time: %d ticks\n", start_time);
//         printf(1,"execution end Time: %d ticks\n", end_time);
//         printf(1, "executiom Time: %d ticks\n", end_time - start_time);
//         printf(1,"turnaround time : %d ticks\n", end_time - arraival_time);
//         printf(1,"response time: %d ticks\n", start_time - arraival_time);
//         printf(1,"------------------------------------\n");
//         tat = tat + end_time - arraival_time;
//         res = res + start_time - arraival_time;

//         completion_times[i] = exec_time;
//         turnaround_sum += exec_time;
//         response_sum += exec_time;
//     }

//     // Calculate averages
//     int avg_turnaround = turnaround_sum / cmdIndex;
//     int avg_response = response_sum / cmdIndex;

//     // Calculate Jain's fairness metric
//     int sum1 = 0;
//     int sum2 = 0;
//     int min = 1000000;
//     int max = 0;
//     for (int i = 0; i < cmdIndex; i++) {
//         sum1 += completion_times[i];
//         sum2 += completion_times[i] * completion_times[i];
//         if (completion_times[i] < min) {
//             min = completion_times[i];
//         }
//         if (completion_times[i] > max) {
//             max = completion_times[i];
//         }
//     }
//     float fairness_metric = (sum1 * sum1) / (cmdIndex * sum2);

//     // Print results
//     printf(1, "Average Turnaround Time: %d ticks\n", avg_turnaround);
//     printf(1, "Average Response Time: %d ticks\n", avg_response);
//     printf(1, "Jain's Fairness Metric sum1 and sum 2: %f\n", fairness_metric );
//     printf(1, "Minimum Completion Time: %d ticks\n", min);
//     printf(1, "Maximum Completion Time: %d ticks\n", max);

//     exit();
// }
