#include <kpmodule.h>

KPM_NAME("virtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

// 函数指针定义
typedef void *(*input_allocate_device_t)(void);
typedef int (*input_register_device_t)(void *dev);
typedef void (*input_unregister_device_t)(void *dev);
typedef void (*input_free_device_t)(void *dev);
typedef void (*input_set_capability_t)(void *dev, int type, unsigned long bit);
typedef void (*input_report_abs_t)(void *dev, int code, int value);
typedef void (*input_sync_t)(void *dev);

static input_allocate_device_t fn_input_allocate_device;
static input_register_device_t fn_input_register_device;
static input_unregister_device_t fn_input_unregister_device;
static input_free_device_t fn_input_free_device;
static input_set_capability_t fn_input_set_capability;
static input_report_abs_t fn_input_report_abs;
static input_sync_t fn_input_sync;

static void *vinput_dev = NULL;

// 事件码定义 (MT多点触摸)
#define EV_ABS      0x03
#define ABS_MT_SLOT             0x2f
#define ABS_MT_TRACKING_ID      0x39
#define ABS_MT_POSITION_X       0x35
#define ABS_MT_POSITION_Y       0x36
#define ABS_MT_TOUCH_MAJOR      0x3a

// 可选：给外部调用的投递触摸点接口（你后面可以用KPM参数调用这个）
static void virtouch_send_touch(int slot, int tracking_id, int x, int y)
{
    if (!vinput_dev || !fn_input_report_abs || !fn_input_sync)
    {
        kp_print("[virtouch] vdev not ready\n");
        return;
    }
    fn_input_report_abs(vinput_dev, ABS_MT_SLOT, slot);
    fn_input_report_abs(vinput_dev, ABS_MT_TRACKING_ID, tracking_id);
    fn_input_report_abs(vinput_dev, ABS_MT_POSITION_X, x);
    fn_input_report_abs(vinput_dev, ABS_MT_POSITION_Y, y);
    fn_input_report_abs(vinput_dev, ABS_MT_TOUCH_MAJOR, 10);
    fn_input_sync(vinput_dev);
}

static long vt_init(const char *args, const char *event, void *reserved)
{
    kp_print("[virtouch] init start\n");

    // 查找全部需要的内核符号
    fn_input_allocate_device = kp_find_symbol("input_allocate_device");
    fn_input_register_device = kp_find_symbol("input_register_device");
    fn_input_unregister_device = kp_find_symbol("input_unregister_device");
    fn_input_free_device = kp_find_symbol("input_free_device");
    fn_input_set_capability = kp_find_symbol("input_set_capability");
    fn_input_report_abs = kp_find_symbol("input_report_abs");
    fn_input_sync = kp_find_symbol("input_sync");

    if (!fn_input_allocate_device || !fn_input_register_device ||
        !fn_input_unregister_device || !fn_input_free_device ||
        !fn_input_set_capability || !fn_input_report_abs || !fn_input_sync)
    {
        kp_print("[virtouch] some symbol missing\n");
        return -1;
    }
    kp_print("[virtouch] all symbols found\n");

    // 分配虚拟input设备
    vinput_dev = fn_input_allocate_device();
    if (!vinput_dev)
    {
        kp_print("[virtouch] allocate dev fail\n");
        return -1;
    }

    // 设置能力：EV_ABS，多点触摸MT
    fn_input_set_capability(vinput_dev, EV_ABS, ABS_MT_SLOT);
    fn_input_set_capability(vinput_dev, EV_ABS, ABS_MT_TRACKING_ID);
    fn_input_set_capability(vinput_dev, EV_ABS, ABS_MT_POSITION_X);
    fn_input_set_capability(vinput_dev, EV_ABS, ABS_MT_POSITION_Y);
    fn_input_set_capability(vinput_dev, EV_ABS, ABS_MT_TOUCH_MAJOR);

    // 注册到内核，生成 /dev/input/eventX
    int ret = fn_input_register_device(vinput_dev);
    if (ret != 0)
    {
        kp_print("[virtouch] register dev fail, ret=%d\n", ret);
        fn_input_free_device(vinput_dev);
        vinput_dev = NULL;
        return -1;
    }
    kp_print("[virtouch] virtual touch dev registered success!\n");
    kp_print("[virtouch] check /dev/input/ for new event node\n");
    return 0;
}

static long vt_exit(void *reserved)
{
    kp_print("[virtouch] unload\n");
    if (vinput_dev)
    {
        fn_input_unregister_device(vinput_dev);
        fn_input_free_device(vinput_dev);
        vinput_dev = NULL;
        kp_print("[virtouch] virtual touch device removed\n");
    }
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
