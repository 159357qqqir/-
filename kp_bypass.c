#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sched.h>

#define SYS_kp_supercall 435
#define SUPERCALL_SELINUX_BYPASS 10

const char *super_key = "123456789aa";

int main(void)
{
    long ret = syscall(SYS_kp_supercall, SUPERCALL_SELINUX_BYPASS, 1, 0,0,0,0, super_key);
    if(ret != 0)
    {
        printf("❌ 开启SELinux绕过失败，ret=%ld\n", ret);
        return -1;
    }
    printf("✅ 当前线程 SELinux 绕过已开启\n");

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
