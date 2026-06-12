#ifndef __PLAT_LOTUS_H__
#define __PLAT_LOTUS_H__

/* Console utility functions */
void lotus_console_boot_init(void);
void lotus_console_boot_end(void);
void lotus_console_runtime_init(void);
void lotus_console_runtime_end(void);

/* Gic interface */
void plat_lotus_gic_save(void);
void plat_lotus_gic_restore(void);

uint8_t cpu_get_rev_var(void);

#endif /* __PLAT_LOTUS_H__ */
