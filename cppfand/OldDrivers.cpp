#pragma once

#include "OldDrivers.h"
#include "../Common/codePages.h"
#include <Windows.h>
#include <stdio.h>
#include <consoleapi.h>
#include <handleapi.h>
#include <iostream>
#include <WinBase.h>
#include "../pascal/random.h"
#include "base.h"
#include "GlobalVariables.h"
#include "legacy.h"
#include "obaseww.h"
#include "../Drivers/screen.h"
#include "wwmenu.h"

// *** KONZOLA ***
Screen screen(TxtCols, TxtRows, &WindMin, &WindMax, &Crs);
Keyboard keyboard;
//SMALL_RECT hWin;
DWORD ConsoleError;
PINPUT_RECORD KbdBuf;
DWORD cNumRead = 0;
// ***

TEvent Event; // r39
BYTE KbdFlgs; // TODO: absolute $417
//pstring KbdBuffer; // r. 83
BYTE LLKeyFlags = 0; // r. 84
short GraphDriver, GraphMode;
WORD ScrSeg, ScrGrSeg;
BYTE NrVFont, BytesPerChar;
bool ChkSnow;
bool IsGraphMode;
BYTE GrBytesPerChar;
WORD GrBytesPerLine;
TPoint LastWhere, LastWhereG, DownWhere;
WORD LastMode;
void* FontArr; void* BGIDriver; void* BGILittFont; void* BGITripFont;
BYTE ButtonCount, MouseButtons, LastButtons, DownButtons, LastDouble;
WORD EventCount, EventQHead, EventQTail;
stEventQueue EventQueue[EventQSize - 1];
TCrs Crs;
int trialInterval;
void* OldIntr08 = nullptr;
WORD AutoTicks, DownTicks, AutoDelay;
void* OldBreakIntr;
void* OldKbdIntr;
Wind WindMin, WindMax;

BYTE TextAttr, StartAttr, StartMode; // r138
enVideoCard VideoCard = enVideoCard::viVga;

// *** KEYBOARD ***
BYTE ofsHeadKeyBuf = 0x1A;
BYTE ofsTailKeyBuf = 0x1C; /*Bios*/
bool BreakFlag = false;
BYTE diHacek = 1; const BYTE diCarka = 2; const BYTE diUmlaut = 3;
char Diak = 0; /*diHacek, diCarka*/



const BYTE CsKbdSize = 67;

void ClearKeyBuf()
{
}

void BreakCheck()
{
	if (BreakFlag) {
		BreakFlag = false;
		ClearKeyBuf();
		Halt(-1);
	}
}

unsigned char CurrToKamen(unsigned char C)
{
	return C;
}

unsigned char NoDiakr(unsigned char C)
{
	if (C < 0x80 || fonts.VFont == TVideoFont::foAscii) return C;
	if (fonts.VFont == TVideoFont::foLatin2) return TabLtN[C];
	return TabKtN[C];
}

void ConvToNoDiakr(unsigned char* Buf, WORD L, TVideoFont FromFont)
{
}

void AddToKbdBuf(WORD KeyCode)
{
}

bool KeyPressed()
{
	return keyboard.Exists();
}

WORD ReadKey()
{
	return ReadKbd();
}

WORD ConvHCU()
{
	return 0;
}

void GetKeyEvent()
{
	KEY_EVENT_RECORD key;
	bool exists = false;

	do {
		exists = keyboard.Get(key);
		if (exists && key.bKeyDown)
		{
			Event.Pressed = PressedKey(key);
			Event.What = evKeyDown;
			return;
		}
	} while (exists);
}

WORD ReadKbd()
{
	// KEYBD.PAS r606
	// v puvodnim kodu cekala, dokud neexistovala udalost Event.What == evKeyDown
	// dokola volala ClrEvent + GetEvent
	// pak vratila KeyCode a do KbdChar ulozila take KeyCode
	// na konci zavolala ClrEvent

	KEY_EVENT_RECORD key;
	bool exists = false;

	while (true) {
		exists = keyboard.Get(key);
		if (exists && key.bKeyDown) {
			PressedKey pressed = PressedKey(key);
			/*Event.Pressed = pressed;
			Event.What = evKeyDown;*/
			ClrEvent();
			return pressed.KeyCombination();
		}
		else {
			Sleep(100);
			ClrEvent();
		}
	}
}

