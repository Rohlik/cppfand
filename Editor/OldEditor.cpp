﻿#include "OldEditor.h"
#include <set>
#include <stdexcept>
#include <memory>

#include "EditorEvents.h"
#include "runedi.h"
#include "../cppfand/compile.h"
#include "EditorHelp.h"
#include "EditorScreen.h"
#include "../cppfand/GlobalVariables.h"
#include "../Drivers/keyboard.h"
#include "../cppfand/oaccess.h"
#include "../cppfand/obase.h"
#include "../cppfand/obaseww.h"
#include "../cppfand/printtxt.h"
#include "../cppfand/wwmenu.h"
#include "../cppfand/wwmix.h"
#include "../cppfand/models/FrmlElem.h"
#include "../Common/textfunc.h"
#include "../Common/compare.h"

const int TXTCOLS = 80;
int Timer = 0;
bool Insert, Indent, Wrap, Just;

// PROMENNE
bool InsPage;
//PartDescr Part;

struct Character {
	char ch = 0;
	BYTE color = 0;
};

///  { ^s - underline, ^w - italic, ^q - expanded, ^d - double, ^b - bold, ^e - compressed, ^a - ELITE }
std::string CtrlKey = "\x13\x17\x11\x04\x02\x05\x01"; 

// *** Promenne metody EDIT
char Arr[SuccLineSize]{ '\0' };  // znaky pro 1 radek
char* T = nullptr;               // ukazatel na vstupni retezec (cely editovany text)
WORD NextLineStartIndex = 0;     // index prvniho znaku na dalsim radku
short TextLineNr = 0;          // cislo radku v celem textu (1 .. N)
short ScreenFirstLineNr = 0;   // cislo radku, ktery je na obrazovce zobrazen jako prvni (1 .. N)
int RScrL = 0;
bool UpdatedL = false, CtrlL = false;
bool HardL = false; // actual line (Arr) ended with CRLF "\r\n" - otherwise only with CR "\r"
WORD columnOffset = 0;
WORD Colu = 0;
WORD Row = 0;
bool ChangeScr = false;
ColorOrd ColScr;
bool IsWrScreen = false;
WORD FirstR = 0, FirstC = 0, LastR = 0, LastC = 0;
WORD MinC = 0, MinR = 0, MaxC = 0, MaxR = 0;
WORD MargLL[4]{ 0, 0, 0, 0 };
WORD PageS = 0, LineS = 0;
bool bScroll = false, FirstScroll = false, HelpScroll = false;
int PredScLn = 0;
WORD PredScPos = 0; // {pozice pred Scroll}
BYTE FrameDir = 0;
WORD WordL = 0; // {Mode=HelpM & ctrl-word is on screen}
bool Konec = false;
WORD i1 = 0, i3 = 0;
short i2 = 0;
// *** konec promennych

const BYTE InterfL = 4; /*sizeof(Insert+Indent+Wrap+Just)*/
const WORD TextStore = 0x1000;
const BYTE TStatL = 35; /*=10(Col Row)+length(InsMsg+IndMsg+WrapMsg+JustMsg+BlockMsg)*/

const BYTE CountC = 7;

std::set<char> Separ = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,
26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,58,59,60,61,62,63,64,
91,92,93,94,96,123,124,125,126,127 };


// {**********global param begin for SaveParams}  // r85
char Mode = '\0';
char TypeT = '\0';
std::string NameT;
std::string ErrMsg;
WORD MaxLenT = 0;
WORD IndexT = 0; // index editovaneho textu (uklada se pro opetovne nacteni na stejne pozici)
WORD ScrT = 0;
size_t LenT = 0; // delka editovaneho textu

//std::vector<EdExitD*> *ExitD = nullptr;

bool SrchT, UpdatT;
WORD LastNr, CtrlLastNr;
short LeftMarg, RightMarg;
bool TypeB;
std::string LastS, CtrlLastS, ShiftLastS, AltLastS, HeadS;
int* LocalPPtr;
bool EditT;

// od r101

BYTE ColKey[CountC + 1]{ 0 };
BYTE TxtColor = 0, BlockColor = 0, SysLColor = 0;
pstring InsMsg, nInsMsg, IndMsg, WrapMsg, JustMsg, BlockMsg;
pstring ViewMsg;
char CharPg = '\0';
bool InsPg = false;
WORD ScreenIndex = 0; // index of the first char on the screen 0 .. N
WORD textIndex = 0;
WORD positionOnActualLine = 0; // position of the cursor on the actual line (1 .. 255)
WORD BPos = 0; // {screen status}
std::string FindStr, ReplaceStr;
bool Replace = false;
pstring OptionStr;
bool FirstEvent = false;
WORD PHNum = 0, PPageS = 0; // {strankovani ve Scroll}

HANDLE TxtFH = nullptr;
pstring TxtPath;
pstring TxtVol;
//bool AllRd = false;
int AbsLenT = 0;
bool ChangePart, UpdPHead;
void RestorePar(int l);

Blocks* blocks = nullptr;


std::vector<std::string> GetLinesFromT()
{
	// create std::string from T
	std::string text(T, LenT);
	return GetAllLinesWithEnds(text);
}

char* GetTfromLines(std::vector<std::string>& lines, size_t& len)
{
	char* output = nullptr;
	len = 0;

	if (lines.empty()) {
		// do nothing
	}
	else {
		// calculate total length
		for (auto& line : lines) {
			len += line.length();
		}

		// generate string
		std::string txt;
		txt.reserve(len);
		for (size_t i = 0; i < lines.size(); i++) {
			txt += lines[i];
		}
		
		if (len != txt.length()) {
			throw std::exception("Bad string size - OldEditor.cpp, method GetT");
		}
		// create c_str
		output = new char[len + 1];
		memcpy(output, txt.c_str(), len);
		output[len] = '\0';
	}
	return output;
}

stEditorParams SaveParams()
{
	stEditorParams save_params;
	save_params.Insert = Insert;
	save_params.Indent = Indent;
	save_params.Wrap = Wrap;
	save_params.Just = Just;
	save_params.Mode = Mode;
	save_params.TypeT = TypeT;
	save_params.NameT = NameT;

	return save_params;
}

void RestoreParams(stEditorParams& editorParams)
{
	Insert = editorParams.Insert;
	Indent = editorParams.Indent;
	Wrap = editorParams.Wrap;
	Just = editorParams.Just;
	Mode = editorParams.Mode;
	TypeT = editorParams.TypeT;
	NameT = editorParams.NameT;
}

FrmlElem* RdFldNameFrmlT(char& FTyp)
{
	Error(8);
	return nullptr;
}

void MyWrLLMsg(pstring s)
{
	if (HandleError == 4) s = "";
	SetMsgPar(s);
	WrLLF10Msg(700 + HandleError);
}

void MyRunError(pstring s, WORD n)
{
	SetMsgPar(s);
	RunError(n);
}

void HMsgExit(pstring s)
{
	switch (HandleError) {
	case 0: return;
	case 1: {
		s = s[1];
		SetMsgPar(s);
		RunError(700 + HandleError);
		break;
	}
	case 2:
	case 3: {
		SetMsgPar(s);
		RunError(700 + HandleError);
		break;
	}
	case 4: {
		RunError(704);
		break;
	}
	}
}

/// <summary>
/// Find N-th char position in the text
/// </summary>
/// <param name="text">input text</param>
/// <param name="length">input text length</param>
/// <param name="c">character to find</param>
/// <param name="idx_from">start position 0..n</param>
/// <param name="n">n-th occur 1..n</param>
/// <returns>index of found character, input text length if not found</returns>
size_t FindCharPosition(char* text, size_t length, char c, size_t idx_from, size_t n)
{
	size_t result = std::string::npos; // as not found
	for (size_t j = 0; j < n; j++) {
		for (size_t i = idx_from; i < length; i++) {
			if (text[i] == c) {
				result = i;
				break;
			}
		}
		idx_from = result + 1;
	}
	return result == std::string::npos ? length : result;
}

bool TestOptStr(char c)
{
	return (OptionStr.first(c) != 0) || (OptionStr.first(toupper(c)) != 0);
}

WORD FindOrdChar(char C, WORD Pos, WORD Len)
{
	WORD I, K; char cc;
	I = Len; K = Pos - 1; cc = C;
	// TODO: ASM
	return Len - I;
}

WORD FindUpcChar(char C, WORD Pos, WORD Len)
{
	WORD I, K; char cc;
	I = Len; K = Pos - 1; cc = C;
	// TODO: ASM
	return Len - I;
}

bool SEquOrder(pstring S1, pstring S2)
{
	short i;
	if (S1.length() != S2.length()) return false;
	for (i = 1; i <= S1.length(); i++)
		if (CharOrdTab[S1[i]] != CharOrdTab[S2[i]]) return false;
	return true;
}

bool FindString(WORD& I, WORD Len)
{
	WORD i1 = 0;
	pstring s1, s2;
	char c = '\0';
	auto result = false;
	c = FindStr[1];
	if (!FindStr.empty())
	{
	label1:
		if (TestOptStr('~')) i1 = FindOrdChar(c, I, Len);
		else if (TestOptStr('u')) i1 = FindUpcChar(c, I, Len);
		else {
			i1 = FindCharPosition(T, Len, c, I);
		}
		I = i1;
		if (I + FindStr.length() > Len) {
			return result;
		}
		s2 = FindStr;
		Move(&T[I], &s1[1], FindStr.length());
		s1[0] = FindStr.length();
		if (TestOptStr('~')) {
			if (!SEquOrder(s1, s2)) {
				I++;
				goto label1;
			}
		}
		else if (TestOptStr('u')) {
			if (!EquUpCase(s1, s2)) {
				I++;
				goto label1;
			}
		}
		else if (s1 != s2) {
			I++;
			goto label1;
		}
		if (TestOptStr('w')) {
			if (I > 1 && !Separ.count(T[I - 1]) || !Separ.count(T[I + FindStr.length()])) {
				I++;
				goto label1;
			}
		}
		result = true;
		I += FindStr.length();
	}
	return result;
}

/**
 * \brief Find control char in the text
 * \param text input text
 * \param textLen input text length
 * \param first first index 0 .. N
 * \param last last index 0 .. N
 * \return index of control char or string::npos if not found
 */
size_t FindCtrlChar(char* text, size_t textLen, size_t first, size_t last)
{
	if (last > textLen - 1) {
		// koncovy index je za textem
		last = textLen - 1;
	}
	if (first > textLen - 1 || first > last) {
		// pocatecni index je za textem nebo za koncovym indexem
		return std::string::npos; // nenalezeno
	}
	else {
		// ^A ^B ^D ^E ^Q ^S ^W
		std::set<char> pc = { 0x01, 0x02, 0x04, 0x05, 0x11, 0x13, 0x17 };
		for (size_t i = first; i < last; i++) {
			if (pc.count(text[i]) > 0) return i;
		}
		return std::string::npos; // nenalezeno
	}
}

void SimplePrintHead()
{
	//pstring ln;
	PHNum = 0;
	PPageS = 0x7FFF;
}

void LastLine(char* input, WORD from, WORD num, WORD& Ind, WORD& Count)
{
	WORD length = Count;
	Count = 0;
	Ind = from;
	for (int i = from; i < length; i++) {
		if (input[i] == _CR) {
			Ind = from + i;
			Count++;
		}
	}
	if (Count > 0 && input[Ind] == _LF) {
		Ind++; // LF
	}
}

bool ReadTextFile()
{
	// kompletne prepsano -> vycte cely soubor do promenne T

	//auto fileSize = FileSizeH(TxtFH);
	//T = new char[fileSize];
	//SeekH(TxtFH, 0);
	//ReadH(TxtFH, fileSize, T);
	//LenT = fileSize;

	auto fileSize = GetFileSize(TxtFH, NULL);
	T = new char[fileSize];

	DWORD dwBytesRead;
	bool readResult = ReadFile(TxtFH, T, fileSize, &dwBytesRead, NULL);
	if (!readResult) {
		LenT = 0;
		HandleError = GetLastError();
		SetMsgPar(TxtPath);
		WrLLF10Msg(700 + HandleError);
	}
	else {
		LenT = fileSize;
		HandleError = 0;
	}

	return false; // return ChangePart
}

void FirstLine(WORD from, WORD num, WORD& Ind, WORD& Count)
{
	char* C = nullptr;
	WORD* COfs = (WORD*)C;
	Count = 0; Ind = from - 1; C = &T[from];
	for (WORD i = 0; i < num - 1; i++) {
		COfs--;
		if (*C == _CR) {
			Count++;
			Ind = from - i;
		}
	}
	if ((Count > 0) && (T[Ind + 1] == _LF)) Ind++;
}

//bool RdPredPart()
//{
//	CharArr* ppa;
//	WORD L1, L11, MI;
//	int BL, FSize, Rest, Max, Pos;
//	WORD Pass;
//	Max = MinL(MaxLenT, StoreAvail() + LenT);
//	Pass = Max - (Max >> 3);
//	Part.MovL = 0;
//	MI = 0;
//	auto result = false;
//	if (Part.PosP == 0) return result;
//	Pos = Part.PosP; BL = Part.LineP;
//	if (LenT <= (Pass >> 1)) goto label1;
//	FirstLine(LenT + 1, LenT - (Pass >> 1), L1, L11);
//	if (L1 < LenT) {
//		AllRd = false;
//		LenT = L1;
//		ReleaseStore(&T[LenT + 1]);
//	}
//
//label1:
//	L11 = LenT;
//	do {
//		if (Pos > 0x1000) L1 = 0x1000;
//		else L1 = Pos;
//		Max = StoreAvail();
//		if (Max > 0x400) Max -= 0x400;
//		if (L1 > Max) L1 = Max;
//		ppa = (CharArr*)GetStore(L1);
//		Move(&T[0], &T[L1 + 1], LenT);
//		if (L1 > 0)
//		{
//			SeekH(TxtFH, Pos - L1); ReadH(TxtFH, L1, T);
//		}
//		LenT += L1; Pos -= L1;
//	} while (!((LenT > Pass) || (Pos == 0) || (L1 == Max)));
//
//	L11 = LenT - L11; FirstLine(L11 + 1, L11, MI, Part.MovL);
//	if (Pos == 0) MI = L11;
//	else if (Part.MovL > 0) { Part.MovL--; MI = L11 - MI; }
//	L1 = L11 - MI; LenT -= L1; Pos += L1;
//	if (L1 > 0)
//	{
//		Move(&T[L1 + 1], T, LenT);
//		ReleaseStore(&T[LenT + 1]);
//	}
//	/* !!! with Part do!!! */
//	Part.PosP = Pos; Part.LineP = BL - Part.MovL; Part.LenP = LenT;
//	Part.MovI = MI; Part.UpdP = false;
//	SetColorOrd(Part.ColorP, 1, MI + 1);
//	if ((LenT == 0)) return result;  /*????????*/
//	result = true;
//	return result;
//}

