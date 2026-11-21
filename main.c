#include <compiler.h>
#include <kpmodule.h>
#include <kputils.h>

KPM_NAME("kpm-hello");
KPM_VERSION("1.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("test");
KPM_DESCRIPTION("hello kpm");

static long hello_init(const char *args, const char *event, void *reserved)
{
    logkd("Hello KPM! \n");
    return 0;
}

KPM_INIT(hello_init);
