#include "FandTFile.h"

#include "files.h"
#include "../cppfand/Coding.h"
#include "../cppfand/FileD.h"
#include "../cppfand/GlobalVariables.h"
#include "../cppfand/obaseww.h"
#include "../pascal/random.h"
#include "../pascal/real48.h"
#include "../Common/textfunc.h"
#include "../Common/compare.h"

struct TT1Page
{
	unsigned short Signum = 0;
	unsigned short OldMaxPage = 0;
	int FreePart = 0;
	bool Rsrvd1 = false, CompileProc = false, CompileAll = false;
	unsigned short IRec = 0;
	// potud se to nekoduje (13B)
	// odtud jsou polozky prohnany XORem
	int FreeRoot = 0, MaxPage = 0;   /*eldest version=>array Pw[1..40] of char;*/
	double TimeStmp = 0.0;
	bool HasCoproc = false;
	char Rsrvd2[25]{ '\0' };
	char Version[4]{ '\0' };
	unsigned char LicText[105]{ 0 };
	unsigned char Sum = 0;
	char X1[295]{ '\0' };
	unsigned short LicNr = 0;
	char X2[11]{ '\0' };
	char PwNew[40]{ '\0' };
	unsigned char Time = 0;

	void Load(unsigned char* input512);
	void Save(unsigned char* output512);
};

// nahraje nactenych 512B ze souboru do struktury
void TT1Page::Load(unsigned char* input512)
{
	size_t index = 0;
	memcpy(&Signum, &input512[index], 2); index += 2;
	memcpy(&OldMaxPage, &input512[index], 2); index += 2;
	memcpy(&FreePart, &input512[index], 4); index += 4;
	memcpy(&Rsrvd1, &input512[index], 1); index++;
	memcpy(&CompileProc, &input512[index], 1); index++;
	memcpy(&CompileAll, &input512[index], 1); index++;
	memcpy(&IRec, &input512[index], 2); index += 2;
	memcpy(&FreeRoot, &input512[index], 4); index += 4;
	memcpy(&MaxPage, &input512[index], 4); index += 4;
	TimeStmp = Real48ToDouble(&input512[index]); index += 6;
	memcpy(&HasCoproc, &input512[index], 1); index++;
	memcpy(&Rsrvd2, &input512[index], 25); index += 25;
	memcpy(&Version, &input512[index], 4); index += 4;
	memcpy(&LicText, &input512[index], 105); index += 105;
	memcpy(&Sum, &input512[index], 1); index++;
	memcpy(&X1, &input512[index], 295); index += 295;
	memcpy(&LicNr, &input512[index], 2); index += 2;
	memcpy(&X2, &input512[index], 11); index += 11;
	memcpy(&PwNew, &input512[index], 40); index += 40;
	memcpy(&Time, &input512[index], 1); index++;
	// index by ted mel mit hodnotu 512
	if (index != 512) throw std::exception("Error in TT1Page::Load");
}

// ulozi strukturu 512B do souboru
void TT1Page::Save(unsigned char* output512)
{
	size_t index = 0;
	memcpy(&output512[index], &Signum, 2); index += 2;
	memcpy(&output512[index], &OldMaxPage, 2); index += 2;
	memcpy(&output512[index], &FreePart, 4); index += 4;
	memcpy(&output512[index], &Rsrvd1, 1); index++;
	memcpy(&output512[index], &CompileProc, 1); index++;
	memcpy(&output512[index], &CompileAll, 1); index++;
	memcpy(&output512[index], &IRec, 2); index += 2;
	memcpy(&output512[index], &FreeRoot, 4); index += 4;
	memcpy(&output512[index], &MaxPage, 4); index += 4;

	const std::array<unsigned char, 6> time_stamp = DoubleToReal48(TimeStmp);
	for (size_t i = 0; i < 6; i++, index++) {
		output512[index] = time_stamp[i];
	}

	memcpy(&output512[index], &HasCoproc, 1); index++;
	memcpy(&output512[index], &Rsrvd2, 25); index += 25;
	memcpy(&output512[index], &Version, 4); index += 4;
	memcpy(&output512[index], &LicText, 105); index += 105;
	memcpy(&output512[index], &Sum, 1); index++;
	memcpy(&output512[index], &X1, 295); index += 295;
	memcpy(&output512[index], &LicNr, 2); index += 2;
	memcpy(&output512[index], &X2, 11); index += 11;
	memcpy(&output512[index], &PwNew, 40); index += 40;
	memcpy(&output512[index], &Time, 1); index++;
	// index by ted mel mit hodnotu 512
	if (index != 512) throw std::exception("Error in TT1Page::Load");
}

