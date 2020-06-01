#include "legacy.h"
#include <SDKDDKVer.h>
#include "windows.h"
#include <ctime>
#include <direct.h>
#include "base.h"

std::vector<std::string> paramstr;
longint ExitCode = 0; // exit kód -> OS
void* ErrorAddr = nullptr; // adresa chyby
void (*ExitProc)() { }; // ukončovací procedura
BYTE OvrResult = 0; // vždy 0 OvrOK



void val(pstring s, BYTE& b, WORD& err)
{
	size_t sz;
	b = std::stoul(s.c_str(), &sz, 10);
	// přeložil se celý řetězec?
	if (sz == s.length() - 1) {	err = 0; }
	else { err = static_cast<WORD>(sz); }
}

void val(pstring s, WORD& b, WORD& err)
{
	if (s.empty()) return;
	size_t idx;
	b = std::stoul(s.c_str(), &idx, 10);
	// přeložil se celý řetězec?
	if (idx == s.length()) { err = 0; }
	else { err = static_cast<WORD>(idx); }
}

void val(pstring s, integer& b, integer& err)
{
	if (s.empty()) return;
	size_t idx;
	b = std::stoul(s.c_str(), &idx, 10);
	// přeložil se celý řetězec?
	if (idx == s.length()) { err = 0; }
	else { err = static_cast<integer>(idx);	}
}

void val(pstring s, double& b, integer& err)
{
	size_t idx = 0;
	b = std::stod(s.c_str(), &idx);
	// přeložil se celý řetězec?
	if (idx == s.length()) { err = 0; }
	else { err = static_cast<integer>(idx);	}
}

void val(pstring s, double& b, WORD& err)
{
	size_t idx = 0;
	b = std::stod(s.c_str(), &idx);
	// přeložil se celý řetězec?
	if (idx == s.length()) { err = 0; }
	else { err = static_cast<WORD>(idx); }
}

void val(pstring s, longint& b, WORD& err)
{
	if (s.length() == 0) { err = 1;	return;	}
	size_t idx = 0;
	b = std::stoi(s.c_str(), &idx);
	// přeložil se celý řetězec?
	if (idx == s.length()) { err = 0; }
	else { err = static_cast<WORD>(idx); }
}

pstring copy(pstring source, size_t index, size_t count)
{
	return source.substr(index - 1, count);
}

void str(int input, pstring& output)
{
	std::string a = std::to_string(input);
	output.replace(a.c_str());
}

void str(double input, int total, int right, pstring& output)
{
	char buffer[255];
	snprintf(buffer, sizeof(buffer), "%*.*f", total, right, input);
	output = buffer;
}

void str(double input, int right, pstring& output)
{
	char buffer[255];
	snprintf(buffer, sizeof(buffer), "%.*f", right, input);
	output = buffer;
}

WORD pred(WORD input)
{
	if (input <= 0) return 0;
	return input - 1;
}

WORD succ(WORD input)
{
	if (input == 0xFFFF) return 0xFFFF;
	return input + 1;
}

void FSplit(pstring fullname, pstring& dir, pstring& name, pstring& ext)
{
	std::string s = fullname;
	size_t foundBackslash = s.find_last_of("/\\");
	dir = s.substr(0, foundBackslash + 1);

	if (foundBackslash < s.length() - 1) // ještě nejsme na konci -> existuje název nebo název s příponou
	{
		std::string filename = s.substr(foundBackslash + 1);
		
		size_t foundDot = filename.find_last_of('.');
		if (foundDot < filename.length()) // přípona existuje
		{
			name = filename.substr(0, foundDot);
			ext = filename.substr(foundDot);
		}
		else
		{
			name = filename;
			ext = "";
		}
	}
	else
	{ // lomítko nebylo nalezeno, bude exitovat jen název a možná přípona
		size_t foundDot = s.find_last_of('.');
		if (foundDot < s.length()) // přípona existuje
		{
			name = s.substr(0, foundDot);
			ext = s.substr(foundDot);
		}
		else
		{ // přípona neexistuje, celé je to název souboru
			name = s;
			ext = "";
		}
	}
}

pstring FSearch(pstring& path, pstring& dirlist)
{
	//std::vector<std::string> vDirs;
	//int lastIndex = 0;
	//int actualIndex = 0;

	//std::string slist = dirlist;
	//for (actualIndex = 0; actualIndex < dirlist.length(); actualIndex++)
	//{
	//	if (actualIndex > 0 && dirlist[actualIndex] == ';')
	//	{
	//		std::string dir = dirlist.substr(lastIndex, actualIndex - lastIndex).c_str();
	//		if (dir[dir.length() - 1] != '\\') dir+= '\\';
	//		vDirs.push_back(dir);
	//		lastIndex = actualIndex + 1;
	//	}
	//}
	//for (auto & dir : vDirs)
	//{
	//	std::string fullname = dir + path.c_str();
	//	FILE* file;
	//	if (!fopen_s(&file, fullname.c_str(), "r")) {
	//		fclose(file);
	//		return fullname;
	//	}
	//}
	return path;
}

