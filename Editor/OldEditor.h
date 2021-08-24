#pragma once

#include <set>
#include <string>
#include "../cppfand/base.h"
#include "../cppfand/constants.h"
#include "../cppfand/pstring.h"

class Instr;
struct EdExitD;
class Instr_setedittxt;

struct MsgStr
{
	std::string Head, Last, CtrlLast, AltLast, ShiftLast;
};

bool EditText(char pMode, char pTxtType, pstring pName, pstring pErrMsg,
	char* pTxtPtr, WORD pMaxLen, size_t& pLen, WORD& pInd, longint& pScr,
	pstring pBreaks, EdExitD* pExD, bool& pSrch, bool& pUpdat,
	WORD pLastNr, WORD pCtrlLastNr, MsgStr* pMsgS); // r169
void SimpleEditText(char pMode, pstring pErrMsg, pstring pName, char* TxtPtr,
	WORD MaxLen, size_t& Len, WORD& Ind, bool& Updat); // r202
WORD FindTextE(const pstring& PstrScreenStr, pstring Popt, char* PTxtPtr, WORD PLen); // r209
void InitTxtEditor();
void EditTxtFile(longint* LP, char Mode, pstring& ErrMsg, EdExitD* ExD, longint TxtPos,
	longint Txtxy, WRect* V, WORD Atr, pstring Hd, BYTE WFlags, MsgStr* MsgS);
void ViewPrinterTxt(); // r353
void SetEditTxt(Instr_setedittxt* PD);
void GetEditTxt(bool& pInsert, bool& pIndent, bool& pWrap, bool& pJust, bool& pColBlk,
	integer& pLeftMarg, integer& pRightMarg); // r162

void Background();
void DisplLL(WORD Flags);
void WrLLMargMsg(std::string& s, WORD n);
void ScrollPress();
void CleanFrameM();
WORD SetInd(WORD Ind, WORD Pos);
void TestUpdFile();
void DelEndT();
void KodLine();
void SetDekLnCurrI(WORD Ind);
size_t FindChar(char* text, size_t length, char c, size_t from);
void MyDelLine();
void MyInsLine();
WORD SetPredI();
void NextLine(bool WrScr);
void HelpLU(char dir);
void BlockLRShift(WORD I1);
void FrameStep(BYTE& odir, WORD EvKeyC);
void WrCharE(char Ch);
void Format(WORD& i, longint First, longint Last, WORD Posit, bool Rep);
void SetPart(longint Idx);
bool WordExist();
void NextPartDek();
void NewLine(char Mode);
bool TestLastPos(WORD F, WORD T);
void HelpRD(char dir);
WORD Column(WORD p);
longint LineAbs(int Ln);
void PredLine();
void BlockUDShift(longint L1);
void TestKod();
void ClrWord();
bool ModPage(longint RLine);
int NewL(longint RLine);
void DekFindLine(longint Num);
WORD CountChar(char C, WORD First, WORD Last);
longint NewRL(int Line);
WORD FindLine(int Num);
bool WordFind(WORD i, WORD WB, WORD WE, WORD LI);
void SetWord(WORD WB, WORD WE);
WORD WordNo2();
WORD LastPosLine();
void RollNext();
void RollPred();
void DekodLine();
void SetScreen(WORD Ind, WORD ScrXY, WORD Pos);
bool MyPromptLL(WORD n, std::string* s);
void DelChar();
void PredPart();
void DeleteL();
void SetDekCurrI(WORD Ind);
void TestLenText(char** text, size_t& textLength, WORD i, int j);
bool TestOptStr(char c);
bool BlockExist();
void SetBlockBound(longint& BBPos, longint& EBPos);
void FindReplaceString(longint First, longint Last);
void PosDekFindLine(longint Num, WORD Pos, bool ChScr);
bool BlockHandle(longint& fs, FILE* W1, char Oper);
void BlockCopyMove(char Oper, void* P1, LongStr* sp);
bool BlockGrasp(char Oper, void* P1, LongStr* sp);
bool BlockCGrasp(char Oper, void* P1, LongStr* sp);
void BlockDrop(char Oper, void* P1, LongStr* sp);
void BlockCDrop(char Oper, void* P1, LongStr* sp);
void FillBlank();
void NullChangePart();
void SetPartLine(longint Ln);
void MyWrLLMsg(pstring s);
void HMsgExit(pstring s);
void Calculate();
longint SavePar();
void RestorePar(longint l);
void OpenTxtFh(char Mode);
void RdFirstPart();
void SimplePrintHead();
bool RdNextPart();
void WrEndT();
WORD WordNo(WORD I);


