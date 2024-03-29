#include "msocket.h"

int m_socket(int domain, int type, int protocol)
{
    // check if the type is SOCK_MTP
    if (type != SOCK_MTP)
    {
        // set errno to EPROTOTYPE
        errno = EPROTOTYPE;
        return -1;
    }

    struct sembuf sem_lock = {0, -1, 0};
    struct sembuf sem_unlock = {0, 1, 0};

    // get the semaphore for the shared memory
    key_t sem_key = ftok("initmsocket.c", 1);
    int sem_id = semget(sem_key, 1, 0666 | IPC_CREAT);

    // get the shared memory
    key_t key = ftok("initmsocket.c", 2);
    int sm_id = shmget(key, sizeof(SM) * 25, 0666 | IPC_CREAT);

    // attach the shared memory to the process
    SM *sm = (SM *)shmat(sm_id, NULL, 0);

    // shmget sem1 sem2
    key_t sockinfo = ftok("initmsocket.c", 3);
    int sockinfo_id = shmget(sockinfo, sizeof(SOCK_INFO), 0666 | IPC_CREAT);

    // attach the shared memory to the process
    SOCK_INFO *si = (SOCK_INFO *)shmat(sockinfo_id, NULL, 0);

    // get the semaphore for the shared memory
    key_t sem_key1 = ftok("initmsocket.c", 4);
    int sem_id1 = semget(sem_key1, 1, 0666 | IPC_CREAT);
    key_t sem_key2 = ftok("initmsocket.c", 5);
    int sem_id2 = semget(sem_key2, 1, 0666 | IPC_CREAT);

    P(sem_id);
    // find the first free SM
    int i;
    for (i = 0; i < 25; i++)
    {
        if (sm[i].alloted == 0 && sm[i].udp_id == 0)
        {
            sm[i].alloted = 1;
            break;
        }
    }

    if (i >= 25)
    {

        // set errno to ENOBUFS
        errno = ENOBUFS;
        V(sem_id);
        return -1;
    }

    // create a udp socket

    V(sem_id1);
    P(sem_id2);
    if (si->sock_id == -1)
    {
        errno = si->errnum;
        memset(si, 0, sizeof(SOCK_INFO));
        V(sem_id);
        return -1;
    }

    sm[i].udp_id = si->sock_id;

    memset(si, 0, sizeof(SOCK_INFO));

    // set the mtp_id in the SM
    sm[i].mtp_id = i + 1;
    sm[i].pid = getpid();

    // set recieve buffer to \0
    for (int j = 0; j < 5; j++)
        memset(sm[i].recvbuffer[j].text, '\0', 1024);

    // set the window size
    sm[i].swnd.window_size = 5;
    sm[i].swnd.left = -1;
    sm[i].swnd.middle = 0;
    sm[i].swnd.right = 0;

    sm[i].rwnd.window_size = 5;
    sm[i].rwnd.left = 0;
    sm[i].rwnd.middle = 0;
    sm[i].rwnd.right = 5;

    // set the sendand receive buffer to variables
    sm[i].sendbuffer_in = -1;
    sm[i].sendbuffer_out = -1;

    sm[i].exp_seq = 0;

    sm[i].last_seq = -1;

    for (int p = 0; p < 15; p++)
    {
        sm[i].rwnd.array[p] = -1;
    }

    sm[i].transmission_cnt = 0;
    V(sem_id);

    // detach the shared memory
    shmdt(sm);

    return i + 1;
}

