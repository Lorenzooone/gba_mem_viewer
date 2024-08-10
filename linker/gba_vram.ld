/***********************************/
/* NINTENDO E-READER LINKER SCRIPT */
/***********************************/
/* Author  : Tim Schuerewegen      */
/* Version : 1.0                   */
/***********************************/

OUTPUT_FORMAT( "elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH( arm)
ENTRY( _start)

MEMORY {
	rom	: ORIGIN = 0x08000000, LENGTH = 32M
	vram		: ORIGIN = 0x06010800, LENGTH = 14K
	vram_iwram	: ORIGIN = 0x06014000, LENGTH = 16K
}

__iwram_start	=	ORIGIN(vram_iwram);
__iwram_top	=	ORIGIN(vram_iwram) + LENGTH(vram_iwram);;

__vram_start	=	ORIGIN(vram);
__vram_end		=	ORIGIN(vram) + LENGTH(vram);;

__sp_irq	=	__iwram_top - 0x060;
__sp_usr	=	__sp_irq - 0x0a0;
__irq_flags	=	__iwram_top - 8;

SECTIONS
{
	.  = 0x06010800;

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
		. = ALIGN(4);
	} = 0xff

	.bss :
	{
		__bss_start = .;
		*(.bss)
		. = ALIGN(4);
		__bss_end__ = ABSOLUTE(.);
		__end__ = ABSOLUTE(.);
	}
}

