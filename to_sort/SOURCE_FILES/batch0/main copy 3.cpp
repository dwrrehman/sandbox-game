//
//  main.cpp
//  client side for the u3 block game.
//
//  Created by Daniel Rehman on 1906075.
//                                                       
//

#include <thread>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pwd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <SDL2/SDL_ttf.h>
#include "glm/gtx/transform.hpp"
#include "glm/glm.hpp"
#include "GL/glew.h"

static const char* vertex_shader_file = R"HERE(
    #version 120

    varying vec4 color0;

    attribute vec3 position;
    attribute vec4 color;

    uniform mat4 transform;

    void main() {
        gl_Position = transform * vec4(position, 1.0);
        color0 = color;
    }
)HERE";

static const char* fragment_shader_file = R"HERE(
    #version 120

    varying vec4 color0;

    void main() {
        gl_FragColor = color0;
    }
)HERE";

static const char* window_name = "u3";
static unsigned window_width = 2000;
static unsigned window_height = 1500;
static const float fov = 70.0f;
static const float z_near = 0.01f;
static const float z_far = 1000.0f;
static const float sensitivity = 0.0040f;
static const float camera_acceleration = 0.020f;
static const float camera_drag = 0.965;
static const size_t frame_delay_us = 4000; // 16000 for 60fps.
static const float block_spacing = 0.01;
static const size_t s = 30;
static const size_t cell_count = s * s * s;

static uint8_t* space = nullptr;

static size_t gamemode = 1;
static size_t debugmode = 0;
static GLuint rendermode = GL_FILL;
static bool is_fullscreen = false;
static bool move_camera = true;
static bool rotation_mode = 0;
static bool escape = false;
static bool tab = false;

struct camera {
    glm::vec3 position = glm::vec3(0,0,0);
    glm::vec3 velocity = glm::vec3(0,0,0);
    glm::vec3 forward = glm::vec3(0,0,1);
    glm::vec3 upward = glm::vec3(0,1,0);
    glm::mat4 perspective = glm::mat4();
};

struct transform_data {
    glm::vec3 position = glm::vec3();
    glm::vec3 rotation = glm::vec3();
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
};

static glm::mat4 get_view_projection(camera& camera) {
    return camera.perspective * glm::lookAt(camera.position, camera.position + camera.forward, camera.upward);
}

static void rotate_camera(camera& camera, glm::vec2 delta, bool should_change_upward) {
    if (should_change_upward) {
        camera.upward = glm::mat3(glm::rotate(-delta.y * sensitivity, camera.forward)) * camera.upward;
        glm::vec3 torotate_around = glm::cross(camera.upward, camera.forward);
        camera.upward = glm::mat3(glm::rotate(-delta.x * sensitivity, torotate_around)) * camera.upward;
    } else {
        camera.forward = glm::mat3(glm::rotate(-delta.x * sensitivity, camera.upward)) * camera.forward;
        glm::vec3 torotate_around = glm::cross(camera.forward, camera.upward);
        camera.forward = glm::mat3(glm::rotate(-delta.y * sensitivity, torotate_around)) * camera.forward;
    }
}

static glm::mat4 getmodel(struct transform_data& data) {
    glm::mat4 position_matrix = glm::translate(data.position);
    
    glm::mat4 rotation_x_matrix = glm::rotate(data.rotation.x, glm::vec3(1, 0, 0));
    glm::mat4 rotation_y_matrix = glm::rotate(data.rotation.y, glm::vec3(0, 1, 0));
    glm::mat4 rotation_z_matrix = glm::rotate(data.rotation.z, glm::vec3(0, 0, 1));

    glm::mat4 scale_matrix = glm::scale(data.scale);
    glm::mat4 rotation_matrix = rotation_z_matrix * rotation_y_matrix * rotation_x_matrix;

    return position_matrix * rotation_matrix * scale_matrix;
}

static void window_changed(camera& camera, SDL_Window *window) {
    int w = 0, h = 0;
    SDL_GetWindowSize(window, &w, &h);
    window_width = w;
    window_height = h;
    camera.perspective = glm::perspective(fov, (float) window_width / (float) window_height, z_near, z_far);
}

static void enable_mouse_rotation() {
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    move_camera = true;
}

