#ifndef KPMODULE_H
#define KPMODULE_H

#define KPM_SEC_INFO   __attribute__((section(".kpm_info")))
#define KPM_SEC_INIT   __attribute__((section(".kpm_init")))
#define KPM_SEC_EXIT   __attribute__((section(".kpm_exit")))

#define KPM_NAME(n)     const char kpm_name[] KPM_SEC_INFO = "name=" n
#define KPM_VERSION(v)  const char kpm_ver[] KPM_SEC_INFO = "version=" v
#define KPM_AUTHOR(a)   const char kpm_author[] KPM_SEC_INFO = "author=" a
#define KPM_DESCRIPTION(d) const char kpm_desc[] KPM_SEC_INFO = "desc=" d

#define KPM_INIT(f)     void (*const kpm_init)(void) KPM_SEC_INIT = f
#define KPM_EXIT(f)     void (*const kpm_exit)(void) KPM_SEC_EXIT = f

#endif
