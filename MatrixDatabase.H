#ifndef _MATRIX_DATABASE_H
#define _MATRIX_DATABASE_H

#include <string>
#include <vector>
#include <type_traits>
#include <functional>
#include <algorithm>
#include <utility>
#include <tuple>
#include <map>
#include <sqlite3.h>
#include <cstring>
#include <memory>
#include <exception>
using namespace std;

class FormatException : exception
{
    public:
        FormatException(const string& what) : _what(what) {}
        const char* what(void) const noexcept { return _what.c_str(); }

    private:
        string _what;
};

template<typename T>
inline typename enable_if<is_integral<T>::value, long>::type
norm_arg(T arg) { return arg; }

#if 0
template<typename T>
typename enable_if<is_unsigned<T>::value, unsigned long>::type
norm_arg(T arg) { return arg; }
#endif

template<typename T>
inline typename enable_if<is_floating_point<T>::value, double>::type
norm_arg(T arg) { return arg; }

template<typename T>
inline typename enable_if<is_pointer<T>::value, T>::type
norm_arg(T arg) { return arg; }

inline const char* norm_arg(const string& arg) { return arg.c_str(); }

inline void verify_format(const char* f)
{
    for (; *f; ++f)
    {
        if ((*f != '%') || *++f == '%')
            continue;

        throw FormatException("Not enough arguments");
    }
}

template<typename T, typename... Ts>
inline void verify_format(const char* f, const T& t, const Ts&... ts)
{
    for (; *f; ++f)
    {
        if ((*f != '%') || (*++f == '%'))
            continue;

        switch (*f)
        {
            case 's':
                if (!is_pointer<T>::value)
                    throw FormatException("Value not a pointer for %s specifier");
                break;
            case 'f':
                if (!is_floating_point<T>::value)
                    throw FormatException("Value not a floating point for %f specifier");
                break;
            case 'd':
                if (!is_integral<T>::value)
                    throw FormatException("Value not an integer for %d specifier");
                break;
            case 'u':
                if (!is_unsigned<T>::value)
                    throw FormatException("Value not unsigned for %u specifier");
                break;
            default:
                throw FormatException(string("Bad format specifier %") + *f);
        }

        return verify_format(++f, ts...);
    }

    throw FormatException("Too many arguments");
}

inline int db_snprintf(char* buf, size_t buf_size, const char* f)
{
    verify_format(f);
    return snprintf(buf, buf_size, "%s", f);
}

template<typename... Ts>
inline int db_snprintf(char* buf, size_t buf_size, const char* f, const Ts&... ts)
{
    verify_format(f, norm_arg(ts)...);
    return snprintf(buf, buf_size, f, norm_arg(ts)...);
}

enum DBFunction
{
    DB_OPEN,
    DB_EXEC,
    DB_PREPARE,
    DB_BIND,
    DB_STEP,
    DB_RESET,
    DB_CLEAR_BINDINGS,
    DB_FINALIZE,
    DB_CLOSE,
};

class DatabaseException : public exception
{
    public:
        DatabaseException(const string& what) : _what(what) { }
        DatabaseException(int rc, DBFunction dbfunc)
        {
            string err_str("(" + to_string(rc) + ") " + sqlite3_errstr(rc)); 

            for (size_t i = 0; i < err_str.size(); i++)
                err_str[i] = toupper(err_str[i]);

            switch (dbfunc)
            {
                case DB_OPEN:
                    _what = "Failed to open database";
                    break;
                case DB_EXEC:
                    _what = "Failed to execute database command";
                    break;
                case DB_PREPARE:
                    _what = "Failed to prepare statement";
                    break;
                case DB_BIND:
                    _what = "Failed to bind value to statment";
                    break;
                case DB_STEP:
                    _what = "Failed to evaluate (step) a statement";
                    break;
                case DB_RESET:
                    _what = "Failed to reset prepared statement";
                    break;
                case DB_CLEAR_BINDINGS:
                    _what = "Failed to clear bindings on prepared statement";
                    break;
                case DB_FINALIZE:
                    _what = "Failed to finalize prepared statement";
                    break;
                case DB_CLOSE:
                    _what = "Failed to close database";
                    break;
            }

            _what += ": " + err_str;
        }

