#include <kpmodule.h>

KPM_NAME("kp_thermal_sys");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

// 基础内核函数声明
extern int pr_info(const char *fmt, ...);
extern int pr_err(const char *fmt, ...);
extern char *strstr(const char *haystack, const char *needle);
extern size_t strlen(const char *s);
extern char *strncpy(char *dest, const char *src, size_t n);
extern unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
extern unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);

// KPM Hook API声明
extern long kp_hook_syscall(long nr, void *handler, void **orig);
extern long kp_unhook_syscall(long nr);

// aarch64 Android 系统调用号
#define __NR_openat 56
#define __NR_read   63
#define EFAULT      14

static long (*orig_openat)(int dfd, const char __user *path, int flags, mode_t mode) = NULL;
static long (*orig_read)(unsigned int fd, char __user *buf, size_t count) = NULL;

static int fake_temp_fd = -1;

static long hooked_openat(int dfd, const char __user *path, int flags, mode_t mode)
{
    char kpath[128];
    long ret = orig_openat(dfd, path, flags, mode);
    if(ret >= 0)
    {
        if(copy_from_user(kpath, path, sizeof(kpath)-1) == 0)
        {
            kpath[sizeof(kpath)-1] = '\0';
            if(strstr(kpath, "/sys/class/thermal/") && strstr(kpath, "/temp"))
            {
                pr_info("[KP-Therm] open thermal temp file fd=%ld\n", ret);
                fake_temp_fd = ret;
            }
        }
    }
    return ret;
}

static long hooked_read(unsigned int fd, char __user *buf, size_t count)
{
    if(fd == fake_temp_fd)
    {
        char fake_buf[] = "40000\n";
        size_t len = strlen(fake_buf);
        if(len > count) len = count;
        copy_to_user(buf, fake_buf, len);
        pr_info("[KP-Therm] return fake temp 40000\n");
        return len;
    }
    return orig_read(fd, buf, count);
}

static long therm_init(const char *args, const char *event, void *reserved)
{
    kp_hook_syscall(__NR_openat, hooked_openat, (void **)&orig_openat);
    kp_hook_syscall(__NR_read, hooked_read, (void **)&orig_read);
    pr_info("[KP-Therm] hook openat+read ok\n");
    return 0;
}

static long therm_exit(void *reserved)
{
    kp_unhook_syscall(__NR_openat);
    kp_unhook_syscall(__NR_read);
    pr_info("[KP-Therm] unhook ok\n");
    return 0;
}

KPM_INIT(therm_init);
KPM_EXIT(therm_exit);
