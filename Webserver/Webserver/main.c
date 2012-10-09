//
//  webserver
//
//
//  Created by Kim Swennen and Ben Tzou.
//
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>   //do we need this?
#include <signal.h>     //this too?

int main (int argc, char * argv[]) {
    
    /*---------socket setup-----------*/
    int sockfd;     //socket file descriptor
    
    sockfd = socket(PF_INET, SOCK_STREAM, 0);   //creating a ipv4 socket to use TCP
    if (sockfd < 0)
        perror("error opening socket");
    
    
    
    
    
}