static void disable_mouse_rotation() {
    SDL_ShowCursor(SDL_ENABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    move_camera = false;
}

static void handle_input(SDL_Window* window, camera& camera) {
    
    const Uint8* key = SDL_GetKeyboardState(nullptr);
    
    rotation_mode = !!key[SDL_SCANCODE_C];
    if (key[SDL_SCANCODE_SPACE]) camera.velocity += camera_acceleration * camera.upward;
    if (key[SDL_SCANCODE_LSHIFT]) camera.velocity -= camera_acceleration * camera.upward;
    if (key[SDL_SCANCODE_W]) camera.velocity += camera_acceleration * camera.forward;
    if (key[SDL_SCANCODE_S]) camera.velocity -= camera_acceleration * camera.forward;
    if (key[SDL_SCANCODE_A]) camera.velocity -= glm::normalize(glm::cross(camera.forward, camera.upward)) * camera_acceleration;
    if (key[SDL_SCANCODE_D]) camera.velocity += glm::normalize(glm::cross(camera.forward, camera.upward)) * camera_acceleration;
    
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        
        if (e.window.type == SDL_WINDOWEVENT_RESIZED) window_changed(camera, window);
        const Uint8* key = SDL_GetKeyboardState(nullptr);
        if (e.type == SDL_QUIT) gamemode = 0;
        //        if (e.type == SDL_MOUSEBUTTONDOWN) {}
        
        if (e.type == SDL_MOUSEMOTION) {
            if (move_camera) rotate_camera(camera, {e.motion.xrel, e.motion.yrel}, rotation_mode);
        }
        if (e.type == SDL_KEYDOWN) {
            if (key[SDL_SCANCODE_GRAVE]) {
                if (escape) {
                    disable_mouse_rotation();
                } else if (tab) {
                    gamemode = 0;
                }
            } else if (key[SDL_SCANCODE_1]) {
                if (escape) {
                    rendermode = GL_FILL;
                    enable_mouse_rotation();
                } else if (tab) {
                    gamemode = 1;
                }
            } else if (key[SDL_SCANCODE_2]) {
                if (escape) {
                    rendermode = GL_LINES;
                } else if (tab) {
                    gamemode = 2;
                }
            } else if (key[SDL_SCANCODE_3]) {
                if (escape) {
                    debugmode = !debugmode;
                } else if (tab) {
                    gamemode = 3;
                }
            } else if (key[SDL_SCANCODE_4]) {
                if (escape) {
                    debugmode = 2;
                } else if (tab) {
                    gamemode = 4;
                }
            }
            escape = false;
            tab = false;
            if (key[SDL_SCANCODE_RETURN]) {
                is_fullscreen = !is_fullscreen;
                SDL_SetWindowFullscreen(window, is_fullscreen);
            }
            if (key[SDL_SCANCODE_TAB]) tab = true;
            if (key[SDL_SCANCODE_ESCAPE]) escape = true;
            if (key[SDL_SCANCODE_X]) camera.upward = glm::vec3(0,1,0);
        }
    }
}

static void compute_vertex_mesh(std::vector<glm::vec3>& positions,
                                std::vector<glm::vec3>& edge_positions,
                                std::vector<glm::vec4>& colors,
                                std::vector<glm::vec4>& edge_colors,
                                std::vector<GLuint>& indicies,
                                std::vector<GLuint>& edge_indicies,
                                const glm::vec4 block_colors[256]
                                ) {
    unsigned block_count = 0;
    for (size_t i = 0; i < s; i++) {
        for (size_t j = 0; j < s; j++) {
            for (size_t k = 0; k < s; k++) {
                const uint8_t value = space[k + j * s + i * s * s];
                if (value) {
                    const float e = block_spacing;
                    positions.insert(positions.end(), {
                        glm::vec3(i+e,   j+e,   k+e),
                        glm::vec3(i+e,   j+e,   k+1-e),
                        glm::vec3(i+e,   j+1-e, k+e),
                        glm::vec3(i+e,   j+1-e, k+1-e),
                        glm::vec3(i+1-e, j+e,   k+e),
                        glm::vec3(i+1-e, j+e,   k+1-e),
                        glm::vec3(i+1-e, j+1-e, k+e),
                        glm::vec3(i+1-e, j+1-e, k+1-e),
                    });
                                        
                    edge_positions.insert(edge_positions.end(), {
                        glm::vec3(i,   j,   k),
                        glm::vec3(i,   j,   k+1),
                        glm::vec3(i,   j+1, k),
                        glm::vec3(i,   j+1, k+1),
                        glm::vec3(i+1, j,   k),
                        glm::vec3(i+1, j,   k+1),
                        glm::vec3(i+1, j+1, k),
                        glm::vec3(i+1, j+1, k+1),
                    });
                                        
                    const auto color = block_colors[value];
                    colors.insert(colors.end(), {
                        color, color, color, color,
                        color, color, color, color
                    });
                    
                    const auto black = glm::vec4(0.0, 0.0, 0.0, 1.0);
                    edge_colors.insert(edge_colors.end(), {
                        black, black, black, black,
                        black, black, black, black
                    });
                    
                    
                    auto v = block_count * 8;
                    indicies.insert(indicies.end(), {
                        v+0, v+1, v+2,
                        v+1, v+3, v+2,
                        
                        v+4, v+6, v+5,
                        v+5, v+6, v+7,
                                                                        
                        v+7, v+1, v+5,
                        v+1, v+7, v+3,
                        
                        v+2, v+7, v+6,
                        v+2, v+3, v+7,
                                                                                                
                        v+0, v+2, v+6,
                        v+0, v+6, v+4,
                        
                        v+0, v+5, v+1,
                        v+0, v+4, v+5,
                    });
                                                                                        
                    edge_indicies.insert(edge_indicies.end(), {
                        v+0,v+1,   v+0,v+2,
                        v+1,v+3,   v+2,v+3,
                        
                        v+4,v+5,   v+4,v+6,
                        v+5,v+7,   v+6,v+7,
                        
                        v+0,v+4,   v+1,v+5,
                        v+2,v+6,   v+3,v+7,
                    });
                    block_count++;
                }
            }
        }
    }
}




