//
//  main.c
//  server
//
//  Created by Daniel Rehman on 2008204.
//                                                       
//

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <pwd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef uint8_t byte; //   0 1 2 3 4 5 6 7 8 9 ..  255

static const size_t port = 10000;

struct client {
    int connection;
    const char* ip;
};

bool server_running = true;

int server = 0;




static char* find_file(const char* filepath, long* out_length) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        perror(filepath);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    *out_length = length;
    rewind(file);
    char* text = (char*) malloc(length);
    fread(text, 1, length, file);
    fclose(file);
    return text;
}

void* compute(void* unused) {
    
    while (server_running) {
//        printf("computing world thread...\n");
        sleep(1);
    }
    
    return NULL;
}




char transcript[256] = {0};





enum commands {ping = 5, display = 9, chat = 13, halt = 100, };

void halt_server() {
    server_running = false;
    shutdown(server, SHUT_RDWR);
    close(server);
}

void* handler(void* raw) {
    struct client parameters = *(struct client*)raw;
    int client = parameters.connection;
    const char* ip = parameters.ip;
    
    printf("connected to %s\n", ip);
    
    char buffer[256] = {0};
    
    while (server_running) {
        
        char command = 0;
        ssize_t n = read(client, &command, 1);
        if (n == 0) {
            printf("{CLIENT DISCONNECTED}\n");
            break;
        } else if (n < 0) {
            printf("server:read connection error.\n");
            break;
        }
        printf("client said command: %d\n", (int) command);
        
        
        
        
        if (command == halt) {
            
            char ack = 1;
            write(client, &ack, 1);
            
            printf("halting server...\n");
            
            halt_server();
            continue;
            
            
            
            
        } else if (command == ping) {
            
            printf("SERVER WAS PINGED!!!\n");
            
            char ack = 1;
            write(client, &ack, 1);
            
            
            
            
        } else if (command == chat) {
            
            memset(buffer, 0, sizeof buffer);
            ssize_t n = read(client, buffer, sizeof buffer);
            if (n == 0) {
                printf("{CLIENT DISCONNECTED}\n");
                break;
            } else if (n < 0) {
                printf("server:read connection error.\n");
                break;
            }
            printf("client said chat message: %s\n", buffer);
            
            strcat(transcript, buffer);
            
            char ack = 1;
            write(client, &ack, 1);
            
                        
            
        } else if (command == display) {
            
            printf("sending chat transcript...\n");
            
            write(client, transcript, sizeof transcript);
            
        } else {
            printf("error: command not recognized:  %d", (int) command);
            sleep(1);
        }

        
        
//        printf("SERVER:> ");
//        fgets(buffer, sizeof buffer, stdin);
//        n = write(client, buffer, sizeof buffer);
//        if ( n == 0) {
//            printf("server:write disconnected.\n");
//            break;
//        }
//        if (!strcmp(buffer, "quit\n")) break;
        
        
        
    }
    close(client);
    free(raw);
    return NULL;
}







/*
 void* handler(void* raw) {
     struct client parameters = *(struct client*)raw;
     int client = parameters.connection;
     const char* ip = parameters.ip;
     
     printf("connected to %s\n", ip);
     
     char buffer[1000] = {0};
     ssize_t n = 0;
     while (server_running) {
         
         memset(buffer, 0, sizeof buffer);
         n = read(client, buffer, sizeof buffer);
         if ( n == 0) {
             printf("server:read disconnected.\n");
             printf("{CLIENT DISCONNECTED}\n");
             break;
         }
         printf("client says: %s\n", buffer);
         
         if (!strcmp(buffer, "halt\n")) {
             printf("halting server...\n");
             halt_server(); continue;
         }
         
         printf("SERVER:> ");
         fgets(buffer, sizeof buffer, stdin);
         n = write(client, buffer, sizeof buffer);
         if ( n == 0) {
             printf("server:write disconnected.\n");
             break;
         }
         if (!strcmp(buffer, "quit\n")) break;
     }
     close(client);
     free(raw);
     return NULL;
 }
 
 */






void connect_to_udp_client(unsigned int port) {

    int connection = socket(AF_INET, SOCK_DGRAM, 0);
    if (!connection) { perror("socket"); return; }
    
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    socklen_t len = sizeof(cliaddr);
    bind(connection, (struct sockaddr*) &servaddr, sizeof(servaddr));
    
    printf("listening for UDP clients...\n");

    char buffer[1000] = {0};
        
    while (server_running) {

        memset(buffer, 0, sizeof buffer);
        ssize_t n = recvfrom(connection, buffer, sizeof buffer, 0, (struct sockaddr*)&cliaddr, &len);
        if (n == 0) {
            printf("UDP server:read disconnected.\n");
            printf("{UDP CLIENT DISCONNECTED}\n");
            break;
        }
        printf("UDP client says: %s\n", buffer);
        
        if (!strcmp(buffer, "halt\n")) {
            printf("halting UDP server...\n");
            halt_server(); continue;
        }

        printf("UDPSERVER:> ");
        fgets(buffer, sizeof buffer, stdin);
        if (!strcmp(buffer, "quit\n")) break;
        sendto(connection, buffer, strlen(buffer), 0, (struct sockaddr*)&cliaddr, len);
    }
    close(connection);
}

void listen_for_tcp_clients(unsigned int port) {
    
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return; }
    int one = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
#ifdef SO_REUSEPORT
    setsockopt(server, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int));
#endif
    struct sockaddr_in server_address = {0}, client_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t client_length = sizeof client_address;
    
    int bind_result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
    if (bind_result < 0) { perror("bind"); return; }
    listen(server, 5);
    
    while (server_running) {
        usleep(100000);
        printf("listening on %d...\n", port);
        int connection = accept(server, (struct sockaddr*) &client_address, &client_length);
        if (connection < 0) { perror("accept"); continue; }
        struct client* client = malloc(sizeof(struct client));
        client->connection = connection;
        client->ip = inet_ntoa(client_address.sin_addr);
        pthread_t handler_thread;
        pthread_create(&handler_thread, NULL, handler, client);
        pthread_detach(handler_thread);
    }
    printf("listen thread killed.\n");
}

int main(int argc, const char** argv) {
    if (argc <= 1) return fprintf(stderr, "usage: \n\t./server <port>\n\n");
    pthread_t thread;
    pthread_create(&thread, NULL, compute, NULL);
    
//    connect_to_udp_client(atoi(argv[1]));
    listen_for_tcp_clients(atoi(argv[1]));
    
    pthread_join(thread, NULL);
}
