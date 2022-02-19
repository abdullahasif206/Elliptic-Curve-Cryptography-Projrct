#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>

#include "../include/cryptocore_ioctl_header.h"

#define GPIO1_BASE				0xFF709000
#define GPIO1_SPAN				0x00000078

#define H2F_BRIDGE_BASE			0xC0000000

#define MWMAC_RAM_BASE			0x00000000
#define MWMAC_RAM_SPAN			0x00000FFF

#define LW_H2F_BRIDGE_BASE		0xFF200000

#define LEDR_BASE          		0x00000000
#define LEDR_SPAN				0x0000000F

#define HEX3_HEX0_BASE			0x00000020
#define HEX3_HEX0_SPAN			0x0000000F

#define HEX5_HEX4_BASE			0x00000030
#define HEX5_HEX4_SPAN			0x0000000F

#define SW_BASE					0x00000040
#define SW_SPAN					0x0000000F

#define KEY_BASE				0x00000050
#define KEY_SPAN				0x0000000F

#define MWMAC_CMD_BASE			0x00000060
#define MWMAC_CMD_SPAN			0x00000007

#define MWMAC_IRQ_BASE         	0x00000070
#define MWMAC_IRQ_SPAN			0x0000000F

#define TRNG_CMD_BASE			0x00000080
#define TRNG_CMD_SPAN			0x0000000F

#define TRNG_CTR_BASE			0x00000090
#define TRNG_CTR_SPAN			0x0000000F

#define TRNG_TSTAB_BASE			0x000000A0
#define TRNG_TSTAB_SPAN			0x0000000F

#define TRNG_TSAMPLE_BASE		0x000000B0
#define TRNG_TSAMPLE_SPAN		0x0000000F

#define TRNG_IRQ_BASE			0x000000C0
#define TRNG_IRQ_SPAN			0x0000000F

#define TRNG_FIFO_BASE			0x00001000
#define TRNG_FIFO_SPAN			0x00000FFF

#define TIMER_BASE            	0x00002000
#define TIMER_SPAN				0x0000001F

#define KEY_IRQ	 			73
#define MWMAC_IRQ	 		74
#define TRNG_IRQ			75

#define DRIVER_NAME "cryptocore" /* Name des Moduls */

// CryptoCore Operations:

#define MONTMULT			0x0
#define MONTR				0x1
#define MONTR2				0x2
#define MONTEXP				0x3
#define MODADD				0x4
#define MODSUB				0x5
#define COPYH2V				0x6
#define COPYV2V				0x7
#define COPYH2H				0x8
#define COPYV2H				0x9
#define MONTMULT1			0xA
#define MONTEXPFULL			0xB

// CryptoCore RAM Locations

#define MWMAC_RAM_B1 			0x0
#define MWMAC_RAM_B2 			0x1
#define MWMAC_RAM_B3 			0x2
#define MWMAC_RAM_B4 			0x3
#define MWMAC_RAM_B5 			0x4
#define MWMAC_RAM_B6 			0x5
#define MWMAC_RAM_B7 			0x6
#define MWMAC_RAM_B8 			0x7

#define MWMAC_RAM_P1			0x8
#define MWMAC_RAM_P2			0x9
#define MWMAC_RAM_P3			0xA
#define MWMAC_RAM_P4			0xB
#define MWMAC_RAM_P5			0xC
#define MWMAC_RAM_P6			0xD
#define MWMAC_RAM_P7			0xE
#define MWMAC_RAM_P8			0xF

#define MWMAC_RAM_TS1			0x10
#define MWMAC_RAM_TS2			0x11
#define MWMAC_RAM_TS3			0x12
#define MWMAC_RAM_TS4			0x13
#define MWMAC_RAM_TS5			0x14
#define MWMAC_RAM_TS6			0x15
#define MWMAC_RAM_TS7			0x16
#define MWMAC_RAM_TS8			0x17

#define MWMAC_RAM_TC1			0x18
#define MWMAC_RAM_TC2			0x19
#define MWMAC_RAM_TC3			0x1A
#define MWMAC_RAM_TC4			0x1B
#define MWMAC_RAM_TC5			0x1C
#define MWMAC_RAM_TC6			0x1D
#define MWMAC_RAM_TC7			0x1E
#define MWMAC_RAM_TC8			0x1F

#define MWMAC_RAM_A1			0x0
#define MWMAC_RAM_A2			0x1
#define MWMAC_RAM_A3			0x2
#define MWMAC_RAM_A4			0x3
#define MWMAC_RAM_A5			0x4
#define MWMAC_RAM_A6			0x5
#define MWMAC_RAM_A7			0x6
#define MWMAC_RAM_A8			0x7

#define MWMAC_RAM_E1			0x8
#define MWMAC_RAM_E2			0x9
#define MWMAC_RAM_E3			0xA
#define MWMAC_RAM_E4			0xB
#define MWMAC_RAM_E5			0xC
#define MWMAC_RAM_E6			0xD
#define MWMAC_RAM_E7			0xE
#define MWMAC_RAM_E8			0xF

#define MWMAC_RAM_X1			0x10
#define MWMAC_RAM_X2			0x11
#define MWMAC_RAM_X3			0x12
#define MWMAC_RAM_X4			0x13
#define MWMAC_RAM_X5			0x14
#define MWMAC_RAM_X6			0x15
#define MWMAC_RAM_X7			0x16
#define MWMAC_RAM_X8			0x17

static dev_t cryptocore_dev_number;
static struct cdev *driver_object;
static struct class *cryptocore_class;
static struct device *cryptocore_dev;

volatile u32 *HPS_GPIO1_ptr;
volatile u32 *LEDR_ptr;
volatile u32 *KEY_ptr;
volatile u32 *MWMAC_RAM_ptr;
volatile u32 *MWMAC_CMD_ptr;
volatile u32 *MWMAC_IRQ_ptr;
volatile u32 *TRNG_CMD_ptr;
volatile u32 *TRNG_CTR_ptr;
volatile u32 *TRNG_TSTAB_ptr;
volatile u32 *TRNG_TSAMPLE_ptr;
volatile u32 *TRNG_IRQ_ptr;
volatile u32 *TRNG_FIFO_ptr;

volatile u32 mwmac_irq_var;

volatile u32 trng_words_available;

// CryptoCore Driver Supported Precisions:
static u32 PRIME_PRECISIONS[13][2]={ {192,0x0}, {224,0x1}, {256,0x2}, {320,0x3}, {384,0x4}, {512,0x5}, {768,0x6}, {1024,0x7}, {1536,0x8}, {2048,0x9}, {3072,0xA}, {4096,0xB}, {448, 0xD}};
static u32 BINARY_PRECISIONS[16][2]={ {131,0x0}, {163,0x1}, {176,0x2}, {191,0x3}, {193,0x4}, {208,0x5}, {233,0x6}, {239,0x7}, {272,0x8}, {283,0x9}, {304,0xA}, {359,0xB}, {368,0xC}, {409,0xD}, {431,0xE}, {571,0xF} };

// CryptoCore Driver Function Prototyps:
static void Clear_MWMAC_RAM(void);
static void MWMAC_MontMult(MontMult_params_t *MontMult_params_ptr);
static void MWMAC_MontR(MontR_params_t *MontR_params_ptr);
static void MWMAC_MontR2(MontR2_params_t *MontR2_params_ptr);
static void MWMAC_MontExp(MontExp_params_t *MontExp_params_ptr);
static void MWMAC_ModAdd(ModAdd_params_t *ModAdd_params_ptr);
static void MWMAC_ModSub(ModSub_params_t *ModSub_params_ptr);
static void MWMAC_CopyH2V(CopyH2V_params_t *CopyH2V_params_ptr);
static void MWMAC_CopyV2V(CopyV2V_params_t *CopyV2V_params_ptr);
static void MWMAC_CopyH2H(CopyH2H_params_t *CopyH2H_params_ptr);
static void MWMAC_CopyV2H(CopyV2H_params_t *CopyV2H_params_ptr);
static void MWMAC_MontMult1(MontMult1_params_t *MontMult1_params_ptr);
static void MWMAC_MontExpFull(MontExp_params_t *MontExp_params_ptr);
static void MWMAC_ModExp(ModExp_params_t *ModExp_params_ptr);
static void MWMAC_ModRed(ModRed_params_t *ModRed_params_ptr);
// Add further Function Prototypes here...
static void MWMAC_InitCurve(InitCurve_params_t *InitCurve_params_ptr);
static void MWMAC_affine_to_jacobi(affine_to_jacobi_params_t *affine_to_jacobi_params_ptr);
static void MWMAC_PointDouble(PointDouble_params_t *PointDouble_params_ptr);
static void MWMAC_PointAdd(PointAdd_params_t *PointAdd_params_ptr);
static void MWMAC_PointMult(PointMult_params_t *PointMult_params_ptr);
static void MWMAC_jacobi_to_affine(jacobi_to_affine_params_t *jacobi_to_affine_params_ptr);


irq_handler_t key_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 led_val;
	led_val = ioread32(LEDR_ptr);
	iowrite32((led_val ^ 0x00000001), LEDR_ptr); // Toggle LEDR[0]
	
	iowrite32(0x0000000F, KEY_ptr+3);
    
	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t mwmac_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 led_val;
	led_val = ioread32(LEDR_ptr);
	iowrite32((led_val ^ 0x00000001), LEDR_ptr); // Toggle LEDR[0]
	
	iowrite32(0x00000001, MWMAC_IRQ_ptr+3);
	mwmac_irq_var = 1;
    
	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t trng_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	iowrite32(0x00000001, TRNG_IRQ_ptr+3);

	trng_words_available = 1024;
    
	return (irq_handler_t) IRQ_HANDLED;
}

static int cryptocore_driver_open ( struct inode *inode, struct file *instance )
{
	dev_info( cryptocore_dev, "cryptocore_driver_open called\n" );
	return 0;
}

