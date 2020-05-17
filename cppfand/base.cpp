#pragma once

#include "base.h"
#include <windows.h>
#include <errhandlingapi.h>
#include <fileapi.h>
#include "drivers.h"
#include "legacy.h"
#include <set>
#include <vector>
#include "obaseww.h"

/*const*/ char Version[] = { '4', '.', '2', '0', '\0' };

Video video;
Spec spec;
Fonts fonts;
Colors colors;

char CharOrdTab[256];
char UpcCharTab[256];
WORD TxtCols = 80;
WORD TxtRows = 25;

integer prCurr, prMax;

wdaystt WDaysTabType;
WORD NWDaysTab;
double WDaysFirst;
double WDaysLast;
wdaystt* WDaysTab;

char AbbrYes = 'Y';
char AbbrNo = 'N';

WORD HandleError;
pstring OldDir;
pstring FandDir;
pstring WrkDir;
pstring FandOvrName;
pstring FandResName;
pstring FandWorkName;
pstring FandWorkXName;
pstring FandWorkTName;
pstring CPath;
pstring CDir;
pstring CName;
pstring CExt;
pstring CVol;

TResFile ResFile;
TMsgIdxItem* MsgIdx;// = TMsgIdx;
WORD MsgIdxN;
longint FrstMsgPos;

WORD F10SpecKey; // �. 293
BYTE ProcAttr;
// bool SetStyleAttr(char c, BYTE& a); // je v KBDWW
pstring MsgLine;
pstring MsgPar[4];

WORD OldNumH; // r1 
void* OldHTPtr = nullptr;
#ifdef FandDemo
WORD files = 30;
#else
WORD files = 250; // {files in CONFIG.SYS -3}
#endif
WORD CardHandles;

WORD CachePageSize;
void* AfterCatFD; // r108
ProcStkD* MyBP;
ProcStkD* ProcMyBP;
WORD BPBound; // r212
bool ExitP, BreakP;
longint LastExitCode = 0; // r215
bool WasLPTCancel;
FILE* WorkHandle;
longint MaxWSize = 0; // {currently occupied in FANDWORK.$$$}
void* FandInt3f; // �. 311
FILE* OvrHandle;
WORD Fand_ss, Fand_sp, Fand_bp, DML_ss, DML_sp, DML_bp;
longint _CallDMLAddr = 0; // {passed to FANDDML by setting "DMLADDR="in env.}
Printer printer[10];
TPrTimeOut OldPrTimeOut;
TPrTimeOut PrTimeOut;  // absolute 0:$478;
bool WasInitDrivers = false;
bool WasInitPgm = false;
WORD LANNode; // �. 431
void (*CallOpenFandFiles)(); // r453
void (*CallCloseFandFiles)(); // r454

double userToday;
ExitRecord ExitBuf;

typedef FILE* filePtr;

std::set<FILE*> Handles;
std::set<FILE*> UpdHandles;
std::set<FILE*> FlshHandles;

//map<WORD, FILE*> fileMap;
// n�hrada za 'WORD OvrHandle = h - 1' - zji�t�n� p�edchoz�ho otev�en�ho souboru;
std::vector<FILE*> vOverHandle;

void SetMsgPar(pstring s)
{
	MsgPar[0] = s;
}

void Set2MsgPar(pstring s1, pstring s2)
{
	MsgPar[0] = s1;
	MsgPar[1] = s2;
}

void Set3MsgPar(pstring s1, pstring s2, pstring s3)
{
	Set2MsgPar(s1, s2);
	MsgPar[2] = s3;
}

void Set4MsgPar(pstring s1, pstring s2, pstring s3, pstring s4)
{
	Set3MsgPar(s1, s2, s3);
	MsgPar[3] = s4;
}

longint PosH(FILE* handle)
{
	const auto result = ftell(handle);
	HandleError = ferror(handle);
	return static_cast<longint>(result);
}

longint MoveH(longint dist, WORD method, FILE* handle)
{
	if (handle == nullptr) return -1;
	// dist - hodnota offsetu
	// method: 0 - od zacatku, 1 - od aktualni, 2 - od konce
	// handle - file handle
	HandleError = fseek(handle, dist, method);
	return ftell(handle);
}

int SeekH(FILE* handle, longint pos)
{
	if (handle == nullptr) RunError(705);
	return MoveH(pos, 0, handle);
}

