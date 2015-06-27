#include "Matrix.H"
#include "Rational.H"
#include "Scientific.H"
#include "Number.H"
#include "MatrixEditor.H"
#include <iostream>
#include <fstream>
#include <cinttypes>
#include <string>
#include <map>
#include <unistd.h>
using namespace std;

static char buffer[1024];

void expression(void)
{
    cout << "Enter expression: ";

    cin.getline(buffer, sizeof(buffer));
    string line(buffer);

    cout << endl;

    try
    {
        Number<Scientific> s = Number<Scientific>::parse_expression(line);

        cout << "Scientific: " << s << endl;
        cout << "  Rational: " << s->toRational() << endl;
        cout << "     Float: " << s->toFloat() << endl;
    }
    catch (NumberParsingException<Scientific>& e)
    {
        e.message();
    }
    catch (RationalException& e)
    {
        e.message();
    }
    catch (ScientificException& e)
    {
        e.message();
    }
}

template<class T>
void solveT(Matrix<T>& A, Matrix<T>& s)
{
    try
    {
        cout << "Coefficient matrix: " << endl;
        cin >> A;
        cout << endl << A << endl;

        cout << "Solution vector: " << endl;
        cin >> s;
        cout << endl << s << endl;

        cin.unget();
        cin.ignore(256, '\n');

        Matrix<T> v = A.solve(s);
        cout << "Solution: " << endl;
        cout << v << endl;

        cout << "Print additional info (y/n): ";
        cin.getline(buffer, sizeof(buffer));

        string line(buffer);

        if (line.find('y') != string::npos)
        {
            cout << "Determinant: " << A.determinant() << endl << endl;

            cout << "Cofactor matrix: " << endl;
            cout << A.cofactor_matrix() << endl;

            cout << "Adjoint: " << endl;
            cout << A.adjoint() << endl;

            cout << "Inverse: " << endl;
            cout << A.inverse() << endl;

            cout << "Cramer's rule: " << endl;
            for (uint32_t j = 1; j <= A.rows(); j++)
            {
                Matrix<T> CR = A.cramers_rule(s, j);
                cout << CR << endl;
            }
        }

        cout << endl << "Press enter to continue... ";
        cin.getline(buffer, sizeof(buffer));
        cout << endl;
    }
    catch(const MatrixException<T>& e)
    {
        e.message();
    }
    catch (const RationalException& e)
    {
        e.message();
    }
    catch (const ScientificException& e)
    {
        e.message();
    }
}

void solve(void)
{
    cout << "Rational (r), Scientific (s) or Float (f): ";

    cin.getline(buffer, sizeof(buffer));
    string line(buffer);

    if (line.find('r') != string::npos)
    {
        Matrix<Rational> A, s;
        solveT(A, s);
    }
    else if (line.find('s') != string::npos)
    {
        Matrix<Scientific> A, s;
        solveT(A, s);
    }
    else if (line.find('f') != string::npos)
    {
        Matrix<double> A, s;
        solveT(A, s);
    }
    else
    {
        cout << "Invalid option: " << line << endl;
    }
}

template<class T>
void readFileT(Matrix<T>& A, Matrix<T>& s, const string& filename)
{
    ifstream ifs(filename);

    try
    {
        while ((ifs >> A) && (ifs >> s))
        {
            cout << "Coefficient matrix: " << endl;
            cout << endl << A << endl;

            cout << "Solution vector: " << endl;
            cout << endl << s << endl;

            Matrix<T> v = A.solve(s);
            cout << "Solution: " << endl;
            cout << v << endl;

            cout << "Print additional info (y/n): ";
            cin.getline(buffer, sizeof(buffer));

            string line(buffer);

            if (line.find('y') != string::npos)
            {
                cout << "Determinant: " << A.determinant() << endl << endl;

                cout << "Cofactor matrix: " << endl;
                cout << A.cofactor_matrix() << endl;

                cout << "Adjoint: " << endl;
                cout << A.adjoint() << endl;

                cout << "Inverse: " << endl;
                cout << A.inverse() << endl;

                cout << "Cramer's rule: " << endl;
                for (uint32_t j = 1; j <= A.rows(); j++)
                {
                    Matrix<T> CR = A.cramers_rule(s, j);
                    cout << CR << endl;
                }
            }
        }
    }
    catch (MatrixException<T>& e)
    {
        e.message();
    }

    ifs.close();
}

void readFile(const char* filename)
{
    //string file_name = "test.txt";
    string file_name(filename);
    //Matrix<Rational> m;
    //Matrix<double> m;
    Matrix<double> A, s;
    cout << "Reading file: " << file_name << endl;
    readFileT(A, s, file_name);
}

template<class T>
void testFileT(Matrix<T>& m, const string& filename)
{
    ifstream ifs(filename);

    try
    {
        while (ifs >> m)
            cout << m << endl;
    }
    catch (MatrixException<T>& e)
    {
        e.message();
    }

    ifs.close();
}

void testFile(const char* filename)
{
    //string file_name = "test.txt";
    string file_name(filename);
    //Matrix<Rational> m;
    //Matrix<double> m;
    Matrix<Scientific> m;
    testFileT(m, file_name);
}

void commandLine(void)
{
    cout << endl;

    while (true)
    {
        cout << "(S)olve set of equations, (E)valuate expression or (Q)uit: ";
        cin.getline(buffer, sizeof(buffer));
        cout << endl;

        string line(buffer);

        if ((line.find('S') != string::npos) || (line.find('s') != string::npos))
            solve();
        else if ((line.find('E') != string::npos) || (line.find('e') != string::npos))
            expression();
        else if ((line.find('Q') != string::npos) || (line.find('q') != string::npos))
            return;
        else if (line.empty())
            continue;
        else
            cout << "Invalid choice: " << line << endl;

        cout << endl;
    }
}

void ui(void)
{
    MatrixEditor me;
    me.init();
    me.loop();
}

int main(int argc, char** argv)
{
    int option = 0;
    bool multi_options = false;
    char* file;
    int ch;

    while ((ch = getopt(argc, argv, "cuf:t:")) != -1)
    {
        switch (ch)
        {
            case 'c':
                break;
            case 'u':
                break;
            case 'f':
                file = optarg;
                break;
            case 't':
                file = optarg;
                break;
            default:
                return -1;
        }

        if (option)
            multi_options = true;

        option = ch;
    }

    if (multi_options)
    {
        cout << "Only one option can be specified" << endl;
        return -1;
    }

    if (option == 'f')
        readFile(file);
    else if (option == 't')
        testFile(file);
    else if (option == 'c')
        commandLine();
    else
        ui();

    return 0;
}
