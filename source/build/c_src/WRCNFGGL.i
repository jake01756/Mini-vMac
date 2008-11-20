/*
	WRCNFGGL.i
	Copyright (C) 2007 Paul C. Pratt

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
	WRite "CNFGGLob.h"
*/


#ifndef NeedIntFormatInfo
#define NeedIntFormatInfo 0
#endif

LOCALPROC WriteCommonCNFGGLOB(void)
{
	if (WriteOpenDestFile(&SrcDirR, "CNFGGLOB", ".h")) { /* C Configuration file */

	WriteDestFileLn("/*");
	++DestFileIndent;
		WriteDestFileLn("configuration options used by both platform specific");
		WriteDestFileLn("and platform independent code");
	--DestFileIndent;
	WriteDestFileLn("*/");

	WriteBlankLineToDestFile();
	WriteDestFileLn("/* adapt to current compiler/host processor */");

#if support_ide_mw8
	if (cur_ide == gbk_ide_mw8) {
		WriteDestFileLn("/* make sure this is correct CNFGGLOB */");

		WriteCheckPreDef("__MWERKS__");
		switch (gbo_cpufam) {
			case gbk_cpufam_68k:
				WriteCheckPreDef("__MC68K__");
				if (cur_targ == gbk_targ_mfpu) {
					WriteCheckPreDef("__MC68881__");
				} else {
					WriteCheckPreNDef("__MC68881__");
				}
				break;
			case gbk_cpufam_ppc:
				WriteCheckPreDef("__POWERPC__");
				break;
			case gbk_cpufam_x86:
				WriteCheckPreDef("__INTEL__");
				break;
		}
	}
#endif

	WriteBlankLineToDestFile();

#if NeedIntFormatInfo
	WriteCompCondBool("MostSigByteFirst",
		(gbo_cpufam == gbk_cpufam_68k) || (gbo_cpufam == gbk_cpufam_ppc));
	WriteCompCondBool("LeastSigByteFirst",
		gbo_cpufam == gbk_cpufam_x86);
	WriteCompCondBool("TwosCompSigned",
		(gbo_cpufam == gbk_cpufam_68k) || (gbo_cpufam == gbk_cpufam_ppc) || (gbo_cpufam == gbk_cpufam_x86));
#endif

	if ((gbo_cpufam == gbk_cpufam_68k) || (gbo_cpufam == gbk_cpufam_ppc)) {
		WriteDestFileLn("#define BigEndianUnaligned 1");
		WriteDestFileLn("#define LittleEndianUnaligned 0");
	} else if (gbo_cpufam == gbk_cpufam_x86) {
		WriteDestFileLn("#define BigEndianUnaligned 0");
		WriteDestFileLn("#define LittleEndianUnaligned 1");
	} else {
		WriteDestFileLn("#define BigEndianUnaligned 0");
		WriteDestFileLn("#define LittleEndianUnaligned 0");
	}

	if (gbo_cpufam == gbk_cpufam_68k) {
		WriteDestFileLn("#define HaveCPUfamM68K 1");
	}

	if ((cur_ide == gbk_ide_bgc) || (cur_ide == gbk_ide_xcd) || (cur_ide == gbk_ide_snc)) {
		WriteDestFileLn("#define MayInline inline");
	} else
#if support_ide_mw8
	if (cur_ide == gbk_ide_mw8) {
		WriteDestFileLn("#define MayInline __inline__");
	} else
#endif
#if SupportWinIDE
	if (cur_ide == gbk_ide_msv) {
		if (ide_vers >= 6000) {
			WriteDestFileLn("#define MayInline __forceinline");
		} else {
			WriteDestFileLn("#define MayInline __inline");
		}
	} else
#endif
	{
		WriteDestFileLn("#define MayInline");
	}

	if ((cur_ide == gbk_ide_bgc) || (cur_ide == gbk_ide_xcd)) {
		WriteDestFileLn("#define MayNotInline __attribute__((noinline))");
	} else
#if SupportWinIDE
	if ((cur_ide == gbk_ide_msv) && (ide_vers >= 7000)) {
		WriteDestFileLn("#define MayNotInline __declspec(noinline)");
	} else
#endif
	{
		WriteDestFileLn("#define MayNotInline");
	}

#if SupportWinIDE
	if (cur_ide == gbk_ide_msv) {
		WriteBlankLineToDestFile();
		WriteDestFileLn("/* --- set up compiler options --- */");
		WriteBlankLineToDestFile();
		WriteDestFileLn("/* ignore integer conversion warnings */");
		WriteDestFileLn("#pragma warning(disable : 4244 4761 4018 4245 4024 4305)");
		WriteBlankLineToDestFile();
		WriteDestFileLn("/* ignore unused inline warning */");
		WriteDestFileLn("#pragma warning(disable : 4514 4714)");
#if 0
		WriteBlankLineToDestFile();
		WriteDestFileLn("/* ignore type redefinition warning */");
		WriteDestFileLn("#pragma warning(disable : 4142)");
#endif
		WriteBlankLineToDestFile();
		WriteDestFileLn("/* ignore unary minus operator applied to unsigned type warning */");
		WriteDestFileLn("#pragma warning(disable : 4146)");
	}
#endif
#if support_ide_mw8
	if (cur_ide == gbk_ide_mw8) {
		if (gbo_dbg != gbk_dbg_on) {
			WriteBlankLineToDestFile();
			WriteDestFileLn("#ifdef OptForSpeed");
			WriteDestFileLn("#pragma optimize_for_size off");
			WriteDestFileLn("#endif");
		}
	}
#endif

	WriteCompCondBool("SmallGlobals", gbo_cpufam == gbk_cpufam_68k);

	if ((cur_ide == gbk_ide_bgc) || (cur_ide == gbk_ide_xcd)) {
		WriteDestFileLn("#define cIncludeUnused 0");
	} else
#if SupportWinIDE
	if (cur_ide == gbk_ide_lcc) {
		WriteBlankLineToDestFile();
		WriteDestFileLn("#define cIncludeUnused 0");
		WriteDestFileLn("#define UnusedParam(x)");
	} else if (cur_ide == gbk_ide_dvc) {
		WriteBlankLineToDestFile();
		WriteDestFileLn("#define cIncludeUnused 0");
	} else
#endif
	{
	}

	if (gbo_dbg != gbk_dbg_off) {
		WriteBlankLineToDestFile();
		WriteDestFileLn("#define Debug 1");
	}

	WriteBlankLineToDestFile();
	WriteDestFileLn("/* version and other info to display to user */");

	WriteBlankLineToDestFile();

	WriteCDefQuote("kStrAppName", WriteStrAppUnabrevName);
	WriteCDefQuote("kAppVariationStr", WriteAppVariationStr);
	WriteCDefQuote("kStrCopyrightYear", WriteAppCopyrightYearStr);
	WriteCDefQuote("kMaintainerName", WriteMaintainerName);
	WriteCDefQuote("kStrHomePage", WriteHomePage);

	WriteBlankLineToDestFile();
	WriteDestFileLn("/* capabilities provided by platform specific code */");

	WriteBlankLineToDestFile();

	WriteCompCondBool("MySoundEnabled", MySoundEnabled);

#ifdef Have_SPCNFGGL
	WriteAppSpecificCNFGGLOBoptions();
#endif

	WriteCloseDestFile();
	}
}