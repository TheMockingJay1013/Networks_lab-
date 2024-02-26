#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char *argv[])
{

    int server_socket, client_socket;
    int new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_len = sizeof(client_addr);

    char buf[2048];

    // creating the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error in creating the socket\n");
        exit(1);
    }

    int port;

    if (argc == 2)
    {
        port = atoi(argv[1]);
    }

    else
    {
        printf("Usage: %s <pop3_port>\n", argv[0]);
        exit(0);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error in binding\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // listen for the connections
    if (listen(server_socket, 5) == -1)
    {
        perror("Error in listening\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("POP3 server is listening on port %d\n", port);

    while (1)
    {
        // accept the connection
        new_sock = accept(server_socket, (struct sockaddr *)&client_addr, &sin_len);

        if (new_sock < 0)
        {
            perror("Error in accepting the connection\n");
            exit(1);
        }

        // identify the client
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // make forks for concurrent connections
        if (fork() == 0)
        {
            close(server_socket);

            // send the welcome message
            char *welcome = "+OK POP3 server ready\r\n";
            send(new_sock, welcome, strlen(welcome), 0);

            memset(buf, 0, sizeof(buf));

            // receive the username
            recv(new_sock, buf, sizeof(buf), 0);
            printf("%s", buf);

            // remove the USER and \r\n from the username
            buf[strlen(buf) - 2] = '\0';

            // check first 4 characters
            if (strncmp(buf, "USER", 4) != 0)
            {
                char *invalid_msg = "-ERR Invalid command\r\n";
                send(new_sock, invalid_msg, strlen(invalid_msg), 0);
                close(new_sock);
                exit(0);
            }

            // now extract the username

            // go to first non space character
            int i = 4;
            while (buf[i] == ' ')
            {
                i++;
            }

            char uname[100];
            strcpy(uname, buf + i);

            // check if username is valid from the file user.txt
            FILE *file = fopen("user.txt", "r");
            char username[100];
            char password[100];
            int valid = 0;

            while (fscanf(file, "%s %s", username, password) != EOF)
            {
                if (strncmp(username, uname, strlen(username)) == 0)
                {
                    valid = 1;
                    break;
                }
            }

            fclose(file);

            if (valid)
            {
                char *valid_msg = "+OK User exists\r\n";
                send(new_sock, valid_msg, strlen(valid_msg), 0);
            }
            else
            {
                char *invalid_msg = "-ERR User does not exist\r\n";
                send(new_sock, invalid_msg, strlen(invalid_msg), 0);
                close(new_sock);
                exit(0);
            }

            memset(buf, 0, sizeof(buf));

            // now receive the password
            // as we have already received the username and the corresponding password is in the file user.txt
            recv(new_sock, buf, sizeof(buf), 0);
            buf[strlen(buf) - 2] = '\0';

            // check first 4 characters
            if (strncmp(buf, "PASS", 4) != 0)
            {
                char *invalid_msg = "-ERR Invalid command\r\n";
                send(new_sock, invalid_msg, strlen(invalid_msg), 0);
                close(new_sock);
                exit(0);
            }

            printf("%s\n", buf);

            // now extract the password

            // go to first non space character
            i = 4;
            while (buf[i] == ' ')
            {
                i++;
            }

            char pass[100];
            strcpy(pass, buf + i);
            FILE *mailbox;

            int mark[1000] = {0};
            char mailboxpath[200];

            if (strcmp(pass, password) == 0)
            {
                // go to mailbox and check for emails and octets
                // char *valid_msg = "+OK";

                // open the mailbox file

                snprintf(mailboxpath, sizeof(mailboxpath), "%s/mailbox", username);
                mailbox = fopen(mailboxpath, "r");

                // lock the mailbox
                // if (flock(fileno(mailbox), LOCK_EX | LOCK_NB) == -1)
                // {
                //     perror("Error in locking the mailbox\n");
                //     close(new_sock);
                //     exit(1);
                // }

                // count the number of emails by tracking \r\n.\r\n
                int emails = 0;
                int chars = 0;
                char blah[100];
                while (fgets(blah, sizeof(blah), mailbox))
                {
                    if (strncmp(blah, ".\r\n", 2) == 0)
                    {
                        emails++;
                    }
                    chars += strlen(blah);
                }
                int octets = chars / 8;

                char msg[200];
                snprintf(msg, sizeof(msg), "+OK %s's maildrop has %d messages (%d octets)\r\n", username, emails, octets);
                send(new_sock, msg, strlen(msg), 0);
            }
            else
            {
                char *invalid_msg = "-ERR Password incorrect\r\n";
                send(new_sock, invalid_msg, strlen(invalid_msg), 0);
                close(new_sock);
                exit(0);
            }
            int len;

            int s;
            while (1)
            {
                // recv command
                memset(buf, 0, sizeof(buf));
                len = recv(new_sock, buf, sizeof(buf), 0);
                printf("%s", buf);

                if (strncmp(buf, "STAT", 4) == 0)
                {
                    // count the number of emails by tracking \r\n.\r\n
                    int emails = 0;
                    int chars = 0;
                    char blah[100];
                    fseek(mailbox, 0, SEEK_SET);
                    while (fgets(blah, sizeof(blah), mailbox))
                    {
                        if (strncmp(blah, ".\r\n", 2) == 0)
                        {
                            emails++;
                        }
                        chars += strlen(blah);
                    }
                    int octets = chars / 8;

                    char msg[200];
                    snprintf(msg, sizeof(msg), "+OK %d %d\r\n", emails, octets);
                    send(new_sock, msg, strlen(msg), 0);
                    s = emails;
                }

                else if (strncmp(buf, "LIST", 4) == 0)
                {

                    char list_msg[1000];
                    strcpy(list_msg, buf);

                    fseek(mailbox, 0, SEEK_SET);
                    int emails = 0;
                    int chars = 0;

                    int c = 1;
                    int mail_char = 0;
                    int char_count[1000] = {0};

                    while (fgets(buf, sizeof(buf), mailbox))
                    {
                        if (mark[c] == 0)
                        {
                            mail_char += strlen(buf);
                        }
                        if (strncmp(buf, ".\r\n", 3) == 0 && mark[c] == 0)
                        {
                            char_count[c] = mail_char;
                            mail_char = 0;
                            emails++;
                            chars += char_count[c];
                            c++;
                        }
                        else if (mark[c] == 1 && strncmp(buf, ".\r\n", 3) == 0)
                        {
                            c++;
                            mail_char = 0;
                        }
                    }

                    // check if list_msg has any additional parameters
                    if (strlen(list_msg) == 6)
                    {
                        char msg[200];
                        snprintf(msg, sizeof(msg), "+OK %d messages (%d octets)\r\n", emails, chars / 8);
                        send(new_sock, msg, strlen(msg), 0);
                        for (int i = 1; i <= c; i++)
                        {
                            if (mark[i] == 0)
                            {
                                char msg[200];
                                snprintf(msg, sizeof(msg), "%d %d\r\n", i, char_count[i] / 8);
                                send(new_sock, msg, strlen(msg), 0);
                            }
                        }
                        send(new_sock, ".\r\n", 3, 0);
                    }
                    else
                    {
                        int mailno;
                        sscanf(list_msg, "LIST %d", &mailno);

                        if (mailno > s || mailno < 1 || mark[mailno])
                        {
                            char invalid_msg[100];
                            snprintf(invalid_msg, sizeof(invalid_msg), "-ERR no such message, only %d messages in maildrop\r\n.\r\n", mailno);
                            send(new_sock, invalid_msg, strlen(invalid_msg), 0);
                            continue;
                        }
                        else
                        {
                            char msg[200];
                            snprintf(msg, sizeof(msg), "+OK %d %d\r\n", mailno, char_count[mailno] / 8);
                            send(new_sock, msg, strlen(msg), 0);
                        }
                    }
                }
                else if (strncmp(buf, "RETR", 4) == 0)
                {
                    // extract the mail number
                    int mailno;
                    int flag = 0;
                    while (1)
                    {
                        sscanf(buf, "RETR %d\r\n", &mailno);

                        // check the range of the mail
                        if (mailno > s || mailno < 1)
                        {
                            char *invalid_msg = "-ERR Invalid mail number or no mails are there\r\n.\r\n";
                            send(new_sock, invalid_msg, strlen(invalid_msg), 0);
                            flag = 1;
                            break;
                        }
                        else if (mark[mailno])
                        {
                            char *invalid_msg = "-ERR message %d already deleted\r\n.\r\n";
                            send(new_sock, invalid_msg, strlen(invalid_msg), 0);
                            flag = 1;
                            break;
                        }
                        else
                            break;
                    }
                    if (flag)
                        continue;

                    // send ok along with the mail
                    char msg[200];
                    memset(msg, 0, sizeof(msg));
                    // find the mail in the mailbox
                    fseek(mailbox, 0, SEEK_SET);
                    int c = 1;
                    int sz = 0;
                    char mail[4096];
                    memset(mail, 0, sizeof(mail));
                    while (fgets(buf, sizeof(buf), mailbox))
                    {
                        if (c == mailno)
                        {
                            sz += strlen(buf);
                            strcat(mail, buf);
                        }
                        if (strncmp(buf, ".\r\n", 3) == 0)
                        {
                            c++;
                        }
                    }
                    sz /= 8;
                    snprintf(msg, sizeof(msg), "+OK %d octets\r\n", sz);
                    send(new_sock, msg, strlen(msg), 0);
                    send(new_sock, mail, strlen(mail), 0);
                    memset(mail, 0, sizeof(mail));
                    memset(buf, 0, sizeof(buf));
                }

                else if (strncmp(buf, "DELE", 4) == 0)
                {
                    // get the mailno
                    int mailno;
                    int flag = 0;
                    char msg[200];

                    sscanf(buf, "DELE %d", &mailno);

                    if (mark[mailno])
                    {
                        snprintf(msg, sizeof(msg), "-ERR message %d already deleted\r\n", mailno);
                        send(new_sock, msg, strlen(msg), 0);
                        continue;
                    }

                    mark[mailno] = 1;
                    snprintf(msg, sizeof(msg), "+OK message %d deleted\r\n", mailno);

                    send(new_sock, msg, strlen(msg), 0);
                }
                else if (strncmp(buf, "RSET", 4) == 0)
                {
                    memset(mark, 0, sizeof(mark));
                }

                else if (strncmp(buf, "QUIT", 4) == 0)
                {
                    char *quit_msg = "+OK POP3 server signing off\r\n";
                    send(new_sock, quit_msg, strlen(quit_msg), 0);

                    fseek(mailbox, 0, SEEK_SET);
                    FILE *temp = fopen("temp", "w");
                    int c = 1;
                    while (fgets(buf, sizeof(buf), mailbox))
                    {
                        if (mark[c] == 0)
                        {
                            fputs(buf, temp);
                        }
                        if (strncmp(buf, ".\r\n", 3) == 0)
                        {
                            c++;
                        }
                    }

                    // unlock file
                    // if (flock(fileno(mailbox), LOCK_UN) == -1)
                    // {
                    //     perror("Error in unlocking the mailbox\n");
                    //     close(new_sock);
                    //     exit(1);
                    // }

                    // close the mailbox
                    fclose(mailbox);

                    // delete mailbox file
                    remove(mailboxpath);

                    // rename temp to mailbox
                    rename("temp", mailboxpath);

                    close(new_sock);
                    break;
                }
            }

            close(new_sock);
            printf("Connection closed\n");
            exit(0);
        }

        close(new_sock);
    }
}