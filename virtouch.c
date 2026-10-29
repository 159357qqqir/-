#include <kpm.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

static struct input_dev *vt_dev;
static struct cdev vt_cdev;
static dev_t vt_devno;
static struct mutex vt_mtx;

static ssize_t vt_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    u8 pkt[7];
    if (count != sizeof(pkt))
        return -EINVAL;
    if (copy_from_user(pkt, buf, sizeof(pkt)))
        return -EFAULT;

    u8 tip    = pkt[0] & 1;
    u8 cid    = (pkt[0] >> 1) & 0x0F;
    u16 x     = (pkt[2] << 8) | pkt[1];
    u16 y     = (pkt[4] << 8) | pkt[3];
    u8 press  = pkt[5];

    mutex_lock(&vt_mtx);
    input_mt_slot(vt_dev, cid);
    if (tip) {
        input_mt_report_slot_state(vt_dev, MT_TOOL_FINGER, true);
        input_report_abs(vt_dev, ABS_MT_POSITION_X, x);
        input_report_abs(vt_dev, ABS_MT_POSITION_Y, y);
        input_report_abs(vt_dev, ABS_MT_PRESSURE, press);
    } else {
        input_mt_report_slot_state(vt_dev, MT_TOOL_FINGER, false);
    }
    input_mt_sync_frame(vt_dev);
    input_sync(vt_dev);
    mutex_unlock(&vt_mtx);
    return count;
}

static const struct file_operations vt_fops = {
    .write = vt_write,
};

KPM_INIT()
{
    int ret;
    mutex_init(&vt_mtx);

    vt_dev = input_allocate_device();
    if (!vt_dev) {
        kpm_err("allocate input dev fail\n");
        return -ENOMEM;
    }
    vt_dev->name = "ap_virt_touch";
    vt_dev->phys = "apatch/vtouch";
    vt_dev->id.bustype = BUS_VIRTUAL;

    __set_bit(EV_ABS, vt_dev->evbit);
    __set_bit(ABS_MT_POSITION_X, vt_dev->absbit);
    __set_bit(ABS_MT_POSITION_Y, vt_dev->absbit);
    __set_bit(ABS_MT_PRESSURE, vt_dev->absbit);
    __set_bit(ABS_MT_TRACKING_ID, vt_dev->absbit);

    input_set_abs_params(vt_dev, ABS_MT_POSITION_X, 0, 4095, 0, 0);
    input_set_abs_params(vt_dev, ABS_MT_POSITION_Y, 0, 4095, 0, 0);
    input_set_abs_params(vt_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
    input_set_abs_params(vt_dev, ABS_MT_TRACKING_ID, 0, 4, 0, 0);

    ret = input_register_device(vt_dev);
    if (ret) {
        input_free_device(vt_dev);
        kpm_err("register input dev fail\n");
        return ret;
    }

    alloc_chrdev_region(&vt_devno, 0, 1, "vtouch");
    cdev_init(&vt_cdev, &vt_fops);
    ret = cdev_add(&vt_cdev, vt_devno, 1);
    if (ret) {
        unregister_chrdev_region(vt_devno,1);
        input_unregister_device(vt_dev);
        input_free_device(vt_dev);
        kpm_err("cdev add fail\n");
        return ret;
    }
    kpm_info("✅ virtouch kpm loaded, /dev/vtouch\n");
    return 0;
}

KPM_EXIT()
{
    cdev_del(&vt_cdev);
    unregister_chrdev_region(vt_devno,1);
    input_unregister_device(vt_dev);
    input_free_device(vt_dev);
    kpm_info("virtouch kpm unloaded\n");
}
