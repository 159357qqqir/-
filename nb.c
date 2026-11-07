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

// 普通应用限速：3Mbps
#define RATE_NORMAL     3000000UL
#define BURST_NORMAL    375000UL
// 无UID硬件卸载包放宽到8Mbps
#define RATE_OFFLOAD    8000000UL
#define BURST_OFFLOAD   1000000UL

static unsigned long token_normal = 0;
static unsigned long token_offload = 0;
static struct timespec last_ts;

static int bucket_normal(size_t pkt_len)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long diff_us = (now.tv_sec - last_ts.tv_sec) * 1000000L
                   + (now.tv_nsec - last_ts.tv_nsec) / 1000L;

    token_normal += diff_us * RATE_NORMAL / 8000000UL;
    if(token_normal > BURST_NORMAL) token_normal = BURST_NORMAL;
    last_ts = now;

    if(token_normal >= pkt_len){
        token_normal -= pkt_len;
        return 1;
    }
    return 0;
}

static int bucket_offload(size_t pkt_len)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long diff_us = (now.tv_sec - last_ts.tv_sec) * 1000000L
                   + (now.tv_nsec - last_ts.tv_nsec) / 1000L;

    token_offload += diff_us * RATE_OFFLOAD / 8000000UL;
    if(token_offload > BURST_OFFLOAD) token_offload = BURST_OFFLOAD;
    last_ts = now;

    if(token_offload >= pkt_len){
        token_offload -= pkt_len;
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

    uint32_t uid = 0;
    int ret_uid = nfq_get_uid(nfa, &uid);
    if(ret_uid >=0 && uid == 10287)
    {
        printf("BYPASS UID=10287 len=%d\n", len);
        nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        return 0;
    }

    if(ret_uid >=0)
    {
        // 正常带UID，非王者：3M限速
        if(bucket_normal(len))
        {
            printf("NORMAL ACCEPT uid=%u len=%d\n",uid,len);
            nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        }else{
            printf("NORMAL DROP uid=%u len=%d\n",uid,len);
            nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
        }
    }else{
        // NO_UID IPA卸载包：放宽8M限速
        if(bucket_offload(len))
        {
            printf("OFFLOAD ACCEPT len=%d\n",len);
            nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        }else{
            printf("OFFLOAD DROP len=%d\n",len);
            nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
        }
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
    printf("Started: UID10287 bypass | normal:3M | offload(NO_UID):8M\n");

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
