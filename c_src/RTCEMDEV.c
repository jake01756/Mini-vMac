/*
	RTCEMDEV.c

	Copyright (C) 2003 Philip Cummins, Paul Pratt

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
	Real Time Clock EMulated DEVice

	Emulates the RTC found in the Mac Plus.

	This code adapted from "RTC.c" in vMac by Philip Cummins.
*/

#ifndef AllFiles
#include "SYSDEPNS.h"
#include "MYOSGLUE.h"
#include "ENDIANAC.h"
#include "MINEM68K.h"
#include "ADDRSPAC.h"
#endif

/*define _RTC_Debug */
#ifdef _RTC_Debug
#include <stdio.h>
#endif

#include "RTCEMDEV.h"

#define HaveXPRAM (CurEmu >= kEmuPlus1M)

#if HaveXPRAM
#define PARAMRAMSize 256
#else
#define PARAMRAMSize 20
#endif

#if HaveXPRAM
#define Group1Base 0x10
#define Group2Base 0x08
#else
#define Group1Base 0x00
#define Group2Base 0x10
#endif

typedef struct
{
	/* RTC VIA Flags */
	ui3b UnEnabled;
	ui3b WrProtect;
	ui3b Clock;
	ui3b DataLine;
	ui3b DataOut;
	ui3b DataNextOut;

	/* RTC Data */
	ui3b ShiftData;
	ui3b Counter;
	ui3b Mode;
	ui3b SavedCmd;
#if HaveXPRAM
	ui3b Sector;
#endif

	/* RTC Registers */
	ui3b Seconds_1[4];
	ui3b PARAMRAM[PARAMRAMSize];
} RTC_Ty;

LOCALVAR RTC_Ty RTC;

/* RTC Functions */

LOCALVAR ui5b LastRealDate;

#ifndef SpeakerVol /* in 0..7 */
#if MySoundEnabled
#define SpeakerVol 7
#else
#define SpeakerVol 0
#endif
#endif

#ifndef TrackSpeed /* in 0..4 */
#define TrackSpeed 0
#endif

#ifndef AlarmOn /* in 0..1 */
#define AlarmOn 0
#endif

#ifndef DoubleClickTime /* in 5,8,12 */
#define DoubleClickTime 5
#endif

#ifndef CaretBlinkTime /* in 3,8,15 */
#define CaretBlinkTime 3
#endif

#ifndef DiskCacheSz /* in 1,2,3,4,6,8,12 */
/* actual cache size is DiskCacheSz * 32k */
#define DiskCacheSz 4
#endif

#ifndef MenuBlink /* in 0..3 */
#define MenuBlink 3
#endif

#ifndef StartUpDisk /* in 0..1 */
#define StartUpDisk 0
#endif

#ifndef DiskCacheOn /* in 0..1 */
#define DiskCacheOn 0
#endif

#ifndef MouseScalingOn /* in 0..1 */
#define MouseScalingOn 0
#endif

#ifndef AutoKeyThresh /* in 0,3,4,6,A */
#define AutoKeyThresh 6
#endif

#ifndef AutoKeyRate /* in 0,6,4,3,1 */
#define AutoKeyRate 3
#endif

#define prb_fontHi 0
#define prb_fontLo 2
#define prb_kbdPrintHi (AutoKeyRate + (AutoKeyThresh << 4))
#define prb_kbdPrintLo 0
#define prb_volClickHi (SpeakerVol + (TrackSpeed << 3) + (AlarmOn << 7))
#define prb_volClickLo (CaretBlinkTime + (DoubleClickTime << 4))
#define prb_miscHi DiskCacheSz
#define prb_miscLo ((MenuBlink << 2) + (StartUpDisk << 3) + (DiskCacheOn << 5) + (MouseScalingOn << 6))