void UpdateFile()
{
	//SeekH(TxtFH, 0);
	//WriteH(TxtFH, LenT, T);
	//if (HandleError != 0) {
	//	SetMsgPar(TxtPath);
	//	WrLLF10Msg(700 + HandleError);
	//}
	//FlushH(TxtFH);
	//TruncH(TxtFH, LenT);
	//AbsLenT = FileSizeH(TxtFH);
	//if (HandleError != 0) {
	//	SetMsgPar(TxtPath);
	//	WrLLF10Msg(700 + HandleError);
	//}

	DWORD seekResult = SetFilePointer(TxtFH, 0, NULL, FILE_BEGIN);
	if (seekResult == INVALID_SET_FILE_POINTER) {
		HandleError = GetLastError();
		SetMsgPar(TxtPath);
		WrLLF10Msg(700 + HandleError);
		return;
	}
	bool writeFile = WriteFile(TxtFH, T, LenT, NULL, NULL);
	if (!writeFile) {
		HandleError = GetLastError();
		SetMsgPar(TxtPath);
		WrLLF10Msg(700 + HandleError);
		return;
	}
	bool setEOF = SetEndOfFile(TxtFH);
	if (!setEOF) {
		HandleError = GetLastError();
		SetMsgPar(TxtPath);
		WrLLF10Msg(700 + HandleError);
		return;
	}
	DWORD fileSize = GetFileSize(TxtFH, NULL);
	if (fileSize != INVALID_FILE_SIZE) {
		AbsLenT = fileSize;
	}
	else {
		HandleError = GetLastError();
		SetMsgPar(TxtPath);
		WrLLF10Msg(700 + HandleError);
		return;
	}
}

//void RdPart()
//{
//	LenT = Part.LenP;
//	//T = (CharArr*)GetStore(LenT);
//	if (LenT == 0) return;
//	SeekH(TxtFH, Part.PosP);
//	ReadH(TxtFH, LenT, T);
//}

//void NullChangePart()
//{
//	ChangePart = false;
//	/*Part.MovI = 0;
//	Part.MovL = 0*/;
//}
//
//void RdFirstPart()
//{
//	NullChangePart();
//	// Part.PosP = 0; Part.LineP = 0; Part.LenP = 0; Part.ColorP = "";
//	//AllRd = false;
//	ChangePart = ReadTextFile();
//}

void OpenTxtFh(char Mode)
{
	FileUseMode UM;
	CPath = TxtPath;
	CVol = TxtVol;
	TestMountVol(CPath[0]);
	if (Mode == ViewM) {
		UM = RdOnly;
	}
	else {
		UM = Exclusive;
	}
	//TxtFH = OpenH(CPath, _isOldNewFile, UM);
	TxtFH = CreateFile(
		CPath.c_str(),                        // file name
		GENERIC_READ | GENERIC_WRITE,         // write access
		0,                                    // no sharing
		NULL,                                 // default security attributes
		OPEN_ALWAYS,                          // open file or create new
		FILE_ATTRIBUTE_NORMAL,                // normal file
		NULL);                    // no template file

	if (TxtFH == INVALID_HANDLE_VALUE) {
		HandleError = GetLastError();
		SetMsgPar(CPath);
		RunError(700 + HandleError);
	}
	//AbsLenT = FileSizeH(TxtFH);
	AbsLenT = GetFileSize(TxtFH, NULL);
}

pstring ShortName(pstring Name)
{
	WORD J = Name.length();
	while (!(Name[J] == '\\' || Name[J] == ':') && (J > 0)) {
		J--;
	}
	pstring s = Name.substr(J, Name.length() - J);
	if (Name[2] == ':') {
		s = Name.substr(0, 2) + s;
	}
	return s;
}

void WrStatusLine()
{
	std::string Blanks;
	if (Mode != HelpM) {
		if (HeadS.length() > 0) {
			Blanks = AddTrailChars(HeadS, ' ', TXTCOLS);
			size_t i = Blanks.find('_');
			if (i == std::string::npos) {
				Blanks = Blanks.substr(0, TStatL + 3) + Blanks.substr(TStatL + 3 - 1, 252 - TStatL);
				for (size_t j = 0; j < TStatL + 2; j++) {
					Blanks[j] = ' ';
				}
			}
			else {
				while ((i < Blanks.length()) && (Blanks[i] == '_')) {
					Blanks[i] = ' ';
					i++;
				}
			}
		}
		else {
			Blanks = RepeatString(' ', TxtCols);
			std::string s = ShortName(NameT);
			size_t i = TStatL + 3 - 1;
			if (s.length() + i >= TXTCOLS) i = TXTCOLS - s.length() - 2;
			for (size_t j = 0; j < s.length(); j++) {
				Blanks[i + j] = s[j];
			}
		}
		screen.ScrWrStr(1, 1, Blanks, SysLColor);
	}
}

void WriteMargins()
{
	CHAR_INFO LastL[201];

	if ((Mode != HelpM) && (Mode != ViewM) && Wrap) {
		screen.ScrRdBuf(FirstC - 1, TxtRows - 1, LastL, LineS);
		LastL[MargLL[0]].Attributes = MargLL[1] >> 8;
		LastL[MargLL[0]].Char.AsciiChar = MargLL[1] & 0x00FF;
		LastL[MargLL[2]].Attributes = MargLL[3] >> 8;
		LastL[MargLL[2]].Char.AsciiChar = MargLL[3] & 0x00FF;

		MargLL[0] = MaxI(0, LeftMarg - BPos);
		if (MargLL[0] > 0) {
			MargLL[1] = (LastL[MargLL[0]].Attributes << 8) + LastL[MargLL[0]].Char.AsciiChar;
			LastL[MargLL[0]].Attributes = LastL[LineS].Attributes;
			LastL[MargLL[0]].Char.AsciiChar = 0x10;
		}
		MargLL[2] = MaxI(0, RightMarg - BPos);
		if (MargLL[2] > 0) {
			MargLL[3] = (LastL[MargLL[2]].Attributes << 8) + LastL[MargLL[2]].Char.AsciiChar;
			LastL[MargLL[2]].Attributes = LastL[LineS].Attributes;
			LastL[MargLL[2]].Char.AsciiChar = 0x11;
		}
		screen.ScrWrCharInfoBuf(short(FirstC - 1), short(TxtRows - 1), LastL, LineS);
	}
}

void WrLLMargMsg(std::string& s, WORD n)
{
	if (!s.empty()) {
		MsgLine = s;
		WrLLMsgTxt();
	}
	else {
		if (n != 0) WrLLMsg(n);
		else {
			if (!LastS.empty()) {
				MsgLine = LastS;
				WrLLMsgTxt();
			}
			else {
				WrLLMsg(LastNr);
			}
			if (Mode == TextM) WriteMargins();
		}
	}
}

/// Inicializuje obrazovku - sirku, vysku editoru
void InitScr()
{
	FirstR = WindMin.Y;
	FirstC = WindMin.X;
	LastR = WindMax.Y;
	LastC = WindMax.X;

	if ((FirstR == 1) && (Mode != HelpM)) FirstR++;
	if (LastR == TxtRows) LastR--;
	MinC = FirstC; MinR = FirstR; MaxC = LastC; MaxR = LastR;
	screen.Window(FirstC, FirstR, LastC, LastR);
	FirstR--;
	if ((Mode != HelpM) && (Mode != ViewM) && Wrap) LastC--;
	PageS = LastR - FirstR; LineS = succ(LastC - FirstC);
}

void UpdStatLine(int Row, int Col, char mode)
{
	char RowCol[] = "RRRRR:CCCCC";
	char StatLine[] = "                                   ";

	if (!HelpScroll) {
		int lRow = Row; // +Part.LineP;
		snprintf(RowCol, sizeof(RowCol), "%5i:%-5i", lRow, Col);
		memcpy(&StatLine[1], RowCol, 11); // 11 znaku ve format 'RRRRR:CCCCC'
		switch (mode) { // uses parameter 'mode', not global variable 'Mode'
		case TextM: {
			if (Insert) Move(&InsMsg[1], &StatLine[10], 5);
			else Move(&nInsMsg[1], &StatLine[10], 5);
			if (Indent) Move(&IndMsg[1], &StatLine[15], 5);
			if (Wrap) Move(&WrapMsg[1], &StatLine[20], 5);
			if (Just) Move(&JustMsg[1], &StatLine[25], 5);
			if (TypeB == ColBlock) Move(&BlockMsg[1], &StatLine[30], 5);
			break;
		}
		case ViewM: { Move(&ViewMsg[1], &StatLine[10], ViewMsg.length()); break; }
		case SinFM: { StatLine[12] = '-'; break; }
		case DouFM: { StatLine[12] = '='; break; }
		case DelFM: { StatLine[12] = '/'; break; }
		default: break;
		}
		short i = 1;
		if (HeadS.length() > 0) {
			size_t find = HeadS.find('_');
			if (find == std::string::npos) {
				find = 0;
			}
			i = MaxW(1, find);
			if (i > TxtCols - TStatL) {
				i = MaxI((short)(TxtCols)-TStatL, 1);
			}
		}
		screen.ScrWrStr(i, 1, StatLine, SysLColor);
	}
}

WORD PColumn(WORD w, char* P)
{
	if (w == 0) {
		return 0;
	}

	WORD ww = 1;
	WORD c = 1;

	while (ww <= w) {
		if (P[ww] >= ' ') {
			c++;
			ww++;
		}
	}

	if (P[w] >= ' ') {
		c--;
	}

	return c;
}

bool MyTestEvent()
{
	if (FirstEvent) return false;
	//return TestEvent();
}

void DelEndT()
{
	T[LenT - 1] = '\0';
	LenT--;
}

void TestUpdFile()
{
	if (TxtFH != nullptr && UpdatT) {
		UpdateFile();
	}
}

void WrEndT()
{
	// vytvori nove pole o delce puvodniho + 1,
	// puvodni pole se do nej prekopiruje a na konec se vlozi '\0'
	if (LenT == 0) {
		delete[] T;
		T = new char[2]; // udelame pole o delce 2 -> mezera zakoncena \0
		T[0] = ' ';
		T[1] = '\0';
		LenT = 1;
	}
	else {
		char* T2 = new char[LenT + 1]; // udelame pole o 1 vetsi nez potrebujeme -> bude zakoncene '0'
		T2[LenT] = '\0';
		memcpy(T2, T, LenT);
		delete[] T;
		T = T2;
	}
}

void MoveIdx(int dir)
{
	WORD mi = -dir; // *Part.MovI;
	WORD ml = -dir; // *Part.MovL;
	ScreenIndex += mi;
	textIndex += mi; // {****GLOBAL***}
	NextLineStartIndex += mi;
	TextLineNr += ml;
	ScreenFirstLineNr += ml; // {****Edit***}
}

void PredPart()
{
	TestUpdFile();
	//ChangePart = RdPredPart();
	MoveIdx(-1);
	WrEndT();
}

/// Counts the number of occurrences of a character;
/// 'first' & 'last' are 0 .. N
size_t CountChar(char* text, size_t text_len, char C, size_t first, size_t last)
{
	size_t count = 0;
	if (first < text_len) {
		if (last >= text_len) last = text_len - 1;
		for (size_t i = first; i <= last; i++) {
			if (text[i] == C) count++;
		}
	}
	else {
		// out of index
	}
	return count;
}

/**
 * \brief Ziska cislo radku, ve kterem je znak na indexu
 * \param idx - index 0 .. n
 * \return vraci cislo radku (1 .. N), ve kterem se nachazi index
 */
WORD GetLine(WORD idx)
{
	return CountChar(T, LenT, _CR, 0, idx) + 1;
}

// vraci index 1. znaku na aktualnim radku (index = 0 .. N)
WORD CurrentLineFirstCharIndex(WORD index)
{
	WORD result = 0;
	while (index > 0) {
		if (T[index] == _CR) {
			// jsme na '\r' -> prazdny radek
			result = index;
			break;
		}

		if (T[index - 1] == _CR || T[index - 1] == _LF) {
			// predchozi znak je '\r' nebo '\n' -> jsme na 1. znaku radku
			//index++;
			//if (T[index] == _LF) index++;
			result = index;
			break;
		}

		index--;
	}
	return result;
}

void SmallerPart(WORD Ind, WORD FreeSize)
{
	WORD i, il, l;
	int lon;
	//NullChangePart();
	if ((StoreAvail() > FreeSize) && (MaxLenT - LenT > FreeSize)) {
		return;
	}
	TestUpdFile();
	WrEndT();
	lon = MinL(LenT + StoreAvail(), MaxLenT);
	lon -= FreeSize;
	if (lon <= 0) { return; }
	lon -= lon >> 3;
	i = 1; il = 0; l = 0;

	while (i < Ind) {
		if (T[i] == _CR)
		{
			l++; il = i;
			if (T[il + 1] == _LF) { il++; }
		}
		if (LenT - il < lon) { i = Ind; i++; }
	}

	if (il > 0) {
		// with Part do:
		//Part.PosP += il; Part.LineP += l;
		//Part.MovI = il; Part.MovL = l;
		//SetColorOrd(Part.ColorP, 1, Part.MovI + 1);
		// end

		LenT -= il;
		Move(&T[il + 1], T, LenT);
		T[LenT] = _CR;
		//ReleaseStore(&T[LenT + 1]);
		ChangePart = true;
		MoveIdx(1);
	}

	Ind -= il;
	if (LenT < lon) {
		return;
	}
	i = LenT;
	il = LenT;
	while (i > Ind) {
		if (T[i] == _CR) {
			il = i;
			if (T[il + 1] == _LF) { il++; }
		}
		i--;
		if (il < lon) { i = Ind; }
	}
	if (il < LenT)
	{
		//if (il < LenT - 1) { AllRd = false; }
		//Part.LenP = il;
		LenT = il + 1;
		T[LenT] = _CR;
		//ReleaseStore(&T[LenT + 1]);
	}
}

//void SetUpdat()
//{
//	UpdatT = true;
//	//if (TypeT == FileT) {
//	//	if (Part.PosP < 0x400) {
//	//		UpdPHead = true;
//	//		Part.UpdP = true;
//	//	}
//	//}
//}

//void TestLenText(char** text, size_t& textLength, size_t F, size_t LL)
//{
//	SetUpdat();
//	//printf("!!!");
//	//throw std::exception("TestLenText() implementation is bad. Don't call it.");
//}

