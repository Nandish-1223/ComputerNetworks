#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 5063
#define SIZE 1024

void ensure_arp_file(void) {
    FILE *fp = fopen("arp.txt", "r");
    if (fp == NULL) {
        fp = fopen("arp.txt", "w");
        if (fp) {
            fprintf(fp, "192.168.1.1 AA:BB:CC:DD:EE:01\n");
            fprintf(fp, "192.168.1.2 AA:BB:CC:DD:EE:02\n");
            fprintf(fp, "127.0.0.1 00:00:00:00:00:00\n");
            fprintf(fp, "192.168.1.10 AA:BB:CC:DD:EE:0A\n");
            fclose(fp);
        }
    } else {
        fclose(fp);
    }
}

void lookup_mac(const char *ip, char *mac, size_t mac_size) {
    FILE *fp = fopen("arp.txt", "r");
    if (!fp) {
        strncpy(mac, "FILE ERROR", mac_size);
        mac[mac_size - 1] = '\0';
        return;
    }

    char file_ip[64], file_mac[64];
    while (fscanf(fp, "%63s %63s", file_ip, file_mac) == 2) {
        if (strcmp(file_ip, ip) == 0) {
            strncpy(mac, file_mac, mac_size);
            mac[mac_size - 1] = '\0';
            fclose(fp);
            return;
        }
    }

    fclose(fp);
    strncpy(mac, "NOT FOUND", mac_size);
    mac[mac_size - 1] = '\0';
}

void chat_service(int s, struct sockaddr_in *client, socklen_t len) {
    char b[SIZE];
    int n, running = 1;
    fd_set readfds;
    struct sockaddr_in from = {0};
    socklen_t from_len = sizeof(from);

    sendto(s, "CHAT_START", 10, 0, (struct sockaddr *)client, len);

    printf("\n[Chat started with client]\nServer> ");
    fflush(stdout);

    while (running) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(s, &readfds);

        if (select(s + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(s, &readfds)) {
            from_len = sizeof(from);
            n = recvfrom(s, b, SIZE - 1, 0, (struct sockaddr *)&from, &from_len);
            if (n <= 0)
                continue;

            b[n] = '\0';

            if (from.sin_addr.s_addr != client->sin_addr.s_addr ||
                from.sin_port != client->sin_port)
                continue;

            printf("\r\033[K");

            if (strcmp(b, "EXIT") == 0) {
                printf("Client left the chat.\n");
                running = 0;
            } else {
                printf("Client: %s\nServer> ", b);
                fflush(stdout);
            }
        }

        if (running && FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(b, sizeof(b), stdin))
                strcpy(b, "EXIT");
            else
                b[strcspn(b, "\n")] = '\0';

            if (b[0] == '\0') {
                printf("Server> ");
                fflush(stdout);
                continue;
            }

            if (sendto(s, b, strlen(b), 0, (struct sockaddr *)client, len) < 0) {
                perror("sendto");
                break;
            }

            if (strcmp(b, "EXIT") == 0) {
                printf("You left the chat.\n");
                running = 0;
            } else {
                printf("Server> ");
                fflush(stdout);
            }
        }
    }
}

void arp_service(int s, struct sockaddr_in *client, socklen_t len) {
    char ip[SIZE], mac[64];
    int n;
    struct sockaddr_in from = {0};
    socklen_t from_len = sizeof(from);

    sendto(s, "SEND_IP", 7, 0, (struct sockaddr *)client, len);

    n = recvfrom(s, ip, SIZE - 1, 0, (struct sockaddr *)&from, &from_len);
    if (n <= 0)
        return;

    ip[n] = '\0';

    if (from.sin_addr.s_addr != client->sin_addr.s_addr ||
        from.sin_port != client->sin_port)
        return;

    lookup_mac(ip, mac, sizeof(mac));
    sendto(s, mac, strlen(mac), 0, (struct sockaddr *)client, len);

    printf("[ARP Request] IP: %s -> MAC: %s\n", ip, mac);
}

int main(void) {
    int s, n;
    char b[SIZE];
    struct sockaddr_in addr = {0}, client = {0};
    socklen_t len = sizeof(client);

    ensure_arp_file();

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s);
        return 1;
    }

    printf("UDP Iterative Server waiting on port %d...\n", PORT);
    printf("ARP table loaded from arp.txt\n");
    printf("Waiting for clients...\n\n");

    while (1) {
        memset(&client, 0, sizeof(client));
        len = sizeof(client);

        n = recvfrom(s, b, SIZE - 1, 0, (struct sockaddr *)&client, &len);
        if (n <= 0) {
            perror("recvfrom");
            continue;
        }

        b[n] = '\0';

        if (strcmp(b, "1") == 0) {
            chat_service(s, &client, len);
            printf("\n[Waiting for next client...]\n\n");
        } else if (strcmp(b, "2") == 0) {
            arp_service(s, &client, len);
        } else if (strcmp(b, "3") == 0) {
            sendto(s, "GOODBYE", 7, 0, (struct sockaddr *)&client, len);
        } else {
            sendto(s, "INVALID", 7, 0, (struct sockaddr *)&client, len);
        }
    }

    close(s);
    return 0;
}
