#include <kpmodule.h>

KPM_NAME("virtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

#define MAX_SLOTS 10

struct my_touch_slot {
    int track_id;
    int x;
    int y;
    int pressed;
};

// 共享区域，hook模块会读取这块内存
static struct my_touch_slot my_touch_dev[MAX_SLOTS];

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

static void my_touch_up(struct my_touch_slot *slot)
{
    slot->pressed = 0;
}

static int my_touch_get_free_slot(void)
{
    int i;
    for (i = 0; i < MAX_SLOTS; i++)
    {
        if (my_touch_dev[i].pressed == 0)
            return i;
    }
    return -1;
}

static long vt_init(const char *args, const char *event, void *reserved)
{
    int i;
    for (i = 0; i < MAX_SLOTS; i++)
    {
        my_touch_clear_slot(&my_touch_dev[i]);
    }
    // 测试触点 (720,1500)
    int sid = my_touch_get_free_slot();
    if (sid >= 0)
    {
        my_touch_down(&my_touch_dev[sid], 1, 720, 1500);
    }
    return 0;
}

static long vt_exit(void *reserved)
{
    int i;
    for (i = 0; i < MAX_SLOTS; i++)
    {
        my_touch_clear_slot(&my_touch_dev[i]);
    }
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);

// KPModule 正确导出符号方式，放在文件末尾
KPM_EXPORT(my_touch_dev);
