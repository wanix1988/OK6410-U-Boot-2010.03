#include <common.h>  
#ifdef CONFIG_S3C64XX  
#include <asm/io.h>  
#include <linux/mtd/nand.h>  
#include <asm/arch/s3c6410.h>  
  
static int nandll_read_page (uchar *buf, ulong addr, int large_block)  
{  
     int i;
  int page_size = 512;

  if (large_block==1)
     page_size = 2048;
  if (large_block==2)
    page_size = 4096;

        NAND_ENABLE_CE();

        NFCMD_REG = NAND_CMD_READ0;

        /* Write Address */
        NFADDR_REG = 0;

  if (large_block)
          NFADDR_REG = 0;

  NFADDR_REG = (addr) & 0xff;
  NFADDR_REG = (addr >> 8) & 0xff;
  NFADDR_REG = (addr >> 16) & 0xff;

  if (large_block)
    NFCMD_REG = NAND_CMD_READSTART;

        NF_TRANSRnB();

  /* for compatibility(2460). u32 cannot be used. by scsuh */
  for(i=0; i < page_size; i++)
  {
                *buf++ = NFDATA8_REG;
         }

        NAND_DISABLE_CE();
        return 0;
}  
  
static int nandll_read_blocks (ulong dst_addr, ulong size, int large_block)  
{  
     uchar *buf = (uchar *)dst_addr;
        int i;
  uint page_shift = 9;

  if (large_block==1)
    page_shift = 11;

        /* Read pages */
  if(large_block==2)
    page_shift = 12;
 
  if(large_block == 2)
  {
   /* Read pages */
    for (i = 0; i < 4; i++, buf+=(1<<(page_shift-1)))
  {
           nandll_read_page(buf, i, large_block);
    }

 

  /* Read pages */
    for (i = 4; i < (0x3c000>>page_shift); i++, buf+=(1<<page_shift))
   {
    nandll_read_page(buf, i, large_block);
    }
  }
  else
  {
    for (i = 0; i < (0x3c000>>page_shift); i++, buf+=(1<<page_shift))
   {
            nandll_read_page(buf, i, large_block);
    }
  }

        return 0;
}  
 
 
int copy_uboot_to_ram(void)  
{  
     int large_block = 0;
  int i;
  vu_char id;
 
        NAND_ENABLE_CE();
        NFCMD_REG = NAND_CMD_READID;
        NFADDR_REG =  0x00;

  /* wait for a while */
        for (i=0; i<200; i++);
   id = NFDATA8_REG;
   id = NFDATA8_REG;

  if (id > 0x80)
    large_block = 1;
  if(id == 0xd5)
    large_block = 2;


 /* read NAND Block.
  * 128KB ->240KB because of U-Boot size increase. by scsuh
  * So, read 0x3c000 bytes not 0x20000(128KB).
  */
  return nandll_read_blocks(CONFIG_SYS_PHY_UBOOT_BASE, 0x3c000, large_block);
}  
  
#endif   

