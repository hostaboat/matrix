#ifndef _MATRIX_H
#define _MATRIX_H

#include "Number.H"
#include <iostream>
#include <cinttypes>
#include <vector>
using namespace std;

template<class T>
class Matrix
{
    public:
        Matrix(void);
        Matrix(uint32_t n);
        Matrix(uint32_t m, uint32_t n);
        Matrix(const vector< vector< Number<T> > >& A);
        Matrix(const vector< Number<T> >& v);
        Matrix(const Matrix<T>& A);

        ~Matrix(void);

        const Number<T>& operator()(uint32_t i, uint32_t j) const;
        Number<T>& operator()(uint32_t i, uint32_t j);

        Matrix<T>& operator=(const Matrix<T>& rhs);

        Matrix<T>& operator+=(const Matrix<T>& rhs);
        Matrix<T>& operator+=(const Number<T>& scalar);

        Matrix<T>& operator-=(const Matrix<T>& rhs);
        Matrix<T>& operator-=(const Number<T>& scalar);

        Matrix<T>& operator*=(const Matrix<T>& rhs);
        Matrix<T>& operator*=(const Number<T>& scalar);

        Matrix<T>& operator/=(const Number<T>& scalar);

        friend bool operator==(const Matrix<T>& lhs, const Matrix<T>& rhs)
        {
            if (&lhs == &rhs)
                return true;

            if ((lhs._m == rhs._m) && (lhs._n == rhs._n))
            {
                for (uint32_t i = 0; i < lhs._m; i++)
                {
                    for (uint32_t j = 0; j < lhs._n; j++)
                    {
                        if (lhs._A[i][j] != rhs._A[i][j])
                            return false;
                    }
                }

                return true;
            }

            return false;
        }

        bool isSquare(void) const;
        uint32_t rows(void) const;
        uint32_t cols(void) const;

        Matrix<T> cofactor_matrix(void) const;
        Number<T> cofactor(uint32_t a_i, uint32_t a_j) const;
        Number<T> determinant(void) const;
        Matrix<T> transpose(void) const;
        Matrix<T> adjoint(void) const;
        Matrix<T> inverse(void) const;

        Matrix<T> cramers_rule(
                const Matrix<T>& A, const Matrix<T>& s, uint32_t j) const;
        Matrix<T> cramers_rule(const Matrix<T>& s, uint32_t j) const;
        Matrix<T> cramers_rule(
                const Matrix<T>& A,
                const vector< Number<T> >& s, uint32_t j) const;
        Matrix<T> cramers_rule(const vector< Number<T> >& s, uint32_t j) const;

        Matrix<T> solve(const Matrix<T>& A, const Matrix<T>& s) const;
        Matrix<T> solve(const Matrix<T>& s) const;
        Matrix<T> solve(const Matrix<T>& A, const vector< Number<T> >& s) const;
        Matrix<T> solve(const vector< Number<T> >& s) const;

        void print(ostream& os) const;
        void read(istream& is);

    private:
        void _initialize(void);

        Number<T> _cofactor(
                uint32_t n, const Matrix<T>& A,
                uint32_t a_i, uint32_t a_j) const;
        Number<T> _determinant(uint32_t n, const Matrix<T>& A) const;
        Matrix<T> _transpose(const Matrix<T>& A) const;

        string _read_matrix(istream& is);

        uint32_t _m;
        uint32_t _n;
        vector< vector< Number<T> > > _A;
};

//template<class T>
//bool operator==(const Matrix<T>& lhs, const Matrix<T>& rhs);

template<class T>
bool operator!=(const Matrix<T>& lhs, const Matrix<T>& rhs);

template<class T>
const Matrix<T> operator+(const Matrix<T>& lhs, const Matrix<T>& rhs);

template<class T>
const Matrix<T> operator+(const Matrix<T>& lhs, const Number<T>& scalar);

template<class T>
const Matrix<T> operator-(const Matrix<T>& lhs, const Matrix<T>& rhs);

template<class T>
const Matrix<T> operator-(const Matrix<T>& lhs, const Number<T>& scalar);

template<class T>
const Matrix<T> operator*(const Matrix<T>& lhs, const Matrix<T>& rhs);

template<class T>
const Matrix<T> operator*(const Matrix<T>& lhs, const Number<T>& scalar);

template<class T>
const Matrix<T> operator/(const Matrix<T>& lhs, const Number<T>& scalar);

template<class T>
ostream& operator<<(ostream& os, const Matrix<T>& rhs);

template<class T>
istream& operator>>(istream& is, Matrix<T>& rhs);


// Exceptions ******************************************************************
template<class T>
class MatrixException
{
    public:
        MatrixException(void);
        ~MatrixException(void);
        virtual void message(void) const;
    private:
};

template<class T>
class MatrixInvalidAccessException : public MatrixException<T>
{
    public:
        MatrixInvalidAccessException(
                const Matrix<T>& A, uint32_t i, uint32_t j);
        ~MatrixInvalidAccessException(void);
        void message(void) const;
    private:
        Matrix<T> _A;
        uint32_t _i, _j;
};

template<class T>
class MatrixMultiplicationException : public MatrixException<T>
{
    public:
        MatrixMultiplicationException(const Matrix<T>& A, const Matrix<T>& B);
        ~MatrixMultiplicationException(void);
        void message(void) const;
    private:
        Matrix<T> _A, _B;
};

template<class T>
class MatrixAdditionException : public MatrixException<T>
{
    public:
        MatrixAdditionException(const Matrix<T>& A, const Matrix<T>& B);
        ~MatrixAdditionException(void);
        void message(void) const;
    private:
        Matrix<T> _A, _B;
};

template<class T>
class MatrixInvalidVectorException : public MatrixException<T>
{
    public:
        MatrixInvalidVectorException(
                const vector< vector< Number<T> > >& invalid_vector);
        MatrixInvalidVectorException(const vector< Number<T> >& invalid_vector);
        ~MatrixInvalidVectorException(void);
        void message(void) const;
    private:
        vector< vector< Number<T> > >* _invalid_vector1;
        vector< Number<T> >* _invalid_vector2;
};

template<class T>
class MatrixNotSquareException : public MatrixException<T>
{
    public:
        MatrixNotSquareException(const Matrix<T>& A);
        ~MatrixNotSquareException(void);
        void message(void) const;
    private:
        Matrix<T> _A;
};

template<class T>
class MatrixSolutionsException : public MatrixException<T>
{
    public:
        MatrixSolutionsException(const Matrix<T>& A, const Matrix<T>& s);
        ~MatrixSolutionsException(void);
        void message(void) const;
    private:
        Matrix<T> _A, _s;
};

template<class T>
class MatrixSingularException : public MatrixException<T>
{
    public:
        MatrixSingularException(const Matrix<T>& A);
        ~MatrixSingularException(void);
        void message(void) const;
    private:
        Matrix<T> _A;
};

enum mpe_reason
{
    MPE_EMPTY_MATRIX,
    MPE_EMPTY_ROW,
    MPE_ROW_SIZE_MISMATCH,
    MPE_INVALID_SYNTAX,
    MPE_EXPRESSION_EXCEPTION,
};

template<class T>
class MatrixParsingException : public MatrixException<T>
{
    public:
        MatrixParsingException(string str, size_t index, mpe_reason reason);
        MatrixParsingException(string str, size_t index);
        ~MatrixParsingException(void);
        void message(void) const;
    private:
        string _str;
        size_t _index;
        mpe_reason _reason;
};

#include "Matrix.C"

#endif
