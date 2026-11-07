#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

#define WANGZHE_UID 10287
#define UP_RATE     3000000UL  // 3Mbps上行
#define BURST       375000UL

static unsigned long tokens = 0;
static struct timespec last;

static int token_bucket(unsigned int pkt_len)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long diff_us = (now.tv_sec - last.tv_sec) * 1000000L + (now.tv_nsec - last.tv_nsec)/1000L;
    tokens += diff_us * UP_RATE / 8000000UL;
    if(tokens > BURST) tokens = BURST;
    last = now;

    if(tokens >= pkt_len){
        tokens -= pkt_len;
        return 1;
    }
    return 0;
}

static int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfa);
    if(!ph) return 0;
    uint32_t id = ph->packet_id;

    unsigned char *pkt;
    int pkt_len = nfq_get_payload(nfa, &pkt);
    if(pkt_len < 0){
        nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        return 0;
    }

    uint32_t uid = 0;
    if(nfq_get_uid(nfa, &uid) < 0){
        nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        return 0;
    }

    if(uid == WANGZHE_UID){
        nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
    }else{
        if(token_bucket(pkt_len)){
            nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        }else{
            nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
        }
    }
    return 0;
}

int main(void)
{
    struct nfq_q_handle *qh;
    struct nfq_handle *h;
    int fd;
    char buf[4096];
    clock_gettime(CLOCK_MONOTONIC, &last);

    h = nfq_open();
    if(!h){ perror("nfq_open"); exit(1); }
    if(nfq_unbind_pf(h, AF_INET) <0 || nfq_bind_pf(h, AF_INET)<0){
        perror("bind pf"); exit(1);
    }
    qh = nfq_create_queue(h, 0, callback, NULL);
    if(!qh){ perror("nfq_create_queue"); exit(1); }
    nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);
    fd = nfq_fd(h);

    system("iptables -t mangle -A OUTPUT -j NFQUEUE --queue-num 0");
    system("iptables -t mangle -A POSTROUTING -j NFQUEUE --queue-num 0");
    printf("nfqueue shaper start, uid %d bypass, up 3M\n",WANGZHE_UID);

    while(1){
        int rv = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
        if(rv>0) nfq_handle_packet(h, buf, rv);
        else if(rv < 0) usleep(500);
    }
    system("iptables -t mangle -D OUTPUT -j NFQUEUE --queue-num 0");
    system("iptables -t mangle -D POSTROUTING -j NFQUEUE --queue-num 0");
    nfq_destroy_queue(qh);
    nfq_close(h);
    return 0;
}
