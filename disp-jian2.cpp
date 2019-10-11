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
		return 0;
}

int disp_draw_h_colorbar_1(unsigned int base, unsigned int width, unsigned int height)
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

int fp =0;
unsigned int fbp;
struct fb_var_screeninfo  vinfo;
struct fb_fix_screeninfo  finfo;

int main(int argc, char **argv)
{

		long screensize=0;
		struct timeval start, stop, diff;
		fp = open ("/dev/graphics/fb1", O_RDWR);
		if(!fp)
		{
				printf("Error: cannot open framebuffer device.\n");
				exit(1);
		}

		if (ioctl(fp, FBIOGET_FSCREENINFO, &finfo))
		{
				printf("Error: reading fixed information.\n");
				exit(2);
		}
		printf("finfo.mmio_start  %ld\n", finfo.mmio_start);
		printf("finfo.smem_start  %ld\n", finfo.smem_start);/* physical address */

		if (ioctl(fp, FBIOGET_VSCREENINFO, &vinfo))
		{
				printf("Error: reading variable information.\n");
				exit(3);
		}
		printf("vinfo.xres %d, vinfo.yres %d, vinfo.xres_virtual %d, vinfo.yres_virtual %d\n",
						vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual);

		printf("vinfo.xoffset %d, vinfo.yoffset %d, vinfo.bits_per_pixel %d\n",
						vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel);

		screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8; /* bbc mask */

		fbp =(int) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
		memset((void*)fbp, 0, screensize);

        FILE *file_in = fopen("rgb", "rb+");
		fread((void*)fbp, 1, screensize, file_in);

		int x =0, y =0, i =0, p =1;
		/*for(i=0; i<3; i++)
		{
				for(y=i*(vinfo.yres/3); y<(i+1)*(vinfo.yres/3); y++)
				{
						for(x=0; x<vinfo.xres; x++)
						{
								long location = x*2 + y*vinfo.xres*2;
								int r=0, g=0,b=0;
								unsigned short rgb;
								if (i==0)
										r=((x*1.0)/vinfo.xres)*32;
								if (i==1)
										g=((x*1.0)/vinfo.xres)*64;
								if (i==2)
										b=((x*1.0)/vinfo.xres)*32;
								rgb = (r<<11) | (g<<5) | b;
								*((unsigned short *)(fbp +location)) = rgb;
						}
				}
		}*/

		do{
				gettimeofday(&start, 0);
				vinfo.yoffset =0;
				if(ioctl(fp, FBIOPUT_VSCREENINFO, &vinfo))
				{
						printf("Error: FBIOPUT_VSCREENINF information.\n");
				}
				if(ioctl(fp, FBIOPAN_DISPLAY, &vinfo))
				{
						printf("Error: FBIOPAN_DISPLAY information.\n");
				}
				gettimeofday(&stop, 0);
				timeval_subtract(&diff, &start, &stop);
				if(i++ ==10){
					printf("time1 is:%ld us\n", diff.tv_usec);
				}
		}while(1);

		munmap ((void*)fbp, screensize);
		close (fp);

		return 0;
}

