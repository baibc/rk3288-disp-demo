//demo1. 在屏幕上显示一个图层
#include "sunxi_display2.h"
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "v.h"
#include "rgbdata.h"
#include "rk_fb.h"
#include <sys/time.h> 


#define writel(val, addr) (*(((unsigned int *)(addr))) = (val))
typedef unsigned int __u32;

/*cat /sys/class/graphics/fb0/disp_info   查看layer信息*/
/*内核修改参考rk_fb_box.c中,有by blues的地方*/

/**
 * 计算两个时间的间隔，得到时间差
 * @param struct timeval* resule 返回计算出来的时间
 * @param struct timeval* x 需要计算的前一个时间
 * @param struct timeval* y 需要计算的后一个时间
 * return -1 failure ,0 success
 **/
int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y)
{
		int nsec;

		if ( x->tv_sec>y->tv_sec )
				return -1;

		if ( (x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec) )
				return -1;

		result->tv_sec = ( y->tv_sec-x->tv_sec );
		result->tv_usec = ( y->tv_usec-x->tv_usec );

		if (result->tv_usec<0)
		{
				result->tv_sec--;
				result->tv_usec+=1000000;
		}

		return 0;
} 

//only for DISP_FORMAT_ARGB_8888 format
int disp_draw_h_colorbar(unsigned int base, unsigned int width, unsigned int height)
{
		unsigned int i=0, j=0;
		for(i = 0; i<height; i++) {
				for(j = 0; j<width/4; j++) {
						unsigned int offset = 0;
						offset = width * i + j;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<1), base + offset*4);
						offset = width * i + j + width/4;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<1), base + offset*4);
						offset = width * i + j + width/4*2;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<1), base + offset*4);
						offset = width * i + j + width/4*3;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<16) | (((1<<8)-1)<<8), base + offset*4);
				}
		}
		//for(i = 0; i<height*width; i++) {
		// *(unsigned int*)(base+i)=0x000000ff;
		//}
		return 0;
}

//only for DISP_FORMAT_ARGB_8888 format
int disp_draw_h_colorbar1(unsigned int base, unsigned int width, unsigned int height)
{
		unsigned int i=0, j=0;
		for(i = 0; i<height; i++) {
				for(j = 0; j<width/4; j++) {
						unsigned int offset = 0;
						offset = width * i + j;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<16) | (((1<<8)-1)<<8), base + offset*4);
						offset = width * i + j + width/4;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<16) | (((1<<8)-1)<<8), base +offset*4);
						offset = width * i + j + width/4*2;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<1), base + offset*4);
						offset = width * i + j + width/4*3;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<1), base + offset*4);
				}
		}

		return 0;
}

#define RK_FBIOSET_VSYNC_ENABLE		0x4629
#define RK_FBIOSET_OVERLAY_STATE    0x5018
#define RK_FBIOSET_YUV_ADDR		0x5002
#define RK_FBIOSET_ENABLE		0x5019


int fp=0;
unsigned int fbp ;
struct fb_var_screeninfo  vinfo;
struct fb_fix_screeninfo  finfo;

int main(int argc, char **argv)
{
#if 1
		unsigned int arg[3];
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int ret = 0;
		unsigned int screen_id = 0;
		unsigned int layer_id = 1;
		unsigned int mem_id = 0;
		unsigned int buffer_num = 2;
		unsigned int dispfh;
		unsigned int fb_width, fb_height;
		unsigned int data_format;//数据格式
		unsigned int xpos, ypos;  //显示坐标
		disp_layer_config  config;


		unsigned int xsize, ysize;//显示区域的大小
		unsigned int vsync_enable;
		struct timeval start, stop, diff;
		struct stat s;

		int fp=0;
		int fp1=0;
		struct fb_var_screeninfo  vinfo;
		struct fb_fix_screeninfo  finfo;
		long screensize=0;
		unsigned int fbp ;
		int x=0, y=0;

		unsigned long virt;
		long unsigned int phys_addr;

		u32 yuv_phy[2];
#endif

		fp = open ("/dev/graphics/fb0",O_RDWR);

		if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo))
		{
				printf("Error reading fixed information/n");
				exit(2);
		}
		printf("finfo.mmio_start  %ld\n",finfo.mmio_start);
		printf("finfo.smem_start  %ld\n",finfo.smem_start);/* physical address */

		if (ioctl(fp,FBIOGET_VSCREENINFO,&vinfo))
		{
				printf("Error reading variable information/n");
				exit(3);
		}
		printf("vinfo.xres %d, vinfo.yres %d, vinfo.xres_virtual %d, vinfo.yres_virtual %d, vinfo.xoffset %d\n",
						vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual, vinfo.xoffset);

		printf("vinfo.yoffset %d, vinfo.bits_per_pixel %d, vinfo.xactivate %d, vinfo.height %d\n",
						vinfo.yoffset, vinfo.bits_per_pixel, vinfo.activate, vinfo.height);

		//计算显存大小
		//vinfo.xres = 1920;
		//vinfo.yres = 1080;

		screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8; /* bbc mask */
		printf("screensize %ld\n",screensize);

