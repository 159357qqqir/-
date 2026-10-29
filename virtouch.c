#include <kpmodule.h>

KPM_NAME("virtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
static kallsyms_lookup_name_t kallsyms_lookup_name;

typedef void (*pr_info_t)(const char *fmt, ...);
static pr_info_t pr_info;

typedef void (*msleep_t)(unsigned int msec);
static msleep_t msleep;

struct input_dev;
struct list_head;

typedef void (*input_report_abs_t)(struct input_dev *, unsigned int, int);
typedef void (*input_sync_t)(struct input_dev *);

static input_report_abs_t input_report_abs;
static input_sync_t input_sync;

#define EV_ABS     0x03
#define EV_KEY     0x01
#define BTN_TOUCH  0x14a
#define ABS_MT_POSITION_X 0x35
#define ABS_MT_POSITION_Y 0x36
#define ABS_MT_TRACKING_ID 0x39

static struct input_dev *g_tp_dev = NULL;

static long vt_init(const char *args, const char *event, void *reserved)
{
    kallsyms_lookup_name = (kallsyms_lookup_name_t)kallsyms_lookup_name("kallsyms_lookup_name");
    if (!kallsyms_lookup_name)
    {
        return -1;
    }

    pr_info = (pr_info_t)kallsyms_lookup_name("pr_info");
    msleep = (msleep_t)kallsyms_lookup_name("msleep");
    input_report_abs = (input_report_abs_t)kallsyms_lookup_name("input_report_abs");
    input_sync = (input_sync_t)kallsyms_lookup_name("input_sync");

    if (!pr_info)
    {
        return -1;
    }

    pr_info("[virtouch] start check symbols\n");

    unsigned long list_head_addr = kallsyms_lookup_name("input_dev_list");
    unsigned long lock_addr = kallsyms_lookup_name("input_dev_list_lock");
    unsigned long unlock_addr = kallsyms_lookup_name("input_dev_list_unlock");

    pr_info("[virtouch] input_dev_list=%lx lock=%lx unlock=%lx\n", list_head_addr, lock_addr, unlock_addr);
    pr_info("[virtouch] input_report_abs=%lx input_sync=%lx\n",
        (unsigned long)input_report_abs, (unsigned long)input_sync);

    if (!list_head_addr || !lock_addr || !unlock_addr)
    {
        pr_info("[virtouch] input dev list symbols missing\n");
        return -1;
    }

    pr_info("[virtouch] symbol found, this is probe only, no touch report\n");
    return 0;
}

// 修复：KPM_EXIT 要求 long (*)(void *)
static long vt_exit(void *reserved)
{
    if (pr_info)
    {
        pr_info("[virtouch] module unload\n");
    }
    g_tp_dev = NULL;
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
