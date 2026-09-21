#ifndef _API_BUFFER_H
#define _API_BUFFER_H

typedef struct{
    u8 *start;
    u8 *end;
    u8 *rptr;
    u8 *wptr;
    volatile u32 data_len;
    volatile u32 total_len;
}cbuf_t;

void circular_buf_init(cbuf_t *cbuf, void *buf, u32 len);           //循环buffer初始化
u32  circular_buf_len_get(cbuf_t *cbuf);                            //获取循环buffer数据长度
void circular_buf_clear(cbuf_t *cbuf);                              //清空循环buffer所有数据和状态
u32  circular_buf_write(cbuf_t *cbuf, void *buf, u32 write_len);    //循环buffer写, return:返回成功写入的字节数
u32  circular_buf_is_write_able(cbuf_t *cbuf, u32 write_len);       //循环buffer可写长度获取, return:剩余可写字节数
u32  circular_buf_read(cbuf_t *cbuf, void *buf, u32 read_len);      //循环buffer读, return:返回读出的字节数
#endif