void DekodLine(size_t lineStartIndex)
{
	WORD lineLen = FindCharPosition(T, LenT, _CR, lineStartIndex) - lineStartIndex;
	HardL = true;
	NextLineStartIndex = lineStartIndex + lineLen + 1; // 1 = CR

	if ((NextLineStartIndex < LenT) && (T[NextLineStartIndex] == _LF)) {
		NextLineStartIndex++;
	}
	else {
		HardL = false;
	}

	if (lineLen > LineMaxSize) {
		lineLen = LineMaxSize;
		if (Mode == TextM) {
			if (PromptYN(402)) {
				WORD LL = lineStartIndex + LineMaxSize;
				//NullChangePart();
				//TestLenText(&T, LenT, LL, (int)LL + 1);
				UpdatT = true;
				//LL -= Part.MovI;
				T[LL] = _CR;
				NextLineStartIndex = lineStartIndex + lineLen + 1;
			}
		}
		else {
			Mode = ViewM;
		}
	}

	FillChar(Arr, LineMaxSize, ' ');
	if (lineLen > 0) {
		// zkopiruj radek do Arr
		memcpy(Arr, &T[lineStartIndex], lineLen);
	}

	UpdatedL = false;
}

/// ziska index 1. znaku akt. radku, vola DekodLine()
void CopyCurrentLineToArr(size_t Ind)
{
	textIndex = CurrentLineFirstCharIndex(Ind);
	DekodLine(textIndex);
}

/// vraci cislo radku, na kterem je index
size_t GetLineNumber(size_t idx)
{
	CopyCurrentLineToArr(idx);
	size_t line = 1;
	if (idx == 0)
	{
		// 1st char is always in 1st line
	}
	else
	{
		line = GetLine(textIndex);
	}
	return line;
}

WORD SetInd(char* text, size_t len_text, WORD Ind, WORD Pos) // { line, pozice --> index}
{
	WORD P = Ind == 0 ? 0 : Ind - 1;
	if (Ind < len_text) {
		while ((Ind - P < Pos) && (text[Ind - 1] != _CR)) { Ind++; }
	}
	return Ind;
}

/**
 * \brief Returns order of N-th character in Arr (because of skipping color chars)
 * \param n N-th character (1..256)
 * \return order of the char (1..256)
 */
WORD Position(WORD n) // {PosToCol}
{
	WORD cc = 1;
	WORD p = 1;
	while (cc <= n) {
		if ((BYTE)Arr[p - 1] >= ' ') cc++;
		p++;
	}
	return p - 1;
}

/**
 * \brief Returns column for N-th character in Arr (because of skipping color chars)
 * \param p order of the char in the Arr (1..256)
 * \return column on the screen (1..256)
 */
WORD Column(WORD p)
{
	if (p == 0) return 0;

	WORD pp = 1;
	WORD c = 1;

	while (pp <= p) {
		if ((BYTE)Arr[pp - 1] >= ' ') {
			c++;
		}
		pp++;
	}

	if ((BYTE)Arr[p - 1] >= ' ') c--;

	return c;
}

/**
 * \brief Counts Arr Line length (without spaces on the end)
 * \return Arr line length (0 .. 255)
 */
WORD GetArrLineLength()
{
	int LP = LineMaxSize;
	while ((LP >= 0) && (Arr[LP] == ' ' || Arr[LP] == '\0')) {
		LP--;
	}
	return LP + 1; // vraci Pascal index 1 .. N;
}

//void NextPart()
//{
//	TestUpdFile();
//	ChangePart = RdNextPart();
//	MoveIdx(1);
//	WrEndT();
//}

/**
 * \brief Get index of the 1st character on the line
 * \param text input text
 * \param text_len input text length
 * \param lineNr line number (1 .. N)
 * \return index of first char on the line (0 .. n), or text length if not found
 */
size_t GetLineStartIndex(size_t lineNr)
{
	// znacne zjednoduseno oproti originalu

	size_t result;

	if (lineNr <= 1) {
		result = 0;
	}
	else {
		// hledame pozici za n-tym vyskytem _CR
		size_t pos = FindCharPosition(T, LenT, _CR, 0, lineNr - 1) + 1;
		// pokud je na nalezene pozici _LF, jdi o 1 dal
		if (pos < LenT && T[pos] == _LF) {
			pos++;
		}
		result = pos;
	}
	return result;
}

void SetPart(int Idx)
{
	//if ((Idx > Part.PosP) && (Idx < Part.PosP + LenT) || (TypeT != FileT)) {
	//	return;
	//}
	TestUpdFile();
	delete[] T; T = nullptr;
	ReadTextFile();
	//while ((Idx > Part.PosP + Part.LenP) && !AllRd)
	//{
	//	ChangePart = RdNextPart();
	//}
	WrEndT();
}

void SetPartLine(int Ln)
{
	//while ((Ln <= Part.LineP) && (Part.PosP > 0)) {
	//	PredPart();
	//}
	//while ((Ln - Part.LineP > 0x7FFF) && !AllRd) {
	//	NextPart();
	//}
}

void DekFindLine(int Num)
{
	SetPartLine(Num);
	TextLineNr = Num; // -Part.LineP;
	textIndex = GetLineStartIndex(TextLineNr);
	DekodLine(textIndex);
}

void PosDekFindLine(int Num, WORD Pos, bool ChScr)
{
	positionOnActualLine = Pos;
	DekFindLine(Num);
	ChangeScr = ChangeScr || ChScr;
}

void WrEndL(bool Hard, int Row)
{
	if ((Mode != HelpM) && (Mode != ViewM) && Wrap) {
		WORD w;
		if (Hard) {
			w = 0x11 + static_cast<WORD>(TxtColor << 8);
		}
		else {
			w = ' ' + static_cast<WORD>(TxtColor << 8);
		}
		screen.ScrWrBuf(WindMin.X + LineS, WindMin.Y + Row - 1, &w, 1);
	}
}

void NextPartDek()
{
	ReadTextFile();
	DekodLine(textIndex);
}

bool ModPage(int RLine)
{
	return false;
}

/**
 * \brief Reads colors in text and creates ColorOrd string from it
 * \param first index of the first char 0 .. n
 * \param last index of the last char 0 .. n
 * \return ColorOrd string with colors
 */
ColorOrd SetColorOrd(size_t first, size_t last)
{
	ColorOrd co;
	size_t index = FindCtrlChar(T, LenT, first, last);
	// if not found -> index = std::string::npos
	while (index < last) {
		size_t pp = co.find(T[index]);
		if (pp != std::string::npos) {
			co.erase(pp);
		}
		else {
			co += T[index];
		}
		index = FindCtrlChar(T, LenT, index + 2, last);
	}
	return co;
}

void UpdScreen()
{
	short r; // row number, starts from 1
	ColorOrd co1;
	WORD oldScreenIndex = ScreenIndex;
	pstring PgStr;

	// create screen object
	std::unique_ptr<EditorScreen> eScr = std::make_unique<EditorScreen>(TXTCOLS, blocks, CtrlKey);

	InsPage = false;
	if (ChangeScr) {
		if (ChangePart) {
			DekodLine(textIndex);
		}
		ChangeScr = false;

		if (bScroll) {
			ScreenIndex = textIndex;
		}
		else {
			ScreenIndex = GetLineStartIndex(ScreenFirstLineNr);
		}

		if (HelpScroll) {
			//ColScr = Part.ColorP;
			ColScr = SetColorOrd(0, ScreenIndex);
		}
	}
	if (bScroll) {
		// {tisk aktualniho radku}
		FillChar(&PgStr[0], 255, CharPg);
		PgStr[0] = 255;
		co1 = ColScr;
		r = 0;
		while (Arr[r] == 0x0C) {
			r++;
		}
		eScr->ScrollWrline(&Arr[r], columnOffset, 1, co1, ColKey, TxtColor, InsPage);
	}
	else if (Mode == HelpM) {
		//co1 = Part.ColorP;
		co1 = SetColorOrd(0, textIndex);
		eScr->ScrollWrline(Arr, columnOffset, TextLineNr - ScreenFirstLineNr + 2, co1, ColKey, TxtColor, InsPage);
	}
	else {
		eScr->EditWrline(Arr, 255, TextLineNr - ScreenFirstLineNr + 1, ColKey, TxtColor, BlockColor);
	}
	WrEndL(HardL, TextLineNr - ScreenFirstLineNr + 1);
	if (MyTestEvent()) return;

	WORD index = ScreenIndex;
	r = 1;
	short rr = 0;
	WORD w = 1;
	InsPage = false;
	ColorOrd co2 = ColScr;
	if (bScroll) {
		while (T[index] == 0x0C) {
			index++;
		}
	}
	do {
		if (MyTestEvent()) return; // {tisk celeho okna}

		if (bScroll && (index < LenT)) {
			if ((InsPg && (ModPage(r - rr + RScrL - 1))) || InsPage) {
				eScr->EditWrline((char*)&PgStr[1], LenT, r, ColKey, TxtColor, BlockColor);
				WrEndL(false, r);
				if (InsPage) rr++;
				InsPage = false;
				goto label1;
			}
		}
		if (!bScroll && (index == textIndex)) {
			index = NextLineStartIndex;
			co2 = co1;
			goto label1;
		}
		if (index < LenT) {
			// index je mensi nez delka textu -> porad je co tisknout
			if (HelpScroll) {
				eScr->ScrollWrline(&T[index], columnOffset, r, co2, ColKey, TxtColor, InsPage);
			}
			else {
				eScr->EditWrline(&T[index], LenT, r, ColKey, TxtColor, BlockColor);
			}
			if (InsPage) {
				// najde konec radku, potrebujeme 1. znak dalsiho radku
				index = FindCharPosition(T, LenT, 0x0C, index) + 1;
			}
			else {
				// najde konec radku, potrebujeme 1. znak dalsiho radku
				index = FindCharPosition(T, LenT, _CR, index) + 1;
			}
			WrEndL((index < LenT) && (T[index] == _LF), r);
			if (index < LenT && T[index] == _LF) {
				index++;
			}
		}
		else {
			eScr->EditWrline(nullptr, 0, r, ColKey, TxtColor, BlockColor);
			WrEndL(false, r);
		}

	label1:
		r++;
		if (index < LenT && bScroll && (T[index] == 0x0C)) {
			InsPage = InsPg;
			index++;
		}
	} while (r <= PageS);
}

void Background()
{
	UpdStatLine(TextLineNr, positionOnActualLine, Mode);
	// TODO: musi to tady byt?
	// if (MyTestEvent()) return;
	if (HelpScroll) {
		WORD p = positionOnActualLine;
		if (Mode == HelpM) {
			if (WordL == TextLineNr) {
				while (Arr[p] != 0x11) {
					p++;
				}
			}
		}
		if (Column(p) - columnOffset > LineS) {
			columnOffset = Column(p) - LineS;
			BPos = Position(columnOffset);
		}
		if (Column(positionOnActualLine) <= columnOffset) {
			columnOffset = Column(positionOnActualLine) - 1;
			BPos = Position(columnOffset);
		}
	}
	else {
		if (positionOnActualLine > LineS) {
			if (positionOnActualLine > BPos + LineS) {
				BPos = positionOnActualLine - LineS;
			}
		}
		if (positionOnActualLine <= BPos) {
			BPos = pred(positionOnActualLine);
		}
	}
	if (TextLineNr < ScreenFirstLineNr) {
		ScreenFirstLineNr = TextLineNr;
		ChangeScr = true;
	}
	if (TextLineNr >= ScreenFirstLineNr + PageS) {
		ScreenFirstLineNr = succ(TextLineNr - PageS);
		ChangeScr = true;
	}
	UpdScreen(); // {tisk obrazovky}
	WriteMargins();
	screen.GotoXY(positionOnActualLine - BPos, TextLineNr - ScreenFirstLineNr + 1);
	IsWrScreen = true;
}

void KodLine()
{
	size_t ArrLineLen = GetArrLineLength(); // Arr bez koncovych mezer
	std::string ArrLine = std::string(Arr, ArrLineLen);

	// create vector of strings from T
	std::vector<std::string>allLines = GetLinesFromT();

	if (TextLineNr == allLines.size()) {
		// posledni radek (nepridavame konec radku)
	}
	else {
		if (HardL) {
			ArrLine += "\r\n";
		}
		else {
			ArrLine += "\r";
		}
	}

	allLines[TextLineNr - 1] = ArrLine;

	//TestLenText(&T, LenT, NextLineStartIndex, textIndex + LP);
	UpdatT = true;

	// create T back from vector
	char* newT = GetTfromLines(allLines, LenT);
	delete[] T;
	T = newT;

	NextLineStartIndex = textIndex + ArrLine.length();
	//LP = NextLineStartIndex - 1;

	UpdatedL = false;
}

void TestKod()
{
	if (UpdatedL) KodLine();
}

int NewRL(int Line)
{
	return blocks->LineAbs(Line);
}

int NewL(int RLine)
{
	return RLine; // -Part.LineP;
}

void ScrollPress()
{
	bool old = bScroll;
	const bool fyz = keyboard.GetState(VK_SCROLL) & 0x0001;
	if (fyz == old) FirstScroll = false;
	bScroll = (fyz || FirstScroll) && (Mode != HelpM);
	HelpScroll = bScroll || (Mode == HelpM);
	int L1 = blocks->LineAbs(ScreenFirstLineNr);
	if (old != bScroll) {
		if (bScroll) {
			WrStatusLine();
			TestKod();
			screen.CrsHide();
			PredScLn = blocks->LineAbs(TextLineNr);
			PredScPos = positionOnActualLine;
			if (UpdPHead) {
				SetPart(1);
				SimplePrintHead();
				DekFindLine(MaxL(L1, PHNum + 1));
			}
			else {
				DekFindLine(MaxL(L1, PHNum + 1));
			}
			ScreenFirstLineNr = TextLineNr;
			RScrL = NewRL(ScreenFirstLineNr);
			if (L1 != blocks->LineAbs(ScreenFirstLineNr)) ChangeScr = true; // { DekodLine; }
			columnOffset = Column(BPos);
			Colu = Column(positionOnActualLine);
			//ColScr = Part.ColorP;
			ColScr = SetColorOrd(0, ScreenIndex);
		}
		else {
			if ((PredScLn < L1) || (PredScLn >= L1 + PageS)) PredScLn = L1;
			if (!(PredScPos >= BPos + 1 && PredScPos <= BPos + LineS)) PredScPos = BPos + 1;
			PosDekFindLine(PredScLn, PredScPos, false);
			if (Mode == ViewM || Mode == SinFM || Mode == DouFM
				|| Mode == DelFM || Mode == NotFM) screen.CrsBig();
			else screen.CrsNorm();
		}
		Background();
	}
}

