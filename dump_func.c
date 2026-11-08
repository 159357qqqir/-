#include "kpm.h"

KPM_INIT()
{
    void *addr = kpm_find_symbol("sec_bat_thermal_check");
    if (!addr)
    {
        kpm_log("[DUMP] symbol not found\n");
        return;
    }
    kpm_log("[DUMP] sec_bat_thermal_check = %p\n", addr);

    uint8_t buf[64];
    int ret = kpm_mem_read(addr, buf, sizeof(buf));
    if (ret == 0)
    {
        kpm_log("[DUMP] hex: ");
        for(int i=0;i<64;i++){
            kpm_log_cont("%02x ", buf[i]);
        }
        kpm_log_cont("\n");
    } else {
        kpm_log("[DUMP] read memory fail\n");
    }
}

KPM_EXIT()
{
    kpm_log("[DUMP] unload\n");
}