        DatabaseException(int rc, DBFunction dbfunc, string s)
            : DatabaseException(rc, dbfunc)
        {
            _what += "\n" + s;
        }

        DatabaseException(const DatabaseException& e) { _what = e._what; }

        const char* what(void) const noexcept { return _what.c_str(); }

    private:
        string _what;
};

template<typename... Tps>
class Statement
{
    public:
        Statement(void)
            : _db(NULL), _stmt(NULL), _params(0), _results(0), _num_cols(0) { }

        template<typename... Ts>
        Statement(sqlite3* db, const char* stmt, const Ts&... ts)
            : _db(db), _stmt(NULL), _params(0), _results(0), _num_cols(0)
        {
            try
            {
                db_snprintf(_stmt_str, sizeof(_stmt_str), stmt, ts...);
            }
            catch (FormatException& e)
            {
                string s(e.what());
                throw DatabaseException("Failed to parse statement: " + s);
            }

            prepare();
        }

        Statement(const Statement& rhs)
        {
            clear_results();
            finalize();

            _db = rhs._db;
            _stmt = rhs._stmt;
            _params = rhs._params;
            _result_funcs = rhs._result_funcs;
            _num_cols = rhs._num_cols;

            for (size_t i = 0; i < sizeof...(Tps); i++)
                _col_names[i] = rhs._col_names[i];
        }

        friend void swap(Statement& lhs, Statement& rhs)
        {
            using std::swap;

            swap(lhs._db, rhs._db);
            swap(lhs._stmt, rhs._stmt);
            swap(lhs._params, rhs._params);
            swap(lhs._result_funcs, rhs._result_funcs);
            swap(lhs._num_cols, rhs._num_cols);

            auto s(lhs._col_names);

            for (size_t i = 0; i < sizeof...(Tps); i++)
                lhs._col_names[i] = rhs._col_names[i];

            for (size_t i = 0; i < sizeof...(Tps); i++)
                rhs._col_names[i] = s[i];
        }

        Statement(Statement&& rhs)
            : Statement()
        {
            clear_results();
            finalize();

            swap(*this, rhs);
        }

        Statement& operator=(Statement rhs)
        {
            clear_results();
            finalize();

            swap(*this, rhs);

            return *this;
        }

        ~Statement(void)
        {
            clear_results();
            finalize();
        }

        // Binding Functions
        template<size_t I = 0, typename... Ts>
        void bind(const Ts&... ts) { }

        template<size_t I = 0, typename T, typename... Ts>
        void bind(const T& t, const Ts&... ts)
        {
            if (I >= _params.size())
            {
                string s("Attempt to bind too many parameters (" + to_string(I)
                        + ") to statement (" + to_string(_params.size()) + ")");
                throw DatabaseException(s);
            }

            int rc;
            if ((rc = bind_arg(I+1, norm_arg(t))) != SQLITE_OK)
                throw DatabaseException(rc, DB_BIND);

            bind<I+1>(ts...);
        }

        template<typename T>
        void operator()(size_t index, T val)
        {
            if (index >= _params.size())
                throw DatabaseException("Attempt to bind at nonexistant index");

            bind_arg(index, val);
        }

        int get_index(const string& name)
        {
            for (auto i : _params)
            {
                if (get<0>(i) == name)
                    return get<1>(i);
            }

            return -1;
        }

        template<typename T>
        void operator()(const string& col, T val)
        {
            int index = get_index(col);
            if (index == -1)
                throw DatabaseException("Attempt to bind to nonexistant column name");

            bind_arg(index, val);
        }

