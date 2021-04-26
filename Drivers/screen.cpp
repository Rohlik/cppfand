#include "screen.h"
#include <exception>
#include <stdarg.h>
#include <vector>
#include "../textfunc/textfunc.h"

const unsigned int BUFFSIZE = 128 * 1024;

Screen::Screen(WORD* TxtCols, WORD* TxtRows, Wind* WindMin, Wind* WindMax, TCrs* Crs)
{
	Screen(*TxtCols, *TxtRows, WindMin, WindMax, Crs);
}

Screen::Screen(short TxtCols, short TxtRows, Wind* WindMin, Wind* WindMax, TCrs* Crs)
{
	this->TxtCols = TxtCols;
	this->TxtRows = TxtRows;
	this->MaxColsIndex = (short)(TxtCols - 1);
	this->MaxRowsIndex = (short)(TxtRows - 1);
	this->WindMin = WindMin;
	this->WindMax = WindMax;
	this->Crs = Crs;

	_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (_handle == INVALID_HANDLE_VALUE) { throw std::exception("Cannot open console output handle."); }
	SMALL_RECT rect{ 0, 0, 79, 24 };
	SetConsoleWindowInfo(_handle, true, &rect);
	SetConsoleScreenBufferSize(_handle, { TxtCols, TxtRows });
	SetConsoleTitle("C++ FAND");
	//DWORD consoleMode = ENABLE_VIRTUAL_TERMINAL_PROCESSING; // | ENABLE_LVB_GRID_WORLDWIDE;
	//bool scm = SetConsoleMode(_handle, 0);
	_actualIndex = 0;
	_inBuffer = 0;
}

Screen::~Screen()
{
	//delete[] _scrBuf;
}

size_t Screen::BufSize()
{
	return BUFFSIZE;
}

void Screen::ScrClr(WORD X, WORD Y, WORD SizeX, WORD SizeY, char C, BYTE Color)
{
	// cislovani radku a sloupcu prichazi od 1 .. X
	if (X < 1 || Y < 1) { throw std::exception("Bad ScrClr index."); }

	DWORD written = 0;
	CHAR_INFO* _buf = new CHAR_INFO[SizeX * SizeY];
	COORD BufferSize = { (short)SizeX, (short)SizeY };
	SMALL_RECT rect = { X - 1, Y - 1, X + SizeX, Y + SizeY };

	CHAR_INFO ci; ci.Char.AsciiChar = C; ci.Attributes = Color;
	for (int i = 0; i < SizeX * SizeY; i++) { _buf[i] = ci; }
	WriteConsoleOutput(_handle, _buf, BufferSize, { 0, 0 }, &rect);

	delete[] _buf;
}

void Screen::ScrWrChar(WORD X, WORD Y, char C, BYTE Color)
{
	SMALL_RECT rect = { X - 1, Y - 1, X, Y };
	CHAR_INFO ci; ci.Char.AsciiChar = C; ci.Attributes = Color;
	WriteConsoleOutput(_handle, &ci, { 1, 1 }, { 0, 0 }, &rect);
}


void Screen::ScrWrStr(WORD X, WORD Y, std::string S, BYTE Color)
{
	// TODO: doresit zobrazeni znaku jako '\r' nebo '\n'

	short len = S.length();
	CHAR_INFO* _buf = new CHAR_INFO[len];
	COORD BufferSize = { len, 1 };
	SMALL_RECT rect = { (short)X - 1, (short)Y - 1, (short)X + len - 1, (short)Y - 1 };

	CHAR_INFO ci;
	ci.Attributes = Color;
	for (int i = 0; i < len; i++)
	{
		ci.Char.AsciiChar = S[i];
		_buf[i] = ci;
	}
	WriteConsoleOutputA(_handle, _buf, BufferSize, { 0, 0 }, &rect);
	delete[] _buf;
}

void Screen::ScrWrFrameLn(WORD X, WORD Y, BYTE Typ, BYTE Width, BYTE Color)
{
	std::string txt;
	txt.reserve(Width);
	txt += FrameChars[Typ];
	for (int i = 0; i < Width - 2; i++) { txt += FrameChars[Typ + 1]; }
	txt += FrameChars[Typ + 2];
	ScrWrStr(X, Y, txt, Color);
}