WORD ReadH(FILE* handle, WORD bytes, void* buffer)
{
	size_t bufferSize = bytes; // sizeof(buffer);
	return fread_s(buffer, bufferSize, 1, bytes, handle);
	//return ReadH(handle, bytes, buffer);
	// read from file INT
	// bytes - po�et byte k p�e�ten�
	// vrac� - po�et skute�n� p�e�ten�ch
}

void RdMsg(integer N)
{
	WORD j, o;
	FILE* h;
	pstring s;
	for (int i = 1; i < MsgIdxN; i++) {
		auto Nr = MsgIdx[i].Nr;
		auto Count = MsgIdx[i].Count;
		auto Ofs = MsgIdx[i].Ofs;
		if (N >= Nr && N < Nr + Count)
		{
			j = N - Nr + 1;
			o = Ofs;
			goto label1;
		}
	}
	o = 0; j = 1;
	MsgPar[1] = std::to_string(N).c_str();

label1:
	h = ResFile.Handle;
	SeekH(h, FrstMsgPos + o);

	for (int i = 1; i <= j; i++)
	{
		ReadH(h, 1, &s[0]); // tady se m� z�ejm� jen vy��st d�lka
		ReadH(h, s.length(), &s[1]);
	}
	ConvKamenToCurr(&s[1], s.length());
	MsgLine = "";
	j = 1;
	s[s.length() + 1] = 0x00;
	for (int i = 1; i <= s.length(); i++) {
		if (s[i] == '$' && s[i + 1] != '$')
		{
			MsgLine += MsgPar[j];
			j++;
		}
		else
		{
			MsgLine.Append(s[i]);
			if (s[i] == '$') i++;
		}
	}
	//printf("MSG: %s\n", MsgLine.c_str());
}

void WriteMsg(WORD N)
{
}

void ClearLL(BYTE attr)
{
}

WORD TResFile::Get(WORD Kod, void** P)
{
	// CPP: kod-1 (indexujeme tady od 0) 
	WORD l = A[Kod - 1].Size;
	//GetMem(P, l);
	*P = new BYTE(l);
	auto sizeF = FileSizeH(Handle);
	auto seekF = SeekH(Handle, A[Kod - 1].Pos);
	auto readF = ReadH(Handle, l, *P);
	return l;
}

std::string TResFile::Get(WORD Kod)
{
	char* tmpCh = new char[A[Kod - 1].Size];
	WORD l = A[Kod - 1].Size;
	
	auto sizeF = FileSizeH(Handle);
	auto seekF = SeekH(Handle, A[Kod - 1].Pos);
	auto readF = ReadH(Handle, l, tmpCh);
	std::string result = tmpCh;
	delete[] tmpCh;
	return result;
}

void* GetStore(WORD Size)
{
	return nullptr;
}

void* GetZStore(WORD Size)
{
	return nullptr;
}

LongStrPtr TResFile::GetStr(WORD Kod)
{
	LongStrPtr s;
	/* !!! with A[Kod] do!!! */
	s = (LongStrPtr)GetStore(A[Kod].Size + 2); s->LL = A[Kod].Size;
	SeekH(Handle, A[Kod].Pos); ReadH(Handle, A[Kod].Size, s->A);
	return s;
}

WORD StackOvr()
{
	/*
	 procedure StackOvr(NewBP:word);
	type SF=record BP:word; case byte of 0:(Ret:pointer); 1:(RetOfs:word) end;
	var p,q:^SF;   pofs:word absolute p; qofs:word absolute q;
	r,t:^word; rofs:word absolute r; tofs:word absolute t;
	label 1;
	begin
	asm mov p.word,bp; mov p[2].word,ss end; pofs:=p^.BP;
	while pofs<NewBP do begin r:=p^.ret; pofs:=p^.BP;
	if (rofs=0) and (r^=$3fcd) then begin
	q:=ptr(SSeg,NewBP); inc(rofs,2);
	while qofs<BPBound do begin t:=q^.ret;
	if (seg(t^)=seg(r^)) and (tofs<>0) then begin
	   r^:=tofs; q^.retofs:=0; goto 1 end;
	qofs:=q^.BP end end;
	1:end;
	end;
	 */
	return 0;
}

