OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{
	. = SBOOTADDR;
	. = ALIGN(4);

	.head :
	{
		main/sboot_head.o (.rodata)
	}
	.text :
	{
		CPUDIR/sboot_entry.o (.text)
		*(.text)
	}
	. = ALIGN(16);
  	.rodata : { *(.rodata) }
	. = ALIGN(16);
  	.data : { *(.data) }

	. = ALIGN(4);
	.bss :
	{
	__bss_start = .;
		*(.bss)
	}
	. = ALIGN(4);
	__bss_end = .;

	_end = .;
}



