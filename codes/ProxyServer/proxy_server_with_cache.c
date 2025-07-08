#include "proxy_parse.h"
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CLIENTS 10

typedef struct cache_element cache_element;

struct cache_element {
    char *data;
    int len;
    char *url;
    time_t lru_time_track;
    cache_element* next;
};

cache_element* find(char* url);
int add_cache_element(char*data, int size, char*url);
void remove_cache_element();

int port_number = 8080;
int proxy_socketId;
pthread_t tid[MAX_CLIENTS];
sem_t semaphore;
/*
sem_wait() and sem_signal()
kinda lock, but have multiple values
suppose sem = 10
client1 makes some req, sem_wait() is called
sem = 9 (updated)
client1's req closed, sem_signal() is called
sem = 10 (updated)
if sum = 0, and a client makes a request,
it will have to wait till becomes sem > 0
*/
pthread_mutex_t lock; 
/* 
suppose reponse from multiple threads access 
lru cache at the same time, that can cause
race condition. So to allow only updation 
or deletion at a time by one thread only, 
lock is used.
 */

cache_element* head;
int cache_size;

int main(int argc, char* argv[]) {
    int client_socketId, client_len;
    struct sockaddr_in server_addr, client_addr;

    sem_init(&semaphore,0, MAX_CLIENTS);
    pthread_mutex_init(&lock, NULL);

    if (argc == 2) {
        port_number = atoi(argv[1]);
    }  
    else {
        printf("Too few arguments\n");
        exit(1);
    } 

    printf("Starting proxy server at port %d\n", port_number);

    proxy_socketId = socket(AF_INET, SOCK_STREAM, 0);

    if (proxy_socketId < 0) {
        perror("Failed to create a socket\n");
        exit(1);
    }

    int reuse = 1; // true

    if (setsockopt(proxy_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        perror("setsockopt failed\n");
        exit(1);
    }
    /*
        setsockopt(
            proxy_socketId,        // the socket you're configuring
            SOL_SOCKET,            // you're setting a "socket-level" option
            SO_REUSEADDR,          // the option to enable
            (const char*)&reuse,   // pointer to the value (1 = true)
            sizeof(reuse)          // size of the value
        );
    */

    bzero((char*)&server_addr, sizeof(server_addr)); // clean o set to zero, as in C default value: garbage
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port =  htons(port_number);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(proxy_socketId, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Port is not available\n");
        exit(1);
    }

    printf("Binding on port &d\n", port_number);

    int listen_status = listen(proxy_socketId, MAX_CLIENTS);

    if (listen_status < 0) {
        perror("Error in listening\n");
        exit(1);
    }

    int i = 0;
    int connected_socketId[MAX_CLIENTS];

    while (1) {
        client_len = sizeof(client_addr);
        bzero((char *)&client_addr, client_len);
        
        client_socketId = accept(proxy_socketId, (struct sockaddr*)&client_addr, client_len);

        if (client_socketId < 0) {
            printf("Not able to connect\n");
            exit(1);
        }

        connected_socketId[i] = client_socketId;

        struct sockaddr_in* client_pt = (struct sockaddr_in*)&client_addr; 
        struct in_addr ip_addr = client_pt->sin_addr;
        char str[INET_ADDRSTRLEN];
        
    }
}