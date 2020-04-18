#pragma once

#include "handle.h"


#include <errhandlingapi.h>
#include <fileapi.h>


#include "common.h"
#include "drivers.h"
#include "kbdww.h"
#include "keybd.h"
#include "legacy.h"
#include "memory.h"


bool IsHandle(filePtr H)
{
	if (H == nullptr) return false;
	return Handles.count(H) > 0;
}

bool IsUpdHandle(filePtr H)
{
	if (H == nullptr) return false;
	return UpdHandles.count(H) > 0;
}

bool IsFlshHandle(filePtr H)
{
	if (H == nullptr) return false;
	return FlshHandles.count(H) > 0;
}

void SetHandle(filePtr H)
{
	if (H == nullptr) return;
	Handles.insert(H);
	CardHandles++;
}

void SetUpdHandle(filePtr H)
{
	if (H == nullptr) return;
	UpdHandles.insert(H);
}

void SetFlshHandle(filePtr H)
{
	if (H == nullptr) return;
	FlshHandles.insert(H);
}

void ResetHandle(filePtr H)
{
	if (H == nullptr) return;
	Handles.erase(H);
	CardHandles--;
}

void ResetUpdHandle(filePtr H)
{
	if (H == nullptr) return;
	UpdHandles.erase(H);
}

void ResetFlshHandle(filePtr H)
{
	if (H == nullptr) return;
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

filePtr OpenH(FileOpenMode Mode, FileUseMode UM)
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

	pstring s;

	string txt[] = { "Clos", "OpRd", "OpRs", "OpSh", "OpEx" };

	pstring path = CPath;
	if (CardHandles == files) RunError(884);
	longint w = 0;
	pstring openFlags(5);
label1:
	switch (Mode) {
	case _isoldfile:
	case _isoldnewfile:
	{
		openFlags = UM == RdOnly ? "rb" : "r+b";
		break;
	}
	case _isoverwritefile:
	{
		openFlags = "w+b";
		break;
	}
	case _isnewfile:
	{
		openFlags = UM == RdOnly ? "rb" : "r+b";
		break;
	}
	}

	filePtr nFile = nullptr;
	HandleError = fopen_s(&nFile, path.c_str(), openFlags.c_str());

	// https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-doserrno-sys-errlist-and-sys-nerr?view=vs-2019
	if (IsNetCVol() && (HandleError == EACCES || HandleError == ENOLCK))
	{
		if (w == 0)
		{
			Set2MsgPar(path, txt[UM]);
			w = PushWrLLMsg(825, false);
		}
		Drivers::LockBeep();
		KbdTimer(spec.NetDelay, 0);
		goto label1;
	}

	if (HandleError == 0)
	{
		SetHandle(nFile);
		if (Mode != _isoldfile) SetUpdHandle(nFile);
	}

	else if (HandleError == ENOENT) // No such file or directory
	{
		if (Mode == _isoldnewfile)
		{
			Mode = _isnewfile;
			goto label1;
		}
	}
	if (w != 0) PopW(w);

	return nFile;
}

WORD ReadH(filePtr handle, WORD bytes, void* buffer)
{
	return ReadH(handle, bytes, buffer);
	// read from file INT
	// bytes - po�et byte k p�e�ten�
	// vrac� - po�et skute�n� p�e�ten�ch
}

WORD ReadLongH(filePtr handle, longint bytes, void* buffer)
{
	if (handle == nullptr) RunError(706);
	if (bytes <= 0) return 0;
	auto readed = fread_s(buffer, bytes, 1, bytes, handle);
	if (readed != static_cast<unsigned int>(bytes))
	{
		// nebyl na�ten po�adovan� po�et B
		auto eofReached = feof(handle);
		HandleError = ferror(handle);
	}
	return WORD(readed);
}

void WriteH(filePtr handle, WORD bytes, void* buffer)
{
	WriteLongH(handle, bytes, buffer);
}

void WriteLongH(filePtr handle, longint bytes, void* buffer)
{
	if (handle == nullptr) RunError(706);
	if (bytes <= 0) return;
	// ulo�� do souboru dan� po�et Byt� z bufferu
	fwrite(buffer, 1, bytes, handle);
	HandleError = ferror(handle);
}

