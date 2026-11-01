
#ifndef __KPM_H
#define __KPM_H
#include <stdint.h>
#include <stddef.h>

#define KPM_NAME(x)     const char *__kpm_name __attribute__((used,section(".kpm.name"))) = x
#define KPM_VERSION(x)  const char *__kpm_version __attribute__((used,section(".kpm.version"))) = x
#define KPM_LICENSE(x)  const char *__kpm_license __attribute__((used,section(".kpm.license"))) = x

#define KPM_INIT  __attribute__((used,section(".kpm.init")))
#define KPM_EXIT  __attribute__((used,section(".kpm.exit")))
#endif
