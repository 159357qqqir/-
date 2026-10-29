#include "kpmodule.h"

KPM_INIT()
{
    kpm_log("virtouch module loaded!\n");
    return 0;
}

KPM_EXIT()
{
    kpm_log("virtouch module unloaded!\n");
    return 0;
}
