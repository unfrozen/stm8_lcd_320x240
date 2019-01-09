/*
 *  File name:  lib_bindec.h
 *  Date first: 12/24/2017
 *  Date last:  12/18/2018
 *
 *  Description: Library of binary/decimal functions for STM8
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2017, 2018 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 *  Convert 16 bit unsigned binary to 5 digit decimal, terminate with zero
 */

void bin16_dec(short, char *);

/*
 *  Convert 32 bit unsigned binary to 10 digit decimal, terminate with zero
 */

void bin32_dec(long, char *);

/*
 *  Convert 8 bit unsigned binary to 2 digit decimal, terminate with zero
 */

void bin8_dec2(char, char *);

/*
 *  Trim leading zeroes from decimal buffer
 *  in:  buffer, maximum zeroes to remove
 *  out: first non-zero digit
 */

char *decimal_rlz(char *, char);

/*
 *  Combine 16 bit binary to decimal and remove leading zeroes
 *  in:  16 bit binary, 6 digit buffer
 *  out: first non-zero digit
 */

char *bin16_dec_rlz(short, char *);

/*
 *  Convert binary byte to hex, terminate with zero
 */

void bin8_hex(char, char *);

/*
 *  Convert ASCII decimal digits to 16-bit binary
 *  Stops on first non-decimal character
 */

int dec_bin16(char *);

/*
 *  Convert ASCII decimal digits to 16-bit binary
 *  Digits may be preceded with sign
 *  Stops on first non-decimal character
 */
int dec_bin16s(char *);
