#ifndef __PARTOOL_H_049D6308912C0CFE63093A469A2476FB__
#define __PARTOOL_H_049D6308912C0CFE63093A469A2476FB__
/******************************************************************************/
#ifdef __BOOT__

extern int
boot_partool_show_byname(int argc, char *argv[],void *buf);

extern int
boot_partool_new(int argc, char *argv[]);

extern void
boot_partool_clean(void);

#endif
/******************************************************************************/
#endif /* __PARTOOL_H_049D6308912C0CFE63093A469A2476FB__ */
