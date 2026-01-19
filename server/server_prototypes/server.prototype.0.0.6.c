#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

// Garbage attempt

#define PORT 8080
#define BUFFER_SIZE 1024

void *write_date(void *arg) {
    FILE *file = fopen("date.txt", "a");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    while (1) {
        time_t current_time = time(NULL);
        char *date_str = ctime(&current_time);
        fprintf(file, "%s", date_str);
        fflush(file);
        sleep(10);
    }

    fclose(file);
    return NULL;
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    FILE *client_file = fopen("client.txt", "a");
    if (client_file == NULL) {
        perror("Error opening file");
        close(client_socket);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            break;
        }

        buffer[bytes_received] = '\0';
        fprintf(client_file, "%s", buffer);
        fflush(client_file);
        send(client_socket, buffer, bytes_received, 0);
    }

    fclose(client_file);
    close(client_socket);
    return NULL;
}

int main() {
    pthread_t date_thread;
    pthread_create(&date_thread, NULL, write_date, NULL);

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return 1;
    }

    listen(server_socket, 5);
    printf("Server listening on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, client_socket_ptr);
        pthread_detach(client_thread);
    }

    close(server_socket);
    return 0;
}