pstring FExpand(pstring path)
{
	pstring dir, name, ext;
	FSplit(path, dir, name, ext);

	// je cesta kompletni?
	if (dir.length() > 0 && name.length() > 0 && ext.length() > 0) return path;

	pstring fullpath = pstring(255);
	GetDir(0, &fullpath);
	fullpath += "\\";
	fullpath += name;
	if (ext.length() > 0) { fullpath += ext; }
	return fullpath;
}

void ChDir(pstring cesta)
{
	if (_chdir(cesta.c_str())) {
		HandleError = errno;
	}
}

void GetDir(BYTE disk, pstring* cesta)
{
	char buf[MAX_PATH];
	if (_getcwd(buf, MAX_PATH) == nullptr)
	{
		HandleError = errno;
	}
	*cesta = buf;
}

pstring GetDir(BYTE disk)
{
	char buf[MAX_PATH];
	if (_getcwd(buf, MAX_PATH) == nullptr)
	{
		HandleError = errno;
	}
	pstring result = buf;
	return result;
}

void MkDir(pstring cesta)
{
	if (_mkdir(cesta.c_str()))
	{
		HandleError = errno;
	}
}

void RmDir(pstring cesta)
{
	if (_rmdir(cesta.c_str()) == -1)
	{
		HandleError = errno;
	}
}

void Rename(pstring soubor, pstring novejmeno)
{
	if (rename(soubor.c_str(), novejmeno.c_str()) != 0)
	{
		HandleError = errno;
	}
}

void Erase(pstring soubor)
{
	if (remove(soubor.c_str()) == -1)
	{
		HandleError = errno;
	}
}

void InitGraph(short GraphDriver, short GraphMode, pstring PathToDriver)
{
	return;
}

void CloseGraph()
{
	return;
}

//double Random()
//{
//	//srand(time(0));
//	double randnr = rand();
//	while (randnr >= 1)
//	{
//		randnr = randnr / 10;
//	}
//	return randnr;
//}
//
//WORD Random(WORD rozsah)
//{
//	//srand(time(0));
//	int randnr = rand();
//	return randnr % rozsah;
//}

WORD ParamCount()
{
	return (WORD)paramstr.size();
}

pstring ParamStr(integer index)
{
	if (index >= paramstr.size()) return "";
	pstring ptmp = paramstr[index].c_str();
	return ptmp;
}

void FillChar(void* cil, int delka, size_t vypln)
{
	memset(cil, vypln, delka);
}

void Move(void* zdroj, void* cil, WORD delka)
{
	memmove(cil, zdroj, delka);
}

BYTE Hi(WORD cislo)
{
	return cislo >> 4;
}

BYTE Lo(WORD cislo)
{
	return cislo & 0x00FF;
}

WORD Swap(WORD cislo)
{
	return ((cislo & 0x00FF) << 4) + (cislo >> 4);
}

void UnPack(void* PackArr, WORD& NumArr, WORD& NoDigits)
{
	return;
}

void Pack(void* NumArr, WORD& PackArr, WORD& NoDigits)
{
	return;
}

double DoubleFrom6Bytes(void* buf)
{
	//BYTE arr[8]{ 0 };
	//memcpy(arr, buf, 6);
	BYTE* real48 = (BYTE*)buf;
	if (real48[0] == 0) return 0.0; // Null exponent = 0
	double exponent = real48[0] - 129.0;
	double mantissa = 0.0;
	for (int i = 1; i < 5; i++) // loop through bytes 1 - 4
	{
		mantissa += real48[i];
		mantissa *= 0.00390625; // mantissa /= 256
	}

	mantissa += (real48[5] & 0x7F);
	mantissa *= 0.0078125; // mantissa /= 128
	mantissa += 1.0;

	if ((real48[5] & 0x80) == 0x80) // Sign bit check
		mantissa = -mantissa;

	return mantissa * pow(2.0, exponent);
}

void beep()
{
}

pstring GetEnv(const char* name)
{
	size_t value = 0;
	char buffer[256];
	getenv_s(&value, buffer, 256, name);
	return pstring(buffer);
}

WORD IOResult()
{
	return 0;
}

WORD DosError()
{
	return 0;
}

double DiskFree(char disk)
{
	return 1000000000.0;
}

const char* TextFile::c_str()
{
	return nullptr;
}

void TextFile::Close()
{
}

void TextFile::Assign(const char*)
{
}

void TextFile::Reset()
{
}

void TextFile::Rewrite()
{
}

bool TextFile::ResetTxt()
{
	return false;
}