static int cryptocore_driver_close ( struct inode *inode, struct file *instance )
{
	dev_info( cryptocore_dev, "cryptocore_driver_close called\n" );
	return 0;
}

static long cryptocore_driver_ioctl( struct file *instance, unsigned int cmd, unsigned long arg)
{	
	// Add CryptoCore Structs here and allocate kernel memory...
	MontMult_params_t *MontMult_params_ptr = kmalloc(sizeof(MontMult_params_t), GFP_DMA);
	MontR_params_t *MontR_params_ptr = kmalloc(sizeof(MontR_params_t), GFP_DMA);
	MontR2_params_t *MontR2_params_ptr = kmalloc(sizeof(MontR2_params_t), GFP_DMA);
	MontExp_params_t *MontExp_params_ptr = kmalloc(sizeof(MontExp_params_t), GFP_DMA);
	ModAdd_params_t *ModAdd_params_ptr = kmalloc(sizeof(ModAdd_params_t), GFP_DMA);
	ModSub_params_t *ModSub_params_ptr = kmalloc(sizeof(ModSub_params_t), GFP_DMA);
	CopyH2V_params_t *CopyH2V_params_ptr = kmalloc(sizeof(CopyH2V_params_t), GFP_DMA);
	CopyV2V_params_t *CopyV2V_params_ptr = kmalloc(sizeof(CopyV2V_params_t), GFP_DMA);
	CopyH2H_params_t *CopyH2H_params_ptr = kmalloc(sizeof(CopyH2H_params_t), GFP_DMA);
	CopyV2H_params_t *CopyV2H_params_ptr = kmalloc(sizeof(CopyV2H_params_t), GFP_DMA);
	MontMult1_params_t *MontMult1_params_ptr = kmalloc(sizeof(MontMult1_params_t), GFP_DMA);
	ModExp_params_t *ModExp_params_ptr = kmalloc(sizeof(ModExp_params_t), GFP_DMA);
	ModRed_params_t *ModRed_params_ptr = kmalloc(sizeof(ModRed_params_t), GFP_DMA);
	InitCurve_params_t *InitCurve_params_ptr = kmalloc(sizeof(InitCurve_params_t), GFP_DMA);
	affine_to_jacobi_params_t *affine_to_jacobi_params_ptr = kmalloc(sizeof(affine_to_jacobi_params_t), GFP_DMA);
	PointDouble_params_t *PointDouble_params_ptr = kmalloc(sizeof(PointDouble_params_t), GFP_DMA);
	PointAdd_params_t *PointAdd_params_ptr = kmalloc(sizeof(PointAdd_params_t), GFP_DMA);
	PointMult_params_t *PointMult_params_ptr = kmalloc(sizeof(PointMult_params_t), GFP_DMA);
	jacobi_to_affine_params_t *jacobi_to_affine_params_ptr = kmalloc(sizeof(jacobi_to_affine_params_t), GFP_DMA);
	
		
	int rc;
	u32 i;
	u32 trng_val = 0;

	mwmac_irq_var = 0;

	dev_info( cryptocore_dev, "cryptocore_driver_ioctl called 0x%4.4x %p\n", cmd, (void *) arg );

	switch(cmd) {
		case IOCTL_SET_TRNG_CMD:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_CMD_ptr);
			if(trng_val | 0x00000001) {
				for(i=0; i<60; i++){
					udelay(1000); // Give TRNG FIFO time to fill
				}
			}
			break;
		case IOCTL_SET_TRNG_CTR:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_CTR_ptr);
			break;
		case IOCTL_SET_TRNG_TSTAB:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_TSTAB_ptr);
			break;
		case IOCTL_SET_TRNG_TSAMPLE:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_TSAMPLE_ptr);
			break;
		case IOCTL_READ_TRNG_FIFO:
			trng_val = ioread32(TRNG_FIFO_ptr);
			trng_words_available--;
			put_user(trng_val, (u32 *)arg);
			if(trng_words_available == 0) {
				for(i=0; i<60; i++){
					udelay(1000); // Give TRNG FIFO time to fill
				}				
			}
			break;
		case IOCTL_MWMAC_MONTMULT:
			rc = copy_from_user(MontMult_params_ptr, (void *)arg, sizeof(MontMult_params_t));
			MWMAC_MontMult(MontMult_params_ptr);
			rc = copy_to_user((void *)arg, MontMult_params_ptr, sizeof(MontMult_params_t));
			break;
		case IOCTL_MWMAC_MONTR:
			rc = copy_from_user(MontR_params_ptr, (void *)arg, sizeof(MontR_params_t));
			MWMAC_MontR(MontR_params_ptr);
			rc = copy_to_user((void *)arg, MontR_params_ptr, sizeof(MontR_params_t));
			break;			
		case IOCTL_MWMAC_MONTR2:
			rc = copy_from_user(MontR2_params_ptr, (void *)arg, sizeof(MontR2_params_t));
			MWMAC_MontR2(MontR2_params_ptr);
			rc = copy_to_user((void *)arg, MontR2_params_ptr, sizeof(MontR2_params_t));
			break;			
		case IOCTL_MWMAC_MONTEXP:
			rc = copy_from_user(MontExp_params_ptr, (void *)arg, sizeof(MontExp_params_t));
			MWMAC_MontExp(MontExp_params_ptr);
			rc = copy_to_user((void *)arg, MontExp_params_ptr, sizeof(MontExp_params_t));
			break;			
		case IOCTL_MWMAC_MODADD:
			rc = copy_from_user(ModAdd_params_ptr, (void *)arg, sizeof(ModAdd_params_t));
			MWMAC_ModAdd(ModAdd_params_ptr);
			rc = copy_to_user((void *)arg, ModAdd_params_ptr, sizeof(ModAdd_params_t));
			break;
		case IOCTL_MWMAC_MODSUB:
			rc = copy_from_user(ModSub_params_ptr, (void *)arg, sizeof(ModSub_params_t));
			MWMAC_ModSub(ModSub_params_ptr);
			rc = copy_to_user((void *)arg, ModSub_params_ptr, sizeof(ModSub_params_t));
			break;
		case IOCTL_MWMAC_COPYH2V:
			rc = copy_from_user(CopyH2V_params_ptr, (void *)arg, sizeof(CopyH2V_params_t));
			MWMAC_CopyH2V(CopyH2V_params_ptr);
			rc = copy_to_user((void *)arg, CopyH2V_params_ptr, sizeof(CopyH2V_params_t));
			break;
		case IOCTL_MWMAC_COPYV2V:
			rc = copy_from_user(CopyV2V_params_ptr, (void *)arg, sizeof(CopyV2V_params_t));
			MWMAC_CopyV2V(CopyV2V_params_ptr);
			rc = copy_to_user((void *)arg, CopyV2V_params_ptr, sizeof(CopyV2V_params_t));
			break;
		case IOCTL_MWMAC_COPYH2H:
			rc = copy_from_user(CopyH2H_params_ptr, (void *)arg, sizeof(CopyH2H_params_t));
			MWMAC_CopyH2H(CopyH2H_params_ptr);
			rc = copy_to_user((void *)arg, CopyH2H_params_ptr, sizeof(CopyH2H_params_t));
			break;
		case IOCTL_MWMAC_COPYV2H:
			rc = copy_from_user(CopyV2H_params_ptr, (void *)arg, sizeof(CopyV2H_params_t));
			MWMAC_CopyV2H(CopyV2H_params_ptr);
			rc = copy_to_user((void *)arg, CopyV2H_params_ptr, sizeof(CopyV2H_params_t));
			break;
		case IOCTL_MWMAC_MONTMULT1:
			rc = copy_from_user(MontMult1_params_ptr, (void *)arg, sizeof(MontMult1_params_t));
			MWMAC_MontMult1(MontMult1_params_ptr);
			rc = copy_to_user((void *)arg, MontMult1_params_ptr, sizeof(MontMult1_params_t));
			break;
		case IOCTL_MWMAC_MONTEXP_FULL:
			rc = copy_from_user(MontExp_params_ptr, (void *)arg, sizeof(MontExp_params_t));
			MWMAC_MontExpFull(MontExp_params_ptr);
			rc = copy_to_user((void *)arg, MontExp_params_ptr, sizeof(MontExp_params_t));
			break;	
		case IOCTL_MWMAC_MODEXP:
			rc = copy_from_user(ModExp_params_ptr, (void *)arg, sizeof(ModExp_params_t));
			MWMAC_ModExp(ModExp_params_ptr);
			rc = copy_to_user((void *)arg, ModExp_params_ptr, sizeof(ModExp_params_t));
			break;
		case IOCTL_MWMAC_MODRED:
			rc = copy_from_user(ModRed_params_ptr, (void *)arg, sizeof(ModRed_params_t));
			MWMAC_ModRed(ModRed_params_ptr);
			rc = copy_to_user((void *)arg, ModRed_params_ptr, sizeof(ModRed_params_t));
			break;
		// Add further CryptoCore commands here

		case IOCTL_MWMAC_affine2jacobi:
			rc = copy_from_user(affine_to_jacobi_params_ptr, (void *)arg, sizeof(affine_to_jacobi_params_t));
			MWMAC_affine_to_jacobi(affine_to_jacobi_params_ptr);
			rc = copy_to_user((void *)arg, affine_to_jacobi_params_ptr, sizeof(affine_to_jacobi_params_t));
			break;
		
		case IOCTL_MWMAC_PODOUBLE:
			rc = copy_from_user(PointDouble_params_ptr, (void *)arg, sizeof(PointDouble_params_t));
			MWMAC_PointDouble(PointDouble_params_ptr);
			rc = copy_to_user((void *)arg, PointDouble_params_ptr, sizeof(PointDouble_params_t));
			break;
			
		case IOCTL_MWMAC_POADD:
			rc = copy_from_user(PointAdd_params_ptr, (void *)arg, sizeof(PointAdd_params_t));
			MWMAC_PointAdd(PointAdd_params_ptr);
			rc = copy_to_user((void *)arg, PointAdd_params_ptr, sizeof(PointAdd_params_t));
			break;
			
		case IOCTL_MWMAC_POMULT:
			rc = copy_from_user(PointMult_params_ptr, (void *)arg, sizeof(PointMult_params_t));
			MWMAC_PointMult(PointMult_params_ptr);
			rc = copy_to_user((void *)arg, PointMult_params_ptr, sizeof(PointMult_params_t));
			break;
			
		case IOCTL_MWMAC_jacobi2affine:
			rc = copy_from_user(jacobi_to_affine_params_ptr, (void *)arg, sizeof(jacobi_to_affine_params_t));
			MWMAC_jacobi_to_affine(jacobi_to_affine_params_ptr);
			rc = copy_to_user((void *)arg, jacobi_to_affine_params_ptr, sizeof(jacobi_to_affine_params_t));
			break;

		case IOCTL_MWMAC_INITCURVE:
			rc = copy_from_user(InitCurve_params_ptr, (void *)arg, sizeof(InitCurve_params_t));
			MWMAC_InitCurve(InitCurve_params_ptr);
			rc = copy_to_user((void *)arg, InitCurve_params_ptr, sizeof(InitCurve_params_t));
			break;

		default:
			printk("unknown IOCTL 0x%x\n", cmd);

			// Free allocated kernel memory for defined CryptoCore Structs here...
			kfree(MontMult_params_ptr);
			kfree(MontR_params_ptr);
			kfree(MontR2_params_ptr);
			kfree(MontExp_params_ptr);
			kfree(ModAdd_params_ptr);
			kfree(ModSub_params_ptr);
			kfree(CopyH2V_params_ptr);
			kfree(CopyV2V_params_ptr);
			kfree(CopyH2H_params_ptr);
			kfree(CopyV2H_params_ptr);
			kfree(MontMult1_params_ptr);
			kfree(ModExp_params_ptr);
			kfree(ModRed_params_ptr);
			kfree(affine_to_jacobi_params_ptr);
			kfree(PointDouble_params_ptr);
			kfree(PointAdd_params_ptr);
			kfree(PointMult_params_ptr);
			kfree(jacobi_to_affine_params_ptr);
			kfree(InitCurve_params_ptr);
			return -EINVAL;
	}
	
	// Free allocated kernel memory for defined CryptoCore Structs here...
	kfree(MontMult_params_ptr);
	kfree(MontR_params_ptr);
	kfree(MontR2_params_ptr);
	kfree(MontExp_params_ptr);
	kfree(ModAdd_params_ptr);
	kfree(ModSub_params_ptr);
	kfree(CopyH2V_params_ptr);
	kfree(CopyV2V_params_ptr);
	kfree(CopyH2H_params_ptr);
	kfree(CopyV2H_params_ptr);
	kfree(MontMult1_params_ptr);
	kfree(ModExp_params_ptr);
	kfree(ModRed_params_ptr);	
	kfree(affine_to_jacobi_params_ptr);
	kfree(PointDouble_params_ptr);
	kfree(PointAdd_params_ptr);
	kfree(PointMult_params_ptr);
	kfree(jacobi_to_affine_params_ptr);
	kfree(InitCurve_params_ptr);
	return 0;
	
}

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = cryptocore_driver_open,
   .release = cryptocore_driver_close,
   .unlocked_ioctl = cryptocore_driver_ioctl,
};

