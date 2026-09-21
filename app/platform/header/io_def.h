#ifndef __IO_DEF_H
#define __IO_DEF_H


#include "global.h"
#include "config.h"
#include "sfr.h"


#if (SD0_MAPPING == SD0MAP_G1)
    //SDCMD(PA4), SDCLK(PA3), SDDAT0(PA5)
    #define SD_MUX_IO_INIT()        {GPIOADE |= BIT(4)|BIT(3)|BIT(5);\
                                    GPIOACLR  = BIT(3);\
                                    GPIOADIR &= ~BIT(3);\
                                    GPIOAPU  &= ~BIT(3);\
                                    GPIOADIR |= BIT(4) | BIT(5);\
                                    GPIOAPU  |= BIT(4) | BIT(5);\
                                    GPIOAFEN |= BIT(4) | BIT(5);\
                                    FUNCMCON0 = SD0MAP_G1;}
    #define SD_IO_INIT()            {GPIOADE |= BIT(4)|BIT(3)|BIT(4);\
                                    GPIOADIR &= ~BIT(3);\
                                    GPIOADIR |= BIT(4) | BIT(5);\
                                    GPIOAPU  |= BIT(4) | BIT(5);\
                                    GPIOAFEN |= BIT(4) | BIT(5);\
                                    FUNCMCON0 = SD0MAP_G1;}
    #define SD_CLK_DIR_IN()         {GPIOADIR |= BIT(3); GPIOAPU  |= BIT(3);}
    #define SD_CLK_IN_DIS_PU10K()   {GPIOADIR |= BIT(3); GPIOAPU  &= ~BIT(3);}
    #define SD_CLK_DIR_OUT()        {GPIOAPU  &= ~BIT(3); GPIOADIR &= ~BIT(3);}
    #define SD_MUX_DETECT_INIT()    {GPIOADE |= BIT(3); GPIOAPU  |= BIT(3); GPIOADIR |= BIT(3);}
    #define SD_MUX_IS_ONLINE()      ((GPIOA & BIT(3)) == 0)
    #define SD_MUX_IS_BUSY()        ((GPIOADIR & BIT(3)) == 0)
    #define SD_MUX_CMD_IS_BUSY()    (GPIOAPU500K & BIT(4))
    #define SD_CMD_MUX_PU300R()     {GPIOAPU500K |= BIT(4); GPIOAPU &= ~BIT(4);}
    #define SD_CMD_MUX_PU10K()      {GPIOAPU |= BIT(4); GPIOAPU500K &= ~BIT(4);}
    #define SD_DAT_MUX_PU300R()     {GPIOAPU500K |= BIT(5); GPIOAPU &= ~BIT(5);}
    #define SD_DAT_MUX_PU10K()      {GPIOAPU |= BIT(5); GPIOAPU500K &= ~BIT(5);}
    #define SD_CMD_MUX_IS_ONLINE()  ((GPIOA & BIT(4)) == 0)


    #define SD_CLK_OUT_H()          {GPIOASET = BIT(3);}
    #define SD_CLK_OUT_L()          {GPIOACLR = BIT(3);}
    #define SD_CLK_STA()            (GPIOA & BIT(3))

    #define SD_DAT_DIR_OUT()        {GPIOADE |= BIT(5); GPIOADIR &= ~BIT(5);}
    #define SD_DAT_DIR_IN()         {GPIOADIR |= BIT(5); GPIOAPU  |= BIT(5);}
    #define SD_DAT_OUT_H()          {GPIOASET = BIT(5);}
    #define SD_DAT_OUT_L()          {GPIOACLR = BIT(5);}
    #define SD_DAT_STA()            (GPIOA & BIT(5))

    #define SD_CMD_DIR_OUT()        {GPIOADE |= BIT(4); GPIOADIR &= ~BIT(4);}
    #define SD_CMD_DIR_IN()         {GPIOADIR |= BIT(4); GPIOAPU  |= BIT(4);}
    #define SD_CMD_OUT_H()          {GPIOASET = BIT(4);}
    #define SD_CMD_OUT_L()          {GPIOACLR = BIT(4);}
    #define SD_CMD_STA()            (GPIOA & BIT(4))

    #define SD_DAT_DIS_UP()
    #define SD_DAT_RES_UP()

    #define SDCLK_IO                IO_PA3
    #define SDCMD_IO                IO_PA4
    #define SDDAT_IO                IO_PA5

