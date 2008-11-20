/*
	ADDRSPAC.c

	Copyright (C) 2006 Bernd Schmidt, Philip Cummins, Paul C. Pratt

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
	ADDRess SPACe

	Implements the address space of the Mac Plus.

	This code is descended from code in vMac by Philip Cummins, in
	turn descended from code in the Un*x Amiga Emulator by
	Bernd Schmidt.
*/

#ifndef AllFiles
#include "SYSDEPNS.h"
#include "MYOSGLUE.h"
#include "ENDIANAC.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#endif

#include "ADDRSPAC.h"

GLOBALVAR ui3b Wires[kNumWires];

IMPORTPROC EmulatedHardwareZap(void);

IMPORTPROC m68k_IPLchangeNtfy(void);
IMPORTPROC MINEM68K_Init(ui3b **BankReadAddr, ui3b **BankWritAddr,
	ui3b *fIPL);

IMPORTFUNC ui5b GetInstructionsRemaining(void);
IMPORTPROC SetInstructionsRemaining(ui5b n);

IMPORTPROC m68k_go_nInstructions(ui5b n);

IMPORTFUNC ui5b SCSI_Access(ui5b Data, blnr WriteMem, CPTR addr);
IMPORTFUNC ui5b SCC_Access(ui5b Data, blnr WriteMem, CPTR addr);
IMPORTFUNC ui5b IWM_Access(ui5b Data, blnr WriteMem, CPTR addr);
IMPORTFUNC ui5b VIA1_Access(ui5b Data, blnr WriteMem, CPTR addr);
#if EmVIA2
IMPORTFUNC ui5b VIA2_Access(ui5b Data, blnr WriteMem, CPTR addr);
#endif
#if EmASC
IMPORTFUNC ui5b ASC_Access(ui5b Data, blnr WriteMem, CPTR addr);
#endif
IMPORTPROC Sony_Access(ui5b Data, CPTR addr);

LOCALVAR blnr GotOneAbnormal = falseblnr;

#ifndef ReportAbnormalInterrupt
#define ReportAbnormalInterrupt 0
#endif

#if DetailedAbnormalReport
GLOBALPROC DoReportAbnormal(char *s)
#else
GLOBALPROC DoReportAbnormal(void)
#endif
{
	if (! GotOneAbnormal) {
#if DetailedAbnormalReport
		WarnMsgAbnormal(s);
#else
		WarnMsgAbnormal();
#endif
#if ReportAbnormalInterrupt
		SetInterruptButton(trueblnr);
#endif
		GotOneAbnormal = trueblnr;
	}
}

IMPORTPROC SCC_Reset(void);

/* top 8 bits out of 32 are ignored, so total size of address space is 2 ** 24 bytes */

#define TotAddrBytes (1UL << ln2TotAddrBytes)
#define kAddrMask (TotAddrBytes - 1)

/* map of address space */

#define kRAM_Base 0x00000000 /* when overlay off */
#if (CurEmMd == kEmMd_PB100) || (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
#define kRAM_ln2Spc 23
#else
#define kRAM_ln2Spc 22
#endif

#if CurEmMd == kEmMd_PB100
#define kROM_Base 0x00900000
#define kROM_ln2Spc 20
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
#define kROM_Base 0x00800000
#define kROM_ln2Spc 20
#else
#define kROM_Base 0x00400000
#define kROM_ln2Spc 20
#endif

#if IncludeVidMem
#if CurEmMd == kEmMd_PB100
#define kVidMem_Base 0x00FA0000
#define kVidMem_ln2Spc 16
#else
#define kVidMem_Base 0x00540000
#define kVidMem_ln2Spc 18
#endif
#endif

#if CurEmMd == kEmMd_PB100
#define kSCSI_Block_Base 0x00F90000
#define kSCSI_ln2Spc 16
#else
#define kSCSI_Block_Base 0x00580000
#define kSCSI_ln2Spc 19
#endif

#define kRAM_Overlay_Base 0x00600000 /* when overlay on */
#define kRAM_Overlay_Top  0x00800000

#if CurEmMd == kEmMd_PB100
#define kSCCRd_Block_Base 0x00FD0000
#define kSCC_ln2Spc 16
#else
#define kSCCRd_Block_Base 0x00800000
#define kSCC_ln2Spc 22
#endif

#if CurEmMd != kEmMd_PB100
#define kSCCWr_Block_Base 0x00A00000
#define kSCCWr_Block_Top  0x00C00000
#endif

