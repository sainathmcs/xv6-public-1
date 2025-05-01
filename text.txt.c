
// #include "types.h"
// #include "stat.h"
// #include "user.h"

// int readlinetoarray(int fd1, char *buf) {
//     char chr[1];
//     int n;
//     int start = 0;
//     while ((n = read(fd1, chr, sizeof(chr))) > 0 && start < 511) {
//         if ((int)chr[0] == 10) {
//             buf[start++] = '\0';
//             return 1;
//         } else {
//             buf[start++] = chr[0];
//         }
//     }
//     if (n < 0) {
//         printf(1, "uniq: read error\n");
//         exit();
//     }
//     buf[start++] = '\0';
//     return 2;
// }
// int tolower(int ch) {
//     if (ch >= 'A' && ch <= 'Z') {
//         return ch + ('a' - 'A');
//     }
//     return ch;
// }

// int strcasecmp(const char *s1, const char *s2) {
//     while (*s1 && *s2) {
//         int c1 = tolower((unsigned char)*s1);
//         int c2 = tolower((unsigned char)*s2);
        
//         if (c1 != c2)
//             return c1 - c2;
        
//         s1++;
//         s2++;
//     }

    
//     if (*s1)
//         return 1;
//     else if (*s2)
//         return -1;
    
//     return 0;  
// }

// int compare_lines(const char *line1, const char *line2, int flag_ignore) {
//     if (flag_ignore)
//         return strcasecmp(line1, line2);
//     else
//         return strcmp(line1, line2);
// }

// void uniq1(const char *filename, int flag_ignore, int flag_count, int flag_dup,int isfile) {
    
//     int fd1 = 0, n;
//     if (isfile && (fd1 = open(filename, 0)) < 0) {
//         printf(1, "uniq: cannot open %s\n", filename);
//         exit();
//     }
//     char current[512];
//     char previous[512] = ""; 
//     int count = 0; 
//     while ((n = readlinetoarray(fd1, current))) {
//         int cmp = compare_lines(current, previous, flag_ignore);
//         if (cmp != 0 ) {
//             if (!flag_dup) {
//                 if(flag_count) printf(1, "%d %s\n", count , previous); 
//                 else {printf(1, "%s\n", previous);} 
//                 }
//             else if(flag_dup && (count > 1)){
                
//                 if(flag_count) {printf(1, "%d %s\n", count , previous);}
//                 else{printf(1, "%s\n", previous);}
//             }
            
//             strcpy(previous, current); 
//             count= 1;
//         } else {
//             count++;
//         }
//         if (n == 2) {
//             //
//             if (!flag_dup) {
//                 if(flag_count) printf(1, "%d %s\n", count , previous); 
//                 else {printf(1, "%s\n", previous);} 
//                 }
//             else if(flag_dup && (count > 1)){
                
//                 if(flag_count) {printf(1, "%d %s\n", count , previous);}
//                 else{printf(1, "%s\n", previous);}
//             }
//             //
//             exit();
//         }
//     }

    
//     if (flag_count && !flag_dup)
//         printf(1, "%d %s\n", count + 1, previous);
//     else if (!flag_dup)
//         printf(1, "%s\n", previous);
// }

// int main(int argc, char const *argv[]) {
//     printf(1,"uniq command is getting executed in user mode.\n");
//     int flag_ignore = 0, flag_count = 0, flag_dup = 0;

//     if (argc == 1) {
//         printf(1, "if1");
//         uniq1(argv[0], flag_ignore, flag_count, flag_dup,0); 
//     } else {
//         printf(1, "else 1");
        
//         for (int i = 1; i < argc; i++) {
//             if (argv[i][0] == '-') {
//                 switch (argv[i][1]) {
//                 case 'i':
//                     flag_ignore = 1;
//                     break;
//                 case 'c':
//                     flag_count = 1;
//                     break;
//                 case 'd':
//                    flag_dup = 1;
//                     break;
//                 default:
//                     printf(1, "uniq: invalid option: %s\n", argv[i]);
//                     exit();
//                 }
//             } else {
//                 printf(1, "else2");
//                 uniq1(argv[i], flag_ignore, flag_count, flag_dup,1);
//                 exit();
//             }
            
//         }
//     }

//     exit();
// }
//888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_LINE_SIZE 512

int readLine(int fd, char *buffer) {
    char character;
    int bytesRead;
    int position = 0;

    while ((bytesRead = read(fd, &character, sizeof(character))) > 0 && position < MAX_LINE_SIZE - 1) {
        if (character == '\n') {
            buffer[position++] = '\0';
            return 1;
        } else {
            buffer[position++] = character;
        }
    }

    if (bytesRead < 0) {
        printf(1, "Error reading file\n");
        exit();
    }

    buffer[position++] = '\0';
    printf(1, "Read: %s\n", buffer);
    return bytesRead == 0 ? 2 : -1;
}

