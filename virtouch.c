#include <kpmodule.h>

KPM_NAME("kp_thermal_exynos_hard8");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

extern unsigned long kallsyms_lookup_name(const char *name);
extern int pr_info(const char *fmt, ...);
extern int pr_err(const char *fmt, ...);
extern long kp_inline_hook(void *target, void *handler, void **orig);
extern long kp_inline_unhook(void *target);

struct thermal_zone_device;
typedef void (*fp_exynos_report_trigger)(struct thermal_zone_device *tz, int trip);
static fp_exynos_report_trigger orig_exynos_report_trigger = NULL;

static void hooked_exynos_report_trigger(struct thermal_zone_device *tz, int trip)
{
    pr_info("[KP-Therm] Drop thermal trigger trip=%d\n", trip);
    // 直接return，不往thermal子系统上报高温，拦截温控链路
    return;
}

static long therm_init(const char *args, const char *event, void *reserved)
{
    unsigned long sym = kallsyms_lookup_name("exynos_report_trigger");
    if(!sym)
    {
        pr_err("[KP-Therm] exynos_report_trigger NOT FOUND\n");
        return -1;
    }
    orig_exynos_report_trigger = (fp_exynos_report_trigger)sym;
    kp_inline_hook((void *)sym, hooked_exynos_report_trigger, (void **)&orig_exynos_report_trigger);
    pr_info("[KP-Therm] hook exynos_report_trigger OK\n");
    return 0;
}

static long therm_exit(void *reserved)
{
    if(orig_exynos_report_trigger)
    {
        kp_inline_unhook((void *)orig_exynos_report_trigger);
        orig_exynos_report_trigger = NULL;
    }
    return 0;
}

KPM_INIT(therm_init);
KPM_EXIT(therm_exit);
