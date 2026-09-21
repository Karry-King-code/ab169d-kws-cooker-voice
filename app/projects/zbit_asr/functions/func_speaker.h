#ifndef _FUNC_SPEAKER_H
#define _FUNC_SPEAKER_H

#define     UART_AUDIO_ESBC      10
#define     AUDIO_TX_FRAME      (8*UART_AUDIO_ESBC)
#define     AUDIO_RX_FRAME      (AUDIO_TX_FRAME * 16)
#define     UART_AUDIO_FRAME    (UART_AUDIO_ESBC * 16)

typedef struct {
    u8  pause       :   1,
        rec_en      :   1,
        draw_update :   1;
}func_speaker_t;
extern func_speaker_t f_spk;

#define  L_TOTAL      240     /* Total size of speech buffer.               */
#define  L_WINDOW     240     /* Window size in LP analysis.                */
#define  L_NEXT       40      /* Lookahead in LP analysis.                  */
#define  L_FRAME      80      /* Frame size.                                */
#define  L_SUBFR      40      /* Subframe size.                             */
#define  M            10      /* Order of LP filter.                        */
#define  MP1          (M+1)   /* Order of LP filter + 1                     */
#define  PIT_MIN      20      /* Minimum pitch lag.                         */
#define  PIT_MAX      143     /* Maximum pitch lag.                         */
#define  L_INTERPOL   (10+1)  /* Length of filter for interpolation.        */
#define  GAMMA1       24576   /* Bandwitdh factor = 0.75   in Q15           */

#define   MA_NP         4      /* MA prediction order for LSP */
#define L_H 22     /* size of truncated impulse response of A(z/g1)/A(z/g2) */

typedef struct {
    s16 y2_hi;
    s16 y2_lo;
    s16 y1_hi;
    s16 y1_lo;
    s16 x0;
    s16 x1;
} post_hpf_cb_t;

typedef struct
{
    s16 bad_lsf;                              /* bad LSF indicator            */
    s16 synth_buf[L_FRAME+M];                 /* Synthesis                    */
    s16 *synth;
    u8  serial[L_FRAME+3];                    /* Serial stream                */
    s16 Az_dec[MP1*2];                        /* Decoded Az for post-filter   */

    s16 *exc;
    s16 sharp;                                /* pitch sharpening of previous frame */
    s16 old_T0;                               /* integer delay of previous frame    */
    s16 gain_code;                            /* Code gain                          */
    s16 gain_pitch;                           /* Pitch gain                         */
    s16 lsp_old[M];
    s16 lsp_new[M];                           /* LSPs             */
    s16 mem_syn[M];                           /* Filter's memory */
    s16 old_exc[L_FRAME+PIT_MAX+L_INTERPOL];  /* Excitation vector */
    s16 acelp_code[L_SUBFR];                  /* ACELP codevector */

    s16 past_qua_en_dec[4];

    s16 freq_prev[MA_NP][M];                  /* Q13 */
    s16 prev_lsp[M];                          /* previous LSP vector         */
    s16 prev_ma;                              /* previous MA prediction coef.*/

    /* inverse filtered synthesis (with A(z/GAMMA2_PST))   */
    s16 res2_buf[PIT_MAX+L_SUBFR];
    s16 scal_res2_buf[PIT_MAX+L_SUBFR];
    s16 *res2;
    s16 *scal_res2;

    /* memory of filter 1/A(z/GAMMA1_PST) */
    s16 mem_syn_pst[M];

    s16 res2_pst[L_SUBFR];                    /* res2[] after pitch postfiltering */
    s16 syn_pst[L_FRAME];                     /* post filtered synthesis speech   */
    s16 Ap3[MP1];
    s16 Ap4[MP1];                             /* bandwidth expanded LP parameters */
    s16 pf_h[L_H];
    s16 mem_pre;
    s16 past_gain;                            /* past_gain = 1.0 (Q12) */

    post_hpf_cb_t post_hpf_cb;
}esbc_dec_cb_t;

void dac_gpdma_reset(void);
void spk_ains3_init(ains3_cb_t *ains3_cb);
void spk_ains3_plus_process(s16 *buf);

void func_speaker_message(u16 msg);
void func_speaker_stop(void);
void func_speaker_start(void);
void func_speaker_pause_play(void);
void func_speaker_mp3_res_play(u32 addr, u32 len);
void dump_putbuf(u8 *buf,  int buf_len, u8 file_idx);
int audio_esbc_encode_decode_init(esbc_dec_cb_t *esbc_d_vars);
int esbc_decode_8kbps(u8 *inbuf, u8 *outbuf, unsigned int size, esbc_dec_cb_t *esbc_d_vars);
int esbc_encode_8kbps(u8 *inbuf, u8 *outbuf, unsigned int size);
#if (GUI_SELECT != GUI_NO)
void func_speaker_display(void);
void func_speaker_enter_display(void);
void func_speaker_exit_display(void);
#else
#define func_speaker_display()
#define func_speaker_enter_display()
#define func_speaker_exit_display()
#endif

#endif // _FUNC_SPEAKER_H