void RandIntByBytes(int& nr)
{
	unsigned char* byte = (unsigned char*)&nr;
	for (size_t i = 0; i < 4; i++)
	{
		byte[i] = byte[i] ^ Random(255);
	}
}

void RandByteByBytes(unsigned short& nr)
{
	unsigned char* byte = (unsigned char*)&nr;
	for (size_t i = 0; i < 2; i++) {
		byte[i] = byte[i] ^ Random(255);
	}
}

void RandReal48ByBytes(double& nr)
{
	unsigned char* byte = (unsigned char*)&nr;
	for (size_t i = 0; i < 6; i++) {
		byte[i] = byte[i] ^ Random(255);
	}
}

void RandBooleanByBytes(bool& nr)
{
	unsigned char* byte = (unsigned char*)&nr;
	for (size_t i = 0; i < sizeof(nr); i++) {
		byte[i] = byte[i] ^ Random(255);
	}
}

void RandArrayByBytes(void* arr, size_t len)
{
	unsigned char* byte = (unsigned char*)arr;
	for (size_t i = 0; i < len; i++) {
		byte[i] = byte[i] ^ Random(255);
	}
}

FandTFile::FandTFile(FandFile* parent)
{
	_parent = parent;
}

FandTFile::FandTFile(const FandTFile& orig, FandFile* parent)
{
	_parent = parent;
	Format = orig.Format;
}

FandTFile::~FandTFile()
{
	if (Handle != nullptr) {
		CloseH(&Handle);
	}
}

void FandTFile::Err(unsigned short n, bool ex)
{
	if (IsWork) {
		SetMsgPar(FandWorkTName);
		WrLLF10Msg(n);
		if (ex) GoExit();
	}
	else {
		FileMsg(_parent->GetFileD(), n, 'T');
		if (ex) CloseGoExit(_parent->GetFileD()->FF);
	}
}

void FandTFile::TestErr()
{
	if (HandleError != 0) Err(700 + HandleError, true);
}

int FandTFile::UsedFileSize()
{
	if (Format == FptFormat) return FreePart * FptFormatBlockSize;
	else return int(MaxPage + 1) << MPageShft;
}

bool FandTFile::NotCached()
{
	return !IsWork && _parent->NotCached();
}

bool FandTFile::Cached()
{
	return !NotCached();
}