enum commands {ping = 5, display = 9, chat = 13, halt = 100, };



void TCP_connect_to_server(const char* playername, const char* ip, unsigned int port) {
    int connection = socket(AF_INET, SOCK_STREAM, 0);
    if (connection < 0) { perror("socket"); exit(1); }
    
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
    
    printf("connecting to %s:%d ...\n", ip, port);
    int result = connect(connection, (struct sockaddr*) &servaddr, sizeof servaddr);
    if (result < 0) { perror("connect"); exit(1); }
    
    char buffer[256] = {0};
    
    while (1) {
        
        printf("CLIENT:> ");
        fgets(buffer, sizeof buffer, stdin);
        
        if (!strcmp(buffer, "quit\n")) {
            
            break;
                        
            
        } else if (!strcmp(buffer, "ping\n")) {
            
            char command = ping;
            write(connection, &command, 1);
            
            char response = 0;
            ssize_t n = read(connection, &response, sizeof response);
            if (n == 0) {
                printf("{SERVER DISCONNECTED}\n");
                break;
            } else if (n < 0) {
                printf("client:read error!\n");
                break;
            }
            
            if (response == 1) {
                printf("ping acknowledged.\n");
            } else {
                printf("ERROR: ping not acknowledged.\n");
            }
            
            
        
            
        } else if (!strcmp(buffer, "display\n")) {
                   
            char command = display;
            write(connection, &command, 1);
            
            memset(buffer, 0, sizeof buffer);
            ssize_t n = read(connection, buffer, sizeof buffer);
            if (n == 0) {
                printf("{SERVER DISCONNECTED}\n");
                break;
            } else if (n < 0) {
                printf("client:read error!\n");
                break;
            }
            
            printf("server: \n%s\n", buffer);
            
            
            
            
        } else if (!strcmp(buffer, "chat\n")) {
                   
            char command = chat;
            write(connection, &command, 1);
            
            printf("message: ");
            fgets(buffer, sizeof buffer, stdin);
            write(connection, buffer, sizeof buffer);
            
            char response = 0;
            ssize_t n = read(connection, &response, sizeof response);
            if (n == 0) {
                printf("{SERVER DISCONNECTED}\n");
                break;
            } else if (n < 0) {
                printf("client:read error!\n");
                break;
            }
            
            if (response == 1) {
                printf("chat acknowledged.\n");
            } else {
                printf("ERROR: chat not acknowledged.\n");
            }
                    
            
            
            
        } else if (!strcmp(buffer, "halt\n")) {
                   
            char command = halt;
            write(connection, &command, 1);
            
            char response = 0;
            ssize_t n = read(connection, &response, sizeof response);
            if (n == 0) {
                printf("{SERVER DISCONNECTED}\n");
                break;
            } else if (n < 0) {
                printf("client:read error!\n");
                break;
            }
            
            if (response == 1) {
                printf("halt acknowledged.\n");
            } else {
                printf("ERROR: halt not acknowledged.\n");
            }
                     
            break;
            
            
            
        } else {
            printf("error: unknown command. "
                   "can either be:\n\t"
                   "halt\n\t"
                   "ping\n\t"
                   "display\n\t"
                   "chat\n\t"
                   "\n");
            continue;
        }
        
    }
    
    close(connection);
}


