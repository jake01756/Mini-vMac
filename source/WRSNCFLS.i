/*
	WRBGCFLS.i
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
	WRite SuN C specific FiLeS
*/

#pragma segment BashGccSupport

static void WriteSncCOptions(void)
{
	WriteCStrToDestFile(" -c -v -fd -xstrconst");
	if (gbo_dbg != gbk_dbg_on) {
		WriteCStrToDestFile(" -xO4 -xspace -Qn");
	} else {
		WriteCStrToDestFile(" -g");
	}
}

LOCALPROC WriteSncMakeFile(void)
{
	WriteDestFileLn("# make file generated by gryphel build system");

	WriteBlankLineToDestFile();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("mk_COptions =");
	WriteSncCOptions();
	WriteEndDestFileLn();
	WriteBlankLineToDestFile();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("TheDefaultOutput : ");
	Write_machobinpath_ToDestFile();
	WriteEndDestFileLn();

	WriteBlankLineToDestFile();
	vCheckWriteDestErr(DoAllSrcFilesWithSetup(DoSrcFileMakeCompile));
	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	WriteCStrToDestFile("ObjFiles = ");
	WriteBackSlashToDestFile();
	WriteEndDestFileLn();
	++DestFileIndent;
		DoAllSrcFilesStandardMakeObjects();
		WriteBlankLineToDestFile();
	--DestFileIndent;
	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	Write_machobinpath_ToDestFile();
	WriteCStrToDestFile(" : $(ObjFiles)");
	WriteEndDestFileLn();
	++DestFileIndent;
		WriteBgnDestFileLn();
		WriteCStrToDestFile("cc");
		if (gbo_dbg != gbk_dbg_on) {
			WriteCStrToDestFile(" -s -Qn -mr");
		}
		WriteCStrToDestFile(" \\");
		WriteEndDestFileLn();
		++DestFileIndent;
			WriteBgnDestFileLn();
			WriteCStrToDestFile("-o ");
			WriteQuoteToDestFile();
			Write_machobinpath_ToDestFile();
			WriteQuoteToDestFile();

			WriteCStrToDestFile(" -L/usr/X11R6/lib -lX11");
#if 0
			if (gbk_targfam_slrs == gbo_targfam) {
				WriteCStrToDestFile(" -lposix4");
			}
			if (MySoundEnabled) {
				WriteCStrToDestFile(" -lasound");
			}
#endif
			WriteCStrToDestFile(" \\");
			WriteEndDestFileLn();
			WriteDestFileLn("$(ObjFiles)");
		--DestFileIndent;
		if (gbo_dbg == gbk_dbg_off) {
			if (cur_ide == gbk_ide_xcd) {
				WriteBgnDestFileLn();
				WriteCStrToDestFile("strip -u -r \"");
				Write_machobinpath_ToDestFile();
				WriteCStrToDestFile("\"");
				WriteEndDestFileLn();
			}
		}
	--DestFileIndent;

	WriteBlankLineToDestFile();
	WriteDestFileLn("clean :");
	++DestFileIndent;
		WriteDestFileLn("rm -f $(ObjFiles)");
		WriteRmFile(WriteAppNamePath);
	--DestFileIndent;
}

LOCALFUNC tMyErr WriteSncSpecificFiles(void)
{
	return WriteADestFile(&OutputDirR,
		"Makefile", "", WriteSncMakeFile);
}