        void clear(void)
        {
            int rc;
            if ((rc = sqlite3_clear_bindings(_stmt)) != SQLITE_OK)
                throw DatabaseException(rc, DB_CLEAR_BINDINGS);
        }

        // Execution functions
        int exec(void)
        {
            //size_t row = 0;
            int rc;

            _results.clear();

            while ((rc = sqlite3_step(_stmt)) == SQLITE_ROW)
            {
                result();
                _results.push_back(_result);
            }

            if (rc != SQLITE_DONE)
                throw DatabaseException(rc, DB_STEP);

            if ((rc = sqlite3_reset(_stmt)) != SQLITE_OK)
                throw DatabaseException(rc, DB_RESET);

            return sqlite3_changes(_db);
        }

        template<typename... Ts>
        int exec(const Ts&... ts)
        {
            bind(ts...);
            return exec();
        }

        // Results functions
        size_t rows(void)
        {
            return _results.size();
        }

        template<size_t I, typename T>
        T column_result(size_t row)
        {
            return get<I>(_results[row]);
        }

        tuple<Tps...> operator[](size_t row)
        {
            if (row >= _results.size())
                throw DatabaseException("Attempt to retrieve nonexistant result row");

            return _results[row];
        }

        void finalize(void)
        {
            if (_stmt != NULL)
                (void)sqlite3_finalize(_stmt);

            _stmt = NULL;
        }

    private:
        void prepare(void)
        {
            if ((_db == NULL) || (_stmt != NULL))
                return;

            int rc = sqlite3_prepare_v2(_db, _stmt_str, -1, &_stmt, NULL);
            if (rc != SQLITE_OK)
                throw DatabaseException(rc, DB_PREPARE, _stmt_str);

            int pcount = sqlite3_bind_parameter_count(_stmt);
            for (int i = 1; i <= pcount; i++)
            {
                const char* param = sqlite3_bind_parameter_name(_stmt, i);
                if (param == NULL)
                    _params.push_back(make_tuple("UNNAMED", i));
                else
                    _params.push_back(make_tuple(param+1, i));
            }

            size_t col_count = sqlite3_column_count(_stmt);
            if (col_count != sizeof...(Tps))
            {
                string s(_stmt_str);
                throw DatabaseException("Mismatched column count "
                        "for prepared statment:\n" + s + "\nExpected " + to_string(sizeof...(Tps)) +
                            " and got " + to_string(col_count));
            }

            set_result_funcs();
        }

        template<typename T> typename enable_if<is_pointer<T>::value, int>::type
        bind_arg(size_t index, T val)
        {
            return sqlite3_bind_text(_stmt, index, val, -1, SQLITE_STATIC);
        } 

        template<typename T> typename enable_if<is_integral<T>::value, int>::type
        bind_arg(size_t index, T val)
        {
            return sqlite3_bind_int(_stmt, index, val);
        }

        template<typename T> typename enable_if<is_floating_point<T>::value, int>::type
        bind_arg(size_t index, T val)
        {
            return sqlite3_bind_double(_stmt, index, val);
        }

        template<size_t I, typename T> typename enable_if<is_pointer<T>::value, void>::type
        result_func(void)
        {
            get<I>(_result_funcs) = std::bind(sqlite3_column_text, _stmt, placeholders::_1);
        } 

        template<size_t I, typename T> typename enable_if<is_integral<T>::value, void>::type
        result_func(void)
        {
            get<I>(_result_funcs) = std::bind(sqlite3_column_int, _stmt, placeholders::_1);
        }

        template<size_t I, typename T> typename enable_if<is_floating_point<T>::value, void>::type
        result_func(void)
        {
            get<I>(_result_funcs) = std::bind(sqlite3_column_double, _stmt, placeholders::_1);
        }

        template<size_t I = 0>
        typename enable_if<I == sizeof...(Tps), void>::type
        set_result_funcs(void) {}