void DisplLL(WORD Flags)
{
	if ((Flags & 0x04) != 0) // { Ctrl }
		WrLLMargMsg(CtrlLastS, CtrlLastNr);
	else if ((Flags & 0x03) != 0) // { Shift }
		WrLLMargMsg(ShiftLastS, 0);
	else if ((Flags & 0x08) != 0) // { Alt }
		WrLLMargMsg(AltLastS, 0);
}

//void MyInsLine()
//{
//	TextAttr = TxtColor;
//	InsLine();
//}

//void MyDelLine()
//{
//	TextAttr = TxtColor;
//	DelLine();
//}

void RollNext()
{
	//if ((NextLineStartIndex >= LenT) && !AllRd) NextPartDek();
	if (NextLineStartIndex <= LenT) {
		screen.GotoXY(1, 1);
		//MyDelLine();
		ScreenFirstLineNr++;
		ChangeScr = true;
		if (TextLineNr < ScreenFirstLineNr) {
			TestKod();
			TextLineNr++;
			textIndex = NextLineStartIndex;
			DekodLine(textIndex);
		}
	}
}

void RollPred()
{
	//if ((ScreenFirstLineNr == 1) && (Part.PosP > 0)) PredPart();
	if (ScreenFirstLineNr > 1) {
		screen.GotoXY(1, 1);
		//MyInsLine();
		ScreenFirstLineNr--;
		ChangeScr = true;
		if (TextLineNr == ScreenFirstLineNr + PageS) {
			TestKod();
			TextLineNr--;
			if (T[textIndex - 1] == _LF) { CopyCurrentLineToArr(textIndex - 2); }
			else { CopyCurrentLineToArr(textIndex - 1); }
		}
	}
}

void direction(BYTE x, BYTE& zn2)
{
	BYTE y = 0x10;
	if (x > 2) { y = y << 1; }
	if (x == 0) { y = 0; }
	if (Mode == DouFM) {
		zn2 = zn2 | y;
	}
	else {
		zn2 = zn2 & !y;
	}
}

void MyWriteln()
{
	TextAttr = TxtColor;
	printf("\n");
}

void PreviousLine()
{
	//WORD mi, ml;
	TestKod();
	//if ((TextLineNr == 1) && (Part.PosP > 0)) PredPart();
	if (TextLineNr > 1) {
		TextLineNr--;
		textIndex = GetLineStartIndex(TextLineNr);
		CopyCurrentLineToArr(textIndex);
		//if (T[textIndex - 1] == _LF) {
		//	size_t line 
		//	CopyCurrentLineToArr(textIndex - 3);
		//}
		//else {
		//	CopyCurrentLineToArr(textIndex - 2);
		//}
		//TextLineNr--;
		if (TextLineNr < ScreenFirstLineNr) {
			screen.GotoXY(1, 1);
			//MyInsLine();
			ScreenFirstLineNr--;
			ChangeScr = true;
			if (bScroll) {
				/*dec(RLineL);*/
				RScrL--;
				/*if (ModPage(RLineL))*/
				if (ModPage(RScrL)) {
					screen.GotoXY(1, 1);
					//MyInsLine();/*dec(RLineL);*/
					RScrL--;
				}
			}
		}
	}
}

void NextLine(bool WrScr)
{
	TestKod();
	//if ((NextLineStartIndex >= LenT) && !AllRd) NextPartDek();
	if (NextLineStartIndex < LenT) {
		textIndex = NextLineStartIndex;
		DekodLine(textIndex);
		TextLineNr++;
		if (bScroll) {
			if (PageS > 1) MyWriteln();
			ScreenFirstLineNr++;
			ChangeScr = true;
			RScrL++;
			if (ModPage(RScrL)) {
				if (PageS > 1) MyWriteln();
				RScrL++;
			}
		}
		else if (WrScr && (TextLineNr == ScreenFirstLineNr + PageS)) {
			//if (PageS > 1) MyWriteln();
			ScreenFirstLineNr++;
			ChangeScr = true;
		}
	}
}

//void Frame(std::vector<EdExitD*>& ExitD, std::vector<WORD>& breakKeys)
//{
//	pstring FrameString(15);
//	FrameString = "\x50\x48\xB3\x4D\xDA\xC0\xC3\x4B\xBF\xD9\xB4\xC4\xC2\xC1\xC5";
//	pstring FS1(15);
//	FS1 = "\x50\x48\xBA\x4D\xD6\xD3\xC7\x4B\xB7\xBD\xB6\xC4\xD2\xD0\xD7";
//	pstring FS2(15);
//	FS2 = "\x50\x48\xB3\x4D\xD5\xD4\xC6\x4B\xB8\xBE\xB5\xCD\xD1\xCF\xD8";
//	pstring FS3(15);
//	FS3 = "\x50\x48\xBA\x4D\xC9\xC8\xCC\x4B\xBB\xBC\xB9\xCD\xCB\xCA\xCE";
//	BYTE dir, zn1, zn2, b;
//
//	UpdStatLine(TextLineNr, positionOnActualLine, Mode);
//	screen.CrsBig();
//	BYTE odir = 0;
//	ClrEvent();
//
//	while (true) /* !!! with Event do!!! */
//	{
//		if (!MyGetEvent(Mode, SysLColor, LastS, LastNr, IsWrScreen, bScroll, ExitD, breakKeys) ||
//			((Event.What == evKeyDown) && (Event.Pressed.KeyCombination() == __ESC)) || (Event.What != evKeyDown)) {
//			ClrEvent();
//			screen.CrsNorm();
//			Mode = TextM;
//			return;
//		}
//		switch (Event.Pressed.KeyCombination()) {
//		case _frmsin_: Mode = SinFM; break;
//		case _frmdoub_: Mode = DouFM; break;
//		case _dfrm_: Mode = DelFM; break;
//		case _nfrm_: Mode = NotFM; break;
//		case __LEFT:
//		case __RIGHT:
//		case __UP:
//		case __DOWN:
//			if (!bScroll) {
//				FrameString[0] = 63;
//				zn1 = FrameString.first(Arr[positionOnActualLine]);
//				zn2 = zn1 & 0x30;
//				zn1 = zn1 & 0x0F;
//				dir = FrameString.first(Hi(Event.Pressed.KeyCombination()));
//				auto dirodir = dir + odir;
//				if (dirodir == 2 || dirodir == 4 || dirodir == 8 || dirodir == 16) odir = 0;
//				if (zn1 == 1 || zn1 == 2 || zn1 == 4 || zn1 == 8) zn1 = 0;
//				char oldzn = Arr[positionOnActualLine];
//				Arr[positionOnActualLine] = ' ';
//				if (Mode == DelFM) b = zn1 && !(odir || dir);
//				else b = zn1 | (odir ^ dir);
//				if (b == 1 || b == 2 || b == 4 || b == 8) b = 0;
//				if ((Mode == DelFM) && (zn1 != 0) && (b == 0)) oldzn = ' ';
//				direction(dir, zn2);
//				direction(odir, zn2);
//				if (Mode == NotFM) b = 0;
//
//				if ((b != 0) && ((Event.Pressed.KeyCombination() == __LEFT) || (Event.Pressed.KeyCombination() == __RIGHT) ||
//					(Event.Pressed.KeyCombination() == __UP) || (Event.Pressed.KeyCombination() == __DOWN)))
//					Arr[positionOnActualLine] = FrameString[zn2 + b];
//				else Arr[positionOnActualLine] = oldzn;
//
//				if ((dir == 1) || (dir == 4)) odir = dir * 2;
//				else odir = dir / 2;
//
//				if (Mode == NotFM) odir = 0;
//				else UpdatedL = true;
//
//				switch (Event.Pressed.KeyCombination()) {
//				case __LEFT: { if (positionOnActualLine > 1) positionOnActualLine--; break; }
//				case __RIGHT: { if (positionOnActualLine < LineMaxSize) positionOnActualLine++; break; }
//				case __UP: { PreviousLine(); break; }
//				case __DOWN: { NextLine(true); break; }
//				default: {};
//				}
//			}
//			break;
//		}
//		ClrEvent();
//		UpdStatLine(TextLineNr, positionOnActualLine, Mode);/*if (not MyTestEvent) */
//		Background();
//	}
//}

void CleanFrame(std::vector<EdExitD*>& ExitD, std::vector<WORD>& breakKeys)
{
	//if (Mode == SinFM || Mode == DouFM || Mode == DelFM || Mode == NotFM) /* !!! with Event do!!! */
	//	if (!MyGetEvent(Mode, SysLColor, LastS, LastNr, IsWrScreen, bScroll, ExitD, breakKeys) ||
	//		((Event.What == evKeyDown) && (Event.Pressed.KeyCombination() == __ESC)) || (Event.What != evKeyDown))
	//	{
	//		ClrEvent();
	//		screen.CrsNorm();
	//		Mode = TextM;
	//		UpdStatLine(TextLineNr, positionOnActualLine, Mode);
	//		return;
	//	}
}

void FrameStep(BYTE& odir, PressedKey EvKeyC)
{
	std::string FrameString = "\x3F\x50\x48\xB3\x4D\xDA\xC0\xC3\x4B\xBF\xD9\xB4\xC4\xC2\xC1\xC5";
	//                                       │       ┌   └   ├       ┐   ┘   ┤   ─   ┬   ┴   ┼
	FrameString += "\x0F\x50\x48\xBA\x4D\xD6\xD3\xC7\x4B\xB7\xBD\xB6\xC4\xD2\xD0\xD7";
	//                                       ║       Í   Ë   ă       Ě   Ż   Â   ─   Ď   đ   Î
	FrameString += "\x0F\x50\x48\xB3\x4D\xD5\xD4\xC6\x4B\xB8\xBE\xB5\xCD\xD1\xCF\xD8";
	//							             │       Ň   ď   Ă       Ş   ż   Á   ═   Đ   ¤   ě
	FrameString += "\x0F\x50\x48\xBA\x4D\xC9\xC8\xCC\x4B\xBB\xBC\xB9\xCD\xCB\xCA\xCE";
	//                                       ║       ╔   ╚   ╠       ╗   ╝   ╣   ═   ╦   ╩   ╬

	switch (EvKeyC.KeyCombination()) {
	case '-': { Mode = SinFM; break; }
	case '=': { Mode = DouFM; break; }
	case '/': { Mode = DelFM; break; }
	case ' ': { Mode = NotFM; break; }
	case __ESC: {
		screen.CrsNorm();
		Mode = TextM;
	}
	case __LEFT:
	case __RIGHT:
	case __UP:
	case __DOWN:
	{
		WORD scanCode = EvKeyC.Key()->wVirtualScanCode;
		size_t idx = FrameString.find_first_of(Arr[positionOnActualLine - 1]);
		BYTE zn1 = (idx == std::string::npos) ? 0 : BYTE(idx);
		BYTE zn2 = zn1 & 0x30;
		zn1 = zn1 & 0x0F;

		idx = FrameString.find_first_of(Lo(scanCode));
		BYTE dir = (idx == std::string::npos) ? 0 : BYTE(idx);
		auto dirodir = dir + odir;
		if (dirodir == 2 || dirodir == 4 || dirodir == 8 || dirodir == 16) {
			odir = 0;
		}
		if (zn1 == 1 || zn1 == 2 || zn1 == 4 || zn1 == 8) {
			zn1 = 0;
		}
		char oldzn = Arr[positionOnActualLine - 1];
		Arr[positionOnActualLine - 1] = ' ';
		BYTE b;
		if (Mode == DelFM) {
			b = zn1 & !(odir | dir);
		}
		else {
			b = zn1 | (odir ^ dir);
		}
		if (b == 1 || b == 2 || b == 4 || b == 8) {
			b = 0;
		}
		if ((Mode == DelFM) && (zn1 != 0) && (b == 0)) {
			oldzn = ' ';
		}
		direction(dir, zn2);
		direction(odir, zn2);
		if (Mode == NotFM) {
			b = 0;
		}

		if ((b != 0) && ((Event.Pressed.KeyCombination() == __LEFT) || (Event.Pressed.KeyCombination() == __RIGHT) ||
			(Event.Pressed.KeyCombination() == __UP) || (Event.Pressed.KeyCombination() == __DOWN))) {
			Arr[positionOnActualLine - 1] = FrameString[zn2 + b];
		}
		else {
			Arr[positionOnActualLine - 1] = oldzn;
		}

		if ((dir == 1) || (dir == 4)) {
			odir = dir * 2;
		}
		else {
			odir = dir / 2;
		}

		if (Mode == NotFM) odir = 0;
		else UpdatedL = true;

		switch (Event.Pressed.KeyCombination()) {
		case __LEFT: {
			if (positionOnActualLine > 1) positionOnActualLine--;
			break;
		}
		case __RIGHT: {
			if (positionOnActualLine < LineMaxSize) positionOnActualLine++;
			break;
		}
		case __UP: {
			PreviousLine();
			break;
		}
		case __DOWN: {
			NextLine(true);
			break;
		}
		default:;
		}
	}
	break;
	}
	UpdStatLine(TextLineNr, positionOnActualLine, Mode);
}

void MoveB(WORD& B, WORD& F, WORD& T)
{
	if (F <= T) { if (B > F) B += T - F; }
	else if (B >= F) B -= F - T;
	else if (B > T) B = T; B = MinW(B, GetArrLineLength() + 1);
}

bool TestLastPos(WORD F, WORD T)
{
	WORD LP = GetArrLineLength();
	if (F > LP) F = LP + 1;
	if (LP + T - F <= LineMaxSize) {
		if (LP >= F) {
			memcpy(&Arr[T - 1], &Arr[F - 1], LP - F + 1);
		}
		if (TypeB == TextBlock) {
			if (blocks->LineAbs(TextLineNr) == blocks->BegBLn) {
				MoveB(blocks->BegBPos, F, T);
			}
			if (blocks->LineAbs(TextLineNr) == blocks->EndBLn) {
				MoveB(blocks->EndBPos, F, T);
			}
		}
		if (F > T) {
			if (T <= LP) {
				memset(&Arr[LP + T - F], ' ', F - T);
			}
		}
		UpdatedL = true;
		return true;
	}
	else return false;
}

void DelChar()
{
	WORD LP;
	TestLastPos(positionOnActualLine + 1, positionOnActualLine);
}