bool KbdTimer(WORD Delta, BYTE Kind)
{
	ULONGLONG EndTime;
	auto result = false;
	EndTime = GetTickCount64() + Delta;
	result = false;
label1:
	switch (Kind) {          /* 0 - wait, 1 - wait || ESC, 2 - wait || any key */
	case 1: {
		if (KeyPressed() && (ReadKey() == __ESC)) return result;
		break;
	}
	case 2: {
		if (KbdPressed()) { ReadKbd(); return result; }
		break;
	}
	}
	if (GetTickCount64() < EndTime) goto label1;
	result = true;
	return result;
}

void ShowMouse()
{
}

void HideMouse()
{
}

void ResetMouse()
{
}

void MouseEvHandler()
{
}

void InitMouseEvents()
{
}

void SetMouse(WORD X, WORD Y, bool Visible)
{
}

void DoneMouseEvents()
{
}

void HideMaus()
{
}

void ShowMaus()
{
}

void GetRedKeyName()
{
	// souvisi s mysi
}

void GetMouseEvent()
{
}

void GetMouseKeyEvent()
{
}

void TestGlobalKey()
{
	WORD i;
	bool InMenu6 = false; bool InMenu8 = false;
	if (Event.What != evKeyDown) return;
	if (Event.Pressed.isChar()) return;
	switch (Event.Pressed.KeyCombination()) {
	case __ALT_F8: {
		if (!InMenu8) {
			ClrEvent(); InMenu8 = true;
			i = Menu(45, spec.KbdTyp + 1);
			if (i != 0) spec.KbdTyp = TKbdConv(i - 1);
			InMenu8 = false;
		}
		break;
	}
	case __ALT_F6: {
		if (!InMenu6) {
			ClrEvent(); InMenu6 = true;
			PrinterMenu(46); InMenu6 = false;
		}
		break;
	}
	case __ESC: {
		if (LLKeyFlags != 0) { LLKeyFlags = 0; ClrEvent(); }
		break;
	}
	default: break;
	}
}

WORD AddCtrlAltShift(BYTE Flgs)
{
	auto key = Event.Pressed.KeyCombination();
	WORD result = 0;
	if (Event.What != evKeyDown) return Event.What;
	if ((Flgs & 0x04) == 0) goto label3;
	if (key != __HOME) goto label1;
	result = __CTRL_HOME;
	goto label6;
label1:
	if (key != __END) goto label2;
	result = __CTRL_END;
	goto label6;
label2:
	if (key != 'Y') goto label3;
	result = 'Y';
	goto label6;
label3:
	if (key < __F1 || key > __F10) return key;
	if ((Flgs & 0x04) == 0) goto label4;
	result = key + CTRL;
	goto label6;
label4:
	if ((Flgs & 0x08) == 0) goto label5;
	result = key + ALT;
	goto label6;
label5:
	if ((Flgs & 0x03) == 0) goto label6;
	result = key + SHIFT;
label6:
	Event.Pressed.UpdateKey(result);
	TestGlobalKey();
	return result;
}

bool TestEvent()
{
label1:
	if (Event.What == 0) GetMouseKeyEvent();
	if (Event.What == 0) GetKeyEvent();
	if (Event.What == 0) return false;
	TestGlobalKey();
	if (Event.What == 0) goto label1;
	return true;
}


#ifdef Trial
int getSec()
{
	WORD h, m, s, ss;
	getTime(h, m, s, ss);
	return h * 3600 + m * 60 + s;
}

void TestTrial()
{
	int now;
	if ((trialStartFand = 0)) { trialStartFand = getSec(); trialInterval = 900; }
	else {
		now = getSec();
		if (now > trialStartFand + trialInterval) {
			trialStartFand = now;
			trialInterval = trialInterval / 3;
			if (trialInterval < 10) trialInterval = 10;
			WrLLF10Msg(71);
		}
	}
}
#endif

