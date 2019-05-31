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

void get_mime_type(char* filename, char *mime_type){
    char *extension = NULL;
    char mime_types[6][2][11] = {
        {"gif" ,"image/gif"},
        {"jpg" ,"image/jpeg"},
        {"jpeg" ,"image/jpeg"},
        {"png" ,"image/png"},
        {"htm" ,"text/html"},
        {"html" ,"text/html"}
    };
    extension = strrchr(filename,'.');
    *extension++;
    for (size_t i = 0; i < 6; i++){
        if(strcmp(extension, mime_types[i][0]) == 0){
            strcpy(mime_type, mime_types[i][1]);
        }
    } 
}

void get_formatted_time(char *string_date){
    time_t t = time(NULL);
    char days[7][4] = {
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat",
        "Sun"
    };
    char months[12][4] = {
        "Jan", 
        "Feb", 
        "Mar", 
        "Apr", 
        "May", 
        "Jun", 
        "Jul", 
        "Aug", 
        "Sep", 
        "Oct", 
        "Nov", 
        "Dec"
    };
    struct tm tm = *localtime(&t);

    sprintf(string_date, "Date: %s, %d %s %d %d:%d:%d GMT\r\n", days[tm.tm_wday], tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

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