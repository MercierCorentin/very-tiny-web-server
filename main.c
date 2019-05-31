#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "functions.h"
#define BUFFER_SIZE 8096
#define WEB_ROOT "/var/www/html"

// Colors definition
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

int main(){
    int server_socket, result, client_socket = 0, addrlen, pid = 0, status = 0;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024];

    server_socket = socket(AF_INET, SOCK_STREAM, 0); // create socket

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // configure server IP and port
    server_addr.sin_port = htons(80);

    result = bind(server_socket, (struct sockaddr*) &server_addr, sizeof( server_addr ));
    
    if(result == 0){
        fprintf(stdout, GREEN"Binding done\n"RESET);
        listen(server_socket, 5);

        while(1){

            addrlen = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &addrlen );
            if(client_socket > 0){
                // Double fork to avoid zombie processes 
                // First fork
                pid = fork();
                if(pid == 0){
                    // Process B; Child of A
                    // Second fork 
                    pid = fork();
                    if(pid == 0){
                        // Process C; Child of C
                        handle_request(client_socket);
                        // Exit C
                        return 0;
                    }else{
                        // Exit B process (kill C process when C ends)
                        return 0;
                    }
                }else{
                    // Wait for B to end and kill it 
                    waitpid(pid, &status, 0);
                }
            }
        }
    }
    else{
        fprintf(stderr, RED"Error: Binding failed\n"RESET);
    }
    close(server_socket);
    return 0;
}