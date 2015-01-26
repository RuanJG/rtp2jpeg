/*************************************************************************
	> File Name: mjpeg_rtp.h
	> Author: 
	> Mail: 
	> Created Time: 2015年01月23日 星期五 16时47分33秒
 ************************************************************************/

#ifndef _MJPEG_RTP_H
#define _MJPEG_RTP_H

typedef struct {
        unsigned char cc:4;
        unsigned char x:1;
        unsigned char p:1;
        unsigned char version:2;
        unsigned char pt:7;
        unsigned char m:1;
        unsigned short seq;
        unsigned int ts;
        unsigned int ssrc;
}rtp_hdr_t;

struct jpeghdr {
    unsigned int tspec:8;   /* type-specific field */
    unsigned int off:24;    /* fragment byte offset */
    unsigned char type;            /* id of jpeg decoder params */
    unsigned char q; /* quantization factor (or table id) */
    unsigned char width;           /* frame width in 8 pixel blocks */
    unsigned char height;          /* frame height in 8 pixel blocks */

};
struct jpeghdr_rst{
    unsigned short dri;
    unsigned int f:1;
    unsigned int l:1;
    unsigned int count:14;
 };
struct jpeghdr_qtable {
    unsigned char  mbz;
    unsigned char  precision;
    unsigned short length;

};

#define RTP_JPEG_RESTART 0x40
#define RTP_HDR_SZ       12
#define RTP_PT_JPEG      26

void jpeg_encode_yuyv422_rtp(unsigned char *jpeg_data,int width,int hight);
void jpeg_encode_yuyv420_rtp(unsigned char *jpeg_data,int width,int hight);



#endif



