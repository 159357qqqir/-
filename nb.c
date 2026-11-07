#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>

#define UP_RATE     3000000UL
#define BURST       375000UL

static const uint16_t game_udp_ports[] = {
    10001,10002,10003,10005
};
#define PORT_COUNT (sizeof(game_udp_ports)/sizeof(game_udp_ports[0]))

static unsigned long tokens = 0;
static struct timespec last_ts;

static int is_game_port(uint16_t dport)
{
    for(int i = 0; i < PORT_COUNT; i++)
    {
        if(game_udp_ports[i] == dport)
            return 1;
    }
    return 0;
}

static int token_bucket(size_t pkt_len)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long diff_us = (now.tv_sec - last_ts.tv_sec) * 1000000L
                   + (now.tv_nsec - last_ts.tv_nsec) / 1000L;

    tokens += diff_us * UP_RATE / 8000000UL;
    if(tokens > BURST)
        tokens = BURST;
    last_ts = now;

    if(tokens >= pkt_len)
    {
        tokens -= pkt_len;
        return 1;
    }
    return 0;
}

static int nf_callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                       struct nfq_data *nfa, void *data)
{
    uint32_t id;
    struct nfqnl_msg_packet_hdr *ph;
    unsigned char *payload;
    int len;

    ph = nfq_get_msg_packet_hdr(nfa);
    if(!ph) return 0;
    id = ntohl(ph->packet_id);
    len = nfq_get_payload(nfa, &payload);
    if(len <= 0)
    {
        nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        return 0;
    }

    struct iphdr *iph = (struct iphdr *)payload;
    if(iph->protocol == IPPROTO_UDP)
    {
        int iphdr_len = iph->ihl * 4;
        if(len >= iphdr_len + sizeof(struct udphdr))
        {
            struct udphdr *udph = (struct udphdr *)(payload + iphdr_len);
            uint16_t dport = ntohs(udph->dest);
            if(is_game_port(dport))
            {
                printf("BYPASS GAME UDP port=%u len=%d\n", dport, len);
                nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
                return 0;
            }
        }
    }

    uint32_t uid = 0;
    int ret_uid = nfq_get_uid(nfa, &uid);
    if(ret_uid >= 0 && uid == 10287)
    {
        printf("BYPASS UID=10287 len=%d\n", len);
        nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        return 0;
    }

    if(token_bucket(len))
    {
        printf("ACCEPT len=%d\n", len);
        nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
    }
    else
    {
        printf("DROP len=%d\n", len);
        nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
    }
    return 0;
}

int main(void)
{
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    int fd;
    char buf[4096];

    system("iptables -t mangle -F OUTPUT");
    system("iptables -t mangle -A OUTPUT -j NFQUEUE --queue-num 0");

    clock_gettime(CLOCK_MONOTONIC, &last_ts);
    h = nfq_open();
    if(!h) { perror("nfq_open"); return 1; }
    if(nfq_unbind_pf(h, AF_INET) < 0) { perror("unbind"); return 1; }
    if(nfq_bind_pf(h, AF_INET) < 0) { perror("bind"); return 1; }

    qh = nfq_create_queue(h, 0, nf_callback, NULL);
    if(!qh) { perror("nfq_create_queue"); return 1; }
    nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);
    fd = nfq_fd(h);
    printf("Started: UID=10287 + Game UDP port bypass, rate 3Mbps\n");

    while(1)
    {
        int rv = recv(fd, buf, sizeof(buf), 0);
        if(rv >= 0)
            nfq_handle_packet(h, buf, rv);
        usleep(1000);
    }
    nfq_destroy_queue(qh);
    nfq_close(h);
    return 0;
}
