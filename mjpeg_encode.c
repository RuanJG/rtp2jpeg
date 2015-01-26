/*************************************************************************
	> File Name: mjpeg_encode.c
	> Author: 
	> Mail: 
	> Created Time: 2015年01月23日 星期五 17时00分16秒
 ************************************************************************/
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <malloc.h>
#include <signal.h>
#include "jpeglib.h"
#include "mjpeg_rtp.h"
#include "sock.h"
#include "config.h"
/*将YUYV422Z转化为RGB数据*/
static int yuyv422torgb(unsigned char *pyuv, 
                        unsigned char *rgb, 
                        unsigned int width, 
                        unsigned int height);
/*将RGB数据压缩未jpeg并且放到内存中*/
static int encode_rgb_to_jpeg_mem(unsigned char *inbuf,
                                  unsigned char **outbuf,
                                  unsigned long *outsize,
                                  int width,
                                  int height);
/*发送数据到VLC*/
static void send_jpeg_rtp(unsigned char *p,
                          int outsize,
                          int width,
                          int height);


/*********************关键函数***********************************/

#ifdef SAVEFIRSTJPEG
int first = 0;
FILE *pf;
char mjpeg_name[100]="mjpegimage.jpg";
#endif


void jpeg_encode_yuyv422_rtp(unsigned char *jpeg_data,int width,int hight)
{
    int i = 0;
    if(jpeg_data[0] == '\0')
        return;
    unsigned long outsize = width*hight*3;
    unsigned char *outbuf = (unsigned char *)malloc(sizeof(char)*outsize);
    unsigned char *frame_buffer = (unsigned char *)malloc(sizeof(char)*outsize);

    yuyv422torgb(jpeg_data,frame_buffer,width,hight); 
    encode_rgb_to_jpeg_mem(frame_buffer,&outbuf,&outsize,width,hight);
#ifdef SAVEFIRSTJPEG
    if(first == 0){
        pf = fopen(mjpeg_name,"wa+");
        fwrite(outbuf,outsize,1,pf);
        first++;
    }
#endif
    dbug("encode rgb to jpeg mem");
    unsigned char *ptcur = outbuf;
    
    
    for(i = 0;i < outsize;++i){
        if((outbuf[i] == 0xFF) && (outbuf[i+1]) == 0xC4)
            break;
    }

    if(i == outsize){
        printf("huffman tables don't exist!\n");
        goto out;
    }
    send_jpeg_rtp(ptcur,outsize,width,hight);
out:
    free(outbuf);
    free(frame_buffer);
}



/*************************yuyv422torgb**************************/
int sign3;

/*辅助函数*/
int yuvtorgb(int y, int u, int v)
{
     unsigned int pixel24 = 0;
     unsigned char *pixel = (unsigned char *)&pixel24;
     int r, g, b;
     static long int ruv, guv, buv;

     if(sign3)
     {
         sign3 = 0;
         ruv = 1159*(v-128);
         guv = 380*(u-128) + 813*(v-128);
         buv = 2018*(u-128);
     }

     r = (1164*(y-16) + ruv) / 1000;
     g = (1164*(y-16) - guv) / 1000;
     b = (1164*(y-16) + buv) / 1000;

     if(r > 255) r = 255;
     if(g > 255) g = 255;
     if(b > 255) b = 255;
     if(r < 0) r = 0;
     if(g < 0) g = 0;
     if(b < 0) b = 0;

     pixel[0] = r;
     pixel[1] = g;
     pixel[2] = b;

     return pixel24;
}
static int yuyv422torgb(unsigned char *pyuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
     unsigned int in, out;
     int y0, u, y1, v;
     unsigned int pixel24;
     unsigned char *pixel = (unsigned char *)&pixel24;
     unsigned int size = width*height*2;
    unsigned char *yuv = pyuv;
     for(in = 0, out = 0; in < size; in += 4, out += 6)
     {
          y0 = yuv[in+0];
          u  = yuv[in+1];
          y1 = yuv[in+2];
          v  = yuv[in+3];

          sign3 = 1;
          pixel24 = yuvtorgb(y0, u, v);
          rgb[out+0] = pixel[0];    //for QT
          rgb[out+1] = pixel[1];
          rgb[out+2] = pixel[2];

          pixel24 = yuvtorgb(y1, u, v);
          rgb[out+3] = pixel[0];
          rgb[out+4] = pixel[1];
          rgb[out+5] = pixel[2];
     }
     return 0;
}

/********************************************************/

static int encode_rgb_to_jpeg_mem(unsigned char *inbuf,unsigned char **outbuf,unsigned long *outsize,int width,int height)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;
    unsigned char*buf = NULL;
    int x;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo,outbuf,outsize);


    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo,80,1);

    jpeg_start_compress(&cinfo,1);
    row_stride = width * 3;
    buf=malloc(row_stride);
    row_pointer[0] = buf;
    while (cinfo.next_scanline < height)
    {
        for (x = 0; x < row_stride; x+=3){
            buf[x]   = inbuf[x];
            buf[x+1] = inbuf[x+1];
            buf[x+2] = inbuf[x+2];
        }
        jpeg_write_scanlines(&cinfo,row_pointer,1);
        inbuf += row_stride;
    }
    jpeg_finish_compress(&cinfo);    
    jpeg_destroy_compress(&cinfo);    
    free(buf);
    return 1;
}

unsigned short seq_num = 0;
unsigned int ts_current = 0;
unsigned start_seq = 0;

