#include <stdio.h>
#include <stdlib.h>

enum Keys {
    CNTRLC_KEY = -1,
    UP_KEY = 1,
    DOWN_KEY = 2,
    LEFT_KEY = 3,
    RIGHT_KEY = 4,
    ENTER_KEY = 10,
};

#ifdef _WIN32
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

int get_key() {
#ifdef _WIN32
    int ch = 220;
    while (ch > 200)
        ch = _getch();
        //printf("Key %d\n",ch);
        switch (ch) {
            case 72: return 1; // Up arrow
            case 80: return 2; // Down arrow
            case 75: return 3; // Left arrow
            case 77: return 4; // Right arrow
            case 13: return 10;
            case 3: return -1;
            default: return 0;
    };
    return ch;
#else
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    if (ch == 27) { // ESC
        ch = getchar();
        if (ch == 91) { // '['
            ch = getchar();
            switch (ch) {
                case 'A': 
                    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                    return 1; // Up arrow
                case 'B': 
                    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                    return 2; // Down arrow
                case 'D':
                    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                    return 3; // Left arrow
                case 'C':
                    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                    return 4; // Right arrow
                default:
                    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                    return 0;
            }
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return 0;
#endif
}

/*
void parse_key(){
    int key = get_key();
    switch (key) {
    case CNTRLC_KEY:
        printf("CNTRL C\tExiting..\n");
        exit(0);
        return;
    case UP_KEY:
        printf("You pressed UP!\n");
        return;
    case DOWN_KEY:
        printf("You pressed DOWN!\n");
        return;
    case LEFT_KEY:
        printf("You pressed LEFT!\n");
        return;
    case RIGHT_KEY:
        printf("You pressed RIGHT!\n");
        return;
    case ENTER_KEY:
        printf("Clicked enter\n");
        return;
    default:
        printf("Invalid command.\n");
        return;
    };
};
*/

void cls() {
#ifdef _WIN32
    system("cls");
#endif
};

