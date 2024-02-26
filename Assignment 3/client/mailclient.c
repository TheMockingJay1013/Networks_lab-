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

// buf is used to store the message and msg is used to send/receive the message
char buf[4096];
char msg[256];

int main(int argc, char *argv[])
{
    char server_ip[100];
    int smtp_port;
    int pop3_port;

    // 3 command line arguments server_ip, smtp_port, pop3_port
    if (argc != 4)
    {
        printf("Usage: %s <server_ip> <smtp_port> <pop3_port>\n", argv[0]);
        exit(0);
    }
    else
    {
        strcpy(server_ip, argv[1]);
        smtp_port = atoi(argv[2]);
        pop3_port = atoi(argv[3]);
    }

    // ask user to enter the username and password
    // not needed for now as we are not using pop3 server
    char username[100];
    char password[100];

    printf("Enter the username: ");
    scanf("%s", username);

    printf("Enter the password: ");
    scanf("%s", password);

    int client_socket;
    struct sockaddr_in server_addr, client_addr;

    // enter the main loop
    while (1)
    {
        // ask user to choose from the menu
        printf("\n1.Manage Mail\n2.Send Mail\n3.Quit\n");

        int choice;
        scanf("%d", &choice);

        if (choice == 3)
        {
            // send receiver that quit is requested
            printf("Quitting\n");

            // break any connection is open
            return 0;
        }

        else if (choice == 1)
        {
            // pop3 server part

            if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Unable to create socket\n");
                exit(0);
            }

            // server info
            server_addr.sin_family = AF_INET;
            inet_aton(server_ip, &server_addr.sin_addr);
            server_addr.sin_port = htons(pop3_port);
            // connect to the server
            if ((connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
            {
                perror("Unable to connect to server");
                exit(0);
            }

            // receive ready from server
            // if the message is not +OK , then close the connection and exit
            memset(buf, 0, sizeof(buf));
            int len;

            len = recv(client_socket, buf, sizeof(buf), 0);

            if (strncmp(buf, "+OK", 3) != 0)
            {
                printf("Error in connection\n");
                close(client_socket);
                exit(0);
            }

            printf("%s\n", buf);

            // send USER name
            memset(msg, 0, sizeof(msg));
            strcpy(msg, "USER ");
            strcat(msg, username);
            strcat(msg, "\r\n");

            send(client_socket, msg, strlen(msg), 0);

            // check if the user is valid
            memset(buf, 0, sizeof(buf));
            len = recv(client_socket, buf, sizeof(buf), 0);
            buf[len - 2] = '\0';

            if (strncmp(buf, "+OK", 3) != 0)
            {
                printf("No such user\n%s\n", buf);
                close(client_socket);
                exit(0);
            }

            printf("%s\n", buf);

            // send PASS password
            memset(msg, 0, sizeof(msg));
            strcpy(msg, "PASS ");
            strcat(msg, password);
            strcat(msg, "\r\n");

            send(client_socket, msg, strlen(msg), 0);

            // check if the password is valid
            memset(buf, 0, sizeof(buf));
            len = recv(client_socket, buf, sizeof(buf), 0);
            buf[len - 2] = '\0';

            if (strncmp(buf, "+OK", 3) != 0)
            {
                printf("Error in authentication\n%s\n", buf);
                close(client_socket);
                exit(0);
            }

            int mark[1000] = {0};

            printf("%s\n", buf);

            while (1)
            {

                // send STAT to get number of mails
                memset(msg, 0, sizeof(msg));
                // memset(buf, 0, sizeof(buf));
                strcpy(msg, "STAT\r\n");
                send(client_socket, msg, strlen(msg), 0);

                // recv +OK
                memset(buf, 0, sizeof(buf));
                len = recv(client_socket, buf, sizeof(buf), 0);
                buf[len - 2] = '\0';

                if (strncmp(buf, "+OK", 3) != 0)
                {
                    printf("Error in connection\n");
                    close(client_socket);
                    exit(0);
                }

                // receive the list of mails character by character
                printf("List of mails:\n");
                printf("%-5s\t%-25s\t%-30s\t%-25s\n", "Sl.No", "From", "Received at", "Subject");

                // extract number of mails
                int nummails;
                sscanf(buf, "+OK %d", &nummails);

                for (int i = 0; i < nummails; i++)
                {
                    if (!mark[i + 1])
                    {
                        // send RETR to get the mail
                        memset(msg, 0, sizeof(msg));
                        sprintf(msg, "RETR %d\r\n", i + 1);
                        send(client_socket, msg, strlen(msg), 0);

                        // receive the mail
                        memset(buf, 0, sizeof(buf));
                        while (1)
                        {
                            // receive the message character by character
                            char x[5];
                            memset(x, 0, sizeof(x));
                            recv(client_socket, x, 1, 0);
                            strcat(buf, x);

                            // breaking case
                            if (strlen(buf) >= 5 && buf[strlen(buf) - 5] == '\r' && buf[strlen(buf) - 4] == '\n' && buf[strlen(buf) - 3] == '.' && buf[strlen(buf) - 2] == '\r' && buf[strlen(buf) - 1] == '\n')
                            {
                                break;
                            }
                        }

                        // get the sender and subject from the mail
                        char from[100];
                        char subject[100];
                        char date[100];
                        int f_flag = 0;
                        int s_flag = 0;
                        int d_flag = 0;

                        for (int i = 0; i < strlen(buf); i++)
                        {

                            if (buf[i] == 'F' && buf[i + 1] == 'r' && buf[i + 2] == 'o' && buf[i + 3] == 'm' && buf[i + 4] == ':')
                            {
                                if (f_flag)
                                    continue;
                                f_flag = 1;
                                int j = i + 5;
                                while (buf[j] == ' ')
                                {
                                    j++;
                                }
                                int k = 0;
                                while (buf[j] != '\r' && buf[j] != ' ')
                                {
                                    from[k] = buf[j];
                                    k++;
                                    j++;
                                }
                                from[k] = '\0';
                            }
                            if (buf[i] == 'S' && buf[i + 1] == 'u' && buf[i + 2] == 'b' && buf[i + 3] == 'j' && buf[i + 4] == 'e' && buf[i + 5] == 'c' && buf[i + 6] == 't' && buf[i + 7] == ':')
                            {
                                if (s_flag)
                                    continue;
                                s_flag = 1;
                                int j = i + 8;
                                while (buf[j] == ' ')
                                {
                                    j++;
                                }
                                int k = 0;
                                while (buf[j] != '\r')
                                {
                                    subject[k] = buf[j];
                                    k++;
                                    j++;
                                }
                                subject[k] = '\0';
                            }
                            if (buf[i] == 'R' && buf[i + 1] == 'e' && buf[i + 2] == 'c' && buf[i + 3] == 'e' && buf[i + 4] == 'i' && buf[i + 5] == 'v' && buf[i + 6] == 'e' && buf[i + 7] == 'd' && buf[i + 8] == ':')
                            {
                                if (d_flag)
                                    continue;
                                d_flag = 1;
                                int j = i + 9;
                                while (buf[j] == ' ')
                                {
                                    j++;
                                }
                                int k = 0;
                                while (buf[j] != '\r')
                                {
                                    date[k] = buf[j];
                                    k++;
                                    j++;
                                }
                                date[k] = '\0';
                            }
                        }

                        // print the mail details
                        printf("%-5d\t%-25s\t%-30s\t%-25s\n", i + 1, from, date, subject);
                    }
                }

                int flag = 0;
                int flag2 = 0;
                while (1)
                {
                    // ask user to enter the mail number he wishes to see
                    int mailno;
                    printf("Enter the mail number you wish to see:(-1 to exit) ");
                    scanf("%d", &mailno);

                    if (mailno == -1)
                    {
                        flag = 1;
                        break;
                    }

                    // check if mailno is valid by sending RETR to the server
                    memset(msg, 0, sizeof(msg));
                    sprintf(msg, "RETR %d\r\n", mailno);
                    send(client_socket, msg, strlen(msg), 0);

                    // receive the mail
                    flag2 = 0;
                    memset(buf, 0, sizeof(buf));
                    while (1)
                    {

                        char x[5];
                        memset(x, 0, sizeof(x));
                        recv(client_socket, x, 1, 0);
                        strcat(buf, x);

                        // breaking case
                        if (strlen(buf) >= 5 && buf[strlen(buf) - 5] == '\r' && buf[strlen(buf) - 4] == '\n' && buf[strlen(buf) - 3] == '.' && buf[strlen(buf) - 2] == '\r' && buf[strlen(buf) - 1] == '\n')
                        {
                            flag2 = 1;
                            buf[strlen(buf) - 1] = '\0';
                            printf("%s\n", buf);
                            if (strlen(buf) > 5 && buf[0] == '-' && buf[1] == 'E' && buf[2] == 'R' && buf[3] == 'R')
                            {
                                printf("Error in mail number\n");
                                flag2 = 0;
                                break;
                            }
                            break;
                        }
                    }

                    if (flag2 == 1)
                    {
                        char d;
                        scanf("\n%c", &d);
                        if (d == 'd')
                        {
                            // send DELE to delete the mail
                            memset(msg, 0, sizeof(msg));
                            sprintf(msg, "DELE %d\r\n", mailno);
                            send(client_socket, msg, strlen(msg), 0);

                            // recv +OK
                            memset(buf, 0, sizeof(buf));
                            len = recv(client_socket, buf, sizeof(buf), 0);
                            buf[len - 2] = '\0';

                            printf("%s\n", buf);
                            mark[mailno] = 1;
                        }

                        break;
                    }
                }
                if (flag == 1)
                {
                    // send QUIT
                    memset(msg, 0, sizeof(msg));
                    strcpy(msg, "QUIT\r\n");

                    send(client_socket, msg, strlen(msg), 0);

                    // recv OK
                    memset(buf, 0, sizeof(buf));
                    len = recv(client_socket, buf, sizeof(buf), 0);
                    buf[len - 2] = '\0';

                    if (strncmp(buf, "+OK", 3) != 0)
                    {
                        printf("Error msg\n");
                        close(client_socket);
                        exit(0);
                    }

                    printf("%s\n", buf);

                    break;
                }

                if (flag == 1)
                    break;
            }

            // close the connection
            close(client_socket);
            printf("Connection closed\n");
        }

        else if (choice == 2)
        {
            printf("\n");
            // opening a socket
            if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Unable to create socket\n");
                exit(0);
            }

            // server info
            server_addr.sin_family = AF_INET;
            inet_aton(server_ip, &server_addr.sin_addr);
            server_addr.sin_port = htons(smtp_port);
            // connect to the server
            if ((connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
            {
                perror("Unable to connect to server");
                exit(0);
            }

            // if the message is not 220 , then close the connection and exit
            memset(buf, 0, sizeof(buf));
            int len;
            len = recv(client_socket, buf, sizeof(buf), 0);

            if (strncmp(buf, "220", 3) != 0)
            {
                printf("Error in connection\n");
                close(client_socket);
                exit(0);
            }

            buf[len - 2] = '\0';
            strcat(buf, inet_ntoa(server_addr.sin_addr));
            strcat(buf, "... Service ready\r\n");

            printf("%s\n", buf);
            printf("%s\n",inet_ntoa(server_addr.sin_addr));

            // send HELO
            memset(msg, 0, sizeof(msg));
            strcat(msg, "HELO ");
            strcat(msg, server_ip);
            strcat(msg, "\r\n");

            send(client_socket, msg, strlen(msg), 0);

            // recv 250
            memset(buf, 0, sizeof(buf));
            len = recv(client_socket, buf, sizeof(buf), 0);

            if (strncmp(buf, "250", 3) != 0)
            {
                printf("Error in connection\n");
                close(client_socket);
                exit(0);
            }

            printf("%s\n", buf);

            // ask user to enter the sender's mail id

            char sender[100];
            printf("Enter the sender's mail id: ");
            scanf("%s", sender);

            // send MAIL FROM
            memset(msg, 0, sizeof(msg));
            strcpy(msg, "MAIL FROM: <");
            strcat(msg, sender);
            strcat(msg, ">\r\n");

            send(client_socket, msg, strlen(msg), 0);

            // recv 250
            memset(buf, 0, sizeof(buf));
            len = recv(client_socket, buf, sizeof(buf), 0);
            buf[len - 2] = '\0';

            if (strncmp(buf, "250", 3) != 0)
            {
                printf("Error in connection\n");
                close(client_socket);
                exit(0);
            }

            printf("%s\n", buf);

            // ask user to enter the receiver's mail id
            char receiver[100];
            printf("Enter the receiver's mail id: ");
            scanf("%s", receiver);

            // send RCPT TO
            memset(msg, 0, sizeof(msg));
            strcpy(msg, "RCPT TO: <");
            strcat(msg, receiver);
            strcat(msg, ">\r\n");

            send(client_socket, msg, strlen(msg), 0);

            // recv 250
            memset(buf, 0, sizeof(buf));
            len = recv(client_socket, buf, sizeof(buf), 0);
            buf[len - 2] = '\0';

            if (strncmp(buf, "250", 3) != 0)
            {
                printf("%s\n", buf);
                close(client_socket);
                continue;
            }

            printf("%s\n", buf);

            // send DATA
            memset(msg, 0, sizeof(msg));
            strcpy(msg, "DATA\r\n");

            send(client_socket, msg, strlen(msg), 0);

            // recv 354
            memset(buf, 0, sizeof(buf));
            len = recv(client_socket, buf, sizeof(buf), 0);

            if (strncmp(buf, "354", 3) != 0)
            {
                printf("Error in connection\n");
                close(client_socket);
                exit(0);
            }

            printf("%s\n", buf);

            printf("Enter the message:(strictly adhere to the mail format) \n");

            // taking msg input from user line by line and checking format
            memset(msg, 0, sizeof(msg));
            memset(buf, 0, sizeof(buf));

            // msgarr is used to store the message line by line
            char msgarr[50][80] = {0};
            int msgarrlen = 0;

            while (1)
            {
                // use formatted input to take input till \n
                // store the input in msgarr
                memset(msg, 0, sizeof(msg));
                scanf("\n%[^\n]s", msg);
                if (strcmp(msg, ".") == 0)
                {
                    strcpy(msgarr[msgarrlen], "\r\n.\r\n");
                    msgarrlen++;
                    break;
                }
                else
                {
                    strcpy(msgarr[msgarrlen], msg);
                    strcat(msgarr[msgarrlen], "\r\n");
                    msgarrlen++;
                }
            }

            // check line 0 for from and @
            if (strncmp(msgarr[0], "From:", 5) != 0)
            {
                printf("Error in format\n");
                close(client_socket);
                continue;
            }
            else
            {
                int i = 5;
                while (msgarr[0][i] != '\0')
                {
                    if (msgarr[0][i] == '@')
                        break;
                    i++;
                }
                if (msgarr[0][i] == '\0')
                {
                    printf("Error in format\n");
                    close(client_socket);
                    continue;
                    ;
                }
            }

            // check line 1 for to and @
            if (strncmp(msgarr[1], "To:", 3) != 0)
            {
                printf("Error in format\n");
                close(client_socket);
                continue;
            }
            else
            {
                int i = 3;
                while (msgarr[1][i] != '\0')
                {
                    if (msgarr[1][i] == '@')
                        break;
                    i++;
                }
                if (msgarr[1][i] == '\0')
                {
                    printf("Error in format\n");
                    close(client_socket);
                    continue;
                }
            }

            // check line 2 for subject
            if (strncmp(msgarr[2], "Subject:", 8) != 0)
            {
                printf("Error in format\n");
                close(client_socket);
                continue;
            }

            // check last line for \r\n.\r\n
            if (strncmp(msgarr[msgarrlen - 1], "\r\n.\r\n", 5) != 0)
            {
                printf("Error in format\n");
                close(client_socket);
                continue;
            }

            // send the message from msgarr
            for (int i = 0; i < msgarrlen; i++)
            {
                send(client_socket, msgarr[i], strlen(msgarr[i]), 0);
            }

            // recv 250
            memset(buf, 0, sizeof(buf));
            while (1)
            {
                len = recv(client_socket, buf, sizeof(buf), 0);
                if (buf[len - 1] == '\n' && buf[len - 2] == '\r')
                    break;
            }

            if (strncmp(buf, "250", 3) != 0)
            {
                printf("Error in connection\n");
                close(client_socket);
                exit(0);
            }

            printf("%s\n", buf);

            // send QUIT
            memset(msg, 0, sizeof(msg));
            strcpy(msg, "QUIT\r\n");

            send(client_socket, msg, strlen(msg), 0);

            // recv 221
            memset(buf, 0, sizeof(buf));
            len = recv(client_socket, buf, sizeof(buf), 0);

            if (strncmp(buf, "221", 3) != 0)
            {
                printf("Error in connection\n");
                close(client_socket);
                exit(0);
            }

            buf[len - 2] = '\0';
            strcat(buf, inet_ntoa(server_addr.sin_addr));
            strcat(buf, "closing connection\r\n");

            printf("%s\n", buf);

            close(client_socket);
            printf("Connection closed\n");
        }
    }
}