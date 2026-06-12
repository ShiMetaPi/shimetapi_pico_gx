
.global str_delay_us

str_delay_us:
loop1:
	b delay_1us
	sub x0, x0, #1
	cmp	w0, #0x0
	b.ne	loop1  // b.any
	ret

delay_1us:
	mov x3, #0x1000000
loop2:
	nop
	sub x3, x3, #1
	cmp x3, #0x0
	b.ne loop2
	ret

#ifdef CRC_CHECK
.global crc16_asm
crc16_asm:
	.word		0xd2800004
	.word		0x529fffe3
	.word		0x6b04003f
	.word		0x54000068
	.word		0x2a2303e0
	.word		0xd65f03c0
	.word		0x38646802
	.word		0x91000484
	.word		0x4a020062
	.word		0x12001c45
	.word		0x531c0c42
	.word		0x4a050042
	.word		0x531d7045
	.word		0x4a0220a5
	.word		0x4a4210a2
	.word		0x4a432043
	.word		0x17fffff2
#endif