void Screen::ScrWrText(WORD X, WORD Y, const char* S)
{
	// tady se spatne tisknuly "systemove" znaky v metode "WriteConsoleOutputCharacterA"
	// proto se vyctou udaje o radku (barva pozadi a textu), doplni se do nich novy text
	// pak se poslou metodou "WriteConsoleOutputA" na konzoli
	X += WindMin->X - 1;
	Y += WindMin->Y - 1;
	DWORD written = 0;

	// budeme predpokladat, ze se muze zobrazit jen 1 radek
	// jeho delka bude dle nastaveneho okna:
	size_t len = min(WindMax->X - WindMin->X + 1, strlen(S));

	// vycteme oblast do bufferu
	auto buff = new CHAR_INFO[len];
	SMALL_RECT XY = { (short)X - 1, (short)Y - 1, (short)X + len - 1, (short)Y -1 };
	ReadConsoleOutput(_handle, buff, { (short)len, 1 }, { 0, 0 }, &XY);
	// vezme jednotlive znaky a "opravime je" ve vyctenem bufferu
	for (size_t i = 0; i < len; i++) {
		buff[i].Char.AsciiChar = S[i];
	}
	// vypisem buffer na obrazovku
	WriteConsoleOutputA(_handle, buff, { (short)len, 1 }, { 0, 0 }, &XY);
	//WriteConsoleOutputCharacterA(_handle, S, len, { (short)X - 1, (short)Y - 1 }, &written);
	delete[] buff;
	if (X + len > TxtCols) { GotoXY(1, Y + 1, absolute); }
	else { GotoXY(X + len, Y, absolute); }
}

void Screen::ScrFormatWrText(WORD X, WORD Y, char const* const _Format, ...)
{
	va_list args;
	va_start(args, _Format);
	char buffer[255]{ 0 };
	vsnprintf(buffer, sizeof(buffer), _Format, args);
	Screen::ScrWrText(X, Y, buffer);
	va_end(args);
}

void Screen::ScrFormatWrStyledText(WORD X, WORD Y, BYTE Color, char const* const _Format, ...)
{
	// souradnice jsou relativni, tiskneme do aktualniho okna
	X += WindMin->X - 1;
	Y += WindMin->Y - 1;
	
	va_list args;
	va_start(args, _Format);
	char buffer[255];
	vsnprintf(buffer, sizeof(buffer), _Format, args);
	size_t len = strlen(buffer);
	auto buff = new CHAR_INFO[len];
	SMALL_RECT XY = { (short)X - 1, (short)Y - 1, (short)X + len - 1, (short)Y - 1 };
	
	for (size_t i = 0; i < len; i++) {
		buff[i].Attributes = Color;
		buff[i].Char.AsciiChar = buffer[i];
	}
	WriteConsoleOutputA(_handle, buff, { (short)len, 1 }, { 0, 0 }, &XY);
	// posuneme souradnici X o vytistene znaky
	GotoXY(WhereXabs() + (WORD)len, WhereYabs(), absolute);
	delete[] buff;
	va_end(args);
}

// vypise pole WORDu (Attr + Znak)
void Screen::ScrWrBuf(WORD X, WORD Y, void* Buf, WORD L)
{
	//X++; // v Pacalu to bylo od 1
	//Y++; // --""--
	SMALL_RECT XY = { (short)X, (short)Y, (short)X + L, (short)Y + 1 };
	COORD BufferSize = { (short)L, 1 };

	// zkonvertujeme WORD do CHAR_INFO
	WORD* pBuf = (WORD*)Buf;
	CHAR_INFO* ci = new CHAR_INFO[L];
	for (int i = 0; i < L; i++)
	{
		ci[i].Attributes = pBuf[i] >> 8;
		ci[i].Char.AsciiChar = pBuf[i] & 0x00FF;
	}
	WriteConsoleOutputA(_handle, ci, BufferSize, { 0, 0 }, &XY);
	delete[] ci;
}

// vypise pole CHAR_INFO
void Screen::ScrWrCharInfoBuf(short X, short Y, CHAR_INFO* Buf, short L)
{
	//X++; // v Pacalu to bylo od 1
	//Y++; // --""--
	SMALL_RECT XY = { X - 1, Y - 1, (short)(X + L), (short)(Y + 1) };
	COORD BufferSize = { (short)L, 1 };
	WriteConsoleOutputA(_handle, Buf, BufferSize, { 0, 0 }, &XY);
}

