#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <linux/fb.h>

//#include "v.h"
//#include "rk_fb.h"
//#include "sunxi_display2.h"

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

		if ( x->tv_sec > y->tv_sec )
				return -1;

		if ( (x->tv_sec == y->tv_sec) && (x->tv_usec > y->tv_usec) )
				return -1;

		result->tv_sec  = ( y->tv_sec  - x->tv_sec );
		result->tv_usec = ( y->tv_usec - x->tv_usec );

		if (result->tv_usec<0)
		{
				result->tv_sec--;
				result->tv_usec+=1000000;
		}

		return 0;
} 

#define writel(val, addr) (*(((unsigned int *)(addr))) = (val))
int disp_draw_0value(unsigned int base, unsigned int width, unsigned int height)
{
		unsigned int i=0, j=0;
		for(i = 0; i<height; i++) {
				for(j = 0; j<width; j++) {
						unsigned int offset = 0;
						offset = width * i + j;
						writel(0, base+offset);
				}
		}
		return 0;
}
int disp_draw_h_colorbar(unsigned int base, unsigned int width, unsigned int height)
{
		unsigned int i=0, j=0;
		for(i = 0; i<height; i++) {
				for(j = 0; j<width/4; j++) {
						unsigned int offset = 0;
						offset = width * i + j;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<16), base + offset*4);

						offset = width * i + j + width/4;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<8), base + offset*4);

						offset = width * i + j + width/4*2;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<0), base + offset*4);

						offset = width * i + j + width/4*3;
						writel((((1<<8)-1)<<24) | (((1<<8)-1)<<16) | (((1<<8)-1)<<8), base + offset*4);
				}
		}
		return 0;
}



int main(int argc, char **argv)
{
		int fbp1;
		int fb_fd1 =0;
		long screensize1 =0;
		unsigned int xpos1, ypos1;
		unsigned int xsize1, ysize1;

		fb_fd1 = open ("/dev/graphics/fb1", O_RDWR);
		if(!fb_fd1)
		{
				printf("Error: cannot open framebuffer device.\n");
				exit(1);
		}

		struct fb_fix_screeninfo  finfo1;
		if (ioctl(fb_fd1, FBIOGET_FSCREENINFO, &finfo1)) /* get fix screeninfo */
		{
				printf("Error: reading fixed information.\n");
				exit(2);
		}
		printf("finfo1.mmio_start  %ld\n", finfo1.mmio_start);
		printf("finfo1.smem_start  %ld\n", finfo1.smem_start);/* physical address */

		struct fb_var_screeninfo  vinfo1;
		if (ioctl(fb_fd1, FBIOGET_VSCREENINFO, &vinfo1)) /* get var screeninfo */
		{
				printf("Error: reading variable information.\n");
				exit(3);
		}
		printf("vinfo1.xres %d, vinfo1.yres %d, vinfo1.xres_virtual %d, vinfo1.yres_virtual %d\n",
						vinfo1.xres, vinfo1.yres, vinfo1.xres_virtual, vinfo1.yres_virtual);
		printf("vinfo1.xoffset %d, vinfo1.yoffset %d, vinfo1.bits_per_pixel %d\n",
						vinfo1.xoffset, vinfo1.yoffset, vinfo1.bits_per_pixel);


		screensize1 = vinfo1.xres * vinfo1.yres * vinfo1.bits_per_pixel / 8; /* mask */
		fbp1 = (int)mmap(0, screensize1, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd1, 0);
		memset((void*)fbp1, 0, screensize1);
		printf("mmap address =0x%x\n\n", fbp1);

        FILE *file_in1  = fopen("mySrcData", "rb+");//rgba
		fread( (void*)fbp1, 1, screensize1, file_in1);
	
		vinfo1.xres =1920;
		vinfo1.yres =1080;
		vinfo1.xres_virtual =1920;
		vinfo1.yres_virtual =1080;
		xsize1 =vinfo1.xres;
		ysize1 =vinfo1.yres;
		vinfo1.grayscale = ((xsize1<<8) | (ysize1<<20)) & 0xff;
		xpos1 =0; 
		ypos1 =0;
		vinfo1.nonstd = (xpos1<<8) | (ypos1<<20) | (2); //(HAL_PIXEL_FORMAT_YV12)
		vinfo1.activate = FB_ACTIVATE_FORCE;

		/* ################################################################### */
		int fbp4;
		int fb_fd4 =0;
		long screensize4 =0;
		unsigned int xpos4, ypos4;

		fb_fd4 = open ("/dev/graphics/fb4", O_RDWR);
		if(!fb_fd4)
		{
				printf("Error: cannot open framebuffer device.\n");
				exit(1);
		}

		struct fb_fix_screeninfo  finfo4;
		if (ioctl(fb_fd4, FBIOGET_FSCREENINFO, &finfo4)) /* get fix screeninfo */
		{
				printf("Error: reading fixed information.\n");
				exit(2);
		}
		printf("finfo4.mmio_start  %ld\n", finfo4.mmio_start);
		printf("finfo4.smem_start  %ld\n", finfo4.smem_start);/* physical address */

		struct fb_var_screeninfo  vinfo4;
		if (ioctl(fb_fd4, FBIOGET_VSCREENINFO, &vinfo4)) /* get var screeninfo */
		{
				printf("Error: reading variable information.\n");
				exit(3);
		}
		printf("vinfo4.xres %d, vinfo4.yres %d, vinfo4.xres_virtual %d, vinfo4.yres_virtual %d\n",
						vinfo4.xres, vinfo4.yres, vinfo4.xres_virtual, vinfo4.yres_virtual);
		printf("vinfo4.xoffset %d, vinfo4.yoffset %d, vinfo4.bits_per_pixel %d\n",
						vinfo4.xoffset, vinfo4.yoffset, vinfo4.bits_per_pixel);

		screensize4 = vinfo4.xres * vinfo4.yres * vinfo4.bits_per_pixel / 8; /* mask */
		fbp4 = (int)mmap(0, screensize4, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd4, 0);
		memset((void*)fbp4, 0, screensize4);
		printf("mmap address =0x%x\n", fbp4);

		//struct timeval t1, t2, t3;
		//gettimeofday(&t1, 0);
		//memcpy((void*)fbp, meBuf, screensize);
		//gettimeofday(&t2, 0);
		//timeval_subtract(&t3, &t1, &t2);
		//printf("memcpy time is:%ld s %ld us\n", t3.tv_sec, t3.tv_usec);

		xpos4 =0; 
		ypos4 =0;
		vinfo4.nonstd = (xpos4<<8) | (ypos4<<20) | (5); //(HAL_PIXEL_FORMAT_YV12)
		/* ################################################################# */
		do{
			ioctl(fb_fd1, FBIOPUT_VSCREENINFO, &vinfo1);

			ioctl(fb_fd4, FBIOPUT_VSCREENINFO, &vinfo4);
		}while(0);

		close (fb_fd4);
		while(1)
		{
			ioctl(fb_fd1, FBIOPUT_VSCREENINFO, &vinfo1);

			disp_draw_0value(fbp4, 32, 32);
			//disp_draw_h_colorbar(fbp4, 32, 32);
		}
		/* ################################################################# */
			
		munmap ((void*)fbp1, screensize1);
		munmap ((void*)fbp4, screensize4);
		close (fb_fd1);

		return 0;
}

/* 记录测试信息如下：
 * fb0->格式１和格式５是相反的，格式２不显示
 * fb1->格式２相对是好的，跟fb0的格式１相似。
 * */
