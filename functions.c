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

int send_reply(int status, int client_socket, char request_path[BUFFER_SIZE + 16]){
    char headers[1021] = { '\0' }, mime_type[15], string_date[38], string_file_size[11], *file_content = NULL, path[BUFFER_SIZE];
    long int file_size = 0;
    FILE *file_descriptor;

    strcpy(path, request_path);

    // Set status header and open right file depending on the status code
    switch (status){
    case 200:
        strcat(headers, "HTTP/1.1 200 OK\r\n");
        file_descriptor = fopen(request_path, "rb");
        if(file_descriptor == NULL ){
            send_reply(404, client_socket, "");
            return 0;
        }
        break;
    case 403:
        strcat(headers, "HTTP/1.1 403 Forbidden\r\n");
        strncpy(path, "/home/oem/c_projects/tcpip/resources/403.html", BUFFER_SIZE);
        file_descriptor = fopen(path, "r");
        break;
    case 404:
        strcat(headers, "HTTP/1.1 404 Not Found\r\n");
        strncpy(path, "/home/oem/c_projects/tcpip/resources/404.html", BUFFER_SIZE);
        fprintf(stderr, "%s\n", path);

        file_descriptor = fopen(path, "r");
        break;
    case 405:
        strcat(headers, "HTTP/1.1 405 Method Not Allowed\r\n");
        strcpy(path, "/home/oem/c_projects/tcpip/resources/405.html");
        file_descriptor = fopen(path, "r");
        break;
    default:
        break;
    }
    
    // Get the size of the file
    fseek(file_descriptor, 0L, SEEK_END);
    file_size = ftell(file_descriptor);
    fseek(file_descriptor, 0L, SEEK_SET);

    // Dynamic memory allocation then transfering the content of the file in a variable
    file_content = (char *) malloc((file_size + 1) * sizeof(char));
    fread(file_content, file_size, 1, file_descriptor);

    // Int to string conversion (For Content-Length header)
    sprintf(string_file_size, "%ld", file_size);
    // Get mime type of file
    get_mime_type(path, mime_type);
    //  Get the actual time in the HTTP/1.1 RFC's format
    get_formatted_time(string_date);

    // adding headers
    strcat(headers, string_date);
    strcat(headers, "Content-Type: ");
    strcat(headers, mime_type);
    strcat(headers, "; charset=utf-8\r\n");
    strcat(headers, "Content-Length: ");
    strcat(headers, string_file_size);
    strcat(headers, "\r\n\r\n");
    // Send headers
    write(client_socket, headers, strlen(headers));

    // Send file content
    if(send(client_socket, file_content, file_size + 1, 0) < file_size){
        fprintf(stdout, RED"Help\n"RESET);
    }else{
        fprintf(stdout, GREEN"OK\n"RESET);
    }

    // Free memory, close file and exit function
    free(file_content);
    fclose(file_descriptor);
    return 0;
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