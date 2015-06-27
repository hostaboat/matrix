#ifndef _NUMBER_H
#define _NUMBER_H

#include <cinttypes>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <stack>
#include <type_traits>
#include <cmath>
using namespace std;

template<class T>
class Number
{
    public:
        //******************************************************************************
        //* Constructors ***************************************************************
        //******************************************************************************
        Number(void)
            : _number()
        {
            // Nothing
        }

        Number(const T& t)
            : _number(t)
        {
            // Nothing
        }

        Number(const Number<T>& n)
            : _number(n._number)
        {
            // Nothing
        }

        static Number<T> parse_expression(const string& str);

        //******************************************************************************
        //* Destructor *****************************************************************
        //******************************************************************************
        ~Number(void)
        {
            // Nothing
        }

        //******************************************************************************
        //* operator= ******************************************************************
        //******************************************************************************
        Number<T>& operator=(const Number<T>& rhs)
        {
            if (this != &rhs)
                _number = rhs._number;

            return *this;
        }

        Number<T>& operator=(const T& rhs)
        {
            _number = rhs._number;
            return *this;
        }

        //******************************************************************************
        //* operator== *****************************************************************
        //******************************************************************************
        friend bool operator==(const Number<T>& lhs, const Number<T>& rhs)
        {
            return (lhs._number == rhs._number);
        }

        friend bool operator==(const Number<T>& lhs, const T& rhs)
        {
            return (lhs._number == rhs);
        }

        friend bool operator==(const T& lhs, const Number<T>& rhs)
        {
            return (lhs == rhs._number);
        }

