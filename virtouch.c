#include <kpmodule.h>

KPM_NAME("kp_test");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("test");

extern unsigned long kallsyms_lookup_name(const char *name);

static long test_init(const char *args, const char *event, void *reserved)
{
    unsigned long addr = kallsyms_lookup_name("thermal_zone_get_temp");
    if (addr)
    {
        // 找到符号，返回0代表加载成功
        return 0;
    }
    return -1;
}

static long test_exit(void *reserved)
{
    return 0;
}

KPM_INIT(test_init);
KPM_EXIT(test_exit);
