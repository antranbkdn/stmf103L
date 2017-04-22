#ifndef __APP_H__
#define __APP_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef  void (*p_jump_func)(void);

#define NORMAL_START_ADDRESS			APP_START_ADDR

extern int boot_main();

#ifdef __cplusplus
}
#endif

#endif //__APP_H__