#if 1
		data_format = 5;
		xsize=1920;
		ysize=1080;
		xpos =0;
		ypos =0;
		vinfo.grayscale = (xsize<<8) | (ysize<<20);
		vinfo.nonstd = (xpos<<8)|(ypos<<20)|(data_format);//设置显示坐标和数据格式　这里请参数文档RockChip_DSS Development Guide v1.2.pdf
		vinfo.yoffset = 0;
		vinfo.xres_virtual = 1920;
		vinfo.yres_virtual = 2160;
		int overlay = 3201;//开  //关3012
		//overlay 数值表示层的显示顺序,个位为最底层,千位为最高层,
		//个位数值为１表示win1在最底层,千位为３表示win3 在最高层.
		//android ui对应win0, 视频层对应win0, 我们自己用的为win2层, win3暂时没有用到.

		fp1 = open ("/dev/graphics/fb1",O_RDWR);

		if(ioctl(fp1, RK_FBIOSET_OVERLAY_STATE, &overlay));
		{
				printf("Error RK_FBIOSET_OVERLAY_STATE  information\n");
		}
		if(ioctl(fp1, RK_FBIOSET_ENABLE, 1));
		{
				printf("RK_FBIOSET_ENABLE  information\n");
		}
#endif

		/*这就是把fp所指的文件中从开始到screensize大小的内容给映射出来，得到一个指向这块空间的指针*/
		fbp =(int) mmap (0, screensize*3, PROT_READ | PROT_WRITE, MAP_SHARED, fp1, 0);
		printf("mmap 0x%x\n",fbp);

        FILE *file_in  = fopen("rgb", "rb+");
		fread((void*)fbp, 1, screensize*3, file_in);
		//disp_draw_h_colorbar(fbp, 1024, 768);
		//disp_draw_h_colorbar1(fbp+1024*768*4, 1024, 768);

		//while(1);
		//双缓存切换演示.
		while(1)
		{
				//gettimeofday(&start,0);
				vinfo.yoffset = 0;
				if(ioctl(fp1,FBIOPUT_VSCREENINFO,&vinfo))
				{
						//printf("Error FBIOPUT_VSCREENINF  information/n");
				}
				if(ioctl(fp1, FBIOPAN_DISPLAY, 1));
				{
						//printf("FBIOPAN_DISPLAY  information\n");
				}
				//gettimeofday(&stop,0);
				//timeval_subtract(&diff,&start,&stop);
				//printf("1 is:%d us\n", diff.tv_usec);
				//gettimeofday(&start,0);

				vinfo.yoffset =1080;
				if(ioctl(fp1,FBIOPUT_VSCREENINFO,&vinfo))
				{
						//printf("Error FBIOPUT_VSCREENINF  information/n");
				}
				if(ioctl(fp1, FBIOPAN_DISPLAY, 1));
				{
						//printf("FBIOPAN_DISPLAY  information\n");
				}
				//gettimeofday(&stop,0);
				//timeval_subtract(&diff,&start,&stop);
				//printf(" 2 is:%d us\n",diff.tv_usec);
		}

		munmap ((void*)fbp, screensize); /*解除映射*/
		close (fp); /*关闭文件*/ 
		close (fp1); 

		return 0;
}