GLOBALFUNC blnr RTC_Init(void)
{
	int Counter;
	ui5b secs;

	RTC.UnEnabled = RTC.Clock = RTC.Mode = RTC.ShiftData = RTC.Counter = 0;
	RTC.DataOut = RTC.DataNextOut = RTC.DataLine = 0;
	RTC.WrProtect = falseblnr;

	secs = CurMacDateInSeconds;
	LastRealDate = secs;

	RTC.Seconds_1[0] = secs & 0xFF;
	RTC.Seconds_1[1] = (secs & 0xFF00) >> 8;
	RTC.Seconds_1[2] = (secs & 0xFF0000) >> 16;
	RTC.Seconds_1[3] = (secs & 0xFF000000) >> 24;

	for (Counter = 0; Counter < PARAMRAMSize; Counter++) {
		RTC.PARAMRAM[Counter] = 0;
	}

#if 1
	RTC.PARAMRAM[0 + Group1Base] = 168; /* valid */
	RTC.PARAMRAM[3 + Group1Base] = 34; /* config, serial ports */
	RTC.PARAMRAM[4 + Group1Base] = 204; /* portA, high */
	RTC.PARAMRAM[5 + Group1Base] = 10; /* portA, low */
	RTC.PARAMRAM[6 + Group1Base] = 204; /* portB, high */
	RTC.PARAMRAM[7 + Group1Base] = 10; /* portB, low */
	RTC.PARAMRAM[13 + Group1Base] = prb_fontLo;
	RTC.PARAMRAM[14 + Group1Base] = prb_kbdPrintHi;

#if prb_volClickHi != 0
	RTC.PARAMRAM[0 + Group2Base] = prb_volClickHi;
#endif
	RTC.PARAMRAM[1 + Group2Base] = prb_volClickLo;
	RTC.PARAMRAM[2 + Group2Base] = prb_miscHi;
	RTC.PARAMRAM[3 + Group2Base] = prb_miscLo;

#if HaveXPRAM /* extended parameter ram initialized */
	RTC.PARAMRAM[12] = 0x42;
	RTC.PARAMRAM[13] = 0x75;
	RTC.PARAMRAM[14] = 0x67;
	RTC.PARAMRAM[15] = 0x73;

	do_put_mem_long(&RTC.PARAMRAM[0xE4], CurMacLatitude);
	do_put_mem_long(&RTC.PARAMRAM[0xE8], CurMacLongitude);
	do_put_mem_long(&RTC.PARAMRAM[0xEC], CurMacDelta);
#endif

#endif

	return trueblnr;
}

IMPORTPROC VIA_Int_One_Second(void);

GLOBALPROC RTC_Interrupt(void)
{
	ui5b Seconds = 0;
	ui5b NewRealDate = CurMacDateInSeconds;
	ui5b DateDelta = NewRealDate - LastRealDate;

	if (DateDelta != 0) {
		Seconds = (RTC.Seconds_1[3] << 24) + (RTC.Seconds_1[2] << 16) + (RTC.Seconds_1[1] << 8) + RTC.Seconds_1[0];
		Seconds += DateDelta;
		RTC.Seconds_1[0] = Seconds & 0xFF;
		RTC.Seconds_1[1] = (Seconds & 0xFF00) >> 8;
		RTC.Seconds_1[2] = (Seconds & 0xFF0000) >> 16;
		RTC.Seconds_1[3] = (Seconds & 0xFF000000) >> 24;

		LastRealDate = NewRealDate;

		VIA_Int_One_Second();
	}
}

LOCALFUNC ui3b RTC_Access_PRAM_Reg(ui3b Data, blnr WriteReg, ui3b t)
{
	if (WriteReg) {
		if (! RTC.WrProtect) {
			RTC.PARAMRAM[t] = Data;
#ifdef _RTC_Debug
			printf("Writing Address %2x, Data %2x\n", t, Data);
#endif
		}
	} else {
		Data = RTC.PARAMRAM[t];
	}
	return Data;
}

LOCALFUNC ui3b RTC_Access_Reg(ui3b Data, blnr WriteReg, ui3b TheCmd)
{
	ui3b t = (TheCmd & 0x7C) >> 2;
	if (t < 8) {
		if (WriteReg) {
			if (! RTC.WrProtect) {
				RTC.Seconds_1[t & 0x03] = Data;
			}
		} else {
			Data = RTC.Seconds_1[t & 0x03];
		}
	} else if (t < 12) {
		Data = RTC_Access_PRAM_Reg(Data, WriteReg,
			(t & 0x03) + Group2Base);
	} else if (t < 16) {
		if (WriteReg) {
			switch (t) {
				case 12 :
					break; /* Test Write, do nothing */
				case 13 :
					RTC.WrProtect = (Data & 0x80) != 0;
					break; /* Write_Protect Register */
				default :
					ReportAbnormal("Write RTC Reg unknown");
					break;
			}
		} else {
			ReportAbnormal("Read RTC Reg unknown");
		}
	} else {
		Data = RTC_Access_PRAM_Reg(Data, WriteReg,
			(t & 0x0F) + Group1Base);
	}
	return Data;
}

