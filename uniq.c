#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_LINE_SIZE 512

// Fetch line function is used to read a single line from the file.
int fetchLine(int fd, char *buffer) {
    char character;
    int bytesRead;
    int bufferIndex = 0;

    // while loop reads from the file until it reaches a newline character or bufferIndex reaches compareWidth.
    while ((bytesRead = read(fd, &character, sizeof(character))) > 0 && bufferIndex < MAX_LINE_SIZE - 1) {
        if (character == '\n') {
            buffer[bufferIndex++] = '\0';
            return 1; // on valid line read.
        } else {
            buffer[bufferIndex++] = character;
        }
    }

    // handling file read error.
    if (bytesRead < 0) {
        printf(1, "Error reading file\n");
        exit();
    }

    buffer[bufferIndex++] = '\0'; //terminate the string.
    //printf(1, "Read: %s\n", buffer); 
    return bytesRead == 0 ? 2 : -1; //returns 2 if EOF is reached or else returns -1.
}

// compareString is used to compare stings of certain length so that it can handle -w flag.
int compareStrings(const char *s1, const char *s2, int compareWidth) {
    while (compareWidth-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return compareWidth < 0 ? 0 :*(const unsigned char *)s1 - *(const unsigned char *)s2;
}

//function to write lines it can handle uniq and uniq -c commands.
void displayLine(const char *line, int lineCount, int includeCount) {
    if (includeCount)
        printf(1, "%d %s\n", lineCount
, line);
    else
        printf(1, "%s\n", line);
}

// executeUniq function handles all the operations of uniq
void executeUniq(int fd, int includeCount, int uniqueOnly, int compareWidth) {
    char currentLine[MAX_LINE_SIZE];
    char lastLine[MAX_LINE_SIZE] = {0};
    int lineCount = 0;
    int lineReadStatus;
    int isFirstLine = 1;  // Flag to check if its the first line.

    while ((lineReadStatus = fetchLine
(fd, currentLine)) != 0) {
        
        //printf(1, "Read line: %s\n", currentLine);

        if (isFirstLine) {
            // handling first line.
            strcpy(lastLine, currentLine);
            lineCount = 1;
            isFirstLine = 0;
        } else {
            int comparison = compareStrings(currentLine, lastLine, compareWidth);
            //printf(1, "Comparison result: %d\n", comparison);  // Debugging print
            //output the previous line if conditions are true.
            if (comparison != 0) {
                if (lineCount > 0 && (!uniqueOnly || lineCount == 1))
                    displayLine(lastLine, lineCount, includeCount);

                strcpy(lastLine, currentLine); // making current line as last line.
                lineCount
         = 1;
            } else {
                lineCount
        ++;
            }
        }

        if (lineReadStatus == 2) break;  // End of file
    }

    // handling the last line if we want to display the last line displaying it
    if (lineCount > 0 && (!uniqueOnly || lineCount == 1)) {
        displayLine(lastLine, lineCount
, includeCount);
    }
}

int main(int argc, char *argv[]) {
    int includeCount = 0, uniqueOnly = 0, compareWidth = MAX_LINE_SIZE;
    int fd = 0; // Default input
    char *filename = (void *)0;
    
    // First, parse all arguments to set the flags
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'c':
                    includeCount = 1;
                    break;
                case 'u':
                    uniqueOnly = 1;
                    break;
                case 'w':
                    if (i + 1 < argc) {
                    // Check if the next argument is a digit or if no digit is provided then print error
                        char* nextArg = argv[++i];
                        for (int j = 0; nextArg[j] != '\0'; j++) {
                            if (nextArg[j] < '0' || nextArg[j] > '9') {
                                printf(1, "uniq: '-w' requires a numeric argument\n");
                                exit();
                            }
                        }
                        compareWidth = atoi(nextArg);   
                    } 
                    else {
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
    // execute uniq now as all the flags are set
    executeUniq(fd, includeCount, uniqueOnly, compareWidth);

    if (fd != 0) {
        close(fd);
    }

    exit();
}