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

    uint32_t uid = 0;
    int ret_uid = nfq_get_uid(nfa, &uid);

    if(ret_uid >= 0 && uid == 10287)
    {
        printf("✅ BYPASS 王者 uid=10287 len=%d\n", len);
        nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
        return 0;
    }

    // 其余全部丢弃，0上行
    printf("❌ DROP 其他 uid=%d len=%d\n", ret_uid>=0 ? (int)uid : -1, len);
    nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
    return 0;
}

int main(void)
{
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    int fd;
    char buf[4096];
    uint32_t cfg_flags = NFQA_CFG_F_UID_GID;

    system("iptables -t mangle -F OUTPUT");
    system("iptables -t mangle -A OUTPUT -j NFQUEUE --queue-num 0");

    h = nfq_open();
    if(!h) { perror("nfq_open"); return 1; }
    if(nfq_unbind_pf(h, AF_INET) < 0) { perror("unbind"); return 1; }
    if(nfq_bind_pf(h, AF_INET) < 0) { perror("bind"); return 1; }

    qh = nfq_create_queue(h, 0, nf_callback, NULL);
    if(!qh) { perror("nfq_create_queue"); return 1; }
    nfq_set_queue_flags(qh, cfg_flags, cfg_flags);
    nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);
    fd = nfq_fd(h);
    printf("=== 已启动：仅UID10287放行，其余全部DROP ===\n");

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
