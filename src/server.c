/**
 * webserver.c -- A webserver written in C
 * 
 * Test with curl (if you don't have it, install it):
 * 
 *    curl -D - http://localhost:3490/
 *    curl -D - http://localhost:3490/d20
 *    curl -D - http://localhost:3490/date
 * 
 * You can also test the above URLs in your browser! They should work!
 * 
 * Posting Data:
 * 
 *    curl -D - -X POST -H 'Content-Type: text/plain' -d 'Hello, sample data!' http://localhost:3490/save
 * 
 * (Posting data is harder to test from a browser.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include "net.h"
#include "file.h"
#include "mime.h"
#include "cache.h"

#define PORT "3490" // the port users will be connecting to

#define SERVER_FILES "./serverfiles"
#define SERVER_ROOT "./serverroot"

#define DEBUG 0

/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 * 
 * Return the value from the send() function.
 */
int send_response(int fd, char *header, char *content_type, void *body, int content_length)
{
    const int max_response_size = 65536;
    char response[max_response_size];
    // Build HTTP response and store it in response

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // TODO: Use memcpy to append the body to response so you can see jpg files
    int response_length = sprintf(response, "%s\nDate:Mon Dec 3 13:05:11 PST 2018\nContent-Length: %d\nConnection: close\nContent-Type:%s\n\n%s\n", header, content_length, content_type, body);

    // Send it all!
    int rv = send(fd, response, response_length, 0);
    if (rv < 0)
    {
        perror("send");
    }

    return rv;
}

/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd)
{
    int max_num = 20;
    int min_num = 1;
    char header[] = "HTTP/1.1 200 OK";
    char content_type[] = "text/html";
    char body[256];
    time_t t;
    int i, n;
    // mime_type_get() use to find the file extension
    n = 1;

    srand((unsigned)time(&t));

    // Generate a random number between 1 and 20 inclusive
    int randomNum = rand() % 20;
    // int randomNum = 20;
    sprintf(body, "%d", randomNum);
    int content_length = strlen(body) + 1;
    // #if DEBUG
    // #endif
    // printf("Body: %s\n", body);

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // Use send_response() to send it back as text/plain data
    send_response(fd, header, content_type, body, content_length);
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    return;
}

/**
 * Send a 404 response
 */
void resp_404(int fd)
{
    char filepath[4096];
    struct file_data *filedata;
    char *mime_type;

    // Fetch the 404.html file
    snprintf(filepath, sizeof filepath, "%s/404.html", SERVER_FILES);
    filedata = file_load(filepath);

    if (filedata == NULL)
    {
        // TODO: make this non-fatal
        fprintf(stderr, "cannot find system 404 file\n");
        exit(3);
    }

    mime_type = mime_type_get(filepath);

    send_response(fd, "HTTP/1.1 404 NOT FOUND", mime_type, filedata->data, filedata->size);

    file_free(filedata);
}

/**
 * Read and return a file from disk or cache
 */
void get_file(int fd, struct cache *cache, char *request_path)
{
    // char request_path[4096];
    char filepath[4096];
    struct file_data *filedata;
    char *mime_type;
    char header[] = "HTTP/1.1 200 OK";
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    snprintf(filepath, sizeof(filepath), "%s%s", SERVER_ROOT, request_path);

    struct cache_entry *get_Cache = cache_get(cache, request_path);
    if (get_Cache == NULL)
    {
        filedata = file_load(filepath);
        if (filedata == NULL)
        {
            resp_404(fd);
            return;
        }
        else
        {

            mime_type = mime_type_get(request_path);

            send_response(fd, header, mime_type, filedata->data, filedata->size);
            cache_put(cache, filepath, mime_type_get(filepath), filedata, sizeof(filedata));
            file_free(filedata);
        }
    }
    else
    {
        // mime_type = mime_type_get(request_path);
        send_response(fd, header, get_Cache->content_type, get_Cache->content, get_Cache->content_length);

    }

#if DEBUG
    printf("---requestpath:\t%s\n", request_path);
    printf("---filepath:\t%s\n", filepath);
    printf("---fullpath:\t%s%s\n", SERVER_ROOT, request_path);
    printf("---filedata NULL?\t%d\n", filedata == NULL);
#endif
}

/**
 * Search for the end of the HTTP header
 * 
 * "Newlines" in HTTP can be \r\n (carriage return followed by newline) or \n
 * (newline) or \r (carriage return).
 */
char *find_start_of_body(char *header)
{
    ///////////////////
    // IMPLEMENT ME! // (Stretch)
    ///////////////////
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0)
    {
        perror("recv");
        return;
    }

    char operation[256];
    char endpoint[256];
    char protocol[256];

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    printf("Request: %s\n", request);
    // Read the three components of the first request line
    sscanf(request, "%s %s %s", &operation, &endpoint, &protocol);

    // If GET, handle the get endpoints
    if (strcmp(operation, "GET") == 0)
    {
        // printf("I am a GET request\n");
        if (strcmp(endpoint, "/d20") == 0)
        {
            // puts("testing d20");
            get_d20(fd);
            return;
        }
        else if (strcmp(endpoint, "/") == 0)
        {
            puts("server get request");
            resp_404(fd);
        }
        else
        {
            get_file(fd, cache, endpoint);
        }
        // else if(strcmp(endpoint, "/index.html") == 0){

        // }
    }
    else if (strcmp(operation, "POST") == 0)
    {
        printf("I am a POST request\n");
    }
    else
    {
        resp_404(fd);
    }
    // (Stretch) If POST, handle the post request
}

/**
 * Main
 */
int main(void)
{
    // put random number generator in the main so that the whole program can call it

    int newfd;                          // listen on sock_fd, new connection on newfd
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];

    struct cache *cache = cache_create(10, 0);

    // Get a listening socket
    int listenfd = get_listener_socket(PORT);

    if (listenfd < 0)
    {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    printf("webserver: waiting for connections on port %s...\n", PORT);

    // This is the main loop that accepts incoming connections and
    // forks a handler process to take care of it. The main parent
    // process then goes back to waiting for new connections.

    while (1)
    {
        socklen_t sin_size = sizeof their_addr;

        // Parent process will block on the accept() call until someone
        // makes a new connection:
        newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newfd == -1)
        {
            perror("accept");
            continue;
        }
        // resp_404(newfd);

        // Print out a message that we got the connection
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        // newfd is a new socket descriptor for the new connection.
        // listenfd is still listening for new connections.

        handle_http_request(newfd, cache);

        close(newfd);
    }

    // Unreachable code

    return 0;
}
