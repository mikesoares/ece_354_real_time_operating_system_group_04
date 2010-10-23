# Makefile
# David Grant, 2004
# Irene Huang, 2010/05/03

CC=m68k-elf-gcc
CXX=m68k-elf-g++
CFLAGS= -Wall -m5307 -pipe -nostdlib
LD=m68k-elf-gcc
AS=m68k-elf-as 
AR=m68k-elf-ar
ARFLAGS=
OBJCPY=m68k-elf-objcopy
OBJDUMP=m68k-elf-objdump
ASM_RTX= core/core.s io/uart_handler.s mm/mem.s messaging/messaging.s timer/timer_handler.s
ASM=core/start.s 
LDFLAGS_RTX = -Tshared/rtx.ld -Wl,-Map=rtx.map
LDFLAGS_RTX_LOADER = -Tcore/rtx_loader.ld -Wl,-Map=rtx_loader.map
LDFLAGS_TEST = -Ttest/rtx_test.ld -Wl,-Map=rtx_test.map

RTX_OBJS=dbug.o util.o core.o initialize.o mm.o messaging.o io_kcd.o io_crt.o io_uart.o timer.o wallclock.o stress.o priority_switch.o

# This set of RTX objects includes the tests we came up with throughout the progression of the project. Tests that we feel are relevant have been included in rtx_test.s19
#RTX_OBJS=dbug.o util.o core.o initialize.o core_test.o mm.o mem_test.o messaging.o io_kcd.o io_crt.o io_uart.o timer.o msg_test.o msg_test3.o wallclock.o stress.o priority_switch.o

RTX_LOADER_OBJS=dbug.o rtx_loader.o
RTX_TEST_OBJS=dbug.o rtx_test.o

all: rtx.s19

test: core.s19 rtx_loader.s19 rtx_test_ext.s19
	python merge.py tmp.s19 core.s19 rtx_test_ext.s19
	python merge.py rtx.s19 tmp.s19 rtx_loader.s19
	rm tmp.s19

core.s19: $(RTX_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX) -o rtx.bin $(ASM) $(ASM_RTX) $(RTX_OBJS) 
	$(OBJCPY) --output-format=srec rtx.bin core.s19
	$(OBJDUMP) -d rtx.bin > rtx.lst

rtx_loader.s19: $(RTX_LOADER_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_LOADER) -o rtx_loader.bin $(ASM) $(RTX_LOADER_OBJS)
	$(OBJCPY) --output-format=srec rtx_loader.bin rtx_loader.s19
	$(OBJDUMP) -d rtx_loader.bin > rtx_loader.lst

rtx_test.s19: $(RTX_TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS_TEST) -o rtx_test.bin $(ASM) $(RTX_TEST_OBJS)
	$(OBJCPY) --output-format=srec rtx_test.bin rtx_test.s19
	$(OBJDUMP) -d rtx_test.bin > rtx_test.lst

rtx.s19: core.s19 rtx_loader.s19 rtx_test.s19
	python merge.py tmp.s19 core.s19 rtx_test.s19
	python merge.py rtx.s19 tmp.s19 rtx_loader.s19
	rm tmp.s19
	chmod 770 rtx.s19

rtx_loader.o: core/rtx_loader.c
	$(CC) $(CFLAGS) -c core/rtx_loader.c

#rtx_test.o: test/rtx_dummy.c test/rtx_test.h
#	$(CC) $(CFLAGS) -c test/rtx_dummy.c -o rtx_test.o

rtx_test.o: test/rtx_test.c test/rtx_test.h
	$(CC) $(CFLAGS) -c test/rtx_test.c -o rtx_test.o

priority_switch.o: core/priority_switch.c
	$(CC) $(CFLAGS) -c core/priority_switch.c

util.o: shared/rtx_util.c
	$(CC) $(CFLAGS) -c shared/rtx_util.c -o util.o

stress.o: stress/stress.c shared/dbug.h
	$(CC) $(CFLAGS) -c stress/stress.c

wallclock.o: wallclock/wallclock.c shared/dbug.h
	$(CC) $(CFLAGS) -c wallclock/wallclock.c

msg_test.o: messaging/msg_test.c shared/dbug.h
	$(CC) $(CFLAGS) -c messaging/msg_test.c
	
msg_test3.o: messaging/msg_test3.c shared/dbug.h
	$(CC) $(CFLAGS) -c messaging/msg_test3.c
	
mem_test.o: mm/mem_test.c shared/dbug.h
	$(CC) $(CFLAGS) -c mm/mem_test.c

timer.o: timer/timer.c shared/dbug.h
	$(CC) $(CFLAGS) -c timer/timer.c

messaging.o: messaging/messaging.c shared/dbug.h
	$(CC) $(CFLAGS) -c messaging/messaging.c

mm.o: mm/mem.c shared/dbug.h
	$(CC) $(CFLAGS) -c mm/mem.c -o mm.o

io_kcd.o: io/kcd.c shared/dbug.h
	$(CC) $(CFLAGS) -c io/kcd.c -o io_kcd.o

io_crt.o: io/crt.c shared/dbug.h
	$(CC) $(CFLAGS) -c io/crt.c -o io_crt.o

io_uart.o: io/uart.c shared/dbug.h
	$(CC) $(CFLAGS) -c io/uart.c -o io_uart.o

core_test.o: core/core_test.c shared/dbug.h
	$(CC) $(CFLAGS) -c core/core_test.c

core.o initialize.o: core/core.c core/core.h core/initialize.c core/initialize.h shared/dbug.h
	$(CC) $(CFLAGS) -c core/core.c core/initialize.c

dbug.o: shared/dbug.c shared/dbug.h
	$(CC) $(CFLAGS) -c shared/dbug.c

clean:
	rm -f *.o *.bin *.map *.lst rtx.s19 rtx_test.s19 rtx_loader.s19 core.s19

# Because Adam can't remember that it's make clean
clear: clean