void FandTFile::RdPrefix(bool Chk)
{
	TT1Page T;
	int* TNxtAvailPage = (int*)&T; /* .DBT */
	struct stFptHd { int FreePart = 0; unsigned short X = 0, BlockSize = 0; }; /* .FPT */
	stFptHd* FptHd = (stFptHd*)&T;
	unsigned char sum = 0;
	int FS = 0, ML = 0, RS = 0;
	unsigned short i = 0, n = 0;
	if (Chk) {
		FS = FileSizeH(Handle);
		if (FS <= 512) {
			//FillChar(PwCode, 40, '@');
			PwCode = "";
			PwCode = AddTrailChars(PwCode, '@', 40);
			Coding::Code(PwCode);
			SetEmpty();
			return;
		}
	}
	unsigned char header512[512];
	RdWrCache(READ, Handle, NotCached(), 0, 512, header512);
	srand(RS);
	LicenseNr = 0;
	if (Format == DbtFormat) {
		MaxPage = *TNxtAvailPage - 1;
		GetMLen();
		return;
	}
	if (Format == FptFormat) {
		FreePart = SwapLong((*FptHd).FreePart);
		FptFormatBlockSize = Swap((*FptHd).BlockSize);
		return;
	}

	// nactena data jsou porad v poli, je nutne je nahrat do T
	T.Load(header512);

	FreePart = T.FreePart; // 4B
	Reserved = T.Rsrvd1; // 1B
	CompileProc = T.CompileProc; // 1B
	CompileAll = T.CompileAll; // 1B
	IRec = T.IRec; // 2B
	FreeRoot = T.FreeRoot; // 4B
	MaxPage = T.MaxPage; // 4B
	TimeStmp = T.TimeStmp; // 6B v Pascalu, 8B v C++ 

	if (!IsWork 
		&& (_parent->GetFileD() == Chpt) 
		&& ((T.HasCoproc != HasCoproc) || (CompArea(Version, T.Version, 4) != _equ))) {
		CompileAll = true;
	}
	if (T.OldMaxPage == 0xffff) {
		goto label1;
	}
	else {
		FreeRoot = 0;
		if (FreePart > 0) {
			if (!Chk) FS = FileSizeH(Handle);
			ML = FS;
			MaxPage = (FS - 1) >> MPageShft;
			GetMLen();
		}
		else {
			FreePart = -FreePart; MaxPage = T.OldMaxPage;
		label1:
			GetMLen();
			ML = MLen;
			if (!Chk) FS = ML;
		}
	}
	if (IRec >= 0x6000) {
		IRec = IRec - 0x2000;
		if (!IsWork && (_parent->file_type == FileType::RDB)) LicenseNr = T.LicNr;
	}
	if (IRec >= 0x4000) {
		IRec = IRec - 0x4000;
		RandSeed = ML + T.Time;
		RandIntByBytes(T.FreeRoot);
		RandIntByBytes(T.MaxPage);
		RandReal48ByBytes(T.TimeStmp);
		RandBooleanByBytes(T.HasCoproc);
		RandArrayByBytes(T.Rsrvd2, 25);
		RandArrayByBytes(T.Version, 4);
		RandArrayByBytes(T.LicText, 105);
		RandArrayByBytes(&T.Sum, 1);
		RandArrayByBytes(T.X1, 295);
		RandByteByBytes(T.LicNr);
		RandArrayByBytes(T.X2, 11);
		RandArrayByBytes(T.PwNew, 40);
		PwCode = std::string(&T.PwNew[0], 20);
		Pw2Code = std::string(&T.PwNew[20], 20);
	}
	else {
		RandSeed = ML + T.Time;
		RandIntByBytes(T.FreeRoot);
		RandIntByBytes(T.MaxPage);
		RandReal48ByBytes(T.TimeStmp);
		RandBooleanByBytes(T.HasCoproc);
		RandArrayByBytes(T.Rsrvd2, 25);
		PwCode = std::string(&T.PwNew[0], 20);
		Pw2Code = std::string(&T.PwNew[20], 20);
	}
	Coding::Code(PwCode);
	Coding::Code(Pw2Code);
	if ((FreePart < MPageSize) || (FreePart > ML) || (FS < ML) ||
		(FreeRoot > MaxPage) || (MaxPage == 0)) {
		Err(893, false);
		MaxPage = (FS - 1) >> MPageShft;
		FreeRoot = 0;
		GetMLen();
		FreePart = NewPage(true);
		SetUpdHandle(Handle);
	}
	eofPos = (MaxPage + 1) * MPageSize;
	srand(RS);
}