static int __init cryptocore_init( void )
{
   int value;

   if( alloc_chrdev_region(&cryptocore_dev_number, 0, 1, DRIVER_NAME) < 0 )
      return -EIO;

   driver_object = cdev_alloc(); /* Anmeldeobject reservieren */

   if( driver_object == NULL )
      goto free_device_number;

   driver_object->owner = THIS_MODULE;
   driver_object->ops = &fops;

   if( cdev_add(driver_object, cryptocore_dev_number, 1) )
      goto free_cdev;

   cryptocore_class = class_create( THIS_MODULE, DRIVER_NAME );

   if( IS_ERR( cryptocore_class ) ) {
      pr_err( "cryptocore: no udev support\n");
      goto free_cdev;
   }

   cryptocore_dev = device_create(cryptocore_class, NULL, cryptocore_dev_number, NULL, "%s", DRIVER_NAME );
   dev_info( cryptocore_dev, "cryptocore_init called\n" );

   if(!(request_mem_region(GPIO1_BASE, GPIO1_SPAN, DRIVER_NAME))) {
      pr_err( "timer: request mem_region (GPIO1) failed!\n");
      goto fail_request_mem_region_1;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (LEDR) failed!\n");
      goto fail_request_mem_region_2;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (KEY) failed!\n");
      goto fail_request_mem_region_3;
   }

   if(!(request_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_RAM) failed!\n");
      goto fail_request_mem_region_4;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_CMD) failed!\n");
      goto fail_request_mem_region_5;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_IRQ) failed!\n");
      goto fail_request_mem_region_6;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_CMD) failed!\n");
      goto fail_request_mem_region_7;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_CTR) failed!\n");
      goto fail_request_mem_region_8;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_TSTAB) failed!\n");
      goto fail_request_mem_region_9;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_TSAMPLE) failed!\n");
      goto fail_request_mem_region_10;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_IRQ) failed!\n");
      goto fail_request_mem_region_11;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_FIFO) failed!\n");
      goto fail_request_mem_region_12;
   }

   if(!(HPS_GPIO1_ptr = ioremap(GPIO1_BASE, GPIO1_SPAN))) {
      pr_err( "cryptocore: ioremap (GPIO1) failed!\n");
      goto fail_ioremap_1;
   }

   if(!(LEDR_ptr = ioremap(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN))) {
      pr_err( "cryptocore: ioremap (LEDR) failed!\n");
      goto fail_ioremap_2;
   }

   if(!(KEY_ptr = ioremap(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN))) {
      pr_err( "cryptocore: ioremap (KEY) failed!\n");
      goto fail_ioremap_3;
   }

   if(!(MWMAC_RAM_ptr = ioremap(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_RAM) failed!\n");
      goto fail_ioremap_4;
   }

   if(!(MWMAC_CMD_ptr = ioremap(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_CMD) failed!\n");
      goto fail_ioremap_5;
   }

   if(!(MWMAC_IRQ_ptr = ioremap(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_IRQ) failed!\n");
      goto fail_ioremap_6;
   }

   if(!(TRNG_CMD_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_CMD) failed!\n");
      goto fail_ioremap_7;
   }

   if(!(TRNG_CTR_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_CTR) failed!\n");
      goto fail_ioremap_8;
   }

   if(!(TRNG_TSTAB_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_TSTAB) failed!\n");
      goto fail_ioremap_9;
   }

   if(!(TRNG_TSAMPLE_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_TSAMPLE) failed!\n");
      goto fail_ioremap_10;
   }

   if(!(TRNG_IRQ_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_IRQ) failed!\n");
      goto fail_ioremap_11;
   }

   if(!(TRNG_FIFO_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_FIFO) failed!\n");
      goto fail_ioremap_12;
   }

   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x00000001, MWMAC_IRQ_ptr+3); 
   // Enable IRQ generation for the CryptoCore
   iowrite32(0x00000001, MWMAC_IRQ_ptr+2); 

   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x00000001, TRNG_IRQ_ptr+3);
   // Enable IRQ generation for the CryptoCore TRNG
   iowrite32(0x00000001, TRNG_IRQ_ptr+2);

   
   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x0000000F, KEY_ptr+3);
   // Enable IRQ generation for the 4 buttons
   iowrite32(0x0000000F, KEY_ptr+2);

   value = request_irq (KEY_IRQ, (irq_handler_t) key_irq_handler, IRQF_SHARED, "cryptocore_key_irq_handler", (void *) (key_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (KEY) failed!\n");
      goto fail_irq_1;
   }

   value = request_irq (MWMAC_IRQ, (irq_handler_t) mwmac_irq_handler, IRQF_SHARED, "cryptocore_mwmac_irq_handler", (void *) (mwmac_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (MWMAC) failed!\n");
      goto fail_irq_2;
   }

   value = request_irq (TRNG_IRQ, (irq_handler_t) trng_irq_handler, IRQF_SHARED, "cryptocore_trng_irq_handler", (void *) (trng_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (TRNG) failed!\n");
      goto fail_irq_3;
   } 

   // Turn on green User LED
   iowrite32(0x01000000, (HPS_GPIO1_ptr+1));
   iowrite32(0x01000000, (HPS_GPIO1_ptr));

   return 0;

fail_irq_3:
   free_irq (MWMAC_IRQ, (void*) mwmac_irq_handler);

fail_irq_2:
   free_irq (KEY_IRQ, (void*) key_irq_handler);

fail_irq_1:
   iounmap(TRNG_FIFO_ptr);

fail_ioremap_12:
   iounmap(TRNG_IRQ_ptr);

fail_ioremap_11:
   iounmap(TRNG_TSAMPLE_ptr);

fail_ioremap_10:
   iounmap(TRNG_TSTAB_ptr);

fail_ioremap_9:
   iounmap(TRNG_CTR_ptr);

fail_ioremap_8:
   iounmap(TRNG_CMD_ptr);

fail_ioremap_7:
   iounmap(MWMAC_IRQ_ptr);

fail_ioremap_6:
   iounmap(MWMAC_CMD_ptr);

fail_ioremap_5:
   iounmap(MWMAC_RAM_ptr);

fail_ioremap_4:
   iounmap(KEY_ptr);

fail_ioremap_3:
   iounmap(LEDR_ptr);

fail_ioremap_2:
   iounmap(HPS_GPIO1_ptr);

fail_ioremap_1:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN);

fail_request_mem_region_12:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN);

fail_request_mem_region_11:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN);

fail_request_mem_region_10:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN);

fail_request_mem_region_9:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN);

fail_request_mem_region_8:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN);

fail_request_mem_region_7:
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN);

fail_request_mem_region_6:
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN);

fail_request_mem_region_5:
   release_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN);

