#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "include/ssl.h"
#include "include/stdoutmod.h"
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef _WIN32
#include <direct.h>
#endif

#pragma comment(lib, "ws2_32.lib")

#define MAX_PATH_LEN 1024
#define MAX_PATHS 255
#define NUM_ACTIONS 1
#define SERVER_IP "192.168.1.229"  // Change if needed
#define SERVER_PORT 22
//123#@29

const char* BLUE = "\033[34m";
const char* GREEN = "\033[32m";
const char* RED = "\033[31m";
const char* YELLOW = "\033[33m";
const char* COL_RESET = "\033[0m";

typedef enum {
    GETF = 0x1,
    SENDF = 0x2,
    MK_DIR = 0x3,
    RM_DIR = 0x4,
    ENTER_DIR = 0x5,
    SUCESS = 0x6,
    FAIL = 0x7
} ACTIONS;

WSADATA wsa;
SOCKET sock;
struct sockaddr_in server;
char buffer[1024];
int current_buf_size;
int recv_size;
char stub[MAX_PATH];
char current_dir[MAX_PATH_LEN] = "/";
char currentwd[MAX_PATH_LEN];
int selected = 1;
SSL *ssl;
const char* actions[NUM_ACTIONS] = { "Create Directory" };

void print_header() {
    printf("%s CURRENT DIRECTORY %s:\t%s\n",BLUE,COL_RESET,current_dir);
    printf("%s CURRENT WORKING DIRECTORY %s:\t%s\n\n",BLUE,COL_RESET,currentwd);
};


void read_items(){
    current_buf_size = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (current_buf_size == SOCKET_ERROR || recv_size == 0) {
        printf("Recv failed or connection closed.\n");
        return;
    }
    
    buffer[current_buf_size] = '\0';
};

void send_action(char* itmname, ACTIONS action) {
    unsigned char raw_buffer[64]; // Make sure it's big enough
    size_t name_len = strlen(itmname);

    if (name_len + 1 > sizeof(raw_buffer)) {
        printf("Item name too long!\n");
        return;
    }

    raw_buffer[0] = (unsigned char)action;        // 1st byte: the ACTION
    memcpy(raw_buffer + 1, itmname, name_len);     // after that: the name

    if (SSL_write(ssl, raw_buffer, name_len) <= 0) {
        printf("Send failed.\n");
        return;
    }
    
    // Receive server reply
    recv_size = SSL_read(ssl, raw_buffer, sizeof(raw_buffer) - 1);
    if (recv_size == SOCKET_ERROR || recv_size == 0) {
        printf("Recv failed or connection closed.\n");
        return;
    };
    raw_buffer[recv_size] = '\0';
    switch (action)
    {
    case MK_DIR:
        if (strncmp(raw_buffer,"FAILED",strlen("FAILED")) == 0) {
            printf("%sFAILED%s\n",RED,COL_RESET);
            _sleep(1000);
        } else {
            printf("%sRECIEVED%s: %s\n",GREEN,COL_RESET,raw_buffer);
            _sleep(1000);
        };
        
        return;
    case ENTER_DIR: {
        if (strncmp(buffer,"FAILED",strlen("FAILED"))) {
            printf("%sFAILED%s\n",RED,COL_RESET);
            _sleep(1000);
        };
        return;
    };
    default:
        break;
    }
    
    return;
};  

void get_and_send_stub(){
    printf("Determining stub.\n");
    _sleep(1000);
    int count = 1; 
    int chars = 0;
    int has_dot = 0;
    for (int i = 0; i < current_buf_size; i++) {
        if (chars == MAX_PATH) { 
            printf("Invalid option. To long\n");
            return;
        };
        if (buffer[i] == '\n') {
            if (selected == count) {
                stub[i] = '\0';
                if (has_dot == 0) {
                    send_action(stub,ENTER_DIR);
                    return;
                } else {
                    send_action(stub,GETF);
                    return;
                }
            };
            count++;
            chars = 0;
            stub[0] = '\0';
            return;
        } else {
            if (buffer[i] == '.') {
                has_dot = 1;
            };
            stub[chars] = buffer[i];
            chars++;
        };

    };
};

