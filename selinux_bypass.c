#include "kpm.h"
#include <linux/string.h>
#include <linux/sched.h>

static void *avc_has_perm_sym = NULL;

typedef int (*avc_has_perm_t)(u32 ssid, u32 tsid, u32 tclass, u32 requested, struct avc_audit_data *auditd);

static int hook_avc_has_perm(u32 ssid, u32 tsid, u32 tclass, u32 requested, struct avc_audit_data *auditd)
{
    struct task_struct *cur = current;
    // 放行 comm = kp_bypass
    if (strcmp(cur->comm, "kp_bypass") == 0)
    {
        return 0;
    }
    return ((avc_has_perm_t)avc_has_perm_sym)(ssid, tsid, tclass, requested, auditd);
}

KPM_INIT()
{
    avc_has_perm_sym = ksymbol_get("avc_has_perm");
    if (avc_has_perm_sym)
    {
        kp_hook(avc_has_perm_sym, hook_avc_has_perm, &avc_has_perm_sym);
        kp_log("[SELBYPASS] ✅ Hook avc_has_perm success, allow kp_bypass");
    }
    else
    {
        kp_log("[SELBYPASS] ❌ symbol avc_has_perm not found");
    }
}

KPM_EXIT()
{
    if (avc_has_perm_sym)
    {
        kp_unhook(avc_has_perm_sym);
        kp_log("[SELBYPASS] 🗑️ unhook ok");
    }
}
