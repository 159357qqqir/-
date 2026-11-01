//main.c
#include "kpmodule.h"

static int my_init(const char *args, const char *event, void *reserved)
{
    return 0;
}

static void my_exit(void *reserved)
{
}

KPM_MODULE_INFO("pnii", "1.0", "Apache-2.0");
KPM_INIT(my_init);
KPM_EXIT(my_exit);
