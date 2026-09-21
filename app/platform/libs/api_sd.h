#ifndef _API_SD_H
#define _API_SD_H

enum {
    RW_IDLE,            //空闲状态
    RW_READ,            //读状态
    RW_WRITE,           //写状态
};

#define sd_insert()                 sd0_insert(0)
#define sd_remove()                 sd0_remove(0)
#define sd1_insert()                sd0_insert(1)
#define sd1_remove()                sd0_remove(1)

void sd_disk_init(void);
void sd0_insert(u8 index);
void sd0_remove(u8 index);

bool sd0_read(void *buf, u32 lba);
bool sd0_write(void* buf, u32 lba);
bool sd0_stop(bool type);            //复用SDCLK检测时type: 0->sdclk设置为输出， 1->sdclk设置为输入上拉，用于检测
bool sd0_init(void);
bool sd_soft_detect(void);
void sd0_go_idle_state(void);
u8 sd0_get_rw_sta(void);
void ude_sd_remove(void);
void sd_other_param_set(u32 param);   //dev_init后调用, 补充设置一些sd参数
void sd_disable_user(void);
void sd_enable_user(void);
void sd0_set_invalid_state(void);

u32 sd_music_get_file_total(void);
void sd_hidden_start_sector_init(void);
void sd_sector_read_next(void);
u32 sd_sector_get_remain(void);
void sd_hidden_read(void *buf, u32 addr, u32 len);
void sd_hidden_music_num_kick(u32 num);
#endif // _API_SD_H
