//
//  connection.hpp
//  server
//
//  Created by Daniel Rehman on 1909286.
//                                                       
//

#ifndef connection_hpp
#define connection_hpp

#include "game_state.hpp"

#include <functional>

namespace network {

    extern bool server_running;
    
    void halt_server();

    void listen(unsigned int port, std::function<void(int, const char*)> handler);

    bool send(int client, const unsigned char* bytes, size_t length);

    bool send(int client, const nat* data);

    bool receive(int client, unsigned char* bytes, size_t length);

    bool receive(int client, nat* data); 

}

#endif /* connection_hpp */