longint MoveH(longint dist, WORD method, filePtr handle)
{
	if (handle == nullptr) return -1;
	// dist - hodnota offsetu
	// method: 0 - od zacatku, 1 - od aktualni, 2 - od konce
	// handle - file handle
	HandleError = fseek(handle, dist, method);
	return ftell(handle);
}

longint PosH(filePtr handle)
{
	const auto result = ftell(handle);
	HandleError = ferror(handle);
	return static_cast<longint>(result);
}

void SeekH(filePtr handle, longint pos)
{
	if (handle == nullptr) RunError(705);
	MoveH(pos, 0, handle);
}

longint FileSizeH(filePtr handle)
{
	longint pos = PosH(handle);
	auto result = MoveH(0, 2, handle);
	SeekH(handle, pos);
	return result;
}

void TruncH(filePtr handle, longint N)
{
	// posune se na pozici N a nic na ni nezap�e? WTF?
	if (handle == nullptr) return;
	if (FileSizeH(handle) > N) {
		SeekH(handle, N);
		SetEndOfFile(handle);
	}
}

void CloseClearH(filePtr h)
{
	if (h == nullptr) return;
	CloseH(h);
	ClearCacheH(h);
	h = nullptr;
}

void SetFileAttr(WORD Attr)
{
	// nastav� atributy souboru/adres��e
	// 0 = read only, 1 = hidden file, 2 = system file, 3 = volume label, 4 = subdirectory,
	// 5 = written since backup, 8 = shareable (Novell NetWare)
	if (SetFileAttributesA(CPath.c_str(), Attr)== 0)
	{
		HandleError = GetLastError();
	}
}

WORD GetFileAttr()
{
	// z�sk� atributy souboru/adres��e
	auto result = GetFileAttributesA(CPath.c_str());
	if (result == INVALID_FILE_ATTRIBUTES) HandleError = GetLastError();
}

void RdWrCache(bool ReadOp, filePtr Handle, bool NotCached, longint Pos, WORD N, void* Buf)
{
	if (Handle == nullptr) return;
	// asi net�eba �e�it
	return;
}

void CloseH(filePtr handle)
{
	if (handle == nullptr) return;
	// uzav�e soubor
	HandleError = fclose(handle);
}

void FlushH(filePtr handle)
{
	if (handle == nullptr) return;
	// k �emu to v�echno?
	// za��d� o nov� handle N
	// zavol� SetHandle(N); SetUpdHandle(H); CloseH(H);
}

void FlushHandles()
{
	//if (CardHandles == files) return;
	//for (int i = 0; i < files; i++)
	//{
	//	if (IsUpdHandle(i) || IsFlshHandle(i)) FlushH(i);
	//}
	//ClearUpdHandles();
	//ClearFlshHandles();
}

longint GetDateTimeH(filePtr handle)
{
	if (handle == nullptr) return;
	// vr�t� �as posledn�ho z�pisu souboru + datum posledn�ho z�pisu souboru
	// 2 + 2 Byte (datum vlevo, �as vpravo)
	FILETIME ft;
	auto result = GetFileTime(handle, nullptr, nullptr, &ft);
	if (result == 0) HandleError = GetLastError();
	return (ft.dwHighDateTime << 16) + ft.dwLowDateTime;
}

void DeleteFile(pstring path)
{
	// sma�e soubor - INT $41
	auto result = remove(path.c_str());
	if (result != 0) HandleError = result;
}

void RenameFile56(pstring OldPath, pstring NewPath, bool Msg)
{
	// p�esouv� nebo p�ejmenov�v� soubor
	// potom:
	auto result = rename(OldPath.c_str(), NewPath.c_str());
	if (result != 0) HandleError = result;
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
		char* env = GetEnv("PATH");
		pstring envps = pstring(env);
		p = FSearch(Nm, envps);
		if (p.empty()) p = Nm;
	}
	ChDir(d);
	return p;
}
