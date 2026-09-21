#include "app_timer.h"
#include "log.h"


static volatile void (*p2ms_fun[P2MS_FUN_COUNT])(void) = {0};


/**
 * @brief 2ms调用一次
 *
 */
static void app_timer_loop(void *priv)
{

    for(int i = 0; i < P2MS_FUN_COUNT; i++)
    {
        if(p2ms_fun[i]) p2ms_fun[i]();
    }

    return;
}


#if 0
typedef struct _app_timer{
    uint32_t tick;
    uint32_t ms2;
    void (*fun)(void);  //just for rising some flags
}app_timer_t;


inline void app_timer_clear(uint8_t timer)
{
    app_timer[timer].tick = 0;

    return;
}

void app_timer_regist(uint8_t timer, uint32_t ms2, void (*fun)(void))
{
    DEBUG_PRINT("\r\n timer: %d\r\n ms2: %d\r\n fun: %p\r\n", timer, ms2, fun);
    app_timer[timer].ms2 = ms2;
    app_timer[timer].fun = fun;
    app_timer[timer].tick = 0;

    return;
}

void app_timer_unregist(uint8_t timer)
{
    // app_timer[timer].ms2 = 0;
    // app_timer[timer].fun = NULL;
    // app_timer[timer].tick = 0;
    memset(&app_timer[timer], 0, sizeof(app_timer[timer]));

    return;
}

#endif



inline void regist_2ms_fun(uint8_t index, void(*fun)(void))
{
    //printf("\r\n app fun index: %d\r\n fun address: %p\r\n", index, fun);
    p2ms_fun[index] = fun;

    return;
}

inline void unregist_2ms_fun(uint8_t index)
{
    p2ms_fun[index] = NULL;

    return;
}

static u16 app_timer_id = 0;
void app_timer_init()
{
    app_timer_id = usr_timer_add(NULL, app_timer_loop, 2, 1);
    //printf("app_timer_id = %d\n",app_timer_id);
}
