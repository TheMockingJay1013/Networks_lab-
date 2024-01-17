#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main()
{
    int clisock ;
    struct sockaddr_in serv_addr , cliaddr ;

    int i ;
    char buf[100] ;

    //opening a socket
    if((clisock = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // binding the client (optional in this case)



    // server info 
    serv_addr.sin_family = AF_INET ;
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
    serv_addr.sin_port = htons(20000);

    // reading from file
    int k=2;
    char filename[1024] = "trial.txt";

    int f;

    
    while(1)
    {
        printf("Enter the filename : ");
        scanf("%s",filename);
        f = open(filename,O_RDONLY);
        if(f!=-1) break;
    }
    printf("FILE FOUND!!\n");

    // input k
    printf("Enter k : ");
    scanf("%d",&k);

    // make the connection 
    if((connect(clisock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)))<0)
    {
        perror("Unable to connect to server");
        exit(0);
    }

    // first send the key value to the server
    buf[0] = '0'+k;
    // buf[1] = '\0';
    send(clisock,buf,1,0);

    // start reading from file and sending it 
    int n = -1;

    while(n!=0)
    {
        n = read(f,buf,100);
        send(clisock,buf,n,0);
        
    }
    printf("Sent file from client to server\n");
    close(f); // closing the file 

    // client receives the encrypted file from the server ans stores in new file

    f = open("encrypted.txt.enc",O_RDWR|O_CREAT);
    if(f<0)
    {
        perror("Unable to create file");
        exit(0);
    }


    char buf2[100]={'\0'};
    while(1)
    {
        int len  = recv(clisock,buf2,100,0);
        buf2[len]='\0';

        write(f,buf2,len);
        // printf("%s",buf2);
        if(len<100)break;
        memset(buf2,'\0',sizeof(buf2));
    }

    printf("File received and stored in encrypted.txt.enc\n");
    close(f);
    // print client message

    // looping this process indefinitely




    return 0;

}