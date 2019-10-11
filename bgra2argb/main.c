/* ************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>

static int64_t getNowUs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_usec + (int64_t) tv.tv_sec * 1000000;
}
static int get_file_size(const char* file) {
        struct stat tbuf;
        stat(file, &tbuf);
        return tbuf.st_size;
}
void convertRGB (unsigned char*, unsigned char*, unsigned int, unsigned int);

int main(int argc, char** argv)
{
	//long signed int startTime = getNowUs(); 
	unsigned char* sp1 = NULL;
	unsigned char* sp2 = NULL;
	long int copySize = get_file_size(argv[1]);

	sp1 = (unsigned char *)malloc(copySize * sizeof(unsigned char));
	sp2 = (unsigned char *)malloc(copySize * sizeof(unsigned char));
	printf("===>>> The fileSize is = %ld\n", copySize);

	FILE *fileIn = fopen(argv[1], "rb+");
	size_t tmp   = fread(sp1, 1, copySize, fileIn);

	convertRGB(sp1, sp2, 1920, 1080);

	FILE *fileOut = fopen("Argb32.raw","wb+");
	size_t tmp1 = fwrite(sp2, 1, copySize, fileOut);

	free(sp1);
	free(sp2);
	//long signed int delay = getNowUs() - startTime;
    //printf("===>>> VPUDecodeTime: %.1f msec.\n", delay/1000.0f);

	return 0;
}

void convertRGB (unsigned char* s1, unsigned char* s2, unsigned int width, unsigned int height)
{
	unsigned int PixelsCount  = width * height *4;
	unsigned int i =0, j =0;

	if (s1 ==NULL || s2 ==NULL){
		return;
	}
	
	for(i=0; i<height; i++){
		for(j=0; j<width; j=j+4){
			s2[j]   = s1[j+3];
			s2[j+1] = s1[j];
			s2[j+2] = s1[j+1];
			s2[j+3] = s1[j+2];
		}
	}
	return;
}