LOCALPROC RTC_DoCmd(void)
{
	switch (RTC.Mode) {
		case 0: /* This Byte is a RTC Command */
#if HaveXPRAM
			if ((RTC.ShiftData & 0x78) == 0x38) { /* Extended Command */
				RTC.SavedCmd = RTC.ShiftData;
				RTC.Mode = 2;
#ifdef _RTC_Debug
				printf("Extended command %2x\n", RTC.ShiftData);
#endif
			} else
#endif
			{
				if ((RTC.ShiftData & 0x80) != 0x00) { /* Read Command */
					RTC.ShiftData = RTC_Access_Reg(0, falseblnr, RTC.ShiftData);
					RTC.DataNextOut = 1;
				} else { /* Write Command */
					RTC.SavedCmd = RTC.ShiftData;
					RTC.Mode = 1;
				}
			}
			break;
		case 1: /* This Byte is data for RTC Write */
			(void) RTC_Access_Reg(RTC.ShiftData, trueblnr, RTC.SavedCmd);
			RTC.Mode = 0;
			break;
#if HaveXPRAM
		case 2: /* This Byte is rest of Extended RTC command address */
#ifdef _RTC_Debug
			printf("Mode 2 %2x\n", RTC.ShiftData);
#endif
			RTC.Sector = ((RTC.SavedCmd & 0x07) << 5)
				| ((RTC.ShiftData & 0x7C) >> 2);
			if ((RTC.SavedCmd & 0x80) != 0x00) { /* Read Command */
				RTC.ShiftData = RTC.PARAMRAM[RTC.Sector];
				RTC.DataNextOut = 1;
				RTC.Mode = 0;
#ifdef _RTC_Debug
				printf("Reading X Address %2x, Data  %2x\n", RTC.Sector, RTC.ShiftData);
#endif
			} else {
				RTC.Mode = 3;
#ifdef _RTC_Debug
				printf("Writing X Address %2x\n", RTC.Sector);
#endif
			}
			break;
		case 3: /* This Byte is data for an Extended RTC Write */
			(void) RTC_Access_PRAM_Reg(RTC.ShiftData, trueblnr, RTC.Sector);
			RTC.Mode = 0;
			break;
#endif
	}
}

/* VIA Interface Functions */

GLOBALFUNC ui3b VIA_GORB2(void) /* RTC Enable */
{
#ifdef _RTC_Debug
	printf("VIA ORB2 attempts to be an input\n");
#endif
	return 0;
}

GLOBALPROC VIA_PORB2(ui3b Data)
{
	if (Data != RTC.UnEnabled) {
		RTC.UnEnabled = Data;
		if (RTC.UnEnabled) {
			/* abort anything going on */
			if (RTC.Counter != 0) {
#ifdef _RTC_Debug
				printf("aborting, %2x\n", RTC.Counter);
#endif
				ReportAbnormal("RTC aborting");
			}
			RTC.Mode = 0;
			RTC.DataOut = 0;
			RTC.DataNextOut = 0;
			RTC.ShiftData = 0;
			RTC.Counter = 0;
		}
	}
}

GLOBALFUNC ui3b VIA_GORB1(void) /* RTC Data Clock */
{
#ifdef _RTC_Debug
	printf("VIA ORB1 attempts to be an input\n");
#endif
	return 0;
}

GLOBALPROC VIA_PORB1(ui3b Data)
{
	if (Data != RTC.Clock) {
		RTC.Clock = Data;

		if (! RTC.UnEnabled) {
			if (RTC.Clock) {
				RTC.Counter = (RTC.Counter - 1) & 0x07;
				if (RTC.DataOut) {
					RTC.DataLine = ((RTC.ShiftData >> RTC.Counter) & 0x01);
					if (RTC.Counter == 0) {
						RTC.DataNextOut = 0;
					}
				} else {
					RTC.ShiftData = (RTC.ShiftData << 1) | RTC.DataLine;
					if (RTC.Counter == 0) {
						RTC_DoCmd();
					}
				}
			} else {
				RTC.DataOut = RTC.DataNextOut;
			}
		}
	}
}

GLOBALFUNC ui3b VIA_GORB0(void) /* RTC Data */
{
	if (RTC.DataOut) {
		return RTC.DataLine;
	} else {
		ReportAbnormal("read RTC Data unexpected direction");
		return 0;
	}
}

GLOBALPROC VIA_PORB0(ui3b Data)
{
	if (RTC.DataOut) {
		ReportAbnormal("write RTC Data unexpected direction");
	} else {
		RTC.DataLine = Data;
	}
}
