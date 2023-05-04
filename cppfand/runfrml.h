#pragma once
#include "access.h"
#include "models/FrmlElem.h"
#include "../Common/pstring.h"

#ifdef FandGraph
#include "graph.h"
#endif
#ifdef FandSQL
#include "channel.h"
#endif

double Owned(FileD* file_d, FrmlElem* Bool, FrmlElem* Sum, LinkD* LD, void* record);
short CompBool(bool B1, bool B2);
short CompReal(double R1, double R2, short M);
LongStr* CopyToLongStr(pstring& SS);
LongStr* CopyToLongStr(std::string& SS);
pstring LeadChar(char C, pstring S);
bool RunBool(FileD* file_d, FrmlElem* X, void* record);
bool InReal(FileD* file_d, FrmlElemIn* frml, void* record);
bool LexInStr(std::string& S, FrmlElemIn* X);
bool InStr(LongStr* S, FrmlElemIn* X);
bool InStr(std::string& S, FrmlElemIn* X);
bool RunModulo(FrmlElem1* X);
bool RunEquMask(FrmlElem0* X);
double RunReal(FileD* file_d, FrmlElem* X, void* record);
int RunInt(FileD* file_d, FrmlElem* X, void* record);
bool CanCopyT(FieldDescr* F, FrmlElem* Z, FandTFile** TF02, FileD** TFD02, int& TF02Pos);
bool TryCopyT(FieldDescr* F, FandTFile* TF, int& pos, FrmlElem1* Z);
void AssgnFrml(FileD* file_d, void* record, FieldDescr* F, FrmlElem* X, bool Delete, bool Add);
void LVAssignFrml(LocVar* LV, bool Add, FrmlElem* X);
std::string DecodeFieldRSB(FieldDescr* F, WORD LWw, double R, std::string& T, bool B);
std::string DecodeField(FieldDescr* F, WORD LWw);
void RunWFrml(WRectFrml& X, BYTE WFlags, WRect& W);
WORD RunWordImpl(FrmlElem* Z, WORD Impl);
bool FieldInList(FieldDescr* F, FieldListEl* FL);
bool FieldInList(FieldDescr* F, std::vector<FieldDescr*>& FL);
bool FieldInList(FieldDescr* F, std::vector<FieldDescr*>* FL);
XKey* GetFromKey(LinkD* LD);
FrmlElem* RunEvalFrml(FrmlElem* Z);
LongStr* RunLongStr(FileD* file_d, FrmlElem* X, void* record);
std::string RunStdStr(FileD* file_d, FrmlElem* X, void* record);
std::string RunShortStr(FileD* file_d, FrmlElem* X, void* record);
void CopyLongStr(LongStr* S, WORD From, WORD Number);
void AddToLongStr(LongStr* S, void* P, WORD L);
void StrMask(double R, pstring& Mask);
LongStr* RunS(FrmlElem* Z);
LongStr* RunSelectStr(FrmlElem0* Z);
void LowCase(LongStr* S);
void LowCase(std::string& text);
double RoundReal(double RR, short M);
LongStr* LongLeadChar(char C, char CNew, LongStr* S);
LongStr* LongTrailChar(char C, char CNew, LongStr* S);
LongStr* RepeatStr(LongStr* S, short N);
void AccRecNoProc(FrmlElem14* X, WORD Msg);
void GetRecNoXString(FrmlElem13* Z, XString& X);
double RunRealStr(FileD* file_d, FrmlElem* X, void* record);
double RMod(FrmlElem0* X);
double LastUpdate(const std::string& path);
WORD TypeDay(double R);
double AddWDays(double R, short N, WORD d);

double DifWDays(double R1, double R2, WORD d);
int GetFileSize();
int RecNoFun(FrmlElem13* Z);
int AbsLogRecNoFun(FrmlElem13* Z);
double LinkProc(FrmlElem15* X);
WORD IntTSR(FrmlElem* X);
WORD PortIn(bool IsWord, WORD Port);

