#include "kp/kp.h"
static int kp_init(void) {
    kp_log("Hello APatch KPM!");
    return 0;
}
static void kp_exit(void) {
    kp_log("KPM unload ok");
}
KP_MODULE("demo_kpm", kp_init, kp_exit);