fail_request_mem_region_4:
   release_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN);

fail_request_mem_region_3:
   release_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN);

fail_request_mem_region_2:
   release_mem_region(GPIO1_BASE, GPIO1_SPAN);

fail_request_mem_region_1:
   device_destroy( cryptocore_class, cryptocore_dev_number );
   class_destroy( cryptocore_class );

free_cdev:
   kobject_put( &driver_object->kobj );

free_device_number:
   unregister_chrdev_region( cryptocore_dev_number, 1 );
   return -EIO;
}

static void __exit cryptocore_exit( void )
{
   dev_info( cryptocore_dev, "cryptocore_exit called\n" );

   iowrite32(0x00000000, (LEDR_ptr));
   iowrite32(0x00000000, (HPS_GPIO1_ptr));
   iowrite32(0x00000000, (HPS_GPIO1_ptr+1));

   free_irq (KEY_IRQ, (void*) key_irq_handler);
   free_irq (MWMAC_IRQ, (void*) mwmac_irq_handler);
   free_irq (TRNG_IRQ, (void*) trng_irq_handler);

   iounmap(TRNG_FIFO_ptr);
   iounmap(TRNG_IRQ_ptr);
   iounmap(TRNG_TSAMPLE_ptr);
   iounmap(TRNG_TSTAB_ptr);
   iounmap(TRNG_CTR_ptr);
   iounmap(TRNG_CMD_ptr);
   iounmap(MWMAC_IRQ_ptr);
   iounmap(MWMAC_CMD_ptr);
   iounmap(MWMAC_RAM_ptr);
   iounmap(KEY_ptr);
   iounmap(LEDR_ptr);
   iounmap(HPS_GPIO1_ptr);

   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN);
   release_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN);
   release_mem_region(GPIO1_BASE, GPIO1_SPAN);

   device_destroy( cryptocore_class, cryptocore_dev_number );
   class_destroy( cryptocore_class );

   cdev_del( driver_object );
   unregister_chrdev_region( cryptocore_dev_number, 1 );

   return;
}

static void Clear_MWMAC_RAM(void)
{
	u32 value;
	u32 i;
	
	value = 0x00000000;
	
	for(i=0; i<128; i++){
		// Clear B
		iowrite32(value, (MWMAC_RAM_ptr+0x3+i*0x4));
		// Clear P
		iowrite32(value, (MWMAC_RAM_ptr+0x2+i*0x4));
		// Clear TS
		iowrite32(value, (MWMAC_RAM_ptr+0x1+i*0x4));
		// Clear TC
		iowrite32(value, (MWMAC_RAM_ptr+0x0+i*0x4));
		// Clear A
		iowrite32(value, (MWMAC_RAM_ptr+0x200+i));
		// Clear E
		iowrite32(value, (MWMAC_RAM_ptr+0x280+i));
		// Clear X
		iowrite32(value, (MWMAC_RAM_ptr+0x300+i));
	}
	
}


// Helper functions :

int MontMult(int src_b, int dest,u32 mwmac_cmd_prec) {
	
	// MontMult(A1, B1, P1)
	u32 mwmac_cmd = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
	//			src_addr     			dest_addr    		src_addr_e   src_addr_x
			| (src_b << 12) | (dest << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	return 0;
}

int Copy(int src, int dst,int operation,u32 mwmac_cmd_prec) { // operation = COPYH2V,COPYH2H,COPYV2V,COPYV2H
	
	// CopyH2V(reg_B1, reg_A1) -> x_MD in A1
	u32 mwmac_cmd = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (operation << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (src << 12) | (dst << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	return 0;
}

int ModAdd(int src,u32 mwmac_cmd_prec) {
	
	// ModAdd(TS1, TC1) 
	u32 mwmac_cmd = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODADD << 8) 
	//			src_addr      			dest_addr    src_addr_e   src_addr_x
			| (src << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	return 0;
}

int ModSub(int src,u32 mwmac_cmd_prec) {
	
	// ModAdd(TS1, TC1) 
	u32 mwmac_cmd = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
	//			src_addr      			dest_addr    src_addr_e   src_addr_x
			| (src << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	return 0;
}



	
	
int MontExp(int src,int dest,int src_e,int src_x,u32 mwmac_cmd_prec) {	
	
	// MontExp(A1, B1, E1, X1, P1)
	u32 mwmac_cmd = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXP << 8) 
	//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
			| (src << 12) | (dest << 17) | (src_e << 22) | (src_x << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	return 0;
}


int MontMult1(int src,u32 mwmac_cmd_prec) {	
	
	// MontMult1(B1, P1)
	u32 mwmac_cmd = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT1 << 8) 
	//			src_addr      dest_addr    src_addr_e   src_addr_x
			| (src << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	return 0;
}




	


int MontR(int src,int dest,u32 mwmac_cmd_prec){
	
	// MontR(P1, B1)
	u32 mwmac_cmd = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (src << 12) | (dest << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	return 0;
}

int MontR2(int src,int dest,u32 mwmac_cmd_prec){ // need to check if dest can be different from MWMAC_RAM_B1
	
	// MontR(P1, B1)
	u32 mwmac_cmd = 0;
	u32 mwmac_f_sel = 1; // Prime field
	u32 mwmac_sec_calc = 0; // No secure calculation
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR2 << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
	| (src << 12) | (dest << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	return 0;
	
}

static void MWMAC_InitCurve(InitCurve_params_t *InitCurve_params_ptr){
	u32 i;
	// u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(InitCurve_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==InitCurve_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(InitCurve_params_ptr->prec % 32) {
					rw_prec = (InitCurve_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = InitCurve_params_ptr->prec;
				}
			}
		}
	}
	
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==InitCurve_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = InitCurve_params_ptr->prec;
			}
		}
	}	
	
	if(InitCurve_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}	
	
	Clear_MWMAC_RAM();
		
	// p	-> reg_P1
	// p_2	-> reg_A1
	// r2	-> reg_A2
	// a	-> reg_A3
	// md_a	-> reg_A3
	// b 	-> reg_A4
	// md_b	-> reg_A4
	// md_2	-> reg_A5
	// md_3	-> reg_A6
	// md_8	-> reg_A7
	

	// Write Parameter p to P Register Memory 512 i 0 -> 15
	for(i=0; i<rw_prec/32; i++){
		iowrite32(InitCurve_params_ptr->p[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
		
	/*
	// Write Parameter p-2 to A1 Register Memory 512 i 0 -> 15
	for(i=0; i<rw_prec/32; i++){
		iowrite32(InitCurve_params_ptr->p_2[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));
	}
	*/
	
	// Calculate R2 and copy it to A2
	MontR2(MWMAC_RAM_P1,MWMAC_RAM_A2,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_A2,COPYH2V,mwmac_cmd_prec);
		
	// Write Parameter a to A3 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(InitCurve_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+0x10+0x10+i));
	}
	
	// Calculate md_a = MontMult(r2,a) and write it to A3
	
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A3,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_A3,COPYH2V,mwmac_cmd_prec);
	
	// Write Parameter b to A4 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(InitCurve_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+0x10+0x10+0x10+i));
	}

	// Calculate md_b = MontMult(r2,b) and write it to A4
	
	Copy(MWMAC_RAM_A2,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A4,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_A4,COPYH2V,mwmac_cmd_prec);
	
	// Write Parameter 2 to A5 Register Memory
	iowrite32(0x00000002,(MWMAC_RAM_ptr+0x200+0x10+0x10+0x10+0x10));
	
	for(i=1; i<rw_prec/32; i++){
		iowrite32(0x00000000, (MWMAC_RAM_ptr+0x200+0x10+0x10+0x10+0x10+i));
	}	
	
	
	// calculate p-2 and store it in A1 :
	
	Copy(MWMAC_RAM_P1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
	Copy(MWMAC_RAM_A5,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
	ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_A1,COPYH2V,mwmac_cmd_prec);
	
	
	// Calculate md_2 = MontMult(r2,2) and write it to A5
	
	Copy(MWMAC_RAM_A2,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A5,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_A5,COPYH2V,mwmac_cmd_prec);
	
	
	// Write Parameter 3 to A6 Register Memory
	iowrite32(0x00000003,(MWMAC_RAM_ptr+0x200+0x10+0x10+0x10+0x10+0x10));
	for(i=1; i<rw_prec/32; i++){
		iowrite32(0x00000000, (MWMAC_RAM_ptr+0x200+0x10+0x10+0x10+0x10+0x10+i));
	}
	
	// Calculate md_3 = MontMult(r2,3) and write it to A6
	
	Copy(MWMAC_RAM_A2,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A6,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_A6,COPYH2V,mwmac_cmd_prec);
	
	// Write Parameter 8 to A7 Register Memory
	iowrite32(0x00000008,(MWMAC_RAM_ptr+0x200+0x10+0x10+0x10+0x10+0x10+0x10));
	for(i=1; i<rw_prec/32; i++){
		iowrite32(0x00000000, (MWMAC_RAM_ptr+0x200+0x10+0x10+0x10+0x10+0x10+0x10+i));
	}

	// Calculate md_8 = MontMult(r2,8) and write it to A7
	
	Copy(MWMAC_RAM_A2,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A7,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_A7,COPYH2V,mwmac_cmd_prec);
	
}

static void MWMAC_affine_to_jacobi(affine_to_jacobi_params_t *affine_to_jacobi_params_ptr){
	u32 i;
	// u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(affine_to_jacobi_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==affine_to_jacobi_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(affine_to_jacobi_params_ptr->prec % 32) {
					rw_prec = (affine_to_jacobi_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = affine_to_jacobi_params_ptr->prec;
				}
			}
		}
	}
	
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==affine_to_jacobi_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = affine_to_jacobi_params_ptr->prec;
			}
		}
	}	
	
	if(affine_to_jacobi_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}	
	
	// Clear_MWMAC_RAM();
	
	// After calling MWMAC_InitCurve() :
	
	// p	-> reg_P1
	// p_2	-> reg_A1
	// r2	-> reg_A2
	// md_a	-> reg_A3
	// md_b	-> reg_A4
	// md_2	-> reg_A5
	// md_3	-> reg_A6
	// md_8	-> reg_A7

	// Write Parameter p to P Register Memory 512 i 0 -> 15
	/*
	for(i=0; i<rw_prec/32; i++){
		iowrite32(affine_to_jacobi_params_ptr->p[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	*/
	// Write Parameter x_affine to E1 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(affine_to_jacobi_params_ptr->x[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}
	
	// Write Parameter y_affine to E2 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(affine_to_jacobi_params_ptr->y[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x290+i));
	}
	
	// intput are (x,y)
	/*
	// Write Parameter z_Jacobi to E3 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(affine_to_jacobi_params_ptr->z[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2A0+i));
	}
	*/





	// Applying affine to jacobi formula 
	

	// p -> register P1
	// x -> register E1
	// y -> register E2

	// MontR2(P1,X1) -> r2 in register B1
	// CopyH2V(B1,X1) -> r2 in register X1
	// MontMult(B1,E1) -> X_MD in register B1
	// CopyH2V(B1,E3) -> X_MD in register E3
	// CopyV2H(X1,B1) -> r2 in register B1
	// MontMult(B1,E2) -> Y_MD in register B1
	// CopyH2V(B1,E4) -> Y_MD in register E4
	
	// MontR(P1,B1) -> Z_MD in register B1
	// CopyH2V(B1,E5) -> Z_MD in register E5 
	

	// keep copy of (X,Y,Z) for multiplication
	
	// X_MD	-> reg_E6
	// Y_MD	-> reg_E7
	// Z_MD	-> reg_E8
	
	// write E3 in x_md
	// write E4 in y_md
	// write E5 in z_md

    MontR2(MWMAC_RAM_P1,MWMAC_RAM_X1,mwmac_cmd_prec); // A1 MUST NOT BE USED
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec);
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E1,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E3,COPYH2V,mwmac_cmd_prec);
	Copy(MWMAC_RAM_X1,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E2,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E4,COPYH2V,mwmac_cmd_prec);
	
	MontR(MWMAC_RAM_P1,MWMAC_RAM_B1,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E5,COPYH2V,mwmac_cmd_prec);

	// keep copy of (X,Y,Z) for multiplication
	if(affine_to_jacobi_params_ptr->make_copy == 1) {
		Copy(MWMAC_RAM_E3,MWMAC_RAM_E6,COPYV2V,mwmac_cmd_prec);
		Copy(MWMAC_RAM_E4,MWMAC_RAM_E7,COPYV2V,mwmac_cmd_prec);
		Copy(MWMAC_RAM_E5,MWMAC_RAM_E8,COPYV2V,mwmac_cmd_prec);
	}
	
	
	// Read Result x_md from E3 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		affine_to_jacobi_params_ptr->x_md[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+2*0x10+i); // 0x280+0x10+0x10 = 0x2A0
	} 

	// Read Result y_md (copy) from E4 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		affine_to_jacobi_params_ptr->y_md[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+3*0x10+i);
	
	} 

	// Read Result z_md (copy) from E5 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		affine_to_jacobi_params_ptr->z_md[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+4*0x10+i);
	}
	
	
	/*
	
	// Read Result x_md (copy) from E6 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		affine_to_jacobi_params_ptr->x_md[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+5*0x10+i); // 0x280+0x10+0x10 = 0x2A0
	} 

	// Read Result y_md (copy) from E7 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		affine_to_jacobi_params_ptr->y_md[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+6*0x10+i);
	
	} 

	// Read Result z_md (copy) from E8 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		affine_to_jacobi_params_ptr->z_md[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+7*0x10+i);
	}
	*/
}

