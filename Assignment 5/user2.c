#include "msocket.h"

int main()
{
    int s = m_socket(AF_INET, SOCK_MTP, 0);
    if (s < 0)
    {
        perror("socket error\n");
        return -1;
    }
    printf("socket created\n");
    printf("socket id = %d\n", s);

    long s_ip = inet_addr("127.0.0.1");
    int s_port = htons(1237);
    long d_ip = inet_addr("127.0.0.1");
    int d_port = htons(1236);

    int ret = m_bind(s, s_ip, s_port, d_ip, d_port);
    if (ret < 0)
    {
        perror("bind error\n");
        return -1;
    }

    printf("bind successful with s_ip = %ld, s_port = %d\n", s_ip, s_port);

    char buf[1024];

    int fd = open("juliet.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    while (1)
    {
        usleep(100000);
        ret = m_recvfrom(s, buf, 1024, 0, d_ip, d_port);
        while (ret < 0)
        {
            perror("recvfrom error");
            usleep(100000);
            ret = m_recvfrom(s, buf, 1024, 0, d_ip, d_port);
        }

        // printf("message received\n");
        // printf("message = %s\n", buf);

        // check if the message is terminal message
        if (strcmp(buf, "##########") == 0)
        {
            break;
        }

        write(fd, buf, strlen(buf));

        printf("message received\n");

        memset(buf, '\0', sizeof(buf));
    }


    // ret = m_close(s);

    // if (ret < 0)
    // {
    //     perror("close error\n");
    //     return -1;
    // }

    // printf("socket closed\n");

    return 0;
}