void NoOvr()
{
	/*
		procedure NoOvr; far; assembler;
		asm   pop ax; pop ax; pop ax{bp}; push ax; push ax; call StackOvr;
			  pop bp; pop ds; pop ax; pop dx; pop sp; push dx; push ax;
		end;
	 */
}

bool CacheLocked = false; // r510

void AddBackSlash(pstring& s)
{
	if (s.empty()) { return; }
	if (s[s.length() - 1] == '\\') return;
	s += "\\";
}

void DelBackSlash(pstring& s)
{
	if (s.empty()) return;
	if (s[s.length() - 1] != '\\') return;
	s[s.length() - 1] = '\0';
	s[0] = s.length() - 1;
}

pstring StrPas(const char* Src)
{
	WORD n; pstring s;
	n = 0;
	while ((n < 255) && (Src[n] != '\0')) { s[n + 1] = Src[n]; n++; }
	s[0] = char(n);
	return s;
}

void ChainLast(Chained* Frst, Chained* New)
{
	// ve Frst najde postupn� poslen� v �et�zu a do n�j dosad� New
	// do New->Chain d� nullptr

	// z�ejm� hled� posledn� Chain a ten vrac�
	if (Frst == nullptr) { New = nullptr; return; }
	void* last = nullptr;
	uintptr_t mask = 0;
	while (true)
	{
		int result = memcmp(Frst, &mask, 4);
		if (result == 0) last = (void*)uintptr_t(last);
		else
		{
			New = last;
			return;
		}
	}
}

void* LastInChain(void* Frst)
{
	return nullptr;
}

void StrLPCopy(char* Dest, pstring s, WORD MaxL)
{
	auto sLen = s.length();
	int len = (sLen < MaxL) ? sLen : MaxL;
	Move((void*)s.c_str(), Dest, len);
}

integer MinI(integer X, integer Y)
{
	if (X < Y) return X;
	return Y;
}

integer MaxI(integer X, integer Y)
{
	if (X > Y) return X;
	return Y;
}

WORD MinW(WORD X, WORD Y)
{
	if (X < Y) return X;
	return Y;
}

WORD MaxW(WORD X, WORD Y)
{
	if (X > Y) return X;
	return Y;
}

longint MinL(longint X, longint Y)
{
	if (X < Y) return X;
	return Y;
}

longint MaxL(longint X, longint Y)
{
	if (X > Y) return X;
	return Y;
}

bool OlympYear(WORD year)
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

WORD OlympYears(WORD year)
{
	if (year < 3) return 0;
	year--;
	return year / 4 - year / 100 + year / 400;
}

void SplitDate(double R, WORD& d, WORD& m, WORD& y)
{
	WORD i, j;

	longint l = (longint)std::trunc(R);

	if (l == 0) { y = 1; m = 1; d = 1; }
	else {
		y = l / 365; y++; l = l % 365;
		while (l <= OlympYears(y)) { y--; l += 365; }
		l = l - OlympYears(y);
		for (j = 1; j <= 12; j++) {
			i = NoDayInMonth[j];
			if ((j == 2) && OlympYear(y)) i++;
			if (i >= l) goto label1;
			l -= i;
		}
	label1:
		m = j; d = l;
	}
}

double Today()
{
	return 0.0;
}

double CurrTime()
{
	return 0.0;
}

void wait()
{
}

bool MouseInRect(WORD X, WORD Y, WORD XSize, WORD Size)
{
	return false;
}

double ValDate(const pstring& Txtpstring, pstring Mask)
{
	return 0.0;
}

pstring StrDate(double R, pstring Mask)
{
	return "";
}

double AddMonth(double R, double RM)
{
	return 0.0;
}

double DifMonth(double R1, double R2)
{
	return 0.0;
}

void MyMove(void* A1, void* A2, WORD N)
{
}

FILE* GetOverHandle(FILE* fptr, int diff)
{
	ptrdiff_t pos = find(vOverHandle.begin(), vOverHandle.end(), fptr) - vOverHandle.begin();
	int newPos = pos + diff;
	if (newPos >= 0 && newPos < vOverHandle.size() - 1) { return vOverHandle[pos - 1]; }
	return nullptr;
}

bool IsHandle(FILE* H)
{
	if (H == nullptr) return false;
	return Handles.count(H) > 0;
}

bool IsUpdHandle(FILE* H)
{
	if (H == nullptr) return false;
	return UpdHandles.count(H) > 0;
}

