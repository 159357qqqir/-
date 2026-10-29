#include <kpmodule.h>

KPM_NAME("kp_thermal_sys");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

#include <linux/string.h>
#include <linux/fs.h>

extern int pr_info(const char *fmt, ...);
extern int pr_err(const char *fmt, ...);

static long (*orig_read)(unsigned int fd, char __user *buf, size_t count) = NULL;

static long hooked_read(unsigned int fd, char __user *buf, size_t count)
{
    char kbuf[64];
    long ret = orig_read(fd, kbuf, sizeof(kbuf)-1);
    if (ret > 0)
    {
        char path[128];
        if (kp_get_fd_path(fd, path, sizeof(path)) == 0)
        {
            if (strstr(path, "/sys/class/thermal/") && strstr(path, "/temp"))
            {
                pr_info("[KP-Therm] fake temp 40000\n");
                strncpy(kbuf, "40000\n", sizeof(kbuf)-1);
                ret = strlen(kbuf);
            }
        }
    }
    if (copy_to_user(buf, kbuf, ret))
        return -EFAULT;
    return ret;
}

static long therm_init(const char *args, const char *event, void *reserved)
{
    kp_hook_syscall(__NR_read, hooked_read, (void **)&orig_read);
    pr_info("[KP-Therm] syscall hook read ok\n");
    return 0;
}

static long therm_exit(void *reserved)
{
    kp_unhook_syscall(__NR_read);
    pr_info("[KP-Therm] unhook ok\n");
    return 0;
}

KPM_INIT(therm_init);
KPM_EXIT(therm_exit);
