#include "runfand.h"

#include <Windows.h>
#include "base.h"
#include "legacy.h"
#include "../Common/pstring.h"
#include "../Common/compare.h"
#include "../pascal/real48.h"
#include "OldDrivers.h"
#include "access.h"
#include "CfgFile.h"
#include "compile.h"
#include "FileD.h"
#include "GlobalVariables.h"
#include "oaccess.h"
#include "obaseww.h"
#include "rdfildcl.h"
#include "runproj.h"
#include "Cfg.h"
#include "wwmenu.h"
#include "wwmix.h"
#include "../Editor/OldEditor.h"
#include "../Editor/runedi.h"
#include "../fandio/directory.h"
#include "../fandio/files.h"
#include "../Common/textfunc.h"


void ScrGraphMode(bool Redraw, WORD OldScrSeg)
{
	// graph mode not supported

	//void* p; void* p1;
	//WORD* pofs = (WORD*)p;
	//WORD sz, cr, i; short err;
	//pstring s(4);
	//if ((VideoCard == viCga) || (TxtCols != 80) || (TxtRows != 25)) RunError(643);
	//DoneMouseEvents();
	//bool b = Crs.Enabled;
	//CrsHide();
	//short n = 80 * 25 * 2;
	//if (OldScrSeg != 0) p = ptr(OldScrSeg, 0);
	//else { AlignParagraph(); p = GetStore(n); }
	//Move(ptr(ScrSeg, 0), p, n);
	//if (OldScrSeg != 0) SetGraphMode(GraphMode);
	//else {
	//	if ((VideoCard == viVga) && (GraphDriver != EGAMono)) {
	//		if (BGIReload) {
	//			if (fonts.VFont == foLatin2) ResFile.Get(Vga8x19L, FontArr);
	//			else ResFile.Get(Vga8x19K, FontArr);
	//		}
	//		GrBytesPerChar = 19;
	//	}
	//	else {
	//		if (BGIReload) {
	//			if (fonts.VFont == foLatin2) ResFile.Get(Ega8x14L, FontArr);
	//			else ResFile.Get(Ega8x14K, FontArr);
	//		}
	//		GrBytesPerChar = 14;
	//	}
	//	GrBytesPerLine = 80 * GrBytesPerChar;
	//	if (VideoCard == viHercules) n = BgiHerc;
	//	else n = BgiEgaVga;
	//	if (BGIReload) ResFile.Get(n, BGIDriver);
	//	if (RegisterBGIDriver(BGIDriver) < 0) RunError(838);
	//	GetMem(p1, 7000); FreeMem(p1, 7000); FreeList = HeapPtr;
	//	InitGraph(GraphDriver, GraphMode, ""); FreeList = nullptr;
	//	err = GraphResult;
	//	if (err != grOK) { str(err,3,s); SetMsgPar(s); RunError(201); }
	//}
	//IsGraphMode = true; ScrSeg = seg(*p); CrsIntrInit();
	//if (Redraw) ScrPopToGraph(0, 0, TxtCols, TxtRows, p, TxtCols);
	//InitMouseEvents(); if (b) CrsShow();
	//if (BGIReload) {
	//	if (fonts.VFont == foLatin2) {
	//		ResFile.Get(ChrLittLat, BGILittFont); ResFile.Get(ChrTripLat, BGITripFont);
	//	}
	//	else {
	//		ResFile.Get(ChrLittKam, BGILittFont); ResFile.Get(ChrTripKam, BGITripFont);
	//	}
	//}
	//if (RegisterBGIFont(BGILittFont) < 0) RunError(838);
	//if (RegisterBGIFont(BGITripFont) < 0) RunError(838);
}

WORD ScrTextMode(bool Redraw, bool Switch)
{
	//bool b;
	//if (!IsGraphMode) return;
	//DoneMouseEvents();
	//b = Crs.Enabled; CrsHide(); CrsIntrDone();
	//if (Switch) RestoreCrtMode; else CloseGraph();
	//IsGraphMode = false;
	//LoadVideoFont();
	//if (Redraw) Move(ptr(ScrSeg, 0), ptr(video.address, 0), 80 * 25 * 2);
	//auto result = ScrSeg;
	//ScrSeg = video.address;
	//InitMouseEvents();
	//CrsShow(); /*is visible*/
	//if (!b) CrsHide();
	//return result;

	return 0;
}

