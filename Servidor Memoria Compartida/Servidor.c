#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#define PORT 8080
#define SHM_KEY 0xa
#define SHM_SIZE 512
#define PAGE_SIZE 32
#define NUM_PAGES (SHM_SIZE / PAGE_SIZE)

typedef struct {
    char data[256];
    int occupied;
    sem_t sem; // Semaphore for each memory page
} SharedString;

SharedString *shared_memory;

void show_shared_memory_table() {
    printf("Shared Memory Table:\n");
    for (int i = 0; i < NUM_PAGES; i++) {
        printf("Position %d - Occupied: %d - Data: %s\n", i, shared_memory[i].occupied, shared_memory[i].data);
    }
}

void *handle_client(void *client_socket) {
    int new_socket = *((int *)client_socket);
    free(client_socket);

    char buffer[1024] = {0};
    int valread = read(new_socket, buffer, 1024);
    printf("Received: %s\n", buffer);

    // Parse command
    char command[20];
    int index;
    char value[256];
    sscanf(buffer, "%s %d %[^\n]", command, &index, value);

    if (index < 0 || index >= NUM_PAGES) {
        char *response = "Index out of bounds\n";
        send(new_socket, response, strlen(response), 0);
    } else if (strcmp(command, "write") == 0) {
        if (sem_trywait(&shared_memory[index].sem) == 0) {
            strcpy(shared_memory[index].data, value);
            shared_memory[index].occupied = 1;
            sem_post(&shared_memory[index].sem);
            char *response = "Write successful\n";
            send(new_socket, response, strlen(response), 0);
        } else {
            char *response = "Position is currently in use, try again later\n";
            send(new_socket, response, strlen(response), 0);
        }
    } else if (strcmp(command, "read") == 0) {
        sem_wait(&shared_memory[index].sem);  // Lock the semaphore
        if (shared_memory[index].occupied) {
            char read_value[256];
            strcpy(read_value, shared_memory[index].data);
            sem_post(&shared_memory[index].sem);  // Unlock the semaphore
            char response[300];
            sprintf(response, "Value at index %d: %s\n", index, read_value);
            send(new_socket, response, strlen(response), 0);
        } else {
            sem_post(&shared_memory[index].sem);  // Unlock the semaphore
            char *response = "Position not occupied\n";
            send(new_socket, response, strlen(response), 0);
        }
    } else if (strcmp(command, "request") == 0) {
        if (sem_trywait(&shared_memory[index].sem) == 0) {
            char *response = "Page successfully acquired\n";
            send(new_socket, response, strlen(response), 0);
        } else {
            char *response = "Page is currently in use, try again later\n";
            send(new_socket, response, strlen(response), 0);
        }
    } else if (strcmp(command, "release") == 0) {
        sem_post(&shared_memory[index].sem);
        char *response = "Page released\n";
        send(new_socket, response, strlen(response), 0);
    } else if (strcmp(command, "show_table") == 0) {
        show_shared_memory_table();
    } else {
        char *response = "Invalid command\n";
        send(new_socket, response, strlen(response), 0);
    }

    // Mostrar la tabla después de cada comando
    show_shared_memory_table();

    close(new_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    const char *ip_address = "127.0.0.1";
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create shared memory
    int shmid = shmget(SHM_KEY, SHM_SIZE * sizeof(SharedString), IPC_CREAT | IPC_EXCL | 0600);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shared_memory = (SharedString *)shmat(shmid, NULL, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory and semaphores
    for (int i = 0; i < NUM_PAGES; i++) {
        shared_memory[i].occupied = 0;
        sem_init(&shared_memory[i].sem, 1, 1);  // Initialize the semaphore for each page
    }

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip_address, &address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    address.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on IP %s, port %d\n", ip_address, PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        pthread_t thread_id;
        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_create(&thread_id, NULL, handle_client, (void *)client_socket);
        pthread_detach(thread_id);
    }

    // Liberar recursos
    for (int i = 0; i < NUM_PAGES; i++) {
        sem_destroy(&shared_memory[i].sem);  // Destruir los semáforos
    }
    shmdt(shared_memory);
    close(server_fd);
    return 0;
}


