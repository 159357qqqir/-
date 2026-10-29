#include <kpmodule.h>

KPM_NAME("vt_touch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

#define MAX_SLOTS 10
#define ABS_MT_POSITION_X 0x35
#define ABS_MT_POSITION_Y 0x36
#define MT_TOOL_FINGER 0

struct input_dev;
void input_mt_slot(struct input_dev *dev, int slot);
void input_mt_report_slot_state(struct input_dev *dev, unsigned int tool_type, bool active);
void input_report_abs(struct input_dev *dev, unsigned int code, int value);
void input_sync(struct input_dev *dev);

struct my_touch_slot {
    int track_id;
    int x;
    int y;
    int pressed;
};

static struct my_touch_slot my_touch_dev[MAX_SLOTS];
static struct input_dev *g_touch_idev = NULL;

// 声明要hook的两个内核函数
kfunc_def(synaptics_ts_set_touch_function);
kfunc_def(synaptics_ts_parse_touch_report);

// 保存钩子句柄
static void *hook_set_touch_fn = NULL;
static void *hook_parse_report = NULL;

static void my_touch_clear_slot(struct my_touch_slot *slot)
{
    slot->track_id = -1;
    slot->x = 0;
    slot->y = 0;
    slot->pressed = 0;
}

static void my_touch_down(struct my_touch_slot *slot, int tid, int x, int y)
{
    slot->track_id = tid;
    slot->x = x;
    slot->y = y;
    slot->pressed = 1;
}

// ========== 钩子回调：synaptics_ts_set_touch_function after ==========
static void hook_set_touch_after(hook_args_t *args)
{
    if (!g_touch_idev)
    {
        g_touch_idev = (struct input_dev *)args->arg0;
    }
}
static hook_wrap_t hook_set_touch_wrap = {
    .after = hook_set_touch_after,
};

// ========== 钩子回调：synaptics_ts_parse_touch_report after ==========
static void hook_parse_after(hook_args_t *args)
{
    if (!g_touch_idev)
        return;

    int i;
    for (i = 0; i < MAX_SLOTS; i++)
    {
        struct my_touch_slot *s = &my_touch_dev[i];
        if (!s->pressed)
            continue;

        input_mt_slot(g_touch_idev, s->track_id);
        input_mt_report_slot_state(g_touch_idev, MT_TOOL_FINGER, true);
        input_report_abs(g_touch_idev, ABS_MT_POSITION_X, s->x);
        input_report_abs(g_touch_idev, ABS_MT_POSITION_Y, s->y);
    }
    input_sync(g_touch_idev);
}
static hook_wrap_t hook_parse_wrap = {
    .after = hook_parse_after,
};

static long vt_init(const char *args, const char *event, void *reserved)
{
    int i;
    for (i = 0; i < MAX_SLOTS; i++)
    {
        my_touch_clear_slot(&my_touch_dev[i]);
    }
    // 测试触点 720,1500
    int sid = -1;
    for (i = 0; i < MAX_SLOTS; i++)
    {
        if (my_touch_dev[i].pressed == 0)
        {
            sid = i;
            break;
        }
    }
    if (sid >= 0)
    {
        my_touch_down(&my_touch_dev[sid], 1, 720, 1500);
    }

    // 安装钩子
    hook_set_touch_fn = hook_wrap2(kfunc_ref(synaptics_ts_set_touch_function), &hook_set_touch_wrap);
    hook_parse_report = hook_wrap2(kfunc_ref(synaptics_ts_parse_touch_report), &hook_parse_wrap);
    return 0;
}

static long vt_exit(void *reserved)
{
    if (hook_set_touch_fn)
    {
        unhook(hook_set_touch_fn);
        hook_set_touch_fn = NULL;
    }
    if (hook_parse_report)
    {
        unhook(hook_parse_report);
        hook_parse_report = NULL;
    }

    int i;
    for (i = 0; i < MAX_SLOTS; i++)
    {
        my_touch_clear_slot(&my_touch_dev[i]);
    }
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
