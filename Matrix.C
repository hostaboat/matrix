#ifndef _MATRIX_C
#define _MATRIX_C

#include "Matrix.H"
#include "Number.H"
#include "Exceptions.H"
#include <iostream>
#include <iomanip>
#include <cinttypes>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <map>
using namespace std;

template<class T>
void Matrix<T>::_initialize(void)
{
    for (uint32_t i = 0; i < _m; i++)
        _A[i].resize(_n);
}

template<class T>
Matrix<T>::Matrix(void)
    : _m(0), _n(0), _A(0)
{
}

template<class T>
Matrix<T>::Matrix(uint32_t n)
    : _m(n), _n(n), _A(n)
{
    _initialize();
}

template<class T>
Matrix<T>::Matrix(uint32_t m, uint32_t n)
    : _m(m), _n(n), _A(m)
{
    _initialize();
}

template<class T>
Matrix<T>::Matrix(const vector< vector< Number<T> > >& A)
{
    uint32_t m = A.size();
    uint32_t n = 0;

    for (uint32_t i = 0; i < m; i++)
    {
        if (n == 0)
            n = A[i].size();

        if ((n == 0) || (A[i].size() != n))
            throw MatrixInvalidVectorException<T>(A);
    }

    _m = m;
    _n = n;
    _A = A;
}

template<class T>
Matrix<T>::Matrix(const vector< Number<T> >& v)
    : _m(v.size()), _n(1), _A(v.size())
{
    if (v.size() == 0)
        throw MatrixInvalidVectorException<T>(v);

    _initialize();

    for (uint32_t i = 0; i < _m; i++)
        _A[i][0] = v[i];
}

template<class T>
Matrix<T>::Matrix(const Matrix& A)
    : _m(A._m), _n(A._n), _A(A._A)
{
}

template<class T>
Matrix<T>::~Matrix(void)
{
}

template<class T>
Number<T>& Matrix<T>::operator()(uint32_t i, uint32_t j)
{
    if ((i > _m) || (j > _n) || (i == 0) || (j == 0))
        throw MatrixInvalidAccessException<T>(*this, i, j);

    return _A[i-1][j-1];
}

template<class T>
const Number<T>& Matrix<T>::operator()(uint32_t i, uint32_t j) const
{
    if ((i > _m) || (j > _n) || (i == 0) || (j == 0))
        throw MatrixInvalidAccessException<T>(*this, i, j);

    return _A[i-1][j-1];
}

template<class T>
Matrix<T>& Matrix<T>::operator=(const Matrix& rhs)
{
    if (this != &rhs)
    {
        _m = rhs._m;
        _n = rhs._n;
        _A = rhs._A;
    }

    return *this;
}

// Defined in class
//template<class T>
//bool operator==(const Matrix<T>& lhs, const Matrix<T>& rhs) {}

template<class T>
bool operator!=(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
    return !(lhs == rhs);
}

template<class T>
Matrix<T>& Matrix<T>::operator+=(const Matrix<T>& rhs)
{
    if ((_m != rhs._m) || (_n != rhs._n))
        throw MatrixAdditionException<T>(*this, rhs);

    for (uint32_t i = 0; i < _m; i++)
    {
        for (uint32_t j = 0; j < _n; j++)
            _A[i][j] += rhs._A[i][j];
    }

    return *this;
}

template<class T>
Matrix<T>& Matrix<T>::operator+=(const Number<T>& scalar)
{
    for (uint32_t i = 0; i < _m; i++)
    {
        for (uint32_t j = 0; j < _n; j++)
            _A[i][j] += scalar;
    }

    return *this;
}

template<class T>
const Matrix<T> operator+(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
    Matrix<T> result(lhs);
    result += rhs;
    return result;
}

template<class T>
const Matrix<T> operator+(const Matrix<T>& lhs, const Number<T>& scalar)
{
    Matrix<T> result(lhs);
    result += scalar;
    return result;
}

