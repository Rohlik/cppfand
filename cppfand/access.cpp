#include "access.h"

#include "../pascal/asm.h"
#include "compile.h"
#include "FieldDescr.h"
#include "FileD.h"
#include "GlobalVariables.h"
#include "KeyFldD.h"
#include "obaseww.h"
#include "runfrml.h"
#include "../fandio/FandFile.h"
#include "../Logging/Logging.h"
#include "../Common/textfunc.h"
#include "../Common/compare.h"

void ResetCFileUpdH()
{
	ResetUpdHandle(CFile->FF->Handle);
	if (CFile->FF->file_type == FileType::INDEX) ResetUpdHandle(CFile->FF->XF->Handle);
	if (CFile->FF->TF != nullptr) ResetUpdHandle(CFile->FF->TF->Handle);
}

void ClearCacheCFile()
{
	// cache nepouzivame
	return;
	/* !!! with CFile^ do!!! */
	/*ClearCacheH(CFile->Handle);
	if (CFile->file_type == INDEX) ClearCacheH(CFile->GetXFile->Handle);
	if (CFile->TF != nullptr) ClearCacheH(CFile->TF->Handle);*/
}

void CloseClearHCFile(FandFile* fand_file)
{
	CloseClearH(&fand_file->Handle);
	if (fand_file->file_type == FileType::INDEX) {
		CloseClearH(&fand_file->XF->Handle);
	}
	if (fand_file->TF != nullptr) {
		CloseClearH(&fand_file->TF->Handle);
	}
}

void CloseGoExit(FandFile* fand_file)
{
	CloseClearHCFile(fand_file);
	GoExit();
}

BYTE ByteMask[_MAX_INT_DIG];


//const BYTE FixS = 8;
//BYTE Fix[FixS];
//BYTE RealMask[DblS + 1];



void TestCPathError()
{
	WORD n;
	if (HandleError != 0) {
		n = 700 + HandleError;
		if ((n == 705) && (CPath[CPath.length()] == '\\')) n = 840;
		SetMsgPar(CPath);
		RunError(n);
	}
}

bool LinkLastRec(FileD* FD, int& N, bool WithT)
{
	CFile = FD;
	CRecPtr = FD->GetRecSpace();
	LockMode md = CFile->NewLockMode(RdMode);
	auto result = true;
#ifdef FandSQL
	if (FD->IsSQLFile)
	{
		if (Strm1->SelectXRec(nullptr, nullptr, _equ, WithT)) N = 1; else goto label1;
	}
	else
#endif
	{
		N = CFile->FF->NRecs;
		if (N == 0) {
		label1:
			CFile->ZeroAllFlds(CRecPtr);
			result = false;
			N = 1;
		}
		else CFile->ReadRec(N, CRecPtr);
	}
	CFile->OldLockMode(md);
	return result;
}

// ulozi hodnotu parametru do souboru
void AsgnParFldFrml(FileD* FD, FieldDescr* F, FrmlElem* Z, bool Ad)
{
	//#ifdef _DEBUG
	std::string FileName = FD->FullPath;
	std::string Varible = F->Name;
	//#endif
	void* p = nullptr; int N = 0; LockMode md; bool b = false;
	FileD* cf = CFile; void* cr = CRecPtr; CFile = FD;
#ifdef FandSQL
	if (CFile->IsSQLFile) {
		CRecPtr = GetRecSpace; ZeroAllFlds; AssgnFrml(F, Z, true, Ad);
		Strm1->UpdateXFld(nullptr, nullptr, F); ClearRecSpace(CRecPtr)
	}
	else
#endif
	{
		md = CFile->NewLockMode(WrMode);
		if (!LinkLastRec(CFile, N, true)) {
			CFile->IncNRecs(1);
			CFile->WriteRec(N, CRecPtr);
		}
		AssgnFrml(CFile, CRecPtr, F, Z, true, Ad);
		CFile->WriteRec(N, CRecPtr);
		CFile->OldLockMode(md);
	}
	ReleaseStore(CRecPtr);
	CFile = cf; CRecPtr = cr;
}

