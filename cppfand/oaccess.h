#pragma once

//#include "fanddml.h"
#include "access.h"
#include "fileacc.h"

// �. 49
void OpenXWorkH();

// �. 57
void OpenTWorkH();

// �. 81
void CloseFANDFiles(bool FromDML);

// �. 94
void OpenFANDFiles(bool FromDML);

// �. 119
bool OpenF1(FileUseMode UM);

// �. 159
bool OpenF2();

// �. 196
bool OpenF(FileUseMode UM);

// �. 239
void TruncF();

// �. 252
void CloseFile();

WORD GetCatIRec(pstring Name, bool MultiLevel); // r364

// �. 400
string RdCatField(WORD CatIRec, FieldDPtr CatF);

// �. 414
bool SetContextDir(pstring& D, bool& IsRdb);

// �. 429
void GetCPathForCat(WORD I);

// �. 441
void SetCPathVol();