static void MWMAC_PointDouble(PointDouble_params_t *PointDouble_params_ptr){
	u32 i;
	// u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(PointDouble_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==PointDouble_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(PointDouble_params_ptr->prec % 32) {
					rw_prec = (PointDouble_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = PointDouble_params_ptr->prec;
				}
			}
		}
	}
	
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==PointDouble_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = PointDouble_params_ptr->prec;
			}
		}
	}	
	
	if(PointDouble_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}	
	
	// Clear_MWMAC_RAM();
	// After calling MWMAC_InitCurve() :
		// p	-> reg_P1
		// p_2	-> reg_A1
		// r2	-> reg_A2
		// md_a	-> reg_A3
		// md_b	-> reg_A4
		// md_2	-> reg_A5
		// md_3	-> reg_A6
		// md_8	-> reg_A7
	
	// After calling MWMAC_affine_to_jacobi :
		// x,y in E1,E2
		// X_MD,Y_MD,Z_MD are in E3,E4,E5
		// copy X_MD,Y_MD,Z_MD is kept in E6,E7,E8
		
	// E3,E4,E5 will store intermediate values (X,Y,Z)
		
	
	// Write Parameter p to P Register Memory 512 i 0 -> 15
	/*
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointDouble_params_ptr->p[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	*/
	
	/*
	
	// Write Parameter X_MD to X1 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointDouble_params_ptr->x[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+i));
	}
	
	// Write Parameter Y_MD to X2 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointDouble_params_ptr->y[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+0x10+i));
	}
	
	// Write Parameter Z_MD to X3 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointDouble_params_ptr->z[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+0x10+0x10+i));
	}
	
	*/


	// Applying point doubling formula 
	
	// After calling MWMAC_InitCurve() :
	// p	-> reg_P1
	// p_2	-> reg_A1
	// r2	-> reg_A2
	// md_a	-> reg_A3
	// md_b	-> reg_A4
	// md_2	-> reg_A5
	// md_3	-> reg_A6
	// md_8	-> reg_A7
	
	// After calling MWMAC_affine_to_jacobi() :
	// x	-> reg_E1
	// y	-> reg_E2
	// X_MD	-> reg_E3
	// Y_MD	-> reg_E4
	// Z_MD	-> reg_E5
	
	// keep copy of (X,Y,Z) for multiplication
	// X_MD	-> reg_E6
	// Y_MD	-> reg_E7
	// Z_MD	-> reg_E8
	
	/*
(
	// CopyV2H(E3,B1) 	-> X_MD in reg_B1
	// MontMult(B1,E3) 	-> XX in reg_B1
	// CopyH2V(B1,X1) 	-> XX in reg_X1
	
	// CopyV2H(E4,B1) 	-> Y_MD in reg_B1
	// MontMult(B1,E4) 	-> YY in reg_B1
	// CopyH2V(B1,X2) 	-> YY in reg_X2
	
	// MontMult(B1,X2) 	-> YYYY in reg_B1
	// CopyH2V(B1,X3) 	-> YYYY in reg_X3
	
	// CopyV2H(E5,B1) 	-> Z_MD in reg_B1
	// MontMult(B1,E5) 	-> ZZ in reg_B1
	// CopyH2V(B1,X4) 	-> ZZ in reg_X4

	// MontMult(B1,X4) 	-> ZZ2 in reg_B1
	// CopyH2V(B1,X5) 	-> ZZ2 in reg_X5
	
	// CopyV2H(E3,TC1) 	-> X_MD in reg_TC1
	// CopyV2H(X2,TS1) 	-> YY in reg_TS1
	// ModAdd(TS1)	   	-> S1 in reg_B1
	
	// CopyH2V(B1,X6)  	-> S1 in reg_X6		// X6 can be used
	// MontMult(B1,X6) 	-> S2 in reg_B1
	
	// CopyH2H(B1,TS1)	-> S2 in reg_TS1
	// CopyV2H(X1,TC1)	-> XX in reg_TC1
	// ModSub(TS1)		-> S3 in reg_B1
	
	// CopyH2H(B1,TS1)	-> S3 in reg_TS1
	// CopyV2H(X3,TC1)	-> YYYY in reg_TC1
	// ModSub(TS1)		-> S4 in reg_B1
	
	// MontMult(B1,A5) 	-> S in reg_B1
	// CopyH2V(B1,X6)  	-> S in reg_X6 		// X6 is not free anymore !!!
	
	// CopyV2H(X1,B1) 	-> XX in reg_B1
	// MontMult(B1,A6) 	-> M1 in reg_B1
	// CopyH2V(B1,X1) 	-> M1 in reg_X1 	// overwrite X1 (XX is not used anymore)
	
	// CopyV2H(X5,B1) 	-> ZZ2 in reg_B1	// X5 can be used
	// MontMult(B1,A3) 	-> M2 in reg_B1

	// CopyH2H(B1,TC1) 	-> M2 in reg_TC1
	// CopyV2H(X1,TS1) 	-> M1 in reg_TS1
	// ModAdd(TS1)	   	-> M in reg_B1		// X1 can be used
	// CopyH2V(B1,X1)	-> M in reg_X1		// X1 is not free anymore !!!
	
	// MontMult(B1,X1) 	-> T1 in reg_B1
	// CopyH2V(B1,X5)	-> T1 in reg_X5		// X5 is not free anymore !!!
	
	// CopyV2H(X6,B1) 	-> S in reg_B1
	// MontMult(B1,A5) 	-> T2 in reg_B1

	// CopyV2H(X5,TS1)	-> T1 in reg_TS1	// X5 can be used
	// CopyH2H(B1,TC1)	-> T2 in reg_TC1
	// ModSub(TS1)		-> X3 in reg_B1		
	// CopyH2V(B1,E3)	-> X3 in reg_E3		
	
	// CopyV2H(X3,B1)	-> YYYY in reg_B1	// X3 can be used
	// MontMult(B1,A7) 	-> Y31 in reg_B1
	// CopyH2V(B1,X3)	-> Y31 in reg_X3	// X3 is not free anymore !!!
	
	// CopyV2H(X6,TS1)	-> S in reg_TS1		// X6 can be used
	// CopyV2H(E3,TC1)	-> X3 in reg_TC1
	// ModSub(TS1)		-> Y32 in reg_B1

	// MontMult(B1,X1) -> Y33 in reg_B1 	// X1 can be used
	
	// CopyH2H(B1,TS1)	-> Y33 in reg_TS1	// X5 can be used
	// CopyV2H(X3,TC1)	-> Y31 in reg_TC1	// X3 can be used 
	// ModSub(TS1)		-> Y3 in reg_B1	
	// CopyH2V(B1,X5)	-> Y3 in reg_X5		// X5 is not free anymore !!!
	
	// CopyV2H(E4,TC1) 	-> Y1 in reg_TC1
	// CopyV2H(E5,TS1) 	-> Z1 in reg_TS1
	// ModAdd(TS1)	   	-> Z31 in reg_B1
	
	// CopyH2V(B1,X1) 	-> Z31 in reg_X1	// X1 is not free anymore !!!
	// MontMult(B1,X1) 	-> Z32 in reg_B1 	// X1 can be used
	
	// CopyH2H(B1,TS1)	-> Z32 in reg_TS1	
	// CopyV2H(X2,TC1)	-> YY in reg_TC1	// X2 can be used
	// ModSub(TS1)		-> Z33 in reg_B1
	
	// CopyH2H(B1,TS1)	-> Z33 in reg_TS1	
	// CopyV2H(X4,TC1)	-> ZZ in reg_TC1	// X4 can be used
	// ModSub(TS1)		-> Z3 in reg_B1
	// CopyH2V(B1,E5)	-> Z3 in reg_E5
	// CopyV2V(X5,E4)	-> Y3 in reg_E4		// X5 can be used
)	
	*/
	
	
    Copy(MWMAC_RAM_E3,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_E3,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec); // XX in reg_X1
    
    Copy(MWMAC_RAM_E4,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_E4,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X2,COPYH2V,mwmac_cmd_prec); // YY in reg_X2
    
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_X2,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X3,COPYH2V,mwmac_cmd_prec); // YYYY in reg_X3
    
    Copy(MWMAC_RAM_E5,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_E5,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X4,COPYH2V,mwmac_cmd_prec); // ZZ in reg_X4
	
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_X4,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X5,COPYH2V,mwmac_cmd_prec);	// ZZ2 in reg_X5
    
    Copy(MWMAC_RAM_E3,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X2,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
    ModAdd(MWMAC_RAM_TS1,mwmac_cmd_prec);					// S1 in reg_B1
    
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X6,COPYH2V,mwmac_cmd_prec);	// S1 in reg_X6
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_X6,mwmac_cmd_prec);		// S2 in reg_B1 
    
    Copy(MWMAC_RAM_B1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X1,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec);					// S3 in reg_B1
    
    Copy(MWMAC_RAM_B1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X3,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec);					// S4 in reg_B1
    
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_A5,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X6,COPYH2V,mwmac_cmd_prec);	// S in reg_X6
    
    Copy(MWMAC_RAM_X1,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // XX in reg_B1
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_A6,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec);	// M1 in reg_X1
    
    Copy(MWMAC_RAM_X5,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_A3,mwmac_cmd_prec);		// M2 in reg_B1
    
    Copy(MWMAC_RAM_B1,MWMAC_RAM_TC1,COPYH2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X1,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
    ModAdd(MWMAC_RAM_TS1,mwmac_cmd_prec);					// M in reg_B1
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec);	// M in reg_X1	
	
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_X1,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X5,COPYH2V,mwmac_cmd_prec); // T1 in reg_X5
    
    Copy(MWMAC_RAM_X6,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_A5,mwmac_cmd_prec);		// T2 in reg_B1
    
    Copy(MWMAC_RAM_X5,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_TC1,COPYH2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_E3,COPYH2V,mwmac_cmd_prec);	// X3 in reg_E3
    
    Copy(MWMAC_RAM_X3,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec);
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_A7,mwmac_cmd_prec);		// Y31 in reg_B1
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X3,COPYH2V,mwmac_cmd_prec);	// Y31 in reg_X3
    
    Copy(MWMAC_RAM_X6,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_E3,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec);					// Y32 in reg_B1
    
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_X1,mwmac_cmd_prec);		// Y33 in reg_B1
    
    Copy(MWMAC_RAM_B1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X3,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec);
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X5,COPYH2V,mwmac_cmd_prec);	// Y3 in reg_X5 (reg_E4)
    
    Copy(MWMAC_RAM_E4,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_E5,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
    ModAdd(MWMAC_RAM_TS1,mwmac_cmd_prec);					// Z31 in reg_B1
    
    Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec);
    MontMult(MWMAC_RAM_B1,MWMAC_RAM_X1,mwmac_cmd_prec);		// Z32 in reg_B1
    
    Copy(MWMAC_RAM_B1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X2,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec);					// Z33 in reg_B1
    
    Copy(MWMAC_RAM_B1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X4,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec);					// Z3 in reg_B1
    Copy(MWMAC_RAM_B1,MWMAC_RAM_E5,COPYH2V,mwmac_cmd_prec);	// Z3 in reg_E5
    Copy(MWMAC_RAM_X5,MWMAC_RAM_E4,COPYV2V,mwmac_cmd_prec);	// Y3 in reg_E4
	
	
	// Read Result a from E3 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointDouble_params_ptr->a[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+0x10+0x10+i); // 0x280+0x10+0x10 = 0x2A0
	}

	// Read Result b from E4 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointDouble_params_ptr->b[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+0x10+0x10+0x10+i);
	}

	// Read Result c from E5 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointDouble_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+0x10+0x10+0x10+0x10+i);
	}
	
}

