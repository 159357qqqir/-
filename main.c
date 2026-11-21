#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <kputils.h>

// 模块基础信息（APatch 识别必需）
KPM_NAME("kpm-hello-demo");
KPM_VERSION("1.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("demo");
KPM_DESCRIPTION("APatch KPM Hello World Demo");

// 模块加载入口（无 __user）
static long hello_init(const char *args, const char *event, void *reserved)
{
    pr_info("[kpm-hello] init success! event: %s, args: %s\n", event, args);
    pr_info("[kpm-hello] KernelPatch version: 0x%x\n", kpver);
    return 0;
}

// 控制回调（out_msg 为用户态指针，需加 __user）
static long hello_control0(const char *args, char *__user out_msg, int outlen)
{
    char echo[64] = "echo from kernel: ";
    strncat(echo, args, 48);
    compat_copy_to_user(out_msg, echo, sizeof(echo));
    return 0;
}

// 模块卸载出口（无 __user）
static long hello_exit(void *reserved)
{
    pr_info("[kpm-hello] exit success\n");
    return 0;
}

// 注册生命周期（固定格式）
KPM_INIT(hello_init);
KPM_CTL0(hello_control0);
KPM_EXIT(hello_exit);
