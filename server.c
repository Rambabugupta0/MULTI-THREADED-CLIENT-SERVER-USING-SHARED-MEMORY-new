//server  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>

#define PORT 4567
#define SHM_SIZE 1024
#define SHM_KEY 3456

// Shared resource
char *shared_data;
sem_t *semaphore;
sem_t client_semaphore;
int shared_data_updated = 0;
int client_count = 0;

void *log_shared_memory(void *arg) {
    FILE *log_file = fopen("shared_memory_log.txt", "a");
    if (!log_file) {
        perror("fopen");
        return NULL;
    }

    while (1) {
        // Check if shared memory has been updated
        sem_wait(semaphore);
        if (shared_data_updated) {
            char data[SHM_SIZE];
            strncpy(data, shared_data, SHM_SIZE);
            shared_data_updated = 0;
            sem_post(semaphore);

            time_t now = time(NULL);
            struct tm *t = localtime(&now);

            fprintf(log_file, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                    t->tm_hour, t->tm_min, t->tm_sec, data);
            fflush(log_file);
        } else {
            sem_post(semaphore);
            sleep(1); // Sleep for a while before checking again
        }
    }

    fclose(log_file);
    return NULL;
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    sem_wait(&client_semaphore);
    int client_number = ++client_count;
    sem_post(&client_semaphore);

    char buffer[256];
    char response[256];
    while (1) {
        memset(buffer, 0, 256);
        int n = read(client_socket, buffer, 255);
        if (n <= 0) {
            break;
        }

        // Update shared memory with client message
        sem_wait(semaphore);
        snprintf(shared_data, SHM_SIZE, "Client %d: %s", client_number, buffer);
        shared_data_updated = 1;
        sem_post(semaphore);

        printf("Received from Client %d: %s", client_number, buffer);

        // Server prompts for a response
        printf("Enter message to Client %d: ", client_number);
        memset(response, 0, 256);
        fgets(response, 255, stdin);

        // Update shared memory with server response
        sem_wait(semaphore);
        snprintf(shared_data, SHM_SIZE, "Server to Client %d: %s", client_number, response);
        shared_data_updated = 1;
        sem_post(semaphore);

        n = write(client_socket, response, strlen(response));
        if (n < 0) {
            perror("ERROR writing to socket");
        }
    }
    printf("Client %d is disconnected\n", client_number);
    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t thread_id, log_thread_id;

    // Initialize shared memory
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    shared_data = shmat(shm_id, NULL, 0);
    if (shared_data == (char *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphores
    semaphore = sem_open("/my_semaphore", O_CREAT, 0666, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    sem_init(&client_semaphore, 0, 1);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("ERROR opening socket");
        sem_close(semaphore);
        sem_unlink("/my_semaphore");
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Prepare server address structure
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR on binding");
        close(server_socket);
        sem_close(semaphore);
        sem_unlink("/my_semaphore");
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listen(server_socket, 5);
    printf("Server listening on port %d\n", PORT);

    // Start logging thread
    if (pthread_create(&log_thread_id, NULL, log_shared_memory, NULL) < 0) {
        perror("could not create logging thread");
        close(server_socket);
        sem_close(semaphore);
        sem_unlink("/my_semaphore");
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections
    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("ERROR on accept");
            continue;
        }
        printf("Client %d is connected\n", client_count + 1);
        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_sock) < 0) {
            perror("could not create thread");
            free(new_sock);
            close(client_socket);
        }
    }

    close(server_socket);
    shmctl(shm_id, IPC_RMID, NULL);
    sem_close(semaphore);
    sem_unlink("/my_semaphore");
    sem_destroy(&client_semaphore);
    return 0;
}


