/*
 *  File name:  lib_graphic.c
 *  Date first: 01/01/2019
 *  Date last:  01/08/2019
 *
 *  Description: STM8 Library for graphic LCD (320x240)
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2019 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 */

#include "stm8s_header.h"

#include "lib_graphic.h"
#include "lib_graphic.font"

#define DATA	_PC_ODR		/* C4-C7 are data, make C3 input */
#define FLM	_PD_ODR,#3	/* first line marker, high on first line CL1 */
#define CL1	_PD_ODR,#2	/* line load */
#define CL2	_PD_ODR,#1	/* 4-bit pixel load */

static char *framebuf;

/*  These variables are for refresh  */

static char *font_ptr;		/* current pixel row */
static char *char_ptr;
static char  row_ctr;		/* pixel row, 12 rows */
static char  row_cur;		/* char row, 20 rows */
static char  col_ct;

static void do_row(void);

/*  These variables are for terminal emulation  */

static char *fptr;		/* next character position */

/******************************************************************************
 *
 *  Initialize graphics
 *  in: text buffer (20x40 chars)
 */

void graph_init(char *buf)
{
    int		i;

#ifdef GRAPH_ISR_SCOPE
    PD_DDR |= 0x1f;	/* using D1, D2, D3. D0 set to 1, D4 for iSR */
#else
    PD_DDR |= 0x0f;
#endif
    PD_ODR |= 0x01;	/* used with "INC PD_ODR" on CL2 */
    PD_CR2 |= 0x0e;	/* fast pins */

    PC_DDR |= 0xf0;	/* using C4-C7 */
    PC_CR2 |= 0xf0;

    font_ptr = font_tab;
    framebuf = buf;
    char_ptr = buf;
    row_ctr  = 12;
    row_cur  = 20;

    fptr = framebuf;

    for (i = 0; i < 800; i++)
	*buf++ = ' ';		/* clear the screen */

    /* set up timer2 for 1mhz clock and 60uSec interrupt */

    TIM2_PSCR = 3;	/* prescale = 8 for 16mhz */
    TIM2_ARRH = 0;
//  TIM2_ARRL = 119;	/* 69hz refresh, free: 10.7uS every 60uS (17.8%) */
    TIM2_ARRL = 137;	/* 60hz refresh, free: 20.5uS every 69uS (29.7%) */

    TIM2_IER |= 1;	/* interrupt on clock reload */
    TIM2_CR1 = 0x01;	/* use TIM2_ARR preload register, enable */
}

/*
 *  Note that the "free" time numbers are the best observed. With a 60hz
 *  refresh, the time could be as as low as 17uS. Moving the main refresh
 *  loop address did not show any change, but the characters on the LCD
 *  did affect the time to service the pixels. This implies that the table
 *  lookups from flash may be causing extra cache refills. But there is no
 *  change in timing even when the display is all the same character.
 *
 ******************************************************************************
 *
 *  Decode and send the row of pixels to display
 */

static void do_row(void)
{
__asm
    ldw		x, _font_ptr	; add 256 every line
    ldw		y, _char_ptr	; add 40 every 12 rows

    mov		_col_ct, #40
00002$:
    ld		a, (y)
    ld		xl, a
    ld		a, (x)

    ld		DATA, a
    swap	a
    bset	CL2
    bres	CL2

    ld		DATA, a
    incw	y
    bset	CL2
    bres	CL2

    dec		_col_ct
    jrne	00002$

    bset	CL1
    bres	CL1
    inc		_font_ptr		; add 256 for next pixel row
    bres	FLM

    dec		_row_ctr
    jrne	00099$
    mov		_row_ctr, #12

    ldw		x, #_font_tab
    ldw		_font_ptr, x

    ldw		x, _char_ptr
    addw	x, #40
    ldw		_char_ptr, x

    dec		_row_cur
    jrne	00099$
    mov		_row_cur, #20
    bset	FLM		; next row will be top

    ldw		x, _framebuf
    ldw		_char_ptr, x
00099$:
__endasm;
}

/******************************************************************************
 *
 *  Timer 2 interrupt, every 60 microseconds
 */

void graph_isr_clk(void) __interrupt (IRQ_TIM2)
{
__asm
    bres        _TIM2_SR1, #0   /* clear interrupt */
#ifdef GRAPH_ISR_SCOPE
    bset	_PD_ODR, #4
    call	_do_row
    bres	_PD_ODR, #4
#else
    call	_do_row
#endif
__endasm;
}

/******************************************************************************
 *
 *  Put char in display
 *  in: char
 */

void graph_putc(char c)
{
    c;
__asm
    ldw		x, _fptr
    ld		a, (3, sp)
    ld		(x), a
    incw	x
    ldw		_fptr, x
__endasm;
}