void FandTFile::WrPrefix()
{
	TT1Page T;

	switch (Format) {
	case DbtFormat: {
		int* TNxtAvailPage = (int*)&T;		/* .DBT */
		memset(&T, ' ', sizeof(T));
		*TNxtAvailPage = MaxPage + 1;
		break;
	}
	case FptFormat: {
		struct stFptHd { int FreePart = 0; unsigned short X = 0, BlockSize = 0; }; /* .FPT */
		stFptHd* FptHd = (stFptHd*)&T;
		memset(&T, 0, sizeof(T));
		(*FptHd).FreePart = SwapLong(FreePart);
		(*FptHd).BlockSize = Swap(FptFormatBlockSize);
		break;
	}
	case T00Format: {
		int RS = 0;
		unsigned short n = 0;
		unsigned short i = 0;
		memset(&T, '@', sizeof(T));
		std::string Pw = PwCode + Pw2Code;
		Coding::Code(Pw);
		RandSeed = RS;
		if (LicenseNr != 0) {
			for (i = 0; i < 20; i++) {
				Pw[i] = static_cast<char>(Random(255));
			}
		}
		n = 0x4000;
		T.Time = Random(100);
		if (Pw.length() != 40) {
			throw std::exception("Bad PwCode + Pw2Code length! Must be 40.");
		}
		memcpy(T.PwNew, Pw.c_str(), Pw.length());
		RandSeed = MLen + T.Time; // srand(MLen + T.Time);
		// for (i = 14; i < 511; i++) TX[i] = TX[i] ^ Random(255);
		RandIntByBytes(T.FreeRoot);
		RandIntByBytes(T.MaxPage);
		RandReal48ByBytes(T.TimeStmp);
		RandBooleanByBytes(T.HasCoproc);
		RandArrayByBytes(T.Rsrvd2, 25);
		RandArrayByBytes(T.Version, 4);
		RandArrayByBytes(T.LicText, 105);
		RandArrayByBytes(&T.Sum, 1);
		RandArrayByBytes(T.X1, 295);
		RandByteByBytes(T.LicNr);
		RandArrayByBytes(T.X2, 11);
		RandArrayByBytes(T.PwNew, 40);

		T.LicNr = LicenseNr;
		if (LicenseNr != 0) {
			unsigned char sum = 0;
			n = 0x6000;
			sum = T.LicNr;
			for (i = 0; i < 105; i++) sum += T.LicText[i];
			T.Sum = sum;
		}
		T.FreePart = FreePart; // 8B
		T.Rsrvd1 = Reserved; // 1B
		T.CompileProc = CompileProc; // 1B
		T.CompileAll = CompileAll; // 1B
		T.IRec = IRec; // 2B
		T.FreeRoot = FreeRoot; // 4B
		T.MaxPage = MaxPage; // 4B
		T.TimeStmp = TimeStmp; // 6B Pascal

		T.OldMaxPage = 0xffff;
		T.Signum = 1;
		T.IRec += n;
		memcpy(T.Version, Version, 4);
		T.HasCoproc = HasCoproc;
		RandSeed = RS;
		break;
	}
	default:;
	}

	unsigned char header512[512]{ 0 };
	T.Save(header512);
	RdWrCache(WRITE, Handle, NotCached(), 0, 512, header512);
}

void FandTFile::SetEmpty()
{
	unsigned char X[MPageSize];
	short* XL = (short*)&X;
	switch (Format) {
	case DbtFormat: {
		MaxPage = 0;
		WrPrefix();
		break;
	}
	case FptFormat: {
		FreePart = 8; FptFormatBlockSize = 64;
		WrPrefix();
		break;
	}
	case T00Format: {
		FreeRoot = 0; MaxPage = 1; FreePart = MPageSize; MLen = 2 * MPageSize;
		WrPrefix();
		memset(X, 0, MPageSize); //FillChar(X, MPageSize, 0); 
		*XL = -510;
		RdWrCache(WRITE, Handle, NotCached(), MPageSize, MPageSize, X);
		break;
	}
	default: break;
	}
}

