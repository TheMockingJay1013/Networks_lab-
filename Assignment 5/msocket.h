#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>

// #include "struct.h"

#define SOCK_MTP 2000
#define T 5
#define P_val 0.05


#define P(s) semop(s, &sem_lock, 1)
#define V(s) semop(s, &sem_unlock, 1)


typedef struct
{
    int sock_id;
    long ip;
    int port;
    int errnum;
} SOCK_INFO;

typedef struct
{
    char text[1024];
} msg;

typedef struct
{
    int window_size; // useful for the sender to know how much space is there in receiver buffer
    int array[15];
    int left;
    int middle;
    int right;
} window;

typedef struct
{
    int alloted;
    pid_t pid;
    int mtp_id;
    int udp_id;
    unsigned long ip;
    int port;
    msg sendbuffer[10];
    int last_seq;
    int sendbuffer_in;
    int sendbuffer_out;
    msg recvbuffer[5];
    int recvbuffer_in;
    int recvbuffer_out;
    window swnd;
    window rwnd;
    int nospace;
    int flag;
    int exp_seq;
    int transmission_cnt;
} SM;


int m_socket(int domain, int type, int protocol);
int m_bind(int sock,unsigned long s_ip, int s_port,unsigned long d_ip, int d_port);
int m_sendto(int sock, char *buf, int len, int flags,unsigned long d_ip, int d_port);
int m_recvfrom(int sock, char *buf, int len, int flags,unsigned long s_ip, int s_port);
int m_close(int sock);

int dropMessage(float p);