#if CurEmMd == kEmMd_PB100
#define kIWM_Block_Base 0x00F60000
#define kIWM_ln2Spc 16
#else
#define kIWM_Block_Base 0x00C00000
#define kIWM_ln2Spc 21
#endif

#if CurEmMd == kEmMd_PB100
#define kVIA1_Block_Base 0x00F70000
#define kVIA1_ln2Spc 16
#else
#define kVIA1_Block_Base 0x00E80000
#define kVIA1_ln2Spc 19
#endif

#if CurEmMd == kEmMd_PB100
#define kASC_Block_Base 0x00FB0000
#define kASC_ln2Spc 16
#endif
#define kASC_Mask 0x00000FFF

#define kDSK_Block_Base 0x00F40000
#define kDSK_ln2Spc 5

/* implementation of read/write for everything but RAM and ROM */

#define kSCC_Mask 0x03

#define kVIA1_Mask 0x00000F
#if EmVIA2
#define kVIA2_Mask 0x00000F
#endif

#define kIWM_Mask 0x00000F /* Allocated Memory Bandwidth for IWM */

#if CurEmMd <= kEmMd_512Ke
#define ROM_CmpZeroMask 0
#elif CurEmMd <= kEmMd_Plus
#define ROM_CmpZeroMask 0x00020000
#elif CurEmMd <= kEmMd_PB100
#define ROM_CmpZeroMask 0
#elif CurEmMd <= kEmMd_IIx
#define ROM_CmpZeroMask 0
#else
#error "ROM_CmpZeroMask not defined"
#endif

#if CurEmMd <= kEmMd_512Ke
#define Overlay_ROM_CmpZeroMask 0x00100000
#elif CurEmMd <= kEmMd_Plus
#define Overlay_ROM_CmpZeroMask 0x00020000
#elif CurEmMd <= kEmMd_Classic
#define Overlay_ROM_CmpZeroMask 0x00300000
#elif CurEmMd <= kEmMd_PB100
#define Overlay_ROM_CmpZeroMask 0
#elif CurEmMd <= kEmMd_IIx
#define Overlay_ROM_CmpZeroMask 0
#else
#error "Overlay_ROM_CmpZeroMask not defined"
#endif

/* devide address space into banks, some of which are mapped to real memory */

LOCALVAR ui3b *BankReadAddr[NumMemBanks];
LOCALVAR ui3b *BankWritAddr[NumMemBanks];
	/* if BankWritAddr[i] != NULL then BankWritAddr[i] == BankReadAddr[i] */

LOCALPROC SetPtrVecToNULL(ui3b **x, ui5b n)
{
	ui5b i;

	for (i = 0; i < n; i++) {
		*x++ = nullpr;
	}
}

LOCALPROC SetUpMemBanks(void)
{
	SetPtrVecToNULL(BankReadAddr, NumMemBanks);
	SetPtrVecToNULL(BankWritAddr, NumMemBanks);
}

LOCALPROC get_RAM_realblock(blnr WriteMem, CPTR addr,
	ui3p *RealStart, ui5b *RealSize)
{
	UnusedParam(WriteMem);

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	{
		ui5r bankbit = 0x00100000 << (((VIA2_iA7 << 1) | VIA2_iA6) << 1);
		if (0 != (addr & bankbit)) {
#if kRAMb_Size != 0
			*RealStart = kRAMa_Size + RAM;
			*RealSize = kRAMb_Size;
#else
			/* fail */
#endif
		} else {
			*RealStart = RAM;
			*RealSize = kRAMa_Size;
		}
	}
#elif (0 == kRAMb_Size) || (kRAMa_Size == kRAMb_Size)
	UnusedParam(addr);

	*RealStart = RAM;
	*RealSize = kRAM_Size;
#else
	/* unbalanced memory */
	if (0 != (addr & kRAMa_Size)) {
		*RealStart = kRAMa_Size + RAM;
		*RealSize = kRAMb_Size;
	} else {
		*RealStart = RAM;
		*RealSize = kRAMa_Size;
	}
#endif
}

LOCALPROC get_ROM_realblock(blnr WriteMem, CPTR addr,
	ui3p *RealStart, ui5b *RealSize)
{
	UnusedParam(addr);

	if (WriteMem) {
		/* fail */
	} else {
		*RealStart = ROM;
		*RealSize = kROM_Size;
	}
}