static void MWMAC_PointAdd(PointAdd_params_t *PointAdd_params_ptr) {
	u32 i;
	// u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(PointAdd_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==PointAdd_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(PointAdd_params_ptr->prec % 32) {
					rw_prec = (PointAdd_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = PointAdd_params_ptr->prec;
				}
			}
		}
	}
	
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==PointAdd_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = PointAdd_params_ptr->prec;
			}
		}
	}	
	
	if(PointAdd_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	// Clear_MWMAC_RAM();
	// After calling MWMAC_InitCurve() :
		// p	-> reg_P1
		// p_2	-> reg_A1
		// r2	-> reg_A2
		// md_a	-> reg_A3
		// md_b	-> reg_A4
		// md_2	-> reg_A5
		// md_3	-> reg_A6
		// md_8	-> reg_A7
	
	// After calling MWMAC_affine_to_jacobi :
		// x,y in E1,E2
		// X_MD,Y_MD,Z_MD are in E3,E4,E5
		// copy X_MD,Y_MD,Z_MD is kept in E6,E7,E8
		
	// E3,E4,E5 will store intermediate values (X,Y,Z)
		
	
	// Write Parameter p to P Register Memory 512 i 0 -> 15
	/*
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointAdd_params_ptr->p[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	*/
	
	
	// ************** READING JACOBIAN VALUES FROM INPUT PARAMS *************************
	
	/*
	
	// Write Parameter Xp to X1 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointAdd_params_ptr->x1[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+i));
	}
	
	// Write Parameter Yp to X2 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointAdd_params_ptr->y1[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+0x10+i));
	}
	
	// Write Parameter Zp to X3 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointAdd_params_ptr->z1[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+0x10+0x10+i));
	}
	
	// Write Parameter Xq to X4 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointAdd_params_ptr->x2[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+0x10+0x10+0x10+i));
	}
	
	// Write Parameter Yq to X5 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointAdd_params_ptr->y2[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+0x10+0x10+0x10+0x10+i));
	}
	
	// Write Parameter Zq to X6 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(PointAdd_params_ptr->z2[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+0x10+0x10+0x10+0x10+0x10+i));
	}
	
	*/
	
	
	
	// Applying point addition formula 
	
	// After calling MWMAC_InitCurve() :
	// p	-> reg_P1
	// p_2	-> reg_A1
	// r2	-> reg_A2
	// md_a	-> reg_A3
	// md_b	-> reg_A4
	// md_2	-> reg_A5
	// md_3	-> reg_A6
	// md_8	-> reg_A7
	
	// calling affine_to_jacobi(makeCopy=1) : (XP,YP,ZP) in E6,E7,E8
	// calling affine_to_jacobi(makeCopy=0) : (XQ,YQ,ZQ) in E3,E4,E5 
	
	
	
	// (XP,YP,ZP) in E6,E7,E8 (untouchable)
	// (XQ,YQ,ZQ) in E3,E4,E5
	
	// if P == Q then call point_double
	
	Copy(MWMAC_RAM_E6,MWMAC_RAM_X5,COPYV2V,mwmac_cmd_prec);
	Copy(MWMAC_RAM_E7,MWMAC_RAM_X6,COPYV2V,mwmac_cmd_prec);
	Copy(MWMAC_RAM_E8,MWMAC_RAM_X7,COPYV2V,mwmac_cmd_prec);
	
	
	// XP,YP,ZP -> X5,X6,X7 
	// XQ,YQ,ZQ -> E3,E4,E5
	
	

	 
	Copy(MWMAC_RAM_X7,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // Z1 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X7,mwmac_cmd_prec); // Z1Z1 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E1,COPYH2V,mwmac_cmd_prec); // Z1Z1 in reg_E1
	
	Copy(MWMAC_RAM_E5,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // Z2 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E5,mwmac_cmd_prec); // Z2Z2 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E2,COPYH2V,mwmac_cmd_prec); // Z2Z2 in reg_E2
	
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X5,mwmac_cmd_prec); // U1 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec); // U1 in reg_X1
	
	
	
	Copy(MWMAC_RAM_E3,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // X2 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E1,mwmac_cmd_prec); // U2 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E3,COPYH2V,mwmac_cmd_prec); // U2 in reg_E3
	
	Copy(MWMAC_RAM_X6,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // Y1 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E5,mwmac_cmd_prec); // S11 in reg_B1

	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E2,mwmac_cmd_prec); // S1 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X2,COPYH2V,mwmac_cmd_prec); // S1 in reg_X2
	
	Copy(MWMAC_RAM_E4,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // Y2 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X7,mwmac_cmd_prec); // S21 in reg_B1

	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E1,mwmac_cmd_prec); // S2 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X3,COPYH2V,mwmac_cmd_prec); // S2 in reg_X3
	

	Copy(MWMAC_RAM_E3,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X1,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec); // H in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X4,COPYH2V,mwmac_cmd_prec); // H in reg_X4
	
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A5,mwmac_cmd_prec); // I1 in reg_B1
	
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X5,COPYH2V,mwmac_cmd_prec); // I1 in reg_X5
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X5,mwmac_cmd_prec); // I in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X5,COPYH2V,mwmac_cmd_prec); // I in reg_X5
		
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X4,mwmac_cmd_prec); // J in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X6,COPYH2V,mwmac_cmd_prec); // J in reg_X6
		
	Copy(MWMAC_RAM_X3,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X2,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec); // r1 in reg_B1
	
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A5,mwmac_cmd_prec); // r in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X3,COPYH2V,mwmac_cmd_prec); // r in reg_X3
	
	Copy(MWMAC_RAM_X1,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // U1 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X5,mwmac_cmd_prec); // V in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X5,COPYH2V,mwmac_cmd_prec); // V in reg_X5
	
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A5,mwmac_cmd_prec); // X32 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E4,COPYH2V,mwmac_cmd_prec); // X32 in reg_E4
	
	Copy(MWMAC_RAM_X3,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // r in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X3,mwmac_cmd_prec); // X31 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec); // X31 in reg_X1
	
	Copy(MWMAC_RAM_X1,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
    Copy(MWMAC_RAM_X6,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
    ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec); // X33 in reg_B1
	
	Copy(MWMAC_RAM_B1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
	Copy(MWMAC_RAM_E4,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
	ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec); // X3 in reg_B1
	// Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec); // X3 in reg_X1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E3,COPYH2V,mwmac_cmd_prec); // X3 in reg_E3 	// override

	Copy(MWMAC_RAM_X5,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
	Copy(MWMAC_RAM_E3,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
	ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec); // Y31 in reg_B1
	
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X3,mwmac_cmd_prec); // Y33 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X5,COPYH2V,mwmac_cmd_prec); // Y33 in reg_X5
	
	Copy(MWMAC_RAM_X2,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // S1 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_A5,mwmac_cmd_prec); // Y32 in reg_B1

	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X6,mwmac_cmd_prec); // Y34 in reg_B1
	
	// Copy(MWMAC_RAM_B1,MWMAC_RAM_X2,COPYH2V,mwmac_cmd_prec); // Y34 in reg_X2
	// Copy(MWMAC_RAM_X5,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
	// Copy(MWMAC_RAM_X2,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
	
	Copy(MWMAC_RAM_X5,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_TC1,COPYH2H,mwmac_cmd_prec);
	ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec); // Y3 in reg_B1
	// Copy(MWMAC_RAM_B1,MWMAC_RAM_X6,COPYH2V,mwmac_cmd_prec); //Y3 in reg_X6	// reg_X5 is free
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E4,COPYH2V,mwmac_cmd_prec); 	//Y3 in reg_E4		override
	// Copy(MWMAC_RAM_X6,MWMAC_RAM_E4,COPYV2V,mwmac_cmd_prec);		//Y3 in reg_E4		override

	Copy(MWMAC_RAM_X7,MWMAC_RAM_TS1,COPYV2H,mwmac_cmd_prec);
	Copy(MWMAC_RAM_E5,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
	ModAdd(MWMAC_RAM_TS1,mwmac_cmd_prec); // Z31 in reg_B1
	
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X2,COPYH2V,mwmac_cmd_prec); // Z31 in reg_X2
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X2,mwmac_cmd_prec);	// Z32 in reg_B1	// reg_X2 is free
	
	Copy(MWMAC_RAM_B1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
	Copy(MWMAC_RAM_E1,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
	ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec); // Z33 in reg_B1		// reg_E1 is free
	
	Copy(MWMAC_RAM_B1,MWMAC_RAM_TS1,COPYH2H,mwmac_cmd_prec);
	Copy(MWMAC_RAM_E2,MWMAC_RAM_TC1,COPYV2H,mwmac_cmd_prec);
	ModSub(MWMAC_RAM_TS1,mwmac_cmd_prec); // Z34 in reg_B1		// reg_E2 is free

	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X4,mwmac_cmd_prec);	//Z3 in reg_B1
	// Copy(MWMAC_RAM_B1,MWMAC_RAM_X4,COPYH2V,mwmac_cmd_prec);	//Z3 in reg_X4
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E5,COPYH2V,mwmac_cmd_prec);	//Z3 in reg_E5


	/*

	// Write Result in a (output) from X1 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointAdd_params_ptr->a[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x300+i);
	}

	// Write Result in b (output) from X6 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointAdd_params_ptr->b[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x300+5*0x10+i);
	}

	// Write Result in c (output) from X4 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointAdd_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x300+3*0x10+i);
	}
	*/
	
	// Write Result in a (output) from E3 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointAdd_params_ptr->a[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+2*0x10+i);
	}

	// Write Result in b (output) from E4 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointAdd_params_ptr->b[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+3*0x10+i);
	}

	// Write Result in c (output) from E5 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		PointAdd_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+4*0x10+i);
	}
}

