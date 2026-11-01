#include "kpmodule.h"

extern void ap_log(const char *msg);

KPM_NAME("new-kpm-shell");
KPM_VERSION("1.0.0");
KPM_AUTHOR("you");
KPM_DESCRIPTION("Pure empty KPM shell for APatch");

static int module_init(void)
{
    ap_log("[KPM] 模块加载成功");
    return 0;
}

static void module_exit(void)
{
    ap_log("[KPM] 模块卸载");
}

KPM_INIT(module_init);
KPM_EXIT(module_exit);