void FillBlank()
{
	KodLine();
	WORD I = GetArrLineLength();
	if (positionOnActualLine > I + 1) {
		//TestLenText(&T, LenT, textIndex + I, textIndex + positionOnActualLine - 1);
		UpdatT = true;
		memset(&T[textIndex + I], ' ', positionOnActualLine - I - 1);
		NextLineStartIndex += positionOnActualLine - I - 1;
	}
}

void DeleteLine()
{
	FillBlank();
	if (LenT == 0) return;
	if (blocks->LineAbs(TextLineNr) + 1 <= blocks->BegBLn)
	{
		blocks->BegBLn--;
		if ((blocks->LineAbs(TextLineNr) == blocks->BegBLn) && (TypeB == TextBlock))
			blocks->BegBPos += GetArrLineLength();
	}
	if (blocks->LineAbs(TextLineNr) + 1 <= blocks->EndBLn)
	{
		blocks->EndBLn--;
		if ((blocks->LineAbs(TextLineNr) == blocks->EndBLn) && (TypeB == TextBlock))
			blocks->EndBPos += GetArrLineLength();
	}
	//if ((NextLineStartIndex >= LenT) && !AllRd) NextPartDek();
	if (NextLineStartIndex <= LenT) {
		auto lines = GetLinesFromT();

		size_t EoL_length = HardL ? 2 : 1;

		if (Event.Pressed.Char == '\b') {
			// if BACKSPACE was pressed we will delete line below cursor (which was already moved)
			if (TextLineNr < 1) return;
			lines[TextLineNr - 1] = lines[TextLineNr - 1].substr(0, lines[TextLineNr - 1].length() - EoL_length) + lines[TextLineNr];
			lines.erase(lines.begin() + TextLineNr);
		}
		else {
			if (TextLineNr < 1 || TextLineNr == lines.size()) {
				// cursor is on the last line, there is nothing more to move
				return;
			}
			else {
				lines[TextLineNr - 1] = lines[TextLineNr - 1].substr(0, lines[TextLineNr - 1].length() - EoL_length) + lines[TextLineNr];
				lines.erase(lines.begin() + TextLineNr);
			}
		}

		auto newT = GetTfromLines(lines, LenT);
		delete[] T;
		T = newT;
	}
	DekodLine(textIndex);
}

void NewLine(char Mode)
{
	KodLine();
	WORD LP = textIndex + MinI(GetArrLineLength(), positionOnActualLine - 1);
	//NullChangePart();

	std::string EoL = HardL ? "\r\n" : "\r";

	auto lines = GetLinesFromT();
	lines.insert(lines.begin() + TextLineNr, "");

	//TestLenText(&T, LenT, LP, LP + 2);
	UpdatT = true;

	// vse od aktualni pozice zkopirujeme na dalsi radek (nove vytvoreny)
	lines[TextLineNr] = lines[TextLineNr - 1].substr(positionOnActualLine - 1);
	// na puvodnim radku zustane vse pred pozici kurzoru a pridame ukonceni radku
	lines[TextLineNr - 1] = lines[TextLineNr - 1].substr(0, positionOnActualLine - 1) + EoL;

 	char* newT = GetTfromLines(lines, LenT);
	delete[] T;
	T = newT;

	//LP -= Part.MovI;
	if (blocks->LineAbs(TextLineNr) <= blocks->BegBLn) {
		if (blocks->LineAbs(TextLineNr) < blocks->BegBLn) {
			blocks->BegBLn++;
		}
		else if ((blocks->BegBPos > positionOnActualLine) && (TypeB == TextBlock)) {
			blocks->BegBLn++;
			blocks->BegBPos -= positionOnActualLine - 1;
		}
	}
	if (blocks->LineAbs(TextLineNr) <= blocks->EndBLn) {
		if (blocks->LineAbs(TextLineNr) < blocks->EndBLn) {
			blocks->EndBLn++;
		}
		else if ((blocks->EndBPos > positionOnActualLine) && (TypeB == TextBlock)) {
			blocks->EndBLn++;
			blocks->EndBPos -= positionOnActualLine - 1;
		}
	}

	if (Mode == 'm') {
		TextLineNr++;
		textIndex = LP + 1;
	}
	DekodLine(textIndex);
}

WORD SetPredI()
{
	//if ((TextLineNr == 1) && (Part.PosP > 0)) PredPart();
	if (textIndex <= 1) return textIndex;
	else if (T[textIndex - 1] == _LF) return CurrentLineFirstCharIndex(textIndex - 2);
	else return CurrentLineFirstCharIndex(textIndex - 1);
}

void WrCharE(char Ch)
{
	if (Insert) {
		if (TestLastPos(positionOnActualLine, positionOnActualLine + 1)) {
			Arr[positionOnActualLine - 1] = Ch;
			if (positionOnActualLine < LineMaxSize) {
				positionOnActualLine++;
			}
		}
	}
	else {
		Arr[positionOnActualLine - 1] = Ch;
		UpdatedL = true;
		if (positionOnActualLine < LineMaxSize) {
			positionOnActualLine++;
		}
	}
}

void Format(WORD& i, int First, int Last, WORD Posit, bool Rep)
{
	WORD lst, ii1;
	short ii;
	char A[260];
	bool bBool;
	WORD rp, nb, nw, n;
	WORD RelPos;

	SetPart(First);
	WORD fst = First; // -Part.PosP;
	int llst = Last; // -Part.PosP;
	if (llst > LenT) lst = LenT;
	else lst = llst;
	do {
		if (LenT > 0x400) ii1 = LenT - 0x400;
		else ii1 = 0;
		//if ((fst >= ii1) && !AllRd) {
		//	NextPartDek();
		//	//fst -= Part.MovI;
		//	//lst -= Part.MovI;
		//	//llst -= Part.MovI;
		//	if (llst > LenT) lst = LenT;
		//	else lst = llst;
		//}
		i = fst; ii1 = i;
		if ((i < 2) || (T[i - 1] == _LF)) {
			while (T[ii1] == ' ') ii1++; Posit = MaxW(Posit, ii1 - i + 1);
		}
		ii1 = i; RelPos = 1;
		if (Posit > 1) {
			Move(&T[i], A, Posit);
			for (ii = 1; ii <= Posit - 1; i++) {
				if (CtrlKey.find(T[i]) == std::string::npos) RelPos++;
				if (T[i] == _CR) A[ii] = ' ';
				else i++;
			}
			if ((T[i] == ' ') && (A[Posit - 1] != ' ')) {
				Posit++; RelPos++;
			}
		}
		while (i < lst) {
			bBool = true; nw = 0; nb = 0;
			if (RelPos < LeftMarg)
				if ((Posit == 1) || (A[Posit - 1] == ' '))
				{
					ii = LeftMarg - RelPos; FillChar(&A[Posit], ii, 32);
					Posit += ii; RelPos = LeftMarg;
				}
				else while (RelPos < LeftMarg)
				{
					Posit++;
					if (CtrlKey.find(T[i]) == std::string::npos) RelPos++;
					if (T[i] != _CR) i++;
					if (T[i] == _CR) A[Posit] = ' ';
					else A[Posit] = T[i];
				}
			while ((RelPos <= RightMarg) && (i < lst)) {
				if ((T[i] == _CR) || (T[i] == ' ')) {
					while (((T[i] == _CR) || (T[i] == ' ')) && (i < lst))
						if (T[i + 1] == _LF) lst = i;
						else { T[i] = ' '; i++; }
					if (!bBool) { nw++; if (i < lst) i--; };
				}
				if (i < lst) {
					bBool = false;
					A[Posit] = T[i];
					if (CtrlKey.find(A[Posit]) == std::string::npos) RelPos++;
					i++; Posit++;
				}
			}
			if ((i < lst) && (T[i] != ' ') && (T[i] != _CR)) {
				ii = Posit - 1;
				if (CtrlKey.find(A[ii]) != std::string::npos) ii--;
				rp = RelPos; RelPos--;
				while ((A[ii] != ' ') && (ii > LeftMarg)) {
					if (CtrlKey.find(A[ii]) == std::string::npos) RelPos--; ii--;
				}
				if (RelPos > LeftMarg) {
					nb = rp - RelPos;
					i -= (Posit - ii);
					Posit = ii;
				}
				else
				{
					while ((T[i] != ' ') && (T[i] != _CR) && (Posit < LineMaxSize)) {
						A[Posit] = T[i]; i++; Posit++;
					}
					while (((T[i] == _CR) || (T[i] == ' ')) && (i < lst)) {
						if (T[i + 1] == _LF) {
							lst = i;
						}
						else {
							T[i] = ' ';
							i++;
						}
					}
				}
			}
			if (Just)
			{
				ii = LeftMarg;
				while ((nb > 0) and (nw > 1))
				{
					while (A[ii] == ' ') ii++;
					while (A[ii] != ' ') ii++;
					nw--; n = nb / nw;
					if ((nw % nb != 0) && (nw % 2) && (nb > n)) n++;
					if (Posit - ii > 0) Move(&A[ii], &A[ii + n], Posit - ii + 1);
					FillChar(&A[ii], n, 32); Posit += n;
					nb -= n;
				}
			}
			ii = 1;
			while (A[ii] == ' ') ii++;
			if (ii >= Posit) Posit = 1;
			if (i < lst) A[Posit] = _CR; else Posit--;
			//TestLenText(&T, LenT, i, int(ii1) + Posit);
			UpdatT = true;
			if (Posit > 0) Move(A, &T[ii1], Posit);
			ii = ii1 + Posit - i; i = ii1 + Posit; lst += ii; llst += ii;
			Posit = 1; RelPos = 1; ii1 = i;
		}
		if (Rep)
		{
			while ((T[i] == _CR) || (T[i] == _LF)) i++; fst = i; Rep = i < llst;
			if (llst > LenT) lst = LenT; else lst = llst;
		}
	} while (Rep);
	blocks->BegBLn = 1; blocks->BegBPos = 1; blocks->EndBLn = 1; blocks->EndBPos = 1; TypeB = TextBlock;
}

void Calculate()
{
	wwmix ww;
	FrmlElem* Z = nullptr;
	std::string txt;
	WORD I; pstring Msg;
	void* p = nullptr;
	char FTyp;
	double R;
	bool Del;
	MarkStore(p);
	//NewExit(Ovr(), er);
	//goto label2;
	try {
		ResetCompilePars();
		RdFldNameFrml = RdFldNameFrmlT;
	label0:
		txt = CalcTxt;
		Del = true; I = 1;
	label1:
		TxtEdCtrlUBrk = true; TxtEdCtrlF4Brk = true;
		ww.PromptLL(114, txt, I, Del);
		if (Event.Pressed.KeyCombination() == _U_) goto label0;
		if (Event.Pressed.KeyCombination() == __ESC || txt.length() == 0) goto label3;
		CalcTxt = txt;
		if (Event.Pressed.KeyCombination() == __CTRL_F4 && Mode == TextM && !bScroll) {
			if (txt.length() > LineMaxSize - GetArrLineLength()) {
				I = LineMaxSize - GetArrLineLength();
				WrLLF10Msg(419);
				goto label1;
			}
			if (positionOnActualLine <= GetArrLineLength()) TestLastPos(positionOnActualLine, positionOnActualLine + txt.length());
			memcpy(&Arr[positionOnActualLine], txt.c_str(), txt.length());
			UpdatedL = true;
			goto label3;
		}
		SetInpStr(txt);
		RdLex();
		Z = RdFrml(FTyp);
		if (Lexem != 0x1A) Error(21);

		switch (FTyp) {
		case 'R': {
			R = RunReal(CFile, Z, CRecPtr);
			str(R, 30, 10, txt);
			txt = LeadChar(' ', TrailChar(txt, '0'));
			if (txt[txt.length() - 1] == '.') {
				txt = txt.substr(0, txt.length() - 1);
			}
			break;
		}
		case 'S': {
			/* wie RdMode fuer T ??*/
			txt = RunShortStr(CFile, Z, CRecPtr);
			break;
		}
		case 'B': {
			if (RunBool(CFile, Z, CRecPtr)) txt = AbbrYes;
			else txt = AbbrNo;
			break;
		}
		}
		I = 1;
		goto label1;
	}
	catch (std::exception& e) {
		//label2:
		Msg = MsgLine;
		I = CurrPos;
		SetMsgPar(Msg);
		WrLLF10Msg(110);
		IsCompileErr = false;
		ReleaseStore(&p);
		Del = false;
		// TODO: goto label1;
	}
label3:
	ReleaseStore(&p);
}

bool BlockExist()
{
	if (TypeB == TextBlock)
		return (blocks->BegBLn < blocks->EndBLn) || (blocks->BegBLn == blocks->EndBLn) && (blocks->BegBPos < blocks->EndBPos);
	return (blocks->BegBLn <= blocks->EndBLn) && (blocks->BegBPos < blocks->EndBPos);
}

void SetBlockBound(int& BBPos, int& EBPos)
{
	SetPartLine(blocks->EndBLn);
	short i = blocks->EndBLn; // -Part.LineP;
	size_t nextLineIdx = GetLineStartIndex(i);
	EBPos = SetInd(T, LenT, nextLineIdx, blocks->EndBPos); // +Part.PosP;
	SetPartLine(blocks->BegBLn);
	i = blocks->BegBLn; // -Part.LineP;
	nextLineIdx = GetLineStartIndex(i);
	BBPos = SetInd(T, LenT, nextLineIdx, blocks->BegBPos); // +Part.PosP;
}

void ResetPrint(char Oper, int& fs, FILE* W1, int LenPrint, ColorOrd* co, WORD& I1, bool isPrintFile, char* p)
{
	//*co = Part.ColorP;
	*co = SetColorOrd(0, I1 - 1);
	isPrintFile = false;
	fs = co->length();
	LenPrint += fs;
	if (Oper == 'p') LenPrint++;
	if ((StoreAvail() > LenPrint) && (LenPrint < 0xFFF0)) {
		char* t = new char[LenPrint];
		p = t;
		Move(&co[1], p, co->length());
	}
	else {
		isPrintFile = true;
		W1 = WorkHandle;
		SeekH(W1, 0);
		WriteH(W1, co->length(), &co[1]);
		HMsgExit(CPath);
	}
}

void LowCase(unsigned char& c)
{
	if ((c >= 'A') && (c <= 'Z')) { c = c + 0x20; return; }
	for (size_t i = 128; i <= 255; i++)
		if (((unsigned char)UpcCharTab[i] == c) && (i != c)) { c = i; return; }
}

void LowCase(char& c)
{
	if ((c >= 'A') && (c <= 'Z')) { c = c + 0x20; return; }
	for (size_t i = 128; i <= 255; i++)
		if ((UpcCharTab[i] == c) && (i != c)) { c = i; return; }
}