LOCALPROC get_address24_realblock(blnr WriteMem, CPTR addr,
	ui3p *RealStart, ui5b *RealSize)
{
	if ((addr >> kRAM_ln2Spc) == (kRAM_Base >> kRAM_ln2Spc)) {
		if (MemOverlay) {
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
			ReportAbnormal("Overlay with 24 bit addressing");
#endif
			if ((addr & Overlay_ROM_CmpZeroMask) != 0) {
				/* fail */
			} else {
				get_ROM_realblock(WriteMem, addr,
					RealStart, RealSize);
			}
		} else {
			get_RAM_realblock(WriteMem, addr,
				RealStart, RealSize);
		}
	} else
#if IncludeVidMem && ((CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx))
	if ((addr >= 0x900000) && ((addr < 0xA00000))) {
		*RealStart = VidMem;
		*RealSize = ((kVidMemRAM_Size - 1) & 0x0FFFFF) + 1;
	} else
#endif
#if IncludeVidMem && (CurEmMd != kEmMd_II) && (CurEmMd != kEmMd_IIx)
	if ((addr >> kVidMem_ln2Spc) == (kVidMem_Base >> kVidMem_ln2Spc)) {
		*RealStart = VidMem;
		*RealSize = kVidMemRAM_Size;
	} else
#endif
	if ((addr >> kROM_ln2Spc) == (kROM_Base >> kROM_ln2Spc)) {
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
		if (MemOverlay) {
			ReportAbnormal("Overlay with 24 bit addressing");
		}
#elif CurEmMd >= kEmMd_SE
		if (MemOverlay != 0) {
			MemOverlay = 0;
			SetUpMemBanks();
		}
#endif
		if ((addr & ROM_CmpZeroMask) != 0) {
			/* fail */
		} else {
			get_ROM_realblock(WriteMem, addr,
				RealStart, RealSize);
		}
	} else
#if (CurEmMd != kEmMd_II) && (CurEmMd != kEmMd_IIx)
	if ((addr >> 19) == (kRAM_Overlay_Base >> 19)) {
		if (MemOverlay) {
			get_RAM_realblock(WriteMem, addr,
				RealStart, RealSize);
		}
	} else
#endif
	{
		/* fail */
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
		ReportAbnormal("bad memory access");
#endif
	}
}

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
LOCALPROC get_address32_realblock(blnr WriteMem, CPTR addr,
	ui3p *RealStart, ui5b *RealSize)
{
	if ((addr >> 30) == (0x00000000 >> 30)) {
		if (MemOverlay) {
			get_ROM_realblock(WriteMem, addr,
				RealStart, RealSize);
		} else {
			get_RAM_realblock(WriteMem, addr,
				RealStart, RealSize);
		}
	} else
	if ((addr >> 28) == (0x40000000 >> 28)) {
		get_ROM_realblock(WriteMem, addr,
			RealStart, RealSize);
	} else
#if 0
	/* haven't persuaded emulated computer to look here yet. */
	if ((addr >> 28) == (0x90000000 >> 28)) {
		/* NuBus super space */
		*RealStart = VidMem;
		*RealSize = kVidMemRAM_Size;
	} else
#endif
	if ((addr >> 28) == (0xF0000000 >> 28)) {
		/* Standard NuBus space */
		if ((addr >> 24) == (0xF9000000 >> 24)) {
			/* if ((addr >= 0xFA000000 - kVidROM_Size) && (addr < 0xFA000000)) */
			/* if (addr >= 0xF9800000) */
			if ((addr >> 20) == (0xF9F00000 >> 20))
			{
				if (WriteMem) {
					/* fail */
				} else {
					*RealStart = VidROM;
					*RealSize = kVidROM_Size;
				}
			} else {
#if kVidMemRAM_Size <= 0x00100000
				*RealStart = VidMem;
				*RealSize = kVidMemRAM_Size;
#else
				/* ugly kludge to allow more 1M of Video Memory */
				int i = (addr >> 20) & 0xF;
				if (i >= 9) {
					i -= 9;
				}
				*RealStart = VidMem + ((i << 20) & (kVidMemRAM_Size - 1));
				*RealSize = 0x00100000;
#endif
			}
		} else {
			/* fail */
		}
	} else
	{
		/* fail */
	}
}
#endif

