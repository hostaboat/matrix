#include "MatrixDatabase.H"
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <sqlite3.h>
using namespace std;

MatrixDatabase::MatrixDatabase(const string& db_name)
    : _db(NULL), _db_name(db_name)
{
    open();
}

MatrixDatabase::~MatrixDatabase(void)
{
    close();
}

void MatrixDatabase::open(void)
{
    if (_db != NULL)
        close();

    int rc = sqlite3_open_v2(_db_name.c_str(), &_db, SQLITE_OPEN_READWRITE, NULL);
    if (rc == SQLITE_OK)
    {
        if (!_verify_table_info())
        {
            close();

            string renamed(_db_name + ".bk");
            (void)rename(_db_name.c_str(), renamed.c_str());

            _create();
        }
    }
    else
    {
        close();
        _create();
    }

    _prepare();
}

void MatrixDatabase::close(void)
{
    if (_db == NULL)
        return;

    try
    {
        get<PS_ENTRIES>(_prepared_stmts).finalize();
        get<PS_ENTRY>(_prepared_stmts).finalize();
        get<PS_QUERY>(_prepared_stmts).finalize();
        get<PS_INSERT>(_prepared_stmts).finalize();
        get<PS_UPDATE>(_prepared_stmts).finalize();
        get<PS_DELETE>(_prepared_stmts).finalize();
        get<PS_CREATE>(_prepared_stmts).finalize();
        get<PS_TRIGGER>(_prepared_stmts).finalize();
        get<PS_TABLES>(_prepared_stmts).finalize();
        get<PS_TABLE_INFO>(_prepared_stmts).finalize();
    }
    catch (DatabaseException& e)
    {
        throw;
    }

    int rc;
    //while ((rc = sqlite3_close(_db)) == SQLITE_BUSY);
    while ((rc = sqlite3_close_v2(_db)) == SQLITE_BUSY);
    if (rc != SQLITE_OK)
        throw DatabaseException(rc, DB_CLOSE);

    _db = NULL;
}

vector<MatrixInfo> MatrixDatabase::entries(void)
{
    try
    {
        (void)get<PS_ENTRIES>(_prepared_stmts).exec();
    }
    catch (DatabaseException& e)
    {
        throw;
    }

    vector<MatrixInfo> ret;

    for (size_t i = 0; i < get<PS_ENTRIES>(_prepared_stmts).rows(); i++)
    {
        MatrixInfo mi;

        tuple<SELECT_TEMPL> row = get<PS_ENTRIES>(_prepared_stmts)[i];

        mi.id = get<0>(row);
        mi.dimension = get<1>(row);
        mi.data = (const char*)get<2>(row);
        mi.notes = (const char*)get<3>(row);

        ret.push_back(mi);
    }

    return ret;
}

MatrixInfo MatrixDatabase::entry(size_t id)
{
    try
    {
        (void)get<PS_ENTRY>(_prepared_stmts).exec(id);
    }
    catch (DatabaseException& e)
    {
        throw;
    }

    MatrixInfo ret;

    if (get<PS_ENTRY>(_prepared_stmts).rows() == 0)
        throw DatabaseException("Entry with id=" + to_string(id) + " not in database.");

    MatrixInfo mi;

    tuple<SELECT_TEMPL> row = get<PS_ENTRY>(_prepared_stmts)[0];

    mi.id = get<0>(row);
    mi.dimension = get<1>(row);
    mi.data = (const char*)get<2>(row);
    mi.notes = (const char*)get<3>(row);

    return mi;
}

vector<MatrixInfo> MatrixDatabase::query(const string& search)
{
    try
    {
        (void)get<PS_QUERY>(_prepared_stmts).exec(search.c_str());
    }
    catch (DatabaseException& e)
    {
        throw;
    }

    vector<MatrixInfo> ret;

    for (size_t i = 0; i < get<PS_QUERY>(_prepared_stmts).rows(); i++)
    {
        MatrixInfo mi;

        tuple<SELECT_TEMPL> row = get<PS_QUERY>(_prepared_stmts)[i];

        mi.id = get<0>(row);
        mi.dimension = get<1>(row);
        mi.data = (const char*)get<2>(row);
        mi.notes = (const char*)get<3>(row);

        ret.push_back(mi);
    }

    return ret;
}

