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
    struct sockaddr_in serv_addr , cli_addr ;

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
    int k;
    char filename[1024];

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
    buf[1] = '\0';
    send(clisock,buf,strlen(buf)+ 1,0);

    // start reading from file and sending it 
    int n = -1;

    // while(1)
    // {
    //     n = read(f,buf,100);
    //     if(n==0)
    //     {
    //         break;
    //     }
    //     buf[n] = '\0';

    //     // printf("%s,%lu\n",buf,strlen(buf));

    //     // send(clisock,buf,n+1,0);


    // }

    n = read(f,buf,100);
    buf[n] = '\0';
    send(clisock,buf,strlen(buf)+1,0);

    // client receives the encrypted file from the server ans stores in new file

    // print client message

    // looping this process indefinitely




    return 0;

}