bool Screen::ScrRdBuf(WORD X, WORD Y, CHAR_INFO* Buf, WORD L)
{
	SMALL_RECT rect{ (short)X, (short)Y, (short)X + L - 1, (short)Y };
	COORD bufSize{ (short)(L), 1 };
	//CHAR_INFO* buf = new CHAR_INFO[bufSize.X * bufSize.Y];
	bool result = ReadConsoleOutput(_handle, Buf, bufSize, { 0, 0 }, &rect);
	return result;
}

void Screen::ScrMove(short X, short Y, short ToX, short ToY, short L)
{
	// souradnice chodi kupodivu od 0 ..
	if (L < 1) return;
	// ulozime obsah obrazovky a "pretiskneme" na jine misto
	CrsHide();
	// cislovani radku a sloupcu prichazi od 1 .. X
	if ((X < 0) || (X > MaxColsIndex) || (Y < 0) || (Y > MaxRowsIndex)) 
		throw std::exception("Bad ScrMove index.");
	if ((ToX < 0) || (ToX > MaxColsIndex) || (ToY < 0) || (ToY > MaxRowsIndex))
		throw std::exception("Bad ScrMove index.");
	SMALL_RECT rectFrom{ (short)X, (short)Y, (short)X + L, (short)Y };
	SMALL_RECT rectTo{ (short)ToX, (short)ToY, (short)ToX + L, (short)ToY };
	COORD bufSize{ (short)L, 1 };
	CHAR_INFO* buf = new CHAR_INFO[L * 1];
	ReadConsoleOutput(_handle, buf, bufSize, { 0, 0 }, &rectFrom);
	WriteConsoleOutputA(_handle, buf, bufSize, { 0, 0 }, &rectTo);
	CrsShow();
}

void Screen::ScrColor(WORD X, WORD Y, WORD L, BYTE Color)
{
	DWORD written = 0;
	FillConsoleOutputAttribute(_handle, Color, L, { (short)X, (short)Y }, &written);
}

// vypise na zadanou pozici 1 znak v zadane barve
void Screen::WriteChar(short X, short Y, char C, BYTE attr, Position pos)
{
	DWORD written = 0;
	switch (pos) {
	case relative: {
		X += WindMin->X - 1;
		Y += WindMin->Y - 1;
		break;
	}
	case absolute: {
		break;
	}
	case actual: {
		X = WhereXabs();
		Y = WhereYabs();
		break;
	}
	default:;
	}
	WORD color = attr;
	auto resatr = WriteConsoleOutputAttribute(_handle, &color, 1, { X - 1, Y - 1 }, &written);
	auto result = WriteConsoleOutputCharacterA(_handle, &C, 1, { X - 1, Y - 1}, &written);
	GotoXY(WhereXabs() + 1, WhereYabs(), absolute); // po zapisu poseneme kurzor
}

