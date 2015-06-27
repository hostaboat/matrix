#include "Rational.H"
#include "Exceptions.H"
#include <iostream>
#include <string>
using namespace std;

// Rational Exceptions *********************************************************
void RationalParsingException::message(void) const
{
    cout << "Error parsing Rational number at: ";
    char c = _str.back();
    if (isprint(c))
        cout << "\'" << c << "\'";
    else if (c == '\n')
        cout << "\'\\n\'";
    else if (isspace(c))
        cout << "<space>";
    else if (iscntrl(c))
        cout << "<control>";
    cout << "." << endl;
    cout << "It is the last character of: " << endl;
    if (isprint(c))
        cout << _str << endl;
    else
        cout << string(_str, 0, _str.size()-1) << endl;
}

