#include "runfrml.h"

/// funkce m� asi o��znout v�echny C na konci �et�zce?
string runfrml::TrailChar(char C, string S)
{
	// ze vstupn�ho stringu ud�l�me pole znak�
	char* tmpchar = new char[S.length()];
	S.copy(tmpchar, S.length());
	tmpchar[S.length()] = '\0';
	
	/*while (Length(S)>0) and (S[Length(S)]=C) do S[0]:=chr(length(S)-1); TrailChar:=S;*/
	while (strlen(tmpchar) > 0 && strrchr(tmpchar, C))
	{
		*strrchr(tmpchar, C) = '\0';
	}
	string result(tmpchar);
	delete[] tmpchar;
	tmpchar = nullptr;
	return result;
}