unsigned char extractQTable1[64];
unsigned char extractQTable2[64];
/*提取量化表*/
static void extractQTable(unsigned char *in,int len)
{
    int i=0,j=0,k=0;
    unsigned char *p = in;
    for(i = 0;i<len;++i){
        if( (p[i] == 0xFF) && p[i+1] == 0xDB ){
            if(k == 0){
                for(j = 0;j<64;++j)
                    extractQTable1[j] = p[i+5+j];
                k = 1;
            }
            if(k == 1){
                for(j = 0;j<64;++j)
                    extractQTable2[j] = p[i+5+j];
                break;
            }
        }
    }
}
#define PACKET_SIZE 1400


static unsigned int convertToRTPTimestamp(/*struct timeval tv*/)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned int  timestampIncrement = (90000*tv.tv_sec);
    timestampIncrement += (unsigned int )((2.0*90000*tv.tv_usec + 1000000.0)/2000000);   

    unsigned int const rtpTimestamp = timestampIncrement;  

    return rtpTimestamp;

}

unsigned short SendFrame(unsigned short start_seq, 
                         unsigned int  ts,
                         unsigned int ssrc,
                         unsigned char *jpeg_data, 
                         int len, 
                         unsigned short type,
                         unsigned short typespec, 
                         int width, 
                         int height, 
                         unsigned short dri,
                         unsigned short q,
                         unsigned char *lqt,
                         unsigned char*cqt)
{
	rtp_hdr_t rtphdr;
	struct jpeghdr jpghdr;
	struct jpeghdr_rst rsthdr;
	struct jpeghdr_qtable qtblhdr;
	unsigned char packet_buf[PACKET_SIZE];
	unsigned char *ptr = NULL;
    unsigned int off = 0;
	int bytes_left = len;
	int seq = start_seq;
	int pkt_len, data_len;
	/* Initialize RTP header
	 */
	rtphdr.version = 2;
	rtphdr.p = 0;
	rtphdr.x = 0;
	rtphdr.cc = 0;
	rtphdr.m = 0;
	rtphdr.pt = RTP_PT_JPEG;
    rtphdr.seq = htons(seq_num);
	rtphdr.ts = htonl(convertToRTPTimestamp());
	rtphdr.ssrc = htonl(ssrc);

	/* Initialize JPEG header
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | Type-specific |              Fragment Offset                  |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      Type     |       Q       |     Width     |     Height    |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 */
	jpghdr.tspec = typespec;/*解码没有用*/
	jpghdr.off = off;/*解码要用*/
	jpghdr.type = type | ((dri != 0) ? RTP_JPEG_RESTART : 0);
    
	jpghdr.q = q;
	jpghdr.width = width/8;
	jpghdr.height = height/8;
	/* Initialize DRI header
    //     0                   1                   2                   3
    //0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|       Restart Interval        |F|L|       Restart Count       |
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 */
	if (dri != 0) {
		rsthdr.dri = dri;
		rsthdr.f = 1;        /* This code does not align RIs */
		rsthdr.l = 1;
		rsthdr.count = 0x3fff;
	}

	/* Initialize quantization table header
	 */
	if (q >= 80) {
		qtblhdr.mbz = 0;/*这个必须为0*/
		qtblhdr.precision = 0; /* This code uses 8 bit tables only */
		qtblhdr.length = 128;  /* 2 64-byte tables */
	}


	while (bytes_left > 0) {
		ptr = packet_buf + RTP_HDR_SZ;
		
        memcpy(ptr, &jpghdr, sizeof(jpghdr));
      	ptr += sizeof(jpghdr);
        rtphdr.m = 0;
        
        if (dri != 0) {
            memcpy(ptr, &rsthdr, sizeof(rsthdr));
            ptr += sizeof(rsthdr);
        }
        if (q >= 80 && jpghdr.off == 0) {/*发送的第一帧数据带上量化表*/
            memcpy(ptr, &qtblhdr, sizeof(qtblhdr));
            ptr += sizeof(qtblhdr);
            memcpy(ptr, lqt, 64);
            ptr += 64;
            memcpy(ptr, cqt, 64);
            ptr += 64;
        }

        data_len = PACKET_SIZE - (ptr - packet_buf);
        if (data_len >= bytes_left) {
            data_len = bytes_left;
            rtphdr.m = 1;
        }
        memcpy(packet_buf, &rtphdr, RTP_HDR_SZ);
        memcpy(ptr, jpeg_data + off, data_len);

        //send(sockfd,packet_buf, (ptr - packet_buf) + data_len,0);
        send_sock(packet_buf,(ptr-packet_buf)+data_len);
        off+=data_len;
        jpghdr.off = htonl(off);//这里有问题
        //jpghdr.off = convert24bit(off);/*协议说要转化为网络字节顺序，但是这个off是24bit ，不知道如何转化为网络字节顺序，如果是用htonl直接报断错误，rfc2435*/
        //jpghdr.off = off;
        bytes_left -= data_len;
        rtphdr.seq = htons(++seq_num);//这里有问题。
        
    }
    return rtphdr.seq;
}

static void send_jpeg_rtp(unsigned char *in,int outsize,int width,int height)
{
    unsigned char typemjpeg = 1;
    unsigned char typespecmjpeg = 1;
    unsigned char drimjpeg = 0;
    unsigned char qmjpeg = 80;
    extractQTable(in,outsize);
    start_seq = SendFrame(start_seq,ts_current,10, in,outsize,typemjpeg,typespecmjpeg,width,height,drimjpeg,qmjpeg,extractQTable1,extractQTable2);

}
