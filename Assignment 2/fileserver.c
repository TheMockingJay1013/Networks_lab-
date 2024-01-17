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
    int servsock , newsock ;

    socklen_t clilen; 
    struct sockaddr_in cliaddr , servaddr ;

    char buf[100];
    buf[1]='\0';

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
            int k ;

            int len = -1;
            len = recv(newsock,buf,1,0);
            k = atoi(buf);
            int i=0;

            int fd = open("foo.txt", O_RDWR|O_CREAT);
            if(fd<0)
            {
                printf("unable to open foo.txt\n");
            }
            int sz;
            printf("checking\n");
            while(1)
            {
                len = recv(newsock,buf,100,0);
                buf[len]='\0';
                // printf("%s",buf);
                sz = write(fd,buf,len);
                if(len<100)break;
            }
            write(fd,"\0",1);

            printf("File contents received and stored in foo.txt\n");
            close(fd);

            // now reading from the file ,ecrypting it and storing it enc.txt
            fd = open("foo.txt",O_RDONLY);

            int f2 = open("enc.txt",O_RDWR|O_CREAT);
            if(f2<0)
            {
                printf("unable to open enc.txt\n");
            }


            int n = -1;


            while(1)
            {
                n = read(fd,buf,100);
                if(n==0)break;
                for(int i=0;i<n;i++)
                {
                    if(buf[i]>='a' && buf[i]<='z')
                    {
                        buf[i] = (buf[i]-'a'+k)%26 + 'a';
                    }
                    else if(buf[i]>='A' && buf[i]<='Z')
                    {
                        buf[i] = (buf[i]-'A'+k)%26 + 'A';
                    }
                }
                // printf("%s",buf,n);
                sz = write(f2,buf,n);
            }   
            close(f2);
            printf("File encrypted and stored in enc.txt\n");
            close(fd);

            // sending the encrypted file to the client
            char buf2[100]={'\0'};
            f2 = open("enc.txt",O_RDONLY);
            while(1)
            {
                if ((n=read(f2,buf2,100))<=0) break;
                // printf("%s",buf2);
                send(newsock,buf2,n,0);
            }

            close(newsock);
            exit(0);
        }
        close(newsock);
    }
    return 0;


}