int MatrixDatabase::insert(MatrixInfo& mi)
{
    const char* ins_id_stmt = "SELECT %s FROM %s WHERE %s=last_insert_rowid();";

    try
    {
        int changes = get<PS_INSERT>(_prepared_stmts).exec(mi.dimension, mi.data.c_str(), mi.notes.c_str());
        if (changes != 1)
            throw DatabaseException("On INSERT: Wrong number of changes");

        Statement<size_t> s(_db, ins_id_stmt, _db_pk, _db_table_name, _db_pk);
        s.exec();
        tuple<size_t> result(s[0]);

        mi.id = get<0>(result);

        return changes;
    }
    catch (DatabaseException& e)
    {
        throw;
    }
}

int MatrixDatabase::update(const MatrixInfo& mi)
{
    try
    {
        int changes = get<PS_UPDATE>(_prepared_stmts).exec(mi.dimension, mi.data.c_str(), mi.notes.c_str(), mi.id);
        if (changes != 1)
            throw DatabaseException("On UPDATE: Wrong number of changes. Expected 1 but got " + to_string(changes));

        return changes;
    }
    catch (DatabaseException& e)
    {
        throw;
    }
}

int MatrixDatabase::remove(size_t id)
{
    try
    {
        int changes = get<PS_DELETE>(_prepared_stmts).exec(id);
        if (changes != 1)
            throw DatabaseException("On DELETE: Wrong number of changes");

        return changes;
    }
    catch (DatabaseException& e)
    {
        throw;
    }
}

bool MatrixDatabase::_verify_table_info(void)
{
    if (_db == NULL)
        return false;

    try
    {
        get<PS_TABLES>(_prepared_stmts) = Statement<TABLE_TEMPL>(_db, _table_stmt);
        get<PS_TABLES>(_prepared_stmts).exec();

        get<PS_TABLE_INFO>(_prepared_stmts) = Statement<INFO_TEMPL>(_db, _table_info_stmt, _db_table_name);
        get<PS_TABLE_INFO>(_prepared_stmts).exec();
    }
    catch (DatabaseException& e)
    {
        throw;
    }

    if (get<PS_TABLES>(_prepared_stmts).rows() != 1)
        return false;

    auto s_cmp = [] (const char* n1, const char* n2) -> bool
    {
        if ((n1 == NULL) && (n2 == NULL))
            return true;
        else if ((n1 == NULL) || (n2 == NULL))
            return false;

        return strcmp(n1, n2) == 0;
    };

    const char* table = (const char*)(get<0>(get<PS_TABLES>(_prepared_stmts)[0]));
    if (!s_cmp(table, _db_table_name))
        return false;

    enum ColumnInfo
    {
        TI_CID,
        TI_NAME,
        TI_TYPE,
        TI_NOT_NULL,
        TI_DEFAULT,
        TI_IS_PK,
    };

    vector<tuple<INFO_TEMPL>> table_info;
    for (size_t i = 0; i < get<PS_TABLE_INFO>(_prepared_stmts).rows(); i++)
        table_info.push_back(get<PS_TABLE_INFO>(_prepared_stmts)[i]);

    if (table_info.size() != _db_table_columns.size())
        return false;

    auto compare_column =
        [&s_cmp]
        (column_t& a, tuple<INFO_TEMPL>& b, map<ColumnType, const char*>& tmap) -> bool
    {
        if (!s_cmp(get<CS_NAME>(a), (const char*)get<TI_NAME>(b)))
            return false;

        if (!s_cmp(tmap[(ColumnType)(get<CS_TYPE>(a))], (const char*)get<TI_TYPE>(b)))
            return false;

        if (get<CS_NULL>(a) == get<TI_NOT_NULL>(b))
            return false;

        if (!s_cmp(get<CS_DEFAULT>(a), (const char*)get<TI_DEFAULT>(a)))
            return false;

        if (get<CS_PK>(a) != get<TI_IS_PK>(b))
            return false;

        return true;
    };

    auto t1 = _db_table_columns;
    auto t2 = table_info;
    map<ColumnType, const char*> tmap = _db_type_str;

    sort(t1.begin(), t1.end(), [&] (column_t& a, column_t& b) -> bool
            { return string(get<CS_NAME>(a)) > string(get<CS_NAME>(b)); });

    sort(t2.begin(), t2.end(), [&] (tuple<INFO_TEMPL>& a, tuple<INFO_TEMPL>& b) -> bool
            { return string((const char*)get<TI_NAME>(a)) > string((const char*)get<TI_NAME>(b)); });

    for (size_t i = 0; i < t1.size(); i++)
    {
        if (!compare_column(t1[i], t2[i], tmap))
            return false;
    }

    return true;
}