LOCALPROC GetBankAddr(blnr WriteMem, CPTR addr,
	ui3p *RealStart)
{
	ui5b bi = bankindex(addr);
	ui3b **CurBanks = WriteMem ? BankWritAddr : BankReadAddr;
	ui3p RealStart0 = CurBanks[bi];

	if (RealStart0 == nullpr) {
		ui5b RealSize0;
		get_address24_realblock(WriteMem, addr,
			&RealStart0, &RealSize0);
		if (RealStart0 != nullpr) {
			RealStart0 += ((bi << ln2BytesPerMemBank)
				& (RealSize0 - 1));
			CurBanks[bi] = RealStart0;
		}
	}

	*RealStart = RealStart0;
	/* *RealSize = BytesPerMemBank; */
}

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
LOCALFUNC ui5b MM_IOAccess(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr)
{
	if ((addr >= 0) && (addr < 0x2000)) {
		if (! ByteSize) {
			ReportAbnormal("access VIA1 word");
		} else if ((addr & 1) != 0) {
			ReportAbnormal("access VIA1 odd");
		} else {
			if ((addr & 0x000001FE) != 0x00000000) {
				ReportAbnormal("access VIA1 nonstandard address");
			}
			Data = VIA1_Access(Data, WriteMem, (addr >> 9) & kVIA1_Mask);
		}
	} else
	if ((addr >= 0x2000) && (addr < 0x4000)) {
		if (! ByteSize) {
			if ((! WriteMem) && ((0x3e00 == addr) || (0x3e02 == addr))) {
				/* for weirdness at offset 0x71E in ROM */
				Data = (VIA2_Access(Data, WriteMem, (addr >> 9) & kVIA2_Mask) << 8)
					| VIA2_Access(Data, WriteMem, (addr >> 9) & kVIA2_Mask);

			} else {
				ReportAbnormal("access VIA2 word");
			}
		} else if ((addr & 1) != 0) {
			if (0x3FFF == addr) {
				/* for weirdness at offset 0x7C4 in ROM. looks like bug. */
				Data = VIA2_Access(Data, WriteMem, (addr >> 9) & kVIA2_Mask);
			} else {
				ReportAbnormal("access VIA2 odd");
			}
		} else {
			if ((addr & 0x000001FE) != 0x00000000) {
				ReportAbnormal("access VIA2 nonstandard address");
			}
			Data = VIA2_Access(Data, WriteMem, (addr >> 9) & kVIA2_Mask);
		}
	} else
	if ((addr >= 0x4000) && (addr < 0x6000)) {
		if (! ByteSize) {
			ReportAbnormal("Attemped Phase Adjust");
		} else
		{
			if ((addr & 0x1FF9) != 0x00000000) {
				ReportAbnormal("access SCC nonstandard address");
			}
			Data = SCC_Access(Data, WriteMem, (addr >> 1) & kSCC_Mask);
		}
	} else
	if ((addr >= 0x0C000) && (addr < 0x0E000)) {
		if (ByteSize) {
			ReportAbnormal("access Sony byte");
		} else if ((addr & 1) != 0) {
			ReportAbnormal("access Sony odd");
		} else if (! WriteMem) {
			ReportAbnormal("access Sony read");
		} else {
			Sony_Access(Data, (addr >> 1) & 0x0F);
		}
	} else
	if ((addr >= 0x10000) && (addr < 0x12000)) {
		if (! ByteSize) {
			ReportAbnormal("access SCSI word");
		} else {
			if ((addr & 0x1F8F) != 0x00000000) {
				ReportAbnormal("access SCC nonstandard address");
			}
			Data = SCSI_Access(Data, WriteMem, (addr >> 4) & 0x07);
		}
	} else
	if ((addr >= 0x14000) && (addr < 0x16000)) {
		if (! ByteSize) {
			if (WriteMem) {
				(void) ASC_Access((Data >> 8) & 0x00FF, WriteMem, addr & kASC_Mask);
				Data = ASC_Access((Data) & 0x00FF, WriteMem, (addr + 1) & kASC_Mask);
			} else {
				ReportAbnormal("access ASC word");
			}
		} else {
			Data = ASC_Access(Data, WriteMem, addr & kASC_Mask);
		}
	} else
	if ((addr >= 0x16000) && (addr < 0x18000)) {
		if (! ByteSize) {
			ReportAbnormal("access IWM word");
		} else if ((addr & 1) != 0) {
			ReportAbnormal("access IWM odd");
		} else {
			Data = IWM_Access(Data, WriteMem, (addr >> 9) & kIWM_Mask);
		}
	} else
#if 0
	if ((addr >= 0x1C000) && (addr < 0x1E000)) {
		/* fail, nothing supposed to be here, but rom accesses it anyway */
		if ((addr != 0x1DA00) && (addr != 0x1DC00)) {
			ReportAbnormal("another unknown access");
		}
	} else
#endif
	{
		ReportAbnormal("MM_Access fails");
	}
	return Data;
}
#endif

