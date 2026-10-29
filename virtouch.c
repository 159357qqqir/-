#include <kpmodule.h>

KPM_NAME("kp_thermal_hard8");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

struct thermal_cooling_device;
// 函数原型：设置cooling档位，state越大=降温越强（降频越狠）
typedef int (*fp_set_cur_state)(struct thermal_cooling_device *, unsigned long);
static fp_set_cur_state orig_set_cur_state = NULL;

static int hooked_set_cur_state(struct thermal_cooling_device *cdev, unsigned long state)
{
    // 核心：拦截所有大于0的降温请求，直接返回成功，不去执行降频/降亮度
    if(state > 0)
    {
        pr_info("[KP-Therm] BLOCK cooling state=%lu\n", state);
        return 0;
    }
    // state=0是恢复满频，放行
    return orig_set_cur_state(cdev, state);
}

static long therm_init(const char *args, const char *event, void *reserved)
{
    unsigned long sym = kallsyms_lookup_name("thermal_cooling_device_set_cur_state");
    if(!sym)
    {
        pr_err("[KP-Therm] symbol NOT FOUND!\n");
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
