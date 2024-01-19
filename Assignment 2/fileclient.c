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

    // connecting to the server
    // make the connection 
    if((connect(clisock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)))<0)
    {
        perror("Unable to connect to server");
        exit(0);
    }

    int f;
    while(1)
    {
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
        buf[0]='#';
        send(clisock,buf,1,0);
        printf("Sent file from client to server\n");
        close(f); // closing the file 

        // client receives the encrypted file from the server ans stores in new file

        char encfilename[1024];
        strcpy(encfilename,filename);
        strcat(encfilename,".enc");
        f = open(encfilename,O_RDWR|O_CREAT);
        if(f<0)
        {
            perror("Unable to create file");
            exit(0);
        }

        int end = 0;
        int sz;
        while(1)
        {
            int len  = recv(clisock,buf,100,0);
            for(int i=0;i<len;i++)
            {
                if(buf[i]=='#')
                {
                    end = 1 ;
                    len = i;
                    break;
                }
            }
            sz = write(f,buf,len);
            if(end)break;
            memset(buf,'\0',sizeof(buf));
        }
        write(f,"\0",1);

        printf("File received and stored in encrypted.txt.enc\n");
        close(f);
        printf("Do you want to continue (y/n) : ");
        char ch;
        scanf(" %c",&ch);
        if(ch=='n')break;
    }
    close(clisock);




    return 0;

}