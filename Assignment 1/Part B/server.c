// Name : Navaneeth Shaji
// Roll no:  21CS30032
// Assignment 1 Part B

// Server 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAXLINE 1024

int main()
{
    int servsock = socket(AF_INET, SOCK_DGRAM, 0);
    if(servsock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr, cliaddr;
    int n;
    socklen_t len;
    char buffer[MAXLINE];
    char buffer2[MAXLINE];

    memset(&servaddr,0,sizeof(servaddr));
    memset(&cliaddr,0,sizeof(cliaddr));

    servaddr.sin_family = AF_INET ;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8181);

    if(bind(servsock,(const struct sockaddr *)&servaddr,sizeof(servaddr))<0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server Running ...\n");

    len = sizeof(cliaddr);
    n = recvfrom(servsock,(char *)buffer,MAXLINE,0,(struct sockaddr *)&cliaddr,&len);
    FILE * fp;
    const char *x = buffer;
    buffer[n]='\0';

    fp = fopen(x,"r");
    if(fp==NULL)
    {
        printf("FILE NOT FOUND!\n");
        char message[120]="NOTFOUND ";
        buffer[n]='\0';
        x = strcat((char *)message,(char *)buffer);
        sendto(servsock,x,strlen(x),0,&cliaddr,sizeof(cliaddr));
        exit(1);
    }
    printf("FILE FOUND!\n");

    // fscanf(fp,"%s",buffer);
    fgets(buffer,1024,fp);
    int l = strlen(buffer);
    buffer[l-1]='\0';

    int d =sendto(servsock,(char *)buffer,l,0,&cliaddr,sizeof(cliaddr));


    while(1)
    {
        int q =recvfrom(servsock,(char *)buffer,MAXLINE,0,(struct sockaddr *)&cliaddr,&len);
        fgets(buffer,MAXLINE,fp);
        l = strlen(buffer);

        sendto(servsock,(char *)buffer,l,0,&cliaddr,sizeof(cliaddr));
        if(strcmp(buffer,"END")==0)
        {
            break;
        }
    }

    close(servsock);
    printf("DONE!!\n");
    return 0;
    

}


