#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>



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

            
            while(1)
            {    
                printf("waiting .. \n");
                int len = -1;
                len = recv(newsock,buf,1,0);
                k = atoi(buf);
                int i=0;

                char filename[1024];
                sprintf(filename,"%s.%d.txt",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

                int fd = open(filename, O_RDWR|O_CREAT|O_TRUNC,0666);
                if(fd<0)
                {
                    printf("unable to open foo.txt\n");
                }
                int sz;
                long long length=0;
                int end = 0 ;
                while(1)
                {
                    len = recv(newsock,buf,100,0);
                    for(int i=0;i<len;i++)
                    {
                        if(buf[i]=='#')
                        {
                            end = 1;
                            len = i+1;
                            break;
                        }
                    }
                    sz = write(fd,buf,len);
                    if(end)break;
                    memset(buf,'$',sizeof(buf));
                }
                write(fd,"\0",1);

                printf("File contents received and stored in %s\n",filename);
                close(fd);

                // now reading from the file ,ecrypting it and storing it enc.txt
                fd = open(filename,O_RDWR);

                char encfilename[1024];
                sprintf(encfilename,"%s.%d.txt.enc",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

                int f2 = open(encfilename,O_RDWR|O_CREAT|O_TRUNC,0666);
                if(f2<0)
                {
                    printf("unable to open %s\n",encfilename);
                }


                int n = -1;

                end = 0;
                while(1)
                {
                    n = read(fd,buf,100);
                    if(n==0)break;
                    for(int i=0;i<n;i++)
                    {
                        if(buf[i]=='#')
                        {
                            end = 1;
                        }
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
                    if(end)break;
                }   
                close(f2);
                close(fd);

                // sending the encrypted file to the client
                f2 = open(encfilename,O_RDWR);
                n=-1;
                while(n!=0)
                {
                    n=read(f2,buf,100);
                    send(newsock,buf,n,0);
                }

                printf("\nThe file has been encrypted and sent back to client\n");
            }
            close(newsock);
            exit(0);
        }
        close(newsock);
    }
    return 0;


}