static void MWMAC_PointMult(PointMult_params_t *PointMult_params_ptr) {
	
	// Generate random number.
	// set the 1st 2bits (MSB) to '01'
	// for each bit b in n (scalar)
		// double()
		// if (b=1) add()
	// write output to E3,E4,E5
}

static void MWMAC_jacobi_to_affine(jacobi_to_affine_params_t *jacobi_to_affine_params_ptr) {


	u32 i;
	// u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(jacobi_to_affine_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==jacobi_to_affine_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(jacobi_to_affine_params_ptr->prec % 32) {
					rw_prec = (jacobi_to_affine_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = jacobi_to_affine_params_ptr->prec;
				}
			}
		}
	}
	
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==jacobi_to_affine_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = jacobi_to_affine_params_ptr->prec;
			}
		}
	}	
	
	if(jacobi_to_affine_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	// Clear_MWMAC_RAM();
	// After calling MWMAC_InitCurve() :
		// p	-> reg_P1
		// p_2	-> reg_A1
		// r2	-> reg_A2
		// md_a	-> reg_A3
		// md_b	-> reg_A4
		// md_2	-> reg_A5
		// md_3	-> reg_A6
		// md_8	-> reg_A7
	
	// After calling MWMAC_affine_to_jacobi :
		// x,y in E1,E2
		// X_MD,Y_MD,Z_MD are in E3,E4,E5
		// copy of P X_MD,Y_MD,Z_MD is kept in E6,E7,E8
		
	// E3,E4,E5 will store intermediate values (X,Y,Z)
	
	
	// Read X_MD value from E3
	// Read Y_MD value from E4
	// Read Z_MD value from E5
	
	

	
	
	
	// Apply jacobi to affine formula
	
	// zinv = ModExp(Z_MD,p-2)


	// dest is the addr of R :
		// MontR(MWMAC_RAM_P1,MWMAC_RAM_B1) // R in reg_B1
		// Copy(B1,X1)						// R in reg_X1
	// Exponent (P-2) in src_e
	// Modulo P in src
	// base (Z) in src_x
	
	// Montgomerize Z_MD
	
	
	Copy(MWMAC_RAM_A2,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // R2 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E5,mwmac_cmd_prec); // Z_MD (montgomorized) in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E5,COPYH2V,mwmac_cmd_prec); // Z_MD (montgomorized) in reg_E5

	MontR(MWMAC_RAM_P1,MWMAC_RAM_B1,mwmac_cmd_prec);		// R in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec); // R in reg_X1
	MontExp(MWMAC_RAM_B1,MWMAC_RAM_X1,MWMAC_RAM_A1,MWMAC_RAM_E5,mwmac_cmd_prec); // zinv_tmp1 in reg_B1 (maybe in reg_X1 also)
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X1,COPYH2V,mwmac_cmd_prec); // zinv_tmp1 in reg_X2

	MontMult1(MWMAC_RAM_B1,mwmac_cmd_prec);
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X2,COPYH2V,mwmac_cmd_prec); // zinv_tmp1 in reg_X2

	Copy(MWMAC_RAM_A2,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // R2 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X2,mwmac_cmd_prec); 	// zinv_tmp2 in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X2,COPYH2V,mwmac_cmd_prec); // zinv_tmp2 in reg_X2

	Copy(MWMAC_RAM_A2,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // R2 in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X2,mwmac_cmd_prec); 	// zinv_MD in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X2,COPYH2V,mwmac_cmd_prec); // zinv_MD in reg_X2
	
	
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X2,mwmac_cmd_prec); 	// zinv2_MD in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X3,COPYH2V,mwmac_cmd_prec); // zinv2_MD in reg_X3

	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X2,mwmac_cmd_prec); 	// zinv3_MD in reg_B1

	MontMult(MWMAC_RAM_B1,MWMAC_RAM_E4,mwmac_cmd_prec); 	// y_affine_MD in reg_B1
/////////// calculate y_affine first (avoid copy)
	Copy(MWMAC_RAM_B1,MWMAC_RAM_X4,COPYH2V,mwmac_cmd_prec); // y_affine_MD in reg_X4

	Copy(MWMAC_RAM_E3,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // X_MD in reg_B1
	MontMult(MWMAC_RAM_B1,MWMAC_RAM_X3,mwmac_cmd_prec); 	// x_affine_MD in reg_B1

	MontMult1(MWMAC_RAM_B1,mwmac_cmd_prec);					// x_affine in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E3,COPYH2V,mwmac_cmd_prec); // x_affine in reg_E3

	Copy(MWMAC_RAM_X4,MWMAC_RAM_B1,COPYV2H,mwmac_cmd_prec); // y_affine_MD in reg_B1
	MontMult1(MWMAC_RAM_B1,mwmac_cmd_prec);					// y_affine in reg_B1
	Copy(MWMAC_RAM_B1,MWMAC_RAM_E4,COPYH2V,mwmac_cmd_prec); // y_affine in reg_E4

	// Write (x,y) in E3 and E4


	// Write Result in a (output) from E3 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		jacobi_to_affine_params_ptr->a[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+2*0x10+i);
	}

	// Write Result in b (output) from E4 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		jacobi_to_affine_params_ptr->b[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+3*0x10+i);
	}

}














































