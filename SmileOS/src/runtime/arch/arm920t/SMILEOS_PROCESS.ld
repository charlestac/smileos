/***********************************************************************/
/*                                                                     */
/*  RAM.ld:  Linker Script File                                        */
/*                                                                     */
/***********************************************************************/
ENTRY(_start)

/* Memory Definitions */
MEMORY
{
  RAM (rw) : ORIGIN = 0x00000000, LENGTH = 0x02000000
}

/* Section Definitions */
SECTIONS
{
  /* first section is .text which is used for code */
  .text :
  {
    *crt0.o (.text)            /* Startup code */
    *(.text)                   /* remaining code */
    *(.rodata)                 /* read-only data (constants) */
    *(.rodata*)
    *(.glue_7)
    *(.glue_7t)
  } > RAM

  __exidx_start = .;
  .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
  __exidx_end = .;

  . = ALIGN(4);
  _etext = . ;
  PROVIDE (etext = .);
  
  .ctors :
  {
      KEEP (*cppRtBegin*.o(.ctors))
      KEEP (*(.preinit_array))
      KEEP (*(.init_array))
      KEEP (*(SORT(.ctors.*)))
      KEEP (*(.ctors))
      KEEP (*cppRtEnd*.o(.ctors))
  } > RAM

  .dtors :
  {
      KEEP (*cppRtBegin*.o(.dtors))
      KEEP (*(.fini_array))
      KEEP (*(SORT(.dtors.*)))
      KEEP (*(.dtors))
      KEEP (*cppRtEnd*.o(.dtors))
  } > RAM

  /* .data section which is used for initialized data */
  .data :
  {
    _data = .;
    *(.data)
  } > RAM

  . = ALIGN(4);
  _edata = . ;
  PROVIDE (edata = .);

  /* .bss section which is used for uninitialized data */
  .bss (NOLOAD) :
  {
    __bss_start = . ;
    __bss_start__ = . ;
    *(.bss)
    *(COMMON)
    . = ALIGN(4);
  } > RAM

  . = ALIGN(4);
  __bss_end__ = . ;
  PROVIDE (__bss_end = .);

  _end = . ;
  PROVIDE (end = .);
}
