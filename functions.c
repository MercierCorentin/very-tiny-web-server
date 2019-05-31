#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
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

int handle_request(int client_socket){
    // Initialisation
    char request[BUFFER_SIZE] = { '\0' }, character = { '\0' }, request_path[BUFFER_SIZE + 16] = WEB_ROOT, *file_content = NULL;
    int i = 0, space_counter = 0, error = 0;

    // Simple information
    printf("Getting request:\n");

    // Buffer reading
    while(read(client_socket, &character, 1 ) == 1 && character != '\n' && character != '\r' && i < sizeof( request ) -1 ){;
        if(character == ' ')
            space_counter++;

        if(space_counter == 2){
            request[i] = '\0'; // terminate string
            break;
        }
        request[i++] = character;
        request[i] = '\0'; // terminate string
    }
    i = 0;

    // Just for log
    fprintf(stdout, "%s\n", request ); // print request

    // Only GET requests are accepted
    if(strncmp("GET", request, 3) != 0){
        send_reply(405, client_socket, " ");
        goto end;
    }

    // Removing "GET "
    strcpy(request, &request[4]);

    // Only the /var/www/html/ content can be accessed 
    if(strstr("..", request) != NULL){
        send_reply(403, client_socket, "");
        goto end;
    }

    // If the requested path ends with / then add index.html to the path
    if(request[strlen(request) -1 ] == '/'){
        strcat(request ,"index.html");
    }

    // Creating path to requested file
    strcat(request_path,request);
    // Send headers and content
    send_reply(200, client_socket, request_path);

    // Close the client socket and ends the function
    end:
    close(client_socket);
    return 0;
}