void print_options() {
    int option_num = 1;
    int total_items = 1;
    while (1) {
        cls();
        system("cls");
        print_header();
        option_num = 1;
        total_items = 1;
        if (selected == option_num) {
            printf("> %s",BLUE);
        } else {
            printf("%d. ",option_num);
        };
        
        for (int i = 0; i < current_buf_size; i++) {
    
            
            if (i + 1 < current_buf_size) {
                printf("%c",buffer[i]);
                if (buffer[i + 1] == '\n') {
                    if (option_num == selected) {
                        printf("%s <",COL_RESET);
                    };
                };
                if (buffer[i] == '\n') {
                    option_num++;
                    total_items++;
                    if (selected == option_num) {
                        printf("> %s",BLUE);
                    } else {
                        printf("%d. ",option_num);
                    };
                };
            };
        };
        printf("\n\nTotal options: %d\n",option_num);
        printf("\n\n---- %sACTIONS%s ----\n\n",YELLOW,COL_RESET);
        for (int i = 0; i < NUM_ACTIONS; i++) {
            option_num++;
            if (option_num == selected) {
                printf("> %s%s%s <\n",BLUE,actions[i],COL_RESET);
            } else {
                printf("%d. %s\n",(i + 1),actions[i]);
            };
        };
        
        int key = get_key();
        switch (key) {
        case CNTRLC_KEY:
            printf("CNTRL C\t%sExiting...%s\n",RED,COL_RESET);
            exit(0);
            break;
        case UP_KEY:
            if (0 != (option_num + NUM_ACTIONS)) {
                selected--;
            };
            break;
        case DOWN_KEY:
            if (selected + 1 <= option_num) {
                selected++;
            };
            break;
        case ENTER_KEY:        
            int diff = selected - total_items;
            char name[MAX_PATH];
            if (diff > 0) {
                switch (diff)
                {
                case 1:
                    printf("%sENTER%s Directory name: ",YELLOW,COL_RESET);
                    fgets(&name[0], sizeof(name), stdin);
                    printf("\n%sMAKING%s directory %s%s%s\n",YELLOW,COL_RESET,BLUE,name,COL_RESET);
                    send_action(&name[0],MK_DIR);
                    break;
                
                default:
                    
                    break;
                }
            } else {
                get_and_send_stub();
            };
            break;
        default:
            printf("Key %d/n",key);
            printf("Invalid command.\n");
            break;
        };
    };
};



int authenticate() {
    printf("Enter password (or 'exit' to quit): ");
    fgets(buffer, sizeof(buffer), stdin);

    // Remove trailing newline
    buffer[strcspn(buffer, "\n")] = 0;

    if (strcmp(buffer, "exit") == 0) {
        return 2;
    }

    // Send password
    if (SSL_write(ssl, buffer, strlen(buffer)) < 0) {
        printf("Send failed.\n");
        return -1;
    }

    // Receive server reply
    recv_size = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (recv_size == SOCKET_ERROR || recv_size == 0) {
        printf("Recv failed or connection closed.\n");
        return -1;
    }

    buffer[recv_size] = '\0';

    printf("%s%s%s\n", GREEN,buffer,COL_RESET);
    // Check if server replied with "AUTHENTICATED"
    if (strncmp(buffer, "AUTHENTICATED", strlen("AUTHENTICATED")) == 0) {
            // Receive server reply
#ifdef _WIN32
        if (_getcwd(currentwd, sizeof(currentwd)) == NULL) {
            perror("_getcwd() error");
            return 1;
        }
#endif

        read_items();
        print_options();
        return 0; // Success
    } else {
        return 1; // Failed authentication
    }
}

int main() {
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Winsock initialized.\n");
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Socket created.\n");

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to remote server
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Connect error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);
#ifdef USESSL
    init_openssl();
    SSL_CTX *ctx = create_context();
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    configure_context(ctx);

#endif
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, (int)sock);
    printf("Performing handshake.\n");
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        goto discon;
        return -1;
    } else {
        printf("TLS connection established!\n");
    };
    
    int res = authenticate();
    if (res != 0) {
        goto discon;
    };
    
    while (1) {
        printf("Enter option (or 'exit' to quit): ");
        fgets(buffer, sizeof(buffer), stdin);

        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Send data
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            printf("Send failed.\n");
            break;
        }

        // Receive response
        if ((recv_size = recv(sock, buffer, sizeof(buffer) - 1, 0)) == SOCKET_ERROR) {
            printf("Recv failed.\n");
            break;
        }

        buffer[recv_size] = '\0';
        printf("Server reply: %s\n", buffer);
    };

    discon:
    SSL_shutdown(ssl);
    SSL_free(ssl);
    closesocket(sock);

    SSL_CTX_free(ctx);
    cleanup_openssl();
    WSACleanup();

    printf("Disconnected.\n");

    return 0;
}
