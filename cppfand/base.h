#pragma once
#include <array>
#include "constants.h"

//#include ""

typedef void* PProcedure;

void wait(); // �. 95

void NewExit(PProcedure POvr, ExitRecord Buf);  // �. 218

WORD HandleError;

DirStr OldDir, FandDir, WrkDir; // �. 230
PathStr FandOvrName, FandResName, FandWorkName, FandWorkXName, FandWorkTName;
PathStr CPath; DirStr CDir; NameStr CName; ExtStr CExt;
VolStr CVol;
bool WasLPTCancel;
WORD WorkHandle;
const longint MaxWSize = 0; // {currently occupied in FANDWORK.$$$}

//WORD ReadH(WORD handle, WORD bytes, void* buffer);

// ********** MESSAGES **********
WORD F10SpecKey; // �. 293
BYTE ProcAttr;
bool SetStyleAttr(char c, BYTE a);
string MsgLine;
ScreenStr MsgPar[4];
void SetMsgPar(string s);
void Set2MsgPar(string s1, string s2);
void Set3MsgPar(string s1, string s2, string s3);
void Set4MsgPar(string s1, string s2, string s3, string s4);
void RdMsg(integer N);
void WriteMsg(WORD N);
void ClearLL(BYTE attr);

// ********** DML **********
void* FandInt3f; // �. 311
WORD OvrHandle, Fand_ss, Fand_sp, Fand_bp, DML_ss, DML_sp, DML_bp;
const longint _CallDMLAddr = 0; // {passed to FANDDML by setting "DMLADDR="in env.}

struct Video // �. 345
{
	WORD address;
	BYTE TxtRows;
	bool ChkSnow;	// {not used }
	WORD CursOn, CursOff, CursBig;
} video;

struct Colors
{
	BYTE userColor[16];
	BYTE mNorm, mHili, mFirst, mDisabled; // menu
	BYTE sNorm, sHili, sMask; // select
	BYTE pNorm, pTxt; // prompt, verify, password
	BYTE zNorm; // message
	BYTE lNorm, lFirst, lSwitch; // last line
	BYTE fNorm; // first line
	BYTE tNorm, tCtrl, tBlock; // text edit
	BYTE tUnderline, tItalic, tDWidth, tDStrike, tEmphasized, tCompressed, tElite; // data edit
	BYTE dNorm, dHili, dSubset, dTxt, dDeleted, dSelect; // -"-
	BYTE uNorm; // user screen
	BYTE hNorm, hHili, hMenu, hSpec;
	BYTE nNorm;
	BYTE ShadowAttr;
	BYTE DesktopColor;
} colors;

char CharOrdTab[256]; // after Colors /FANDDML/ // �. 370
char UpcCharTab[256]; // TODO: v obou ��dc�ch bylo 'array[char] of char;' - WTF?
WORD TxtCols, TxtRows;

typedef array<BYTE, 4> TPrTimeOut; // �. 418
TPrTimeOut OldPrTimeOut;
TPrTimeOut PrTimeOut;

const char AbbrYes = 'Y'; const char AbbrNo = 'N';
bool WasInitDrivers = false;
bool WasInitPgm = false;

WORD LANNode; // �. 431

const BYTE FandFace = 16;

class TResFile // �. 440
{
public:
	WORD Handle;
	struct st
	{
		longint Pos;
		WORD Size;
	} A[FandFace];
	WORD Get(WORD Kod, void* P);
	string* GetStr(WORD Kod);
};
struct TMsgIdxItem { WORD Nr, Ofs; BYTE Count; } ;
//TMsgIdxItem TMsgIdx[100];
TResFile ResFile;
TMsgIdxItem* MsgIdx;// = TMsgIdx;
WORD MsgIdxN; longint FrstMsgPos;

// �. 577
void OpenWorkH();
