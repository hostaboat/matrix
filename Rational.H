#ifndef _RATIONAL_H
#define _RATIONAL_H

#include <cinttypes>
#include <iostream>
#include <string>
using namespace std;

class Rational
{
    public:
        Rational(void);
        Rational(int64_t n);
        Rational(int64_t n, int64_t d);
        Rational(const string& s);
        Rational(const Rational& n);
        ~Rational(void);

        Rational& operator=(const Rational& rhs);
        Rational& operator+=(const Rational& rhs);
        Rational& operator-=(const Rational& rhs);
        Rational& operator*=(const Rational& rhs);
        Rational& operator/=(const Rational& rhs);
        Rational& operator^=(const Rational& rhs);

        Rational& operator++(void);
        Rational operator++(int);
        Rational& operator--(void);
        Rational operator--(int);

        friend bool operator==(const Rational& lhs, const Rational& rhs);
        friend bool operator>=(const Rational& lhs, const Rational& rhs);
        friend bool operator<=(const Rational& lhs, const Rational& rhs);

        double toFloat(void) const;
        void reduce(void);
        void invert(void);

        void print(ostream& os) const;
        void read(istream& is);

    protected:
        Rational get_number(istream& is);

        int8_t  _sign;
        int64_t _num;
        int64_t _den;

    private:
        uint64_t _gcd(uint64_t a, uint64_t b);
};

bool operator==(const Rational& lhs, const Rational& rhs);
bool operator!=(const Rational& lhs, const Rational& rhs);
bool operator<(const Rational& lhs, const Rational& rhs);
bool operator>(const Rational& lhs, const Rational& rhs);
bool operator>=(const Rational& lhs, const Rational& rhs);
bool operator<=(const Rational& lhs, const Rational& rhs);

Rational operator+(const Rational& lhs, const Rational& rhs);
Rational operator-(const Rational& lhs, const Rational& rhs);
Rational operator*(const Rational& lhs, const Rational& rhs);
Rational operator/(const Rational& lhs, const Rational& rhs);
Rational operator^(const Rational& lhs, const Rational& rhs);

ostream& operator<<(ostream& os, const Rational& rhs);
istream& operator>>(istream& is, Rational& rhs);

#endif
