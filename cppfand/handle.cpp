#pragma once

#include "handle.h"

#include "common.h"
#include "legacy.h"
#include "memory.h"


bool IsHandle(WORD H)
{
	return H != 0xFF && (Handles.count(H) > 0);
}

bool IsUpdHandle(WORD H)
{
	return H != 0xFF && UpdHandles.count(H) > 0;
}

bool IsFlshHandle(WORD H)
{
	return H != 0xFF && FlshHandles.count(H) > 0;
}

void SetHandle(WORD H)
{
	if (H == 0xFF) return;
	Handles.insert(H);
	CardHandles++;
}

void SetUpdHandle(WORD H)
{
	if (H == 0xFF) return;
	UpdHandles.insert(H);
}

void SetFlshHandle(WORD H)
{
	if (H == 0xFF) return;
	FlshHandles.insert(H);
}

void ResetHandle(WORD H)
{
	if (H == 0xFF) return;
	Handles.erase(H);
	CardHandles--;
}

void ResetUpdHandle(WORD H)
{
	if (H == 0xFF) return;
	UpdHandles.erase(H);
}

void ResetFlshHandle(WORD H)
{
	if (H == 0xFF) return;
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

WORD OpenH(FileOpenMode Mode, FileUseMode UM)
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
}

WORD ReadH(WORD handle, WORD bytes, void* buffer)
{
	if (handle == 0xff) RunError(706);

	// read from file INT
	// bytes - po�et byte k p�e�ten�
	// vrac� - po�et skute�n� p�e�ten�ch
	
	return 0;
}

WORD ReadLongH(WORD handle, longint bytes, void* buffer)
{
	return ReadH(handle, bytes, buffer);
}


longint MoveH(longint dist, WORD method, WORD handle)
{
	// dist - hodnota offsetu
	// method: 0 - od zacatku, 1 - od aktualni, 2 - od konce
	// handle - file handle
	return -1;
}

longint PosH(WORD handle)
{
	return MoveH(0, 1, handle);
}

void SeekH(WORD handle, longint pos)
{
	if (handle == 0xff) RunError(705);
	MoveH(pos, 0, handle);
}

longint FileSizeH(WORD handle)
{
	longint pos = PosH(handle);
	auto result = MoveH(0, 2, handle);
	SeekH(handle, pos);
	return result;
}

void TruncH(WORD handle, longint N)
{
	// posune se na pozici N a nic na ni nezap�e? WTF?
	if (handle == 0xff) return;
	if (FileSizeH(handle) > N) {
		SeekH(handle, N);
		WriteH(handle, 0, (void*)&"");
	}
}

void CloseClearH(WORD& h)
{
	if (h == 0xFF) return;
	CloseH(h);
	ClearCacheH(h);
	h = 0xFF;
}

void SetFileAttr(WORD Attr)
{
	// nastav� atributy souboru/adres��e
	// 0 = read only, 1 = hidden file, 2 = system file, 3 = volume label, 4 = subdirectory,
	// 5 = written since backup, 8 = shareable (Novell NetWare)
}

WORD GetFileAttr()
{
	// z�sk� atributy souboru/adres��e
	return 0;
}

void RdWrCache(bool ReadOp, WORD Handle, bool NotCached, longint Pos, WORD N, void* Buf)
{
	// asi net�eba �e�it
	return;
}

void WriteH(WORD handle, WORD bytes, void* buffer)
{
	if (handle == 0xff) RunError(706);
	// ulo�� do souboru dan� po�et Byt� z bufferu
}

void WriteLongH(WORD handle, longint bytes, void* buffer)
{
	WriteH(handle, bytes, buffer);
}

void CloseH(WORD handle)
{
	// uzav�e soubor
}

void FlushH(WORD handle)
{
	// k �emu to v�echno?
	// za��d� o nov� handle N
	// zavol� SetHandle(N); SetUpdHandle(H); CloseH(H);
}

void FlushHandles()
{
	if (CardHandles == files) return;
	for (int i=0; i<files; i++)
	{
		if (IsUpdHandle(i) || IsFlshHandle(i)) FlushH(i);
	}
	ClearUpdHandles();
	ClearFlshHandles();
}

longint GetDateTimeH(WORD handle)
{
	// vr�t� �as posledn�ho z�pisu souboru + datum posledn�ho z�pisu souboru
	// 2 + 2 Byte (datum vlevo, �as vpravo)
	return 0;
}

void DeleteFile(pstring path)
{
	// sma�e soubor - INT $41
}

void RenameFile56(pstring OldPath, pstring NewPath, bool Msg)
{
	// p�esouv� nebo p�ejmenov�v� soubor
	// potom:
	if (Msg && HandleError != 0)
	{
		Set2MsgPar(OldPath, NewPath);
		RunError(829);
	}
}

pstring MyFExpand(pstring Nm, pstring EnvName)
{
	pstring d;
	GetDir(0, d);
	pstring f = FandDir;
	DelBackSlash(f);
	ChDir(f);
	pstring p = GetEnv(EnvName.c_str());
	AddBackSlash(p);
	if (!p.empty()) p += Nm;
	else {
		p = FSearch(Nm, GetEnv("PATH"));
		if (p.empty()) p = Nm; 
	}
	ChDir(d);
	return p;
}