/*
 while (1) {
       printf("CLIENT:> ");
       fgets(buffer, sizeof buffer, stdin);
       if (!strcmp(buffer, "quit\n")) break;
       write(connection, buffer, sizeof buffer);
       
       memset(buffer, 0, sizeof buffer);
       ssize_t n = read(connection, buffer, sizeof buffer);
       if (n == 0) {
           printf("client:read disconnected.\n");
           printf("{SERVER DISCONNECTED}\n");
           break;
       } else if (n < 0) {
           printf("client:read error!\n");
           printf("{SERVER CONNECTION ERROR}\n");
           break;
       }
       
       printf("server says: %s\n", buffer);
   }
 */











void UDP_connect_to_server(const char* playername, const char* ip, unsigned int port) {
    int connection = socket(AF_INET, SOCK_DGRAM, 0);
    if (connection < 0) { perror("socket"); exit(1); }
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    socklen_t len = sizeof(servaddr);
    
    printf("connecting to UDP server...\n");
        
    char buffer[1024] = {0};

    while (1) {
        printf("UDP CLIENT:> ");
        fgets(buffer, sizeof buffer, stdin);
        if (!strcmp(buffer, "quit\n")) break;
        sendto(connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, len);

        memset(buffer, 0, sizeof buffer);
        ssize_t n = recvfrom(connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, &len);
        if (n == 0) {
            printf("UDP client:read disconnected.\n");
            printf("{UDP SERVER DISCONNECTED}\n");
            break;
        }
        printf("UDP server says: %s\n", buffer);
    }
    close(connection);
}