bool IsFlshHandle(FILE* H)
{
	if (H == nullptr) return false;
	return FlshHandles.count(H) > 0;
}

void SetHandle(FILE* H)
{
	if (H == nullptr) return;
	Handles.insert(H);
	CardHandles++;
}

void SetUpdHandle(FILE* H)
{
	if (H == nullptr) return;
	UpdHandles.insert(H);
}

WORD SLeadEqu(pstring S1, pstring S2)
{
	return 0;
}

void SetFlshHandle(FILE* H)
{
	if (H == nullptr) return;
	FlshHandles.insert(H);
}

void ResetHandle(FILE* H)
{
	if (H == nullptr) return;
	Handles.erase(H);
	CardHandles--;
}

void ResetUpdHandle(FILE* H)
{
	if (H == nullptr) return;
	UpdHandles.erase(H);
}

void ResetFlshHandle(FILE* H)
{
	if (H == nullptr) return;
	FlshHandles.erase(H);
}

void ClearHandles()
{
	Handles.clear();
	CardHandles = 0;
}

void ClearUpdHandles()
{
	UpdHandles.clear();
}

void ClearFlshHandles()
{
	FlshHandles.clear();
}

bool IsNetCVol()
{
#ifdef FandNetV
	return CVol == "#" || CVol = "##" || SEquUpcase(CVol, "#R");
#else
	return false;
#endif
}

void ExtendHandles()
{
	// p�esouv� OldHTPtr na NewHT
}

void UnExtendHandles()
{
	// zav�e v�echny otev�en� soubory, p�esune zp�t NewHT do Old... prom�nn�ch
}

filePtr OpenH(FileOpenMode Mode, FileUseMode UM)
{
	// $3C vytvo�� nebo p�ep�e soubor
	// $3D otev�r� exituj�c� soubor
	// $5B vytvo�� nov� soubor - pokud ji� exituje, vyhod� chybu
	//
	// bit 0: read-only, 1: hidden, 2: system, 3: volume label, 4: reserved, must be zero (directory)
	//        5: archive bit, 7: if set, file is shareable under Novell NetWare
	//
	// p�i 'IsNetCVol' se chov� jinak
	// RdOnly $20, RdShared $40, Shared $42, Exclusive $12

	pstring s;

	pstring txt[] = { "Clos", "OpRd", "OpRs", "OpSh", "OpEx" };

	pstring path = CPath;
	if (CardHandles == files) RunError(884);
	longint w = 0;
	pstring openFlags(5);
label1:
	switch (Mode) {
	case _isoldfile:
	case _isoldnewfile:
	{
		openFlags = UM == RdOnly ? "rb" : "r+b";
		break;
	}
	case _isoverwritefile:
	{
		openFlags = "w+b";
		break;
	}
	case _isnewfile:
	{
		openFlags = "w+b"; // UM == RdOnly ? "w+b" : "w+b";
		break;
	}
	}

	FILE* nFile = nullptr;
	HandleError = fopen_s(&nFile, path.c_str(), openFlags.c_str());

	// https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-doserrno-sys-errlist-and-sys-nerr?view=vs-2019
	if (IsNetCVol() && (HandleError == EACCES || HandleError == ENOLCK))
	{
		if (w == 0)
		{
			Set2MsgPar(path, txt[UM]);
			w = PushWrLLMsg(825, false);
		}
		LockBeep();
		KbdTimer(spec.NetDelay, 0);
		goto label1;
	}

	if (HandleError == 0)
	{
		SetHandle(nFile);
		if (Mode != _isoldfile) SetUpdHandle(nFile);
	}

	else if (HandleError == ENOENT) // No such file or directory
	{
		if (Mode == _isoldnewfile)
		{
			Mode = _isnewfile;
			goto label1;
		}
	}
	if (w != 0) PopW(w);

	// p�id�n� FILE* od vektoru kv�li 'WORD OvrHandle = h - 1;'
	vOverHandle.push_back(nFile);
	return nFile;
}

WORD ReadLongH(filePtr handle, longint bytes, void* buffer)
{
	if (handle == nullptr) RunError(706);
	if (bytes <= 0) return 0;
	auto readed = fread_s(buffer, bytes, 1, bytes, handle);
	if (readed != static_cast<unsigned int>(bytes))
	{
		// nebyl na�ten po�adovan� po�et B
		auto eofReached = feof(handle);
		HandleError = ferror(handle);
	}
	return WORD(readed);
}

