#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void send_command(const char *ip_address, const char *command) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[2048] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return;
    }

    send(sock, command, strlen(command), 0);
    printf("Command sent: %s\n", command);
    int valread = read(sock, buffer, 2048);
    printf("Server response: %s\n", buffer);

    close(sock);
}

void print_menu() {
    printf("Menu:\n");
    printf("1. Write\n");
    printf("2. Read\n");
    printf("3. Show table\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <IP_ADDRESS>\n", argv[0]);
        return 1;
    }

    const char *ip_address = argv[1];
    int choice;
    char command[1024];
    int index;
    char value[256];

    while (1) {
        print_menu();
        scanf("%d", &choice);
        getchar(); // To consume the newline character left by scanf

        switch (choice) {
            case 1:
                printf("Enter index to write: ");
                scanf("%d", &index);
                getchar(); // To consume the newline character left by scanf
                printf("Enter value to write: ");
                fgets(value, sizeof(value), stdin);
                value[strcspn(value, "\n")] = 0; // Remove the newline character from fgets
                snprintf(command, sizeof(command), "write %d %s", index, value);
                send_command(ip_address, command);
                printf("Simulating long write operation...\n");
                sleep(60); // Simulate long write operation
                snprintf(command, sizeof(command), "confirm_write %d", index);
                send_command(ip_address, command);
                break;
            case 2:
                printf("Enter index to read: ");
                scanf("%d", &index);
                snprintf(command, sizeof(command), "read %d", index);
                send_command(ip_address, command);
                break;
            case 3:
                snprintf(command, sizeof(command), "show_table");
                send_command(ip_address, command);
                break;
            case 4:
                printf("Exiting...\n");
                exit(0);
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }

    return 0;
}

