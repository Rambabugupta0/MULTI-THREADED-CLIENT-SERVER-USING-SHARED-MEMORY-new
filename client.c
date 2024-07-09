///client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <semaphore.h>

#define PORT 4567
#define SHM_SIZE 1024
#define SHM_KEY 3456

int main() {
    int sockfd, n;
    struct sockaddr_in server_addr;
    char buffer[256];

    // Initialize shared memory
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    char *shared_data = shmat(shm_id, NULL, 0);
    if (shared_data == (char *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphore
    sem_t *semaphore = sem_open("/my_semaphore", 0);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    // Prepare server address structure
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("ERROR invalid address");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Enter message to server: ");
        memset(buffer, 0, 256);
        fgets(buffer, 255, stdin);
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            perror("ERROR writing to socket");
        }

        memset(buffer, 0, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0) {
            perror("ERROR reading from socket");
        }
        printf("Received from server: %s", buffer);

        // Print shared memory contents after receiving the response
        sem_wait(semaphore);
        printf("Shared memory contains: %s\n", shared_data);
        sem_post(semaphore);
    }

    close(sockfd);
    return 0;
}