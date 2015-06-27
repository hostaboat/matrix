#include "Rational.H"
#include "Exceptions.H"
#include <cinttypes>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <vector>
using namespace std;

Rational::Rational(void)
    : _sign(1), _num(0), _den(1)
{
}

Rational::Rational(int64_t n)
{
    if (n < 0)
        _sign = -1;
    else
        _sign = 1;

    _num = abs(n);
    _den = 1;
}

Rational::Rational(int64_t n, int64_t d)
{
    if (d == 0)
        throw RationalDivideByZeroException(n, d);

    if ((n < 0) && (d < 0))
        _sign = 1;
    else if ((n < 0) || (d < 0))
        _sign = -1;
    else
        _sign = 1;

    _num = abs(n);
    _den = abs(d);

    reduce();
}

Rational::Rational(const string& s)
{
    istringstream iss(s);
    read(iss);
    reduce();
}

Rational::Rational(const Rational& n)
    : _sign(n._sign), _num(n._num), _den(n._den)
{
    reduce();
}

Rational::~Rational(void)
{
}

Rational& Rational::operator=(const Rational& rhs)
{
    if (this != &rhs)
    {
        _sign = rhs._sign;
        _num = rhs._num;
        _den = rhs._den;
    }

    return *this;
}

uint64_t Rational::_gcd(uint64_t a, uint64_t b)
{
    if ((a == 0) || (b == 0))
        return 0;

    if ((a == 1) || (b == 1))
        return 1;

    if (a == b)
        return a;

    uint64_t d = 0;

    while (!(a & 1) && !(b & 1))
    {
        a >>= 1;
        b >>= 1;
        d++;
    }

    while (a != b)
    {
        if (!(a & 1))
            a >>= 1;
        else if (!(b & 1))
            b >>= 1;
        else if (a > b)
            a = ((a - b) >> 1);
        else
            b = ((b - a) >> 1);
    }

    a <<= d;

    return a;
}

void Rational::reduce(void)
{
    if ((_num == 1) || (_den == 1))
        return;

    if (_num == 0)
    {
        _den = 1;
        _sign = 1;
        return;
    }

    if (_num == _den)
    {
        _num = 1;
        _den = 1;
        return;
    }

    uint64_t gcd = _gcd((uint64_t)_num, (uint64_t)_den);
    
    if (gcd == 1)
        return;

    _num /= gcd;
    _den /= gcd;
}

void Rational::invert(void)
{
    int64_t swap = _num;
    _num = _den;
    _den = swap;
}

bool operator==(const Rational& lhs, const Rational& rhs)
{
    if (&lhs == &rhs)
        return true;

    if ((lhs._num == rhs._num) && (lhs._den == rhs._den)
            && (lhs._sign == rhs._sign))
        return true;

    return false;

}

bool operator!=(const Rational& lhs, const Rational& rhs)
{
    return !(rhs == lhs);
}

bool operator<=(const Rational& lhs, const Rational& rhs)
{
    if (&lhs == &rhs)
        return true;

    if (lhs._sign != rhs._sign)
    {
        if (lhs._sign < rhs._sign)
            return true;

        return false;
    }

    if (lhs._den == rhs._den)
    {
        if (lhs._num <= rhs._num)
            return true;

        return false;
    }

    if ((lhs._num * rhs._den) <= (rhs._num * lhs._den))
        return true;

    return false;
}

bool operator<(const Rational& lhs, const Rational& rhs)
{
    return !(rhs <= lhs);
}

bool operator>=(const Rational& lhs, const Rational& rhs)
{
    return (rhs <= lhs);
}

bool operator>(const Rational& lhs, const Rational& rhs)
{
    return (rhs < lhs);
}

