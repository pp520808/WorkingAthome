#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define LED_MAJOR    200
#define LED_NAME     "led"
/* 地址映射 */
#define  CCM_CCGR1_BASE         (0x020C406C)
#define  SW_MUX_GPIO1_IO03_BASE (0x020E0068)
#define  SW_PAD_GPIO1_IO03_BASE (0x020E02F4)
#define  GPIO1_DR_BASE          (0x0209C000)
#define  GPIO1_GDIR_BASE        (0x0209C004)

/* 地址映射后的虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

#define LED_OFF  0
#define LED_ON   1
static void led_switch(u8 data)
{
    u32 val = 0;
    if(data == LED_OFF){
         /* 关闭led*/
        val = readl(GPIO1_DR);
        //val &= ~(1<<3); //清除之前的设置 bit 3
        val |= (1<<3);  //设置新值 bit 3为1
        writel(val,GPIO1_DR);
    }else if(data == LED_ON){
        val = readl(GPIO1_DR);
        val &= ~(1<<3); //清除之前的设置 bit 3
        //val |= (1<<3);  //设置新值 bit 3为1
        writel(val,GPIO1_DR);
    }
}
static int led_write(struct file *filp,const char __user *buf,size_t count,loff_t *ppos)
{
    int ret ;
    unsigned char databuf[1];
    ret = copy_from_user(databuf,buf,count);
    if(ret < 0){
        printk("kernel write failed\r\n");
        return -EFAULT;
    }
    /* 判断开灯还是关灯 */
    led_switch(databuf[0]);
    
    return 0;
}
static int led_open(struct inode *inode,struct file *filp)
{
    return 0;
}
static int led_release(struct inode *inode,struct file *filp)
{
    u32 val =0;
    val = readl(GPIO1_DR);
    //val &= ~(1<<3); //清除之前的设置 bit 3
    val |= (1<<3);  //设置新值 bit 3为
    return 0;
}
static const struct file_operations led_fops ={
    .owner = THIS_MODULE,
    .write = led_write,
    .open  = led_open,
    .release = led_release,
};

/* 入口函数 */
static int __init led_init(void)
{
    int ret = 0;
    u32 val = 0;
    /* 初始化LED灯 */
    /* 1.地址映射 */
    IMX6U_CCM_CCGR1   = ioremap(CCM_CCGR1_BASE,4);
    SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE,4);
    SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE,4);
    GPIO1_DR          = ioremap(GPIO1_DR_BASE,4);
    GPIO1_GDIR        = ioremap(GPIO1_GDIR_BASE,4);
    /*  2.初始化 */
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3<<26); //清除之前的设置 bit 26,27
    val |= (3<<26);  //设置新值 bit 26,27为1
    writel(val,IMX6U_CCM_CCGR1);
    /*  3.设置复用功能复用为GPIO1_IO03，设置IO属性*/
    writel(5,SW_MUX_GPIO1_IO03);
    writel(0x10B0,SW_PAD_GPIO1_IO03);
    /*  4.设置输出模式*/
    val = readl(GPIO1_GDIR);
    val &= ~(1<<3); //清除之前的设置 bit 3
    val |= (1<<3);  //设置新值 bit 3为1
    writel(val,GPIO1_GDIR);
    /* 5.默认开关*/
    val = readl(GPIO1_DR);
    val &= ~(1<<3); //清除之前的设置 bit 3
    //val |= (1<<3);  //设置新值 bit 3为1
    writel(val,GPIO1_DR);
    /* 注册字符设备 */
    
    ret = register_chrdev(LED_MAJOR,LED_NAME,&led_fops);
    if(ret < 0){
        printk("register chardev failed\r\n");
        return -EIO;
    }

    printk("led_init\r\n");
    return 0;
}
/* 出口函数 */
static void __exit led_exit(void)
{
    u32 val =0;
    /* 关闭led*/
    val = readl(GPIO1_DR);
    //val &= ~(1<<3); //清除之前的设置 bit 3
    val |= (1<<3);  //设置新值 bit 3为1
    writel(val,GPIO1_DR);
    /* 取消地址映射 */
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);
    /* 注销字符设备 */
    unregister_chrdev(LED_MAJOR,LED_NAME);

    printk("led_exit\r\n");
}
/* 注册驱动和卸载函数 */
module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman");