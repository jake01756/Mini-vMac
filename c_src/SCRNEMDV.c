/*
	SCRNEMDV.c

	Copyright (C) 2003 Philip Cummins, Richard F. Bannister, Paul Pratt

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
	SCReeN EMulated DeVice

	Emulation of the screen in the Mac Plus.

	This code descended from "Screen-MacOS.c" in Richard F. Bannister's
	Macintosh port of vMac, by Philip Cummins.
*/

#ifndef AllFiles
#include "SYSDEPNS.h"
#include "MYOSGLUE.h"
#if UseControlKeys
#include "CONTROLM.h"
#endif
#include "ENDIANAC.h"
#include "MINEM68K.h"
#include "ADDRSPAC.h"
#endif

#include "SCRNEMDV.h"

LOCALVAR ui3b vPage2 = 1;

LOCALFUNC blnr FindFirstChangeInLVecs(long *ptr1, long *ptr2,
					long L, long *j)
{
/*
	find index of first difference
*/
	long *p1 = ptr1;
	long *p2 = ptr2;
	long i;

	for (i = L; --i >= 0; ) {
		if (*p1++ != *p2++) {
			--p1;
			*j = p1 - ptr1;
			return trueblnr;
		}
	}
	return falseblnr;
}

LOCALFUNC blnr FindLastChangeInLVecs(long *ptr1, long *ptr2,
					long L, long *j)
{
/*
	find index of last difference
*/
	long *p1 = ptr1 + L;
	long *p2 = ptr2 + L;
	long i;

	for (i = L; --i >= 0; ) {
		if (*--p1 != *--p2) {
			*j = p1 - ptr1;
			return trueblnr;
		}
	}
	return falseblnr;
}

LOCALVAR unsigned long NextDrawRow = 0;

/* Draw the screen */
GLOBALPROC Screen_Draw(void)
{
	char *screencurrentbuff;
	long j0;
	long j1;
	long copysize;
	long copyoffset;
	long copyrows;
	unsigned long LimitDrawRow;
	unsigned long MaxRowsDrawnPerTick;

	if (TimeAdjust <= 2) {
		MaxRowsDrawnPerTick = vMacScreenHeight;
	} else if (TimeAdjust <= 4) {
		MaxRowsDrawnPerTick = vMacScreenHeight / 2;
	} else {
		MaxRowsDrawnPerTick = vMacScreenHeight / 4;
	}

	if (vPage2 == 1) {
		screencurrentbuff = (char *) get_ram_address(kMain_Buffer);
	} else {
		screencurrentbuff = (char *) get_ram_address(kAlternate_Buffer);
	}

#if UseControlKeys
	if (CurControlMode != 0) {
		MyMoveBytes((anyp)screencurrentbuff, (anyp)CntrlDisplayBuff, vMacScreenNumBytes);
		screencurrentbuff = CntrlDisplayBuff;

		DrawControlMode();
	}
#endif

	if (! FindFirstChangeInLVecs((long *)screencurrentbuff + NextDrawRow * (vMacScreenWidth / 32),
			(long *)screencomparebuff + NextDrawRow * (vMacScreenWidth / 32),
			((long)(vMacScreenHeight - NextDrawRow) * (long)vMacScreenWidth) / 32, &j0))
	{
		NextDrawRow = 0;
		return;
	}
	j0 /= (vMacScreenWidth / 32);
	j0 += NextDrawRow;
	LimitDrawRow = j0 + MaxRowsDrawnPerTick;
	if (LimitDrawRow >= vMacScreenHeight) {
		LimitDrawRow = vMacScreenHeight;
		NextDrawRow = 0;
	} else {
		NextDrawRow = LimitDrawRow;
	}
	if (FindLastChangeInLVecs((long *)screencurrentbuff,
		(long *)screencomparebuff,
		((long)LimitDrawRow * (long)vMacScreenWidth) / 32, &j1))
	{
		j1 /= (vMacScreenWidth / 32); j1++;

		copyrows = j1 - j0;
		copyoffset = j0 * vMacScreenByteWidth;
		copysize = copyrows * vMacScreenByteWidth;
		MyMoveBytes((anyp)screencurrentbuff + copyoffset, (anyp)screencomparebuff + copyoffset, copysize);

		HaveChangedScreenBuff(j0, 0, j0 + copyrows, vMacScreenWidth);
	}
}

/* VIA Interface Functions */

GLOBALFUNC ui3b VIA_GORA6(void) /* Main/Alternate Screen Buffer */
{
#ifdef _VIA_Interface_Debug
	printf("VIA ORA6 attempts to be an input\n");
#endif
	return 0;
}

GLOBALPROC VIA_PORA6(ui3b Data)
{
	vPage2 = Data;
}

GLOBALFUNC ui3b VIA_GORB6(void) /* Video Beam in Display */
{
	return 0; /* Assume it is */
}

GLOBALPROC VIA_PORB6(ui3b Data)
{
	UnusedParam(Data);
#ifdef _VIA_Interface_Debug
	printf("VIA ORB6 attempts to be an output\n");
#endif
}