GLOBALFUNC ui5b MM_Access(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr)
{
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	if (Addr32) {
		ui3p RealStart = nullpr;
		ui5b RealSize;
		ui3p m;

		get_address32_realblock(WriteMem, addr,
			&RealStart, &RealSize);
		if (nullpr != RealStart) {
			m = RealStart + (addr & (RealSize - 1));

			if (ByteSize) {
				if (WriteMem) {
					*m = Data;
				} else {
					Data = (si5b)(si3b)*m;
				}
			} else {
				if (WriteMem) {
					do_put_mem_word(m, Data);
				} else {
					Data = (si5b)(si4b)do_get_mem_word(m);
				}
			}
		} else {
			if ((addr >> 24) == (0x50000000 >> 24)) {
				Data = MM_IOAccess(Data, WriteMem, ByteSize, addr & 0x1FFFF);
			} else
			if ((addr >= 0x58000000) && (addr < 0x58000004)) {
				/* test hardware. fail */
			} else
			{
				/* ReportAbnormal("Other IO access"); */
			}
		}
	} else
#endif
	{
		CPTR mAddressBus = addr & kAddrMask;

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
		if ((mAddressBus >> 20) == (0x00F00000 >> 20)) {
			Data = MM_IOAccess(Data, WriteMem, ByteSize, mAddressBus & 0x1FFFF);
		} else
#else
		if ((mAddressBus >> kVIA1_ln2Spc) == (kVIA1_Block_Base >> kVIA1_ln2Spc)) {
			if (! ByteSize) {
				ReportAbnormal("access VIA word");
			} else if ((mAddressBus & 1) != 0) {
				ReportAbnormal("access VIA odd");
			} else {
#if CurEmMd != kEmMd_PB100
				if ((mAddressBus & 0x000FE1FE) != 0x000FE1FE) {
					ReportAbnormal("access VIA nonstandard address");
				}
#endif
				Data = VIA1_Access(Data, WriteMem, (mAddressBus >> 9) & kVIA1_Mask);
			}
		} else
		if ((mAddressBus >> kSCC_ln2Spc) == (kSCCRd_Block_Base >> kSCC_ln2Spc)) {
#if CurEmMd >= kEmMd_SE
			if ((mAddressBus & 0x00100000) == 0) {
				ReportAbnormal("access SCC unassigned address");
			} else
#endif
			if (! ByteSize) {
				ReportAbnormal("Attemped Phase Adjust");
			} else if (WriteMem != ((mAddressBus & 1) != 0)) {
				if (WriteMem) {
#if CurEmMd >= kEmMd_512Ke
#if CurEmMd != kEmMd_PB100
					ReportAbnormal("access SCC even/odd");
					/*
						This happens on boot with 64k ROM.
					*/
#endif
#endif
				} else {
					SCC_Reset();
				}
			} else
#if CurEmMd != kEmMd_PB100
			if (WriteMem != (mAddressBus >= kSCCWr_Block_Base)) {
				ReportAbnormal("access SCC wr/rd base wrong");
			} else
#endif
			{
#if CurEmMd != kEmMd_PB100
				if ((mAddressBus & 0x001FFFF8) != 0x001FFFF8) {
					ReportAbnormal("access SCC nonstandard address");
				}
#endif
				Data = SCC_Access(Data, WriteMem, (mAddressBus >> 1) & kSCC_Mask);
			}
		} else
		if ((mAddressBus >> kDSK_ln2Spc) == (kDSK_Block_Base >> kDSK_ln2Spc)) {
			if (ByteSize) {
				ReportAbnormal("access Sony byte");
			} else if ((mAddressBus & 1) != 0) {
				ReportAbnormal("access Sony odd");
			} else if (! WriteMem) {
				ReportAbnormal("access Sony read");
			} else {
				Sony_Access(Data, (mAddressBus >> 1) & 0x0F);
			}
		} else
#if CurEmMd == kEmMd_PB100
		if ((mAddressBus >> kASC_ln2Spc) == (kASC_Block_Base >> kASC_ln2Spc)) {
			if (! ByteSize) {
				ReportAbnormal("access ASC word");
			} else {
				Data = ASC_Access(Data, WriteMem, mAddressBus & kASC_Mask);
			}
		} else
#endif
		if ((mAddressBus >> kSCSI_ln2Spc) == (kSCSI_Block_Base >> kSCSI_ln2Spc)) {
			if (! ByteSize) {
				ReportAbnormal("access SCSI word");
			} else if (WriteMem != ((mAddressBus & 1) != 0)) {
				ReportAbnormal("access SCSI even/odd");
			} else {
				Data = SCSI_Access(Data, WriteMem, (mAddressBus >> 4) & 0x07);
			}
		} else
		if ((mAddressBus >> kIWM_ln2Spc) == (kIWM_Block_Base >> kIWM_ln2Spc)) {
#if CurEmMd >= kEmMd_SE
			if ((mAddressBus & 0x00100000) == 0) {
				ReportAbnormal("access IWM unassigned address");
			} else
#endif
			if (! ByteSize) {
#if ExtraAbnormalReports
				ReportAbnormal("access IWM word");
				/*
					This happens when quitting 'Glider 3.1.2'.
					perhaps a bad handle is being disposed of.
				*/
#endif
			} else if ((mAddressBus & 1) == 0) {
				ReportAbnormal("access IWM even");
			} else {
#if CurEmMd != kEmMd_PB100
				if ((mAddressBus & 0x001FE1FF) != 0x001FE1FF) {
					ReportAbnormal("access IWM nonstandard address");
				}
#endif
				Data = IWM_Access(Data, WriteMem, (mAddressBus >> 9) & kIWM_Mask);
			}
		} else
#endif
		{
			ui3p RealStart;

			GetBankAddr(WriteMem, mAddressBus, &RealStart);
			if (nullpr != RealStart) {
				ui3p m = RealStart + (mAddressBus & (BytesPerMemBank - 1));
				if (ByteSize) {
					if (WriteMem) {
						*m = Data;
					} else {
						Data = (si5b)(si3b)*m;
					}
				} else {
					if (WriteMem) {
						do_put_mem_word(m, Data);
					} else {
						Data = (si5b)(si4b)do_get_mem_word(m);
					}
				}
			}
		}
	}
	return Data;
}

