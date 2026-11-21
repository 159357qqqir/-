#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Force SELinux permissive for 6.1 GKI");
MODULE_AUTHOR("Self");

#define ENFORCING_OFFSET 0x0

static struct selinux_state *selinux_state_ptr = NULL;

static int __init permissive_init(void)
{
    selinux_state_ptr = (void *)kallsyms_lookup_name("selinux_state");
    if (!selinux_state_ptr)
    {
        pr_err("[selinux_perm] selinux_state symbol not found!\n");
        return -ENODEV;
    }

    *(bool *)((char *)selinux_state_ptr + ENFORCING_OFFSET) = false;
    pr_info("[selinux_perm] Set SELinux permissive SUCCESS");
    return 0;
}

static void __exit permissive_exit(void)
{
    if (selinux_state_ptr)
    {
        *(bool *)((char *)selinux_state_ptr + ENFORCING_OFFSET) = true;
        pr_info("[selinux_perm] Restore SELinux enforcing");
    }
}

module_init(permissive_init);
module_exit(permissive_exit);
