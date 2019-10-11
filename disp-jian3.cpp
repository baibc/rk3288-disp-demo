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
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <linux/fb.h>

#include "v.h"
#include "rk_fb.h"
#include "sunxi_display2.h"

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

int main(int argc, char **argv)
{

		int fb_fd =0;
		unsigned int fbp;
		long screensize =0;
		unsigned int xpos, ypos;
		unsigned int xsize, ysize;
		struct timeval start, stop, diff;

		fb_fd = open ("/dev/graphics/fb1", O_RDWR);
		if(!fb_fd)
		{
				printf("Error: cannot open framebuffer device.\n");
				exit(1);
		}

		struct fb_fix_screeninfo  finfo;
		if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo)) /* get fix screeninfo */
		{
				printf("Error: reading fixed information.\n");
				exit(2);
		}
		printf("finfo.mmio_start  %ld\n", finfo.mmio_start);
		printf("finfo.smem_start  %ld\n", finfo.smem_start);/* physical address */

		struct fb_var_screeninfo  vinfo;
		if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) /* get var screeninfo */
		{
				printf("Error: reading variable information.\n");
				exit(3);
		}
		printf("vinfo.xres %d, vinfo.yres %d, vinfo.xres_virtual %d, vinfo.yres_virtual %d\n",
						vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual);

		printf("vinfo.xoffset %d, vinfo.yoffset %d, vinfo.bits_per_pixel %d\n",
						vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel);

		

		screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8; /* mask */
		//screensize = 1280 * 720 * 32 / 8;

		fbp =(int) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
		memset((void*)fbp, 0, screensize);

        FILE *file_in = fopen("mySrcData", "rb+");
		fread( (void*)fbp, 1, screensize, file_in);

		int x =0, y =0, i =0;
		/*char *srcdata = (char*)fbp;
		for(i=0; i<screensize; i++){
				srcdata[i] =1;
		}*/
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

		xsize =vinfo.xres;
		ysize =vinfo.yres;
		xpos =0; ypos =0;
		vinfo.grayscale = ((xsize<<8) | (ysize<<20)) & 0xff;
		vinfo.nonstd = (xpos<<8) | (ypos<<20) | (2); //(HAL_PIXEL_FORMAT_YV12)

		do{
				gettimeofday(&start, 0);
				if(ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo))
				{
						printf("Error: FBIOPUT_VSCREENINF information.\n");
				}

				if(ioctl(fb_fd, FBIOPAN_DISPLAY, &vinfo))
				{
						printf("Error: FBIOPAN_DISPLAY information.\n");
				}
				gettimeofday(&stop, 0);
				timeval_subtract(&diff, &start, &stop);
				if(i++ ==10){
					printf("render time is:%ld s %ld us\n", diff.tv_sec, diff.tv_usec);
				}
		}while(1);

		munmap ((void*)fbp, screensize);
		close (fb_fd);

		return 0;
}

