#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#define PHONE_IP    "192.168.175.140"
#define PHONE_PORT  8888
#define BUF_SIZE    65536
#define BOUNDARY    "FILEUP123456789"

static int wsa_init(void)
{
    WSADATA wd;
    return WSAStartup(MAKEWORD(2,2), &wd);
}

static SOCKET tcp_connect(const char* ip, int port)
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == INVALID_SOCKET) return INVALID_SOCKET;
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons((u_short)port);
    inet_pton(AF_INET, ip, &sa.sin_addr);
    if(connect(s, (struct sockaddr*)&sa, sizeof(sa)) != 0)
    {
        closesocket(s);
        return INVALID_SOCKET;
    }
    return s;
}

int upload_file(const char* local_path, const char* save_name)
{
    SOCKET s = tcp_connect(PHONE_IP, PHONE_PORT);
    if(s == INVALID_SOCKET)
    {
        printf("❌ 连接手机失败\n");
        return -1;
    }
    FILE* fp = fopen(local_path, "rb");
    if(!fp)
    {
        closesocket(s);
        printf("❌ 打开本地文件失败\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long long total_size = _ftelli64(fp);
    fseek(fp, 0, SEEK_SET);

    char header[1024];
    int hlen = snprintf(header, sizeof(header),
"POST / HTTP/1.1\r\n"
"Host: %s:%d\r\n"
"Connection: close\r\n"
"Content-Type: multipart/form-data; boundary=%s\r\n\r\n"
"--%s\r\n"
"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n\r\n",
        PHONE_IP, PHONE_PORT, BOUNDARY, BOUNDARY, save_name);
    send(s, header, hlen, 0);

    char buf[BUF_SIZE];
    size_t rd;
    long long send_bytes = 0;
    printf("开始上传，总大小：%lld 字节\n", total_size);
    while((rd = fread(buf, 1, BUF_SIZE, fp)) > 0)
    {
        send(s, buf, (int)rd, 0);
        send_bytes += rd;
        printf("\r已发送：%lld / %lld", send_bytes, total_size);
    }
    fclose(fp);

    char end_buf[256];
    int elen = snprintf(end_buf, sizeof(end_buf), "\r\n--%s--\r\n", BOUNDARY);
    send(s, end_buf, elen, 0);

    char resp[512] = {0};
    recv(s, resp, sizeof(resp)-1, 0);
    closesocket(s);

    printf("\n✅ 传输完成！手机保存文件名：%s\n", save_name);
    printf("服务端应答：%s\n", resp);
    return 0;
}

void usage(void)
{
    printf("用法：upload.exe 本地完整路径 手机保存文件名\n");
    printf("例子：upload.exe D:\\29g.iso 29g.iso\n");
}

int main(int argc, char** argv)
{
    if(wsa_init() != 0)
    {
        printf("Winsock初始化失败\n");
        return 1;
    }
    if(argc != 3)
    {
        usage();
        WSACleanup();
        return 1;
    }
    upload_file(argv[1], argv[2]);
    WSACleanup();
    return 0;
}
