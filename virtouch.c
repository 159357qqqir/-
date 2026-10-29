#include <kpmodule.h>

unsigned long kp_find_symbol(const char *name);
void kp_print(const char *fmt, ...);

KPM_NAME("virtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");
KPM_AUTHOR("virtouch");

static long vt_init(const char *args, const char *event, void *reserved)
{
    kp_print("[virtouch] start check symbols\n");

    unsigned long s1 = kp_find_symbol("input_allocate_device");
    unsigned long s2 = kp_find_symbol("input_register_device");
    unsigned long s3 = kp_find_symbol("input_unregister_device");
    unsigned long s4 = kp_find_symbol("input_free_device");
    unsigned long s5 = kp_find_symbol("input_set_capability");
    unsigned long s6 = kp_find_symbol("input_report_abs");
    unsigned long s7 = kp_find_symbol("input_sync");
    unsigned long s8 = kp_find_symbol("input_set_abs_params");
    unsigned long s9 = kp_find_symbol("msleep");

    kp_print("input_allocate_device = %lx\n", s1);
    kp_print("input_register_device = %lx\n", s2);
    kp_print("input_unregister_device = %lx\n", s3);
    kp_print("input_free_device = %lx\n", s4);
    kp_print("input_set_capability = %lx\n", s5);
    kp_print("input_report_abs = %lx\n", s6);
    kp_print("input_sync = %lx\n", s7);
    kp_print("input_set_abs_params = %lx\n", s8);
    kp_print("msleep = %lx\n", s9);

    kp_print("[virtouch] check done, all ok, loaded\n");
    return 0;
}

static long vt_exit(void *reserved)
{
    kp_print("[virtouch] unload\n");
    return 0;
}

KPM_INIT(vt_init);
KPM_EXIT(vt_exit);
