#include "msocket.h"

int msgs = 0;
struct sembuf sem_lock = {0, -1, 0};
struct sembuf sem_unlock = {0, 1, 0};


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
    int s_port = htons(1236);
    long d_ip = inet_addr("127.0.0.1");
    int d_port = htons(1237);

    int ret = m_bind(s, s_ip, s_port, d_ip, d_port);
    if (ret < 0)
    {
        perror("bind error\n");
        return -1;
    }
    printf("bind successful with s_ip = %ld, s_port = %d\n", s_ip, s_port);

    sleep(5);

    char buf[1024];

    int fd = open("romeo.txt", O_RDONLY);
    // int df = open("trial.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    while (1)
    {
        int x = read(fd, buf, 1023);
        // write(df, buf, x);
        msgs++;
        if (x == 0)
        {
            break;
        }

        ret = m_sendto(s, buf, strlen(buf)+1, 0, d_ip, d_port);
        usleep(100000);
        while (ret < 0)
        {
            perror("sendto error");
            usleep(100000);
            ret = m_sendto(s, buf, strlen(buf)+1, 0, d_ip, d_port);
        }
        
        // print message sent along with first 8 characters

        printf("message sent\n");

        memset(buf, '\0', sizeof(buf));
    }

    // terminal message
    // 10 # symbols
    strcpy(buf, "##########");

    ret = m_sendto(s, buf, strlen(buf), 0, d_ip, d_port);
    while (ret < 0)
    {
        perror("sendto error");
        usleep(100000);
        ret = m_sendto(s, buf, strlen(buf), 0, d_ip, d_port);
    }
    msgs++;

    // get the semaphore for the shared memory
    key_t sem_key = ftok("initmsocket.c", 1);
    int sem_id = semget(sem_key, 1, 0666 | IPC_CREAT);

    // get the shared memory
    key_t key = ftok("initmsocket.c", 2);
    int sm_id = shmget(key, sizeof(SM) * 25, 0666 | IPC_CREAT);

    // attach the shared memory to the process
    SM *sm = (SM *)shmat(sm_id, NULL, 0);

    sleep(60);

    P(sem_id);
    double avg_transmission_cnt = (1.0*sm[s-1].transmission_cnt) / msgs;
    printf("Average transmission count = %lf\n", avg_transmission_cnt);
    V(sem_id);

    shmdt(sm);

    // ret = m_close(s);
    // if (ret < 0)
    // {
    //     perror("close error\n");
    //     return -1;
    // }
    // printf("socket closed\n");

    // get the shared memory
    // key_t key = ftok("initmsocket.c", 2);
    // int sm_id = shmget(key, sizeof(SM) * 25, 0666|IPC_CREAT);

    // attach the shared memory to the process
    // SM *sm = (SM *)shmat(sm_id, NULL, 0);

    // for(int i=15;i<100;i++)
    // {
    //     printf("%d\n",i);
    //     for(int j=0;j<10;j++)
    //     {
    //         printf("sendbuffer[%d] = %s\n", j, sm[0].sendbuffer[j].text);
    //     }
    //     printf("\n");
    //     sleep(1);
    // }

    return 0;
}