Rational& Rational::operator+=(const Rational& rhs)
{
    if ((_num == 0) || (rhs._num == 0))
    {
        if (rhs._num != 0)
        {
            _sign = rhs._sign;
            _num = rhs._num;
            _den = rhs._den;
        }

        return *this;
    }

    uint64_t gcd = _gcd(_den, rhs._den);
    int64_t num = (_sign * _num * (rhs._den/gcd)) + (rhs._sign * rhs._num * (_den/gcd));
    int64_t den = _den/gcd * rhs._den;

    _num = abs(num);
    _den = den;
    _sign = (num < 0) ? -1 : 1;

    return *this;
}

Rational operator+(const Rational& lhs, const Rational& rhs)
{
    Rational result(lhs);
    result += rhs;
    return result;
}

Rational& Rational::operator-=(const Rational& rhs)
{
    if ((_num == 0) || (rhs._num == 0))
    {
        if (rhs._num != 0)
        {
            _sign = -1 * rhs._sign;
            _num = rhs._num;
            _den = rhs._den;
        }

        return *this;
    }

    uint64_t gcd = _gcd(_den, rhs._den);
    int64_t num = (_sign * _num * (rhs._den/gcd)) - (rhs._sign * rhs._num * (_den/gcd));
    int64_t den = _den/gcd * rhs._den;

    _num = abs(num);
    _den = den;
    _sign = (num < 0) ? -1 : 1;

    return *this;
}

Rational operator-(const Rational& lhs, const Rational& rhs)
{
    Rational result(lhs);
    result -= rhs;
    return result;
}

Rational& Rational::operator*=(const Rational& rhs)
{
    if ((_num == 0) || (rhs._num == 0))
    {
        if (_num != 0)
        {
            _sign = 1;
            _num = 0;
            _den = 1;
        }

        return *this;
    }
    else if ((rhs._num == 1) && (rhs._den == 1))
    {
        _sign *= rhs._sign;

        return *this;
    }
    else if ((_num == 1) && (_den == 1))
    {
        _sign *= rhs._sign;
        _num = rhs._num;
        _den = rhs._den;

        return *this;
    }

    uint64_t gcd1 = _gcd(_num, rhs._den);
    uint64_t gcd2 = _gcd(_den, rhs._num);

    _num = _num/gcd1 * rhs._num/gcd2;
    _den = _den/gcd2 * rhs._den/gcd1;
    _sign *= rhs._sign;

    return *this;
}

Rational operator*(const Rational& lhs, const Rational& rhs)
{
    Rational result(lhs);
    result *= rhs;
    return result;
}

Rational& Rational::operator/=(const Rational& rhs)
{
    if ((_num == 0) || (rhs._num == 0))
    {
        if (rhs._num == 0)
            throw RationalDivideByZeroException(*this, rhs);

        return *this;
    }
    else if ((rhs._num == 1) && (rhs._den == 1))
    {
        _sign *= rhs._sign;

        return *this;
    }
    else if ((_num == 1) && (_den == 1))
    {
        _sign *= rhs._sign;
        _num = rhs._den;
        _den = rhs._num;

        return *this;
    }

    uint64_t gcd1 = _gcd(_num, rhs._num);
    uint64_t gcd2 = _gcd(_den, rhs._den);

    _num = _num/gcd1 * rhs._den/gcd2;
    _den = _den/gcd2 * rhs._num/gcd1;
    _sign *= rhs._sign;

    return *this;
}

Rational operator/(const Rational& lhs, const Rational& rhs)
{
    Rational result(lhs);
    result /= rhs;
    return result;
}

Rational& Rational::operator^=(const Rational& rhs)
{
    if (rhs._den != 1)
    {
        throw RationalNotSupportedException(
                "Exponentiation is only supported with integers");
    }

    if (rhs == 0)
    {
        _num = 1;
        _den = 1;
        _sign = 1;

        return *this;
    }
    else if (rhs == 1)
    {
        return *this;
    }
    else if (rhs == -1)
    {
        invert();
        return *this;
    }

#if 0
    Rational base(*this);
    Rational result(1);

    if (rhs < 0)
        result.invert();

    int64_t exp = rhs._num;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    *this = result;
#endif

    int64_t nbase, dbase;
    int64_t nresult = 1, dresult = 1;
    int64_t exp = rhs._num;

    _sign = (exp & 1) ? _sign : 1;

    if (rhs._sign > 0)
    {
        nbase = _num;
        dbase = _den;
    }
    else
    {
        nbase = _den;
        dbase = _num;
    }

    while (exp)
    {
        if (exp & 1)
        {
            nresult *= nbase;
            dresult *= dbase;
        }

        exp >>= 1;

        nbase *= nbase;
        dbase *= dbase;
    }

    _num = nresult;
    _den = dresult;

    return *this;
}

