#ifndef __KPMODULE_H__
#define __KPMODULE_H__

#include <stdint.h>

typedef int (*kpm_init_t)(void);
typedef void (*kpm_exit_t)(void);

struct kpm_info {
    const char *name;
    const char *version;
    const char *license;
    kpm_init_t init;
    kpm_exit_t exit;
};

#define KPM_MODULE(_name, _ver, _lic, _init, _exit) \
    static const struct kpm_info __kpm_info \
    __attribute__((used, section(".kpm.info"))) = { \
        .name = _name, \
        .version = _ver, \
        .license = _lic, \
        .init = _init, \
        .exit = _exit, \
    }

#endif