void WriteLongH(filePtr handle, longint bytes, void* buffer)
{
	if (handle == nullptr) RunError(706);
	if (bytes <= 0) return;
	// ulo�� do souboru dan� po�et Byt� z bufferu
	fwrite(buffer, 1, bytes, handle);
	HandleError = ferror(handle);
}

void WriteH(FILE* handle, WORD bytes, void* buffer)
{
	WriteLongH(handle, bytes, buffer);
}

longint FileSizeH(FILE* handle)
{
	longint pos = PosH(handle);
	auto result = MoveH(0, 2, handle);
	SeekH(handle, pos);
	return result;
}

longint SwapLong(longint N)
{
	return 0;
}

bool TryLockH(filePtr Handle, longint Pos, WORD Len)
{
	return false;
}

bool UnLockH(filePtr Handle, longint Pos, WORD Len)
{
	return false;
}

void TruncH(FILE* handle, longint N)
{
	// cilem je zkratit delku souboru na N
	if (handle == nullptr) return;
	if (FileSizeH(handle) > N) {
		SeekH(handle, N);
		SetEndOfFile(handle);
	}
}

void CloseH(FILE* handle)
{
	if (handle == nullptr) return;
	// uzav�e soubor
	HandleError = fclose(handle);
}


void ClearCacheH(FILE* h)
{
}

void CloseClearH(FILE* h)
{
	if (h == nullptr) return;
	CloseH(h);
	ClearCacheH(h);
	h = nullptr;
}

void SetFileAttr(WORD Attr)
{
	// nastav� atributy souboru/adres��e
	// 0 = read only, 1 = hidden file, 2 = system file, 3 = volume label, 4 = subdirectory,
	// 5 = written since backup, 8 = shareable (Novell NetWare)
	if (SetFileAttributesA(CPath.c_str(), Attr) == 0)
	{
		HandleError = GetLastError();
	}
}

WORD GetFileAttr()
{
	// z�sk� atributy souboru/adres��e
	auto result = GetFileAttributesA(CPath.c_str());
	if (result == INVALID_FILE_ATTRIBUTES) HandleError = GetLastError();
	return result;
}

void RdWrCache(bool ReadOp, FILE* Handle, bool NotCached, longint Pos, WORD N, void* Buf)
{
	if (Handle == nullptr) return;
	// asi net�eba �e�it
	return;
}

void FlushH(FILE* handle)
{
	if (handle == nullptr) return;

	auto result = fflush(handle);
	if (result == EOF) { HandleError = result; }
	//SetHandle(handle);
	SetUpdHandle(handle);
	//CloseH(handle);
}

void DeleteFile(pstring path)
{
}

WORD FindCtrlM(LongStrPtr s, WORD i, WORD n)
{
	WORD l = s->LL;
	while (i <= 1)
	{
		if (s->A[i] == '0x0D') {
			if (n > 1) n--;
			else return i;
		}
		i++;
	}
	return l + 1;
}

WORD SkipCtrlMJ(LongStrPtr s, WORD i)
{
	WORD l = s->LL;
	if (i<=1)
	{
		i++;
		if (i <= 1 && s->A[i] == 0x0A) i++;
	}
	return i;
}

void FlushHandles()
{
	for (auto handle : UpdHandles)
	{
		FlushH(handle);
	}
	for (auto handle : FlshHandles)
	{
		FlushH(handle);
	}
	ClearUpdHandles();
	ClearFlshHandles();
}

longint GetDateTimeH(filePtr handle)
{
	if (handle == nullptr) return -1;
	// vr�t� �as posledn�ho z�pisu souboru + datum posledn�ho z�pisu souboru
	// 2 + 2 Byte (datum vlevo, �as vpravo)
	FILETIME ft;
	auto result = GetFileTime(handle, nullptr, nullptr, &ft);
	if (result == 0) HandleError = GetLastError();
	return (ft.dwHighDateTime << 16) + ft.dwLowDateTime;
}

void MyDeleteFile(pstring path)
{
	// sma�e soubor - INT $41
	auto result = remove(path.c_str());
	if (result != 0) HandleError = result;
}

