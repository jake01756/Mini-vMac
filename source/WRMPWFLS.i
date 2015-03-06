/*
	WRMPWFLS.i
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
	WRite Macintosh Programmer's Workshop specific FiLeS
*/


#pragma segment MPWSupport

LOCALPROC WriteLONGGLUEContents(void)
{
	++DestFileIndent;
		WriteDestFileLn("CASE ON");
		WriteBlankLineToDestFile();
		WriteDestFileLn("IMPORT long_main: CODE ");
		WriteBlankLineToDestFile();
	--DestFileIndent;
	WriteDestFileLn("main PROC EXPORT");
	++DestFileIndent;
		WriteBlankLineToDestFile();
		WriteDestFileLn("JMP (long_main).L");
		WriteBlankLineToDestFile();
		WriteDestFileLn("ENDP");
		WriteBlankLineToDestFile();
		WriteDestFileLn("END");
	--DestFileIndent;
}

LOCALPROC WriteLongGlueObjName(void)
{
	WriteCStrToDestFile("LONGGLUE.o");
}

LOCALPROC WriteLongGlueObjPath(void)
{
	WriteFileInDirToDestFile0(Write_obj_d_ToDestFile,
		WriteLongGlueObjName);
}

LOCALPROC WriteLongGlueSourceName(void)
{
	WriteCStrToDestFile("LONGGLUE.S");
}

LOCALPROC WriteLongGlueSourcePath(void)
{
	WriteFileInDirToDestFile0(Write_src_d_ToDestFile,
		WriteLongGlueSourceName);
}

LOCALPROC DoLongGlueMakeCompileDeps(void)
{
	WriteMakeDependFile(WriteLongGlueSourcePath);
}

LOCALPROC DoLongGlueMakeCompileBody(void)
{
	WriteBgnDestFileLn();
	WriteCStrToDestFile("Asm");
	WritePathArgInMakeCmnd(WriteLongGlueSourcePath);
	WriteCStrToDestFile(" -o");
	WritePathArgInMakeCmnd(WriteLongGlueObjPath);
	WriteCStrToDestFile(" -model far");
	WriteEndDestFileLn();
}

LOCALPROC DoSrcFileMPWMakeObjects(void)
{
	WriteBgnDestFileLn();
	WriteQuoteToDestFile();
	WriteSrcFileObjPath();
	WriteQuoteToDestFile();
	WriteCStrToDestFile(" \266");
	WriteEndDestFileLn();
}

static void WriteMPWCOptions(blnr fast)
{
	if (gbo_cpufam == gbk_cpufam_68k) {
		WriteCStrToDestFile(" -proto strict -w 17 -align mac68k -b");
		if (cur_targ == gbk_targ_mfpu) {
			WriteCStrToDestFile(" -mc68020 -mc68881 -elems881");
		}
		if (gbo_dbg == gbk_dbg_off) {
			WriteCStrToDestFile(" -mbg off");
		}
		WriteCStrToDestFile(" -model farCode");
	} else if (gbo_cpufam == gbk_cpufam_ppc) {
		WriteCStrToDestFile(" -proto strict -w 17");
		if (gbo_dbg != gbk_dbg_off) {
			WriteCStrToDestFile(" -traceback");
		}
#if UseAlignMac68k
		WriteCStrToDestFile(" -align mac68k");
#endif
	}
	if (gbo_dbg != gbk_dbg_on) {
		if (fast) {
			if (gbo_cpufam == gbk_cpufam_68k) {
				WriteCStrToDestFile(" -opt speed");
			} else if (gbo_cpufam == gbk_cpufam_ppc) {
				WriteCStrToDestFile(" -opt speed");
			}
		} else {
			if (gbo_cpufam == gbk_cpufam_68k) {
				WriteCStrToDestFile(" -opt space");
			} else if (gbo_cpufam == gbk_cpufam_ppc) {
				WriteCStrToDestFile(" -opt size");
				/* this may not be reliable? */
			}
		}
	}
}

LOCALPROC WriteMainRsrcObjDeps(void)
{
	WriteMakeDependFile(WriteMainRsrcSrcPath);
}

LOCALPROC WriteMainRsrcObjMPWbody(void)
{
	WriteBgnDestFileLn();
	WriteCStrToDestFile("Rez -t rsrc -c RSED -i \"{RIncludes}\" \"");
	WriteMainRsrcSrcPath();
	WriteCStrToDestFile("\" -o \"");
	WriteMainRsrcObjPath();
	WriteCStrToDestFile("\"");
	WriteEndDestFileLn();
}