static void MWMAC_MontMult(MontMult_params_t *MontMult_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontMult_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontMult_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontMult_params_ptr->prec % 32) {
					rw_prec = (MontMult_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontMult_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontMult_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontMult_params_ptr->prec;
			}
		}
	}	
	
	if(MontMult_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}	
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
	// Write Parameter a to A Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));
	}

	// Write Parameter b to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}	
	
	// MontMult(A1, B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
	//			src_addr     			dest_addr    		src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontMult_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_MontR(MontR_params_t *MontR_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontR_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontR_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontR_params_ptr->prec % 32) {
					rw_prec = (MontR_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontR_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontR_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontR_params_ptr->prec;
			}
		}
	}	
	
	if(MontR_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontR_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result r from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontR_params_ptr->r[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_MontR2(MontR2_params_t *MontR2_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontR2_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontR2_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontR2_params_ptr->prec % 32) {
					rw_prec = (MontR2_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontR2_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontR2_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontR2_params_ptr->prec;
			}
		}
	}			
	
	if(MontR2_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontR2_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// MontR2(P1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR2 << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result r from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontR2_params_ptr->r2[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_MontExp(MontExp_params_t *MontExp_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontExp_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontExp_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontExp_params_ptr->prec % 32) {
					rw_prec = (MontExp_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontExp_params_ptr->prec;
				}	
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontExp_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontExp_params_ptr->prec;
			}
		}
	}	
	
	if(MontExp_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// Write Parameter e to E Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->e[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}	

	// Write Parameter b to X Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+i));
	}
	
	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// MontExp(A1, B1, E1, X1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXP << 8) 
	//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontExp_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_ModAdd(ModAdd_params_t *ModAdd_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(ModAdd_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ModAdd_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ModAdd_params_ptr->prec % 32) {
					rw_prec = (ModAdd_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ModAdd_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ModAdd_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ModAdd_params_ptr->prec;
			}
		}
	}		
	
	if(ModAdd_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModAdd_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
	// Write Parameter a to TS Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModAdd_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x1+i*0x4));
	}	

	// Write Parameter b to TC Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModAdd_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x0+i*0x4));
	}	
	
	// ModAdd(TS1, TC1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODADD << 8) 
	//			src_addr      			dest_addr    src_addr_e   src_addr_x
			| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ModAdd_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_ModSub(ModSub_params_t *ModSub_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(ModSub_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ModSub_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ModSub_params_ptr->prec % 32) {
					rw_prec = (ModSub_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ModSub_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ModSub_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ModSub_params_ptr->prec;
			}
		}
	}		
	
	if(ModSub_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModSub_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
	// Write Parameter a to TS Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModSub_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x1+i*0x4));
	}	

	// Write Parameter b to TC Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModSub_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x0+i*0x4));
	}	
	
	// ModSub(TS1, TC1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
	//			src_addr      			dest_addr    src_addr_e   src_addr_x
			| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ModSub_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_CopyH2V(CopyH2V_params_t *CopyH2V_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(CopyH2V_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==CopyH2V_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(CopyH2V_params_ptr->prec % 32) {
					rw_prec = (CopyH2V_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = CopyH2V_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==CopyH2V_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = CopyH2V_params_ptr->prec;
			}
		}
	}	
	
	if(CopyH2V_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter a to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(CopyH2V_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}
	
	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result acopy from A Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		CopyH2V_params_ptr->acopy[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x200+i);
	} 
}

static void MWMAC_CopyV2V(CopyV2V_params_t *CopyV2V_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(CopyV2V_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==CopyV2V_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(CopyV2V_params_ptr->prec % 32) {
					rw_prec = (CopyV2V_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = CopyV2V_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==CopyV2V_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = CopyV2V_params_ptr->prec;
			}
		}
	}			
	
	if(CopyV2V_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter a to A Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(CopyV2V_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));
	}
	
	// CopyV2V(A1, X1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_A1 << 12) | (MWMAC_RAM_X1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result acopy from X Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		CopyV2V_params_ptr->acopy[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x300+i);
	} 
}

static void MWMAC_CopyH2H(CopyH2H_params_t *CopyH2H_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;
	
	if(CopyH2H_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==CopyH2H_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(CopyH2H_params_ptr->prec % 32) {
					rw_prec = (CopyH2H_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = CopyH2H_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==CopyH2H_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = CopyH2H_params_ptr->prec;
			}
		}
	}		
	
	if(CopyH2H_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter a to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(CopyH2H_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}
	
	// CopyH2H(B1, TS1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
	//			src_addr      			dest_addr    			src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result acopy from TS Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		CopyH2H_params_ptr->acopy[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x1+i*0x4);
	} 	

}

static void MWMAC_CopyV2H(CopyV2H_params_t *CopyV2H_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;
	
	if(CopyV2H_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==CopyV2H_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(CopyV2H_params_ptr->prec % 32) {
					rw_prec = (CopyV2H_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = CopyV2H_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==CopyV2H_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = CopyV2H_params_ptr->prec;
			}
		}
	}		
	
	if(CopyV2H_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter a to A Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(CopyV2H_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));
	}
	
	// CopyV2H(A1, TS1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
	//			src_addr      			dest_addr    		src_addr_e   src_addr_x
			| (MWMAC_RAM_A1 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result acopy from TS Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		CopyV2H_params_ptr->acopy[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x1+i*0x4);
	} 
}

static void MWMAC_MontMult1(MontMult1_params_t *MontMult1_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontMult1_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontMult1_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontMult1_params_ptr->prec % 32) {
					rw_prec = (MontMult1_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontMult1_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontMult1_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontMult1_params_ptr->prec;
			}
		}
	}			
	
	if(MontMult1_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult1_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
	// Write Parameter b to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult1_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}	
	
	// MontMult1(B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT1 << 8) 
	//			src_addr      dest_addr    src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;		
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontMult1_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_MontExpFull(MontExp_params_t *MontExp_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontExp_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontExp_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontExp_params_ptr->prec % 32) {
					rw_prec = (MontExp_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontExp_params_ptr->prec;
				}	
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontExp_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontExp_params_ptr->prec;
			}
		}
	}	
	
	if(MontExp_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// Write Parameter e to E Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->e[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}	

	// Write Parameter b to X Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+i));
	}
	
	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// MontExp(A1, B1, E1, X1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXPFULL << 8) 
	//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontExp_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_ModExp(ModExp_params_t *ModExp_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;
	
	if(ModExp_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ModExp_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ModExp_params_ptr->prec % 32) {
					rw_prec = (ModExp_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ModExp_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ModExp_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ModExp_params_ptr->prec;
			}
		}
	}	
	
	if(ModExp_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModExp_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// MontR2(P1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR2 << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// Write Parameter b to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModExp_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}
	
	// Write Parameter e to E Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModExp_params_ptr->e[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}	
	
	// MontMult(A1, B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
	//			src_addr     			dest_addr    		src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// CopyH2V(B1, X1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_X1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// MontExp(A1, B1, E1, X1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXP << 8) 
	//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// MontMult1(B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT1 << 8) 
	//			src_addr      dest_addr    src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ModExp_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_ModRed(ModRed_params_t *ModRed_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_full_prec = 0;
	u32 mwmac_cmd_half_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_full_prec = 0;
	u32 rw_half_prec = 0;

	if(ModRed_params_ptr->f_sel == 0) {
		// Modular Reduction is not supported in GF(2^m)
		mwmac_f_sel = 0;
//		for(i=0; i<16; i++){
//			if(BINARY_PRECISIONS[i][0]==ModRed_params_ptr->prec){
//				mwmac_cmd_full_prec = BINARY_PRECISIONS[i][1];
//			}
//			if(BINARY_PRECISIONS[i][0]==ModRed_params_ptr->prec/2){
//				mwmac_cmd_half_prec = BINARY_PRECISIONS[i][1];
//			}
//		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ModRed_params_ptr->prec){
				mwmac_cmd_full_prec = PRIME_PRECISIONS[i][1];
				rw_full_prec = ModRed_params_ptr->prec;
			}
			if(PRIME_PRECISIONS[i][0]==ModRed_params_ptr->prec/2){
				mwmac_cmd_half_prec = PRIME_PRECISIONS[i][1];
				rw_half_prec = ModRed_params_ptr->prec/2;
			}
		}
	}		
	
	if(ModRed_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}

	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_half_prec/32; i++){
		iowrite32(ModRed_params_ptr->n[rw_half_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// MontR2(P1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_half_prec << 4) | (MONTR2 << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_half_prec << 4) | (COPYH2V << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// Write Parameter a to B Register Memory
	for(i=0; i<rw_full_prec/32; i++){
		iowrite32(ModRed_params_ptr->a[rw_full_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}
	
	// MontMult(A1, B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_full_prec << 4) | (MONTMULT << 8) 
	//			src_addr     			dest_addr    		src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_full_prec/32; i++){
		ModRed_params_ptr->c[rw_full_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

// Add further CryptoCore Functions here...






module_init( cryptocore_init );
module_exit( cryptocore_exit );

MODULE_AUTHOR("MAI - Selected Topics of Embedded Software Development II");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("The driver for the FPGA-based CryptoCore");
MODULE_SUPPORTED_DEVICE("CryptoCore");