template<class T>
Matrix<T>& Matrix<T>::operator-=(const Matrix<T>& rhs)
{
    if ((_m != rhs._m) || (_n != rhs._n))
        throw MatrixAdditionException<T>(*this, rhs);

    for (uint32_t i = 0; i < _m; i++)
    {
        for (uint32_t j = 0; j < _n; j++)
            _A[i][j] -= rhs._A[i][j];
    }

    return *this;
}

template<class T>
Matrix<T>& Matrix<T>::operator-=(const Number<T>& scalar)
{
    for (uint32_t i = 0; i < _m; i++)
    {
        for (uint32_t j = 0; j < _n; j++)
            _A[i][j] -= scalar;
    }

    return *this;
}

template<class T>
const Matrix<T> operator-(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
    Matrix<T> result(lhs);
    result -= rhs;
    return result;
}

template<class T>
const Matrix<T> operator-(const Matrix<T>& lhs, const Number<T>& scalar)
{
    Matrix<T> result(lhs);
    result -= scalar;
    return result;
}

template<class T>
Matrix<T>& Matrix<T>::operator*=(const Matrix<T>& rhs)
{
    if (_n != rhs._m)
        throw MatrixMultiplicationException<T>(*this, rhs);

    // m x n * n x p  ->  m x p
    uint32_t m = _m, n = _n, p = rhs._n;  // For clarity

    Matrix<T> result(m, p);

    for (uint32_t i = 0; i < m; i++)
    {
        for (uint32_t j = 0; j < p; j++)
        {
            for (uint32_t k = 0; k < n; k++)
                result._A[i][j] += _A[i][k] * rhs._A[k][j];
        }
    }

    *this = result;

    return *this;
}

template<class T>
Matrix<T>& Matrix<T>::operator*=(const Number<T>& scalar)
{
    for (uint32_t i = 0; i < _m; i++)
    {
        for (uint32_t j = 0; j < _n; j++)
            _A[i][j] *= scalar;
    }

    return *this;
}

template<class T>
const Matrix<T> operator*(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
    Matrix<T> result(lhs);
    result *= rhs;
    return result;
}

template<class T>
const Matrix<T> operator*(const Matrix<T>& lhs, const Number<T>& scalar)
{
    Matrix<T> result(lhs);
    result *= scalar;
    return result;
}

template<class T>
Matrix<T>& Matrix<T>::operator/=(const Number<T>& scalar)
{
    for (uint32_t i = 0; i < _m; i++)
    {
        for (uint32_t j = 0; j < _n; j++)
            _A[i][j] /= scalar;
    }

    return *this;
}

template<class T>
const Matrix<T> operator/(const Matrix<T>& lhs, const Number<T>& scalar)
{
    Number<T> result(lhs);
    result /= scalar;
    return result;
}

template<class T>
bool Matrix<T>::isSquare(void) const
{
    return (_m == _n);
}

template<class T>
uint32_t Matrix<T>::rows(void) const
{
    return _m;
}

template<class T>
uint32_t Matrix<T>::cols(void) const
{
    return _n;
}

template<class T>
Number<T> Matrix<T>::_cofactor(
        uint32_t n, const Matrix<T>& A,
        uint32_t a_i, uint32_t a_j) const
{
    if (n == 1)
        return A(1,1);

    uint32_t i, j;
    uint32_t m_i, m_j;
    Matrix<T> M(n-1);

    for (i = 1, m_i = 1; i <= n; i++)
    {
        if (i == a_i)
            continue;

        for (j = 1, m_j = 1; j <= n; j++)
        {
            if (j == a_j)
                continue;

            M(m_i,m_j) = A(i,j);

            m_j++;
        }

        m_i++;
    }

    int minor_multiplier = ((a_i + a_j) & 1) ? -1 : 1;

    return (minor_multiplier * _determinant(n-1, M));
}

template<class T>
Number<T> Matrix<T>::cofactor(uint32_t a_i, uint32_t a_j) const
{
    if (!isSquare())
        throw MatrixNotSquareException<T>(*this);

    return _cofactor(_n, *this, a_i, a_j);
}

