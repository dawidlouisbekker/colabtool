#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"  // Change if needed
#define SERVER_PORT 22
//123#@29
WSADATA wsa;
SOCKET sock;
struct sockaddr_in server;
char buffer[1024];
int recv_size;

int authenticate() {
    printf("Enter password (or 'exit' to quit): ");
    fgets(buffer, sizeof(buffer), stdin);

    // Remove trailing newline
    buffer[strcspn(buffer, "\n")] = 0;

    if (strcmp(buffer, "exit") == 0) {
        return 2;
    }

    // Send password
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        printf("Send failed.\n");
        return -1;
    }

    // Receive server reply
    recv_size = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (recv_size == SOCKET_ERROR || recv_size == 0) {
        printf("Recv failed or connection closed.\n");
        return -1;
    }

    buffer[recv_size] = '\0';

    printf("Server reply: %s\n\n", buffer);

    // Check if server replied with "AUTHENTICATED"
    if (strncmp(buffer, "AUTHENTICATED", strlen("AUTHENTICATED")) == 0) {
            // Receive server reply
        recv_size = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (recv_size == SOCKET_ERROR || recv_size == 0) {
            printf("Recv failed or connection closed.\n");
            return -1;
        }
        buffer[recv_size] = '\0';

        printf("--- DIR ---\n %s\n\n", buffer);
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
    int res = authenticate();
    if (res != 0) {
        goto discon;
    };
    
    while (1) {
        printf("Enter message (or 'exit' to quit): ");
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
    closesocket(sock);
    WSACleanup();

    printf("Disconnected.\n");

    return 0;
}
