/*************************************************************************
	> File Name: config.h
	> Author: 
	> Mail: 
	> Created Time: 2015年01月23日 星期五 15时19分47秒
 ************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

#define OUTPUT_CAMINFO


#define CAM_WIDGH 640
#define CAM_HEIGHT  480
// height widgh
#define CAM_DEVICE "/dev/video0"

#if 0
    #define dbug(x) printf("%s %s %d %s\n",__FILE__,__func__,__LINE__,x)
#else
    #define dbug(x) 
#endif
#endif