GLOBALFUNC si5b get_vm_word(CPTR addr)
{
	ui3p ba = BankReadAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui3p m = (addr & MemBankAddrMask) + ba;
		return (si5b)(si4b)do_get_mem_word(m);
	} else {
		return (si5b)(si4b)(ui4b) MM_Access(0, falseblnr, falseblnr, addr);
	}
}

GLOBALFUNC si5b get_vm_byte(CPTR addr)
{
	ui3p ba = BankReadAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui3p m = (addr & MemBankAddrMask) + ba;
		return (si5b)(si3b)*m;
	} else {
		return (si5b)(si3b)(ui3b) MM_Access(0, falseblnr, trueblnr, addr);
	}
}

GLOBALFUNC ui5b get_vm_long(CPTR addr)
{
	ui3p ba = BankReadAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui3p m = (addr & MemBankAddrMask) + ba;
		return do_get_mem_long(m);
	} else {
		si5b hi = get_vm_word(addr);
		si5b lo = get_vm_word(addr + 2);
		return (ui5b) ((((ui5b)hi) << 16) & 0xFFFF0000)
			| (((ui5b)lo) & 0x0000FFFF);
	}
}

GLOBALPROC put_vm_word(CPTR addr, ui5b w)
{
	ui3p ba = BankWritAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui3p m = (addr & MemBankAddrMask) + ba;
		do_put_mem_word(m, w);
	} else {
		(void) MM_Access(w & 0x0000FFFF, trueblnr, falseblnr, addr);
	}
}

GLOBALPROC put_vm_byte(CPTR addr, ui5b b)
{
	ui3p ba = BankWritAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui3p m = (addr & MemBankAddrMask) + ba;
		*m = b;
	} else {
		(void) MM_Access(b & 0x00FF, trueblnr, trueblnr, addr);
	}
}

