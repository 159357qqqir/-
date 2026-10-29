#include <kpmodule.h>

KPM_NAME("virtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

static long vt_init(const char *args, const char *event, void *reserved)
{
    // 空逻辑，不调用任何内核函数
    return 0;
}

static long vt_exit(void *reserved)
{
    // 空逻辑，不调用任何内核函数
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