/******************************************************************************
 *
 *  Put string in display
 *  in: string
 */

void graph_puts(char *s)
{
    s;
__asm
    ldw		x, _fptr
    ldw		y, (3, sp)
00001$:
    ld		a, (y)
    jreq	000099$
    ld		(x), a
    incw	x
    incw	y
    jra		00001$
00099$:
    ldw		_fptr, x
__endasm;
}

/******************************************************************************
 *
 *  Set cursor
 *  in: line (0-19), column (0-39)
 */

void graph_curs(char line, char col)
{
    fptr = framebuf + line * 40 + col;
}

/******************************************************************************
 *
 *  Plot a dot using the dot characters
 *  in: X (0-79), Y (0-59) from lower left corner, new value
 */

void graph_plot(char x, char y, char val)
{
#ifdef ORIGINAL_C
    char	*ptr;
    char	 pix, bit;

    y = 59 - y;

    ptr  = framebuf;
    ptr += 40 * (y / 3);
    ptr += x / 2;

    pix = *ptr;
    if ((pix & 0xc0) != 0x80)	/* check for non-dot character */
	pix = 0x80;		/* convert to dot range */
    bit = 1;
    bit <<= (y % 3);
    if (x & 1)
	bit <<= 3;
    if (val)
	pix |= bit;
    else
	pix &= (~bit);
    *ptr = pix;
#else
    x, y, val;			/* located at sp+3, sp+4 sp+5 */
__asm
    ld		a, (3, sp)	; X coordinate
    push	a
    push	#0		; 16 bit version of X
    push	#1		; bit to set/clear at

    ld		a, #59
    sub		a, (7, sp)	; y = 59 - y
    jrc		00099$		; (optional)

    clrw	x
    ld		xl, a
    ld		a, #3
    div		x, a		; Xreg = y / 3, Areg = y % 3
    tnz		a
    jreq	00002$

00001$:
    sll		(1, sp)		; bit <<= y % 3
    dec		a
    jrne	00001$

00002$:
    srl		(3, sp)		; x / 2
    jrnc	00003$
    sll		(1, sp)
    sll		(1, sp)
    sll		(1, sp)

00003$:
    ld		a, #40
    mul		x, a
    addw	x, (2, sp)
    addw	x, _framebuf

    ld		a, (x)		; now, check for non-dot character
    and		a, #0xc0
    cp		a, #0x80	; This can be simplified with "tnz" if there
    jrne	00010$		; won't be collisions with chars in the
    ld		a, (x)		; 0xc0 to 0xff range.

00005$:
    tnz		(8, sp)
    jreq	00006$		; clear pixel
    or		a, (1, sp)	; set pixel
    jra		00090$

00010$:
    ld		a, #0x80	; collision with non-dot character
    jra		00005$		; so convert it

00006$:
    cpl		(1, sp)
    and		a, (1, sp)	; clear pixel
00090$:
    ld		(x), a
00099$:
    add		sp, #3
__endasm;
#endif
}

/******************************************************************************
 *
 *  Draw the outline of a box using graphic chars
 *  in: upper left corner, width, height
 */

void graph_box(char line, char col, char w, char h)
{
    char	*ptr;
    char	i;

    ptr = framebuf + line * 40 + col;

    *ptr++ = GRAPH_DRAW_UL;

    for (i = 2; i < w; i++)
	*ptr++ = GRAPH_DRAW_HOR;
    *ptr = GRAPH_DRAW_UR;
    ptr += 40;
    for (i = 2; i < h; i++) {
	*ptr = GRAPH_DRAW_VERT;
	ptr += 40;
    }
    *ptr-- = GRAPH_DRAW_LR;
    for (i = 2; i < w; i++)
	*ptr-- = GRAPH_DRAW_HOR;
    *ptr = GRAPH_DRAW_LL;
    ptr -= 40;
    for (i = 2; i < h; i++) {
	*ptr = GRAPH_DRAW_VERT;
	ptr -= 40;
    }
}

/******************************************************************************
 *
 *  Pin configuration
 *
 * C4-C7	D1-D4
 * C3		(input)
 * 

14 pin connector pinout:
Last update: Dec 6, 2012

1       vo      internet (don't connect)
2       vee     internet -19.7 to -22.7
3       DI4     visual
4       DI3     visual
5       DI2     visual
6       DI1     visual
7       GND     visual
8       Vcc +5  internet
9       CP      visual  (CL2 to 79401 internet)
10      CL1 to 79430 (internet)
11      FLM     internet
12      LED K   internet
13      LED A   internet
14      N/C     internet

*/