void RenameFile56(pstring OldPath, pstring NewPath, bool Msg)
{
	// p�esouv� nebo p�ejmenov�v� soubor
	// potom:
	auto result = rename(OldPath.c_str(), NewPath.c_str());
	if (result != 0) HandleError = result;
	if (Msg && HandleError != 0)
	{
		Set2MsgPar(OldPath, NewPath);
		RunError(829);
	}
}

pstring MyFExpand(pstring Nm, pstring EnvName)
{
	pstring d;
	GetDir(0, &d);
	pstring f = FandDir;
	DelBackSlash(f);
	ChDir(f);
	pstring p = GetEnv(EnvName.c_str());
	AddBackSlash(p);
	if (!p.empty()) p += Nm;
	else {
		pstring envp = GetEnv("PATH");
		p = FSearch(Nm, envp);
		if (p.empty()) p = Nm;
	}
	ChDir(d);
	return p;
}

void* Normalize(longint L)
{
	return nullptr;
}

longint AbsAdr(void* P)
{
	return 0;
}

void ExChange(void* X, void* Y, WORD L)
{
}

void ReplaceChar(pstring S, char C1, char C2)
{
}

bool SetStyleAttr(char C, BYTE& a)
{
	auto result = true;
	if (C == 0x13) a = colors.tUnderline;
	else if (C == 0x17) a = colors.tItalic;
	else if (C == 0x11) a = colors.tDWidth;
	else if (C == 0x04) a = colors.tDStrike;
	else if (C == 0x02) a = colors.tEmphasized;
	else if (C == 0x05) a = colors.tCompressed;
	else if (C == 0x01) a = colors.tElite;
	else result = false;
	return result;
}

WORD LenStyleStr(pstring s)
{
	WORD l, i;
	l = s.length(); for (i = 1; i < s.length(); i++)
	{
		if (s[i] == 0x13 || s[i] == 0x17 || s[i] == 0x11 || s[i] == 0x04
			|| s[i] == 0x02 || s[i] == 0x05 || s[i] == 0x01)
			l--;
	}
	return l;
}

pstring CStyle(10);
pstring CColor(11);

void WrStyleChar(char C)
{
	BYTE a; bool b; WORD i;
	if (SetStyleAttr(C, a))
	{
		i = CStyle.first(C);
		if (i != 0)
		{
			CStyle.Delete(i, 1); CColor.Delete(i, 1);
		}
		else {
			pstring oldCStyle = CStyle;
			CStyle = C;
			CStyle += oldCStyle;
			pstring oldCColor = CColor;
			CColor = a;
			CColor += oldCColor;
		}
		TextAttr = CColor[1];
	}
	else if (C == 0x0D) printf("\r\n");
	else if (C != 0x0A) printf("%c", C);
}

void WrStyleStr(pstring s, WORD Attr)
{
	WORD i;
	TextAttr = Attr, CStyle = ""; CColor = char(Attr);
	for (i=1; i < s.length(); i++)
	{
		WrStyleChar(s[i]);
	}
	TextAttr = Attr;
}

void WrLongStyleStr(LongStr* S, WORD Attr)
{
	WORD i;
	TextAttr = Attr; CStyle = ""; CColor = char(Attr);
	for (i = 1; i < S->LL; i++) WrStyleChar(S->A[i]); TextAttr = Attr;
}

WORD LogToAbsLenStyleStr(pstring s, WORD l)
{
	WORD i = 1;
	while ((i <= s.length()) && (l > 0))
	{
		if (!(s[i] == 0x13 || s[i] == 0x17 || s[i] == 0x11 || s[i] == 0x04
			|| s[i] == 0x02 || s[i] == 0x05 || s[i] == 0x01)) l--;
		i++;
	}
	return i - 1;
}

void CloseXMS()
{
	if (XMSCachePages > 0)
	{
		// asm mov ah,0AH; mov dx,XMSHandle; call [XMSFun] ;
	}
}

void MoveToXMS(WORD NPage, void* Src)
{
}

void MoveFromXMS(WORD NPage, void* Dest)
{
}

bool CacheExist()
{
	return NCachePages > 0;
}

void SetMyHeapEnd()
{
	// MyHeapEnd = ptr(PtrRec(CacheEnd).Seg - CachePageSz, PtrRec(CacheEnd).Ofs);
}

void NewCachePage(CachePage* ZLast, CachePage* Z)
{
}

void FormatCache()
{
}