bool BlockHandle(int& fs, FILE* W1, char Oper)
{
	WORD i, I1;
	int LL1, LL2;
	ColorOrd co;
	bool isPrintFile = false;
	char* p = nullptr;
	bool tb; char c;

	TestKod();
	int Ln = blocks->LineAbs(TextLineNr);
	WORD Ps = positionOnActualLine;
	if (Oper == 'p') {
		tb = TypeB;
		TypeB = TextBlock;
	}
	else
		if (!BlockExist()) { return false; }
	screen.CrsHide();
	auto result = true;
	if (TypeB == TextBlock) {
		WORD I2;
		if (Oper == 'p') {
			LL2 = AbsLenT + LenT; // -Part.LenP;
			LL1 = SetInd(T, LenT, textIndex, positionOnActualLine); // +Part.PosP;
		}
		else {
			SetBlockBound(LL1, LL2);
		}
		I1 = LL1; // -Part.PosP;
		if (toupper(Oper) == 'P') {
			ResetPrint(Oper, fs, W1, LL2 - LL1, &co, I1, isPrintFile, p);
		}
		do {
			if (LL2 > /*Part.PosP +*/ LenT) I2 = LenT;
			else I2 = LL2; // -Part.PosP;
			switch (Oper) {
			case 'Y': {
				//TestLenText(&T, LenT, I2, I1);
				UpdatT = true;
				LL2 -= I2 - I1;
				break;
			}
			case 'U': {
				for (i = I1; i <= I2 - 1; i++) T[i] = UpcCharTab[T[i]];
				LL1 += I2 - I1;
				break;
			}
			case 'L': {
				for (i = I1; i <= I2 - 1; i++) LowCase(T[i]);
				LL1 += I2 - I1;
				break;
			}
			case 'p':
			case 'P': {
				if (isPrintFile) {
					WriteH(W1, I2 - I1, &T[I1]);
					HMsgExit(CPath);
				}
				else {
					Move(&T[I1], &p[fs + 1], I2 - I1);
					fs += I2 - I1; LL1 += I2 - I1;
				}
				break;
			}
			case 'W': {
				SeekH(W1, fs);
				WriteH(W1, I2 - I1, &T[I1]);
				HMsgExit(CPath);
				fs += I2 - I1;
				LL1 += I2 - I1;
				break;
			}
			}
			if (Oper == 'U' || Oper == 'L' || Oper == 'Y') {
				//SetUpdat();
				UpdatT = true;
			}
			if ((Oper == 'p') /* && AllRd*/) LL1 = LL2;
			//if (!AllRd && (LL1 < LL2))
			//{
			//	I1 = LenT;
			//	NextPart();
			//	//I1 -= Part.MovI;
			//}
		} while (LL1 != LL2);
	}
	else              /*ColBlock*/
	{
		PosDekFindLine(blocks->BegBLn, blocks->BegBPos, false);
		I1 = blocks->EndBPos - blocks->BegBPos;
		LL1 = (blocks->EndBLn - blocks->BegBLn + 1) * (I1 + 2);
		LL2 = 0;
		if (Oper == 'P') ResetPrint(Oper, fs, W1, LL1, &co, I1, isPrintFile, p);
		do {
			switch (Oper) {
			case 'Y': { TestLastPos(blocks->EndBPos, blocks->BegBPos); break; }
			case 'U': {
				for (i = blocks->BegBPos; i <= blocks->EndBPos - 1; i++) {
					Arr[i] = UpcCharTab[Arr[i]];
				}
				UpdatedL = true;
				break;
			}
			case 'L': {
				for (i = blocks->BegBPos; i <= blocks->EndBPos - 1; i++) {
					LowCase(Arr[i]);
				}
				UpdatedL = true;
				break;
			}
			case 'W':
			case 'P': {
				char* a = nullptr;
				Move(&Arr[blocks->BegBPos], a, I1);
				a[I1 + 1] = _CR;
				a[I1 + 2] = _LF;
				if ((Oper == 'P') && !isPrintFile) {
					Move(a, &p[fs + 1], I1 + 2);
				}
				else {
					WriteH(W1, I1 + 2, a);
					HMsgExit(CPath);
				}
				fs += I1 + 2;
				break;
			}
			}
			LL2 += I1 + 2;
			NextLine(false);
		} while (LL2 != LL1);

	}
	if (toupper(Oper) == 'P') {
		if (isPrintFile) {
			WriteH(W1, 0, T);/*truncH*/
			PrintFandWork();
		}
		else {
			PrintArray(p, fs, false);
			delete[] p; p = nullptr;
		}
	}
	if (Oper == 'p') { TypeB = tb; }
	if (Oper == 'Y') { PosDekFindLine(blocks->BegBLn, blocks->BegBPos, true); }
	else {
		if (Oper == 'p') SetPart(1);
		PosDekFindLine(Ln, Ps, true);
	}
	if (!bScroll) screen.CrsShow();
	return result;
}

void DelStorClpBd(void* P1, LongStr* sp)
{
	TWork.Delete(ClpBdPos);
	ClpBdPos = TWork.Store(sp->A, sp->LL);
	ReleaseStore(&P1);
}

void MarkRdClpBd(void* P1, LongStr* sp)
{
	MarkStore(P1);
	sp = TWork.Read(ClpBdPos);
}

void MovePart(WORD Ind)
{
	if (TypeT != FileT) return;
	TestUpdFile();
	WrEndT();
	{
		//Part.MovI = CurrentLineFirstCharIndex(Ind) - 1;
		//Part.MovL = GetLine(Part.MovI) - 1;
		//Part.LineP += Part.MovL;
		//Part.PosP += Part.MovI;
		//Part.LenP -= Part.MovI;
		//SetColorOrd(Part.ColorP, 1, Part.MovI + 1);
		//TestLenText(&T, LenT, Part.MovI + 1, 1);
		ChangePart = true;
	}
}

bool BlockGrasp(char Oper, void* P1, LongStr* sp)
{
	int L, L1, L2, ln;
	WORD I1;
	auto result = false;
	if (!BlockExist()) return result;
	L = /*Part.PosP +*/ textIndex + positionOnActualLine - 1;
	ln = blocks->LineAbs(TextLineNr); if (Oper == 'G') TestKod();
	SetBlockBound(L1, L2);
	if ((L > L1) and (L < L2) and (Oper != 'G')) return result;
	L = L2 - L1; if (L > 0x7FFF) { WrLLF10Msg(418); return result; }
	if (L2 > /*Part.PosP +*/ LenT) MovePart(L1 /* - Part.PosP*/);
	I1 = L1 /* - Part.PosP*/;
	MarkStore(P1);
	sp = new LongStr(L + 2);
	sp->LL = L;
	Move(&T[I1], sp->A, L);
	if (Oper == 'M') {
		//TestLenText(&T, LenT, I1 + L, I1);
		UpdatT = true;
		/*   if (L1>Part.PosP+I1) dec(L1,L);*/
		if (blocks->EndBLn <= ln)
		{
			if ((blocks->EndBLn == ln) && (positionOnActualLine >= blocks->EndBPos))
				positionOnActualLine = blocks->BegBPos + positionOnActualLine - blocks->EndBPos;
			ln -= blocks->EndBLn - blocks->BegBLn;
		}
	}
	if (Oper == 'G') DelStorClpBd(P1, sp);
	PosDekFindLine(ln, positionOnActualLine, false);
	result = true;
	return result;
}

void BlockDrop(char Oper, void* P1, LongStr* sp)
{
	WORD I, I2;
	if (Oper == 'D') MarkRdClpBd(P1, sp); if (sp->LL == 0) return;
	/* hlidani sp->LL a StoreAvail, MaxLenT, dela TestLenText, prip.SmallerPart */
	if (Oper == 'D') FillBlank();
	I = textIndex + positionOnActualLine - 1; I2 = sp->LL;
	blocks->BegBLn = blocks->LineAbs(TextLineNr);
	blocks->BegBPos = positionOnActualLine;
	//NullChangePart();
	//TestLenText(&T, LenT, I, int(I) + I2);
	UpdatT = true;
	//if (ChangePart) I -= Part.MovI;
	Move(sp->A, &T[I], I2);
	ReleaseStore(&P1);
	TextLineNr = GetLineNumber(I + I2);
	blocks->EndBLn = /*Part.LineP +*/ TextLineNr;
	blocks->EndBPos = succ(I + I2 - textIndex);
	PosDekFindLine(blocks->BegBLn, blocks->BegBPos, true); /*ChangeScr = true;*/
}

bool BlockCGrasp(char Oper, void* P1, LongStr* sp)
{
	WORD i, I2;
	int L;
	char* a = nullptr;

	auto result = false;
	if (!BlockExist()) return result;
	TestKod();
	L = blocks->LineAbs(TextLineNr);
	if ((L >= blocks->BegBLn && L <= blocks->EndBLn) && (positionOnActualLine >= blocks->BegBPos + 1 && positionOnActualLine <= blocks->EndBPos - 1) && (Oper != 'G')) return result;
	int l1 = (blocks->EndBLn - blocks->BegBLn + 1) * (blocks->EndBPos - blocks->BegBPos + 2);
	if (l1 > 0x7FFF) { WrLLF10Msg(418); return result; }
	MarkStore(P1);
	sp = new LongStr(l1 + 2);
	sp->LL = l1;
	PosDekFindLine(blocks->BegBLn, positionOnActualLine, false);
	I2 = 0;
	i = blocks->EndBPos - blocks->BegBPos;
	do {
		Move(&Arr[blocks->BegBPos], a, i); a[i + 1] = _CR; a[i + 2] = _LF;
		if (Oper == 'M') TestLastPos(blocks->EndBPos, blocks->BegBPos);
		Move(a, &sp->A[I2 + 1], i + 2); I2 += i + 2;
		TestKod();
		NextLine(false);
	} while (I2 != sp->LL);
	if ((Oper == 'M') && (L >= blocks->BegBLn && L <= blocks->EndBLn) && (positionOnActualLine > blocks->EndBPos)) {
		positionOnActualLine -= blocks->EndBPos - blocks->BegBPos;
	}
	if (Oper == 'G') DelStorClpBd(P1, sp); PosDekFindLine(L, positionOnActualLine, false);
	result = true;
	return result;
}

void InsertLine(WORD& i, WORD& I1, WORD& I3, WORD& ww, LongStr* sp)
{
	i = MinW(I1 - I3, LineMaxSize - GetArrLineLength());
	if (i > 0) {
		TestLastPos(ww, ww + i);
		Move(&sp->A[I3], &Arr[ww], i);
	}
	TestKod();
}

void BlockCDrop(char Oper, void* P1, LongStr* sp)
{
	WORD i, I1, I3, ww;
	if (Oper == 'D') MarkRdClpBd(P1, sp);
	if (sp->LL == 0) return;
	/* hlidani sp->LL a StoreAvail, MaxLenT
		dela NextLine - prechazi mezi segmenty */
	if (Oper != 'R') {
		blocks->EndBPos = positionOnActualLine;
		blocks->BegBPos = positionOnActualLine;
		blocks->BegBLn = TextLineNr /* + Part.LineP*/;
	}
	ww = blocks->BegBPos; I1 = 1; I3 = 1;
	do {
		if (sp->A[I1] == _CR) {
			InsertLine(i, I1, I3, ww, sp);
			ww = blocks->BegBPos; blocks->EndBPos = MaxW(ww + i, blocks->EndBPos);
			if ((NextLineStartIndex > LenT) && ((TypeT != FileT) || true /*AllRd*/)) {
				//TestLenText(&T, LenT, LenT, (int)LenT + 2);
				UpdatT = true;
				T[LenT - 2] = _CR;
				T[LenT - 1] = _LF;
				NextLineStartIndex = LenT;
			}
			NextLine(false);
		}
		if (sp->A[I1] == _CR || sp->A[I1] == _LF || sp->A[I1] == 0x1A) I3 = I1 + 1;
		I1++;
	} while (I1 <= sp->LL);
	if (I3 < I1) InsertLine(i, I1, I3, ww, sp);
	if (Oper != 'R') {
		blocks->EndBLn = /*Part.LineP +*/ TextLineNr - 1;
		ReleaseStore(&P1);
		PosDekFindLine(blocks->BegBLn, blocks->BegBPos, true);
	}
}

void BlockCopyMove(char Oper, void* P1, LongStr* sp)
{
	bool b;
	if (!BlockExist()) return;
	FillBlank();
	if (TypeB == TextBlock) {
		if (BlockGrasp(Oper, P1, sp)) { BlockDrop(Oper, P1, sp); }
	}
	else if (BlockCGrasp(Oper, P1, sp)) { BlockCDrop(Oper, P1, sp); }
}

bool ColBlockExist()
{
	bool b;
	if ((TypeB == ColBlock) && (blocks->BegBPos == blocks->EndBPos) && (blocks->BegBLn < blocks->EndBLn)) return true;
	else return BlockExist();
}

void NewBlock1(WORD& I1, int& L2)
{
	if (I1 != positionOnActualLine) {
		blocks->BegBLn = L2;
		blocks->EndBLn = L2;
		blocks->BegBPos = MinW(I1, positionOnActualLine);
		blocks->EndBPos = MaxW(I1, positionOnActualLine);
	}
}

void BlockLRShift(WORD I1)
{
	if (!bScroll && (Mode != HelpM) && ((KbdFlgs & 0x03) != 0))   /*Shift*/
	{
		int L2 = blocks->LineAbs(TextLineNr);
		if (!ColBlockExist()) NewBlock1(I1, L2);
		else
			switch (TypeB) {
			case TextBlock: {
				if ((blocks->BegBLn == blocks->EndBLn) && (L2 == blocks->BegBLn) && (blocks->EndBPos == blocks->BegBPos)
					&& (I1 == blocks->BegBPos)) if (I1 > positionOnActualLine) blocks->BegBPos = positionOnActualLine;
					else blocks->EndBPos = positionOnActualLine;
				else if ((L2 == blocks->BegBLn) && (I1 == blocks->BegBPos)) blocks->BegBPos = positionOnActualLine;
				else if ((L2 == blocks->EndBLn) && (I1 == blocks->EndBPos)) blocks->EndBPos = positionOnActualLine;
				else NewBlock1(I1, L2);
				break;
			}
			case ColBlock: {
				if ((L2 >= blocks->BegBLn) && (L2 <= blocks->EndBLn))
					if ((blocks->EndBPos == blocks->BegBPos) && (I1 == blocks->BegBPos))
						if (I1 > positionOnActualLine) blocks->BegBPos = positionOnActualLine;
						else blocks->EndBPos = positionOnActualLine;
					else if (I1 == blocks->BegBPos) blocks->BegBPos = positionOnActualLine;
					else if (I1 == blocks->EndBPos) blocks->EndBPos = positionOnActualLine;
					else NewBlock1(I1, L2);
				else NewBlock1(I1, L2);
				break;
			}
			}
	}
}

