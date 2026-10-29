#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/thermal.h>

typedef int (*__thermal_zone_get_temp_t)(struct thermal_zone_device *, int *);
static __thermal_zone_get_temp_t orig_func = (__thermal_zone_get_temp_t *)0xFFFFFFC008C1CD04;

static int __init test_kcfi_init(void)
{
    int temp;
    pr_info("KPM test: try call thermal_zone_get_temp\n");
    if (orig_func) {
        // 注意：这里传NULL tz，大概率直接oops，只是测试编译通过！
        int ret = orig_func(NULL, &temp);
        pr_info("ret:%d temp:%d\n", ret, temp);
    }
    return 0;
}

static void __exit test_kcfi_exit(void)
{
    pr_info("test exit\n");
}

module_init(test_kcfi_init);
module_exit(test_kcfi_exit);
MODULE_LICENSE("GPL");
