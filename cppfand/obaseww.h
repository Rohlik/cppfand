#pragma once
#include "constants.h"

using namespace std;

// �. 142
void WriteWFrame(BYTE WFlags, string top, string bottom);
void WrHd(ScreenStr s, string Hd, WORD Row, WORD MaxCols);
void CFileMsg(WORD n, char Typ); // �. 279