void NewBlock2(int& L1, int& L2)
{
	if (L1 != L2) {
		blocks->BegBPos = positionOnActualLine; blocks->EndBPos = positionOnActualLine;
		blocks->BegBLn = MinL(L1, L2); blocks->EndBLn = MaxL(L1, L2);
	}
}

void BlockUDShift(int L1)
{
	int L2;
	if (!bScroll && (Mode != HelpM) && ((KbdFlgs & 0x03) != 0))   /*Shift*/
	{
		L2 = blocks->LineAbs(TextLineNr);
		if (!ColBlockExist()) NewBlock2(L1, L2);
		else {
			switch (TypeB) {
			case TextBlock: {
				if ((blocks->BegBLn == blocks->EndBLn) && (L1 == blocks->BegBLn))
					if ((positionOnActualLine >= blocks->BegBPos) && (positionOnActualLine <= blocks->EndBPos))
						if (L1 < L2) { blocks->EndBLn = L2; blocks->EndBPos = positionOnActualLine; }
						else { blocks->BegBLn = L2; blocks->BegBPos = positionOnActualLine; }
					else NewBlock2(L1, L2);
				else if ((L1 == blocks->BegBLn) && (blocks->BegBPos == positionOnActualLine)) blocks->BegBLn = L2;
				else if ((L1 == blocks->EndBLn) && (blocks->EndBPos == positionOnActualLine)) blocks->EndBLn = L2;
				else NewBlock2(L1, L2);
				break;
			}
			case ColBlock:
				if ((positionOnActualLine >= blocks->BegBPos) && (positionOnActualLine <= blocks->EndBPos))
					if ((blocks->BegBLn == blocks->EndBLn) && (L1 == blocks->BegBLn))
						if (L1 < L2) blocks->EndBLn = L2;
						else blocks->BegBLn = L2;
					else if (L1 == blocks->BegBLn) blocks->BegBLn = L2;
					else if (L1 == blocks->EndBLn) blocks->EndBLn = L2;
					else NewBlock2(L1, L2);
				else NewBlock2(L1, L2);
			}
		}
	}
}

bool MyPromptLL(WORD n, std::string& s)
{
	wwmix ww;
	ww.PromptLL(n, s, 1, true);
	return Event.Pressed.KeyCombination() == __ESC;
}

void ChangeP(WORD& fst)
{
	if (ChangePart) {
		//if (fst <= Part.MovI) fst = 1;
		//else fst -= Part.MovI;
		/* if (Last>Part.PosP+LenT) lst = LenT-1 else lst = Last-Part.PosP; */
		//NullChangePart();
	}
}

void SetScreen(WORD Ind, WORD ScrXY, WORD Pos)
{
	TextLineNr = GetLineNumber(Ind);
	positionOnActualLine = MinI(LineMaxSize, MaxI(MaxW(1, Pos), Ind - textIndex + 1));
	if (ScrXY > 0) {
		ScreenFirstLineNr = TextLineNr - (ScrXY >> 8) + 1;
		positionOnActualLine = MaxW(positionOnActualLine, ScrXY & 0x00FF);
		BPos = positionOnActualLine - (ScrXY & 0x00FF);
		ChangeScr = true;
	}
	Colu = Column(positionOnActualLine);
	columnOffset = Column(BPos);
	if (bScroll) {
		RScrL = NewRL(ScreenFirstLineNr);
		TextLineNr = MaxI(PHNum + 1, blocks->LineAbs(TextLineNr)); // -Part.LineP;
		int rl = NewRL(TextLineNr);
		if ((rl >= RScrL + PageS) || (rl < RScrL)) {
			if (rl > 10) RScrL = rl - 10;
			else RScrL = 1;
			ChangeScr = true; ScreenFirstLineNr = NewL(RScrL);
		}
		TextLineNr = ScreenFirstLineNr;
		DekFindLine(blocks->LineAbs(TextLineNr));
	}
	else {
		if ((TextLineNr >= ScreenFirstLineNr + PageS) || (TextLineNr < ScreenFirstLineNr)) {
			if (TextLineNr > 10) ScreenFirstLineNr = TextLineNr - 10;
			else ScreenFirstLineNr = 1;
			ChangeScr = true;
		}
	}
}

void ReplaceString(WORD& J, WORD& fst, WORD& lst, int& Last)
{
	size_t r = ReplaceStr.length();
	size_t f = FindStr.length();
	//TestLenText(&T, LenT, J, int(J) + r - f);
	UpdatT = true;
	ChangeP(fst);
	//if (TestLastPos(positionOnActualLine, positionOnActualLine + r - f));
	if (!ReplaceStr.empty()) Move(&ReplaceStr[1], &T[J - f], r);
	J += r - f;
	SetScreen(J, 0, 0);
	lst += r - f;
	Last += r - f;
}

char MyVerifyLL(WORD n, pstring s)
{
	char cc;
	WORD c2 = screen.WhereX() + FirstC - 1;
	WORD r2 = screen.WhereY() + FirstR;
	int w = PushW(1, 1, TxtCols, TxtRows);
	screen.GotoXY(1, TxtRows);
	TextAttr = screen.colors.pTxt;
	ClrEol();
	SetMsgPar(s);
	WriteMsg(n);
	WORD c1 = screen.WhereX();
	WORD r1 = screen.WhereY();
	TextAttr = screen.colors.pNorm;
	printf(" ");
	screen.CrsNorm();
	int t = Timer + 15;
	WORD r = r1;
	do {
		while (!KbdPressed())
			if (Timer >= t) {
				t = Timer + 15;
				if (r == r1) { screen.GotoXY(c2, r2); r = r2; }
				else { screen.GotoXY(c1, r1); r = r1; }
			}
		cc = toupper(ReadKbd());
	} while (!(cc == AbbrYes || cc == AbbrNo || cc == __ESC));
	PopW(w);
	return cc;
}

void FindReplaceString(int First, int Last)
{
	WORD lst;
	if (First >= Last) {
		if ((TypeT == MemoT) && TestOptStr('e')) {
			SrchT = true;
			Konec = true;
		}
		return;
	}
	FirstEvent = false;
	SetPart(First);
	WORD fst = First; // -Part.PosP;
	//NullChangePart();
label1:
	if (Last > /*Part.PosP +*/ LenT) lst = LenT - 1;
	else lst = Last; // -Part.PosP;
	ChangeP(fst);            /* Background muze volat NextPart */
	if (FindString(fst, lst)) {
		SetScreen(fst, 0, 0);
		if (Replace) {
			if (TestOptStr('n')) {
				ReplaceString(fst, fst, lst, Last);
				UpdStatLine(TextLineNr, positionOnActualLine, Mode);/*BackGround*/
			}
			else {
				FirstEvent = true;
				Background();
				FirstEvent = false;
				char c = MyVerifyLL(408, "");
				if (c == AbbrYes) ReplaceString(fst, fst, lst, Last);
				else if (c == _ESC) return;
			}
			if (TestOptStr('g') || TestOptStr('e') || TestOptStr('l')) goto label1;
		}
	}
	else {                       /* !FindString */
		//if (!AllRd && (Last > Part.PosP + LenT)) {
		//	NextPart();
		//	goto label1;
		//}
		//else {
		if (TestOptStr('e') && (TypeT == MemoT)) {
			SrchT = true; Konec = true;
		}
		else {
			SetScreen(lst, 0, 0);
		}
		//}
	}
	/* BackGround; */
}

WORD WordNo(WORD I)
{
	return (CountChar(T, LenT, 0x13 /* ^S */, 1, MinW(LenT, I)) + 1) / 2;
}

bool WordExist()
{
	return (WordL >= ScreenFirstLineNr) && (WordL < ScreenFirstLineNr + PageS);
}

WORD WordNo2()
{
	WORD wNo;
	bool wExists = WordExist();

	if (wExists) {
		wNo = WordNo(SetInd(T, LenT, textIndex, positionOnActualLine));
	}
	else {
		wNo = WordNo(ScreenIndex + 1);
	}

	return wNo;
}

void ClrWord()
{
	//WORD k = 0;
	//k = FindCharPosition(T, LenT, 0x11, k);
	//while (k < LenT) {
	//	T[k] = 0x13;
	//	k = FindCharPosition(T, LenT, 0x11, k) + 1;
	//}
}

bool WordFind(WORD i, WORD& WB, short& WE, WORD& LI)
{
	bool result = false;
	if (i == 0) return result;
	i = i * 2 - 1;
	WORD k = FindCharPosition(T, LenT, 0x13, i - 1);
	if (k >= LenT) return result;
	WB = k - 1;
	k++;
	while (T[k] != 0x13) {
		k++;
	}
	if (k >= LenT) return result;
	WE = k;
	LI = GetLine(WB) + 1;
	result = true;
	return result;
}

void SetWord(WORD WB, WORD WE)
{
	T[WB] = 0x11;
	T[WE] = 0x11;
	TextLineNr = GetLineNumber(WB);
	WordL = TextLineNr;
	positionOnActualLine = WB - textIndex + 1;
	Colu = Column(positionOnActualLine);
}

void HelpLU(char dir)
{
	WORD I = 0, I1 = 0, h1 = 0, h2 = 0;
	short I2 = 0;
	ClrWord();
	h1 = WordNo2();
	if (dir == 'U') {
		DekFindLine(TextLineNr - 1);
		positionOnActualLine = Position(Colu);
		h2 = MinW(h1, WordNo2() + 1);
	}
	else {
		h2 = h1;
	}
	if (WordFind(h2, I1, I2, I) && (I >= ScreenFirstLineNr - 1)) {
		SetWord(I1, I2);
	}
	else {
		if (WordFind(h1 + 1, I1, I2, I) && (I >= ScreenFirstLineNr)) SetWord(I1, I2);
		else { I1 = SetInd(T, LenT, textIndex, positionOnActualLine); WordL = 0; }
		I = ScreenFirstLineNr - 1;
	}
	if (I <= ScreenFirstLineNr - 1) {
		DekFindLine(ScreenFirstLineNr);
		RollPred();
	}
	if (WordExist()) {
		TextLineNr = GetLineNumber(I1);
	}
}

void HelpRD(char dir)
{
	WORD I = 0, I1 = 0, h1 = 0, h2 = 0;
	short I2 = 0;
	ClrWord();
	h1 = WordNo2();
	if (WordExist()) {
		h1++;
	}
	if (dir == 'D') {
		NextLine(false);
		positionOnActualLine = Position(Colu);
		while ((positionOnActualLine > 0) && (Arr[positionOnActualLine] != 0x13)) positionOnActualLine--;
		positionOnActualLine++;
		h2 = MaxW(h1 + 1, WordNo2() + 1);
	}
	else {
		h2 = h1 + 1;
	}
	if (WordFind(h2, I1, I2, I) && (I <= ScreenFirstLineNr + PageS)) {
		SetWord(I1, I2);
	}
	else {
		if (WordNo2() > h1) {
			h1++;
		}
		if (WordFind(h1, I1, I2, I) && (I <= ScreenFirstLineNr + PageS)) {
			SetWord(I1, I2);
		}
		else {
			I1 = SetInd(T, LenT, textIndex, positionOnActualLine); WordL = 0;
		}
		I = ScreenFirstLineNr + PageS;
	}
	if (I >= ScreenFirstLineNr + PageS) {
		DekFindLine(ScreenFirstLineNr + PageS - 1);
		RollNext();
	}
	if (WordExist()) {
		TextLineNr = GetLineNumber(I1);
	}
}

void CursorWord()
{
	std::set<char> O;

	LexWord = "";
	WORD pp = positionOnActualLine;
	if (Mode == HelpM) O.insert(0x11);
	else {
		O = Separ;
		if (O.count(Arr[pp]) > 0) { pp--; }
	}
	while ((pp > 0) && !O.count(Arr[pp])) { pp--; }
	pp++;
	while ((pp <= GetArrLineLength()) && !O.count(Arr[pp])) {
		LexWord = LexWord + Arr[pp];
		pp++;
	}
}

void Edit(std::vector<EdExitD*>& ExitD, std::vector<WORD>& breakKeys)
{
	blocks = new Blocks();
	InitScr();
	IsWrScreen = false;
	WrEndT();
	IndexT = min(IndexT, LenT - 1);
	blocks->BegBLn = 1;
	blocks->EndBLn = 1;
	blocks->BegBPos = 1;
	blocks->EndBPos = 1;
	ScreenFirstLineNr = 1;
	ScreenIndex = 0;
	RScrL = 1;
	PredScLn = 1;
	PredScPos = 1;
	UpdPHead = false;
	if (TypeT != FileT) {
		//AllRd = true;
		AbsLenT = LenT - 1;
		//Part.LineP = 0;
		//Part.PosP = 0;
		//Part.LenP = (WORD)AbsLenT;
		//Part.ColorP = "";
		//Part.UpdP = false;
		//NullChangePart();
		SimplePrintHead();
	}

	FirstScroll = Mode == ViewM;
	const bool keybScroll = GetKeyState(VK_SCROLL) & 0x0001;
	bScroll = (keybScroll || FirstScroll) && (Mode != HelpM);
	if (bScroll)
	{
		ScreenFirstLineNr = NewL(RScrL);
		ChangeScr = true;
	}
	HelpScroll = bScroll || (Mode == HelpM);
	if (HelpScroll) {
		screen.CrsHide();
	}
	else {
		screen.CrsNorm();
	}
	columnOffset = 0;
	BPos = 0;
	SetScreen(IndexT, ScrT, positionOnActualLine);
	Konec = false;

	if (Mode == HelpM) {
		WordL = 0;
		ScreenIndex = SetInd(T, LenT, textIndex, positionOnActualLine) - 1;
		if (WordFind(WordNo2() + 1, i1, i2, i3)) {
			SetWord(i1, i2);
		}
		if (!WordExist()) {
			TextLineNr = GetLineNumber(IndexT);
			ScreenIndex = 0;
		}
	}
	FillChar((char*)MargLL, sizeof(MargLL), 0);
	//ColScr = Part.ColorP;
	WrStatusLine();
	TextAttr = TxtColor;
	ClrScr();
	Background();
	FirstEvent = false;

	// {!!!!!!!!!!!!!!}
	if (!ErrMsg.empty()) {
		SetMsgPar(ErrMsg);
		F10SpecKey = 0xffff;
		WrLLF10Msg(110);
		ClearKbdBuf();
		AddToKbdBuf(Event.Pressed.KeyCombination());
	}
	FillChar((char*)MargLL, sizeof(MargLL), 0);
	WrLLMargMsg(LastS, LastNr);

	do {
		//if (TypeT == FileT) {
		//	NullChangePart();
		//}
		HandleEvent(Mode, IsWrScreen, SysLColor, LastS, LastNr, ExitD, breakKeys);
		if (!(Konec || IsWrScreen)) {
			Background();
		}
	} while (!Konec);

	if (bScroll && (Mode != HelpM)) {
		positionOnActualLine = BPos + 1;
		TextLineNr = ScreenFirstLineNr;
		textIndex = ScreenIndex;
	}

	IndexT = SetInd(T, LenT, textIndex, positionOnActualLine);
	ScrT = ((TextLineNr - ScreenFirstLineNr + 1) << 8) + positionOnActualLine - BPos;
	if (Mode != HelpM) {
		TxtXY = ScrT + ((int)positionOnActualLine << 16);
		CursorWord();
		if (Mode == HelpM) { ClrWord(); }
	}
	screen.CrsHide();
	screen.Window(MinC, MinR, MaxC, MaxR);
	TestUpdFile();

	delete blocks;
	blocks = nullptr;
}