int m_bind(int sock, unsigned long s_ip, int s_port, unsigned long d_ip, int d_port)
{
    struct sembuf sem_lock = {0, -1, 0};
    struct sembuf sem_unlock = {0, 1, 0};

    // get the semaphore for the shared memory
    key_t sem_key = ftok("initmsocket.c", 1);
    int sem_id = semget(sem_key, 1, 0666 | IPC_CREAT);

    // get the shared memory
    key_t key = ftok("initmsocket.c", 2);
    int sm_id = shmget(key, sizeof(SM) * 25, 0666 | IPC_CREAT);

    // attach the shared memory to the process
    SM *sm = (SM *)shmat(sm_id, NULL, 0);

    P(sem_id);

    // find the SM with the given sock
    int i;
    for (i = 0; i < 25; i++)
    {
        if (sm[i].mtp_id == sock)
            break;
    }

    if (i >= 25)
    {
        // set errno to EBADF
        errno = EBADF;
        V(sem_id);
        return -1;
    }

    // get the shared memory of sockinfo
    key_t sockinfo = ftok("initmsocket.c", 3);
    int sockinfo_id = shmget(sockinfo, sizeof(SOCK_INFO), 0666 | IPC_CREAT);

    // attach the shared memory to the process
    SOCK_INFO *si = (SOCK_INFO *)shmat(sockinfo_id, NULL, 0);

    // get the semaphore for the shared memory
    key_t sem_key1 = ftok("initmsocket.c", 4);
    int sem_id1 = semget(sem_key1, 1, 0666 | IPC_CREAT);
    key_t sem_key2 = ftok("initmsocket.c", 5);
    int sem_id2 = semget(sem_key2, 1, 0666 | IPC_CREAT);

    si->ip = s_ip;
    si->port = s_port;
    si->sock_id = sm[i].udp_id;

    V(sem_id1);
    P(sem_id2);
    if (si->sock_id == -1)
    {
        errno = si->errnum;
        memset(si, 0, sizeof(SOCK_INFO));
        V(sem_id);
        return -1;
    }

    memset(si, 0, sizeof(SOCK_INFO));

    sm[i].ip = d_ip;
    sm[i].port = d_port;

    V(sem_id);

    // detach the shared memory
    shmdt(sm);

    return 0;
}

int m_sendto(int sock, char *buf, int len, int flags, unsigned long d_ip, int d_port)
{
    struct sembuf sem_lock = {0, -1, 0};
    struct sembuf sem_unlock = {0, 1, 0};

    // get the semaphore for the shared memory
    key_t sem_key = ftok("initmsocket.c", 1);
    int sem_id = semget(sem_key, 1, 0666 | IPC_CREAT);

    // get the shared memory
    key_t key = ftok("initmsocket.c", 2);
    int sm_id = shmget(key, sizeof(SM) * 25, 0666 | IPC_CREAT);

    // attach the shared memory to the process
    SM *sm = (SM *)shmat(sm_id, NULL, 0);

    P(sem_id);

    // find the SM with the given sock
    int i;
    for (i = 0; i < 25; i++)
    {
        if (sm[i].mtp_id == sock)
            break;
    }

    if (i >= 25)
    {
        // set errno to EBADF
        errno = EBADF;
        V(sem_id);
        return -1;
    }

    char msg[1024];
    memset(msg, '\0', 1024);
    strcpy(msg, buf);

    // pad the message with \0
    for (int j = strlen(buf); j < 1024; j++)
        msg[j] = '\0';

    // check is the d_ip and d_port are valid
    if ((sm[i].ip != d_ip) || (sm[i].port != d_port))
    {
        errno = ENOTCONN;
        V(sem_id);
        shmdt(sm);
        return -1;
    }

    // check if the send buffer is full
    if ((sm[i].sendbuffer_in + 1) % 10 == sm[i].sendbuffer_out)
    {
        errno = ENOBUFS;
        V(sem_id);
        shmdt(sm);
        return -1;
    }

    if (sm[i].sendbuffer_in == -1 && sm[i].sendbuffer_out == -1)
    {
        sm[i].last_seq = (sm[i].last_seq + 1) % 15;
        sm[i].sendbuffer_in = 0;
        sm[i].sendbuffer_out = 0;
        strcpy(sm[i].sendbuffer[sm[i].sendbuffer_in].text, msg);
        sm[i].swnd.array[sm[i].last_seq] = sm[i].sendbuffer_in;
    }
    else
    {
        // check if that place is already filled
        // if (strncmp(sm[i].sendbuffer[(sm[i].sendbuffer_in + 1)%10].text, "\0", 1) != 0)
        // {
        //     errno = ENOBUFS;
        //     V(sem_id);
        //     shmdt(sm);
        //     return -1;
        // }
        sm[i].last_seq = (sm[i].last_seq + 1) % 15;
        sm[i].sendbuffer_in = (sm[i].sendbuffer_in + 1) % 10;
        strcpy(sm[i].sendbuffer[sm[i].sendbuffer_in].text, msg);
        sm[i].swnd.array[sm[i].last_seq] = sm[i].sendbuffer_in;
    }

    V(sem_id);

    // detach the shared memory
    shmdt(sm);

    return len;
}

