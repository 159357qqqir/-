#ifndef __KPMODULE_H__
#define __KPMODULE_H__

#include <stdint.h>

typedef int (*kpm_init_t)(const char *args, const char *event, void *reserved);
typedef void (*kpm_exit_t)(void *reserved);

#define KPM_MODULE_INFO(_name, _ver, _lic) \
    static const char *__kpm_name __attribute__((used, section(".kpm.info.name"))) = _name; \
    static const char *__kpm_ver __attribute__((used, section(".kpm.info.version"))) = _ver; \
    static const char *__kpm_lic __attribute__((used, section(".kpm.info.license"))) = _lic;

#define KPM_INIT(_func) \
    static const kpm_init_t __kpm_init __attribute__((used, section(".kpm.init"))) = _func;

#define KPM_EXIT(_func) \
    static const kpm_exit_t __kpm_exit __attribute__((used, section(".kpm.exit"))) = _func;

#endif
