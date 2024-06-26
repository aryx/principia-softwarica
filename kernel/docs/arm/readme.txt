-*- org -*-

* raspberry pi
(was in 'words' file in miller repo)

broadcom 2835 SoC (based on 2708)
arm1176jzf-s (v6 arch) 700MHz cpu, apparently dual-issue, with vfp2
videocore 4 gpu

l1 I & D VIPT caches
	16K each: 4-way, 128 sets, 32-byte lines
	l1 D is write-through, l1 I is write-back
unified l2 PIPT cache 128K: 4-way?, 1024? sets, 32-byte lines, mostly for gpu
(by default CPU doesn't see it)

we arrange that device register accesses are uncached and unbuffered
(strongly ordered, in armv6/v7 terminology).

256MB or 512MB of dram at physical address 0, shared with gpu
non-16550 uart for console
	uart serial voltages are TTL (3.3v, not rs232 which is nominally 12v);
	could use usb serial (ick).
there's no real ethernet controller, so we have to use usb ether,
and the usb controller is nastier than usual.

There's a serial port (115200b/s) on P1 connector pins (GND,TXD,RXD) =
(6,8,10).  These are 3v TTL signals: use a level-shifter to convert to
RS232, or a USB-to-TTL-serial adapter.  Add the line "console=0
b115200" to the /cfg/pxe file on the server, or the parameter
"console='0 b115200'" to cmdline.txt on the SD card.

9pi is a Plan 9 terminal, which can boot with local fossil root on the
sd card (/dev/sdM0), or with root from a Plan 9 file server via tcp.

9picpu is a Plan 9 cpu server, which could be used in a headless
configuration without screen, keyboard or mouse.

9pifat is a minimal configuration which boots a shell script boot.rc
with root in /plan9 on the dos partition, maybe useful for embedded
applications where a full Plan 9 system is not needed.

Network booting with u-boot:
start with a normal rpi u-boot sd (e.g. raspberry-pi-uboot-20120707).
update the start.elf with a version from a newer rpi distro (see below).
mk installall
add new system to ndb
see booting(8)

Booting from sd card:
- start with a normal rpi distro sd (e.g. 2012-08-16-wheezy-raspbian)
  [NB: versions of start.elf earlier than this may not be compatible]
- copy 9pi to sd's root directory
- add or change "kernel=" line in config.txt to "kernel=9pi"
- plan9.ini is built from the "kernel arguments" in cmdline.txt - each
  var=value entry becomes one plan9.ini line, so entries with spaces will
  need single quotes.


	physical mem map

hex addr size	what
----
0	 256MB	sdram, cached (newer models have 512MB)
00000000 64	exception vectors
00000100 7936	boot ATAGs (inc. cmdline.txt)
00002000 4K	Mach
00003000 1K	L2 page table for exception vectors
00003400 1K	videocore mailbox buffer
00003800 2K	FIQ stack
00004000 16K	L1 page table for kernel
00008000 -	default kernel load address
01000000 16K	u-boot env
20000000 16M	peripherals
20003000	system timer(s)
20007000	dma
2000B000	arm control: intr, timers 0 & 1, semas, doorbells, mboxes
20100000	power, reset, watchdog
20200000	gpio
20201000	uart0
20202000	mmc
20215040	uart1 (mini uart)
20300000	eMMC
20600000	smi
20980000	otg usb

40000000	l2 cache only
7e00b000	arm control
7e2000c0	jtag
7e201000?	pl011 usrt
7e215000	aux: uart1, spi[12]

80000000

c0000000	bypass caches

	virtual mem map (from cpu address map & mmu mappings)

hex addr size	what
----
0	 512MB	user process address space
7e000000 16M	i/o registers
80000000 <=224M	kzero, kernel ram (reserve some for GPU)
ffff0000 4K	exception vectors

Linux params at *R2 (default 0x100) are a sequence of ATAGs
  struct atag {
	u32int size;		/* size of ATAG in words, including header */
	u32int tag;		/* ATAG_CORE is first, ATAG_NONE is last */
	u32int data[size-2];
  };
00000000	ATAG_NONE
54410001	ATAG_CORE
54410002	ATAG_MEM
54410009	ATAG_CMDLINE

uart dmas	15, 14

intrs (96)
irq1
0	timer0
1	timer1
2	timer2
3	timer3
8	isp
9	usb
16	dma0
17	dma1
⋯
28	dma12
29	aux: uart1
30	arm
31	vpu dma

irq2
35	sdc
36	dsio
40	hdmi0
41	hdmi1
48	smi
56	sdio
57	uart1 aka "vc uart"

irq0
64	timer
65	mbox
66	doorbell0
67	doorbell1
75	usb
77	dma2
78	dma3
82	sdio
83	uart0


* raspberry pi 2 -- differences from raspberry pi
(was in 'words2' file in miller repo)

broadcom 2836 SoC (based on 2709)
4 x cortex-a7 (v7 arch) 900Mhz cpu, dual-issue, vfpv3+

integral l2 cache

1GB of dram at physical address 0

peripherals   at 0x3F000000
gpu/dma space at 0xC0000000
