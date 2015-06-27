#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "Rational.H"
#include "Scientific.H"
#include <string>
using namespace std;

class RationalException
{
    public:
        RationalException(void) {}
        ~RationalException(void) {}
        virtual void message(void) const { cout << "Rational Exception" << endl; }
    private:
};

class RationalDivideByZeroException : public RationalException
{
    public:
        RationalDivideByZeroException(const Rational& num, const Rational& den)
            : _num(num), _den(den) {}
        ~RationalDivideByZeroException(void) {}
        void message(void) const
        {
            cout << "Rational divide-by-zero: ("
                << _num << ") / (" << _den << ")" << endl;
        }
    private:
        Rational _num, _den;
};

class RationalNotSupportedException : public RationalException
{
    public:
        RationalNotSupportedException(const string& reason)
            : _reason(reason) {}
        ~RationalNotSupportedException(void) {}
        void message(void) const { cout << _reason << endl; }
    private:
        string _reason;
};

class RationalParsingException : public RationalException
{
    public:
        RationalParsingException(const string& str)
            : _str(str) {}
        ~RationalParsingException(void) {}
        void message(void) const;
    private:
        string _str;
};

class ScientificException
{
    public:
        ScientificException(void) {}
        ~ScientificException(void) {}
        virtual void message(void) const { cout << "Scientific Exception" << endl; }
    private:
};

class ScientificNotSupportedException : public ScientificException
{
    public:
        ScientificNotSupportedException(const string& reason)
            : _reason(reason) {}
        ~ScientificNotSupportedException(void) {}
        void message(void) const { cout << _reason << endl; }
    private:
        string _reason;
};

#endif
