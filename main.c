#include <kpmodule.h>

static int test_init(void *ctx)
{
    kp_log("Hello KPM! 编译成功");
    return 0;
}

static int test_exit(void *ctx)
{
    kp_log("KPM unload");
    return 0;
}

KPM_INIT(test_init);
KPM_EXIT(test_exit);
KPM_NAME("test");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
