//
//  webserver
//
//
//  Created by Kim Swennen and Ben Tzou.
//
//

/*QUESTIONS:
    nothing new here; just checking that the commit works
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define true 1
#define false 0

#define MYPORT 2000  //server port number
#define BACKLOG 20
#define BUFSIZE 256

void respondWithHTML(int socketfd);
const int isValidHttpRequest(const char* response);
const char* getRequestedFilename(const char* buffer);


void sigChildHandler (int s) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main (int argc, char * argv[]) {
 
    /*---------variable declarations------------*/
    
    int sockfd;     //socket file descriptor
    int childSockfd;    //socket descriptor for forked child
    socklen_t clientLength = sizeof(struct sockaddr_in);   //length of client address
    short portno = MYPORT;
    int backlog = BACKLOG;
    struct sockaddr_in serverAddress, clientAddress;
    pid_t pid;
    struct sigaction signalAction;
    
    /*----------code to set up signal handling for zombie child processes---------------*/
    signalAction.sa_handler = sigChildHandler;
    if (sigemptyset(&signalAction.sa_mask) < 0) {
        perror("sigemptyset error");
        exit(1);
    }
    signalAction.sa_flags=SA_RESTART;
    if (sigaction(SIGCHLD, &signalAction, NULL) < 0){
        perror("error on sigaction");
        exit(1);
    }
    
    /*---------socket setup-----------*/
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);   //creating a ipv4 socket to use TCP
    if (sockfd < 0) {
        perror("error opening socket");
        exit(1);
    }
    
    /*---------set up address structure------------*/
    bzero((char *) &serverAddress, sizeof(serverAddress));     //zero out the contents of the serverAddress
    //set server address details:
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portno);     //in network byte order
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    
    
    /*---------bind-----------*/
    if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) < 0) {
        perror("error on binding");
        exit(1);
    }
    
    
    /*---------listen------------*/
    if (listen(sockfd, backlog) < 0) {
        perror("error on listening");
        exit(1);
    }
    
    while (1) {             //will run forever
        //create new socket for incoming client
        childSockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &clientLength);   
        if (childSockfd < 1)  {
            perror("error on accept");
            exit(1);
        }
        
   //     if (clientLength < sizeof(struct sockaddr_in)      ->  do we need to check for this?
        
        pid = fork();
        if (pid < 0) {
            perror("error on fork");
            exit(1);
        }
        
        if (pid == 0) {
            if (close(sockfd) < 0){
                perror("error on close in child");
                exit(1);
            }
            respondWithHTML(childSockfd);
            exit(0);
        }
        
        else
            if (close(childSockfd)<0){
                perror("error on close in server");
                exit (1);
            }
    }
    
    return 0;
}

        
void dumpRequestMessage(int socketfd) {
    char buf [BUFSIZE];
    long int test;
    bzero(buf, BUFSIZE);

    test = read(socketfd, buf, BUFSIZE);
    
    if (test < 0){  //check if read failed
        perror("error on read");
        exit(1);
    }
    
    else if (test == 0)  //socket closed
        fprintf(stderr, "client socket closed - no request messages received\n");
    
    else {
        printf("%s",buf);
    }    
}


void respondWithHTML(int socketfd) {
    char buf[BUFSIZE];
    long int test;
    bzero(buf, BUFSIZE);
    char header[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
    char body[] = "<html><h1>Hello World</h1></html>";
    
    test = read(socketfd, buf, BUFSIZE);
    
    if (test < 0) {
        perror("error on read");
        exit(1);
    }
    
    else if (test == 0)  //socket closed
        fprintf(stderr, "client socket closed - no request messages received\n");

    else {
        printf("%s", buf);  //print request
        
        // writing to client OK response and HTML
        test = write(socketfd, header, sizeof(header));
        test = write(socketfd, body, sizeof(body));
        
        if (test < 0)
            perror("error on write");
    }
}


// Input: response, containing the HTTP GET request
// Output: true if valid HTTP GET request, false otherwise
const int isValidHttpRequest(const char* response) {
    char buffer[strlen(response)];
    strcpy(buffer, response);
    
    char *line = strtok(buffer, "\n");
    
    // check if first line of HTTP request is null
    if (line != NULL) {
        char *word = strtok(line, " ");
        
        // check if first word is GET
        if (strcmp(word, "GET") == 0) {
            /* char *filename = */ strtok(NULL, " ");   // avoiding warning for unused variable
            char *httpVersion = strtok(NULL, " ");
            char *endOfGet = strtok(NULL, " ");
            
            // check if HTTP version is 1.x and no other characters follow HTTP version
            if (httpVersion != NULL && endOfGet == NULL &&
                    strncmp(httpVersion, "HTTP/1.", 7) == 0)
                
                // everything checks out, return filename
                return true;
        }
    }
    
    // if any problem with GET format, send
    return false;
}


// Input: response, containing the HTTP GET request
// Output: requested filename
// Assumptions: HTTP GET request has been validated by isValidHttpRequest
const char* getRequestedFilename(const char* response) {
    char buffer[strlen(response)];
    strcpy(buffer, response);
    
    strtok(buffer, " ");
    return strtok(NULL, " ");
}