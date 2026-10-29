#include <kpmodule.h>

KPM_NAME("kp_thermal_gettemp");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

extern unsigned long kallsyms_lookup_name(const char *name);
extern int pr_info(const char *fmt, ...);
extern int pr_err(const char *fmt, ...);
extern long kp_inline_hook(void *target, void *handler, void **orig);
extern long kp_inline_unhook(void *target);

struct thermal_zone_device;
typedef int (*fp_thermal_zone_get_temp)(struct thermal_zone_device *tz, int *temp);
static fp_thermal_zone_get_temp orig_thermal_zone_get_temp = NULL;

static int hooked_thermal_zone_get_temp(struct thermal_zone_device *tz, int *temp)
{
    // 固定返回 40000 (40℃)
    pr_info("[KP-Therm] fake temp 40000\n");
    *temp = 40000;
    return 0;
}

static long therm_init(const char *args, const char *event, void *reserved)
{
    unsigned long sym = kallsyms_lookup_name("thermal_zone_get_temp");
    if (!sym)
    {
        pr_err("[KP-Therm] thermal_zone_get_temp NOT FOUND!\n");
        return -1;
    }
    orig_thermal_zone_get_temp = (fp_thermal_zone_get_temp)sym;
    kp_inline_hook((void *)sym, hooked_thermal_zone_get_temp, (void **)&orig_thermal_zone_get_temp);
    pr_info("[KP-Therm] hook thermal_zone_get_temp SUCCESS\n");
    return 0;
}

static long therm_exit(void *reserved)
{
    if (orig_thermal_zone_get_temp)
    {
        kp_inline_unhook((void *)orig_thermal_zone_get_temp);
        orig_thermal_zone_get_temp = NULL;
    }
    return 0;
}

KPM_INIT(therm_init);
KPM_EXIT(therm_exit);