// vypise stylizovany text do aktualniho okna a vrati pocet vypsanych znaku
size_t Screen::WriteStyledStringToWindow(std::string text, BYTE Attr)
{
	if (text.length() == 0) return 0;

	std::string CStyle;
	std::string CColor;
	CColor = (char)Attr;

	// celkovy pocet vytistenych znaku
	size_t totalChars = 0;

	CHAR_INFO ci;

	short cols = WindMax->X - WindMin->X + 1;
	short rows = WindMax->Y - WindMin->Y + 1;

	// ziskame jendotlive radky textu
	//std::vector<std::string> vStr;// = GetAllRows(text, cols);
	auto vStr = GetAllRows(text, cols);

	// buffer bude mit delku jednoho radku okna
	CHAR_INFO* _buf = new CHAR_INFO[cols];

	// prevezmeme aktualni pozici kurzoru:
	actualWindowCol = WhereX();
	actualWindowRow = WhereY();

	// pocet radku je mensi hodnota z poctu textu nebo radku okna
	short rowsToPrint = min(rows, vStr.size());
	for (size_t i = 0; i < rowsToPrint; i++)
	{
		auto str = vStr[i];
		auto strLen = str.length();
		// okenko bude mit jen 1 radek
		SMALL_RECT rect = { 
			WindMin->X + actualWindowCol - 2,			// oba parametry jsou cislovane od 1
			WindMin->Y + actualWindowRow - 2,			// oba parametry jsou cislovane od 1
			WindMax->X - 1,								// prava strana zustava stejna
			WindMin->Y + actualWindowRow - 2,			// dolni strana stejna jako horni (jen 1 radek)
		};

		size_t ctrlCharsCount = 0;

		for (size_t i = 0; i < strLen; i++)
		{
			char c = str[i];
			BYTE a = 0;
			if (SetStyleAttr(c, a))
			{
				ctrlCharsCount++;
				size_t i = CStyle.find_first_of(c);
				if (i != std::string::npos)
				{
					CStyle.erase(i, 1);
					CColor.erase(i, 1);
				}
				else {
					CStyle = c + CStyle;
					CColor = (char)a + CColor;
				}
				Attr = CColor[0];
				continue;
			}
			if (c == '\n' || c == '\r') {
				ctrlCharsCount++;
				continue;
			}
			ci.Attributes = Attr;
			ci.Char.AsciiChar = c;
			size_t position = i - ctrlCharsCount;
			if (position > cols - 1) {
				// retezec se do radku nevleze, ale budeme pokracovat kvuli nastaveni barev
				continue;
			}
			_buf[position] = ci;
		}
		COORD BufferSize = { strLen - ctrlCharsCount, 1 }; // pocet tisknutelnych znaku, 1 radek
		WriteConsoleOutputA(_handle, _buf, BufferSize, { 0, 0 }, &rect);
		totalChars += strLen - ctrlCharsCount;
		// nastavime zacatek dalsiho radku, pokud se nejedna o posledni radek
		if (i < rowsToPrint - 1) {
			actualWindowRow++;
			actualWindowCol = 1;
		}
		// pokud se jedna o posledni radek, nastavime korektne RELATIVNI souradnice
		else {
			// pokud jsme na konci radku, prejdeme na zacatek
			if (BufferSize.X + 1 > WindMax->X) {
				GotoXY(1, actualWindowRow);
			}
			else {
				GotoXY(BufferSize.X + 1, actualWindowRow);
			}
		}
	}
	delete[] _buf;
	return totalChars;
}

void Screen::LF()
{
	actualWindowRow++;
	actualWindowCol = 1;
	GotoXY(actualWindowCol, actualWindowRow);
}

bool Screen::SetStyleAttr(char C, BYTE& a)
{
	auto result = true;
	if (C == 0x13) a = colors.tUnderline;
	else if (C == 0x17) a = colors.tItalic;
	else if (C == 0x11) a = colors.tDWidth;
	else if (C == 0x04) a = colors.tDStrike;
	else if (C == 0x02) a = colors.tEmphasized;
	else if (C == 0x05) a = colors.tCompressed;
	else if (C == 0x01) a = colors.tElite;
	else result = false;
	return result;
}

TCrs Screen::CrsGet()
{
	TCrs crs;
	crs.X = WhereXabs();
	crs.Y = WhereYabs();
	crs.Big = Crs->Big;
	crs.Enabled = Crs->Enabled;
	crs.Ticks = 0;
	return crs;
}

void Screen::CrsSet(TCrs S)
{
	CrsHide();
	Crs->X = S.X;
	Crs->Y = S.Y;
	Crs->Big = S.Big;
	Crs->Enabled = S.Enabled;
	GotoXY(Crs->X, Crs->Y, absolute);
	if (Crs->Enabled) CrsShow();
}

void Screen::CrsShow()
{
	CONSOLE_CURSOR_INFO visible{ 1, true };
	SetConsoleCursorInfo(_handle, &visible);
	Crs->Enabled = true;
}

void Screen::CrsHide()
{
#ifndef _DEBUG
	CONSOLE_CURSOR_INFO invisible{ 1, false };
	SetConsoleCursorInfo(_handle, &invisible);
	Crs->Enabled = false;
#else
	CrsShow();
#endif
}

void Screen::CrsBig()
{
	if (!Crs->Big) { CrsHide(); Crs->Big = true; } CrsShow();
}

void Screen::CrsNorm()
{
	if (Crs->Big) { CrsHide(); Crs->Big = false; } CrsShow();
}

void Screen::GotoXY(WORD X, WORD Y, Position pos)
{
	// if (X > WindMax->X || Y > WindMax->Y) return;
	switch (pos)
	{
	case relative: {
		X = X + WindMin->X - 1;
		Y = Y + WindMin->Y - 1;
		break;
	}
	case absolute: break;
	case actual: return;
	default: return;
	}
	bool succ = SetConsoleCursorPosition(_handle, { (short)X - 1, (short)Y - 1 });
	if (!succ) {
		printf("GotoXY() fail");
	}
}