typedef std::string ColorOrd;
struct PartDescr
{
	longint PosP = 0; longint LineP = 0;
	WORD LenP = 0, MovI = 0, MovL = 0;
	bool UpdP = false;
	ColorOrd ColorP;
};
const int SuccLineSize = 256;
extern PartDescr Part;
extern pstring Breaks;
extern size_t LenT;
extern char Arr[SuccLineSize];
extern char* T;
extern bool bScroll;
extern EdExitD* ExitD;
extern WORD MaxLenT, IndT, ScrT;
extern WORD ScrI, LineI, Posi, BPos;
extern WORD NextI, PageS, LineS;
extern integer LineL, ScrL;
extern longint RScrL;
extern bool Konec;
extern bool EditT, ChangeScr;
extern char TypeT;
extern bool SrchT, UpdatT;
extern longint* LocalPPtr;
extern FILE* TxtFH;
extern bool AllRd;
extern longint AbsLenT;
extern BYTE FrameDir;
extern bool Insert, Indent, Wrap, Just;
extern integer LeftMarg, RightMarg;
extern WORD BCol, Colu, Row;
extern bool InsPg, ChangePart, TypeB;
extern WORD WordL, LastC, FirstC, FirstR, LastR;
extern longint BegBLn, EndBLn;
extern WORD BegBPos, EndBPos;
extern bool UpdatedL;
extern std::string FindStr, ReplaceStr;
extern bool Replace, FirstEvent;
extern pstring OptionStr;
extern WORD MargLL[4];


extern std::set<char> Separ;

const BYTE LineSize = 255;
const bool TextBlock = false;
const bool ColBlock = true;

const char TextM = 'T'; const char ViewM = 'V'; const char HelpM = 'H';
const char SinFM = 'S'; const char DouFM = 'D'; const char DelFM = 'F';
const char NotFM = 'N'; const char FileT = 'F'; const char LocalT = 'V';
const char MemoT = 'M';

const WORD _QY_ = 0x1119; const WORD _QL_ = 0x110C; const WORD _QK_ = 0x110B;
const WORD _QB_ = 0x1102; const WORD _QI_ = 0x1109; const WORD _QF_ = 0x1106;
const WORD _QE_ = 0x1105; const WORD _QX_ = 0x1118; const WORD _QA_ = 0x1101;
const WORD _framesingle_ = 0x112D; const WORD _framedouble_ = 0x113D;
const WORD _delframe_ = 0x112F; const WORD _frmsin_ = 0x2D;
const WORD _frmdoub_ = 0x3D; const WORD _dfrm_ = 0x2F; const WORD _nfrm_ = 0x20;
const WORD _KB_ = 0x0B02; const WORD _KK_ = 0x0B0B; const WORD _KH_ = 0x0B08;
const WORD _KS_ = 0x0B13; const WORD _KY_ = 0x0B19; const WORD _KC_ = 0x0B03;
const WORD _KV_ = 0x0B16; const WORD _KW_ = 0x0B17; const WORD _KR_ = 0x0B12;
const WORD _KP_ = 0x0B10; const WORD _KN_ = 0x0B0E; const WORD _KU_ = 0x0B15;
const WORD _KL_ = 0x0B0C; const WORD _OW_ = 0x0F17; const WORD _OL_ = 0x0F0C;
const WORD _OR_ = 0x0F12; const WORD _OJ_ = 0x0F0A; const WORD _OC_ = 0x0F03;
const WORD _KF_ = 0x0B06;
