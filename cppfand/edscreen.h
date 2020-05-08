#pragma once
#include "pstring.h"
#include "constants.h"
#include "ededit.h"

const int TXTCOLS = 80;

// PROMENNE
bool InsPage;

// METODY
void WrStatusLine();
void WriteMargins();
void WrLLMargMsg(pstring* s, WORD n);
void InitScr();
void UpdStatLine(int Row, int Col);
void EditWrline(ArrPtr P, int Row);
void ScrollWrline(ArrPtr P, int Row, ColorOrd CO);
BYTE Color(ColorOrd CO); // vno�en� do p�ede�l�
WORD PColumn(WORD w, ArrPtr P);
bool MyTestEvent();
void UpdScreen();
void WrEndL(bool Hard, int Row); // vno�en� do p�ede�l�
void Background();
