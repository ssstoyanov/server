#include <pthread.h>
#include <winsock2.h>
#include <stdio.h>

#define PORT 8080

pthread_mutex_t mutex;

void *clientHandler(void *param) {
    SOCKET clientSocket = (SOCKET) param;
    char recieve[1024], transmit[1024];
    int ret;
    ret = recv(clientSocket, recieve, 1024, 0);
    if (!ret || ret == SOCKET_ERROR) {
        pthread_mutex_lock(&mutex);
        printf("Error getting data\n");
        pthread_mutex_unlock(&mutex);
        return (void *) 1;
    }
    recieve[ret] = '\0';
    pthread_mutex_lock(&mutex);
    printf("%s\n", recieve);
    pthread_mutex_unlock(&mutex);
    printf_s(transmit, "%s %s %s\n", "Your data", recieve, "received");
    ret = send(clientSocket, transmit, sizeof(transmit), 0);
    if (ret == SOCKET_ERROR) {
        pthread_mutex_lock(&mutex);
        printf("Error sending data\n");
        pthread_mutex_unlock(&mutex);
        return (void *) 2;
    }
    return (void *) 0;
}

void clientAcceptor(SOCKET server) {
    int size;
    struct sockaddr_in clientaddr;
    SOCKET client;
    while (1) {

        size = sizeof(clientaddr);
        client = accept(server, (struct sockaddr *) &clientaddr, &size);
        if (client == INVALID_SOCKET) {
            printf("Error accept client\n");
            continue;
        }
        pthread_t mythread;
        int status = pthread_create(&mythread, NULL, clientHandler, (void *) client);
        pthread_detach(mythread);
        /*
         * crutch to avoid endless warnings
         */
        if(status == 3) break;
    }
    pthread_mutex_destroy(&mutex);
    printf("Server is stopped\n");
    closesocket(server);
}

int initServer() {
    SOCKET server;
    struct sockaddr_in localaddr;

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server == INVALID_SOCKET) {
        printf("Error create server\n");
        return 1;
    }
    localaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(PORT);//port number is for example, must be more than 1024
    if (bind(server, (struct sockaddr *) &localaddr, sizeof(localaddr)) == SOCKET_ERROR) {
        printf("Can't start server\n");
        return 2;
    } else {
        printf("Server is started\n");
    }
    listen(server, 50);//50 клиентов в очереди могут стоять
    pthread_mutex_init(&mutex, NULL);
    clientAcceptor(server);
    return 0;
}

int main() {
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(1, 1), &wsd) == 0) {
        printf("Connected to socket lib\n");
    } else {
        return 1;
    }
    int err = 0;
    if (err = initServer()) {
        return err;
    }

    return 0;

}