void MatrixDatabase::_create(void)
{
    int rc = sqlite3_open_v2(_db_name.c_str(), &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK)
        throw DatabaseException(rc, DB_OPEN);

    char* errmsg;
    rc = sqlite3_exec(_db, "BEGIN TRANSACTION;", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        string s(errmsg);
        sqlite3_free(errmsg);

        throw DatabaseException(s);
    }

    try
    {
        get<PS_CREATE>(_prepared_stmts) =
            Statement<>(_db, _create_stmt, _db_table_name, _db_pk, _db_timestamp,
                    _db_dimension, _db_data, _db_notes, _db_pk, _db_dimension);
        get<PS_CREATE>(_prepared_stmts).exec();

        get<PS_TRIGGER>(_prepared_stmts) =
            Statement<>(_db, _trigger_stmt, _db_table_name,
                    _db_table_name, _db_timestamp, _db_pk, _db_pk);
        get<PS_TRIGGER>(_prepared_stmts).exec();
    }
    catch (DatabaseException& e)
    {
        rc = sqlite3_exec(_db, "ROLLBACK TRANSACTION;", NULL, NULL, &errmsg);
        if (rc != SQLITE_OK)
        {
            string s(string(e.what()) + "\n" + string(errmsg));
            sqlite3_free(errmsg);

            throw DatabaseException(s);
        }

        throw;
    }

    rc = sqlite3_exec(_db, "COMMIT TRANSACTION;", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        string s(errmsg);
        sqlite3_free(errmsg);

        throw DatabaseException(s);
    }
}

void MatrixDatabase::_prepare(void)
{
    try
    {
        get<PS_ENTRIES>(_prepared_stmts) =
            Statement<SELECT_TEMPL>(_db, _entries_stmt, _db_pk, _db_dimension,
                    _db_data, _db_notes, _db_table_name, _db_timestamp);

        get<PS_ENTRY>(_prepared_stmts) =
            Statement<SELECT_TEMPL>(_db, _entry_stmt, _db_pk, _db_dimension,
                    _db_data, _db_notes, _db_table_name, _db_pk, _db_pk);

        get<PS_QUERY>(_prepared_stmts) =
            Statement<SELECT_TEMPL>(_db, _query_stmt, _db_pk, _db_dimension,
                    _db_data, _db_notes, _db_table_name, _db_notes, _db_notes);

        get<PS_INSERT>(_prepared_stmts) =
            Statement<>(_db, _insert_stmt, _db_table_name, _db_dimension,
                    _db_data, _db_notes, _db_dimension, _db_data, _db_notes);

        get<PS_UPDATE>(_prepared_stmts) =
            Statement<>(_db, _update_stmt, _db_table_name, _db_dimension, _db_dimension,
                    _db_data, _db_data, _db_notes, _db_notes, _db_pk, _db_pk);

        get<PS_DELETE>(_prepared_stmts) =
            Statement<>(_db, _delete_stmt, _db_table_name, _db_pk, _db_pk);
    }
    catch (DatabaseException& e)
    {
        throw;
    }
}

#if 0
int main(int argc, char** argv)
{
    MatrixDatabase db("matrix_panes.db");
    try
    {
        //size_t id = db.insert(3, "[[3,4][5,6][6,7]][[4][5][3]][[3][2][1]]", "This is another note");
        //cout << "Inserted (" << id << ")" << endl;

        size_t id = db.insert(4, "[[7]][[3]][[1]]", "This is another note");
        cout << "Inserted (" << id << ")" << endl;
        db.update(id, 3, "[[3,4][5,6][6,7]][[4][5][3]][[3][2][1]]", "This is an amended note");
        db.remove(id - 1);
        db.query();
    }
    catch (DatabaseException& e)
    {
        cout << e.what() << endl;
    }

    db.close();

    return 0;
}
#endif
