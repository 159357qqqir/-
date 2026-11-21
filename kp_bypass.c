#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sched.h>

#define SYS_kp_supercall 435
#define SUPERCALL_SELINUX_BYPASS 10

const char *super_key = "6.6.6.a";

long kp_supercall(int cmd, long a1, long a2, long a3, long a4, long a5, const char *key)
{
    return syscall(SYS_kp_supercall, cmd, a1, a2, a3, a4, a5, key);
}

int main(void)
{
    long ret = kp_supercall(SUPERCALL_SELINUX_BYPASS, 1, 0,0,0,0, super_key);
    if(ret != 0)
    {
        printf("❌ 开启SELinux绕过失败，核对秘钥\n");
        return -1;
    }
    printf("✅ 当前线程 SELinux 绕过已开启\n");

    // 测试 /dev/mem
    int fd = open("/dev/mem", O_RDONLY);
    if (fd >= 0)
    {
        printf("✅ /dev/mem 打开成功\n");
        close(fd);
    }
    else
    {
        perror("open /dev/mem");
    }

    // 测试创建独立网络命名空间 netns
    if (unshare(CLONE_NEWNET) == 0)
    {
        printf("✅ 网络命名空间创建成功\n");
    }
    else
    {
        perror("unshare netns");
    }

    return 0;
}