template<class T>
Matrix<T> Matrix<T>::cofactor_matrix(void) const
{
    if (!isSquare())
        throw MatrixNotSquareException<T>(*this);

    Matrix<T> C(_n);

    for (uint32_t i = 1; i <= _n; i++)
    {
        for (uint32_t j = 1; j <= _n; j++)
            C(i,j) = _cofactor(_n, *this, i, j);
    }

    return C;
}

#if 0
#include <thread>
#include <future>
template<class T>
Number<T> Matrix<T>::_determinant(uint32_t n, const Matrix<T>& A) const
{
    Number<T> det(0);

    if (n == 1)
        return A(1,1);

    if ((_n > 6) && ((_n - n) < 4))
    {
        vector< future< Number<T> > > threads;
        for (uint32_t i = 1, j = 1; j <= n; j++)
            threads.push_back(async(&Matrix<T>::_cofactor, *this, n, A, i, j));
            //k.push_back(async(launch::async, &Matrix<T>::_cofactor, *this, n, A, i, j));

        for (uint32_t i = 1, j = 1; j <= n; j++)
            det += A(i,j) * threads[j-1].get();
    }
    else
    {
        // Expand via the the first row: i == 1
        for (uint32_t i = 1, j = 1; j <= n; j++)
            det += A(i,j) * _cofactor(n, A, i, j);
        //cout << "NON thread: " << det << endl;
    }

    return det;
}
#endif

template<class T>
Number<T> Matrix<T>::_determinant(uint32_t n, const Matrix<T>& A) const
{
    Number<T> det(0);

    if (n == 1)
        return A(1,1);

    // Expand via the the first row: i == 1
    for (uint32_t i = 1, j = 1; j <= n; j++)
        det += A(i,j) * _cofactor(n, A, i, j);

    return det;
}

template<class T>
Number<T> Matrix<T>::determinant(void) const
{
    if (!isSquare())
        throw MatrixNotSquareException<T>(*this);

    return _determinant(_n, *this);
}

template<class T>
Matrix<T> Matrix<T>::_transpose(const Matrix<T>& A) const
{
    Matrix<T> trans(A._n, A._m);

    for (uint32_t i = 0; i < trans._m; i++)
    {
        for (uint32_t j = 0; j < trans._n; j++)
            trans._A[i][j] = A._A[j][i];
    }

    return trans;
}

template<class T>
Matrix<T> Matrix<T>::transpose(void) const
{
    return _transpose(*this);
}

template<class T>
Matrix<T> Matrix<T>::adjoint(void) const
{
    if (!isSquare())
        throw MatrixNotSquareException<T>(*this);

    return _transpose(cofactor_matrix());
}

#if 0
template<class T>
Matrix<T> Matrix<T>::inverse(void) const
{
    if (!isSquare())
        throw MatrixNotSquareException<T>(*this);

    Matrix<T> inv(_n);
    Matrix<T> c_matrix(cofactor_matrix());

    Number<T> det(0);

    for (uint32_t j = 1; j <= _n; j++)
        det += _A[0][j-1] * c_matrix(1,j);

    if (det == 0)
        throw MatrixSingularException<T>(*this);

    Matrix<T> adj(_transpose(c_matrix));

    for (uint32_t i = 1; i <= _m; i++)
    {
        for (uint32_t j = 1; j <= _n; j++)
            inv(i,j) = (1 / det) * adj(i,j);
    }

    return inv;
}
#endif

template<class T>
Matrix<T> Matrix<T>::inverse(void) const
{
    if (!isSquare())
        throw MatrixNotSquareException<T>(*this);

    Matrix<T> inv(_n);
    Number<T> det = determinant();
    Matrix<T> adj = adjoint();

    if (det == 0)
        throw MatrixSingularException<T>(*this);

    for (uint32_t i = 1; i <= _m; i++)
    {
        for (uint32_t j = 1; j <= _n; j++)
            inv(i,j) = (1 / det) * adj(i,j);
    }

    return inv;
}

