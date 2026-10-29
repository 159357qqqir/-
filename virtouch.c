#include <kpmodule.h>

KPM_NAME("vt_touch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

#define ABS_MT_POSITION_X 0x35
#define ABS_MT_POSITION_Y 0x36
#define MT_TOOL_FINGER 0
#define EV_ABS 0x03

struct input_dev;
extern unsigned long kallsyms_lookup_name(const char *name);

void input_mt_slot(struct input_dev *dev, int slot);
void input_mt_report_slot_state(struct input_dev *dev, unsigned int tool_type, int active);
void input_report_abs(struct input_dev *dev, unsigned int code, int value);
void input_sync(struct input_dev *dev);

static struct input_dev *g_tp_dev = NULL;

// 下发触摸事件
static void send_touch(int x, int y)
{
    if (!g_tp_dev)
        return;

    // 按下
    input_mt_slot(g_tp_dev, 1);
    input_mt_report_slot_state(g_tp_dev, MT_TOOL_FINGER, 1);
    input_report_abs(g_tp_dev, ABS_MT_POSITION_X, x);
    input_report_abs(g_tp_dev, ABS_MT_POSITION_Y, y);
    input_sync(g_tp_dev);

    // 抬起
    input_mt_slot(g_tp_dev, 1);
    input_mt_report_slot_state(g_tp_dev, MT_TOOL_FINGER, 0);
    input_sync(g_tp_dev);
}

// kpctl 回调：用户执行 kpctl send vt_touch x y
static long vt_ctl(const char *args, const char *out, unsigned int out_len)
{
    int x, y;
    if (sscanf(args, "%d %d", &x, &y) != 2)
    {
        return -1;
    }
    send_touch(x, y);
    return 0;
}

static long vt_init(const char *args, const char *event, void *reserved)
{
    // 三星synaptics触屏驱动全局input_dev符号
    unsigned long dev_addr = kallsyms_lookup_name("synaptics_ts_input_dev");
    if (dev_addr != 0)
    {
        g_tp_dev = (struct input_dev *)dev_addr;
    }
    return 0;
}

static long vt_exit(void *reserved)
{
    g_tp_dev = NULL;
    return 0;
}

KPM_CTL(vt_ctl);
KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