int m_recvfrom(int sock, char *buf, int len, int flags, unsigned long s_ip, int s_port)
{
    struct sembuf sem_lock = {0, -1, 0};
    struct sembuf sem_unlock = {0, 1, 0};

    // get the semaphore for the shared memory
    key_t sem_key = ftok("initmsocket.c", 1);
    int sem_id = semget(sem_key, 1, 0666 | IPC_CREAT);

    // get the shared memory
    key_t key = ftok("initmsocket.c", 2);
    int sm_id = shmget(key, sizeof(SM) * 25, 0666 | IPC_CREAT);

    // attach the shared memory to the process
    SM *sm = (SM *)shmat(sm_id, NULL, 0);

    P(sem_id);

    // find the SM with the given sock
    int i;
    for (i = 0; i < 25; i++)
    {
        if (sm[i].mtp_id == sock)
            break;
    }

    if (i >= 25)
    {
        // set errno to EBADF
        errno = EBADF;
        V(sem_id);
        return -1;
    }

    // check is the s_ip and s_port are valid
    if (sm[i].ip != s_ip || sm[i].port != s_port)
    {
        errno = ENOTCONN;
        V(sem_id);
        shmdt(sm);
        return -1;
    }

    // check if there is something to receive inorder
    // this msg would be in recvbuffer[exp_seq%5]


    if (strncmp(sm[i].recvbuffer[sm[i].exp_seq % 5].text, "\0", 1) == 0)
    {
        errno = ENOMSG;
        V(sem_id);
        shmdt(sm);
        return -1;
    }
    // memset(buf, '\0', 1024);
    // copy the message to the buffer
    strcpy(buf, sm[i].recvbuffer[sm[i].exp_seq % 5].text);
    sm[i].rwnd.array[sm[i].exp_seq] = -1;

    memset(sm[i].recvbuffer[sm[i].exp_seq % 5].text, '\0', 1024);
    sm[i].exp_seq = (sm[i].exp_seq + 1) % 15;
    if (sm[i].nospace == 1)
    {
        sm[i].nospace = 0;
        sm[i].flag = 1;
    }
    sm[i].rwnd.left = (sm[i].rwnd.left + 1) % 15;
    sm[i].rwnd.right = (sm[i].rwnd.right + 1) % 15;
    V(sem_id);

    // detach the shared memory
    shmdt(sm);

    return strlen(buf);
}

int m_close(int sock)
{
    struct sembuf sem_lock = {0, -1, 0};
    struct sembuf sem_unlock = {0, 1, 0};
    
    // get the semaphore for the shared memory
    key_t sem_key = ftok("initmsocket.c", 1);
    int sem_id = semget(sem_key, 1, 0666 | IPC_CREAT);

    // get the shared memory
    key_t key = ftok("initmsocket.c", 2);
    int sm_id = shmget(key, sizeof(SM) * 25, 0666 | IPC_CREAT);

    // attach the shared memory to the process
    SM *sm = (SM *)shmat(sm_id, NULL, 0);

    P(sem_id);

    // find the SM with the given sock
    int i;
    for (i = 0; i < 25; i++)
    {
        if (sm[i].mtp_id == sock)
            break;
    }

    if (i >= 25)
    {
        // set errno to EBADF
        errno = EBADF;
        V(sem_id);
        return -1;
    }

    memset(&sm[i], 0, sizeof(SM));

    V(sem_id);

    // detach the shared memory
    shmdt(sm);

    return 0;
}

int dropMessage(float p)
{

    // generate a random number between 0 and 1, if it is less than p, return 1, else return 0
    double r = ((double)rand()) / INT_MAX;
    // printf("r = %lf p = %lf\n", r, p);
    if (r < p)
        return 1;
    else
        return 0;
}