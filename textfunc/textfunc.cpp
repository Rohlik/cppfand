#include "textfunc.h"

/// <summary>
/// pomocna metoda - zkontroluje delku retezce;
/// pokud je dlouhy, rozdeli jej na mensi;
/// cely nebo rozdeleny retezec pak vlozi do vektoru
/// </summary>
/// <param name="vStr">Cilovy vektor stringu</param>
/// <param name="line">Vstupni retezec</param>
/// <param name="maxLineLen">Maximalni delka retezce (zobrazitelnych znaku)</param>
void CheckMaxLineLengthAndAddToOutputVector(std::vector<std::string>& vStr, std::string& line, size_t maxLineLen)
{
	if (maxLineLen > 0 && line.length() > maxLineLen) {
		// je -> musime jej rozsekat
		size_t i2 = 0;
		while (i2 < line.length()) {
			std::string part = GetStyledStringOfLength(line, i2, maxLineLen);
			i2 += part.length();
			vStr.push_back(part);
		}
	}
	else {
		vStr.push_back(line);
	}
}

std::vector<std::string> GetAllRows(std::string input, size_t maxLineLen)
{
	std::vector<std::string> vStr;
	size_t nextStart = 0;
	for (size_t i = 0; i < input.length(); i++)
	{
		if (input[i] == '\r' || input[i] == '\n') {
			size_t nl = 1; // kolik znaku ma ukonceni?
			// narazili jsme na konec radku
			if (i < input.length() - 1 && input[i] == '\r' && input[i + 1] == '\n') {
				// jedna se o CR+LF
				nl++;
				i++;
			}
			std::string nStr = input.substr(nextStart, i - nextStart + 1 - nl);
			// neni radek delsi nez max mozny?
			CheckMaxLineLengthAndAddToOutputVector(vStr, nStr, maxLineLen);
			nextStart = i + 1;
		}
	}
	std::string nStr = input.substr(nextStart, input.length() - nextStart);
	// i posledni cast retezce muze byt prilis dlouha -> zkontolujeme
	CheckMaxLineLengthAndAddToOutputVector(vStr, nStr, maxLineLen);
	
	return vStr;
}

/// odstrani znaky na konci retezce
std::string TrailChar(std::string& input, char c)
{
	size_t Count = 0;
	for (int i = (int)input.length() - 1; i >= 0; i--) {
		if (input[i] == c) Count++;
		else break;
	}
	return input.substr(0, input.length() - Count);
}

/// odstrani znaky na zacatku retezce
std::string LeadChar(std::string& input, char c)
{
	size_t startIndex = 0;
	for (size_t i = 0; i < input.length(); i++) {
		if (input[i] == c) startIndex++;
		else break;
	}
	return input.substr(startIndex);
}

std::string GetNthLine(std::string& input, size_t from, size_t count, char delimiter)
{
	if (delimiter == '\n') delimiter = '\r'; // pokud bude pozadavek na hledani LF, budeme hledat CR
	
	if (input.empty() || from < 1 || count < 1) return "";
	size_t startIndex = 0;
	size_t stopIndex = input.find(delimiter);
	for (size_t i = 1; i < from; i++) {
		startIndex = stopIndex + 1;
		stopIndex = input.find(delimiter, stopIndex + 1);
	}
	for (size_t i = 1; i < count; i++) {
		// je pozadovano vice radku, budeme navysovat stop index
		stopIndex = input.find(delimiter, stopIndex + 1);
	}
	auto result = input.substr(startIndex, stopIndex - startIndex);

	if (delimiter == '\r') {
		// rozdelujeme podle '\r', zkontrolujeme, zda na zacatku radku nezbylo '\n'
		if (!result.empty() && result[0] == '\n')
			result.erase(0, 1);
	}
	return result;
}

size_t CountLines(std::string& input, char delimiter)
{
	size_t count = 0;
	for (char i : input)
	{
		if (i == delimiter) count++;
	}
	// casti bude o 1 vic, nez je delicich znaku
	return count + 1;
}

std::string GetStyledStringOfLength(std::string& input, size_t from, size_t length)
{
	if (length == 0) return "";
	if (input.length() <= length) return input;
	std::string result;
	size_t charsInserted = 0;
	for (size_t i = from; i < input.length(); i++)
	{
		char c = input[i];
		result += c;
		if (!(c == 0x13 || c == 0x17 || c == 0x11 || c == 0x04
			|| c == 0x02 || c == 0x05 || c == 0x01)) charsInserted++;
		if (charsInserted == length) break;
	}
	return result;
}

std::string RepeatString(std::string& input, size_t count)
{
	std::string result;
	result.reserve(input.length() * count);
	for (size_t i = 0; i < count; i++) {
		result += input;
	}
	return result;
}

std::string RepeatString(char input, size_t count)
{
	std::string result;
	result.reserve(count);
	for (size_t i = 0; i < count; i++) {
		result += input;
	}
	return result;
}

// old functions
WORD CountDLines(void* Buf, WORD L, char C)
{
	std::string s = std::string((char*)Buf, L);
	return (WORD)CountLines(s, C);
}

//pstring GetDLine(void* Buf, WORD L, char C, WORD I) // I = 1 .. N
//{
//	std::string input((const char*)Buf, L);
//	std::vector<std::string> lines;
//	size_t pos = 0;
//	while ((pos = input.find(C)) != std::string::npos) {
//		std::string token = input.substr(0, pos);
//		lines.push_back(token);
//		input.erase(0, pos + 1); // smazani vc. oddelovace
//	}
//	lines.push_back(input); // pridame zbyvajici cast retezce
//	if (I <= lines.size()) return lines[I - 1]; // Pascal cislovani od 1
//	return "";
//}