void FandTFile::Create(const std::string& path)
{
	Handle = OpenH(path, _isOverwriteFile, Exclusive);
	TestErr();
	IRec = 1; LicenseNr = 0;

	PwCode = "";
	PwCode = AddTrailChars(PwCode, '@', 20);
	Coding::Code(PwCode);

	Pw2Code = "";
	Pw2Code = AddTrailChars(Pw2Code, '@', 20);
	Coding::Code(Pw2Code);

	eofPos = 2 * MPageSize;

	SetEmpty();
}

int FandTFile::NewPage(bool NegL)
{
	int PosPg;
	unsigned char X[MPageSize]{ 0 };
	int* L = (int*)&X;
	if (FreeRoot != 0) {
		PosPg = FreeRoot << MPageShft;
		RdWrCache(READ, Handle, NotCached(), PosPg, 4, &FreeRoot);
		if (FreeRoot > MaxPage) {
			Err(888, false);
			FreeRoot = 0;
			goto label1;
		}
	}
	else {
	label1:
		MaxPage++;
		MLen += MPageSize;
		PosPg = MaxPage << MPageShft;
		//pos = eofPos;			// NE
		eofPos += MPageSize;		// prodlouzim soubor o logickou stranku
		//TruncH(this->Handle, eofPos);			// kvuli FANDu i o fyzickou
	}
	//FillChar(X, MPageSize, 0); 
	if (NegL) *L = -510;
	RdWrCache(WRITE, Handle, NotCached(), PosPg, MPageSize, X);
	return PosPg;
}

void FandTFile::ReleasePage(int PosPg)
{
	unsigned char X[MPageSize]{ 0 };
	*(__int32*)X = FreeRoot;
	RdWrCache(WRITE, Handle, NotCached(), PosPg, MPageSize, X);
	FreeRoot = PosPg >> MPageShft;
}

LongStr* FandTFile::Read(int Pos)
{
	LongStr* s = nullptr;
	unsigned short i = 0, l = 0;
	char* p = nullptr;
	int offset = 0;
	struct stFptD { int Typ = 0, Len = 0; } FptD;
	Pos -= LicenseNr;
	if (Pos <= 0 /*OldTxt=-1 in RDB!*/) {
		s = new LongStr(l);
		s->LL = 0;
		return s;
	}
	else {
		switch (Format) {
		case T00Format: {
			if ((Pos < MPageSize) || (Pos >= MLen)) {
				Err(891, false);
				return s;
			}
			RdWrCache(READ, Handle, NotCached(), Pos, 2, &l);
			if (l > MaxLStrLen + 1) {
				Err(891, false);
				s = new LongStr(l);
				s->LL = 0;
				return s;
			}
			if (l == MaxLStrLen + 1) { l--; } // 65001
			s = new LongStr(l + 2);
			s->LL = l;
			RdWr(READ, Pos + 2, l, s->A);
			break;
		}
		case DbtFormat: {
			s = new LongStr(32768); //(LongStr*)GetStore(32770);
			Pos = Pos << MPageShft;
			p = s->A;
			l = 0;
			while (l <= 32768 - MPageSize) {
				RdWrCache(READ, Handle, NotCached(), Pos, MPageSize, &p[offset]);
				for (i = 1; i <= MPageSize; i++) {
					if (p[offset + i] == 0x1A) {
						s->LL = l;
						//ReleaseStore(&s->A[l + 1]);
						return s;
					}
					l++;
				}
				offset += MPageSize;
				Pos += MPageSize;
			}
			l--;
			s->LL = l;
			//ReleaseStore(&s->A[l + 1]);
			break;
		}
		case FptFormat: {
			Pos = Pos * FptFormatBlockSize;
			RdWrCache(READ, Handle, NotCached(), Pos, sizeof(FptD), &FptD);
			if (SwapLong(FptD.Typ) != 1/*text*/) {
				s = new LongStr(l);
				s->LL = 0;
				return s;
			}
			else {
				l = SwapLong(FptD.Len) & 0x7FFF;
				s = new LongStr(l);
				s->LL = l;
				RdWrCache(READ, Handle, NotCached(), Pos + sizeof(FptD), l, s->A);
			}
			break;
		}
		default: break;
		}
	}
	return s;
}