GLOBALPROC put_vm_long(CPTR addr, ui5b l)
{
	ui3p ba = BankWritAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui3p m = (addr & MemBankAddrMask) + ba;
		do_put_mem_long(m, l);
	} else {
		put_vm_word(addr, l >> 16);
		put_vm_word(addr + 2, l);
	}
}

GLOBALPROC MemOverlay_ChangeNtfy(void)
{
#if CurEmMd <= kEmMd_Plus
	SetUpMemBanks();
#endif
}

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
GLOBALPROC Addr32_ChangeNtfy(void)
{
	SetUpMemBanks();
}
#endif


/*
	unlike in the real Mac Plus, Mini vMac
	will allow misaligned memory access,
	since it is easier to allow it than
	it is to correctly simulate a bus error
	and back out of the current instruction.
*/

LOCALPROC get_address_realblock(blnr WriteMem, CPTR addr,
	ui3p *RealStart, ui5b *RealSize)
{
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	if (Addr32) {
		*RealStart = nullpr;
		get_address32_realblock(WriteMem, addr,
			RealStart, RealSize);
	} else
#endif
	{
		*RealSize = BytesPerMemBank;
		GetBankAddr(WriteMem, addr & kAddrMask, RealStart);
	}
}

GLOBALFUNC ui3p get_real_address(ui5b L, blnr WritableMem, CPTR addr)
{
	ui3p RealStart;
	ui5b RealSize;

	get_address_realblock(WritableMem, addr,
		&RealStart, &RealSize);
	if (nullpr == RealStart) {
		return nullpr;
	} else {
		ui5b bankoffset = addr & (RealSize - 1);
		ui5b bankleft = RealSize - bankoffset;
		ui3p p = bankoffset + RealStart;
label_1:
		if (bankleft >= L) {
			return p; /* ok */
		} else {
			ui3p bankend = RealSize + RealStart;

			addr += bankleft;
			get_address_realblock(WritableMem, addr,
				&RealStart, &RealSize);
			if ((nullpr == RealStart)
				|| (RealStart != bankend))
			{
				return nullpr;
			} else {
				bankoffset = addr & (RealSize - 1);
				if (bankoffset != 0) {
					ReportAbnormal("problem with get_address_realblock");
				}
				L -= bankleft;
				bankleft = RealSize;
				goto label_1;
			}
		}
	}
}

GLOBALVAR blnr InterruptButton = falseblnr;

GLOBALPROC SetInterruptButton(blnr v)
{
	if (InterruptButton != v) {
		InterruptButton = v;
		VIAorSCCinterruptChngNtfy();
	}
}

IMPORTPROC DoMacReset(void);

GLOBALPROC InterruptReset_Update(void)
{
	SetInterruptButton(falseblnr);
		/*
			in case has been set. so only stays set
			for 60th of a second.
		*/

	if (WantMacInterrupt) {
		SetInterruptButton(trueblnr);
		WantMacInterrupt = falseblnr;
	}
	if (WantMacReset) {
		DoMacReset();
		WantMacReset = falseblnr;
	}
}

LOCALVAR ui3b CurIPL = 0;

GLOBALPROC VIAorSCCinterruptChngNtfy(void)
{
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	ui3b NewIPL;

	if (InterruptButton) {
		NewIPL = 7;
	} else if (SCCInterruptRequest) {
		NewIPL = 4;
	} else if (VIA2_InterruptRequest) {
		NewIPL = 2;
	} else if (VIA1_InterruptRequest) {
		NewIPL = 1;
	} else {
		NewIPL = 0;
	}
#else
	ui3b VIAandNotSCC = VIA1_InterruptRequest
		& ~ SCCInterruptRequest;
	ui3b NewIPL = VIAandNotSCC
		| (SCCInterruptRequest << 1)
		| (InterruptButton << 2);
#endif
	if (NewIPL != CurIPL) {
		CurIPL = NewIPL;
		m68k_IPLchangeNtfy();
	}
}

GLOBALFUNC blnr AddrSpac_Init(void)
{
	int i;

	for (i = 0; i < kNumWires; i++) {
		Wires[i] = 1;
	}

	MINEM68K_Init(BankReadAddr, BankWritAddr,
		&CurIPL);
	EmulatedHardwareZap();
	return trueblnr;
}

