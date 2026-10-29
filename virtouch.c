#include <kpmodule.h>

KPM_NAME("kp_empty");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("test");

static long empty_init(const char *args, const char *event, void *reserved)
{
    // 啥都不干，直接返回成功
    return 0;
}

static long empty_exit(void *reserved)
{
    return 0;
}

KPM_INIT(empty_init);
KPM_EXIT(empty_exit);
