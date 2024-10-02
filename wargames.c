#include <stdio.h>
#include <unistd.h>
#include <string.h>

void moveCursorRight(int steps) {
    for (int i = 0; i < steps; i++) {
        printf("  ");
        fflush(stdout);
        usleep(50000);
    }
}
void printDelay(const char *str) {
    while (*str) {
        printf("%c", *str);
        fflush(stdout);
        usleep(30000);  
        str++;
    }
}

int main() {
    moveCursorRight(7);
    printf("               ");
    moveCursorRight(16);
    printf("                         ");
    moveCursorRight(13);
   moveCursorRight(7);
    printf("               ");
    moveCursorRight(16);
    printf("                         ");
    moveCursorRight(13);
    char logon[255]; 
    int logonLoop = 1;
    while (logonLoop){
        printDelay("LOGON:  ");
        fgets(logon, sizeof(logon), stdin);
        logon[strcspn(logon, "\n")] = 0;
        usleep(3000000);
        if (strcmp(logon, "Joshua") == 0){
            printDelay("OK\n");
            logonLoop = 0;
        }
        else if (strcmp(logon, "Help Logon") == 0){
            printDelay("HELP NOT AVAILABLE\n\n");
        }
        else {
            printDelay("INDENTIFICATION NOT RECOGNIZED BY SYSTEM\n");
            printDelay("--CONNECTION TERMINATED--");
            logonLoop = 0;
        }
    }
    return 0;
}