void InitAccess()
{
	ResetCompilePars();
	SpecFDNameAllowed = false;
	FillChar(&XWork, sizeof(XWork), 0);
}

void RdCFG()
{
	// open CFG file
	CfgFile cfgFile;
	CPath = MyFExpand("FAND.CFG", "FANDCFG");
	cfgFile.Open(CPath);

	cfgFile.ReadSpec(spec);
	cfgFile.ReadVideoAndColors(video, StartMode, VideoCard, screen, TxtCols, TxtRows); // nacteni konfigurace (VGA + barvy)
	cfgFile.ReadFonts(fonts);          // nacteni fontu
	cfgFile.ReadCodeTables(); 

	cfgFile.RdPrinter(prMax, printer);
	SetCurrPrinter(0);

	cfgFile.RdWDaysTab(&WDaysTab, NWDaysTab, WDaysFirst, WDaysLast);

	cfgFile.Close();
}

void CompileHelpCatDcl()
{
	FileDRoot = nullptr;
	Chpt = FileDRoot;
	CRdb = nullptr;

	// process help
	std::string help_definition = ReadMessage(56);
	SetInpStr(help_definition);
#if defined (FandRunV)
	RdFileD("UFANDHLP", FileType::FAND16, "");
#else
	RdFileD("FANDHLP", FileType::FAND16, "");
#endif
	HelpFD = CFile;

	// process catalog
	std::string catalog_definition = ReadMessage(52);
	SetInpStr(catalog_definition);
	RdFileD("Catalog", FileType::CAT, "");
	
	FileDRoot = nullptr;
	Chpt = FileDRoot;
	CatFD = new Catalog(CFile);
}

bool SetTopDir(std::string& p, std::string& n)
{
	std::string e;
	bool result = false;
	try {
		FSplit(FExpand(p), TopRdbDir, n, e);
		if (!IsIdentifStr(n)) {
			WrLLF10Msg(881);
			return result;
		}
		EditDRoot = nullptr;
		LinkDRoot.clear();
		FuncDRoot = nullptr;
		TopDataDir = GetEnv("FANDDATA");
		DelBackSlash(TopRdbDir);
		DelBackSlash(TopDataDir);
		if (!TopDataDir.empty()) {
			TopDataDir = FExpand(TopDataDir);
		}
		ChDir(TopRdbDir);
		if (IOResult() != 0) {
			SetMsgPar(p);
			WrLLF10Msg(703);
			return result;
		}
		CatFDName = n;
		//NewExit(Ovr(), er);
		//goto label1;
		CFile = CatFD->GetCatalogFile();
		OpenF(CFile, CPath, Exclusive);
		result = true;
	}
	catch (std::exception& e) {
		// TODO: log error
	}
	return result;
}

void RunRdb(std::string p)
{
	std::string n;
	if (!p.empty() && SetTopDir(p, n)) {
		wwmix ww;
		EditExecRdb(n, "main", nullptr, &ww);
		CFile = CatFD->GetCatalogFile();
		CFile->CloseFile();
	}
}

void SelectRunRdb(bool OnFace)
{
	wwmix ww;
	std::string p = ww.SelectDiskFile(".RDB", 34, OnFace);
	RunRdb(p);
}

void CallInstallRdb()
{
	wwmix ww;
	std::string p = ww.SelectDiskFile(".RDB", 35, true);
	std::string n;
	if ((!p.empty()) && SetTopDir(p, n)) {
		InstallRdb(n);
		CFile = CatFD->GetCatalogFile();
		CFile->CloseFile();
	}
}

void CallEditTxt()
{
	CPath = FExpand(CPath);
	CVol = "";
	std::string errMessage;
	std::vector<EdExitD*> emptyEdExit;
	EditTxtFile(nullptr, 'T', errMessage, emptyEdExit, 0, 0, nullptr, 0, "", 0, nullptr);
}

void SelectEditTxt(const std::string& ext, bool OnFace)
{
	wwmix ww;
	CPath = ww.SelectDiskFile(ext, 35, OnFace);
	if (CPath.empty()) return;
	CallEditTxt();
}

