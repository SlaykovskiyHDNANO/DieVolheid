#include "preProc.hpp"
#include "findWord.hpp"
#include <iostream>
#include <Windows.h>

using namespace std;

void main()
{
	setlocale(LC_CTYPE, "rus");
	//locale loc("Russian");
	//locale::global(loc);
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	/*preProc p("");
	cout << p.caseOf("пришел");*/
	wstring slovo = L"ЁЖ";
	findWord alg;
	alg.tryRoFindWordInNomn(slovo);
	//cout << alg.tryRoFindWordInNomn(slovo) << endl;
	cin.get();
}