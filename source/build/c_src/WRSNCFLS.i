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
	WRite SuN C specific FiLes
*/

#pragma segment BashGccSupport

LOCALPROC DoSrcFileSncMakeObjects(void)
{
	WriteBgnDestFileLn();
	WriteSrcFileObjPath();
	WriteCStrToDestFile(" ");
	WriteBackSlashToDestFile();
	WriteEndDestFileLn();
}

static void WriteSncCOptions(void)
{

	WriteCStrToDestFile(" -c -v -fd -xstrconst");
	if (gbo_dbg != gbk_dbg_on) {
		WriteCStrToDestFile(" -xO4 -xspace -Qn");
	} else {
		WriteCStrToDestFile(" -g");
	}
}

static void WriteSncSpecificFiles(void)
{
	if (WriteOpenDestFile(&OutputDirR, "Makefile", "")) { /* Make file */

	WriteDestFileLn("# make file generated by gryphel build system");

	WriteBlankLineToDestFile();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("mk_COptions =");
	WriteSncCOptions();
	WriteEndDestFileLn();
	WriteBlankLineToDestFile();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("TheDefaultOutput : ");
	if (CurPackageOut) {
		WriteAppBinTgzPath();
	} else {
		Write_machobinpath_ToDestFile();
	}
	WriteEndDestFileLn();

	WriteBlankLineToDestFile();
	DoAllSrcFilesWithSetup(DoSrcFileMakeCompile);
	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	WriteCStrToDestFile("ObjFiles = ");
	WriteBackSlashToDestFile();
	WriteEndDestFileLn();
	++DestFileIndent;
		DoAllSrcFilesWithSetup(DoSrcFileSncMakeObjects);
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
			switch (cur_targ) {
				case gbk_targ_slrs:
				case gbk_targ_sl86:
					WriteCStrToDestFile(" -lposix4");
					break;
				default:
					break;
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

	if (CurPackageOut) {
		WriteBlankLineToDestFile();
		WriteBgnDestFileLn();
		WriteAppBinTgzPath();
		WriteCStrToDestFile(" : ");
		Write_machobinpath_ToDestFile();
		WriteEndDestFileLn();
		++DestFileIndent;
			WriteBgnDestFileLn();
			WriteCStrToDestFile("touch -am -r README.txt");
			WritePathArgInMakeCmnd(WriteAppNamePath);
			WriteEndDestFileLn();

			WriteBgnDestFileLn();
			WriteCStrToDestFile("tar -cf");
			WritePathArgInMakeCmnd(WriteAppBinTarPath);
			WritePathArgInMakeCmnd(WriteAppUnabrevPath);
			WriteEndDestFileLn();

			WriteBgnDestFileLn();
			WriteCStrToDestFile("touch -am -r README.txt");
			WritePathArgInMakeCmnd(WriteAppBinTarPath);
			WriteEndDestFileLn();

			WriteBgnDestFileLn();
			WriteCStrToDestFile("gzip <");
			WritePathArgInMakeCmnd(WriteAppBinTarPath);
			WriteCStrToDestFile(" >");
			WritePathArgInMakeCmnd(WriteAppBinTgzPath);
			WriteEndDestFileLn();

			WriteRmFile(WriteAppBinTarPath);

			WriteBgnDestFileLn();
			switch (cur_targ) {
				case gbk_targ_slrs:
				case gbk_targ_sl86:
					WriteCStrToDestFile("digest -a md5");
					break;
				default:
					if (cur_ide == gbk_ide_xcd) {
						WriteCStrToDestFile("md5 -r");
					} else {
						WriteCStrToDestFile("md5sum");
					}
					break;
			}
			WritePathArgInMakeCmnd(WriteAppBinTgzPath);
			WriteCStrToDestFile(" >");
			WritePathArgInMakeCmnd(WriteCheckSumFilePath);
			WriteEndDestFileLn();

			switch (cur_targ) {
				case gbk_targ_slrs:
				case gbk_targ_sl86:
					WriteBgnDestFileLn();
					WriteCStrToDestFile("echo \" ");
					WriteAppBinTgzName();
					WriteCStrToDestFile("\" >>");
					WritePathArgInMakeCmnd(WriteCheckSumFilePath);
					WriteEndDestFileLn();
					break;
				default:
					break;
			}
		--DestFileIndent;
	}

	WriteBlankLineToDestFile();
	WriteDestFileLn("clean :");
	++DestFileIndent;
		WriteDestFileLn("rm -f $(ObjFiles)");
		WriteRmFile(WriteAppNamePath);

		if (CurPackageOut) {
			WriteRmFile(WriteAppBinTgzPath);
			WriteRmFile(WriteCheckSumFilePath);
		}
	--DestFileIndent;

	WriteCloseDestFile();
	}
}