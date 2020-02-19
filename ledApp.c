#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

#define LED_ON 1
#define LED_OFF 0
int main(int argc,char *argv[])
{
    int fd;
    char *filename;
    unsigned char databuf[1]; 
    int ret;
    if(argc != 3)
    {
        printf("Error Usage\r\n");
    }
    filename = argv[1];

    fd = open(filename,O_RDWR);
    if(fd < 0){
        printf("file %s open failed!\r\n",filename);
        return -1;
    }
    databuf[0] = atoi(argv[2]); //字符转换为数字

    ret = write(fd,databuf,sizeof(databuf));
    if(ret < 0){
        printf("LED control failed\r\n");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;

}