#elif (SD0_MAPPING == SD0MAP_G2)
    //SDCMD(PA9), SDCLK(PA8), SDDAT0(PA10)
    #define SD_MUX_IO_INIT()        {GPIOADE |= BIT(9)|BIT(8)|BIT(10);\
                                    GPIOACLR  = BIT(8);\
                                    GPIOADIR &= ~BIT(8);\
                                    GPIOAPU  &= ~BIT(8);\
                                    GPIOADIR |= BIT(9) | BIT(10);\
                                    GPIOAPU  |= BIT(9) | BIT(10);\
                                    GPIOAFEN |= BIT(9) | BIT(10);\
                                    FUNCMCON0 = SD0MAP_G2;}
    #define SD_IO_INIT()            {GPIOADE |= BIT(9)|BIT(8)|BIT(10);\
                                    GPIOADIR &= ~BIT(8);\
                                    GPIOADIR |= BIT(9) | BIT(10);\
                                    GPIOAPU  |= BIT(9) | BIT(10);\
                                    GPIOAFEN |= BIT(9) | BIT(10);\
                                    FUNCMCON0 = SD0MAP_G2;}
    #define SD_CLK_DIR_IN()         {GPIOADIR |= BIT(8); GPIOAPU  |= BIT(8);}
    #define SD_CLK_IN_DIS_PU10K()   {GPIOADIR |= BIT(8); GPIOAPU  &= ~BIT(8);}
    #define SD_CLK_DIR_OUT()        {GPIOAPU  &= ~BIT(8); GPIOADIR &= ~BIT(8);}
    #define SD_MUX_DETECT_INIT()    {GPIOADE |= BIT(8); GPIOAPU  |= BIT(8); GPIOADIR |= BIT(8);}
    #define SD_MUX_IS_ONLINE()      ((GPIOA & BIT(8)) == 0)
    #define SD_MUX_IS_BUSY()        ((GPIOADIR & BIT(8)) == 0)
    #define SD_MUX_CMD_IS_BUSY()    (GPIOAPU500K & BIT(9))
    #define SD_CMD_MUX_PU300R()     {GPIOAPU500K |= BIT(9); GPIOAPU &= ~BIT(9);}
    #define SD_CMD_MUX_PU10K()      {GPIOAPU |= BIT(9); GPIOAPU500K &= ~BIT(9);}
    #define SD_DAT_MUX_PU300R()     {GPIOAPU500K |= BIT(10); GPIOAPU &= ~BIT(10);}
    #define SD_DAT_MUX_PU10K()      {GPIOAPU |= BIT(10); GPIOAPU500K &= ~BIT(10);}
    #define SD_CMD_MUX_IS_ONLINE()  ((GPIOA & BIT(9)) == 0)


    #define SD_CLK_OUT_H()          {GPIOASET = BIT(8);}
    #define SD_CLK_OUT_L()          {GPIOACLR = BIT(8);}
    #define SD_CLK_STA()            (GPIOA & BIT(8))

    #define SD_DAT_DIR_OUT()        {GPIOADE |= BIT(10); GPIOADIR &= ~BIT(10);}
    #define SD_DAT_DIR_IN()         {GPIOADIR |= BIT(10); GPIOAPU  |= BIT(10);}
    #define SD_DAT_OUT_H()          {GPIOASET = BIT(10);}
    #define SD_DAT_OUT_L()          {GPIOACLR = BIT(10);}
    #define SD_DAT_STA()            (GPIOA & BIT(10))

    #define SD_CMD_DIR_OUT()        {GPIOADE |= BIT(9); GPIOADIR &= ~BIT(9);}
    #define SD_CMD_DIR_IN()         {GPIOADIR |= BIT(9); GPIOAPU  |= BIT(9);}
    #define SD_CMD_OUT_H()          {GPIOASET = BIT(9);}
    #define SD_CMD_OUT_L()          {GPIOACLR = BIT(9);}
    #define SD_CMD_STA()            (GPIOA & BIT(9))

    #define SD_DAT_DIS_UP()
    #define SD_DAT_RES_UP()

    #define SDCLK_IO                IO_PA8
    #define SDCMD_IO                IO_PA9
    #define SDDAT_IO                IO_PA10

#else
    #define SD_MUX_IO_INIT()
    #define SD_IO_INIT()
    #define SD_CLK_DIR_IN()
    #define SD_CLK_IN_DIS_PU10K()
    #define SD_CLK_DIR_OUT()
    #define SD_MUX_DETECT_INIT()
    #define SD_MUX_IS_ONLINE()      0
    #define SD_MUX_IS_BUSY()        0
    #define SD_MUX_CMD_IS_BUSY()    0
    #define SD_CMD_MUX_PU300R()
    #define SD_CMD_MUX_PU10K()
    #define SD_CMD_MUX_IS_ONLINE()  0

    #define SD_CLK_OUT_H()
    #define SD_CLK_OUT_L()
    #define SD_CLK_STA()            0

    #define SD_DAT_DIR_OUT()
    #define SD_DAT_DIR_IN()
    #define SD_DAT_OUT_H()
    #define SD_DAT_OUT_L()
    #define SD_DAT_STA()            0

    #define SD_CMD_DIR_OUT()
    #define SD_CMD_DIR_IN()
    #define SD_CMD_OUT_H()
    #define SD_CMD_OUT_L()
    #define SD_CMD_STA()            0

    #define SD_DAT_DIS_UP()
    #define SD_DAT_RES_UP()

    #define SDCLK_IO                IO_NONE
    #define SDCMD_IO                IO_NONE
    #define SDDAT_IO                IO_NONE
#endif

#endif //__IO_DEF_H