void TPoint::Assign(WORD XX, WORD YY)
{
	// asm les di,Self; mov ax,XX; mov es:[di].TPoint.X,ax;
	// mov ax, YY; mov es : [di] .TPoint.Y, ax end;
}

void Assign(WORD XX, WORD YY)
{
}

void Assign(pstring XX, pstring YY)
{
}

char CurrToKamen(char C)
{
	return C;
}

void ConvKamenToCurr(std::string& text)
{
	BYTE* tab = TabKtN;
	if (!fonts.NoDiakrSupported) {
		tab = TabKtL;
		if (fonts.VFont != TVideoFont::foLatin2) return;
	}
	for (size_t i = 0; i < text.length(); i++) {
		short index = (BYTE)text[i] - 0x80;
		if (index > 0) {
			BYTE kam = text[i];
			BYTE lat = tab[kam];
			text[i] = (char)lat;
		}
	}
}

void ConvKamenLatin(void* Buf, WORD L, bool ToLatin)
{
}

char ToggleCS(char C)
{
	char result = 0;
	BYTE input = (BYTE)C;

	if (input == 0) {
		// null stays null
	}
	else if (input > 0 && input < 32) {
		// non-printable char -> stays same
		result = input;
	}
	else {
		result = (char)toggleLatin2[input];
		if (result == 0) {
			// substitute not exists -> return input char
			result = C;
		}
	}
	return result;
}

char NoDiakr(char C)
{
	BYTE byteC = static_cast<BYTE>(C);
	if (byteC < 0x80) return C;

	switch (fonts.VFont) {
	case TVideoFont::foAscii: return C; break;
	case TVideoFont::foLatin2: return static_cast<char>(TabLtN[byteC]); break;
	case TVideoFont::foKamen: return static_cast<char>(TabKtN[byteC]); break;
	default: return C; break;
	}
}

void ConvToNoDiakr(void* Buf, WORD L, TVideoFont FromFont)
{
	auto c = static_cast<BYTE*>(Buf);
	if (FromFont == TVideoFont::foAscii) return;
	if (FromFont == TVideoFont::foLatin2) {
		for (size_t i = 0; i < L; i++) {
			if (c[i] < 0x80) continue;
			c[i] = TabLtN[c[i]];
		}
	}
	else {
		for (size_t i = 0; i < L; i++) {
			if (c[i] < 0x80) continue;
			c[i] = TabKtN[c[i]];
		}
	}
}

void ClearKbdBuf()
{
	// keyboard.ClearBuf();
}

bool KbdPressed()
{
	if (keyboard.Exists()) return true;
	if (KeyPressed()) return true;
	Event.What = evNothing;
	GetMouseKeyEvent();
	if (Event.What == evKeyDown)
	{
		AddToKbdBuf(Event.Pressed.KeyCombination());
		ClrEvent();
		return true;
	}
	return false;
}

bool ESCPressed()
{
	if (KeyPressed()) {
		if (ReadKey() == __ESC) return true;
	}
	else {
		GetMouseKeyEvent();
		if (Event.What == evKeyDown) {
			if (Event.Pressed.KeyCombination() == __ESC) {
				ClrEvent(); return true;
			}
		}
		ClrEvent();
	}
	return false;
}

void Delay(WORD N)
{
}

void Sound(WORD N)
{
}

void NoSound()
{
}

void ClrScr()
{
	//screen.ScrClr(WindMin.X - 1, WindMin.Y - 1, WindMax.X - WindMin.X + 1, WindMax.Y - WindMin.Y + 1, ' ', TextAttr);
	screen.ScrClr(WindMin.X, WindMin.Y, WindMax.X - WindMin.X + 1, WindMax.Y - WindMin.Y + 1, ' ', TextAttr);
	screen.GotoXY(WindMin.X, WindMin.Y, absolute);
}

void ClrEol()
{
	auto X = screen.WhereXabs();
	auto Y = screen.WhereYabs();
	screen.ScrClr(X, Y, WindMax.X - X + 1, 1, ' ', TextAttr);
}

void TextBackGround(BYTE Color)
{
}

void TextColor(BYTE Color)
{
}

