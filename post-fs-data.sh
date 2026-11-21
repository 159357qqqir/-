#!/system/bin/sh
MODDIR=${0%/*}
KO_PATH="${MODDIR}/selinux_perm.ko"
LOG_FILE="/data/adb/selinux_perm.log"

if [ -f /data/adb/disable_selinux_perm ]; then
    echo "[$(date)] 检测到禁用标记，跳过加载" >> $LOG_FILE
    exit 0
fi

echo "[$(date)] 尝试加载selinux_perm.ko" >> $LOG_FILE
if [ -f "$KO_PATH" ]; then
    insmod "$KO_PATH"
    echo "[$(date)] insmod 执行完成" >> $LOG_FILE
else
    echo "[$(date)] 错误：找不到ko文件" >> $LOG_FILE
fi