bool WrCPage(FILE* Handle, longint N, void* Buf, WORD ErrH)
{
	return true;
}

bool WriteCachePage(CachePage* Z, WORD ErrH)
{
	return true;
}

void ReadCachePage(CachePage* Z)
{
}

CachePage* Cache(BYTE Handle, longint Page)
{
	return nullptr;
}

void LockCache()
{
}

void UnLockCache()
{
}

bool SaveCache(WORD ErrH)
{
	return true;
}

void SubstHandle(WORD h1, WORD h2)
{
}

void FreeCachePage(CachePage* Z)
{
}

void ExpandCacheUp()
{
}

void ExpandCacheDown()
{
}

integer HeapErrFun(WORD Size)
{
	return 0;
}

void AlignParagraph()
{
}

void* GetStore2(WORD Size)
{
	return nullptr;
}

void* GetZStore2(WORD Size)
{
	return nullptr;
}

pstring* StoreStr(pstring S)
{
	return nullptr;
}

void MarkStore(void* p)
{
}

void MarkStore2(void* p)
{
}

void MarkBoth(void* p, void* p2)
{
}

void ReleaseStore(void* pointer)
{
}

void ReleaseAfterLongStr(void* pointer)
{
}

WORD CountDLines(void* Buf, WORD L, char C)
{
	WORD count = 0;
	for (int i = 0; i < L; i++) { if (((char*)Buf)[i] == C) count++; }
	return count + 1; // za posledni polozkou neni '/'
}

pstring GetDLine(void* Buf, WORD L, char C, WORD I) // I = 1 .. N
{
	std::string input((const char*)Buf, L);
	std::vector<std::string> lines;
	size_t pos = 0;
	std::string token;
	while ((pos = input.find(C)) != std::string::npos) {
		token = input.substr(0, pos);
		lines.push_back(token);
		input.erase(0, pos + 1); // smazani vc. oddelovace
	}
	lines.push_back(input); // pridame zbyvajici cast retezce
	if (I <= lines.size()) return lines[I - 1]; // Pascal cislovani od 1
	return "";
}

bool OverlapByteStr(void* p1, void* p2)
{
	return false;
}

bool MouseInRectProc(WORD X, WORD Y, WORD XSize, WORD Size)
{
	return false;
}

bool EqualsMask(void* p, WORD l, pstring Mask)
{
	return false;
}

WORD ListLength(void* P)
{
	return 0;
}

void ReleaseStore2(void* p)
{
}

void ReleaseBoth(void* p, void* p2)
{
	ReleaseStore(p); ReleaseStore2(p2);
}

longint StoreAvail()
{
	return /*512 * */1024 * 1024;
}

void AlignLongStr()
{
}

void NewExit(PProcedure POvr, ExitRecord* Buf)
{
}

void GoExit()
{
	printf("%s\n", MsgLine.c_str());
}

void RestoreExit(ExitRecord& Buf)
{
}

bool OSshell(pstring Path, pstring CmdLine, bool NoCancel, bool FreeMm, bool LdFont, bool TextMd)
{
	return true;
}

pstring PrTab(WORD N)
{
	void* p;
	p = printer[prCurr].Strg;

	// ASM
	return "";
}

void SetCurrPrinter(integer NewPr)
{
	if (NewPr >= prMax) return;
	if (prCurr >= 0) /* !!! with printer[prCurr] do!!! */
		if (printer[prCurr].TmOut != 0)	PrTimeOut[printer[prCurr].Lpti] = OldPrTimeOut[printer[prCurr].Lpti];
	prCurr = NewPr;
	if (prCurr >= 0) /* !!! with printer[prCurr] do!!! */
		if (printer[prCurr].TmOut != 0) PrTimeOut[printer[prCurr].Lpti] = printer[prCurr].TmOut;
}

void (*ExitSave)(); //535

void WrTurboErr()
{
	pstring s = pstring(9);
	str(ExitCode, s);
	SetMsgPar(s);
	WrLLF10Msg(626);
	ErrorAddr = nullptr;
	ExitCode = 0;
}

