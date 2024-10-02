#include <stdio.h>
#include <unistd.h>

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
    printDelay("LOGON:  ");
    fgets(logon, sizeof(logon), stdin);
    if (logon != "Joshua"){
        usleep(3000000);
        printDelay("INDENTIFICATION NOT RECOGNIZED BY SYSTEM\n");
        printDelay("--CONNECTION TERMINATED--");
    }
    return 0;
}
