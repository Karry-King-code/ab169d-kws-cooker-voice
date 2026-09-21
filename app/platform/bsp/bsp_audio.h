#ifndef __BSP_AUDIO_H__
#define __BSP_AUDIO_H__

#define AUDIO_PATH_SPEAKER         0

void audio_path_init(u8 path_idx);
void audio_path_exit(u8 path_idx);
void audio_path_start(u8 path_idx);

#endif //__BSP_AUDIO_H__

