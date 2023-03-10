/*
 * (C) Copyright 2018
 * wangwei <wangwei@allwinnertech.com>
 */

#include <config.h>
#include <private_boot0.h>
#include <commit_info.h>

#ifdef CFG_ARCH_RISCV
#define BROM_FILE_HEAD_SIZE		(sizeof(boot0_file_head_t) & 0x00FFFFF)
#define BROM_FILE_HEAD_BIT_10_1		((BROM_FILE_HEAD_SIZE & 0x7FE) >> 1)
#define BROM_FILE_HEAD_BIT_11		((BROM_FILE_HEAD_SIZE & 0x800) >> 11)
#define BROM_FILE_HEAD_BIT_19_12	((BROM_FILE_HEAD_SIZE & 0xFF000) >> 12)
#define BROM_FILE_HEAD_BIT_20		((BROM_FILE_HEAD_SIZE & 0x100000) >> 20)

#define BROM_FILE_HEAD_SIZE_OFFSET	((BROM_FILE_HEAD_BIT_20 << 31) | \
									(BROM_FILE_HEAD_BIT_10_1 << 21) | \
									(BROM_FILE_HEAD_BIT_11 << 20) | \
									(BROM_FILE_HEAD_BIT_19_12 << 12))
#define JUMP_INSTRUCTION		(BROM_FILE_HEAD_SIZE_OFFSET | 0x6f)
#else
#define BROM_FILE_HEAD_SIZE_OFFSET	(((sizeof(boot0_file_head_t) + sizeof(int) - 1) / sizeof(int) - 2))
#define JUMP_INSTRUCTION		(BROM_FILE_HEAD_SIZE_OFFSET | 0xEA000000)
#endif

const boot0_file_head_t  BT0_head = {
	{
		/* jump_instruction*/
		JUMP_INSTRUCTION,
		BOOT0_MAGIC,
		STAMP_VALUE,
#ifdef ALIGN_SIZE_8K
		0x2000,
#else
		0x1000,
#endif
		sizeof(boot_file_head_t),
		BOOT_PUB_HEAD_VERSION,
		CONFIG_BOOT0_RET_ADDR,
		CONFIG_BOOT0_RUN_ADDR,
		0,
		{
		/*brom modify: nand-4bytes sdmmc-2bytes*/
		0, 0, 0, 0, '4', '.', '0', 0
		},
	},

	{
		/*__u32 prvt_head_size;*/
		0,
		/*char prvt_head_vsn[4];*/
		1,
		0,/* power_mode */
		{0},/* reserver[2]  */
		/*unsigned int     dram_para[32] ;*/
		{0},
		/*__s32	uart_port;*/
		0,
		/*normal_gpio_cfg   uart_ctrl[2];*/
		{
		{6, 2, 4, 1, 1, 0, {0} },/*PB8: 4--RX*/
		{6, 4, 4, 1, 1, 0, {0} },/*PB9: 4--TX*/
		},
		/*__s32 enable_jtag;*/
		0,
		/*normal_gpio_cfg	 jtag_gpio[5];*/
		{{0}, {0}, {0}, {0}, {0} },
		/*normal_gpio_cfg   storage_gpio[32];*/
		{
			{0, 0, 2, 1, 2, 0, {0} },/*PF0-5: 2--SDC*/
			{0, 1, 2, 1, 2, 0, {0} },
			{0, 2, 2, 1, 2, 0, {0} },
			{0, 3, 2, 1, 2, 0, {0} },
			{0, 4, 2, 1, 2, 0, {0} },
			{0, 5, 2, 1, 2, 0, {0} },
		},
		/*char  storage_data[512 - sizeof(normal_gpio_cfg) * 32];*/
		{0}
	},

	{CI_INFO},
#ifdef CFG_SUNXI_SELECT_DRAM_PARA
	.fes_union_addr.extd_head.magic = DRAM_EXT_MAGIC,
#endif
};



/*******************************************************************************
*
*                  ??????Boot_file_head??????jump_instruction??????
*
*  jump_instruction???????????????????????????????????????( B  BACK_OF_Boot_file_head )?????????
*??????????????????????????????????????????Boot_file_head????????????????????????
*
*  ARM????????????B?????????????????????
*          +--------+---------+------------------------------+
*          | 31--28 | 27--24  |            23--0             |
*          +--------+---------+------------------------------+
*          |  cond  | 1 0 1 0 |        signed_immed_24       |
*          +--------+---------+------------------------------+
*  ???ARM Architecture Reference Manual????????????????????????????????????
*  Syntax :
*  B{<cond>}  <target_address>
*    <cond>    Is the condition under which the instruction is executed. If the
*              <cond> is ommitted, the AL(always,its code is 0b1110 )is used.
*    <target_address>
*              Specified the address to branch to. The branch target address is
*              calculated by:
*              1.  Sign-extending the 24-bit signed(wro's complement)immediate
*                  to 32 bits.
*              2.  Shifting the result left two bits.
*              3.  Adding to the contents of the PC, which contains the address
*                  of the branch instruction plus 8.
*
*  ???????????????????????????????????????8?????????0b11101010??????24?????????Boot_file_head????????????
*????????????????????????????????????????????????
*  ( sizeof( boot_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int )
*                                              ??????????????????????????????????????????
*  - 2                                         ??????PC?????????????????????
*  & 0x00FFFFFF                                ??????signed-immed-24
*  | 0xEA000000                                ?????????B??????
*
*******************************************************************************/