int strncmp(const char *s1, const char *s2, int width) {
    while (width-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return width < 0 ? 0 :*(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void displayLine(const char *line, int count, int countLines) {
    if (countLines)
        printf(1, "%d %s\n", count, line);
    else
        printf(1, "%s\n", line);
}
void executeUniq(int fd, int countLines, int uniqueLines, int width) {
    char currentLine[MAX_LINE_SIZE];
    char lastLine[MAX_LINE_SIZE] = {0};
    int count = 0;
    int readStatus;
    int isFirstLine = 1;  // Flag for first line

    while ((readStatus = readLine(fd, currentLine)) != 0) {
        // Debugging output
        //printf(1, "Read line: %s\n", currentLine);

        if (isFirstLine) {
            // For the first line, copy it to lastLine
            strcpy(lastLine, currentLine);
            count = 1;
            isFirstLine = 0;
        } else {
            int comparison = strncmp(currentLine, lastLine, width);
            //printf(1, "Comparison result: %d\n", comparison);  // Debugging print

            if (comparison != 0) {
                if (count > 0 && (!uniqueLines || count == 1))
                    displayLine(lastLine, count, countLines);

                strcpy(lastLine, currentLine);
                count = 1;
            } else {
                count++;
            }
        }

        if (readStatus == 2) break;  // End of file
    }

    if (count > 0 && (!uniqueLines || count == 1)) {
        displayLine(lastLine, count, countLines);
    }
}


// void executeUniq(int fd, int countLines, int uniqueLines, int width) {
//     char currentLine[MAX_LINE_SIZE];
//     char lastLine[MAX_LINE_SIZE] = "";
//     int count = 0;
//     int readStatus;

//     while ((readStatus = readLine(fd, currentLine)) != 0) {
//         printf(1, "Read line: %s\n", currentLine);  // Debugging print

//         int comparison = strncmp(currentLine, lastLine, width);
//         printf(1, "Comparison result: %d\n", comparison);  // Debugging print

//         if (comparison != 0 && count > 0) {
//             if (!uniqueLines || count == 1)
//                 displayLine(lastLine, count, countLines);

//             strcpy(lastLine, currentLine);
//             count = 1;
//         } else {
//             count++;
//         }

//         if (readStatus == 2) { // End of file
//             break;
//         }
//     }

//     if ((count > 0) && (!uniqueLines || count == 1)) {
//         displayLine(lastLine, count, countLines);
//     }
// }

// void executeUniq(int fd, int countLines, int uniqueLines, int width) {
//     char currentLine[MAX_LINE_SIZE];
//     char lastLine[MAX_LINE_SIZE] = "";
//     int count = 0;
//     int readStatus;

//     while ((readStatus = readLine(fd, currentLine)) != 0) {
//         int comparison = strncmp(currentLine, lastLine, width);

//         if (comparison != 0 && count > 0) {
//             if (!uniqueLines || count == 1)
//                 displayLine(lastLine, count, countLines);

//             strcpy(lastLine, currentLine);
//             count = 1;
//         } else {
//             count++;
//         }

//         if (readStatus == 2) { // End of file
//             break;
//         }
//     }

//     if ((count > 0) && (!uniqueLines || count == 1)) {
//         displayLine(lastLine, count, countLines);
//     }
// }

int main(int argc, char *argv[]) {
    int countLines = 0, uniqueLines = 0, width = MAX_LINE_SIZE;
    int fd = 0; // Default to standard input
    char *filename = (void *)0;
    // First, parse all arguments to set the flags
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'c':
                    countLines = 1;
                    break;
                case 'u':
                    uniqueLines = 1;
                    break;
                case 'w':
                    if (i + 1 < argc) {
                        width = atoi(argv[++i]);
                    } else {
                        printf(1, "uniq: '-w' requires an argument\n");
                        exit();
                    }
                    break;
                default:
                    printf(1, "uniq: invalid option: %s\n", argv[i]);
                    exit();
            }
        } else {
            // Store filename for later use
            filename = argv[i];
        }
    }
        if (filename != (void *)0) {
        fd = open(filename, 0);
        if (fd < 0) {
            printf(1, "uniq: cannot open %s\n", filename);
            exit();
        }
    }
    // Now that all flags are set, execute uniq
    executeUniq(fd, countLines, uniqueLines, width);

    if (fd != 0) {
        close(fd);
    }

    exit();
}