LOCALPROC WriteMPWMakeFile(void)
{
	WriteDestFileLn("# make file generated by gryphel build system");

	WriteBlankLineToDestFile();
#if AsmSupported
	if (HaveAsm) {
		if (gbo_cpufam == gbk_cpufam_68k) {
			WriteDestFileLn("mk_AOptions = -wb -model far");
		} else {
			WriteDestFileLn("mk_AOptions = ");
		}
	}
#endif

	WriteBgnDestFileLn();
	WriteCStrToDestFile("mk_COptions =");
	WriteMPWCOptions(falseblnr);
	WriteEndDestFileLn();
	WriteBlankLineToDestFile();

	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	WriteCStrToDestFile("TheDefaultOutput \304");
	WriteMakeDependFile(Write_machobinpath_ToDestFile);
	WriteEndDestFileLn();

	WriteBlankLineToDestFile();

	if (gbo_cpufam == gbk_cpufam_68k) {
		WriteMakeRule(WriteLongGlueObjPath,
			DoLongGlueMakeCompileDeps,
			DoLongGlueMakeCompileBody);
	}

	vCheckWriteDestErr(DoAllSrcFilesWithSetup(DoSrcFileMakeCompile));
	WriteBlankLineToDestFile();
	WriteDestFileLn("ObjFiles = \266");
	++DestFileIndent;
		vCheckWriteDestErr(
			DoAllSrcFilesSortWithSetup(DoSrcFileMPWMakeObjects));
		WriteBlankLineToDestFile();
	--DestFileIndent;

	if (HaveMacBundleApp) {
		WriteBlankLineToDestFile();
		WriteMakeRule(Write_machoAppIconPath,
			Write_tmachoShellDeps,
			Write_tmachoShell);
	}

	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	WriteCStrToDestFile("\"");
	Write_machobinpath_ToDestFile();
	WriteCStrToDestFile("\" \304");
	WriteCStrToDestFile(" {ObjFiles}");
	if (HaveMacBundleApp) {
		WriteMakeDependFile(Write_machoAppIconPath);
	} else {
		WriteMakeDependFile(WriteMainRsrcObjPath);
	}
	if (gbo_cpufam == gbk_cpufam_68k) {
		WritePathArgInMakeCmnd(WriteLongGlueObjPath);
	}
	WriteEndDestFileLn();
	++DestFileIndent;
		if (HaveMacRrscs) {
			WriteBgnDestFileLn();
			WriteCStrToDestFile("Duplicate -y \"");
			WriteMainRsrcObjPath();
			WriteCStrToDestFile("\" \"");
			Write_machobinpath_ToDestFile();
			WriteCStrToDestFile("\"");
			WriteEndDestFileLn();
		}

		WriteBgnDestFileLn();
			if (gbo_cpufam == gbk_cpufam_68k) {
				WriteCStrToDestFile("Link");
				if (gbo_dbg == gbk_dbg_off) {
					WriteCStrToDestFile(" -rn");
				}
				WriteCStrToDestFile(
					" -model far -sg Main"
					"=STDCLIB,SANELIB,CSANELib,SADEV,STDIO");
			} else if (gbo_cpufam == gbk_cpufam_ppc) {
				WriteCStrToDestFile("PPCLink");
			}

			if (cur_targ == gbk_targ_carb) {
				WriteCStrToDestFile(" -m main");
			}
			WriteCStrToDestFile(" -t APPL -c ");
			WriteCStrToDestFile(kMacCreatorSig);
			WriteCStrToDestFile(" \266");
		WriteEndDestFileLn();

		++DestFileIndent;

			WriteDestFileLn("{ObjFiles} \266");

			if (cur_targ == gbk_targ_carb) {
				WriteDestFileLn("\"{SharedLibraries}CarbonLib\" \266");
#if UseOpenGLinOSX
				WriteDestFileLn(
					"\"{SharedLibraries}OpenGLLibraryStub\" \266");
#endif
				WriteDestFileLn("\"{PPCLibraries}PPCToolLibs.o\" \266");
				WriteDestFileLn("\"{PPCLibraries}PPCCRuntime.o\" \266");
				WriteDestFileLn("\"{SharedLibraries}StdCLib\" \266");
			} else if (cur_targ == gbk_targ_mppc) {
				WriteDestFileLn("\"{PPCLibraries}PPCToolLibs.o\" \266");
				WriteDestFileLn("\"{PPCLibraries}PPCCRuntime.o\" \266");
				WriteDestFileLn("\"{PPCLibraries}StdCRuntime.o\" \266");
				WriteDestFileLn(
					"\"{SharedLibraries}InterfaceLib\" \266");
				WriteDestFileLn("\"{SharedLibraries}MathLib\" \266");
				WriteDestFileLn("\"{SharedLibraries}StdCLib\" \266");
				WriteDestFileLn("-weaklib AppearanceLib \266");
				WriteDestFileLn(
					"\"{SharedLibraries}AppearanceLib\" \266");
				WriteDestFileLn("-weaklib MenusLib \266");
				WriteDestFileLn("\"{SharedLibraries}MenusLib\" \266");
				WriteDestFileLn("-weaklib NavigationLib \266");
				WriteDestFileLn(
					"\"{SharedLibraries}NavigationLib\" \266");
				WriteDestFileLn("-weaklib DragLib \266");
				WriteDestFileLn("\"{SharedLibraries}DragLib\" \266");
				WriteDestFileLn("-weaklib WindowsLib \266");
				WriteDestFileLn("\"{SharedLibraries}WindowsLib\" \266");
			} else if (cur_targ == gbk_targ_m68k) {
				WriteDestFileLn("\"{Libraries}Interface.o\" \266");
				WriteDestFileLn("\"{Libraries}Navigation.o\" \266");
				WriteDestFileLn("\"{Libraries}MacRuntime.o\" \266");
				/* WriteDestFileLn("\"{Libraries}MathLib.o\" \266"); */
			} else if (cur_targ == gbk_targ_mfpu) {
				WriteDestFileLn("\"{Libraries}Interface.o\" \266");
				WriteDestFileLn("\"{Libraries}Navigation.o\" \266");
				WriteDestFileLn("\"{Libraries}MacRuntime.o\" \266");
				/*
					WriteDestFileLn("\"{Libraries}MathLib881.o\" \266");
				*/
			}
			if (gbo_cpufam == gbk_cpufam_68k) {
				WriteBgnDestFileLn();
				WriteQuoteToDestFile();
				WriteLongGlueObjPath();
				WriteQuoteToDestFile();
				WriteCStrToDestFile(" \266");
				WriteEndDestFileLn();
			}

			WriteBgnDestFileLn();
			WriteCStrToDestFile("-o");
			WritePathArgInMakeCmnd(Write_machobinpath_ToDestFile);
			WriteEndDestFileLn();
		--DestFileIndent;

		WriteBgnDestFileLn();
		WriteCStrToDestFile("SetFile -d . -m .");
		if (! HaveMacBundleApp) {
			WriteCStrToDestFile(" -a B");
		}
		WriteCStrToDestFile(" \"");
		Write_machobinpath_ToDestFile();
		WriteCStrToDestFile("\"");
		WriteEndDestFileLn();
	--DestFileIndent;

	if (HaveMacRrscs) {
		WriteBlankLineToDestFile();
		WriteMakeRule(WriteMainRsrcObjPath,
			WriteMainRsrcObjDeps, WriteMainRsrcObjMPWbody);
	}

	WriteBlankLineToDestFile();
	WriteDestFileLn("clean \304");
	++DestFileIndent;
		WriteDestFileLn("Delete -i {ObjFiles}");
		if (HaveMacBundleApp) {
			WriteRmDir(WriteAppNamePath);
		} else {
			WriteRmFile(WriteAppNamePath);
			WriteRmFile(WriteMainRsrcObjPath);
		}
	--DestFileIndent;
	WriteBlankLineToDestFile();
}

LOCALFUNC tMyErr WriteMPWSpecificFiles(void)
{
	tMyErr err;

	if ((gbo_cpufam != gbk_cpufam_68k) ||
		(noErr == (err = WriteADestFile(&SrcDirR, "LONGGLUE", ".S",
			WriteLONGGLUEContents)))
		)
	if ((! HaveMacBundleApp) || (noErr == (err = WritePListData())))
	if (noErr == (err = WriteADestFile(&OutputDirR,
		"Makefile", "", WriteMPWMakeFile)))
	{
		/* ok */
	}

	return err;
}