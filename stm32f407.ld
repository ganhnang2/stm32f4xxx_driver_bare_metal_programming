/*
  Bộ nhớ FLASH:
  -----------------------------------------------------
  |                   .text (Code)                    |
  -----------------------------------------------------
  |                   .rodata (Constant)              |
  -----------------------------------------------------
  |                   .preinit_array                  |
  -----------------------------------------------------
  |                   .init_array                     |
  -----------------------------------------------------
  |                   .fini_array                     |
  -----------------------------------------------------
  |                  .data (Initialized Data)         |
  -----------------------------------------------------
  |                  .bss (Uninitialized Data)        |
  -----------------------------------------------------
  |                  .heap (Heap)                     |
  -----------------------------------------------------
  |                  .stack (Stack)                   |
  -----------------------------------------------------
  
  Bộ nhớ RAM:
  -----------------------------------------------------
  |                   .data 		                  |
  -----------------------------------------------------
  |                   .bss 			                  |
  -----------------------------------------------------
  |                   Heap			                  |
  -----------------------------------------------------
  |                   Stack		                  	  |
  -----------------------------------------------------
*/

/* Điểm bắt đầu của chương trình */
ENTRY(Reset_Handler)

/* Đỉnh của Stack (112KB SRAM1) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Kích thước tối thiểu dành riêng cho Heap và Stack
 * Linker sẽ báo lỗi nếu RAM không đủ chứa cả hai	*/
_Min_Heap_Size  = 0x1000;  /* 4 KB  — Tăng lên để printf và semihosting hoạt động ổn định */
_Min_Stack_Size = 0x1000;  /* 4 KB  — Tăng lên cho call stack của semihosting */

/*	Bộ nhớ trong vi điều khiển STM32F407xxx	*/
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 112K
}

SECTIONS
{
	/* Lưu trữ bảng vector vào đầu bộ nhớ Flash */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))	/* Cú pháp KEEP(): Để vùng nhớ này không bị xóa đi khi bạn bật cờ -Wl,--gc-sections(Garbage Collection) */
    . = ALIGN(4);
  } >FLASH		/* VMA = LMA = FLASH */
	
	/* Lưu trữ đoạn code (firmware) vào bộ nhớ Flash */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text.*)		    /* Gom tất cả các section .text. vào một section .text */
    KEEP (*(.init))   /* Lưu giữ các hàm trong file init.s */
    KEEP (*(.fini))   /* Lưu giữ các hàm trong file fini.s */
    . = ALIGN(4);
  } >FLASH		/* VMA = LMA = FLASH */
  
	/* Lưu trữ các hằng số (constant) trong bộ nhớ Flash */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata.*)
    . = ALIGN(4);
  } >FLASH		/* VMA = LMA = FLASH */
  
	/* Định nghĩa các phân vùng khởi tạo */
  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* Địa chỉ bắt đầu của vùng nhớ .data trong bộ nhớ FLASH  */
  _sidata = LOADADDR(.data);
	
	/* Lưu trữ các biến toàn cục và các biến static được khởi tạo khác 0 */ 
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data.*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH	/* VMA = RAM, LMA = FLASH */
  
	/* Lưu trữ các biến toàn cục được khởi tạo = 0 hoặc không được khởi tạo */ 
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM			/* VMA = LMA = RAM */
  
	/* Vùng nhớ dành cho cấp phát bộ nhớ động (như hàm malloc()) */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end  = . );     /* Alias chuẩn — syscalls.c dùng tên này	*/
    PROVIDE ( _end = . );     /* Alias bổ sung cho một số newlib version  */
    PROVIDE ( __end__ = . );  /* Alias bổ sung cho rdimon semihosting */
    . = . + _Min_Heap_Size;	/* Tối thiểu 4kB cho Heap và 4kB cho Stack */
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM
}