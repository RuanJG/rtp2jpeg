/*************************************************************************
	> File Name: video_capture.h
	> Author: 
	> Mail: 
	> Created Time: 2015年01月23日 星期五 14时49分02秒
 ************************************************************************/

#ifndef _VIDEO_CAPTURE_H
#define _VIDEO_CAPTURE_H

#include <linux/videodev2.h>
struct buffer{
    void *start;
    size_t length;
};
struct camera{
    char *device_name;
    int fd;
    int width;
    int height;
    int display_depth;
    int image_size;
    int frame_number;
    unsigned char support_fmt;
    /*  7  6  5  4  3      2      1       0 
     *                     yuv420 yuyv422 jpeg
     * */
    int fmt_select;/*如果为FMT_AUTO 则程序自动选则输出格式，并且选则的优先级为先jpeg,yuyv422,yuv420,如果设置为FMT_YUYV422,表示输出YUYV422,FMT_YUV420表示输出yuv420*/
    unsigned int n_buffers;
    struct v4l2_capability v4l2_cap;
    struct v4l2_cropcap v4l2_cropcap;
    struct v4l2_format v4l2_fmt;
    struct v4l2_crop v4l2_crop;
    struct v4l2_fmtdesc v4l2_fmtdesc;
    struct buffer *buffers;
};
/*上面参数fmt_select选择*/
#define FMT_AUTO    101
#define FMT_YUYV422 102
#define FMT_YUV420  103

/*辅助函数*/
//void errno_exit(const char *s);
//int xioctl(int fd,int request,void *arg);
/*对摄像头操作函数*/
//void open_camera(struct camera *cam);
//void close_camera(struct camera *cam);

//void init_camera(struct camera *cam);
//void exit_camera(struct camera *cam);

//void start_capturing(struct camera *cam);
//void stop_capturing(struct camera *cam);

/*视频数据读取处理函数
 * 此函数用于读取yuyv 或者yuv422数据 不能读取JPEG数据
 * buffer必须是已经分配好的内存 内存大小为cam->width*cam->height*3
 * 成功返回 0 继续读取返回1 错误返回-1
 *
 * */
int read_frame(struct camera *cam,unsigned char *buffer,int *len);
/*初始化退出总函数*/
void v4l2_init(struct camera *cam);
void v4l2_exit(struct camera *cam);

#endif
