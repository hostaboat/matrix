#include "Scientific.H"
#include "Exceptions.H"
#include <cinttypes>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

void Scientific::reduce(void)
{
    if (_num == 0)
    {
        _exp = 0;
        return;
    }

    while ((_num % 10) == 0)
    {
        _num /= 10;
        _exp++;
    }

    while ((_den % 10) == 0)
    {
        _den /= 10;
        _exp--;
    }
}

void equate(Scientific& f1, Scientific& f2)
{
    // This was causing problems at one point and I never figured out why
    // but it seems to be ok now, as it should be, I think.
    if ((f1._num == 0) || (f2._num == 0))
        return;

    Rational& r1 = f1;
    Rational& r2 = f2;
    int8_t e1 = f1._exp;
    int8_t e2 = f2._exp;
    int8_t mid_e = (e1 + e2) / 2;

    if (e1 < mid_e)
    {
        for (; e1 != mid_e; e1++)
            r1 /= 10;
    }
    else if (e1 > mid_e)
    {
        for (; e1 != mid_e; e1--)
            r1 *= 10;
    }

    if (e2 < mid_e)
    {
        for (; e2 != mid_e; e2++)
            r2 /= 10;
    }
    else if (e2 > mid_e)
    {
        for (; e2 != mid_e; e2--)
            r2 *= 10;
    }

    f1._exp = f2._exp = mid_e;
}

Scientific::Scientific(void)
    : Rational(0), _exp(0)
{
}

Scientific::Scientific(int64_t n)
    : Rational(n), _exp(0)
{
    reduce();
}

Scientific::Scientific(int64_t n, int8_t e)
    : Rational(n), _exp(e)
{
    reduce();
}

Scientific::Scientific(int64_t n, int64_t d, int8_t e)
    : Rational(n, d), _exp(e)
{
    reduce();
}

Scientific::Scientific(const Rational& r)
    : Rational(r), _exp(0)
{
    reduce();
}

Scientific::Scientific(const Rational& r, int8_t e)
    : Rational(r), _exp(e)
{
    reduce();
}

Scientific::Scientific(const Scientific& n)
    : Rational(n), _exp(n._exp)
{
}

Scientific::~Scientific(void)
{
}

Scientific& Scientific::operator=(const Scientific& rhs)
{
    if (this != &rhs)
    {
        Rational::operator=(rhs);
        _exp = rhs._exp;
    }

    return *this;
}

bool operator==(const Scientific& lhs, const Scientific& rhs)
{
    if (&lhs == &rhs)
        return true;

    if (((Rational)lhs == (Rational)rhs) && (lhs._exp == rhs._exp))
        return true;

    return false;
}

bool operator!=(const Scientific& lhs, const Scientific& rhs)
{
    return !(rhs == lhs);
}

bool operator<=(const Scientific& lhs, const Scientific& rhs)
{
    if (&lhs == &rhs)
        return true;

    if (lhs._exp == rhs._exp)
        return ((Rational)lhs <= (Rational)rhs);

    if ((((Rational)lhs <= 0) && ((Rational)rhs >= 0))
            || (((Rational)lhs >= 0) && ((Rational)rhs <= 0)))
    {
        return ((Rational)lhs <= (Rational)rhs);
    }

    if ((lhs._exp <= rhs._exp) && ((Rational)lhs <= (Rational)rhs))
        return true;

    if ((lhs._exp >= rhs._exp) && ((Rational)lhs > (Rational)rhs))
        return false;

    Scientific f1(lhs);
    Scientific f2(rhs);

    equate(f1, f2);

    return ((Rational)f1 <= (Rational)f2);
}

bool operator<(const Scientific& lhs, const Scientific& rhs)
{
    return !(rhs <= lhs);
}

bool operator>=(const Scientific& lhs, const Scientific& rhs)
{
    return (rhs <= lhs);
}

bool operator>(const Scientific& lhs, const Scientific& rhs)
{
    return (rhs < lhs);
}

Scientific& Scientific::operator+=(const Scientific& rhs)
{
    if ((_num == 0) || (rhs._num == 0))
    {
        Rational::operator+=(rhs);

        if (rhs._num != 0)
            _exp = rhs._exp;

        return *this;
    }

    Scientific f1(*this);
    Scientific f2(rhs);

    equate(f1, f2);

    *this = f1;
    Rational::operator+=(f2);

    reduce();

    return *this;
}

Scientific operator+(const Scientific& lhs, const Scientific& rhs)
{
    Scientific result(lhs);
    result += rhs;
    return result;
}

Scientific& Scientific::operator-=(const Scientific& rhs)
{
    if ((_num == 0) || (rhs._num == 0))
    {
        Rational::operator-=(rhs);

        if (rhs._num != 0)
            _exp = rhs._exp;

        return *this;
    }

    Scientific f1(*this);
    Scientific f2(rhs);

    equate(f1, f2);

    *this = f1;
    Rational::operator-=(f2);

    reduce();

    return *this;
}

Scientific operator-(const Scientific& lhs, const Scientific& rhs)
{
    Scientific result(lhs);
    result -= rhs;
    return result;
}

Scientific& Scientific::operator*=(const Scientific& rhs)
{
    Rational::operator*=(rhs);

    if (_num == 0)
    {
        _exp = 0;
    }
    else
    {
        _exp += rhs._exp;
        reduce();
    }
    
    return *this;
}

Scientific operator*(const Scientific& lhs, const Scientific& rhs)
{
    Scientific result(lhs);
    result *= rhs;
    return result;
}