int FandTFile::Store(char* s, size_t l)
{
	int pos = 0;
	char X[MPageSize + 1]{ 0 };
	struct stFptD { int Typ = 0, Len = 0; } FptD;

	if (l == 0) {
		return pos;
	}

	SetUpdHandle(Handle);

	switch (Format) {
	case T00Format: {
		if (l > MaxLStrLen) {
			l = MaxLStrLen;
		}
		if (l > MPageSize - 2) {
			// long text
			pos = NewPage(false);
		}
		else {
			// short text
			int rest = MPageSize - FreePart % MPageSize;
			if (l + 2 <= rest) {
				pos = FreePart;
			}
			else {
				pos = NewPage(false);
				FreePart = pos;
				rest = MPageSize;
			}
			if (l + 4 >= rest) FreePart = NewPage(false);
			else {
				FreePart += l + 2;
				rest = l + 4 - rest;
				RdWrCache(WRITE, Handle, NotCached(), FreePart, 2, &rest);
			}
		}
		RdWrCache(WRITE, Handle, NotCached(), pos, 2, &l);
		RdWr(WRITE, pos + 2, l, s);
		break;
	}
	case DbtFormat: {
		pos = MaxPage + 1;
		int N = pos << MPageShft;
		if (l > 0x7fff) l = 0x7fff;
		RdWrCache(WRITE, Handle, NotCached(), N, l, s);
		FillChar(X, MPageSize, ' ');
		X[0] = 0x1A; X[1] = 0x1A;
		int rest = MPageSize - (l + 2) % MPageSize;
		RdWrCache(WRITE, Handle, NotCached(), N + l, rest + 2, X);
		MaxPage += (l + 2 + rest) / MPageSize;
		break;
	}
	case FptFormat: {
		pos = FreePart;
		int N = FreePart * FptFormatBlockSize;
		if (l > 0x7fff) l = 0x7fff;
		FreePart = FreePart + (sizeof(FptD) + l - 1) / FptFormatBlockSize + 1;
		FptD.Len = SwapLong(l);
		RdWrCache(WRITE, Handle, NotCached(), N, sizeof(FptD), &FptD);
		N += sizeof(FptD);
		RdWrCache(WRITE, Handle, NotCached(), N, l, s);
		N += l;
		l = FreePart * FptFormatBlockSize - N;
		if (l > 0) {
			unsigned char* p = new unsigned char[l];
			FillChar(p, l, ' ');
			RdWrCache(WRITE, Handle, NotCached(), N, l, p);
			delete[] p; p = nullptr;
		}
		break;
	}
	default: break;
	}
	return pos;
}

void FandTFile::CloseFile()
{
	if (Handle != nullptr) {
		CloseClearH(&Handle);
		if (HandleError == 0) Handle = nullptr;
		if ((!_parent->IsShared()) && (_parent->NRecs == 0) && (_parent->file_type != FileType::DBF)) {
			SetPathAndVolume(_parent->GetFileD());
			CPath = CExtToT(CDir, CName, CExt);
			MyDeleteFile(CPath);
		}
	}
}