        //******************************************************************************
        //* operator!= *****************************************************************
        //******************************************************************************
        friend bool operator!=(const Number<T>& lhs, const Number<T>& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator!=(const Number<T>& lhs, const T& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator!=(const T& lhs, const Number<T>& rhs)
        {
            return !(lhs == rhs);
        }

        //******************************************************************************
        //* operator<= *****************************************************************
        //******************************************************************************
        friend bool operator<=(const Number<T>& lhs, const Number<T>& rhs)
        {
            return (lhs._number <= rhs._number);
        }

        friend bool operator<=(const Number<T>& lhs, const T& rhs)
        {
            return (lhs._number <= rhs);
        }

        friend bool operator<=(const T& lhs, const Number<T>& rhs)
        {
            return (lhs <= rhs._number);
        }

        //******************************************************************************
        //* operator< ******************************************************************
        //******************************************************************************
        friend bool operator<(const Number<T>& lhs, const Number<T>& rhs)
        {
            return !(rhs <= lhs);
        }

        friend bool operator<(const Number<T>& lhs, const T& rhs)
        {
            return !(rhs <= lhs);
        }

        friend bool operator<(const T& lhs, const Number<T>& rhs)
        {
            return !(rhs <= lhs);
        }

        //******************************************************************************
        //* operator>= *****************************************************************
        //******************************************************************************
        friend bool operator>=(const Number<T>& lhs, const Number<T>& rhs)
        {
            return (lhs._number >= rhs._number);
        }

        friend bool operator>=(const Number<T>& lhs, const T& rhs)
        {
            return (lhs._number >= rhs);
        }

        friend bool operator>=(const T& lhs, const Number<T>& rhs)
        {
            return (lhs >= rhs._number);
        }

        //******************************************************************************
        //* operator> ******************************************************************
        //******************************************************************************
        friend bool operator>(const Number<T>& lhs, const Number<T>& rhs)
        {
            return !(rhs >= lhs);
        }

        friend bool operator>(const Number<T>& lhs, const T& rhs)
        {
            return !(rhs >= lhs);
        }

        friend bool operator>(const T& lhs, const Number<T>& rhs)
        {
            return !(rhs >= lhs);
        }

        //******************************************************************************
        //* operator+= *****************************************************************
        //******************************************************************************
        Number<T>& operator+=(const Number<T>& rhs)
        {
            _number += rhs._number;
            return *this;
        }

        Number<T>& operator+=(const T& rhs)
        {
            _number += rhs;
            return *this;
        }

        //******************************************************************************
        //* operator+ ******************************************************************
        //******************************************************************************
        friend Number<T> operator+(const Number<T>& lhs, const Number<T>& rhs)
        {
            Number<T> result(lhs);
            result += rhs;
            return result;
        }

        friend Number<T> operator+(const Number<T>& lhs, const T& rhs)
        {
            Number<T> result(lhs);
            result += rhs;
            return result;
        }

        friend Number<T> operator+(const T& lhs, const Number<T>& rhs)
        {
            Number<T> result(lhs);
            result += rhs;
            return result;
        }

        //******************************************************************************
        //* operator-= *****************************************************************
        //******************************************************************************
        Number<T>& operator-=(const Number<T>& rhs)
        {
            _number -= rhs._number;
            return *this;
        }

        Number<T>& operator-=(const T& rhs)
        {
            _number -= rhs;
            return *this;
        }

        //******************************************************************************
        //* operator- ******************************************************************
        //******************************************************************************
        friend Number<T> operator-(const Number<T>& lhs, const Number<T>& rhs)
        {
            Number<T> result(lhs);
            result -= rhs;
            return result;
        }

        friend Number<T> operator-(const Number<T>& lhs, const T& rhs)
        {
            Number<T> result(lhs);
            result -= rhs;
            return result;
        }

        friend Number<T> operator-(const T& lhs, const Number<T>& rhs)
        {
            Number<T> result(lhs);
            result -= rhs;
            return result;
        }

        //******************************************************************************
        //* operator*= *****************************************************************
        //******************************************************************************
        Number<T>& operator*=(const Number<T>& rhs)
        {
            _number *= rhs._number;
            return *this;
        }

        Number<T>& operator*=(const T& rhs)
        {
            _number *= rhs;
            return *this;
        }

        //******************************************************************************
        //* operator* ******************************************************************
        //******************************************************************************
        friend Number<T> operator*(const Number<T>& lhs, const Number<T>& rhs)
        {
            Number<T> result(lhs);
            result *= rhs;
            return result;
        }

        friend Number<T> operator*(const Number<T>& lhs, const T& rhs)
        {
            Number<T> result(lhs);
            result *= rhs;
            return result;
        }

        friend Number<T> operator*(const T& lhs, const Number<T>& rhs)
        {
            Number<T> result(lhs);
            result *= rhs;
            return result;
        }

        //******************************************************************************
        //* operator/= *****************************************************************
        //******************************************************************************
        Number<T>& operator/=(const Number<T>& rhs)
        {
            _number /= rhs._number;
            return *this;
        }

        Number<T>& operator/=(const T& rhs)
        {
            _number /= rhs;
            return *this;
        }

        //******************************************************************************
        //* operator/ ******************************************************************
        //******************************************************************************
        friend Number<T> operator/(const Number<T>& lhs, const Number<T>& rhs)
        {
            Number<T> result(lhs);
            result /= rhs;
            return result;
        }

        friend Number<T> operator/(const Number<T>& lhs, const T& rhs)
        {
            Number<T> result(lhs);
            result /= rhs;
            return result;
        }

        friend Number<T> operator/(const T& lhs, const Number<T>& rhs)
        {
            Number<T> result(lhs);
            result /= rhs;
            return result;
        }

        //******************************************************************************
        //* operator++ *****************************************************************
        //******************************************************************************
        Number<T>& operator++(void)
        {
            _number += 1;
            return *this;
        }

        Number<T> operator++(int)
        {
            Number<T> result(*this);
            ++(*this);
            return result;
        }

        //******************************************************************************
        //* operator-- *****************************************************************
        //******************************************************************************
        Number<T>& operator--(void)
        {
            _number -= 1;
            return *this;
        }

        Number<T> operator--(int)
        {
            Number<T> result(*this);
            --(*this);
            return result;
        }

        //******************************************************************************
        //* operator-> *****************************************************************
        //******************************************************************************
        const T* operator->(void) const
        {
            return &_number;
        }

        //******************************************************************************
        //* operator<< *****************************************************************
        //******************************************************************************
        friend ostream& operator<<(ostream& os, const Number<T>& rhs)
        {
            rhs.print(os);
            return os;
        }

        void print(ostream& os) const
        {
            os << _number;
        }

        //******************************************************************************
        //* operator>> *****************************************************************
        //******************************************************************************
        friend istream& operator>>(istream& is, Number<T>& rhs)
        {
            rhs.read(is);
            return is;
        }

        void read(istream& is)
        {
            is >> _number;
        }

    private:
        static T _get_number(const string& str, size_t& index, char sign);
        static T _parse_expression(const string& str, size_t& index);

        T _number;
};

template<class T>
class NumberParsingException
{
    public:
        NumberParsingException(string str, size_t index)
        {
            _str = str;
            _index = index;
        }

        ~NumberParsingException(void)
        {
            // Nothing
        }

        string str(void)
        {
            return _str;
        }

        size_t index(void)
        {
            return _index;
        }

        void message(void)
        {
            cout << "Error with expression:" << endl;
            cout << "  " << _str << endl;
            cout << "  " << setw(_index+1) << "^" << endl;
        }

    private:
        string _str;
        size_t _index;
};

static map<char,int> operators = {
    { '+', 0 },
    { '-', 0 },
    { '*', 1 },
    { '/', 1 },
    { '^', 2 },
    { 'E', 2 },
    { 'e', 2 },
};

static set<char> special_characters = {
    '^',
    'E',
    'e',
    '+',
    '-',
    '*',
    '/',
    '(',
    ')',
};

static inline bool is_sign(char c)
{
    return ((c == '+') || (c == '-'));
}

static inline bool is_operator(char c)
{
    return (operators.find(c) != operators.end());
}

static inline bool is_special(char c)
{
    return (special_characters.find(c) != special_characters.end());
}

static inline int precedence(char op)
{
    return operators[op];
}

template<class T>
typename enable_if<is_arithmetic<T>::value, void>::type
npow(T& lt, T& rt)
{
    lt = pow(lt, rt);
}

template<class T>
typename enable_if<!is_arithmetic<T>::value, void>::type
npow(T& lt, T& rt)
{
    lt ^= rt;
}

template<class T>
typename enable_if<is_arithmetic<T>::value, void>::type
epow(T& lt, T& rt)
{
    lt *= pow(10, rt);
}

template<class T>
typename enable_if<!is_arithmetic<T>::value, void>::type
epow(T& lt, T& rt)
{
    lt *= 10^rt;
}

template<class T>
bool operate(T& lt, char op, T& rt)
{
    if (op == '+')
    {
        lt += rt;
    }
    else if (op == '-')
    {
        lt -= rt;
    }
    else if (op == '*')
    {
        lt *= rt;
    }
    else if (op == '/')
    {
        if (rt == 0)
            return false;

        lt /= rt;
    }
    else if (op == '^')
    {
        npow(lt, rt);
    }
    else if ((op == 'e') || (op == 'E'))
    {
        epow(lt, rt);
    }

    return true;
}

template<class T>
T Number<T>::_get_number(const string& str, size_t& index, char sign)
{
    T t;
    string rstr;
    size_t rindex = index;

    char c = str[rindex];
    while ((rindex < str.size()) && !is_special(c) && !isspace(c))
    {
        rstr += c;
        c = str[++rindex];
    }

    if (rstr.size() == 0)
        throw NumberParsingException<T>(str, index);

    if (sign)
        rstr = sign + rstr;

    istringstream iss(rstr);
    iss >> t;

    if (!iss.eof())
    {
        if (iss.fail())
            throw NumberParsingException<T>(str, index);

        index = rindex;
        (void)iss.get();
        while (iss.good())
        {
            (void)iss.get();
            index--;
        }

        throw NumberParsingException<T>(str, index);
    }

    index = rindex;

    return t;
}

template<class T>
T Number<T>::_parse_expression(const string& str, size_t& index)
{
    const int max_state = 5;
    stack<T> nstack;
    stack<char> ostack;
    char sign = 0;
    int state = 0;
    size_t next_op = 0;

    while ((index < str.size()) && (state <= max_state))
    {
        char c = str[index];

        switch (state)
        {
            case 0:  // Initialization
                if (isspace(c))
                {
                    index++;
                    break;
                }

                state = 1;
                next_op = 0;
                sign = 0;

                break;

            case 1:  // Sign state
                if (is_sign(c))
                {
                    sign = c;
                    index++;
                }

                state = 2;

                break;

            case 2:  // Paren or number state
                if (c == '(')
                {
                    nstack.push(_parse_expression(str, ++index));

                    // If there isn't a matching right paren, fail
                    if ((index == str.size()) || (str[index] != ')'))
                        throw NumberParsingException<T>(str, index);

                    if (sign == '-')
                        nstack.top() *= -1;

                    index++;
                }
                else
                {
                    nstack.push(_get_number(str, index, sign));
                }

                state = 3;

                break;

            case 3:
                if (c == ')')
                {
                    state = max_state + 1;
                    break;
                }
                else if (isspace(c))
                {
                    state = 4;
                }
                else
                {
                    if (!is_operator(c))
                        throw NumberParsingException<T>(str, index);

                    state = 5;
                    next_op = index;
                }

                index++;

                break;

            case 4:
                if (next_op)
                {
                    if (!isspace(c))
                        throw NumberParsingException<T>(str, index);

                    state = 5;
                }
                else if (!isspace(c))
                {
                    if (c == ')')
                    {
                        state = max_state + 1;
                        break;
                    }

                    if (!is_operator(c))
                        throw NumberParsingException<T>(str, index);

                    if (!is_sign(c))
                        state = 5;

                    next_op = index;
                }

                index++;

                break;

            case 5:
                state = 0;

                if (ostack.empty())
                {
                    ostack.push(str[next_op]);
                }
                else
                {
                    char op = ostack.top();
                    T& rt = nstack.top();

                    while (precedence(op) >= precedence(str[next_op]))
                    {
                        ostack.pop();
                        nstack.pop();

                        T& lt = nstack.top();

                        if (!operate(lt, op, rt))
                            throw NumberParsingException<T>(str, index);

                        rt = lt;

                        if (ostack.empty())
                            break;

                        op = ostack.top();
                    }

                    ostack.push(str[next_op]);
                }

                break;
        }
    }

    if (next_op)
        throw NumberParsingException<T>(str, next_op);

    while (!ostack.empty())
    {
        char op = ostack.top();
        T& rt = nstack.top();

        ostack.pop();
        nstack.pop();

        if (nstack.empty())
            throw NumberParsingException<T>(str, str.size()-1);

        T& lt = nstack.top();

        if (!operate(lt, op, rt))
            throw NumberParsingException<T>(str, str.size()-1);
    }

    if (nstack.empty())
        throw NumberParsingException<T>(str, 0);

    return nstack.top();
}

template<class T>
Number<T> Number<T>::parse_expression(const string& str)
{
    size_t index = 0;

    Number<T> n(_parse_expression(str, index));

    return n;
}

#endif