Scientific& Scientific::operator/=(const Scientific& rhs)
{
    Rational::operator/=(rhs);

    if (_num == 0)
    {
        _exp = 0;
    }
    else
    {
        _exp -= rhs._exp;
        reduce();
    }

    return *this;
}

Scientific operator/(const Scientific& lhs, const Scientific& rhs)
{
    Scientific result(lhs);
    result /= rhs;
    return result;
}

Scientific& Scientific::operator^=(const Scientific& rhs)
{
    if (rhs._den != 1)
    {
        throw ScientificNotSupportedException(
                "Exponentiation is only supported with integers");
    }

    if (rhs._exp == 0)
    {
        Rational::operator^=(rhs);
        _exp *= rhs._sign * rhs._num;
    }
    else if (rhs._exp == 1)
    {
        Rational r(rhs._sign * rhs._num * 10, rhs._den);
        Rational::operator^=(r);
        _exp *= rhs._sign * rhs._num * 10;
    }
    else
    {
        throw ScientificNotSupportedException(
                "Exponentiation is only supported with integers "
                "with exponents of 0 or 1");
    }

    reduce();

    return *this;
}

Scientific operator^(const Scientific& lhs, const Scientific& rhs)
{
    Scientific result(lhs);
    result ^= rhs;
    return result;
}


Scientific& Scientific::operator++(void)
{
    *this += 1;
    return *this;
}

Scientific Scientific::operator++(int)
{
    Scientific ret(*this);
    ++(*this);
    return ret;
}

Scientific& Scientific::operator--(void)
{
    *this -= 1;
    return *this;
}

Scientific Scientific::operator--(int)
{
    Scientific ret(*this);
    --(*this);
    return ret;
}

double Scientific::toFloat(void) const
{
    return (Rational::toFloat() * pow(10, _exp));
}

Rational Scientific::toRational(void) const
{
    int64_t num = _num;
    int64_t den = _den;
    int8_t exp = _exp;

    for (; exp > 0; exp--)
        num *= 10;

    for (; exp < 0; exp++)
        den *= 10;

    return Rational(_sign * num, den);
}

void Scientific::print(ostream& os) const
{
    int64_t num = _num;
    int64_t den = _den;
    int8_t exp = _exp;

    ios_base::fmtflags s_flags(os.flags());
    size_t s_width = os.width();
    size_t s_precision = os.precision();
    char s_fill = os.fill();

    if (num != 0)
    {
        while ((num / den) >= 10)
        {
            den *= 10;
            exp++;
        }

        while ((num / den) < 1)
        {
            num *= 10;
            exp--;
        }

        while ((exp % 3) != 0)
        {
            num *= 10;
            exp--;
        }

        num *= _sign;
    }

    //os << fixed << right << setprecision(5) << (double)num / den
    os << fixed << right << setprecision(9) << (double)num / den
        << "e" << ((exp < 0) ? '-' : '+') << setw(2) << setfill('0') << (int)abs(exp);
    //os << setw(10) << fixed << right << setprecision(5) << (double)num / den
    //    << "e" << ((exp < 0) ? '-' : '+') << setw(2) << setfill('0') << (int)abs(exp);

    os.flags(s_flags);
    os.width(s_width);
    os.precision(s_precision);
    os.fill(s_fill);
}

ostream& operator<<(ostream& os, const Scientific& rhs)
{
    rhs.print(os);
    return os;
}

inline bool is_sign(char c)
{
    return ((c == '-') || (c == '+'));
}

inline bool is_e_notation(char c)
{
    return ((c == 'e') || (c == 'E'));
}

int8_t Scientific::get_exponent(istream& is)
{
    char c, sign = '+', state = 0;
    vector<char> exp;

    while (state != -1)
    {
        c = is.peek();

        switch (state)
        {
            case 0:
                if (is_e_notation(c))
                {
                    state = 1;
                    (void)is.get();
                }
                else
                {
                    return 0;
                }
                break;

            case 1:
                if (is_sign(c))
                {
                    state = 2;
                    sign = c;
                    (void)is.get();
                }
                else if (isdigit(c))
                {
                    state = 2;
                }
                else
                {
                    state = -1;
                }
                break;

            case 2:
                if (isdigit(c))
                {
                    exp.insert(exp.begin(), c);
                    (void)is.get();
                }
                else
                {
                    state = -1;
                }
                break;
        }
    }

    if (!exp.size())
    {
        is.setstate(ios::failbit);
        return 0;
    }

    int64_t exponent = 0;

    for (size_t i = 0; i < exp.size(); i++)
        exponent += ((exp[i] - '0') * pow(10, i));

    if (sign == '-')
        exponent *= -1;

    if ((exponent > INT8_MAX) || (exponent < INT8_MIN))
    {
        is.setstate(ios::failbit);
        return 0;
    }

    return (int8_t)exponent;
}

void Scientific::read(istream& is)
{
    while (isspace(is.peek()))
        (void)is.get();

    Scientific num(Rational::get_number(is));

    if (is.good())
        num._exp = get_exponent(is);

    if (is.good() && (is.peek() == '/'))
    {
        (void)is.get();

        Scientific den(Rational::get_number(is));

        if (den._num == 0)
            throw RationalDivideByZeroException(num, den);

        if (is.good())
            den._exp = get_exponent(is);

        num /= den;
    }

    *this = num;

    reduce();
}

istream& operator>>(istream& is, Scientific& rhs)
{
    rhs.read(is);
    return is;
}

