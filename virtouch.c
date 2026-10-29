#include "kpmodule.h"
#include "linux/printk.h"

KPM_NAME("virtouch");
KPM_VERSION("1.0.0");
KPM_LICENSE("GPL");

static long vt_init(const char *args, const char *event, void *reserved)
{
    pr_info("virtouch module loaded!\n");
    return 0;
}

static long vt_exit(void *reserved)
{
    pr_info("virtouch module unloaded!\n");
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
