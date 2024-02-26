#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

struct user{
    char name[100];
    char ip[100];
    int port;
};

int main(int argc, char *argv[])
{
    int servsock, newsock1, newsock2;
    int clientother1, clientother2;
    struct sockaddr_in clientother1addr, clientother2addr;
    struct sockaddr_in cliaddr1, cliaddr2;

    struct user users[4];
    strcpy(users[1].name ,"user_1");
    strcpy(users[1].ip ,"127.0.0.1");
    users[1].port = 50000;

    strcpy(users[2].name ,"user_2");
    strcpy(users[2].ip ,"127.0.0.1");
    users[2].port = 50001;

    strcpy(users[3].name ,"user_3");
    strcpy(users[3].ip ,"127.0.0.1");
    users[3].port = 50002;

    // identify self

    int self ;
    int port = atoi(argv[1]);

    if(users[1].port == port)
    {
        self = 1;
    }
    else if(users[2].port == port)
    {
        self = 2;
    }
    else if(users[3].port == port)
    {
        self = 3;
    }
    else
    {
        printf("Invalid port\n");
        exit(0);
    }

    // identify the other users
    int other1, other2;
    if(self == 1)
    {
        other1 = 2;
        other2 = 3;
    }
    else if(self == 2)
    {
        other1 = 1;
        other2 = 3;
    }
    else
    {
        other1 = 1;
        other2 = 2;
    }

    // flags to know if the other users are connected
    int f1 = 0, f2 = 0;


    // server info
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY ;
    servaddr.sin_port = htons(port);

    // create socket
    if((servsock  = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // binding
    if(bind(servsock,(struct sockaddr *) &servaddr,sizeof(servaddr)) < 0)
    {
        printf("Unable to bind to local address");
        exit(0);
    }

    printf("Server started on port %d\n",port);

    // setting number of concurret client requests to 3
    listen(servsock,3);


    // use fd_set to handle multiple clients
    fd_set fd;

    FD_ZERO(&fd);
    FD_SET(servsock, &fd);
    FD_SET(STDIN_FILENO, &fd);
    
    while(1)
    {
        select(FD_SETSIZE, &fd, NULL, NULL, NULL);

        if(FD_ISSET(servsock,&fd))
        {
            // its an accept call for the server
            socklen_t clilen = sizeof(cliaddr1);
            if(!(f1&&f2)){
                newsock1 = accept(servsock,(struct sockaddr *)&cliaddr1,&clilen);
                if(newsock1 < 0)
                {
                    printf("Accepting error\n");
                    exit(0);
                }

                FD_SET(newsock1, &fd);
            }
            else
            {
                newsock2 = accept(servsock,(struct sockaddr *)&cliaddr2,&clilen);
                if(newsock2 < 0)
                {
                    printf("Accepting error\n");
                    exit(0);
                }

                FD_SET(newsock2, &fd);
            }  

            FD_SET(servsock, &fd);
        }

        if(FD_ISSET(STDIN_FILENO,&fd))
        {
            char buf[100];
            scanf("\n%[^\n]s",buf);

            // extract the message and the user
            char message[100];
            int receiver;
            sscanf(buf,"user_%d/",&receiver);

            strcpy(message,buf+8);
            // remove first 2 and last 2 characters
            // message[strlen(message)-2] = '\n';
            message[strlen(message)-1] = '\0';

            printf("Message : %s\n",message);


            // identify the receiver
            if(receiver==other1)
            {
                char x[100];
                strcpy(x,"Message from ");
                strcat(x,users[self].name);
                strcat(x,": ");
                strcat(x,message);
                if(f1)
                {
                    // connection is already there
                    send(clientother1,x,strlen(x),0);
                }
                else 
                {
                    // request a connection to the receiver
                    if((clientother1 = socket(AF_INET, SOCK_STREAM, 0))<0)
                    {
                        printf("Socket creation failed\n");
                        exit(0);
                    }
                    clientother1addr.sin_family = AF_INET;
                    inet_aton("127.0.0.1",&clientother1addr.sin_addr);
                    clientother1addr.sin_port = htons(users[other1].port);


                    if(connect(clientother1, (struct sockaddr *)&clientother1addr, sizeof(clientother1addr)) < 0)
                    {
                        printf("Connection failed\n");
                        exit(0);
                    }


                    // send the message
                    send(clientother1,x,strlen(x),0);
                    memset(x,0,sizeof(x));
                    memset(message,0,sizeof(message));
                    
                    f1 = 1;
                }
            }
            else if(receiver==other2)
            {
                char x[100];
                strcpy(x,"Message from ");
                strcat(x,users[self].name);
                strcat(x,": ");
                strcat(x,message);
                if(f2)
                {
                    // connection is already there
                    send(clientother2,x,strlen(x),0);
                }
                else 
                {
                    // request a connection to the receiver
                    clientother2 = socket(AF_INET, SOCK_STREAM, 0);
                    clientother2addr.sin_family = AF_INET;
                    inet_aton("127.0.0.1",&clientother2addr.sin_addr);
                    clientother2addr.sin_port = htons(users[other2].port);


                    if(connect(clientother2, (struct sockaddr *)&clientother2addr, sizeof(clientother2addr)) < 0)
                    {
                        printf("Connection failed\n");
                        exit(0);
                    }


                    // send the message
                    send(clientother2,x,strlen(x),0);
                    memset(x,0,sizeof(x));
                    memset(message,0,sizeof(message));
                    f2=1;
                
                }
            }
            else
            {
                printf("Invalid receiver\n");
            }

            
            FD_SET(STDIN_FILENO, &fd);
        }
        if(FD_ISSET(newsock1,&fd))
        {
            char buff[100];
            int len ;
            len = recv(newsock1,buff,100,0);

            if(strncmp(buff,"Message from",12)==0)
            {
                char t[100];
                int sender;
                sscanf(buff,"Message from user_%d: %s",&sender,t);
                if(sender==other1)
                {
                    f1 = 1;
                }
                else if(sender==other2)
                {
                    f2 = 1;
                }
                printf("%s\n",buff);
            }
            FD_SET(newsock1, &fd);
        }
        if(FD_ISSET(newsock2,&fd))
        {
            char buff[100];
            int len ;
            len = recv(newsock2,buff,100,0);
            if(strncmp(buff,"Message from",12)==0)
            {
                char t[100];
                int sender;
                sscanf(buff,"Message from user_%d: %s",&sender,t);
                if(sender==other1)
                {
                    f1 = 1;
                }
                else if(sender==other2)
                {
                    f2 = 1;
                }
                printf("%s\n",buff);
            }
            
            FD_SET(newsock2, &fd);
        }

    }
    return 0;











}