/*
 *  File name:  lib_graphic.h
 *  Date first: 01/01/2019
 *  Date last:  01/06/2019
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
 *  This library uses Timer2.
 *
 *  OPTIONS:
 */

#define GRAPH_ISR_SCOPE	/* See ISR time on pin D4 */

/*
 *  Initialize graphics
 *  in: text framebuffer (20x40)
 */

void graph_init(char *);

/*
 *  Set cursor
 *  in: line (0-19), column (0-39)
 */

void graph_curs(char, char);

/*
 *  Send char to display
 *  in: char
 */

void graph_putc(char);

/*
 *  Send string to display
 *  in: string (note: use putc to send zero char)
 */

void graph_puts(char *);

/*
 *  Plot a dot using the dot characters
 *  in: X (0-79), Y (0-59) from lower left corner, new val
 */

void graph_plot(char X, char Y, char);

/*
 *  Draw the outline of a box using graphic chars
 *  in: upper left corner, width, height
 */

void graph_box(char line, char col, char w, char h);

/*  The font has characters for drawing boxes  */

#define GRAPH_DRAW_UL	0	/* upper left graphic corner */
#define GRAPH_DRAW_HOR	1	/* horizontal graphic */
#define GRAPH_DRAW_UR	2	/* upper right graphic corner */
#define GRAPH_DRAW_VERT	3	/* vertical graphic */
#define GRAPH_DRAW_LL	4	/* lower left graphic corner */
#define GRAPH_DRAW_LR	5	/* lower right graphic corner */


void graph_isr_clk(void) __interrupt (IRQ_TIM2);
