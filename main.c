#include "kpmodule.h"
#include <linux/printk.h>

static int my_init(const char *args, const char *event, void *reserved)
{
    pr_info("[KPM] loaded success!\n");
    return 0;
}

static void my_exit(void *reserved)
{
    pr_info("[KPM] unloaded!\n");
}

KPM_MODULE_INFO("pnii", "1.0", "Apache-2.0");
KPM_INIT(my_init);
KPM_EXIT(my_exit);
