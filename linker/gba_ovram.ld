OUTPUT_FORMAT( "elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH( arm)
ENTRY( _start)

MEMORY {
	rom	: ORIGIN = 0x08000000, LENGTH = 32M
	ovram		: ORIGIN = 0x06015400, LENGTH = 10K
	ovram_iwram	: ORIGIN = 0x06017C00, LENGTH = 1K
}

__iwram_start	=	ORIGIN(ovram_iwram);
__iwram_top		=	ORIGIN(ovram_iwram) + LENGTH(ovram_iwram);;

__ram_start		=	ORIGIN(ovram);
__ram_end		=	ORIGIN(ovram) + LENGTH(ovram);;

__sp_irq		=	__iwram_top - 0x060;
__sp_usr		=	__sp_irq - 0x0a0;
__irq_flags		=	__iwram_top - 8;

SECTIONS
{
	.  = __ram_start;

	.crt0 :
	{
		*(.crt0)
		. = ALIGN(4);
	} = 0xff

	.text :
	{
		*(.text)
		. = ALIGN(4);
	} = 0xff

	.rodata :
	{
		*(.rodata)
		. = ALIGN(4);
	} = 0xff

	.data :
	{
		*(.data)
		. = ALIGN(16);
		_prog_end = ABSOLUTE(.);
	} = 0xff

	.bss :
	{
		__bss_start__ = .;
		*(.bss)
		. = ALIGN(4);
		__bss_end__ = ABSOLUTE(.);
		__end__ = ABSOLUTE(.);
	}
}