void render() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0 or TTF_Init() < 0) fprintf(stderr, "error: could not initialize SDL2: %s\n", SDL_GetError());
       
       SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) ;
       SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
       SDL_ShowCursor(SDL_DISABLE);
       SDL_SetRelativeMouseMode(SDL_TRUE);
       
       SDL_Window* window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
       if (!window) fprintf(stderr,"error: could not create window: %s\n", SDL_GetError());
       SDL_WarpMouseInWindow(window, window_width / 2.0, window_height / 2.0);
       SDL_GLContext context = SDL_GL_CreateContext(window);
       glewExperimental = GL_TRUE;
       GLenum status = glewInit();
       if (status != GLEW_OK) fprintf(stderr, "could not initialize glew: %s\n", glewGetString(status));
       
       glEnable(GL_DEPTH_TEST);
       glFrontFace(GL_CCW);
       glCullFace(GL_BACK);
       glEnable(GL_CULL_FACE);
       
       /// ------------- temp ---------------------
       srand((unsigned)time(0));
       space = (uint8_t*) malloc(cell_count);
       if (not space) fprintf(stderr, "error: not enough memory for s = %lu.\n", s);
       else memset(space, 0, cell_count);
       for (size_t i = 0; i < cell_count; i++) {
           space[i] = ((rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2)) * (rand() % 6);
       } /// ------------------------------------
       
       GLchar error[1024]; GLint success = 0, length = 0;
       GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
       if (not vertex_shader) fprintf(stderr, "error: could not create vertex shader!\n");
       length = (GLint) strlen(vertex_shader_file);
       glShaderSource(vertex_shader, 1, &vertex_shader_file, &length);
       glCompileShader(vertex_shader);
       glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
       glGetShaderInfoLog(vertex_shader, sizeof error , nullptr, error);
       if (!success) fprintf(stderr, "error: vertex shader compilation: %s\n", error);
       
       GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
       if (not fragment_shader) fprintf(stderr, "error: could not create fragment shader!\n");
       length = (GLint) strlen(fragment_shader_file);
       glShaderSource(fragment_shader, 1, &fragment_shader_file, &length);
       glCompileShader(fragment_shader);
       glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
       glGetShaderInfoLog(fragment_shader, sizeof error , nullptr, error);
       if (!success) fprintf(stderr, "error: vertex shader compilation: %s\n", error);

       GLuint program = glCreateProgram();
       glAttachShader(program, vertex_shader);
       glAttachShader(program, fragment_shader);
       glBindAttribLocation(program, 0, "position");
       glBindAttribLocation(program, 1, "color");
       glLinkProgram(program);
       glGetProgramiv(program, GL_LINK_STATUS, &success);
       glGetProgramInfoLog(program, sizeof error, nullptr, error);
       if (!success) fprintf(stderr, "program linking failure: %s\n", error);
       glValidateProgram(program);
       glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
       glGetProgramInfoLog(program, sizeof error, nullptr, error);
       if (!success) fprintf(stderr, "error: program validation failure: %s\n", error);
           
       GLuint uniform = glGetUniformLocation(program, "transform");
       struct transform_data transform = {};
       struct camera camera = {};
       camera.perspective = glm::perspective(fov, (float) window_width / (float) window_height, z_near, z_far);
       
       while (gamemode) {
           
           handle_input(window, camera);
           camera.position += camera.velocity;
           camera.velocity *= camera_drag;
           
           glUseProgram(program);
           glLineWidth(10);
           glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
           glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
           
           glm::mat4 model = get_view_projection(camera) * getmodel(transform);
           glUniformMatrix4fv(uniform, 1, GL_FALSE, &model[0][0]);
           
           std::vector<glm::vec3> positions = {}, edge_positions = {};
           std::vector<glm::vec4> colors = {}, edge_colors = {};
           std::vector<GLuint> indicies = {}, edge_indicies = {};
           
           static const glm::vec4 block_colors[256] = { // temp.
               {0.0, 0.0, 0.0, 1.0},
               {0.1, 0.1, 0.1, 1.0},
               {0.2, 0.2, 0.2, 1.0},
               {0.3, 0.3, 0.3, 1.0},
               {0.4, 0.4, 0.4, 1.0},
               {0.5, 0.5, 0.5, 1.0},
           };
           
           compute_vertex_mesh(positions, edge_positions, colors, edge_colors, indicies, edge_indicies, block_colors);
       
           GLuint vertex_array;
           glGenVertexArrays(1, &vertex_array);
           glBindVertexArray(vertex_array);
           
           GLuint vertex_array_buffers[3];
           glGenBuffers(3, vertex_array_buffers);

           glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffers[0]);
           glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof positions[0], positions.data(), GL_STATIC_DRAW);
           glEnableVertexAttribArray(0);
           glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

           glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffers[1]);
           glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof colors[0], colors.data(), GL_STATIC_DRAW);
           glEnableVertexAttribArray(1);
           glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

           glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_array_buffers[2]);
           glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof indicies[0], indicies.data(), GL_STATIC_DRAW);

           glBindVertexArray(vertex_array);
           glPolygonMode(GL_FRONT_AND_BACK, rendermode);
           glDrawElements(GL_TRIANGLES, (unsigned) indicies.size(), GL_UNSIGNED_INT, 0);

           glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffers[0]);
           glBufferData(GL_ARRAY_BUFFER, edge_positions.size() * sizeof edge_positions[0], edge_positions.data(), GL_STATIC_DRAW);
           glEnableVertexAttribArray(0);
           glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

           glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffers[1]);
           glBufferData(GL_ARRAY_BUFFER, edge_colors.size() * sizeof edge_colors[0], edge_colors.data(), GL_STATIC_DRAW);
           glEnableVertexAttribArray(1);
           glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

           glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_array_buffers[2]);
           glBufferData(GL_ELEMENT_ARRAY_BUFFER, edge_indicies.size() * sizeof edge_indicies[0], edge_indicies.data(), GL_STATIC_DRAW);
           
           glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
           glDrawElements(GL_LINES, (unsigned) edge_indicies.size(), GL_UNSIGNED_INT, 0);

           SDL_GL_SwapWindow(window);
           glDeleteVertexArrays(1, &vertex_array);
           glDeleteBuffers(3, vertex_array_buffers);
           usleep(frame_delay_us);
       }
       
       glDetachShader(program, vertex_shader);
       glDeleteShader(vertex_shader);
       glDetachShader(program, fragment_shader);
       glDeleteShader(fragment_shader);
       glDeleteProgram(program);
       SDL_CloseAudio();
       SDL_GL_DeleteContext(context);
       SDL_DestroyWindow(window);
       SDL_Quit();
       TTF_Quit();
}

int main(int argc, const char** argv) {
    
    if (argc <= 3) return fprintf(stderr, "usage:\n\t ./u <playername> <ipaddress> <port>\n\n");

    TCP_connect_to_server(argv[1], argv[2], atoi(argv[3]));
    
//    render();
}
