#include "kpmodule.h"

static int my_init(void)
{
    return 0;
}

static void my_exit(void)
{
}

KPM_MODULE("pnii", "1.0", "Apache-2.0", my_init, my_exit);
