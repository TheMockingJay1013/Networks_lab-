#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int servsock , newsock ;

    int clilen; 
    struct sockaddr_in cliaddr , servaddr ;

    char buf[100];

    //binding
    if((servsock  = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    //server info
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY ;
    servaddr.sin_port = htons(20000);

    // binding the serer with the above server info
    if(bind(servsock,(struct sockaddr *) &servaddr,sizeof(servaddr)) < 0)
    {
        printf("Unable to bind to local address");
        exit(0);
    }

    // setting number of concurret client requests to 4
    listen(servsock,4);

    while(1)
    {
        clilen = sizeof(cliaddr);
        newsock = accept(servsock,(struct sockaddr *)&cliaddr,&clilen);

        if(newsock < 0)
        {
            printf("Accepting error\n");
            exit(0);
        }

        // making the fork for concurrency 

        if(fork() == 0)
        {
            // close old socket in the child process
            close(servsock);

            int len = recv(newsock,buf,100,0);
            printf("%s\n",buf);

            close(newsock);
            exit(0);
        }
        close(newsock);
    }
    return 0;


}