template<class T>
Matrix<T> Matrix<T>::cramers_rule(
        const Matrix<T>& A, const Matrix<T>& s, uint32_t j) const
{
    if (!A.isSquare())
        throw MatrixNotSquareException<T>(A);

    if ((A._n != s._m) || (s._n != 1))
        throw MatrixSolutionsException<T>(A, s);

    if (j > A._n)
        throw MatrixInvalidAccessException<T>(A, j, j);

    if (j > s._m)
        throw MatrixInvalidAccessException<T>(s, j, j);

    Matrix<T> CR(A);

    for (uint32_t i = 1; i <= _m; i++)
        CR(i,j) = s(i,1);

    Number<T> Aj_det(CR.determinant());
    Number<T> A_det(determinant());
    Number<T> x_i(Aj_det / A_det);

    cout << "|A" << j << "| / |A| = (" << Aj_det << ") / (" << A_det
        << ") = " << x_i << endl << endl;

    return CR;
}

template<class T>
Matrix<T> Matrix<T>::cramers_rule(const Matrix<T>& s, uint32_t j) const
{
    return cramers_rule(*this, s, j);
}

template<class T>
Matrix<T> Matrix<T>::cramers_rule(
        const Matrix<T>& A, const vector< Number<T> >& s, uint32_t j) const
{
    return cramers_rule(A, Matrix<T>(s), j);
}

template<class T>
Matrix<T> Matrix<T>::cramers_rule(
        const vector< Number<T> >& s, uint32_t j) const
{
    return cramers_rule(*this, s, j);
}

template<class T>
Matrix<T> Matrix<T>::solve(const Matrix<T>& A, const Matrix<T>& s) const
{
    if (!A.isSquare())
        throw MatrixNotSquareException<T>(A);

    if ((A._n != s._m) || (s._n != 1))
        throw MatrixSolutionsException<T>(A, s);

    return A.inverse() * s;
}

template<class T>
Matrix<T> Matrix<T>::solve(const Matrix<T>& s) const
{
    return solve(*this, s);
}

template<class T>
Matrix<T> Matrix<T>::solve(
        const Matrix<T>& A, const vector< Number<T> >& s) const
{
    return solve(A, Matrix<T>(s));
}

template<class T>
Matrix<T> Matrix<T>::solve(const vector< Number<T> >& s) const
{
    return solve(*this, s);
}

template<class T>
void Matrix<T>::print(ostream& os) const
{
    vector< vector<string> > rstrs(_m);
    vector<char> col_align_char(_n, 0);
    vector<size_t> col_right_width(_n, 0), col_left_width(_n, 0), col_width(_n, 0);
    bool no_align_char = true;

    for (uint32_t i = 0; i < _m; i++)
    {
        for (uint32_t j = 0; j < _n; j++)
        {
            ostringstream oss;
            oss.precision(5);
            oss.setf(ios_base::fixed|ios_base::right);

            oss << _A[i][j];

            string str(oss.str());

            size_t pos;
            if (!col_align_char[j])
            {
                if ((pos = str.find('.')) != string::npos)
                    col_align_char[j] = '.';
                else if ((pos = str.find('/')) != string::npos)
                    col_align_char[j] = '/';
            }
            else
            {
                pos = str.find(col_align_char[j]);
            }

            if (pos != string::npos)
            {
                if (pos > col_left_width[j])
                    col_left_width[j] = pos;

                if (((str.size() - 1) - pos) > col_right_width[j])
                    col_right_width[j] = (str.size() - 1) - pos;

                no_align_char = false;
            }
            else
            {
                if (str.size() > col_left_width[j])
                    col_left_width[j] = str.size();
            }

            rstrs[i].push_back(str);
        }
    }

    ios_base::fmtflags s_flags(os.flags());
    size_t s_width = os.width();
    char s_fill = os.fill();
    size_t s_precision = os.precision();

    for (uint32_t i = 0; i < rstrs.size(); i++)
    {
        os << "[ ";
        for (uint32_t j = 0; j < rstrs[i].size(); j++)
        {
            string element = rstrs[i][j];
            size_t pos = element.find(col_align_char[j]);
            if (pos != string::npos)
            {
                string a(element, 0, pos);
                char align = element[pos];
                string b(element, pos+1);

                os << setw(col_left_width[j]) << right << a
                    << align << setw(col_right_width[j]) << left << b;
            }
            else if (!no_align_char)
            {
                os << setw(col_left_width[j]) << right << rstrs[i][j]
                    << setw(col_right_width[j]+1) << left << "";
            }
            else
            {
                os << setw(col_left_width[j]) << right << rstrs[i][j];
            }

            if ((rstrs[i].size() > 0) && (j < (rstrs[i].size() - 1)))
                os << "  ";
        }
        os << " ]" << endl;
    }

    os.flags(s_flags);
    os.width(s_width);
    os.fill(s_fill);
    os.precision(s_precision);
}

