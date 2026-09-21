#ifndef _API_NR_H
#define _API_NR_H

#define _MAX_GAIN                            32767
#define SOFT_DIG_P0DB                       (_MAX_GAIN * 1.000000)
#define SOFT_DIG_P1DB                     	(_MAX_GAIN * 1.122018)
#define SOFT_DIG_P2DB                     	(_MAX_GAIN * 1.258925)
#define SOFT_DIG_P3DB                     	(_MAX_GAIN * 1.412538)
#define SOFT_DIG_P4DB                     	(_MAX_GAIN * 1.584893)
#define SOFT_DIG_P5DB                     	(_MAX_GAIN * 1.778279)
#define SOFT_DIG_P6DB                     	(_MAX_GAIN * 1.995262)
#define SOFT_DIG_P7DB                     	(_MAX_GAIN * 2.238721)
#define SOFT_DIG_P8DB                     	(_MAX_GAIN * 2.511886)
#define SOFT_DIG_P9DB                     	(_MAX_GAIN * 2.818383)
#define SOFT_DIG_P10DB                    	(_MAX_GAIN * 3.162278)
#define SOFT_DIG_P11DB                    	(_MAX_GAIN * 3.548134)
#define SOFT_DIG_P12DB                    	(_MAX_GAIN * 3.981072)
#define SOFT_DIG_P13DB                    	(_MAX_GAIN * 4.466836)
#define SOFT_DIG_P14DB                    	(_MAX_GAIN * 5.011872)
#define SOFT_DIG_P15DB                    	(_MAX_GAIN * 5.623413)

//typedef struct {
//    u32 nt;
//    u8  prior_opt_idx;
//} ains3_cb_t;

typedef struct {
	u16 nt;
	s16 exp_range_H;
	s16 exp_range_L;
	u8  model_select;
	u16 min_value;
	u16 nostation_floor;
	u8  wind_thr;
	u8  wind_en;
	u8  noise_ps_rate;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
    u8  param_printf;                           //使能参数打印
    u8  wind_level;
	u16 wind_range;
	u16 low_fre_range;
	u16 low_fre_range0;
	u8  pitch_filter_en;
	u16 mask_floor;
	u8  mask_floor_r;
	u8  music_lev;
	u16 gain_expand;
	u8  nn_only;
	u16 nn_only_len;
	u16 gain_assign;
	u8  sin_gain_post_en;
	u16 sin_gain_post_len;
	u16 sin_gain_post_len_f;
	u8  preem_en;
} dnn_cb_t;

void spk_dnn_init(dnn_cb_t *dnn_cb);
void spk_dnn_process(s16 *buf);
#endif