void FandTFile::Delete(int pos)
{
	// funkce smaze cast T00 nebo TTT souboru
	// s ohledem na to, ze v TTT souboru jsou predkompilovana data
	// a tato data pak nejsou nicim nahrazena (novou kompilaci neumime ulozit)
	// dojde ke ztrate dat

	// proto budeme kontrolovat, zda se jedna o TTT soubor a pokud ano,
	// tak koncime a nic mazat nebudeme

	/*if (_parent->GetFileD() != nullptr) {
		std::string name = upperCaseString(_parent->GetFileD()->FullPath);
		if (name.find("TTT") != std::string::npos) {
			return;
		}
	}*/

	if (pos <= 0) return;
	if ((Format != T00Format) || NotCached()) return;
	if ((pos < MPageSize) || (pos >= MLen)) {
		Err(889, false);
		return;
	}

	SetUpdHandle(Handle);
	if (pos < MPageSize || pos >= eofPos)
		return;								// mimo datovou oblast souboru

	unsigned char X[MPageSize]{ 0 };
	__int32 posPg = (__int32)pos & (0xFFFFFFFF << MPageShft);
	__int32 PosI = (__int32)pos & (MPageSize - 1);

	RdWrCache(READ, Handle, NotCached(), posPg, MPageSize, X);
	void* wp = (unsigned short*)&X[PosI];
	__int32 l = *(unsigned short*)wp;

	if (l <= MPageSize - 2) {
		// small text on 1 page
		*(short*)&X[PosI] = (short)(-l);
		__int32 N = 0;
		wp = (short*)X;
		while (N < MPageSize - 2) {
			if (*(short*)wp > 0) {
				memset(&X[PosI + 2], 0, l);
				goto label1;
			}
			N = N - *(short*)wp + 2;
			wp = (short*)&X[N];
		}
		if ((FreePart >= posPg) && (FreePart < posPg + MPageSize)) {
			memset(X, 0, MPageSize);
			*(short*)X = -510;
			FreePart = posPg;
		label1:
			RdWrCache(WRITE, Handle, NotCached(), posPg, MPageSize, X);
		}
		else {
			ReleasePage(posPg);
		}
	}
	else {
		// long text on more than 1 page
		if (PosI != 0) {
			goto label3;
		}
	label2:
		l = *(unsigned short*)X;
		if (l > MaxLStrLen + 1) {
		label3:
			Err(889, false);
			return;
		}
		const bool IsLongTxt = (l == MaxLStrLen + 1);
		l += 2;
	label4:
		ReleasePage(posPg);
		if ((l > MPageSize) || IsLongTxt) {
			posPg = *(__int32*)&X[MPageSize - 4];

			if ((posPg < MPageSize) || (posPg + MPageSize > MLen)) {
				Err(888, false);
				return;
			}
			RdWrCache(READ, Handle, NotCached(), posPg, MPageSize, X);
			if (l <= MPageSize) {
				goto label2;
			}
			l = l - (MPageSize - 4);
			goto label4;
		}
	}
}

void FandTFile::RdWr(FileOperation operation, size_t position, size_t count, char* buffer)
{
	Logging* log = Logging::getInstance();
	// log->log(loglevel::DEBUG, "FandTFile::RdWr() 0x%p %s pos: %i, len: %i", Handle, ReadOp ? "read" : "write", position, count);
	unsigned short L = 0;
	int NxtPg = 0;
	int offset = 0;
	unsigned short Rest = MPageSize - (unsigned short(position) & (MPageSize - 1));
	while (count > Rest) {
		L = Rest - 4;
		RdWrCache(operation, Handle, NotCached(), position, L, &buffer[offset]);
		offset += L;
		count -= L;
		if (operation == WRITE) {
			NxtPg = NewPage(false);
		}
		RdWrCache(operation, Handle, NotCached(), position + L, 4, &NxtPg);
		position = NxtPg;
		if ((operation == READ) && ((position < MPageSize) || (position + MPageSize > MLen))) {
			Err(890, false);
			memset(&buffer[offset], ' ', count);
			return;
		}
		Rest = MPageSize;
	}
	RdWrCache(operation, Handle, NotCached(), position, count, &buffer[offset]);
}

void FandTFile::GetMLen()
{
	MLen = (MaxPage + 1) << MPageShft;
}

void WrDBaseHd()
{
	
}

