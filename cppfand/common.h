#pragma once
#include "constants.h"

using namespace std;

void AddBackSlash(string s);

// �. 37
bool SEquUpcase(string s1, string s2);

// �. 362
float Today();

// �. 55
void StrLPCopy(string& Dest, string s, WORD MaxL);

// �. 235
void SplitDate(float R, WORD& d, WORD& m, WORD& y);

//�. 217
bool OlympYear(WORD year);

//�. 219
WORD OlympYears(WORD year);
