#include <stdio.h>
#include <unistd.h>

void moveCursorRight(int steps) {
    for (int i = 0; i < steps; i++) {
        printf("  "); // Print the cursor
        fflush(stdout); // Ensure the output is displayed immediately
        usleep(50000); // Sleep for 25 milliseconds
    }
}
void printLOGON(){
    printf("L");
    fflush(stdout);
    usleep(25000);
    printf("O");
    fflush(stdout);
    usleep(25000);
    printf("G");
    fflush(stdout);
    usleep(25000);
    printf("O");
    fflush(stdout);
    usleep(25000);
    printf("N");
    fflush(stdout);
    usleep(25000);
    printf(":");
    fflush(stdout);
    usleep(25000);
    printf("  ");
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
    printLOGON();
    fgets(logon, sizeof(logon), stdin);
    return 0;
}
