/*
 *  File name:  test_graphic.c
 *  Date first: 01/03/2019
 *  Date last:  01/08/2019
 *
 *  Description: Test library for graphic LCD (320x240)
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

#include "lib_bindec.h"
#include "lib_clock.h"
#include "lib_graphic.h"

void setup(void);

volatile char	clock_tenths;	/* 1/10 second counter 0-255 */
volatile char   clock_msecs;	/* millisecond counter */

/* callbacks provided by lib_clock */

void clock_ms(void);	/* millisecond callback */
void clock_10(void);	/* 1/10 second callback */

void screen_black(void);
void screen_setup(void);

static char framebuf[20*40];

/******************************************************************************
 *
 *  Test the graphic LCD
 */

int main() {
    char	display[10];
    char	last_tenth, next_tenth;
    short	count16;
    char	plot, diag;

    setup();
    clock_init(clock_ms, clock_10);
    graph_init(framebuf);

    last_tenth = 0;
    next_tenth = 10;
    count16 = 0;
    plot = 0;
    diag = 0;

    screen_setup();

    do {
	while (last_tenth == clock_tenths);	/* wait for 1/10 second */
	last_tenth = clock_tenths;

	/* do 100ms work */

	PB_ODR ^= 0x20;
	graph_plot(18 + (plot & 31), 22, plot & 1);
	graph_plot(18 + (plot & 31), 21, plot & 32);
	graph_plot(18 + (plot & 31), 20, plot & 2);
	plot++;
#ifdef UNDEF
	graph_plot(diag, diag, 1);
#endif
	diag++;
	if (diag == 60)
	    diag = 0;

	if (clock_tenths != next_tenth)
	    continue;		/* update display every second */
	next_tenth += 10;

	count16++;

	graph_curs(10, 0);
	graph_puts("Count=");
	graph_puts(bin16_dec_rlz(count16, display));

	graph_curs(10, 20);
	clock_string(display);
	graph_puts(display);


    } while(1);
}

/******************************************************************************
 *
 *  Millisecond callback
 */

void clock_ms(void)
{
    clock_msecs++;
}

/******************************************************************************
 *
 *  1/10 second callback
 */

void clock_10(void)
{
    clock_tenths++;

    if (clock_tenths & 3)
	PB_ODR |= 0x20;		/* LED off 75% */
    else
	PB_ODR &= 0xdf;		/* LED on 25% */
}

/******************************************************************************
 *
 *  Set up static screen
 */

void screen_setup(void)
{
    int		i;

    graph_curs(0, 0);
    for (i = 0; i < 800; i++)
	graph_putc(' ');

    graph_curs(0, 0);
    for (i = 0; i < 128; i++)
	graph_putc(i);

    graph_curs(5, 0);
    graph_puts("Although golf was originally restricted to wealthy, "\
	       "overweight Protestants, today it's open to anybody who "\
	       "owns hideous clothing.  -- Dave Barry");

    graph_curs(18, 0);
    graph_puts("Line 19 --->");

    graph_curs(19, 0);
    for (i = 0; i < 40; i++)
	graph_putc(i + '@');

    graph_box(11, 8, 30, 4);
    graph_box(15, 0, 40, 2);
    graph_box(12, 1, 2, 2);
    graph_box(12, 3, 4, 2);
}

/******************************************************************************
 *
 *  Black out screen
 */

void screen_black(void)
{
    int		i;

    graph_curs(0, 0);

    for (i = 0; i < 800; i++)
	graph_putc(191);	/* solid black cell */
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void setup(void)
{
    CLK_CKDIVR = 0x00;	/* clock 16mhz */

    clock_tenths = 0;

    PA_ODR = 0;
    PA_DDR = 0x00;
    PA_CR1 = 0xff;
    PA_CR2 = 0x00;

    PB_DDR = 0x20;	/* output LED */
    PB_CR1 = 0xff;     	/* inputs have pullup, outputs not open drain */
    PB_CR2 = 0x00;	/* no interrupts, 2mhz output */

    PC_CR1 = 0xff;	/* LCD uses C4-C7, C3 must be input */
    PD_CR1 = 0xff;	/* LCD uses D1-D3, D0 set to one */

  __asm__ ("rim");
}
/* Available ports on STM8S103P3:
 *
 * A1..A3	A3 is HS
 * B4..B5	Open drain
 * C3..C7	HS
 * D1..D6	HS
 */

