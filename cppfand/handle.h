#pragma once
#include <set>

#include "base.h"
#include "pstring.h"
#include "constants.h"

WORD OldNumH; // r1 
void* OldHTPtr = nullptr;
#ifdef FandDemo
WORD files = 30;
#else
WORD files = 250; // {files in CONFIG.SYS -3}
#endif
WORD CardHandles;

typedef FILE* filePtr;

std::set<FILE*> Handles;
std::set<FILE*> UpdHandles;
std::set<FILE*> FlshHandles;

//map<WORD, FILE*> fileMap;
// n�hrada za 'WORD OvrHandle = h - 1' - zji�t�n� p�edoz�ho otev�en�ho souboru;
std::vector<FILE*> vOverHandle = std::vector<FILE*>(files);
FILE* GetOverHandle(FILE* fptr, int diff);

void SetRes(WORD FLAGS, WORD AX); // r11 - toto je spec. MSDOS registers, flags
bool IsHandle(filePtr H); // r13
bool IsUpdHandle(filePtr H); // r15
bool IsFlshHandle(filePtr H); // r17
void SetHandle(filePtr H);
void SetUpdHandle(filePtr H);
void SetFlshHandle(filePtr H);
void ResetHandle(filePtr H);
void ResetUpdHandle(filePtr H);
void ResetFlshHandle(filePtr H);
void ClearHandles();
void ClearUpdHandles();
void ClearFlshHandles();
bool IsNetCVol();
void ExtendHandles(); // r55 - pr�ce s pam�t� - p�esun n�kam
void UnExtendHandles();  // -''- - p�esun zp�t

longint MoveH(longint dist, WORD method, filePtr handle); // r66 - soubor, nastaven� ukazatele (INT $42)
longint PosH(filePtr handle); // -''-
void SeekH(filePtr handle, longint pos); // -''-
longint FileSizeH(filePtr handle); // -''-
bool TryLockH(filePtr Handle, longint Pos, WORD Len); // r91 - vol� FileLock (INT $5C)
bool UnLockH(filePtr Handle, longint Pos, WORD Len); // -''-

filePtr OpenH(FileOpenMode Mode, FileUseMode UM); // INT $3D, $3C, $5B
WORD ReadH(filePtr handle, WORD bytes, void* buffer); // INT $3F
WORD ReadLongH(filePtr handle, longint bytes, void* buffer); // �e�� segmenatci a �te postupn�
void WriteH(filePtr handle, WORD bytes, void* buffer); // z�pis INT $40
void WriteLongH(filePtr handle, longint bytes, void* buffer); // -''-
void CloseH(filePtr handle); // uzav�en� souboru INT $3E
void FlushH(filePtr& handle); // duplik�t handleru INT $45
void FlushHandles();
void TruncH(filePtr handle, longint N);
void CloseClearH(filePtr h);
void SetFileAttr(WORD Attr); // INT $4301
WORD GetFileAttr(); // INT $4300
void RdWrCache(bool ReadOp, filePtr Handle, bool NotCached, longint Pos, WORD N, void* Buf); // pr�ce s cache


// _____________________________________________________
// od r. 254

longint GetDateTimeH(filePtr handle);
void DeleteFile(pstring path); // INT $41
void RenameFile56(pstring OldPath, pstring NewPaht, bool Msg); // INT $56
pstring MyFExpand(pstring Nm, pstring EnvName);