#if EmClassicKbrd
IMPORTPROC DoKybd_ReceiveEndCommand(void);
IMPORTPROC DoKybd_ReceiveCommand(void);
#endif
#if EmADB
IMPORTPROC ADB_DoNewState(void);
#endif
#if EmPMU
IMPORTPROC PMU_DoTask(void);
#endif
IMPORTPROC VIA1_DoTimer1Check(void);
IMPORTPROC VIA1_DoTimer2Check(void);
#if EmVIA2
IMPORTPROC VIA2_DoTimer1Check(void);
IMPORTPROC VIA2_DoTimer2Check(void);
#endif

LOCALPROC ICT_DoTask(int taskid)
{
	switch (taskid) {
#if EmClassicKbrd
		case kICT_Kybd_ReceiveEndCommand:
			DoKybd_ReceiveEndCommand();
			break;
		case kICT_Kybd_ReceiveCommand:
			DoKybd_ReceiveCommand();
			break;
#endif
#if EmADB
		case kICT_ADB_NewState:
			ADB_DoNewState();
			break;
#endif
#if EmPMU
		case kICT_PMU_Task:
			PMU_DoTask();
			break;
#endif
		case kICT_VIA1_Timer1Check:
			VIA1_DoTimer1Check();
			break;
		case kICT_VIA1_Timer2Check:
			VIA1_DoTimer2Check();
			break;
#if EmVIA2
		case kICT_VIA2_Timer1Check:
			VIA2_DoTimer1Check();
			break;
		case kICT_VIA2_Timer2Check:
			VIA2_DoTimer2Check();
			break;
#endif
		default:
			ReportAbnormal("unknown taskid in ICT_DoTask");
			break;
	}
}

#ifdef _VIA_Debug
#include <stdio.h>
#endif

LOCALVAR blnr ICTactive[kNumICTs];
LOCALVAR iCountt ICTwhen[kNumICTs];

LOCALPROC ICT_Zap(void)
{
	int i;

	for (i = 0; i < kNumICTs; i++) {
		ICTactive[i] = falseblnr;
	}
}

LOCALPROC InsertICT(int taskid, iCountt when)
{
	ICTwhen[taskid] = when;
	ICTactive[taskid] = trueblnr;
}

LOCALVAR iCountt NextiCount = 0;

GLOBALFUNC iCountt GetCuriCount(void)
{
	return NextiCount - GetInstructionsRemaining();
}

GLOBALPROC ICT_add(int taskid, ui5b n)
{
	/* n must be > 0 */
	ui5b x = GetInstructionsRemaining();
	ui5b when = NextiCount - x + n;

#ifdef _VIA_Debug
	fprintf(stderr, "ICT_add: %d, %d, %d\n", when, taskid, n);
#endif
	InsertICT(taskid, when);

	if (x > n) {
		SetInstructionsRemaining(n);
		NextiCount = when;
	}
}

LOCALPROC ICT_DoCurrentTasks(void)
{
	int i;

	for (i = 0; i < kNumICTs; i++) {
		if (ICTactive[i]) {
			if (ICTwhen[i] == NextiCount) {
				ICTactive[i] = falseblnr;
#ifdef _VIA_Debug
				fprintf(stderr, "doing task %d, %d\n", NextiCount, i);
#endif
				ICT_DoTask(i);

				/*
					A Task may set the time of
					any task, including itself.
					But it cannot set any task
					to execute immediately, so
					one pass is sufficient.
				*/
			}
		}
	}
}

LOCALFUNC ui5b ICT_DoGetNext(ui5b maxn)
{
	int i;
	ui5b v = maxn;

	for (i = 0; i < kNumICTs; i++) {
		if (ICTactive[i]) {
			ui5b d = ICTwhen[i] - NextiCount;
			/* at this point d must be > 0 */
			if (d < v) {
#ifdef _VIA_Debug
				fprintf(stderr, "coming task %d, %d, %d\n", NextiCount, i, d);
#endif
				v = d;
			}
		}
	}
	return v;
}

GLOBALPROC m68k_go_nInstructions_1(ui5b n)
{
	ui5b n2;
	ui5b StopiCount = NextiCount + n;
	do {
		ICT_DoCurrentTasks();
		n2 = ICT_DoGetNext(n);
		NextiCount += n2;
		m68k_go_nInstructions(n2);
		n = StopiCount - NextiCount;
	} while (n != 0);
}

GLOBALPROC Memory_Reset(void)
{
	MemOverlay = 1;
	SetUpMemBanks();
	ICT_Zap();
}