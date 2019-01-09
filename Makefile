# Define either STM8103 (8 KB flash) or STM8105 (16 KB flash)

CC   = gcc -g -DSTM8103
#SDCC = sdcc -mstm8 -I../libs -L../libs -DSTM8103
SDCC = sdcc -mstm8 -DSTM8103

#LIBS = lib_stm8.lib
OBJS = lib_graphic.rel lib_bindec.rel lib_clock.rel

FONT_RAW = font_8x12.arm
FONT_INC = lib_graphic.font
FONT_FIX = lcd_font


all: test_graphic.ihx

lib_graphic.rel: lib_graphic.font 
test_graphic.ihx: test_graphic.c $(OBJS)

$(FONT_INC): $(FONT_RAW) $(FONT_FIX)
	./$(FONT_FIX) < $(FONT_RAW) > $(FONT_INC)

$(FONT_FIX): $(FONT_FIX).c
	$(CC) -o $(FONT_FIX) $(FONT_FIX).c


.SUFFIXES : .rel .c .ihx

.rel.ihx :
	$(SDCC) $< $(LIBS) $(OBJS)
#	cp test_graphic.ihx /media/share/stm8

.c.rel :
	$(SDCC) -c $<
clean:
	- rm -f *.adb *.asm *.cdb *.ihx *.lk *.lst *.map *.rel *.rst *.sym *.font