template<class T>
ostream& operator<<(ostream& os, const Matrix<T>& rhs)
{
    rhs.print(os);
    return os;
}

template<class T>
string Matrix<T>::_read_matrix(istream& is)
{
    int state = 0, save_state = 0;
    bool in_comment = false;
    string matrix;
    size_t index = -1;

    while (state != 6)
    {
        char c = is.get();

        if (c == EOF)
            break;

        if (in_comment)
        {
            if (c != '\n')
                continue;

            state = save_state;
            in_comment = false;
        }
        else if (c == '#')
        {
            save_state = state;
            in_comment = true;
            continue;
        }

        if ((state == 0) && isspace(c))
            continue;

        matrix += c;
        index++;

        switch (state)
        {
            case 0:
                if (c == '[')
                    state = 1;
                else if (!isspace(c))
                    state = 5;
                break;

            case 1:
                if (c == '[')
                    state = 2;
                else if (!isspace(c))
                    state = 4;
                break;

            case 2:
                if (c == ']')
                    state = 3;
                else if (c == '[')
                    throw MatrixParsingException<T>(
                            matrix, index, MPE_INVALID_SYNTAX);
                break;
            case 3:
                if (c == ']')
                    state = 6;
                else if (c == '[')
                    state = 2;
                break;

            case 4:
                if (c == ']')
                    state = 6;
                break;

            case 5:
                if (c == '\n')
                    state = 6;
                break;

            case 6:
                break;
        }
    }

    if (state == 0)
        return "";

    if (state != 6)
        throw MatrixParsingException<T>(matrix, matrix.size()-1, MPE_INVALID_SYNTAX);

    return matrix;
}

template<class T>
void Matrix<T>::read(istream& is)
{
    size_t index = 0;
    string matrix(_read_matrix(is));

    if (matrix.empty())
        return;

    vector< vector< Number<T> > > rows;
    size_t row_end, row_length = 0;

    index = matrix.find('[');
    if (index == string::npos)
        index = 0;
    else
        index++;

    while (index < matrix.size())
    {
        size_t row_start = matrix.find('[', index);
        if (row_start == string::npos)
        {
            row_end = matrix.find_first_of(";\n", index);
            if (row_end == string::npos)
                row_end = matrix.size()-1;
        }
        else
        {
            index = row_start + 1;
            row_end = matrix.find(']', index);
            if (row_end == string::npos)
                throw MatrixParsingException<T>(matrix, index, MPE_INVALID_SYNTAX);
        }

        while ((index < row_end) && isspace(matrix[index]))
            index++;

        if (index == row_end)
        {
            index++;
            continue;
        }

        vector< Number<T> > cols;
        string row(matrix, index, row_end - index);
        size_t rindex = 0;
        while (rindex < row.size())
        {
            size_t col_end = row.find(',', rindex);
            if (col_end == string::npos)
            {
                col_end = row.size();

                try
                {
                    string exp(row, rindex, col_end - rindex);
                    Number<T> n = Number<T>::parse_expression(exp);
                }
                catch (NumberParsingException<T>& e)
                {
                    col_end = rindex + e.index() - 1;
                    while ((col_end > rindex) && !isspace(row[col_end]))
                        col_end--;
                }
            }

            while ((rindex < col_end) && isspace(row[rindex]))
                rindex++;

            if (rindex == col_end)
            {
                rindex++;
                continue;
            }

            if ((row_length != 0) && (cols.size() == row_length))
                throw MatrixParsingException<T>(matrix, index+rindex-1, MPE_ROW_SIZE_MISMATCH);

            string col(row, rindex, col_end - rindex);
            try
            {
                Number<T> n = Number<T>::parse_expression(col);
                cols.push_back(n);
            }
            catch (NumberParsingException<T>& e)
            {
                //cout << "Error processing expression in column: \"" << col << "\"" << endl;
                throw MatrixParsingException<T>(matrix,
                        (index + rindex + e.index()), MPE_EXPRESSION_EXCEPTION);
            }

            rindex = col_end + 1;
        }

        if (cols.size() == 0)
            throw MatrixParsingException<T>(matrix, index, MPE_EMPTY_ROW);
        else if (row_length == 0)
            row_length = cols.size();
        else if (cols.size() != row_length)
            throw MatrixParsingException<T>(matrix, index, MPE_ROW_SIZE_MISMATCH);

        rows.push_back(cols);

        index = row_end + 1;

        if (matrix[row_start] == '[')
        {
            while ((index < matrix.size()) && (matrix[index] != '['))
                index++;
        }

        if ((index < matrix.size()) && (matrix[index] == ']'))
            break;
    }

    if (rows.empty())
        throw MatrixParsingException<T>(matrix, matrix.size()-1, MPE_EMPTY_MATRIX);

    *this = Matrix<T>(rows);
}

