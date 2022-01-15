#pragma once
#include "base.h"
#include "Chained.h"
#include "Rdb.h"
#include "../Indexes/XKey.h"


class Instr;
class FileD;
class ChkD;
class FrmlElem;
class FrmlElemSum;
class FieldDescr;
class LocVarBlkD;

struct FieldListEl : public Chained<FieldListEl> // r32
{
	FieldDescr* FldD = nullptr;
};
typedef FieldListEl* FieldList;

struct FrmlListEl : public Chained<FrmlListEl> // �. 34
{
	FrmlElem* Frml = nullptr;
};
typedef FrmlListEl* FrmlList;

struct StringListEl : public Chained<StringListEl> // �. 38
{
	std::string S;
};
typedef StringListEl* StringList;

struct KeyListEl : public Chained<KeyListEl> // �. 49
{
	XKey* Key = nullptr;
};
typedef KeyListEl* KeyList;

struct KeyInD : public Chained<KeyListEl> // r89
{
	FrmlListEl* FL1 = nullptr;
	FrmlListEl* FL2 = nullptr;
	longint XNrBeg = 0, N = 0;
	std::string X1;
	std::string X2;
};

struct structXPath
{
	longint Page = 0;
	WORD I = 0;
};

struct ImplD : public Chained<ImplD>
{
	//ImplD* pChain; 
	FieldDescr* FldD = nullptr;
	FrmlElem* Frml = nullptr;
};
typedef ImplD* ImplDPtr;

struct LiRoots
{
	ChkD* Chks = nullptr;
	ImplD* Impls = nullptr;
};
typedef LiRoots* LiRootsPtr;

struct DBaseFld // �. 208
{
	pstring Name;
	char Typ = 0;
	longint Displ = 0;
	BYTE Len = 0, Dec = 0;
	BYTE x2[14];
};

struct DBaseHd // �. 213
{
	BYTE Ver = 0;
	BYTE Date[4]{ 0,0,0,0 };
	longint NRecs = 0;
	WORD HdLen = 0, RecLen = 0;
	DBaseFld Flds[1];
};

struct LinkD
{
	WORD IndexRoot = 0;
	BYTE MemberRef = 0; // { 0-no, 1-!, 2-!!(no delete)}
	KeyFldD* Args = nullptr;
	FileD* FromFD = nullptr;
	FileD* ToFD = nullptr;
	XKey* ToKey = nullptr;
	std::string RoleName;
};

struct WRectFrml // r251
{
	FrmlElem* C1 = nullptr;
	FrmlElem* R1 = nullptr;
	FrmlElem* C2 = nullptr;
	FrmlElem* R2 = nullptr;
};

struct CompInpD // r402
{
	CompInpD* ChainBack = nullptr;
	CharArr* InpArrPtr = nullptr;
	RdbPos InpRdbPos;
	WORD InpArrLen = 0, CurrPos = 0, OldErrPos = 0;
};