void SetEditTxt(Instr_setedittxt* PD)
{
	if (PD->Insert != nullptr) Insert = !RunBool(CFile, PD->Insert, CRecPtr);
	if (PD->Indent != nullptr) Indent = RunBool(CFile, PD->Indent, CRecPtr);
	if (PD->Wrap != nullptr) Wrap = RunBool(CFile, PD->Wrap, CRecPtr);
	if (PD->Just != nullptr) Just = RunBool(CFile, PD->Just, CRecPtr);
	if (PD->ColBlk != nullptr) TypeB = RunBool(CFile, PD->ColBlk, CRecPtr);
	if (PD->Left != nullptr) LeftMarg = MaxI(1, RunInt(CFile, PD->Left, CRecPtr));
	if (PD->Right != nullptr) RightMarg = MaxI(LeftMarg, MinI(255, RunInt(CFile, PD->Right, CRecPtr)));
}

void GetEditTxt(bool& pInsert, bool& pIndent, bool& pWrap, bool& pJust, bool& pColBlk, short& pLeftMarg, short& pRightMarg)
{
	pInsert = Insert; pIndent = Indent; pWrap = Wrap; pJust = Just; pColBlk = TypeB;
	pLeftMarg = LeftMarg; pRightMarg = RightMarg;
}

bool EditText(char pMode, char pTxtType, std::string pName, std::string pErrMsg, LongStr* pLS, WORD pMaxLen,
	WORD& pInd, int& pScr, std::vector<WORD>& break_keys, std::vector<EdExitD*>& pExD, bool& pSrch, bool& pUpdat, WORD pLastNr,
	WORD pCtrlLastNr, MsgStr* pMsgS)
{
	bool oldEdOK = EdOk; EditT = true;
	Mode = pMode;
	TypeT = pTxtType;
	NameT = pName;
	ErrMsg = pErrMsg;
	T = pLS->A;
	MaxLenT = pMaxLen;
	LenT = pLS->LL;
	IndexT = pInd;
	ScrT = pScr & 0xFFFF;
	positionOnActualLine = pScr >> 16;
	//Breaks = break_keys;
	//ExitD = pExD;
	SrchT = pSrch; UpdatT = pUpdat;
	LastNr = pLastNr; CtrlLastNr = pCtrlLastNr;
	if (pMsgS != nullptr) {
		LastS = pMsgS->Last;
		CtrlLastS = pMsgS->CtrlLast;
		ShiftLastS = pMsgS->ShiftLast;
		AltLastS = pMsgS->AltLast;
		HeadS = pMsgS->Head;
	}
	else {
		/*LastS = nullptr; CtrlLastS = nullptr; ShiftLastS = nullptr; AltLastS = nullptr; HeadS = nullptr;*/
		LastS = ""; CtrlLastS = ""; ShiftLastS = ""; AltLastS = ""; HeadS = "";
	}
	if (Mode != HelpM) TxtColor = TextAttr;
	FirstEvent = !SrchT;
	if (SrchT) {
		SrchT = false;
		keyboard.AddToFrontKeyBuf(0x0C); // '^L' .. '\f' .. #12
		/*pstring OldKbdBuffer = KbdBuffer;
		KbdBuffer = 0x0C;
		KbdBuffer += OldKbdBuffer;*/
		Event.Pressed.UpdateKey('L');
		IndexT = 0;
	}

	Edit(pExD, break_keys);
	if (Mode != HelpM) { TextAttr = TxtColor; }
	pUpdat = UpdatT;
	pSrch = SrchT;
	pLS->LL = LenT;
	pLS->A = T;
	pInd = IndexT;
	pScr = ScrT + ((int)positionOnActualLine << 16);
	EdOk = oldEdOK;
	return EditT;
}

void SimpleEditText(char pMode, std::string pErrMsg, std::string pName, LongStr* pLS, WORD MaxLen, WORD& Ind, bool& Updat)
{
	bool Srch = false;
	int Scr = 0;
	std::vector<WORD> emptyBreakKeys;
	std::vector<EdExitD*> emptyExitD;
	EditText(pMode, LocalT, std::move(pName), std::move(pErrMsg), pLS, MaxLen, Ind, Scr,
		emptyBreakKeys, emptyExitD, Srch, Updat, 0, 0, nullptr);
}

WORD FindTextE(const pstring& Pstr, pstring Popt, char* PTxtPtr, WORD PLen)
{
	auto* origT = T;
	T = (char*)PTxtPtr;
	pstring f = FindStr;
	pstring o = OptionStr;
	bool r = Replace;
	FindStr = Pstr;
	OptionStr = Popt;
	Replace = false;
	WORD I = 1;
	WORD result;
	if (FindString(I, PLen + 1)) result = I;
	else result = 0;
	FindStr = f;
	OptionStr = o;
	Replace = r;
	T = origT;
	return result;
}

void EditTxtFile(std::string* locVar, char Mode, std::string& ErrMsg, std::vector<EdExitD*>& ExD,
	int TxtPos, int Txtxy, WRect* V, WORD Atr,
	const std::string Hd, BYTE WFlags, MsgStr* MsgS)
{
	bool Srch = false, Upd = false;
	int Size = 0; // , L = 0;
	int w1 = 0;
	bool Loc = false;
	WORD Ind = 0, oldInd = 0;
	int oldTxtxy = 0;
	LongStr* LS = nullptr;
	pstring compErrTxt;

	if (Atr == 0) {
		Atr = screen.colors.tNorm;
	}
	int w2 = 0;
	int w3 = 0;
	if (V != nullptr) {
		w1 = PushW(1, 1, TxtCols, 1, (WFlags & WPushPixel) != 0, false);
		w2 = PushW(1, TxtRows, TxtCols, TxtRows, (WFlags & WPushPixel) != 0, false);
		w3 = PushWFramed(V->C1, V->R1, V->C2, V->R2, Atr, Hd, "", WFlags);
	}
	else {
		w1 = PushW(1, 1, TxtCols, TxtRows);
		TextAttr = Atr;
	}

	try {
		Loc = (locVar != nullptr);
		//LocalPPtr = locVar;
		if (!Loc) {
			MaxLenT = 0xFFF0; LenT = 0;
			//Part.UpdP = false;
			TxtPath = CPath; TxtVol = CVol;
			// zacatek prace se souborem
			OpenTxtFh(Mode);
			ReadTextFile();
			SimplePrintHead();
			//while ((TxtPos > Part.PosP + Part.LenP) && !AllRd) {
			//	RdNextPart();
			//}
			Ind = TxtPos; // -Part.PosP;
		}
		else {
			LS = new LongStr(locVar->length()); // TWork.Read(1, *LP);
			LS->LL = locVar->length();
			Ind = TxtPos;
			memcpy(LS->A, locVar->c_str(), LS->LL);
			//L = StoreInTWork(LS);
		}
		oldInd = Ind;
		oldTxtxy = Txtxy;

		while (true) {
			Srch = false;
			Upd = false;
			if (!Loc) {
				LongStr* LS2 = new LongStr(T, LenT);
				std::vector<WORD> brkKeys = { __F1, __F6, __F9, __ALT_F10 };
				EditText(Mode, FileT, TxtPath, ErrMsg, LS2, 0xFFF0, Ind, Txtxy,
					brkKeys, ExD, Srch, Upd, 126, 143, MsgS);
				T = LS2->A;
				LenT = LS2->LL;
			}
			else {
				std::vector<WORD> brkKeys = { __F1, __F6 };
				EditText(Mode, LocalT, "", ErrMsg, LS, MaxLStrLen, Ind, Txtxy,
					brkKeys, ExD, Srch, Upd, 126, 143, MsgS);
			}
			TxtPos = Ind; // +Part.PosP;
			if (Upd) EdUpdated = true;
			WORD KbdChar = Event.Pressed.KeyCombination();
			if ((KbdChar == __ALT_EQUAL) || (KbdChar == 'U')) {
				// UNDO CHANGES
				//ReleaseStore(LS);
				//LS = TWork.Read(1, L);
				delete LS;
				LS = new LongStr(locVar->length()); // TWork.Read(1, *LP);
				memcpy(LS->A, locVar->c_str(), LS->LL);

				if (KbdChar == __ALT_EQUAL) {
					Event.Pressed.UpdateKey(__ESC);
					goto label4;
				}
				else {
					Ind = oldInd;
					Txtxy = oldTxtxy;
					continue;
				}
			}
			if (!Loc) {
				delete[] T;
				T = nullptr;
			}
			if (EdBreak == 0xFFFF) {
				switch (KbdChar) {
				case __F9: {
					if (Loc) {
						//TWork.Delete(*LP);
						//*LP = StoreInTWork(LS);
						*locVar = std::string(LS->A, LS->LL);
					}
					else {
						//RdPart();
					}
					continue;
				}
				case __F10: {
					if (Event.Pressed.Alt()) {
						Help(nullptr, "", false);
						goto label2;
					}
					break;
				}
				case __F1: {
					ReadMessage(6);
					Help((RdbD*)HelpFD, MsgLine, false);
				label2:
					if (!Loc) {
						// RdPart();
					}
					continue;
				}
				}
			}
			if (!Loc) {
				//Size = FileSizeH(TxtFH);
				Size = GetFileSize(TxtFH, NULL);
				//CloseH(&TxtFH);
				CloseHandle(TxtFH);
				TxtFH = NULL;
			}
			if ((EdBreak == 0xFFFF) && (KbdChar == __F6)) {
				if (Loc) {
					PrintArray(T, LenT, false);
					continue;
				}
				else {
					CPath = TxtPath;
					CVol = TxtVol;
					PrintTxtFile(0);
					OpenTxtFh(Mode);
					// RdPart();
					continue;
				}
			}
			if (!Loc && (Size < 1)) MyDeleteFile(TxtPath);
			if (Loc && (KbdChar == __ESC)) LS->LL = LenT;

		label4:
			if (IsCompileErr) {
				IsCompileErr = false;
				compErrTxt = MsgLine;
				SetMsgPar(compErrTxt);
				WrLLF10Msg(110);
			}
			if (Loc) {
				*locVar = std::string(LS->A, LS->LL);
				delete LS;
				LS = nullptr;
			}
			if (w3 != 0) {
				PopW(w3, (WFlags & WNoPop) == 0);
			}
			if (w2 != 0) {
				PopW(w2);
			}
			PopW(w1);
			LastTxtPos = Ind; // +Part.PosP;
			break;
		}
	}
	catch (std::exception& e) {
		// TODO: log error
	}
}

void ViewHelpText(std::string& s, WORD& TxtPos)
{
	// make copy of text from std::string because it changes in EditText()
	char* helpText = new char[s.length()];
	memcpy(helpText, s.c_str(), s.length());
	auto S = std::make_unique<LongStr>(helpText, s.length());

	stEditorParams ep;

	try {
		ep = SaveParams();
		TxtColor = screen.colors.hNorm;
		FillChar(ColKey, 8, screen.colors.tCtrl);
		ColKey[5] = screen.colors.hSpec;
		ColKey[3] = screen.colors.hHili;
		ColKey[1] = screen.colors.hMenu;
		bool Srch = false;
		bool Upd = false;
		int Scr = 0;
		while (true) {
			std::vector<WORD> brkKeys;
			brkKeys.push_back(__F1);
			brkKeys.push_back(__F6);
			brkKeys.push_back(__F10);
			brkKeys.push_back(__CTRL_HOME);
			brkKeys.push_back(__CTRL_END);
			std::vector<EdExitD*> emptyExitD;
			EditText(HelpM, MemoT, "", "", S.get(), 0xFFF0, TxtPos, Scr,
				brkKeys, emptyExitD, Srch, Upd, 142, 145, nullptr);
			if (Event.Pressed.KeyCombination() == __F6) {
				PrintArray(&S->A, S->LL, true);
				continue;
			}
			break;
		}
		RestoreParams(ep);
	}
	catch (std::exception& e)
	{
		RestoreParams(ep);
	}
}

void InitTxtEditor()
{
	FindStr = ""; ReplaceStr = "";
	OptionStr[0] = 0; Replace = false;
	TxtColor = screen.colors.tNorm;
	BlockColor = screen.colors.tBlock;
	SysLColor = screen.colors.fNorm;
	ColKey[0] = screen.colors.tCtrl;
	ColKey[1] = screen.colors.tUnderline;
	ColKey[2] = screen.colors.tItalic;
	ColKey[3] = screen.colors.tDWidth;
	ColKey[4] = screen.colors.tDStrike;
	ColKey[5] = screen.colors.tEmphasized;
	ColKey[6] = screen.colors.tCompressed;
	ColKey[7] = screen.colors.tElite;

	ReadMessage(411); InsMsg = MsgLine;
	ReadMessage(412); nInsMsg = MsgLine;
	ReadMessage(413); IndMsg = MsgLine;
	ReadMessage(414); WrapMsg = MsgLine;
	ReadMessage(415); JustMsg = MsgLine;
	ReadMessage(417); BlockMsg = MsgLine;
	ReadMessage(416); ViewMsg = MsgLine;
	Insert = true; Indent = true; Wrap = false; Just = false; TypeB = false;
	LeftMarg = 1; RightMarg = 78;
	CharPg = /*char(250)*/ spec.TxtCharPg;
	InsPg = /*true*/ spec.TxtInsPg;
}
