
#ifndef KPMODULE_H
#define KPMODULE_H

#include <stdint.h>

#define KPM_SEC_INFO    ".kpm.info"
#define KPM_SEC_INIT    ".kpm.init"
#define KPM_SEC_EXIT    ".kpm.exit"

#define KPM_NAME(s)         static const char kpm_name[] __attribute__((section(KPM_SEC_INFO))) = "name=" s
#define KPM_VERSION(s)      static const char kpm_ver[] __attribute__((section(KPM_SEC_INFO))) = "version=" s
#define KPM_AUTHOR(s)       static const char kpm_author[] __attribute__((section(KPM_SEC_INFO))) = "author=" s
#define KPM_DESCRIPTION(s)  static const char kpm_desc[] __attribute__((section(KPM_SEC_INFO))) = "desc=" s

typedef void (*kpm_exit_fn)(void);
typedef int (*kpm_init_fn)(void);

#define KPM_INIT(func)      static kpm_init_fn __kpm_init __attribute__((section(KPM_SEC_INIT), used)) = func
#define KPM_EXIT(func)      static kpm_exit_fn __kpm_exit __attribute__((section(KPM_SEC_EXIT), used)) = func

#endif
