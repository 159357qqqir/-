#include <kpmodule.h>

KPM_NAME("kp_thermal_hard8");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

// ========= 手动补全函数声明，解决 implicit 报错 =========
extern unsigned long kallsyms_lookup_name(const char *name);
extern int pr_info(const char *fmt, ...);
extern int pr_err(const char *fmt, ...);

// KPM hook 接口声明
extern long kp_inline_hook(void *target, void *handler, void **orig);
extern long kp_inline_unhook(void *target);
// =========================================================

struct thermal_cooling_device;
typedef int (*fp_set_cur_state)(struct thermal_cooling_device *, unsigned long);
static fp_set_cur_state orig_set_cur_state = NULL;

static int hooked_set_cur_state(struct thermal_cooling_device *cdev, unsigned long state)
{
    if(state > 0)
    {
        pr_info("[KP-Therm] BLOCK cooling state=%lu\n", state);
        return 0;
    }
    return orig_set_cur_state(cdev, state);
}

static long therm_init(const char *args, const char *event, void *reserved)
{
    unsigned long sym = kallsyms_lookup_name("thermal_cooling_device_set_cur_state");
    if(!sym)
    {
        pr_err("[KP-Therm] symbol thermal_cooling_device_set_cur_state NOT FOUND!\n");
        return -1;
    }
    orig_set_cur_state = (fp_set_cur_state)sym;
    kp_inline_hook((void *)sym, hooked_set_cur_state, (void **)&orig_set_cur_state);
    pr_info("[KP-Therm] hook success\n");
    return 0;
}

static long therm_exit(void *reserved)
{
    if(orig_set_cur_state)
    {
        kp_inline_unhook((void *)orig_set_cur_state);
        orig_set_cur_state = NULL;
    }
    return 0;
}

KPM_INIT(therm_init);
KPM_EXIT(therm_exit);
