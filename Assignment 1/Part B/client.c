// Name : Navaneeth Shaji
// Roll no:  21CS30032
// Assignment 1 Part B


// Client

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

int main()
{
    int clisock ,err ;
    struct sockaddr_in servaddr , cliaddr ;
    int n;
    socklen_t len ;
    char * filename = "Trial.txt";
    char buffer[1024];

    //making the client socket
    clisock = socket(AF_INET,SOCK_DGRAM,0);
    if(clisock<0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    cliaddr.sin_family=AF_INET;
    cliaddr.sin_addr.s_addr = INADDR_ANY;
    cliaddr.sin_port = htons(6543);

    if(bind(clisock,(const struct sockaddr *)&cliaddr,sizeof(cliaddr))<0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Client running ...\n");

    servaddr.sin_family = AF_INET ;
    servaddr.sin_port = htons(8181);
    err = inet_aton("127.0.0.1",&servaddr.sin_addr);
    if(err == 0)
    {
        printf("Error in ip-conversion\n");
	    exit(EXIT_FAILURE);
    }

    sendto(clisock,(const char *)filename,strlen(filename),0,(const struct sockaddr *)&servaddr,sizeof(servaddr));
    int t;
    if((t =recvfrom(clisock,(char *)buffer,1024,0,(struct sockaddr *)&servaddr,sizeof(servaddr)))<0)printf("ERROR while receiving hello %d\n",t);
    char hel[] = "HELLO";
    printf("%d\n",t);
    buffer[5]='\0';
    printf("t = %d ,buffer = %s , %d\n triea",t,buffer,strlen(buffer));
    
    if(strcmp(buffer,hel)==0)
    {
        printf("HELLO FOUND");
        int i=0;
        char word[] = "WORD ";
        int wordlen = strlen(word);
        FILE * fp = fopen("new.txt","a");

        while(1)
        {   
            i++;
            word[wordlen-2]=i+'0';

            sendto(clisock,(const char*)word,wordlen,0,(const struct sockaddr *)&servaddr,sizeof(servaddr));
            recvfrom(clisock,(char *)buffer,1024,0,(struct sockaddr *)&servaddr,sizeof(servaddr));

            if(strcmp(buffer,"END")==0)
            {
                fclose(fp);
                break;
            }
            else 
                fputs(buffer,fp);


        }
        
    }
    else printf("HELLO NOT READ\n");
    close(clisock);
    return 0;



}