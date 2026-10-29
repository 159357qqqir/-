#include <kpmodule.h>

KPM_NAME("vt_touch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

#define ABS_MT_POSITION_X 0x35
#define ABS_MT_POSITION_Y 0x36
#define MT_TOOL_FINGER 0

struct input_dev;
extern unsigned long kallsyms_lookup_name(const char *name);

void input_mt_slot(struct input_dev *dev, int slot);
void input_mt_report_slot_state(struct input_dev *dev, unsigned int tool_type, int active);
void input_report_abs(struct input_dev *dev, unsigned int code, int value);
void input_sync(struct input_dev *dev);

static struct input_dev *g_tp_dev = NULL;

static void send_one_click(int x, int y)
{
    if (!g_tp_dev)
        return;
    input_mt_slot(g_tp_dev, 1);
    input_mt_report_slot_state(g_tp_dev, MT_TOOL_FINGER, 1);
    input_report_abs(g_tp_dev, ABS_MT_POSITION_X, x);
    input_report_abs(g_tp_dev, ABS_MT_POSITION_Y, y);
    input_sync(g_tp_dev);

    input_mt_slot(g_tp_dev, 1);
    input_mt_report_slot_state(g_tp_dev, MT_TOOL_FINGER, 0);
    input_sync(g_tp_dev);
}

static long vt_init(const char *args, const char *event, void *reserved)
{
    unsigned long dev_addr = kallsyms_lookup_name("synaptics_ts_input_dev");
    if (dev_addr != 0)
    {
        g_tp_dev = (struct input_dev *)dev_addr;
        send_one_click(720, 1500);
    }
    return 0;
}

static long vt_exit(void *reserved)
{
    g_tp_dev = NULL;
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
