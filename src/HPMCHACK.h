/*
	HPMCHACK.c

	Copyright (C) 2016 Steve Chamberlin, Paul C. Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

/*
	HaPpy MaCintosh Hack

	Patch the ROM for alternatives to the
	Happy Macintosh icon displayed on boot
	when a disk is inserted.
*/

#define kAHM_aside 0
#define kAHM_cheese 1
#define kAHM_evil 2
#define kAHM_horror 3
#define kAHM_lady_mac 4
#define kAHM_moustache 5
#define kAHM_nerdy 6
#define kAHM_pirate 7
#define kAHM_sleepy 8
#define kAHM_sly 9
#define kAHM_sunglasses 10
#define kAHM_surprise 11
#define kAHM_tongue 12
#define kAHM_yuck 13
#define kAHM_zombie 14

LOCALVAR const ui3b my_HappyMac_icon[] = {
#if CurAltHappyMac == kAHM_aside
	0x00, 0x00,
	0x39, 0x38,
	0x21, 0x20,
	0x01, 0x00,
	0x01, 0x00,
	0x03, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x07, 0x80,
	0x00, 0x00
#endif
#if CurAltHappyMac == kAHM_cheese
	0x10, 0x10,
	0x28, 0x28,
	0x00, 0x00,
	0x00, 0x00,
	0x3F, 0xF8,
	0x20, 0x08,
	0x20, 0x08,
	0x20, 0x08,
	0x10, 0x10,
	0x0F, 0xE0,
#endif
#if CurAltHappyMac == kAHM_evil
	0x00, 0x00,
	0x10, 0x10,
	0x08, 0x20,
	0x0C, 0x60,
	0x00, 0x00,
	0x20, 0x08,
	0x20, 0x08,
	0x1F, 0xF0,
	0x00, 0x00,
	0x00, 0x00
#endif
#if CurAltHappyMac == kAHM_horror
	0x38, 0x38,
	0x44, 0x44,
	0x44, 0x44,
	0x44, 0x44,
	0x38, 0x38,
	0x03, 0x80,
	0x03, 0x80,
	0x03, 0x80,
	0x03, 0x80,
	0x03, 0x80
#endif
#if CurAltHappyMac == kAHM_lady_mac
	0x38, 0x38,
	0x45, 0x44,
	0x55, 0x54,
	0x45, 0x44,
	0x39, 0x38,
	0x03, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x07, 0x80,
	0x03, 0x00
#endif
#if CurAltHappyMac == kAHM_moustache
	0x00, 0x00,
	0x11, 0x10,
	0x11, 0x10,
	0x01, 0x00,
	0x01, 0x00,
	0x03, 0x00,
	0x1F, 0xE0,
	0x00, 0x00,
	0x08, 0x40,
	0x07, 0x80
#endif
#if CurAltHappyMac == kAHM_nerdy
	0x38, 0x38,
	0x45, 0x45,
	0xD7, 0xD6,
	0x45, 0x44,
	0x39, 0x38,
	0x03, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x0F, 0xC0,
	0x00, 0x00
#endif
#if CurAltHappyMac == kAHM_pirate
	0x00, 0x81,
	0x00, 0x7E,
	0x11, 0x7E,
	0x11, 0x3C,
	0x01, 0x3C,
	0x01, 0x18,
	0x03, 0x00,
	0x00, 0x00,
	0x08, 0x40,
	0x07, 0x80
#endif
#if CurAltHappyMac == kAHM_sleepy
	0x00, 0x00,
	0x1C, 0x70,
	0x22, 0x88,
	0x00, 0x00,
	0x1C, 0x70,
	0x08, 0x20,
	0x00, 0x00,
	0x00, 0x00,
	0x03, 0x80,
	0x00, 0x00
#endif
#if CurAltHappyMac == kAHM_sly
	0x00, 0x00,
	0x08, 0x20,
	0x14, 0x50,
	0x00, 0x00,
	0x00, 0x00,
	0x20, 0x08,
	0x3F, 0xF8,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00
#endif
#if CurAltHappyMac == kAHM_sunglasses
	0x00, 0x00,
	0xFF, 0xFE,
	0x7D, 0x7C,
	0x7D, 0x7C,
	0x39, 0x38,
	0x03, 0x00,
	0x00, 0x00,
	0x1F, 0xF0,
	0x00, 0x00,
	0x00, 0x00
#endif
#if CurAltHappyMac == kAHM_surprise
	0x1C, 0x70,
	0x22, 0x88,
	0x41, 0x04,
	0x49, 0x24,
	0x41, 0x04,
	0x22, 0x88,
	0x1C, 0x70,
	0x01, 0x00,
	0x03, 0x80,
	0x03, 0x80
#endif
#if CurAltHappyMac == kAHM_tongue
	0x00, 0x00,
	0x1E, 0x78,
	0x00, 0x00,
	0x00, 0x00,
	0x20, 0x04,
	0x3F, 0xFC,
	0x05, 0x40,
	0x05, 0x40,
	0x04, 0x40,
	0x03, 0x80
#endif
#if CurAltHappyMac == kAHM_yuck
	0x00, 0x00,
	0x18, 0x30,
	0x04, 0x40,
	0x02, 0x80,
	0x00, 0x00,
	0x00, 0x00,
	0x1F, 0xF0,
	0x15, 0x50,
	0x04, 0x40,
	0x03, 0x80
#endif
#if CurAltHappyMac == kAHM_zombie
	0x70, 0x7C,
	0x88, 0x82,
	0x88, 0x8A,
	0xA8, 0x8A,
	0x70, 0x82,
	0x00, 0x42,
	0x00, 0x3C,
	0x1E, 0x00,
	0x3F, 0x00,
	0x3F, 0x00
#endif
};

#if CurEmMd <= kEmMd_Twig43
#define HappyMacBase 0xA34
#elif CurEmMd <= kEmMd_Twiggy
#define HappyMacBase 0x8F4
#elif CurEmMd <= kEmMd_128K
#define HappyMacBase 0x8A0
#elif CurEmMd <= kEmMd_Plus
#define HappyMacBase 0xFD2
#elif CurEmMd <= kEmMd_Classic
#define HappyMacBase 0x125C
#elif CurEmMd <= kEmMd_PB100
#define HappyMacBase 0x2BB0
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
#define HappyMacBase 0x1948
#endif

LOCALPROC PatchHappyMac(void)
{
#if (CurEmMd == kEmMd_PB100) \
	|| (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)

	int i;
	ui3b *dst = HappyMacBase + ROM + 0x18;
	ui3b *src = (ui3b *)my_HappyMac_icon;

	for (i = 10; --i >= 0; ) {
		++dst;
		*dst++ = *src++;
		*dst++ = *src++;
		++dst;
	}

#else
	MyMoveBytes((anyp)my_HappyMac_icon,
		(anyp)(HappyMacBase + ROM),
		sizeof(my_HappyMac_icon));
#endif
}
