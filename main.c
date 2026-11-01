
#include "kpm.h"

KPM_NAME("my_kpm");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");

KPM_INIT
int __kpm_init(void)
{
    return 0;
}

KPM_EXIT
void __kpm_exit(void)
{
}