        template<size_t I = 0>
        typename enable_if<I != sizeof...(Tps), void>::type
        set_result_funcs(void)
        {
            using T = typename tuple_element<I, tuple<Tps...> >::type;
            result_func<I,T>();
            set_result_funcs<I+1>();
        }

        template<size_t I, typename T>
        typename enable_if<is_pointer<T>::value, void>::type
        clear_result(size_t row)
        {
            char* str = (char*)get<I>(_results[row]);
            if (str != nullptr)
                delete [] get<I>(_results[row]);
        }

        template<size_t I, typename T>
        typename enable_if<!is_pointer<T>::value, void>::type
        clear_result(size_t row) { }

        template<size_t I = 0>
        typename enable_if<I == sizeof...(Tps), void>::type
        clear_results(void) {}

        template<size_t I = 0>
        typename enable_if<I != sizeof...(Tps), void>::type
        clear_results(void)
        {
            using T = typename tuple_element<I, tuple<Tps...> >::type;

            for (size_t i = 0; i < _results.size(); i++)
                clear_result<I,T>(i);

            clear_results<I+1>();
        }

        template<size_t I, typename T>
        typename enable_if<is_pointer<T>::value, void>::type
        copy_result(void)
        {
            const char* src = (const char*)get<I>(_result_funcs)(I);
            if (src == nullptr)
            {
                get<I>(_result) = nullptr;
                return;
            }

            char* dst = new char[strlen(src)+1];
            strcpy(dst, src);
            get<I>(_result) = (unsigned const char*)dst;
        }

        template<size_t I, typename T>
        typename enable_if<!is_pointer<T>::value, void>::type
        copy_result(void)
        {
            get<I>(_result) = get<I>(_result_funcs)(I);
        }

        template<size_t I = 0>
        typename enable_if<I == sizeof...(Tps), void>::type
        result(void) {}

        template<size_t I = 0>
        typename enable_if<I != sizeof...(Tps), void>::type
        result(void)
        {
            if (_col_names[I].empty())
                _col_names[I] = sqlite3_column_name(_stmt, I);

            using T = typename tuple_element<I, tuple<Tps...> >::type;
            copy_result<I,T>();

            result<I+1>();
        }

        sqlite3* _db;
        sqlite3_stmt* _stmt;
        char _stmt_str[512];
        vector<tuple<string,int>> _params;
        string _col_names[sizeof...(Tps)];
        tuple<Tps...> _result;
        vector<tuple<Tps...>> _results;
        tuple<function<Tps(int)>...> _result_funcs;
        int _num_cols;
};

struct MatrixInfo
{
    size_t id;
    string timestamp;
    size_t dimension;
    string data;
    string notes;
};

class MatrixDatabase
{
    public:
        MatrixDatabase(const string& db_name);
        ~MatrixDatabase(void);
        void open(void);
        void close(void);
        vector<MatrixInfo> entries(void);
        MatrixInfo entry(size_t id);
        vector<MatrixInfo> query(const string& search);
        int insert(MatrixInfo& mi);
        int update(const MatrixInfo& mi);
        int remove(size_t id);

    private:
        void _create(void);
        void _prepare(void);
        bool _verify_table_info(void);

        sqlite3* _db;
        string _db_name;

        enum PreparedStatement
        {
            PS_ENTRIES = 0,
            PS_ENTRY,
            PS_QUERY,
            PS_INSERT,
            PS_UPDATE,
            PS_DELETE,
            PS_CREATE,
            PS_TRIGGER,
            PS_TABLES,
            PS_TABLE_INFO
        };

#define SELECT_TEMPL int,int,const unsigned char*,const unsigned char*
#define TABLE_TEMPL const unsigned char*
#define INFO_TEMPL int, const unsigned char*, const unsigned char*, int, const unsigned char*, int

