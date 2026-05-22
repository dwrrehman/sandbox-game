//
//  connection.cpp
//  server
//
//  Created by Daniel Rehman on 1909286.
//                                                       
//

#include "connection.hpp"
#include "game_state.hpp"


#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>


void check(bool condition, const char* message, bool should_exit) {
    if (not condition) {            
        perror(message);
        if (should_exit) exit(1);
    }
}

void allow_reuse_port(int client) {

	std::cout << "allowing the reuse of this port...\n";
    int one = 1;
    setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)); 
#ifdef SO_REUSEPORT
    setsockopt(client, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int));    
#endif

    std::cout << "allowed the reuse of this port.\n";


}




namespace network {

    bool server_running = true;
    
    int server = 0;
    
    
    void halt_server() {
        std::cout << "halting server!\n";
        server_running = false;        
        shutdown(server, SHUT_RDWR);
        close(server);
    }

    void listen(unsigned int port, std::function<void(int, const char*)> handler) {
        
        server = socket(AF_INET, SOCK_STREAM, 0);
        check(server, "socket", true);
        allow_reuse_port(server);
        
        sockaddr_in server_address = {0}, client_address = {0};
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        socklen_t client_length = sizeof client_address;
        
	std::cout << "binding now.\n";

        auto bind_result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
        check(bind_result >= 0, "bind", true);
        ::listen(server, 4);
        
	std::cout << "we are going to listen for clients now!\n";

        while (server_running) {
            usleep(100000);
            printf("listening...\n");
            auto connection = accept(server, (struct sockaddr*) &client_address, &client_length);
            check(connection >= 0, "accept", false);

            const char* client_name = "dummy_client"; // gethostbyaddr(&client_address.sin_addr.s_addr, client_length, AF_INET)->h_name;
            printf("connected to %s\n", client_name);
            std::thread handler_thread(handler, connection, client_name);
            handler_thread.detach();
        }
    }

    bool send(int client, const unsigned char* data, size_t length) {
        auto n = ::send(client, data, length, 0);
        if (n < 0) {
            perror("network::send(bytes)");
            abort();
            
        } else if (!n) return false;
        else return true;
    }

    bool send(int client, const nat* data) {
        auto n = ::send(client, reinterpret_cast<const unsigned char*>(data), 4, 0);
        if (n < 0) {
            perror("network::send(int)");
            abort();
            
        } else if (!n) return false;
        else return true;
    }


    bool receive(int client, unsigned char* data, size_t length) { 
        auto n = ::recv(client, data, length, 0);
        if (n < 0) {
            perror("network::receive(bytes)");
            abort();
            
        } else if (!n) return false;
        else return true;
    }

    bool receive(int client, nat* data) { 
        auto n = ::recv(client, reinterpret_cast<unsigned char*>(data), 4, 0);
        if (n < 0) {
            perror("network::receive(int)");
            abort();
            
        } else if (!n) return false;
        else return true;
    }
}