void InitRunFand()
{
	WORD n = 0, l = 0, err = 0, hourmin = 0;
	FILE* h = nullptr;
	std::string s;
	BYTE nb, sec = 0;
	short j, MsgNr;
	TMenuBoxS* mb = nullptr;
	int w = 0;
	void* p = nullptr;
	WORD xofs = 1;
	std::string txt;
	double r;


	ClrEvent(); // instead of InitDrivers();

	//ConsoleInit();
	//WasInitDrivers = true;
	InitAccess();

	FandDir = getDirectory(paramstr[0]);
	printf("FAND DIR:    %s\n", FandDir.c_str());

	const unsigned long maxDir = 260;
	char currentDir[maxDir];
	GetCurrentDirectory(maxDir, currentDir);
	printf("Current DIR: %s\n", currentDir);

	NonameStartFunction();
#ifdef FandDML
	InitDML();
#endif

	//NewExit(Ovr(), *er);
	// StackLimit += 256;
	OldPrTimeOut = PrTimeOut;
	//CallOpenFandFiles = OpenFandFiles;  // TODO: CallOpenFandFiles: procedure(FromDML:boolean);
	//CallCloseFandFiles = CloseFandFiles;  // TODO: CallCloseFandFiles: procedure(FromDML:boolean);
	video.CursOn = 0x0607; // {if exit before reading.CFG}
	keyboard.DeleteKeyBuf(); // KbdBuffer[0] = 0x0;
	F10SpecKey = 0;
	if (!GetEnv("DMLADDR").empty()) {
		printf("type 'exit' to return to FAND");
		wait();
		return; // puvodne wait;
	}

	WrkDir = GetEnv("FANDWORK");
	if (WrkDir.empty()) WrkDir = FandDir;
	AddBackSlash(WrkDir);
	s = WrkDir + "FANDWORK";
	printf("FANDWORK DIR: %s\n", s.c_str());
	FandWorkName = s + ".$$$";
	FandWorkXName = s + ".X$$";
	FandWorkTName = s + ".T$$";

	LANNode = 0;
	s = GetEnv("LANNODE");
	s = TrailChar(s, ' ');
	if (!s.empty()) {
		val(s, nb, err);
#ifndef FandRunV
		if (nb <= 3) {
#endif 
			if (err == 0) LANNode = nb;
#ifndef FandRunV
		}
#endif 
	}
	printf("LANNODE: %s\n", s.c_str());
	
	//h = ResFile.Handle;
	//ReadH(h, 2, &n);
	//if (n != ResVersion) {
	//	printf("FAND.RES incorr. version\n");
	//	system("pause");
	//	wait(); Halt(0);
	//}

	//for (int i = 0; i < FandFace; i++) {
	//	ReadH(h, sizeof(resFile.A->Pos), &resFile.A[i].Pos);
	//	ReadH(h, sizeof(resFile.A->Size), &resFile.A[i].Size);
	//}

	//// *** NACTENI INFORMACI O ZPRAVACH Z FAND.RES
	//ReadH(h, 2, &MsgIdxN);
	//for (int ii = 0; ii < MsgIdxN; ii++) {
	//	TMsgIdxItem newItem{ 0, 0, 0 };
	//	ReadH(h, 2, &newItem.Nr);    // WORD
	//	ReadH(h, 2, &newItem.Ofs);   // WORD
	//	ReadH(h, 1, &newItem.Count); // BYTE
	//	MsgIdx.push_back(newItem);
	//}
	//FrstMsgPos = PosH(h);
	//// *** konec ***
	resFile.ReadInfo();

	//std::vector<std::string> messages;
	//for (size_t iii = 0; iii < 10000; iii++) {
	//	ReadMessage(iii);
	//	if (MsgLine.length() == 26 && MsgLine.find("ve FAND.RES") == 15) {
	//		printf("%i, ", iii);
	//	}
	//	else {
	//		std::string s = MsgLine;
	//		for (int jj = 0; jj < s.length(); jj++) {
	//			if ((BYTE)s[jj] < ' ') {
	//				std::string xxxx = std::format("{:#x}", s[jj]);
	//				std::string hexString = "&#x" + xxxx.substr(2) + ";";
	//				s.replace(jj, 1, hexString);
	//			}
	//			else if (s[jj] == '<')
	//			{
	//				s.replace(jj, 1, "&lt;");
	//			}
	//			else if (s[jj] == '>')
	//			{
	//				s.replace(jj, 1, "&gt;");
	//			}
	//		}
	//		messages.push_back("<message ID=\"" + std::to_string(iii) + "\">" + s + "</message>" + "\r\n");
	//	}
	//}
	//FILE* hhh;
	//fopen_s(&hhh, "c:\\PCFAND\\messages.txt", "wb");
	//for (auto& s : messages) {
	//	fwrite(s.c_str(), 1, s.length(), hhh);
	//}
	//fclose(hhh);


	// NACTENI ZNAKU PRO 'ANO' A 'NE' - original je Y a N
	ReadMessage(50);
	AbbrYes = MsgLine[0];
	AbbrNo = MsgLine[1];

	RdCFG();

	// je nactena konfigurace -> reinicializace obrazovky
	screen.ReInit((short)TxtCols, (short)TxtRows);

	ProcAttr = screen.colors.uNorm;

	if (video.TxtRows != 0) {
		TxtRows = video.TxtRows;
	}

	CRdb = nullptr;
	for (int i = 0; i < FloppyDrives; i++) { MountedVol[i] = ""; }
	// Ww
	ss.Empty = true;
	ss.Pointto = nullptr;
	TxtEdCtrlUBrk = false;
	TxtEdCtrlF4Brk = false;
	InitMouseEvents();
	// Editor
	InitTxtEditor();

	WasInitPgm = true;
	CompileHelpCatDcl();

	OpenWorkH();
	OpenFANDFiles(false);

	if (paramstr.size() > 1 && !paramstr.at(1).empty() && paramstr.at(1) != "?") {
#ifndef FandRunV
		if (paramstr.size() > 2 && EquUpCase(paramstr[2], "D")) {
			IsTestRun = true;
			goto label0;
		}
		else
#endif
			if (paramstr.size() > 2 && EquUpCase(paramstr.at(2), "T")) {
				CPath = paramstr.at(1);
				if (copy(CPath, 1, 2) == "*.")
					SelectEditTxt(CPath.substr(1, 4), false);
				else CallEditTxt();
				return;
			}
			else {
			label0:
				//if (maska == podretez) {
				if (paramstr.at(1).substr(0, 2) == "*.") {
					SelectRunRdb(false);
				}
				else {
					RunRdb(paramstr.at(1));
				}
				if (IsTestRun) {
					IsTestRun = false;
				}
				else {
					return;
				}
			}
	}

	TextAttr = screen.colors.DesktopColor;
	screen.Window(1, 1, (BYTE)TxtCols, TxtRows - 1);
	WriteWFrame(WHasFrame + WDoubleFrame, "", "");
	screen.ScrClr(2, 2, TxtCols - 2, TxtRows - 13, (char)0xB1, TextAttr);
	screen.ScrClr(2, TxtRows - 11, TxtCols - 2, 10, (char)0xb2, TextAttr);

	std::string ResText = resFile.Get(FandFace - 1);

	xofs++;
	for (int ii = -11; ii <= -6; ii++) {
		std::string sPrint = ResText.substr(xofs, TxtCols - 2);
		screen.ScrWrStr(2, TxtRows + ii + 1, sPrint, TextAttr);
		xofs += 82;
	}
	TextAttr = screen.colors.mHili;
	screen.ScrClr(4, TxtRows - 3, TxtCols - 6, 1, ' ', TextAttr);

#if defined (Trial)
	ReadMessage(70);
#elif defined (FandRunV)
	ReadMessage(42);
#elif defined (FandDemo)
	ReadMessage(43);
#else
	ReadMessage(41);
#endif
	txt = "";
#ifdef FandNetV
	txt = "LAN,";
#endif
#ifdef FandSQL
	txt = "SQL,";
#endif
#ifndef FandGraph
	txt += "~GRAPH,";
#endif
#ifndef FandProlog
	txt += "~PRL,";
#endif
/*#ifndef FandDML
	txt += "~DML,";
#endif*/
//#ifdef Coproc
	txt = txt + "COPROC,";
//#endif
#ifdef FandTest
	txt = "test," + txt;
#endif
#ifdef FandAng
	txt = txt + "En ";
#endif

	if (!txt.empty()) {
		if (txt[txt.length() - 1] == ',') txt = txt.substr(0, txt.length() - 1);
		txt += ")";
		MsgLine = MsgLine + "x (" + txt;
	}
	else MsgLine += 'x';

	screen.ScrWrText(5, TxtRows - 3, MsgLine.c_str());

#ifdef FandRunV 
#ifndef FandNetV
	goto label2;
#endif
#endif

#ifndef FandDemo
	if (TxtCols >= 80) {
		std::string license = ReadMessage(40) + " " + std::to_string(UserLicNrShow);
		screen.ScrWrText(51, TxtRows - 3, license.c_str());
	}
#endif

label2:
	ReleaseStore(&p);
	MsgNr = 2;

#ifdef FandRunV
	MsgNr = 14;
#endif

#ifdef FandDemo
	if (Today() > 731215.0) WrLLF10Msg(47);
#endif

#ifdef FandTest
	// if (today > 730210.0) { WrLLF10Msg(47); /*exit;*/ }
#endif

	ReadMessage(MsgNr);
	mb = new TMenuBoxS(4, 3, MsgLine);
	int i = 1;

	while (true) {
		i = mb->Exec(i);
		j = i;
#ifdef FandRunV
		if (j != 0) j++;
#endif
		w = PushW(1, 1, TxtCols, TxtRows);

		switch (j) {
		case 1: {
			IsTestRun = true;
			SelectRunRdb(true);
			IsTestRun = false;
			break;
		}
		case 2: {
			IsTestRun = false;
			SelectRunRdb(true);
			IsTestRun = false;
			break;
		}
		case 3: {
			IsInstallRun = true;
			CallInstallRdb();
			IsInstallRun = false;
			break; }
		case 4: {
			SelectEditTxt(".TXT", true);
			break;
		}
		case 5: {
			// OSshell("", "", false, true, true, true);
			OpenFileDialog();
			break;
		}
		case 0:
		case 6: {
			CloseH(&WorkHandle);
			CloseFANDFiles(false);
			return;
			break;
		}
		default:;
		}
		PopW(w);
	}
}

