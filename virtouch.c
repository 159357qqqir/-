#include <kpmodule.h>

// 提前声明KP导出函数原型
unsigned long kp_find_symbol(const char *name);
void kp_print(const char *fmt, ...);

// 内核input函数指针
static struct input_dev *(*p_input_allocate_device)(void);
static void (*p_input_free_device)(struct input_dev *dev);
static int (*p_input_register_device)(struct input_dev *dev);
static void (*p_input_unregister_device)(struct input_dev *dev);
static void (*p_input_set_capability)(struct input_dev *dev, unsigned int type,
                                      unsigned int code);
static void (*p_input_report_abs)(struct input_dev *dev, unsigned int code, int value);
static void (*p_input_sync)(struct input_dev *dev);

static struct input_dev *vt_dev = NULL;

KPM_NAME("virtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

static long vt_init(const char *args, const char *event, void *reserved)
{
    kp_print("[virtouch] start check symbols\n");

    p_input_allocate_device = (void *)kp_find_symbol("input_allocate_device");
    p_input_register_device = (void *)kp_find_symbol("input_register_device");
    p_input_unregister_device = (void *)kp_find_symbol("input_unregister_device");
    p_input_free_device = (void *)kp_find_symbol("input_free_device");
    p_input_set_capability = (void *)kp_find_symbol("input_set_capability");
    p_input_report_abs = (void *)kp_find_symbol("input_report_abs");
    p_input_sync = (void *)kp_find_symbol("input_sync");

    kp_print("input_allocate_device = %lx\n", (unsigned long)p_input_allocate_device);
    kp_print("input_register_device = %lx\n", (unsigned long)p_input_register_device);
    kp_print("input_unregister_device = %lx\n", (unsigned long)p_input_unregister_device);
    kp_print("input_free_device = %lx\n", (unsigned long)p_input_free_device);
    kp_print("input_set_capability = %lx\n", (unsigned long)p_input_set_capability);
    kp_print("input_report_abs = %lx\n", (unsigned long)p_input_report_abs);
    kp_print("input_sync = %lx\n", (unsigned long)p_input_sync);

    // 任意一个符号为空，直接退出
    if (!p_input_allocate_device || !p_input_register_device || !p_input_free_device ||
        !p_input_unregister_device || !p_input_set_capability || !p_input_report_abs || !p_input_sync)
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

    // 设置设备名称
    snprintf(vt_dev->name, sizeof(vt_dev->name), "KP_VirtualTouch");

    // 开启EV_ABS、EV_KEY，BTN_TOUCH、多点ABS_MT
    p_input_set_capability(vt_dev, EV_KEY, BTN_TOUCH);
    p_input_set_capability(vt_dev, EV_ABS, ABS_MT_SLOT);
    p_input_set_capability(vt_dev, EV_ABS, ABS_MT_TRACKING_ID);
    p_input_set_capability(vt_dev, EV_ABS, ABS_MT_POSITION_X);
    p_input_set_capability(vt_dev, EV_ABS, ABS_MT_POSITION_Y);

    // 坐标范围，根据你的屏幕改
    vt_dev->absinfo[ABS_MT_POSITION_X].minimum = 0;
    vt_dev->absinfo[ABS_MT_POSITION_X].maximum = 1080;
    vt_dev->absinfo[ABS_MT_POSITION_Y].minimum = 0;
    vt_dev->absinfo[ABS_MT_POSITION_Y].maximum = 2400;
    vt_dev->absinfo[ABS_MT_SLOT].minimum = 0;
    vt_dev->absinfo[ABS_MT_SLOT].maximum = 4;

    int ret = p_input_register_device(vt_dev);
    if (ret)
    {
        kp_print("[virtouch] register dev fail, ret=%d\n", ret);
        p_input_free_device(vt_dev);
        vt_dev = NULL;
        return ret;
    }
    kp_print("[virtouch] device register ok!\n");

    // 示例上报一次点击 x=540 y=1200
    p_input_report_abs(vt_dev, ABS_MT_SLOT, 0);
    p_input_report_abs(vt_dev, ABS_MT_TRACKING_ID, 1);
    p_input_report_abs(vt_dev, ABS_MT_POSITION_X, 540);
    p_input_report_abs(vt_dev, ABS_MT_POSITION_Y, 1200);
    p_input_report_abs(vt_dev, BTN_TOUCH, 1);
    p_input_sync(vt_dev);

    msleep(200);

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