        tuple<
            Statement<SELECT_TEMPL>,   // PS_ENTRIES
            Statement<SELECT_TEMPL>,   // PS_ENTRY
            Statement<SELECT_TEMPL>,   // PS_QUERY
            Statement<>,               // PS_INSERT
            Statement<>,               // PS_UPDATE
            Statement<>,               // PS_DELETE
            Statement<>,               // PS_CREATE
            Statement<>,               // PS_TRIGGER
            Statement<TABLE_TEMPL>,    // PS_TABLES
            Statement<INFO_TEMPL>      // PS_TABLE_INFO
                > _prepared_stmts;

        const char* _db_table_name = "matrix_pane";
        const char* _db_pk = "id";
        const char* _db_timestamp = "timestamp";
        const char* _db_dimension = "n";
        const char* _db_data = "data";
        const char* _db_notes = "notes";

        const char* _create_stmt =
            "CREATE TABLE IF NOT EXISTS %s(\n"
            "    %s           INTEGER NOT NULL,\n"
            "    %s           DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
            "    %s           INTEGER NOT NULL,\n"
            "    %s           TEXT NOT NULL,\n"
            "    %s           TEXT,\n"
            "    PRIMARY KEY(%s),\n"
            "    CHECK (%s >= 2)\n"
            ");";

        const char* _trigger_stmt =
            "CREATE TRIGGER IF NOT EXISTS update_timestamp AFTER UPDATE ON %s\n"
            "    BEGIN\n"
            "        UPDATE %s SET %s=datetime('now') WHERE %s=old.%s;\n"
            "    END;\n";

        const char* _table_stmt = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
        const char* _table_info_stmt = "PRAGMA table_info(%s);";

        const char* _entries_stmt = "SELECT %s, %s, %s, %s FROM %s ORDER BY %s DESC;";
        const char* _entry_stmt   = "SELECT %s, %s, %s, %s FROM %s WHERE %s=@%s;";
        const char* _query_stmt   = "SELECT %s, %s, %s, %s FROM %s WHERE %s LIKE '%%' || @%s || '%%';";
        const char* _insert_stmt  = "INSERT INTO %s (%s, %s, %s) VALUES (@%s, @%s, @%s);";
        const char* _update_stmt  = "UPDATE %s SET %s=@%s, %s=@%s, %s=@%s WHERE %s=@%s;";
        const char* _delete_stmt  = "DELETE FROM %s WHERE %s=@%s;";

        enum ColumnType
        {
            CT_INTEGER,
            CT_DATETIME,
            CT_TEXT,
            CT_DOUBLE,
            CT_BLOB,
        };

        const map<ColumnType, const char*> _db_type_str = {
            { CT_INTEGER, "INTEGER" },
            { CT_DATETIME, "DATETIME" },
            { CT_TEXT, "TEXT" },
            { CT_DOUBLE, "DOUBLE" },
            { CT_BLOB, "BLOB" },
        };

        enum ColumnSpec
        {
            CS_NAME,
            CS_TYPE,
            CS_PK,
            CS_NULL,
            CS_DEFAULT,
        };

        typedef tuple<const char*, int, bool, bool, const char*> column_t;
        const vector<column_t> _db_table_columns = {
            make_tuple(_db_pk, CT_INTEGER, true, false, nullptr),
            make_tuple(_db_timestamp, CT_DATETIME, false, false, "CURRENT_TIMESTAMP"),
            make_tuple(_db_dimension, CT_INTEGER, false, false, nullptr),
            make_tuple(_db_data, CT_TEXT, false, false, nullptr),
            make_tuple(_db_notes, CT_TEXT, false, true, nullptr),
        };

        enum TableSpec
        {
            TS_NAME,
            TS_COLUMNS,
            TS_PK,
        };

        typedef tuple<const char*, vector<column_t>, const char*> table_t;

        const table_t _db_table = make_tuple(_db_table_name, _db_table_columns, _db_pk);
};

#endif
