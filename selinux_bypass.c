#include "kpm.h"
#include <linux/string.h>
#include <linux/sched.h>

static void *avc_has_perm_noaudit_sym = NULL;

typedef int (*avc_has_perm_noaudit_t)(u32 ssid, u32 tsid, u32 tclass, u32 requested);

static int hook_avc_has_perm_noaudit(u32 ssid, u32 tsid, u32 tclass, u32 requested)
{
    struct task_struct *cur = current;
    // 放行进程名 kp_bypass
    if (strcmp(cur->comm, "kp_bypass") == 0)
    {
        return 0;
    }
    return ((avc_has_perm_noaudit_t)avc_has_perm_noaudit_sym)(ssid, tsid, tclass, requested);
}

KPM_INIT()
{
    avc_has_perm_noaudit_sym = ksymbol_get("avc_has_perm_noaudit");
    if (avc_has_perm_noaudit_sym)
    {
        kp_hook(avc_has_perm_noaudit_sym, hook_avc_has_perm_noaudit, &avc_has_perm_noaudit_sym);
        kp_log("[SELBYPASS] ✅ Hook avc_has_perm_noaudit ok, allow kp_bypass");
    }
    else
    {
        kp_log("[SELBYPASS] ❌ symbol avc_has_perm_noaudit not found");
    }
}

KPM_EXIT()
{
    if (avc_has_perm_noaudit_sym)
    {
        kp_unhook(avc_has_perm_noaudit_sym);
        kp_log("[SELBYPASS] 🗑️ unhook success");
    }
}
