#ifndef _SCIENTIFIC_H
#define _SCIENTIFIC_H

#include "Rational.H"
#include <cinttypes>
#include <iostream>
using namespace std;

class Scientific : public Rational
{
    public:
        Scientific(void);
        Scientific(int64_t n);
        Scientific(int64_t n, int8_t e);
        Scientific(int64_t n, int64_t d, int8_t e);
        Scientific(const Rational& r);
        Scientific(const Rational& r, int8_t e);
        Scientific(const Scientific& n);

        ~Scientific(void);

        Scientific& operator=(const Scientific& rhs);
        Scientific& operator+=(const Scientific& rhs);
        Scientific& operator-=(const Scientific& rhs);
        Scientific& operator*=(const Scientific& rhs);
        Scientific& operator/=(const Scientific& rhs);
        Scientific& operator^=(const Scientific& rhs);

        Scientific& operator++(void);
        Scientific operator++(int);
        Scientific& operator--(void);
        Scientific operator--(int);

        friend bool operator==(const Scientific& lhs, const Scientific& rhs);
        friend bool operator>=(const Scientific& lhs, const Scientific& rhs);
        friend bool operator<=(const Scientific& lhs, const Scientific& rhs);

        double toFloat(void) const;
        Rational toRational(void) const;

        void print(ostream& os) const;
        void read(istream& is);

        void reduce(void);
        friend void equate(Scientific& f1, Scientific& f2);

    private:
        int8_t get_exponent(istream& is);

        int8_t  _exp;
};

void equate(Scientific& f1, Scientific& f2);

bool operator==(const Scientific& lhs, const Scientific& rhs);
bool operator!=(const Scientific& lhs, const Scientific& rhs);
bool operator<(const Scientific& lhs, const Scientific& rhs);
bool operator>(const Scientific& lhs, const Scientific& rhs);
bool operator>=(const Scientific& lhs, const Scientific& rhs);
bool operator<=(const Scientific& lhs, const Scientific& rhs);

Scientific operator+(const Scientific& lhs, const Scientific& rhs);
Scientific operator-(const Scientific& lhs, const Scientific& rhs);
Scientific operator*(const Scientific& lhs, const Scientific& rhs);
Scientific operator/(const Scientific& lhs, const Scientific& rhs);
Scientific operator^(const Scientific& lhs, const Scientific& rhs);

ostream& operator<<(ostream& os, const Scientific& rhs);
istream& operator>>(istream& is, Scientific& rhs);

#endif