// zrejme zajistuje pristup do jine tabulky (cizi klic)
bool LinkUpw(LinkD* LD, int& N, bool WithT)
{
	FileD* ToFD = LD->ToFD;
	FileD* CF = CFile;
	void* CP = CRecPtr;
	XKey* K = LD->ToKey;

	XString x;
	x.PackKF(CFile, LD->Args, CRecPtr);

	CFile = ToFD;
	void* RecPtr = CFile->GetRecSpace();
	CRecPtr = RecPtr;
#ifdef FandSQL
	if (CFile->IsSQLFile) {
		LU = Strm1->SelectXRec(K, @X, _equ, WithT); N = 1;
		if (LU) goto label2; else goto label1;
	}
#endif
	const LockMode md = CFile->NewLockMode(RdMode);
	bool lu;
	if (ToFD->FF->file_type == FileType::INDEX) {
		CFile->FF->TestXFExist();
		lu = K->SearchInterval(CFile, x, false, N);
	}
	else if (CFile->FF->NRecs == 0) {
		lu = false;
		N = 1;
	}
	else {
		lu = CFile->FF->SearchKey(x, K, N, CRecPtr);
	}

	if (lu) {
		CFile->ReadRec(N, CRecPtr);
	}
	else {
		bool b = false;
		double r = 0.0;
		CFile->ZeroAllFlds(CRecPtr);
		const KeyFldD* KF = K->KFlds;
		for (auto& arg : LD->Args) {
			FieldDescr* F = arg->FldD;
			FieldDescr* F2 = KF->FldD;
			CFile = CF;
			CRecPtr = CP;
			if ((F2->Flg & f_Stored) != 0)
				switch (F->frml_type) {
				case 'S': {
					x.S = CFile->loadOldS(F, CRecPtr);
					CFile = ToFD;
					CRecPtr = RecPtr;
					CFile->saveS(F2, x.S, CRecPtr);
					break;
				}
				case 'R': {
					r = CFile->loadR(F, CRecPtr);
					CFile = ToFD; CRecPtr = RecPtr;
					CFile->saveR(F2, r, CRecPtr);
					break;
				}
				case 'B': {
					b = CFile->loadB(F, CRecPtr);
					CFile = ToFD; CRecPtr = RecPtr;
					CFile->saveB(F2, b, CRecPtr);
					break;
				}
				}
			KF = KF->pChain;
		}
		CFile = ToFD;
		CRecPtr = RecPtr;
	}

	auto result = lu;
#ifdef FandSQL
	if (!CFile->IsSQLFile)
#endif
		CFile->OldLockMode(md);
	return result;
}

LocVar* LocVarBlkD::GetRoot()
{
	if (this->vLocVar.size() > 0) return this->vLocVar[0];
	return nullptr;
}

LocVar* LocVarBlkD::FindByName(std::string Name)
{
	for (auto& i : vLocVar) {
		if (EquUpCase(Name, i->Name)) return i;
	}
	return nullptr;
}

void* LocVarAd(LocVar* LV)
{
	return nullptr;
}

void DirMinusBackslash(pstring& D)
{
	if ((D.length() > 3) && (D[D.length() - 1] == '\\')) D[0]--;
}

void ForAllFDs(ForAllFilesOperation op, FileD** file_d, WORD i)
{
	FileD* cf = CFile;
	RdbD* R = CRdb;
	while (R != nullptr) {
		CFile = R->FD;
		while (CFile != nullptr) {
			switch (op) {
			case ForAllFilesOperation::close: {
				CFile->Close();
				break;
			}
			case ForAllFilesOperation::save: {
				break;
			}
			case ForAllFilesOperation::clear_xf_update_lock: {
				CFile->FF->ClearXFUpdLock();
				break;
			}
			case ForAllFilesOperation::save_l_mode: {
				CFile->FF->ExLMode = CFile->FF->LMode;
				break;
			}
			case ForAllFilesOperation::set_old_lock_mode: {
				CFile->OldLockMode(CFile->FF->ExLMode);
				break;
			}
			case ForAllFilesOperation::close_passive_fd: {
				if ((CFile->FF->file_type != FileType::RDB) && (CFile->FF->LMode == NullMode)) {
					CFile->CloseFile();
				}
				break;
			}
			case ForAllFilesOperation::find_fd_for_i: {
				if ((*file_d == nullptr) && (CFile->CatIRec == i)) {
					*file_d = CFile;
				}
				break;
			}
			default:;
			}
			CFile = CFile->pChain;
		}
		R = R->ChainBack;
	}
	CFile = cf;
}

void ResetCompilePars()
{
	RdFldNameFrml = RdFldNameFrmlF;
	RdFunction = nullptr;
	ChainSumEl = nullptr;
	FileVarsAllowed = true;
	FDLocVarAllowed = false;
	IdxLocVarAllowed = false;
	PrevCompInp.clear();
}

std::string TranslateOrd(std::string text)
{
	std::string trans;
	for (size_t i = 0; i < text.length(); i++) {
		char c = CharOrdTab[(BYTE)text[i]];
#ifndef FandAng
		if (c == 0x49 && trans.length() > 0) {       // znak 'H'
			if (trans[trans.length() - 1] == 0x43) { // posledni znak ve vystupnim retezci je 'C' ?
				trans[trans.length() - 1] = 0x4A;    // na vstupu bude 'J' jako 'CH'
				continue;
			}
		}
#endif
		trans += c;
	}
	return trans;
}

std::string CExtToX(const std::string dir, const std::string name, std::string ext)
{
	ext[1] = 'X';
	return dir + name + ext;
}

std::string CExtToT(const std::string& dir, const std::string& name, std::string ext)
{
	if (EquUpCase(ext, ".RDB")) ext = ".TTT";
	else if (EquUpCase(ext, ".DBF")) {
		if (CFile->FF->TF->Format == FandTFile::FptFormat) {
			ext = ".FPT";
		}
		else {
			ext = ".DBT";
		}
	}
	else ext[1] = 'T';
	return dir + name + ext;
}