template<class T>
istream& operator>>(istream& is, Matrix<T>& rhs)
{
    rhs.read(is);
    return is;
}

// Exceptions ******************************************************************
template<class T>
MatrixException<T>::MatrixException(void)
{
}

template<class T>
MatrixException<T>::~MatrixException(void)
{
}

template<class T>
void MatrixException<T>::message(void) const
{
    cout << "MATRIX EXCEPTION" << endl;
}

template<class T>
MatrixInvalidAccessException<T>::MatrixInvalidAccessException(
        const Matrix<T>& A, uint32_t i, uint32_t j)
{
    _A = A;
    _i = i;
    _j = j;
}

template<class T>
MatrixInvalidAccessException<T>::~MatrixInvalidAccessException(void)
{
}

template<class T>
void MatrixInvalidAccessException<T>::message(void) const
{
    cout << "Invalid attempt to access row " << _i << " and column "
        << _j << " of a " << _A.rows() << "x" << _A.cols()
        << " matrix." << endl;
}

template<class T>
MatrixMultiplicationException<T>::MatrixMultiplicationException(
        const Matrix<T>& A, const Matrix<T>& B)
{
    _A = A;
    _B = B;
}

template<class T>
MatrixMultiplicationException<T>::~MatrixMultiplicationException(void)
{
}

template<class T>
void MatrixMultiplicationException<T>::message(void) const
{
    cout << "Can not multiply " << _A.rows() << "x" << _A.cols()
        << " matrix A with " << _B.rows() << "x" << _B.cols()
        << " matrix B." << endl;
}

template<class T>
MatrixAdditionException<T>::MatrixAdditionException(
        const Matrix<T>& A, const Matrix<T>& B)
{
    _A = A;
    _B = B;
}

template<class T>
MatrixAdditionException<T>::~MatrixAdditionException(void)
{
}

template<class T>
void MatrixAdditionException<T>::message(void) const
{
    cout << "Can not add " << _A.rows() << "x" << _A.cols()
        << " matrix A with " << _B.rows() << "x" << _B.cols()
        << " matrix B." << endl;
}

template<class T>
MatrixInvalidVectorException<T>::MatrixInvalidVectorException(
        const vector< vector< Number<T> > >& invalid_vector)
{
    _invalid_vector1 = new vector< vector< Number<T> > >(invalid_vector);
    _invalid_vector2 = nullptr;
}

template<class T>
MatrixInvalidVectorException<T>::MatrixInvalidVectorException(
        const vector< Number<T> >& invalid_vector)
{
    _invalid_vector1 = nullptr;
    _invalid_vector2 = new vector< Number<T> >(invalid_vector);
}

template<class T>
MatrixInvalidVectorException<T>::~MatrixInvalidVectorException(void)
{
    if (_invalid_vector1 != nullptr)
        delete _invalid_vector1;

    if (_invalid_vector2 != nullptr)
        delete _invalid_vector2;
}