short Screen::WhereX()
{
	// vrac� relativn� pozici (k aktu�ln�mu oknu) ��slovanou od 1
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	GetConsoleScreenBufferInfo(_handle, &sbi);
	return (sbi.dwCursorPosition.X + 1) - WindMin->X + 1;
}

short Screen::WhereY()
{
	// vrac� relativn� pozici (k aktu�ln�mu oknu) ��slovanou od 1
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	GetConsoleScreenBufferInfo(_handle, &sbi);
	return (sbi.dwCursorPosition.Y + 1) - WindMin->Y + 1;
}

short Screen::WhereXabs()
{
	// vrac� absolutn� pozici ��slovanou od 1
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	GetConsoleScreenBufferInfo(_handle, &sbi);
	return sbi.dwCursorPosition.X + 1;
}

short Screen::WhereYabs()
{
	// vrac� absolutn� pozici ��slovanou od 1
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	GetConsoleScreenBufferInfo(_handle, &sbi);
	return sbi.dwCursorPosition.Y + 1;
}

void Screen::Window(BYTE X1, BYTE Y1, BYTE X2, BYTE Y2)
{
	// p�vodn� k�d z ASM
	if (X2 < X1) return;
	if (Y2 < Y1) return;
	if (X2 > TxtCols) return;
	if (Y2 > TxtRows) return;
	WindMin->X = X1;
	WindMin->Y = Y1;
	WindMax->X = X2;
	WindMax->Y = Y2;
	actualWindowRow = 1;
	actualWindowCol = 1;
	GotoXY(1, 1, relative);
}

void Screen::ScrWr()
{
	throw std::exception("Screen::ScrWr() not implemented");
}

void Screen::CrsDark()
{
	throw std::exception("Screen::CrsDark() not implemented");
}

void Screen::CrsBlink()
{
	throw std::exception("Screen::CrsBlink() not implemented");
}

void Screen::CrsGotoXY(WORD aX, WORD aY)
{
	Crs->X = aX;
	Crs->Y = aY;
	bool succ = SetConsoleCursorPosition(_handle, { (short)Crs->X, (short)Crs->Y });
	if (!succ) {
		printf("GotoXY() fail");
	}
}

int Screen::ScrPush1(WORD X, WORD Y, WORD SizeX, WORD SizeY, void* P)
{
	SMALL_RECT rect{ (short)X, (short)Y, (short)X + (short)SizeX, (short)Y + (short)SizeY };
	// do ukazatele z�ejm� ulo�� obsah videopam�ti ...
	ReadConsoleOutput(_handle, (CHAR_INFO*)P, { (short)SizeX, (short)SizeY }, { 0, 0 }, &rect);
	return SizeX * SizeY;
}

void Screen::pushScreen(storeWindow sw)
{
	_windowStack.push(sw);
}

storeWindow Screen::popScreen()
{
	auto result = _windowStack.top();
	_windowStack.pop();
	return result;
}

int Screen::SaveScreen(WParam* wp, short c1, short r1, short c2, short r2)
{
	// cislovani radku a sloupcu prichazi od 1 .. X
	if (c1 < 1 || c2 > 80 || r1 < 1 || r2 > 25) { throw std::exception("Bad SaveScreen index."); }

	c1--; c2--;
	r1--; r2--;

	SMALL_RECT rect{ c1, r1, c2, r2 };
	COORD bufSize{ (short)(c2 - c1 + 1), (short)r2 - r1 + 1 };
	CHAR_INFO* buf = new CHAR_INFO[bufSize.X * bufSize.Y];
	ReadConsoleOutput(_handle, buf, bufSize, { 0, 0 }, &rect);
	_windowStack.push({ wp, bufSize, rect, buf });
	return _windowStack.size();
}

WParam* Screen::LoadScreen(bool draw)
{
	if (_windowStack.empty()) {
			printf("Screen::LoadScreen() zasobnik je prazdny!!!\n");
		return nullptr;
	}
	auto scr = _windowStack.top();
	_windowStack.pop();
	if (draw) {
		WriteConsoleOutput(_handle, scr.content, scr.coord, { 0, 0 }, &scr.rect);
	}
	delete[] scr.content;
	return scr.wp;
}
