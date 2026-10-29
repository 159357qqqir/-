#include <kpmodule.h>
#include <stdarg.h>
#include <string.h>

// 手动定义input事件常量
#define EV_KEY      0x01
#define EV_ABS      0x03
#define BTN_TOUCH   0x14a

#define ABS_MT_SLOT             0x2f
#define ABS_MT_TRACKING_ID      0x2d
#define ABS_MT_POSITION_X       0x35
#define ABS_MT_POSITION_Y       0x36

unsigned long kp_find_symbol(const char *name);
void kp_print(const char *fmt, ...);

// 函数指针
static struct input_dev *(*p_input_allocate_device)(void);
static void (*p_input_free_device)(struct input_dev *dev);
static int (*p_input_register_device)(struct input_dev *dev);
static void (*p_input_unregister_device)(struct input_dev *dev);
static void (*p_input_set_capability)(struct input_dev *dev, unsigned int type, unsigned int code);
static void (*p_input_report_abs)(struct input_dev *dev, unsigned int code, int value);
static void (*p_input_sync)(struct input_dev *dev);
static void (*p_input_set_abs_params)(struct input_dev *dev, unsigned int axis, int min, int max, int fuzz, int flat);
static void (*p_msleep)(unsigned int msec);

static struct input_dev *vt_dev = NULL;

KPM_NAME("virtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

static long vt_init(const char *args, const char *event, void *reserved)
{
    kp_print("[virtouch] start check symbols\n");

    p_input_allocate_device    = (void *)kp_find_symbol("input_allocate_device");
    p_input_register_device    = (void *)kp_find_symbol("input_register_device");
    p_input_unregister_device  = (void *)kp_find_symbol("input_unregister_device");
    p_input_free_device        = (void *)kp_find_symbol("input_free_device");
    p_input_set_capability     = (void *)kp_find_symbol("input_set_capability");
    p_input_report_abs         = (void *)kp_find_symbol("input_report_abs");
    p_input_sync               = (void *)kp_find_symbol("input_sync");
    p_input_set_abs_params     = (void *)kp_find_symbol("input_set_abs_params");
    p_msleep                   = (void *)kp_find_symbol("msleep");

    kp_print("input_allocate_device = %lx\n", (unsigned long)p_input_allocate_device);
    kp_print("input_register_device = %lx\n", (unsigned long)p_input_register_device);
    kp_print("input_unregister_device = %lx\n", (unsigned long)p_input_unregister_device);
    kp_print("input_free_device = %lx\n", (unsigned long)p_input_free_device);
    kp_print("input_set_capability = %lx\n", (unsigned long)p_input_set_capability);
    kp_print("input_report_abs = %lx\n", (unsigned long)p_input_report_abs);
    kp_print("input_sync = %lx\n", (unsigned long)p_input_sync);
    kp_print("input_set_abs_params = %lx\n", (unsigned long)p_input_set_abs_params);
    kp_print("msleep = %lx\n", (unsigned long)p_msleep);

    if (!p_input_allocate_device || !p_input_register_device || !p_input_free_device ||
        !p_input_unregister_device || !p_input_set_capability || !p_input_report_abs ||
        !p_input_sync || !p_input_set_abs_params || !p_msleep)
    {
        kp_print("[virtouch] ERROR: some symbol missing!\n");
        return -1;
    }

    vt_dev = p_input_allocate_device();
    if (!vt_dev)
    {
        kp_print("[virtouch] allocate dev fail\n");
        return -1;
    }

    // 能力注册
    p_input_set_capability(vt_dev, EV_KEY, BTN_TOUCH);
    p_input_set_capability(vt_dev, EV_ABS, ABS_MT_SLOT);
    p_input_set_capability(vt_dev, EV_ABS, ABS_MT_TRACKING_ID);
    p_input_set_capability(vt_dev, EV_ABS, ABS_MT_POSITION_X);
    p_input_set_capability(vt_dev, EV_ABS, ABS_MT_POSITION_Y);

    p_input_set_abs_params(vt_dev, ABS_MT_SLOT, 0, 4, 0, 0);
    p_input_set_abs_params(vt_dev, ABS_MT_TRACKING_ID, 0, 0xFFFF, 0, 0);
    p_input_set_abs_params(vt_dev, ABS_MT_POSITION_X, 0, 1080, 0, 0);
    p_input_set_abs_params(vt_dev, ABS_MT_POSITION_Y, 0, 2400, 0, 0);

    int ret = p_input_register_device(vt_dev);
    if (ret)
    {
        kp_print("[virtouch] register dev fail, ret=%d\n", ret);
        p_input_free_device(vt_dev);
        vt_dev = NULL;
        return ret;
    }
    kp_print("[virtouch] device register ok!\n");

    // 测试点击 x=540 y=1200
    p_input_report_abs(vt_dev, ABS_MT_SLOT, 0);
    p_input_report_abs(vt_dev, ABS_MT_TRACKING_ID, 1);
    p_input_report_abs(vt_dev, ABS_MT_POSITION_X, 540);
    p_input_report_abs(vt_dev, ABS_MT_POSITION_Y, 1200);
    p_input_report_abs(vt_dev, BTN_TOUCH, 1);
    p_input_sync(vt_dev);

    p_msleep(200);

    p_input_report_abs(vt_dev, ABS_MT_TRACKING_ID, -1);
    p_input_report_abs(vt_dev, BTN_TOUCH, 0);
    p_input_sync(vt_dev);

    kp_print("[virtouch] check done, all ok, loaded\n");
    return 0;
}

static long vt_exit(void *reserved)
{
    kp_print("[virtouch] unload\n");
    if (vt_dev)
    {
        p_input_unregister_device(vt_dev);
        p_input_free_device(vt_dev);
        vt_dev = NULL;
    }
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
