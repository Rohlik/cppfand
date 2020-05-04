#pragma once

//#include "fanddml.h"
#include "access.h"
#include "fileacc.h"

void OpenXWorkH(); // �. 49
void OpenTWorkH(); // �. 57
void SaveFD();
void SaveFiles(); // r69
void ClosePassiveFD(); // r76
void CloseFANDFiles(bool FromDML);// �. 81
void OpenFANDFiles(bool FromDML);// �. 94
void SetCPathMountVolSetNet(FileUseMode UM);
bool OpenF1(FileUseMode UM); // �. 119
bool OpenF2(); // �. 159
bool OpenF(FileUseMode UM); // �. 196
void CreateF();
bool OpenCreateF(FileUseMode UM); // r218
LockMode RewriteF(bool Append);
void TruncF(); // �. 239
void CloseFile(); // �. 252
void CloseFAfter(FileD* FD);
bool ActiveRdbOnDrive(WORD D);
void CloseFilesOnDrive(WORD D);
WORD TestMountVol(char DriveC); // r301
void ReleaseDrive(WORD D);
void SetCPathForH(FILE* handle);
#ifdef FandSQL
void SetIsSQLFile();
#endif
WORD GetCatIRec(pstring Name, bool MultiLevel); // r364
WORD Generation();
void TurnCat(WORD Frst, WORD N, integer I);
pstring RdCatField(WORD CatIRec, FieldDPtr CatF);// �. 400
void WrCatField(WORD CatIRec, FieldDescr* CatF, pstring Txt);
void RdCatPathVol(WORD CatIRec);
bool SetContextDir(pstring& D, bool& IsRdb);// �. 414
void GetCPathForCat(WORD I);// �. 429
void SetCPathVol(); // �. 441
void SetTxtPathVol(pstring Path, WORD CatIRec); // r463
void SetTempCExt(char Typ, bool IsNet);
FileD* OpenDuplF(bool CrTF);
void CopyDuplF(FileD* TempFD, bool DelTF);
void CopyH(FILE* h1, FILE* h2);
void SubstDuplF(FileD* TempFD, bool DelTF);
void TestDelErr(pstring* P);
void DelDuplF(FileD* TempFD);