void MyExit()
{
	// { asm mov ax, SEG @Data; mov ds, ax end; }
	ExitProc = ExitSave;
	if (!WasInitPgm) { UnExtendHandles(); goto label1; }

	if (ErrorAddr != nullptr)
		switch (ExitCode)
		{
		case 202: // {stack overflow}
		{
			// asm mov sp, ExitBuf.rSP
			WrLLF10Msg(625);
			break;
		}
		case 209: //{overlay read error}
			WrLLF10Msg(648);
			break;
		default: WrTurboErr(); break;
		}
#ifdef FandSQL
	SQLDisconnect();
#endif

	UnExtendHandles();
	MyDeleteFile(FandWorkName);
	MyDeleteFile(FandWorkXName);
	MyDeleteFile(FandWorkTName);
	// TODO? CloseXMS();
label1: if (WasInitDrivers) {
	// TODO? DoneMouseEvents();
	// CrsIntrDone();
	BreakIntrDone();
	if (IsGraphMode) {
		CloseGraph();
		IsGraphMode = false;
		// TODO? ScrSeg = video.Address;
		/*asm  push bp; mov ah,0fH; int 10H; cmp al,StartMode; je @1;
			 mov ah,0; mov al,StartMode; int 10H;
		@1:  pop bp end; */
		screen.Window(1, 1, TxtCols, TxtRows);
		TextAttr = StartAttr;
		ClrScr();
		screen.CrsNorm();
		ChDir(OldDir);
		SetCurrPrinter(-1);
	}
	if (ExitCode == 202) Halt(202);
}
}

void OpenResFile()
{
	CPath = FandResName; CVol = "";
	ResFile.Handle = OpenH(_isoldfile, RdOnly);
	if (HandleError != 0)
	{
		printf("can't open %s", FandResName.c_str());
		wait();
		Halt(0);
	}
}

void InitOverlays()
{
	pstring name; pstring ext; integer sz, err; longint l; pstring s;
	const BYTE OvrlSz = 124;

	GetDir(0, &OldDir);
	//OldDir = GetDir(0);
	FSplit(FExpand(ParamStr(1)), FandDir, name, ext);
	FandOvrName = MyFExpand(name + ".OVR", "FANDOVR");
	//OvrInit(FandOvrName);
	//if (OvrResult != 0) {          /*reshandle-1*/
	//	FandOvrName = ParamStr(0);
	//	OvrInit(FandOvrName);
	//	if (OvrResult != 0) {
	//		printf("can't open FAND.OVR"); wait(); Halt(-1);
	//	}
	//}
	//OvrInitEMS();
	s = GetEnv("FANDOVRB");
	while ((s.length() > 0) && (s[s.length()] == ' ')) s[0] = s.length() - 1;
	val(s, sz, err);
	if ((err != 0) || (sz < 80) || (sz > OvrlSz + 10)) sz = OvrlSz; l = longint(sz) * 1024;
	//OvrSetBuf(l);
	//OvrSetRetry(l / 2);
	//TODO: FreeList = nullptr;
}

void OpenWorkH()
{
	CPath = FandWorkName;
	CVol = "";
	WorkHandle = OpenH(_isoldnewfile, Exclusive);
	if (HandleError != 0) {
		printf("cant't open %s", FandWorkName.c_str());
		wait();
		Halt(-1);
	}
}

bool SEquUpcase(pstring S1, pstring S2)
{
	size_t s1_len = S1.length();
	size_t s2_len = S2.length();
	if (s1_len != s2_len) return false;
	if (s1_len == 0) return true;

	const char* s1_c = S1.c_str();
	const char* s2_c = S2.c_str();

	for (size_t i = 0; i < s1_len; i++)
	{
		if (toupper(s1_c[i]) != toupper(s2_c[i])) return false;
	}
	return true;
}

//void OpenOvrFile()
//{
//	FILE* h;
//	CPath = FandOvrName;
//	CVol = "";
//	h = OpenH(_isoldfile, RdOnly);
//		if (h != OvrHandle)
//		{
//			printf("can't open FAND.OVR");
//			wait();
//			Halt(-1);
//		}
//}

void NonameStartFunction()
{
	integer UserLicNr;
	double userToday;
	// TODO:
	// CurPSP = ptr(PrefixSeg, 0);
	// MyHeapEnd = HeapEnd;
	ExtendHandles();
	prCurr = -1;
	// InitOverlays();
	ExitSave = ExitProc;
	ExitProc = MyExit;
	MyBP = nullptr;
	UserLicNr = WORD(UserLicNrShow) & 0x7FFF;
	FandResName = MyFExpand("Fand.Res", "FANDRES");
	OpenResFile();
}