//void InsLine()
//{
//	Scroll(WindMin.X, Crs.Y, WindMax.X - WindMin.X + 1, WindMax.Y - Crs.Y + 1, false);
//}

//void DelLine()
//{
//	Scroll(WindMin.X, Crs.Y, WindMax.X - WindMin.X + 1, WindMax.Y - Crs.Y + 1, true);
//}

void Beep()
{
	printf("%c", '\a');
}

void LockBeep()
{
	if (spec.LockBeepAllowed) printf("%c", '\a');
}

void ScrBeep()
{
}

WORD WaitEvent(WORD Delta)
{
	ULONGLONG t = 0;
	int t1 = 0, pos = 0, l = 555;
	BYTE Flgs = 0;
	WORD x = 0, y = 0;
	bool vis = false, ce = false;
	const BYTE MoveDelay = 10;
	WORD result = 0;

	Flgs = KbdFlgs;
label0:
	pos = 0; t = GetTickCount64();
label1:
	if (Event.What == 0) { GetMouseKeyEvent(); }
	if (Event.What == 0) { GetKeyEvent(); }
	if (Event.What != 0) { result = 0; goto label2; }
	if (Flgs != KbdFlgs) { result = 1; goto label2; }
	if ((Delta != 0) && (GetTickCount64() > t + Delta)) { result = 2; goto label2; }
	if (pos != 0)
	{
		if (GetTickCount64() > t1 + MoveDelay) {
			screen.ScrWrStr(x, y, "       ", 7);
			x = Random(TxtCols - 8); y = Random(TxtRows - 1);
			screen.ScrWrStr(x, y, "PC FAND", 7);
			t1 = GetTickCount64();
		}
	}
	else
	{
		if ((spec.ScreenDelay > 0) && (GetTickCount() > t + spec.ScreenDelay)) {
			l = TxtCols * TxtRows * 2 + 50;
			ce = Crs.Enabled;
			screen.CrsHide();
			pos = PushW(1, 1, TxtCols, TxtRows, true, true);
			TextAttr = 0; ClrScr(); vis = MausVisible; HideMouse(); l = 555;
			t1 = GetTickCount() - MoveDelay;
		}
	}
	goto label1;
label2:
	if (pos != 0)
	{
		srand(l);
		if (vis) ShowMouse();
		PopW(pos);
		if (ce) screen.CrsShow();
		if (Event.What != 0)
		{
			Event.What = evNothing;
			goto label0;
		}
	}
	TestGlobalKey();
#ifdef Trial
	TestTrial();
#endif
	return result;

	//GetKeyEvent();
	//return ReadKey();
}

void GetEvent()
{
	do { WaitEvent(0); } while (Event.What == 0);
}

void ClrEvent()
{
	Event.What = evNothing;
}

void AssignCrt(pstring* filepath)
{
	TextFile* F = (TextFile*)filepath;
	/* !!! with F do!!! */
	F->Mode = "fmClosed";
	F->bufsize = 128; //BufPtr = buffer;
	F->openfunc = OpenCrt;
	F->name[0] = 0;
}

void GetMonoColor()
{
}

void EgaWriteArr(WORD X, WORD Y, WORD L, void* From)
{
}

void EgaScroll(WORD X, WORD Y, WORD SizeX, WORD SizeY, bool Up)
{
}

void CrsDraw()
{
}

void HideMausIn()
{
}


void Scroll(WORD X, WORD Y, WORD SizeX, WORD SizeY, bool Up)
{
	printf("Scroll neumime! Zatim ...");
}

void WrDirect()
{
}

void ScrollUp()
{
	Scroll(WindMin.X, WindMin.Y, WindMax.X - WindMin.X + 1, WindMax.Y - WindMin.Y + 1, true);
}

void LineFeed()
{
}

short WrOutput(TextFile* F)
{
	return 0;
}

short DummyCrt(TextFile* F)
{
	return 0;
}

short OpenCrt(TextFile* F)
{
	/* !!! with F do!!! */
	F->inoutfunc = WrOutput;
	F->flushfunc = WrOutput;
	F->closefunc = DummyCrt;
	return 0;
}

unsigned long long getAvailPhysMemory()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullAvailPhys;
}