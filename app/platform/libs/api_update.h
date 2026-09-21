#ifndef _API_UPDATE_H
#define _API_UPDATE_H

int updatefile_init(const char *file);      //初始化UPDATE模块
void updateproc(void);                      //升级流程
void ota_enter(void);                       //进入OTA流程
void ota_exit(void);                        //退出OTA流程

#endif
