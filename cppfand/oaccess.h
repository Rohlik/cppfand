#pragma once

//#include "fanddml.h"
#include "access.h"
#include "fileacc.h"

void OpenXWorkH(); // �. 49
void OpenTWorkH(); // �. 57
void SaveFiles(); // r69
void ClosePassiveFD(); // r76
void CloseFANDFiles(bool FromDML);// �. 81
void OpenFANDFiles(bool FromDML);// �. 94
bool OpenF1(FileUseMode UM);// �. 119
bool OpenF2();// �. 159
bool OpenF(FileUseMode UM);// �. 196
bool OpenCreateF(FileUseMode UM); // r218
void TruncF();// �. 239
void CloseFile();// �. 252
WORD TestMountVol(char DriveC); // r301
WORD GetCatIRec(pstring Name, bool MultiLevel); // r364
pstring RdCatField(WORD CatIRec, FieldDPtr CatF);// �. 400
bool SetContextDir(pstring& D, bool& IsRdb);// �. 414
void GetCPathForCat(WORD I);// �. 429
void SetCPathVol(); // �. 441
void SetTxtPathVol(pstring Path, WORD CatIRec); // r463
