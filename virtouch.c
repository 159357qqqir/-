#include <kpmodule.h>
#include <linux/input.h>
#include <linux/delay.h>

KPM_NAME("virtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
static kallsyms_lookup_name_t kallsyms_lookup_name;

typedef struct input_dev *(*input_allocate_device_t)(void);
static input_allocate_device_t input_allocate_device;

typedef int (*input_register_device_t)(struct input_dev *);
static input_register_device_t input_register_device;

typedef void (*input_unregister_device_t)(struct input_dev *);
static input_unregister_device_t input_unregister_device;

typedef void (*input_free_device_t)(struct input_dev *);
static input_free_device_t input_free_device;

typedef void (*input_set_capability_t)(struct input_dev *, unsigned int, unsigned int, unsigned long);
static input_set_capability_t input_set_capability;

typedef void (*input_report_abs_t)(struct input_dev *, unsigned int, int);
static input_report_abs_t input_report_abs;

typedef void (*input_sync_t)(struct input_dev *);
static input_sync_t input_sync;

typedef void (*input_set_abs_params_t)(struct input_dev *, unsigned int, int, int, int, int);
static input_set_abs_params_t input_set_abs_params;

static struct input_dev *vdev;

static long vt_init(const char *args, const char *event, void *reserved)
{
    pr_info("[virtouch] start check symbols\n");

    kallsyms_lookup_name = (kallsyms_lookup_name_t)kallsyms_lookup_name("kallsyms_lookup_name");
    if (!kallsyms_lookup_name)
    {
        pr_info("[virtouch] kallsyms_lookup_name not found!\n");
        return -1;
    }

    unsigned long s1 = kallsyms_lookup_name("input_allocate_device");
    unsigned long s2 = kallsyms_lookup_name("input_register_device");
    unsigned long s3 = kallsyms_lookup_name("input_unregister_device");
    unsigned long s4 = kallsyms_lookup_name("input_free_device");
    unsigned long s5 = kallsyms_lookup_name("input_set_capability");
    unsigned long s6 = kallsyms_lookup_name("input_report_abs");
    unsigned long s7 = kallsyms_lookup_name("input_sync");
    unsigned long s8 = kallsyms_lookup_name("input_set_abs_params");

    pr_info("[virtouch] input_allocate_device = %lx\n", s1);
    pr_info("[virtouch] input_register_device = %lx\n", s2);
    pr_info("[virtouch] input_unregister_device = %lx\n", s3);
    pr_info("[virtouch] input_free_device = %lx\n", s4);
    pr_info("[virtouch] input_set_capability = %lx\n", s5);
    pr_info("[virtouch] input_report_abs = %lx\n", s6);
    pr_info("[virtouch] input_sync = %lx\n", s7);
    pr_info("[virtouch] input_set_abs_params = %lx\n", s8);

    input_allocate_device = (input_allocate_device_t)s1;
    input_register_device = (input_register_device_t)s2;
    input_unregister_device = (input_unregister_device_t)s3;
    input_free_device = (input_free_device_t)s4;
    input_set_capability = (input_set_capability_t)s5;
    input_report_abs = (input_report_abs_t)s6;
    input_sync = (input_sync_t)s7;
    input_set_abs_params = (input_set_abs_params_t)s8;

    if (!input_allocate_device || !input_register_device || !input_report_abs || !input_sync)
    {
        pr_info("[virtouch] some input symbol missing!\n");
        return -1;
    }

    vdev = input_allocate_device();
    if (!vdev)
    {
        pr_info("[virtouch] allocate input dev fail\n");
        return -1;
    }

    vdev->name = "virtouch-kpm";

    __set_bit(EV_ABS, vdev->evbit);
    __set_bit(EV_KEY, vdev->evbit);
    __set_bit(BTN_TOUCH, vdev->keybit);

    input_set_abs_params(vdev, ABS_MT_POSITION_X, 0, 1440, 0, 0);
    input_set_abs_params(vdev, ABS_MT_POSITION_Y, 0, 3120, 0, 0);
    input_set_abs_params(vdev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);

    if (input_register_device(vdev))
    {
        pr_info("[virtouch] register dev failed\n");
        input_free_device(vdev);
        vdev = NULL;
        return -1;
    }
    pr_info("[virtouch] device ready, try touch down at (720,1500)\n");

    input_report_abs(vdev, ABS_MT_TRACKING_ID, 0);
    input_report_abs(vdev, ABS_MT_POSITION_X, 720);
    input_report_abs(vdev, ABS_MT_POSITION_Y, 1500);
    input_report_key(vdev, BTN_TOUCH, 1);
    input_sync(vdev);

    msleep(150);

    input_report_abs(vdev, ABS_MT_TRACKING_ID, -1);
    input_report_key(vdev, BTN_TOUCH, 0);
    input_sync(vdev);

    pr_info("[virtouch] touch event sent\n");
    return 0;
}

static void vt_exit(void)
{
    pr_info("[virtouch] module unload\n");
    if (vdev)
    {
        input_unregister_device(vdev);
        input_free_device(vdev);
        vdev = NULL;
    }
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
