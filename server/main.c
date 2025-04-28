#include "include/filesmng.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>


#define MAX_CLIENTS 10
#define PORT 22


int server_fd = -1;
int client_sockets[MAX_CLIENTS];

__attribute__((destructor)) void cleanup() {
    if (server_fd != -1) {
        close(server_fd);
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != -1) {
            //Send close message
            close(client_sockets[i]);
        };
    };
}

void init() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
    }
}


void thread_cleanup(int client_fd) {
    printf("Client %d disconnected\n", client_fd);
    close(client_fd);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == client_fd) {
            client_sockets[i] = -1;
            break;
        }
    };
    pthread_exit(NULL);
};

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg); // free the dynamically allocated memory
    const char passw[] = "123#@29"; 
    int passwd_len = strlen(passw);
    char buffer[1024];
    int bytes_read;
    int out_buff_len = 0;
    //char* dirs = NULL;
    //char* files = NULL;
    char* dirs[255][MAX_PATH_LENGTH];
    char* files[255][MAX_PATH_LENGTH];

    char dir[MAX_PATH_LENGTH] = "./files";
    printf("Handling client %d\n", client_fd);

    // Receive password attempt
    bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        printf("Failed to receive password from client %d\n", client_fd);
        thread_cleanup(client_fd);
        pthread_exit(NULL);
    }

    buffer[bytes_read] = '\0'; // Null-terminate

    printf("Received from client %d: %s\n", client_fd, buffer);

    // Check password length first
    if (bytes_read != passwd_len) {
        printf("Password length mismatch for client %d\n", client_fd);
        thread_cleanup(client_fd);
        pthread_exit(NULL);
    }

    // Check if password matches
    if (strncmp(buffer, passw, passwd_len) != 0) {
        printf("Invalid password from client %d\n", client_fd);
        thread_cleanup(client_fd);
        pthread_exit(NULL);
    }

    // Authenticated!
    send(client_fd, "AUTHENTICATED\n", strlen("AUTHENTICATED\n"), 0);
    list_directory("./files",dirs,files,&out_buff_len, buffer);
    send(client_fd,buffer,out_buff_len,0);
    printf("len: %d\n",out_buff_len);
    printf("%s",buffer);

    printf("\n");
    // Now echo back everything the client sends
    while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate
        printf("Received from client %d: %s\n", client_fd, buffer);
        send(client_fd, buffer, bytes_read, 0);
    };

    printf("Client %d disconnected.\n",client_fd);
    thread_cleanup(client_fd);
    pthread_exit(NULL);
}

int main() {
    init();
    struct sockaddr_in sockaddr;
    socklen_t addr_len = sizeof(sockaddr);
    int* client_fd;
    pthread_t thread_id;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Zero out sockaddr
    memset(&sockaddr, 0, sizeof(sockaddr));
    
    sockaddr.sin_family = AF_INET; // You forgot this line
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("Bind failed");
        return -1;
    }

    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        return -1;
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        client_fd = malloc(sizeof(int)); // allocate memory for client fd
        if (!client_fd) {
            perror("Malloc failed");
            continue;
        }

        *client_fd = accept(server_fd, (struct sockaddr*)&sockaddr, &addr_len);
        if (*client_fd == -1) {
            perror("Accept failed");
            free(client_fd);
            continue;
        }

        printf("New client connected: %d\n", *client_fd);

        // Save client socket
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == -1) {
                client_sockets[i] = *client_fd;
                break;
            }
        }

        if (pthread_create(&thread_id, NULL, handle_client, client_fd) != 0) {
            perror("Thread creation failed");
            close(*client_fd);
            free(client_fd);
        } else {
            pthread_detach(thread_id); // Auto clean up thread resources after it exits
        }
    }

    return 0;
}