template<class T>
void MatrixInvalidVectorException<T>::message(void) const
{
    if (_invalid_vector1 != nullptr)
    {
        set<int> mismatches;

        for (size_t i = 0; i < _invalid_vector1->size(); i++)
            mismatches.insert((*_invalid_vector1)[i].size());

        cout << "Vector used to create matrix had different "
            "row lengths." << endl;
        cout << "Mismatched vector row lengths: " << endl;
        while (!mismatches.empty())
        {
            cout << *mismatches.begin() << " ";
            mismatches.erase(mismatches.begin());
        }

        cout << endl;
    }
    else if (_invalid_vector2 != nullptr)
    {
        cout << "Invalid vector of size " << _invalid_vector2->size()
            << " cannot be used to create a matrix." << endl;
    }
}

template<class T>
MatrixNotSquareException<T>::MatrixNotSquareException(const Matrix<T>& A)
{
    _A = A;
}

template<class T>
MatrixNotSquareException<T>::~MatrixNotSquareException(void)
{
}

template<class T>
void MatrixNotSquareException<T>::message(void) const
{
    cout << "Attempt to perform an operation that requires "
       "a square n x n matrix with a matrix that is "
       << _A.rows() << "x" << _A.cols() << "." << endl;
}

template<class T>
MatrixSolutionsException<T>::MatrixSolutionsException(
        const Matrix<T>& A, const Matrix<T>& s)
{
    _A = A;
    _s = s;
}

template<class T>
MatrixSolutionsException<T>::~MatrixSolutionsException(void)
{
}

template<class T>
void MatrixSolutionsException<T>::message(void) const
{
    cout << "Attempt to get set of solutions with incompatible matrix "
        "of size " << _A.rows() << "x" << _A.cols() << " and solutions "
        "vector of size " << _s.rows() << "x" << _s.cols() << "." << endl;
}

template<class T>
MatrixSingularException<T>::MatrixSingularException(const Matrix<T>& A)
{
    _A = A;
}

template<class T>
MatrixSingularException<T>::~MatrixSingularException(void)
{
}

template<class T>
void MatrixSingularException<T>::message(void) const
{
    cout << "Matrix is singular and does not have a set of "
        "unique solutions." << endl;
}

template<class T>
MatrixParsingException<T>::MatrixParsingException(
        string str, size_t index, mpe_reason reason)
{
    _str = str;
    _index = index;
    _reason = reason;
}

template<class T>
MatrixParsingException<T>::~MatrixParsingException(void)
{
}

static map<mpe_reason,string> mpe_string_map = {
    { MPE_EMPTY_MATRIX, "Empty matrix." },
    { MPE_EMPTY_ROW, "Empty row." },
    { MPE_ROW_SIZE_MISMATCH, "Rows have different number of columns." },
    { MPE_INVALID_SYNTAX, "Invalid matrix syntax." },
    { MPE_EXPRESSION_EXCEPTION, "Expression in matrix entry is invalid." },
};

template<class T>
void MatrixParsingException<T>::message(void) const
{
    cout << "Error parsing matrix: ";
    if (mpe_string_map.find(_reason) != mpe_string_map.end())
        cout << mpe_string_map[_reason] << endl;
    else
        cout << "Reason unknown." << endl;

    size_t i = 0;
    size_t offset = 0;
    while (i < _str.size())
    {
        char c = _str[i];

        if (_index == i)
        {
            for (; i < _str.size(); i++)
            {
                if (_str[i] == '\n')
                    break;

                if (isspace(_str[i]))
                    cout << " ";
                else
                    cout << _str[i];
            }

            if (c == '\n')
                cout << " ";
            cout << endl;

            //cout << setw(_index-offset) << "^" << endl;
            for (size_t j = 0; j < _index-offset; j++)
                cout << '-';
            cout << "^" << endl;

            if (c == '\n')
                cout << c;
        }
        else
        {
            if (isspace(c) && (c != '\n'))
                cout << " ";
            else
                cout << c;
        }

        i++;

        if (c == '\n')
            offset = i;
    }
}

#endif
