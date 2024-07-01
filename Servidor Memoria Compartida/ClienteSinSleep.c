#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <IP_ADDRESS>\n", argv[0]);
        return 1;
    }

    const char *ip_address = argv[1];
    int option, sub_option, index;
    char command[1024], value[256];

    // Menú del cliente
    while (1) {
        printf("Client Menu:\n");
        printf("1. Request page\n");
        printf("2. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        switch (option) {
            case 1:
                printf("Enter page index: ");
                scanf("%d", &index);
                snprintf(command, sizeof(command), "request %d", index);
                send_command(ip_address, command);

                // Submenú para escribir, leer o liberar la página
                while (1) {
                    printf("Submenu:\n");
                    printf("1. Write to page\n");
                    printf("2. Read from page\n");
                    printf("3. Release page\n");
                    printf("4. Return to main menu\n");
                    printf("Enter your choice: ");
                    scanf("%d", &sub_option);

                    switch (sub_option) {
                        case 1:
                            printf("Enter value to write: ");
                            scanf(" %[^\n]", value);
                            snprintf(command, sizeof(command), "write %d %s", index, value);
                            send_command(ip_address, command);
                            break;
                        case 2:
                            snprintf(command, sizeof(command), "read %d", index);
                            send_command(ip_address, command);
                            break;
                        case 3:
                            snprintf(command, sizeof(command), "release %d", index);
                            send_command(ip_address, command);
                            break;
                        case 4:
                            break;
                        default:
                            printf("Invalid option. Please try again.\n");
                    }

                    if (sub_option == 4) {
                        break;  // Regresar al menú principal
                    }
                }
                break;
            case 2:
                exit(0);
            default:
                printf("Invalid option. Please try again.\n");
        }
    }

    return 0;
}

