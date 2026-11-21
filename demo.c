#include "kpmodule.h"

KPM_NAME("hello_demo");
KPM_VERSION("1.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("test");
KPM_DESCRIPTION("Minimal test KPM");

static long kpm_init(const char *args, const char *event, void *reserved)
{
    kp_log("✅ Hello KPM loaded successfully!\n");
    return 0; // 这里return非0直接加载失败-1
}

static long kpm_exit(void *reserved)
{
    kp_log("✅ KPM unloaded\n");
    return 0;
}

KPM_INIT(kpm_init);
KPM_EXIT(kpm_exit);