void DeleteFandFiles()
{
	if (WorkHandle != nullptr) {
		try {
			fclose(WorkHandle);
			deleteFile(FandWorkName);
		}
		catch (std::exception&) {}
		WorkHandle = nullptr;
	}

	if (XWork.Handle != nullptr) {
		try {
			fclose(XWork.Handle);
			deleteFile(FandWorkXName);
		}
		catch (std::exception&) {}
		XWork.Handle = nullptr;
	}

	if (TWork.Handle != nullptr) {
		try {
			fclose(TWork.Handle);
			deleteFile(FandWorkTName);
		}
		catch (std::exception&) {}
		TWork.Handle = nullptr;
	}
}

void OpenFileDialog()
{
	char filename[MAX_PATH];
	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "�loha RDB\0*.RDB\0v�echny soubory\0*.*\0\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Vyberte �lohu";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	if (GetOpenFileName(&ofn))
	{
		printf("Dob�e to dopadlo :-)");
	}
	else
	{
		switch (CommDlgExtendedError())
		{
		case CDERR_DIALOGFAILURE: printf("CDERR_DIALOGFAILURE\n"); break;
		case CDERR_FINDRESFAILURE: printf("CDERR_FINDRESFAILURE\n"); break;
		case CDERR_INITIALIZATION: printf("CDERR_INITIALIZATION\n"); break;
		case CDERR_LOADRESFAILURE: printf("CDERR_LOADRESFAILURE\n"); break;
		case CDERR_LOADSTRFAILURE: printf("CDERR_LOADSTRFAILURE\n"); break;
		case CDERR_LOCKRESFAILURE: printf("CDERR_LOCKRESFAILURE\n"); break;
		case CDERR_MEMALLOCFAILURE: printf("CDERR_MEMALLOCFAILURE\n"); break;
		case CDERR_MEMLOCKFAILURE: printf("CDERR_MEMLOCKFAILURE\n"); break;
		case CDERR_NOHINSTANCE: printf("CDERR_NOHINSTANCE\n"); break;
		case CDERR_NOHOOK: printf("CDERR_NOHOOK\n"); break;
		case CDERR_NOTEMPLATE: printf("CDERR_NOTEMPLATE\n"); break;
		case CDERR_STRUCTSIZE: printf("CDERR_STRUCTSIZE\n"); break;
		case FNERR_BUFFERTOOSMALL: printf("FNERR_BUFFERTOOSMALL\n"); break;
		case FNERR_INVALIDFILENAME: printf("FNERR_INVALIDFILENAME\n"); break;
		case FNERR_SUBCLASSFAILURE: printf("FNERR_SUBCLASSFAILURE\n"); break;
		default: printf("You cancelled.\n");
		}
	}
}