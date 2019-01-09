/*
 *  Name:       lcd_font.c
 *  Date first: 12/29/2012
 *  Date last:  01/06/2019
 *
 *  Description: Parse font file from font program and create LCD pixel data.
 *
 *******************************************************************************
 *
 *  Copyright (C) 2012, 2019 Richard Hodges.  All rights reserved.
 *  Permission is hereby granted for any use.
 *
 *******************************************************************************
 *
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *  Find the top of memory for STM8S103 (8 KB) or STM8S105 (16 KB)
 *  The font table is 12 pages of 256 pixels (0x0c00 bytes)
 */

#ifdef STM8103
#define FONT_BASE "0x9400"
#endif
#ifdef STM8105
#define FONT_BASE "0xb400"
#endif

/*******************************************************************************
 *
 * Globals
 */

#define FONT_COUNT	256	/* number chars in font */
#define FONT_WIDTH	1	/* character width, in bytes */
#define FONT_HEIGHT	12	/* character height, in bytes */
#define FONT_EMPTY	-1

typedef struct {
    short data[FONT_COUNT][FONT_HEIGHT][FONT_WIDTH];
    int	  width;
    int	  height;
    int	  count;
} FONT;

static void	 font_init(FONT *);	/* initialize font object */

static int	 font_read (FONT *, FILE *);
static int	 font_write(FONT *, FILE *);

static int	 bit_swap(int);
static int	 bit_test(void);

/*******************************************************************************
 *
 * Read in font data, write out LCD pixel data.
 */


int main(int argc, char **argv)
{
    FONT	 font;
    FILE	*fin, *fout;
    int		 retval;

    fin  = stdin;
    fout = stdout;

    font_init(&font);
    retval = font_read(&font, fin);
    if (retval)
	exit (1);
#ifdef UNDEF
#endif
    retval = font_write(&font, fout);
    if (retval)
	exit (2);

    exit (0);
}

/*******************************************************************************
 *
 * Initialize font object
 *
 * in:  FONT *
 */

static void font_init(FONT *font)
{
    int		 i, j, k;

    font->width  = FONT_WIDTH;
    font->height = FONT_HEIGHT;
    font->count  = FONT_COUNT;

    for (i = 0; i < font->count; i++)
	for (j = 0; j < font->height; j++)
	    for (k = 0; k < font->width; k++)
		font->data[i][j][k] = FONT_EMPTY;
}

/*******************************************************************************
 *
 * Example of input line created by font program:
 * "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
 *
 *******************************************************************************
 *
 * Read in font file
 *
 *  in: FONT, FILE
 * out: zero = success
 */

static int font_read(FONT *font, FILE *fin)
{
    char	 line[200], *ptr, *q2, c;
    int		 i, j, k, val, nyb;

    i = 0;
    for (;;) {
	ptr = fgets(line, sizeof(line), fin);
	if (ptr == NULL)
	    break;
	ptr = strchr(ptr, '"');	/* find first quote mark */
	if (ptr == NULL)
	    continue;
	ptr++;
	q2 = strchr(ptr, '"');	/* find second quote mark */
	if (q2 == NULL)
	    continue;
	*q2 = 0;

	/* Presumably, the input line is acceptable here */
	j = 0;
	k = 0;
	nyb = 0;
	while (c = *ptr++) {
	    c = toupper(c);
	    if (c < '0' ||
		c > 'F')
		continue;
	    if (c > '9' &&
		c < 'A')
		continue;
	    c -= '0';
	    if (c > 9)
		c -= 7;
	    val <<= 4;
	    val  |= c;
	    nyb++;
	    if (nyb & 1)
		continue;
	    val &= 0xff;
	    font->data[i][j][k] = val;
	    k++;
	    if (k < font->width)
		continue;
	    k = 0;
	    j++;
	}
	i++;
    }
    return (0);
}

/*******************************************************************************
 *
 * Write out font file, one pixel row at a time
 *
 *  in: FONT, FILE
 * out: zero = success
 */

static int font_write(FONT *font, FILE *fout)
{
    int		 i, j, k, c;
    int		 pix, lsb, msb, even;

    fprintf(fout, "const unsigned char __at (" FONT_BASE ") font_tab[12*256] = {\n");

    for (i = 0; i < font->height; i++) {
	fprintf(fout, "/* pixel row %d */\n", i);
	for (j = 0; j < font->count; j++) {
	    pix = font->data[j][i][0];
	    if (pix == FONT_EMPTY)
		pix = 0;
	    fprintf(fout, "0x%02x, ", pix);
	    if ((j & 15) == 15)
		fprintf(fout, "\n");
	}
    }
    fprintf(fout, "};\n");

    return (0);
}

