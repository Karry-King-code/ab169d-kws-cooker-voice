#include "config.h"
setid(11111111-1111-1111-1111-111111111111);
setflash(1, FLASH_SIZE, FLASH_ERASE_4K, FLASH_DUAL_READ, FLASH_QUAD_READ);
setauth(0x3CC9B441,zbit_auth_private_test123);
//setspace(0x2000);
#if (TDNN_HW_EN && ASR_RECOG_EN)
    #if (ASR_LIBS_SEL == ASR_ZH)
        #if (WEIGHT_MINI_EN == 0)
        setuserbin(WEIGHT_BIN_ADDR, WEIGHT_BIN_LEN, weight.bin, SETUSERBIN_PARAM3);
        #else
        setuserbin(WEIGHT_MINI_BIN_ADDR, WEIGHT_MINI_BIN_LEN, weight_mini.bin, SETUSERBIN_PARAM3);
        #endif
    #elif (ASR_LIBS_SEL == ASR_EN)
        setuserbin(WEIGHT_BIN_ADDR, WEIGHT_EN_BIN_LEN, weight_en.bin, SETUSERBIN_PARAM3);
    #endif
#endif
#if USER_BOOT_EN
setheader(header.bin);
setxcfg(xcfg.bin);
setuserboot(userboot.bin, USER_BOOT_SIZE); //user_boot的app.bin的大小
setuserbootfota(0);                        //单备份升级方案必须添加该语句，导出的FOT文件中不带userboot.bin
setcode(app.bin, 0x20000);                 //先USER_BOOT_EN置0，编译，再设置>=app.bin大小，对齐4KB
setres(res.bin, 0x2000);                   //设置>=res.bin大小，对齐4KB
makedcf(app.dcf);
#else
make(dcf_buf, header.bin, app.bin, res.bin, xcfg.bin, updater.bin);
save(dcf_buf, app.dcf);
#endif


//------------------------app.xm_cmd------------------------
//**setflash(param0, param1, param2, param3, param4);
//说明：     设置用户资源区
//param0:    type:默认1，代表flash
//param1:    FLASH_SIZE:flash大小
//param2:    FLASH_ERASE_4K:是否支持4K擦除
//param3:    FLASH_DUAL_READ:是否支持两线读取
//param4:    FLASH_QUAD_READ:是否支持4线读取
//示例：     setflash(1, FLASH_SIZE, FLASH_ERASE_4K, FLASH_DUAL_READ, FLASH_QUAD_READ)

//**setspace(param0)
//说明：    设置参数区大小
//param0:   size:参数区大小
//示例：    setspace(0x5000)

//**setuserbin(param0,param1,param2,param3)
//说明：     设置用户资源区
//param0:    start_addr:用户资源区起始地址
//param1:    bin_size:用户资源区大小
//param2:    bin_file:用户资源文件
//param3:    type:0:导出fot文件不带资源 1：导出fot文件携带压缩资源
//                2:导出fot携带资源，资源不压缩
//示例：     setuserbin(0x87000,0x304000, ui.bin,1);

//**setunpack(param0)
//说明：          设置FOTA压缩升级引导loader
//param0:file:    同一目录下bin文件
//示例：          setunpack(unpack.bin)

//**setpkgarea(param0,param1)
//说明：     设置FOTA压缩升级压缩包存放信息
//param0:    start_addr:压缩包存放起始地址
//param1:    end_addr:压缩包大小
//示例：     setunpack(0x400000,0x300000)
//压缩包存放起始地址为0x400000,最大为0x300000
//------------------------app.xm_cmd------------------------