Rational operator^(const Rational& lhs, const Rational& rhs)
{
    Rational result(lhs);
    result ^= rhs;
    return result;
}

Rational& Rational::operator++(void)
{
    *this += 1;
    return *this;
}

Rational Rational::operator++(int)
{
    Rational result(*this);
    ++(*this);
    return result;
}

Rational& Rational::operator--(void)
{
    *this -= 1;
    return *this;
}

Rational Rational::operator--(int)
{
    Rational result(*this);
    --(*this);
    return result;
}

double Rational::toFloat(void) const
{
    return (double)(_num * _sign) / _den;
}

void Rational::print(ostream& os) const
{
    ios_base::fmtflags s_flags(os.flags());
    size_t s_width = os.width();
    //char s_fill = os.fill();
    //size_t s_precision = os.precision();

    //if (_den == 1)
    //    os << setw(10) << _sign * _num;
    //else
    //    os << setw(10) << right << _sign * _num << '/' << setw(10) << left << _den;

    if (_den == 1)
        os << _sign * _num;
    else
        os << _sign * _num << '/' <<  _den;

    os.flags(s_flags);
    os.width(s_width);
    //os.fill(s_fill);
    //os.precision(s_precision);
}

ostream& operator<<(ostream& os, const Rational& rhs)
{
    rhs.print(os);
    return os;
}

inline bool is_sign(char c)
{
    return ((c == '-') || (c == '+'));
}

Rational Rational::get_number(istream& is)
{
    char c, sign = '+', state = 0;
    vector<char> int_digits, frac_digits;

    while (state != -1)
    {
        c = is.peek();

        switch (state)
        {
            case 0:
                if (is_sign(c))
                {
                    sign = c;
                    state = 1;
                    (void)is.get();
                }
                else if (isdigit(c))
                {
                    state = 1;
                }
                else if (c == '.')
                {
                    state = 2;
                    (void)is.get();
                }
                else
                {
                    state = -1;
                }
                break;

            case 1:
                if (isdigit(c))
                {
                    int_digits.insert(int_digits.begin(), c);
                    (void)is.get();
                }
                else if (c == '.')
                {
                    state = 2;
                    (void)is.get();
                }
                else
                {
                    state = -1;
                }
                break;

            case 2:
                if (isdigit(c))
                {
                    frac_digits.push_back(c);
                    (void)is.get();
                }
                else
                {
                    state = -1;
                }
                break;
        }
    }

    Rational r(0);

    if (!int_digits.size() && !frac_digits.size())
    {
        is.setstate(ios::failbit);
        return r;
    }

    for (size_t i = 0; i < int_digits.size(); i++)
        r += Rational((int64_t)((int_digits[i] - '0') * pow(10, i)));

    for (size_t i = 0; i < frac_digits.size(); i++)
        r += Rational((int64_t)(frac_digits[i] - '0'), (int64_t)pow(10, i+1));

    if (sign == '-')
        r._sign = -1;

    return r;
}

void Rational::read(istream& is)
{
    while (isspace(is.peek()))
        (void)is.get();

    Rational num(get_number(is));

    if (is.good() && (is.peek() == '/'))
    {
        (void)is.get();

        Rational den = get_number(is); 
        if (den._num == 0)
            throw RationalDivideByZeroException(num, den);

        num /= den;
    }

    *this = num;
}

istream& operator>>(istream& is, Rational& rhs)
{
    rhs.read(is);
    return is;
}

