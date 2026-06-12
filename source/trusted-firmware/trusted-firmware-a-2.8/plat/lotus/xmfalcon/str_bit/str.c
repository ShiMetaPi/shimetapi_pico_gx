/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

//#define CRC_CHECK
extern void str_delay_us(int us);
#ifdef CRC_CHECK
extern unsigned short crc16_asm(unsigned char *data, unsigned int length);
#endif

void lotus_pm_ddr_saving(void)
{
	unsigned int ddr_type;

	ddr_type = *(volatile unsigned int *)(0x120d0000);
	ddr_type &= 0x7;

	*(volatile unsigned int *)(0x12040000) = 'e';

#ifdef CRC_CHECK
	*(volatile unsigned int *)(0x12120350) = crc16_asm((unsigned char *)0x40000000,0x10000000);
#endif

	__asm__ volatile("dsb sy");
	__asm__ volatile("isb");
	while(*(volatile unsigned int *)(0x120d2900) != 0x1);

	// enter ddr selfflush channel0
	*(volatile unsigned int *)(0x120d315c) = 0x7200a0;
	*(volatile unsigned int *)(0x120d313c) = 0x1000005;
	*(volatile unsigned int *)(0x120d3138) = 0x1;
	*(volatile unsigned int *)(0x120d3138) = 0x0;

	if (ddr_type == 3) { //LPDDR4
		// enter ddr selfflush channel1
		*(volatile unsigned int *)(0x120d415c) = 0x7200a0;
		*(volatile unsigned int *)(0x120d413c) = 0x1000005;
		*(volatile unsigned int *)(0x120d4138) = 0x1;
		*(volatile unsigned int *)(0x120d4138) = 0x0;
	}

	str_delay_us(1);
	//read ddr training result to lpds
	*(volatile unsigned int *)(0x12120080) = *(volatile unsigned int *)(0x12103104);
	*(volatile unsigned int *)(0x12120084) = *(volatile unsigned int *)(0x12103120);
	*(volatile unsigned int *)(0x12120088) = *(volatile unsigned int *)(0x12103124);
	*(volatile unsigned int *)(0x1212008c) = *(volatile unsigned int *)(0x12103128);
	*(volatile unsigned int *)(0x12120090) = *(volatile unsigned int *)(0x1210312c);
	*(volatile unsigned int *)(0x12120094) = *(volatile unsigned int *)(0x12103130);
	*(volatile unsigned int *)(0x12120098) = *(volatile unsigned int *)(0x12103134);
	*(volatile unsigned int *)(0x1212009c) = *(volatile unsigned int *)(0x12103138);
	*(volatile unsigned int *)(0x121200a0) = *(volatile unsigned int *)(0x1210313c);
	*(volatile unsigned int *)(0x121200a4) = *(volatile unsigned int *)(0x12103110);
	*(volatile unsigned int *)(0x121200a8) = *(volatile unsigned int *)(0x12103100);
	*(volatile unsigned int *)(0x121200ac) = *(volatile unsigned int *)(0x12104108);
	*(volatile unsigned int *)(0x121200b0) = *(volatile unsigned int *)(0x12104130);
	*(volatile unsigned int *)(0x121200b4) = *(volatile unsigned int *)(0x12104134);
	*(volatile unsigned int *)(0x121200b8) = *(volatile unsigned int *)(0x12104138);
	*(volatile unsigned int *)(0x121200bc) = *(volatile unsigned int *)(0x1210413c);
	*(volatile unsigned int *)(0x121200c0) = *(volatile unsigned int *)(0x12104140);
	*(volatile unsigned int *)(0x121200c4) = *(volatile unsigned int *)(0x12105108);
	*(volatile unsigned int *)(0x121200c8) = *(volatile unsigned int *)(0x12105130);
	*(volatile unsigned int *)(0x121200cc) = *(volatile unsigned int *)(0x12105134);
	*(volatile unsigned int *)(0x121200d0) = *(volatile unsigned int *)(0x12105138);
	*(volatile unsigned int *)(0x121200d4) = *(volatile unsigned int *)(0x1210513c);
	*(volatile unsigned int *)(0x121200d8) = *(volatile unsigned int *)(0x12105140);
	*(volatile unsigned int *)(0x121200dc) = *(volatile unsigned int *)(0x12106108);
	*(volatile unsigned int *)(0x121200e0) = *(volatile unsigned int *)(0x12106130);
	*(volatile unsigned int *)(0x121200e4) = *(volatile unsigned int *)(0x12106134);
	*(volatile unsigned int *)(0x121200e8) = *(volatile unsigned int *)(0x12106138);
	*(volatile unsigned int *)(0x121200ec) = *(volatile unsigned int *)(0x1210613c);
	*(volatile unsigned int *)(0x121200f0) = *(volatile unsigned int *)(0x12106140);
	*(volatile unsigned int *)(0x121200f4) = *(volatile unsigned int *)(0x12107108);
	*(volatile unsigned int *)(0x121200f8) = *(volatile unsigned int *)(0x12107130);
	*(volatile unsigned int *)(0x121200fc) = *(volatile unsigned int *)(0x12107134);
	*(volatile unsigned int *)(0x12120100) = *(volatile unsigned int *)(0x12107138);
	*(volatile unsigned int *)(0x12120104) = *(volatile unsigned int *)(0x1210713c);
	*(volatile unsigned int *)(0x12120108) = *(volatile unsigned int *)(0x12107140);
	*(volatile unsigned int *)(0x1212010c) = *(volatile unsigned int *)(0x12104100);
	*(volatile unsigned int *)(0x12120110) = *(volatile unsigned int *)(0x12104110);
	*(volatile unsigned int *)(0x12120114) = *(volatile unsigned int *)(0x12104114);
	*(volatile unsigned int *)(0x12120118) = *(volatile unsigned int *)(0x12104118);
	*(volatile unsigned int *)(0x1212011c) = *(volatile unsigned int *)(0x1210411c);
	*(volatile unsigned int *)(0x12120120) = *(volatile unsigned int *)(0x12104120);
	*(volatile unsigned int *)(0x12120124) = *(volatile unsigned int *)(0x12105100);
	*(volatile unsigned int *)(0x12120128) = *(volatile unsigned int *)(0x12105110);
	*(volatile unsigned int *)(0x1212012c) = *(volatile unsigned int *)(0x12105114);
	*(volatile unsigned int *)(0x12120130) = *(volatile unsigned int *)(0x12105118);
	*(volatile unsigned int *)(0x12120134) = *(volatile unsigned int *)(0x1210511c);
	*(volatile unsigned int *)(0x12120138) = *(volatile unsigned int *)(0x12105120);
	*(volatile unsigned int *)(0x1212013c) = *(volatile unsigned int *)(0x12106100);
	*(volatile unsigned int *)(0x12120140) = *(volatile unsigned int *)(0x12106110);
	*(volatile unsigned int *)(0x12120144) = *(volatile unsigned int *)(0x12106114);
	*(volatile unsigned int *)(0x12120148) = *(volatile unsigned int *)(0x12106118);
	*(volatile unsigned int *)(0x1212014c) = *(volatile unsigned int *)(0x1210611c);
	*(volatile unsigned int *)(0x12120150) = *(volatile unsigned int *)(0x12106120);
	*(volatile unsigned int *)(0x12120154) = *(volatile unsigned int *)(0x12107100);
	*(volatile unsigned int *)(0x12120158) = *(volatile unsigned int *)(0x12107110);
	*(volatile unsigned int *)(0x1212015c) = *(volatile unsigned int *)(0x12107114);
	*(volatile unsigned int *)(0x12120160) = *(volatile unsigned int *)(0x12107118);
	*(volatile unsigned int *)(0x12120164) = *(volatile unsigned int *)(0x1210711c);
	*(volatile unsigned int *)(0x12120168) = *(volatile unsigned int *)(0x12107120);
	*(volatile unsigned int *)(0x1212016c) = *(volatile unsigned int *)(0x12101204);
	*(volatile unsigned int *)(0x12120170) = *(volatile unsigned int *)(0x1210410c);
	*(volatile unsigned int *)(0x12120174) = *(volatile unsigned int *)(0x1210510c);
	*(volatile unsigned int *)(0x12120178) = *(volatile unsigned int *)(0x1210610c);
	*(volatile unsigned int *)(0x1212017c) = *(volatile unsigned int *)(0x1210710c);

	*(volatile unsigned int *)(0x12120180) = *(volatile unsigned int *)(0x12103200);
	*(volatile unsigned int *)(0x12120184) = *(volatile unsigned int *)(0x12104160);
	*(volatile unsigned int *)(0x12120188) = *(volatile unsigned int *)(0x12105160);
	*(volatile unsigned int *)(0x1212018c) = *(volatile unsigned int *)(0x12106160);
	*(volatile unsigned int *)(0x12120190) = *(volatile unsigned int *)(0x12107160);

	//ZQCAL
	*(volatile unsigned int *)(0x12120194) = *(volatile unsigned int *)(0x12100010);

	//PHYCTRL
	*(volatile unsigned int *)(0x12120198) = *(volatile unsigned int *)(0x12101004);
	*(volatile unsigned int *)(0x1212019C) = *(volatile unsigned int *)(0x1210101c);
	*(volatile unsigned int *)(0x121201a0) = *(volatile unsigned int *)(0x12101200);

	//DFICLK_CACTRL
	*(volatile unsigned int *)(0x121201a4) = *(volatile unsigned int *)(0x12103108);
	*(volatile unsigned int *)(0x121201a8) = *(volatile unsigned int *)(0x12103114);
	*(volatile unsigned int *)(0x121201ac) = *(volatile unsigned int *)(0x12103118);
	*(volatile unsigned int *)(0x121201b0) = *(volatile unsigned int *)(0x12103140);
	*(volatile unsigned int *)(0x121201b4) = *(volatile unsigned int *)(0x12103144);
	*(volatile unsigned int *)(0x121201b8) = *(volatile unsigned int *)(0x12103148);
	*(volatile unsigned int *)(0x121201bc) = *(volatile unsigned int *)(0x12103200);
	*(volatile unsigned int *)(0x121201c0) = *(volatile unsigned int *)(0x12103204);
	*(volatile unsigned int *)(0x121201c4) = *(volatile unsigned int *)(0x12103208);
	*(volatile unsigned int *)(0x121201c8) = *(volatile unsigned int *)(0x1210320C);
	*(volatile unsigned int *)(0x121201cc) = *(volatile unsigned int *)(0x12103298);
	*(volatile unsigned int *)(0x121201d0) = *(volatile unsigned int *)(0x1210329C);
	*(volatile unsigned int *)(0x121201d4) = *(volatile unsigned int *)(0x121032A0);
	*(volatile unsigned int *)(0x121201d8) = *(volatile unsigned int *)(0x121032A4);
	*(volatile unsigned int *)(0x121201dc) = *(volatile unsigned int *)(0x121032A8);
	*(volatile unsigned int *)(0x121201e0) = *(volatile unsigned int *)(0x121032AC);

	//DFICLK_DQCTRL
	*(volatile unsigned int *)(0x121201e4) = *(volatile unsigned int *)(0x12104000);
	*(volatile unsigned int *)(0x121201e8) = *(volatile unsigned int *)(0x12104004);
	*(volatile unsigned int *)(0x121201ec) = *(volatile unsigned int *)(0x12104124);
	*(volatile unsigned int *)(0x121201f0) = *(volatile unsigned int *)(0x12104128);
	*(volatile unsigned int *)(0x121201f4) = *(volatile unsigned int *)(0x1210414c);
	*(volatile unsigned int *)(0x121201f8) = *(volatile unsigned int *)(0x12104150);
	*(volatile unsigned int *)(0x121201fc) = *(volatile unsigned int *)(0x12104158);
	*(volatile unsigned int *)(0x12120200) = *(volatile unsigned int *)(0x1210415c);
	*(volatile unsigned int *)(0x12120204) = *(volatile unsigned int *)(0x12104160);
	*(volatile unsigned int *)(0x12120208) = *(volatile unsigned int *)(0x12104164);
	*(volatile unsigned int *)(0x1212020c) = *(volatile unsigned int *)(0x12104168);
	*(volatile unsigned int *)(0x12120210) = *(volatile unsigned int *)(0x1210416c);

	*(volatile unsigned int *)(0x12120214) = *(volatile unsigned int *)(0x12105000);
	*(volatile unsigned int *)(0x12120218) = *(volatile unsigned int *)(0x12105004);
	*(volatile unsigned int *)(0x1212021c) = *(volatile unsigned int *)(0x12105124);
	*(volatile unsigned int *)(0x12120220) = *(volatile unsigned int *)(0x12105128);
	*(volatile unsigned int *)(0x12120224) = *(volatile unsigned int *)(0x1210514c);
	*(volatile unsigned int *)(0x12120228) = *(volatile unsigned int *)(0x12105150);
	*(volatile unsigned int *)(0x1212022c) = *(volatile unsigned int *)(0x12105158);
	*(volatile unsigned int *)(0x12120230) = *(volatile unsigned int *)(0x1210515c);
	*(volatile unsigned int *)(0x12120234) = *(volatile unsigned int *)(0x12105160);
	*(volatile unsigned int *)(0x12120238) = *(volatile unsigned int *)(0x12105164);
	*(volatile unsigned int *)(0x1212023c) = *(volatile unsigned int *)(0x12105168);
	*(volatile unsigned int *)(0x12120240) = *(volatile unsigned int *)(0x1210516c);

	*(volatile unsigned int *)(0x12120244) = *(volatile unsigned int *)(0x12106000);
	*(volatile unsigned int *)(0x12120248) = *(volatile unsigned int *)(0x12106004);
	*(volatile unsigned int *)(0x1212024c) = *(volatile unsigned int *)(0x12106124);
	*(volatile unsigned int *)(0x12120250) = *(volatile unsigned int *)(0x12106128);
	*(volatile unsigned int *)(0x12120254) = *(volatile unsigned int *)(0x1210614c);
	*(volatile unsigned int *)(0x12120258) = *(volatile unsigned int *)(0x12106150);
	*(volatile unsigned int *)(0x1212025c) = *(volatile unsigned int *)(0x12106158);
	*(volatile unsigned int *)(0x12120260) = *(volatile unsigned int *)(0x1210615c);
	*(volatile unsigned int *)(0x12120264) = *(volatile unsigned int *)(0x12106160);
	*(volatile unsigned int *)(0x12120268) = *(volatile unsigned int *)(0x12106164);
	*(volatile unsigned int *)(0x1212026c) = *(volatile unsigned int *)(0x12106168);
	*(volatile unsigned int *)(0x12120270) = *(volatile unsigned int *)(0x1210616c);

	*(volatile unsigned int *)(0x12120274) = *(volatile unsigned int *)(0x12107000);
	*(volatile unsigned int *)(0x12120278) = *(volatile unsigned int *)(0x12107004);
	*(volatile unsigned int *)(0x1212027c) = *(volatile unsigned int *)(0x12107124);
	*(volatile unsigned int *)(0x12120280) = *(volatile unsigned int *)(0x12107128);
	*(volatile unsigned int *)(0x12120284) = *(volatile unsigned int *)(0x1210714c);
	*(volatile unsigned int *)(0x12120288) = *(volatile unsigned int *)(0x12107150);
	*(volatile unsigned int *)(0x1212028c) = *(volatile unsigned int *)(0x12107158);
	*(volatile unsigned int *)(0x12120290) = *(volatile unsigned int *)(0x1210715c);
	*(volatile unsigned int *)(0x12120294) = *(volatile unsigned int *)(0x12107160);
	*(volatile unsigned int *)(0x12120298) = *(volatile unsigned int *)(0x12107164);
	*(volatile unsigned int *)(0x1212029c) = *(volatile unsigned int *)(0x12107168);
	*(volatile unsigned int *)(0x121202a0) = *(volatile unsigned int *)(0x1210716c);

	*(volatile unsigned int *)(0x121202a4) = *(volatile unsigned int *)(0x12101220);
	*(volatile unsigned int *)(0x121202a8) = *(volatile unsigned int *)(0x12101224);
	*(volatile unsigned int *)(0x121202ac) = *(volatile unsigned int *)(0x12101228);
	*(volatile unsigned int *)(0x121202b0) = *(volatile unsigned int *)(0x1210122C);
	*(volatile unsigned int *)(0x121202b4) = *(volatile unsigned int *)(0x12101230);
	*(volatile unsigned int *)(0x121202b8) = *(volatile unsigned int *)(0x12101234);
	*(volatile unsigned int *)(0x121202bc) = *(volatile unsigned int *)(0x12101238);
	*(volatile unsigned int *)(0x121202c0) = *(volatile unsigned int *)(0x1210123C);
	*(volatile unsigned int *)(0x121202c4) = *(volatile unsigned int *)(0x12101248);
	*(volatile unsigned int *)(0x121202c8) = *(volatile unsigned int *)(0x1210124C);
	*(volatile unsigned int *)(0x121202cc) = *(volatile unsigned int *)(0x12101250);
	*(volatile unsigned int *)(0x121202d0) = *(volatile unsigned int *)(0x12101254);
	*(volatile unsigned int *)(0x121202d4) = *(volatile unsigned int *)(0x12101258);

	*(volatile unsigned int *)(0x121202d8) = *(volatile unsigned int *)(0x12102220);
	*(volatile unsigned int *)(0x121202dc) = *(volatile unsigned int *)(0x12102224);
	*(volatile unsigned int *)(0x121202e0) = *(volatile unsigned int *)(0x12102228);
	*(volatile unsigned int *)(0x121202e4) = *(volatile unsigned int *)(0x1210222C);
	*(volatile unsigned int *)(0x121202e8) = *(volatile unsigned int *)(0x12102230);
	*(volatile unsigned int *)(0x121202ec) = *(volatile unsigned int *)(0x12102234);
	*(volatile unsigned int *)(0x121202f0) = *(volatile unsigned int *)(0x12102238);
	*(volatile unsigned int *)(0x121202f4) = *(volatile unsigned int *)(0x1210223C);
	*(volatile unsigned int *)(0x121202f8) = *(volatile unsigned int *)(0x12102248);
	*(volatile unsigned int *)(0x121202fc) = *(volatile unsigned int *)(0x1210224C);
	*(volatile unsigned int *)(0x12120300) = *(volatile unsigned int *)(0x12102250);
	*(volatile unsigned int *)(0x12120304) = *(volatile unsigned int *)(0x12102254);
	*(volatile unsigned int *)(0x12120308) = *(volatile unsigned int *)(0x12102258);

	if (ddr_type == 3) { //LPDDR4
		*(volatile unsigned int *)(0x12101020) = 0x50000; //swicth to channel1
		*(volatile unsigned int *)(0x1212030c) = *(volatile unsigned int *)(0x12103104);
		*(volatile unsigned int *)(0x12120310) = *(volatile unsigned int *)(0x12103120);
		*(volatile unsigned int *)(0x12120314) = *(volatile unsigned int *)(0x12103124);
		*(volatile unsigned int *)(0x12120318) = *(volatile unsigned int *)(0x12103128);
		*(volatile unsigned int *)(0x1212031c) = *(volatile unsigned int *)(0x1210312c);
		*(volatile unsigned int *)(0x12120320) = *(volatile unsigned int *)(0x12103130);
		*(volatile unsigned int *)(0x12120324) = *(volatile unsigned int *)(0x12103134);
		*(volatile unsigned int *)(0x12120328) = *(volatile unsigned int *)(0x12103138);
		*(volatile unsigned int *)(0x1212032c) = *(volatile unsigned int *)(0x1210313c);
		*(volatile unsigned int *)(0x12120330) = *(volatile unsigned int *)(0x12103110);
		*(volatile unsigned int *)(0x12120334) = *(volatile unsigned int *)(0x12103100);
		*(volatile unsigned int *)(0x12101020) = 0x0; //switch back to channel0
	}

	//DDR SIZE register
	*(volatile unsigned int *)(0x12120338) = *(volatile unsigned int *)(0x120200c0);

	*(volatile unsigned int *)(0x12040000) = 'n';

	// enable io_retention_en
	*(volatile unsigned int *)(0x120f0008) = 0x10;

	str_delay_us(50);

	__asm__ volatile("dsb sy");
	__asm__ volatile("isb");

	//entry standby
	*(volatile unsigned int *)(0x120f0010) = (*(volatile unsigned int *)(0x120f0010)) | 0x1;  //bit0 start_standby;bit1 start_powerdown
	*(volatile unsigned int *)(0x12040000) = 'd';

	while(1);

}
