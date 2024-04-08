#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <limits.h>
#include <net/if.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#define BUFFER_SIZE 65536

typedef struct page_table_entry
{
    int alloted;
    char query[1000];
    int id;
    int tries;
} page_table_entry;

page_table_entry page_table[20];

int sockfd;

// signal handler for SIGINT and SIGTSTP
void sighand(int signum)
{
    if (signum == SIGINT)
    {
        printf("\nYou pressed Ctrl+C, Exiting\n");
        close(sockfd);
        exit(1);
    }
    else if (signum == SIGTSTP)
    {
        printf("\nYou pressed Ctrl+Z, Exiting\n");
        close(sockfd);
        exit(1);
    }
}

int main()
{
    signal(SIGINT, sighand);
    signal(SIGTSTP, sighand);
    srand(time(0));
    memset(page_table, 0, sizeof(page_table));
    struct sockaddr_ll sa;
    char sendbuffer[BUFFER_SIZE] = {'\0'};
    char recvbuffer[BUFFER_SIZE] = {'\0'};
    ssize_t packet_len;

    // Create a raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // make the socket non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    printf("Specify the interface to sniff on: (eth0|lo|eno1): ");
    char interface[10]={'\0'};
    scanf("%s", interface);

    // Set up the sockaddr_ll structure
    memset(&sa, 0, sizeof(struct sockaddr_ll));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ALL);
    sa.sll_ifindex = if_nametoindex(interface);
    if (sa.sll_ifindex == 0)
    {
        perror("if_nametoindex");
        exit(EXIT_FAILURE);
    }

    

    printf("Enter the MAC address of the Client: (xx:xx:xx:xx:xx:xx): ");
    unsigned char client_mac[6];
    scanf("%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &client_mac[0], &client_mac[1], &client_mac[2], &client_mac[3], &client_mac[4], &client_mac[5]);

    printf("Enter the IP address of the Client: (x.x.x.x): ");
    char client_ip[INET_ADDRSTRLEN];
    scanf("%s", client_ip);

    long client_ip_addr;
    inet_pton(AF_INET, client_ip, &client_ip_addr);

    printf("Enter the MAC address of the Server: (xx:xx:xx:xx:xx:xx): ");
    unsigned char server_mac[6];
    scanf("%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &server_mac[0], &server_mac[1], &server_mac[2], &server_mac[3], &server_mac[4], &server_mac[5]);

    printf("Enter the IP address of the Server: (x.x.x.x): ");
    char server_ip[INET_ADDRSTRLEN];
    scanf("%s", server_ip);

    long server_ip_addr;
    inet_pton(AF_INET, server_ip, &server_ip_addr);

    

    char simDNSquery[1000] = {'\0'};

    int id = 1001;

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(sockfd, &fd);
    FD_SET(0, &fd);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    printf("Enter the query: (getIP N <domain-1> <domain-2> <domain-3> … <domain-N>) or EXIT\n");

    while (1)
    {
        fd_set read_fd = fd;
        int ret = select(sockfd + 1, &read_fd, NULL, NULL, &tv);

        if (ret == -1)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        else if (ret == 0)
        {

            // reset the timer
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            for (int i = 0; i < 20; i++)
            {
                if (page_table[i].alloted == 1)
                {
                    if (page_table[i].tries == 4)
                    {
                        // delete the entry from the page table

                        // get the id
                        int qid = 0;
                        for (int j = 0; j < 16; j++)
                        {
                            qid = qid * 2 + (page_table[i].query[j] - '0');
                        }
                        printf("\nQuery ID: %d timed out\n\n\n", qid);

                        memset(page_table[i].query, 0, sizeof(page_table[i].query));
                        page_table[i].alloted = 0;
                        page_table[i].id = 0;
                        page_table[i].tries = 0;
                    }
                    else
                    {
                        // we can retransmit this query

                        memset(sendbuffer, 0, sizeof(sendbuffer));

                        // creating the Ethernet header
                        struct ethhdr *eth = (struct ethhdr *)sendbuffer;
                        // set the destination MAC address in mac
                        memcpy(eth->h_dest, server_mac, ETH_ALEN); // Destination MAC address
                        memcpy(eth->h_source, client_mac, ETH_ALEN);
                        eth->h_proto = htons(ETH_P_IP); // EtherType for IP

                        // creating the IP header
                        struct iphdr *ip = (struct iphdr *)(sendbuffer + sizeof(struct ethhdr));
                        ip->ihl = 5;
                        ip->version = 4;
                        ip->tos = 0;
                        ip->tot_len = htons(sizeof(struct iphdr) + strlen(page_table[i].query));
                        ip->id = rand() % 10000;
                        ip->frag_off = 0;
                        ip->ttl = 8;
                        ip->protocol = 254;
                        ip->check = 0;
                        ip->saddr = client_ip_addr;
                        ip->daddr = server_ip_addr;

                        // adding the query to the buffer
                        char *data = (char *)(sendbuffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
                        strcpy(data, page_table[i].query);

                        // sending the packet
                        printf("Retransmitting packet with Query id: %d\n", page_table[i].id);
                        int len = sendto(sockfd, sendbuffer, sizeof(struct ethhdr) + sizeof(struct iphdr) + strlen(page_table[i].query), 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));

                        if (len == -1)
                        {
                            perror("sendto");
                            exit(EXIT_FAILURE);
                        }

                        page_table[i].tries++;
                    }
                }
            }
        }

        else
        {
            if (FD_ISSET(0, &read_fd))
            {
                // asking user for query input
                char query[1000] = {'\0'};

                scanf("\n%[^\n]", query);
                fflush(stdin);

                if (strncmp(query, "EXIT", 4) == 0)
                {
                    // close the socket and exit
                    close(sockfd);
                    return 0;
                }

                // checking if the query is valid
                if (strncmp(query, "getIP", 5) != 0)
                {
                    printf("Invalid format\n");
                    memset(query, '\0', sizeof(query));
                    continue;
                }

                // checking value of N
                int n;
                sscanf(query, "getIP %d", &n);

                if (n < 1)
                {
                    printf("Invalid format\n");
                    memset(query, '\0', sizeof(query));
                    continue;
                }
                if (n > 8)
                {
                    printf("N should be less than or equal to 8\n");
                    memset(query, '\0', sizeof(query));
                    continue;
                }

                // checking if actual number of domains are equal to N
                int count = 0;
                for (int i = 7; i < strlen(query) - 1; i++)
                {
                    if (query[i] == ' ' && query[i + 1] != ' ')
                    {
                        count++;
                    }
                }

                if (count != n)
                {
                    printf("Number of domains should be equal to %d\n", n);
                    continue;
                }

                // check if the has only alphanumeric characters and dots and hyphens
                int flag = 0;

                int p = 7;
                int cnt = 0;

                for (int i = 7; i < strlen(query) + 1; i++)
                {
                    if ((query[i] >= 'a' && query[i] <= 'z') || (query[i] >= 'A' && query[i] <= 'Z') || (query[i] >= '0' && query[i] <= '9') || query[i] == '.' || query[i] == '-' || query[i] == ' ' || query[i] == '\n' || query[i] == '\0')
                    {
                        if (query[i] == ' ' || query[i] == '\n' || query[i] == '\0')
                        {
                            if (cnt > 31 || cnt == 1 || cnt == 2)
                            {
                                flag = 1;
                                break;
                            }
                            cnt = 0;
                        }
                        else
                        {
                            cnt++;
                        }
                        if (query[i] == '-')
                        {
                            if (i == 0)
                            {
                                flag = 1;
                                break;
                            }
                            else
                            {
                                if (query[i - 1] == '-' || query[i - 1] == ' ')
                                {
                                    flag = 1;
                                    break;
                                }
                            }
                        }
                        continue;
                    }
                    else
                    {
                        flag = 1;
                        break;
                    }
                }

                if (flag == 1)
                {
                    printf("Invalid domain name\n");
                    memset(query, '\0', sizeof(query));
                    continue;
                }

                int j = 15;
                // fill the first 16 char with the id
                int tempid = id;
                while (j >= 0)
                {
                    simDNSquery[j] = tempid % 2 + '0';
                    tempid /= 2;
                    j--;
                }
                // it is a query message
                simDNSquery[16] = '0';

                j = 19;
                int temp = n - 1;
                while (j > 16)
                {
                    simDNSquery[j] = '0' + temp % 2;
                    temp /= 2;
                    j--;
                }
                j = 20;
                // find length of each domain and the domain
                int t = 7;
                while (n--)
                {
                    j--;
                    while (query[t] == ' ' || query[t] == '\0')
                    {
                        t++;
                    }
                    int len = 0;
                    int start = t;

                    while (query[t] != ' ' && query[t] != '\0')
                    {
                        len++;
                        t++;
                    }

                    // put the len in the 4 bits from j+1 to j+5
                    int temp = len;
                    int m = 5;
                    while (m)
                    {
                        simDNSquery[j + m] = '0' + temp % 2;
                        temp /= 2;
                        m--;
                    }

                    j = j + 6;

                    // put the domain in the message
                    for (int i = start; i < start + len; i++)
                    {
                        simDNSquery[j] = query[i];
                        j++;
                    }
                }

                // creating the Ethernet header
                struct ethhdr *eth = (struct ethhdr *)sendbuffer;
                memcpy(eth->h_dest, server_mac, ETH_ALEN);   // Destination MAC address
                memcpy(eth->h_source, client_mac, ETH_ALEN); // Source MAC address
                eth->h_proto = htons(ETH_P_IP);              // EtherType for IP

                // creating the IP header
                struct iphdr *ip = (struct iphdr *)(sendbuffer + sizeof(struct ethhdr));
                ip->ihl = 5;
                ip->version = 4;
                ip->tos = 0;
                ip->tot_len = htons(sizeof(struct iphdr) + strlen(simDNSquery));
                ip->id = rand() % 10000;
                ip->frag_off = 0;
                ip->ttl = 8;
                ip->protocol = 254;
                ip->check = 0;
                ip->saddr = client_ip_addr;
                ip->daddr = server_ip_addr;

                // adding the simDNSquery to the buffer
                char *data = (char *)(sendbuffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
                strcpy(data, simDNSquery);

                // sending the packet
                int len = sendto(sockfd, sendbuffer, sizeof(struct ethhdr) + sizeof(struct iphdr) + strlen(simDNSquery), 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));

                if (len == -1)
                {
                    perror("sendto");
                    exit(EXIT_FAILURE);
                }

                // add it in the page query table
                // find the first empty slot
                t = 0;
                while (page_table[t].alloted == 1)
                {
                    t++;
                }
                if (t >= 20)
                {
                    printf("No empty slot in the page table\n");
                    memset(query, '\0', sizeof(query));
                    continue;
                }

                page_table[t].alloted = 1;
                strcpy(page_table[t].query, simDNSquery);
                page_table[t].id = id;
                page_table[t].tries = 1;

                id++;
            }

            if (FD_ISSET(sockfd, &read_fd))
            {
                while (1)
                {
                    // received a reponse
                    int len = recvfrom(sockfd, recvbuffer, BUFFER_SIZE, 0, NULL, NULL);

                    if (len == -1)
                    {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
                        {
                            break;
                        }
                        else
                        {
                            perror("recvfrom");
                            exit(EXIT_FAILURE);
                        }
                    }

                    // check the source ip address
                    struct iphdr *ip = (struct iphdr *)(recvbuffer + sizeof(struct ethhdr));
                    char *data = (char *)(recvbuffer + sizeof(struct ethhdr) + sizeof(struct iphdr));

                    if (ip->saddr != server_ip_addr)
                    {
                        // printf("Invalid source IP address\n");
                        continue;
                    }

                    // check the destination ip address
                    if (ip->daddr != client_ip_addr)
                    {
                        // printf("Invalid destination IP address\n");
                        continue;
                    }

                    // check if ip protocol is 254
                    if (ip->protocol != 254)
                    {
                        // printf("Invalid protocol\n");
                        continue;
                    }

                    // check if the message is a response
                    if (data[16] != '1')
                    {
                        // printf("Not a query response packet\n");
                        continue;
                    }

                    // printf("Received data : %s\n", data);

                    // check the id
                    int responseid = 0;
                    for (int i = 0; i < 16; i++)
                    {
                        responseid = responseid * 2 + (data[i] - '0');
                    }


                    // check if the id is valid in the table
                    int t = 0;
                    while (t < 20)
                    {
                        // printf("%d %d %d\n", t, page_table[t].alloted, page_table[t].id);
                        if (page_table[t].alloted == 1 && page_table[t].id == responseid)
                        {
                            break;
                        }
                        t++;
                    }

                    if (t >= 20)
                    {
                        // printf("Invalid id\n");
                        continue;
                    }

                    printf("\nQuery ID: %d\n", responseid);

                    // check the number of ip addresses
                    int n = 0;
                    n = n * 2 + (data[17] - '0');
                    n = n * 2 + (data[18] - '0');
                    n = n * 2 + (data[19] - '0');
                    n++;

                    printf("Total query strings: %d\n\n", n);

                    int j = 20;
                    int k = 20;
                    int length;

                    while (n--)
                    {
                        length = 0;
                        // store the value in bits j to j+4 in len
                        for (int r = 0; r < 5; r++)
                        {
                            length = length * 2 + page_table[t].query[j] - '0';
                            j++;
                        }

                        int temp = j + length;
                        while (j < temp)
                        {
                            printf("%c", page_table[t].query[j]);
                            j++;
                        }

                        printf(": ");

                        // get the ip address
                        long ip = 0;

                        // kth bit will tell if the ip is valid or not
                        if (data[k] == '1')
                        {
                            k++;
                            // valid ip
                            for (int r = 0; r < 32; r++)
                            {
                                ip = ip * 2 + data[k] - '0';
                                k++;
                            }
                        }
                        else
                        {
                            printf("No IP address found\n");
                            k += 33;
                            continue;
                        }

                        // print the ip in the required format
                        printf("%ld.%ld.%ld.%ld\n", (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
                    }
                    printf("\n\n");
                    // delete the entry from the page table
                    memset(page_table[t].query, 0, sizeof(page_table[t].query));
                    page_table[t].alloted = 0;
                    page_table[t].id = 0;
                    page_table[t].tries = 0;
                }
            }
        }
    }
}
