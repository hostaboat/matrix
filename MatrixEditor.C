#include "MatrixEditor.H"
#include "Number.H"
#include "Rational.H"
#include "Scientific.H"
#include "Matrix.H"
#include <ncurses.h>
#include <list>
#include <vector>
#include <string>
#include <map>
#include <csignal>
using namespace std;

vector<int> ColorPair::color_pairs(50, 0);

Cursor::Cursor(void)
    : _row(0), _col(0)
{
}

Cursor::Cursor(int row, int col)
    : _row(row), _col(col)
{
}

Cursor::Cursor(const Cursor& cursor)
    : _row(cursor._row), _col(cursor._col)
{
}

Cursor& Cursor::operator=(const Cursor& rhs)
{
    if (this != &rhs)
    {
        _row = rhs._row;
        _col = rhs._col;
    }

    return *this;
}

int Cursor::operator+(int amount)
{
    _row += amount;
    return _row;
}

int Cursor::operator-(int amount)
{
    _row -= amount;
    return _row;
}

int Cursor::operator>>(int amount)
{
    _col += amount;
    return _col;
}

int Cursor::operator<<(int amount)
{
    _col -= amount;
    return _col;
}

int Cursor::row(void)
{
    return _row;
}

int Cursor::col(void)
{
    return _col;
}

WindowEntry::WindowEntry(void)
    : _pos(0), _vstart(-1), _win(NULL), _win_pos(Cursor(0, 0))
{
}

WindowEntry::WindowEntry(WINDOW* w, const Cursor& c)
    : _pos(0), _vstart(-1), _win(w), _win_pos(c)
{
}

WindowEntry::WindowEntry(WINDOW* w, const Cursor& c, string s)
    : _data(s), _pos(0), _vstart(-1), _win(w), _win_pos(c)
{
}

WindowEntry& WindowEntry::operator=(const WindowEntry& rhs)
{
    if (this != &rhs)
    {
        _data = rhs._data;
        _pos = rhs._pos;
        _vstart = rhs._vstart;
        _win = rhs._win;
        _win_pos = rhs._win_pos;
        _default_attrs = rhs._default_attrs;
    }

    return *this;
}

WindowEntry& WindowEntry::operator=(const string& rhs)
{
    if (rhs.empty())
        return *this;

    clear();
    insert(rhs);

    return *this;
}

char& WindowEntry::operator[](size_t position)
{
    if (position >= _data.size())
    {
        // XXX Throw exception
        return _data[0];
    }

    return _data[position];
}

void WindowEntry::set(void)
{
    wmove(_win, _win_pos.row(), _win_pos.col());
}

void WindowEntry::draw(void)
{
    attr_t attrs;
    short opair;

    wattr_get(_win, &attrs, &opair, NULL);

    for (size_t i = 0; i < _data.size(); i++)
    {
        AttrColor ac;
        short cpair = -1;

        ATTR_CALL(_data[i], ac);

        if (ac._color_pair != -1)
            cpair = ac._color_pair;
        else if (_default_attrs._color_pair != -1)
            cpair = _default_attrs._color_pair;

        if (cpair != -1)
            wcolor_set(_win, ac._color_pair, NULL);

        waddch(_win, _data[i] | ac._attrs | _default_attrs._attrs);

        if (cpair != -1)
            wcolor_set(_win, opair, NULL);
    }

    wattr_set(_win, attrs, opair, NULL);
}

void WindowEntry::update(void)
{
    int old_curs = curs_set(0);

    wmove(_win, _win_pos.row(), _win_pos.col() - _pos);
    wclrtoeol(_win);

    draw();

    wmove(_win, _win_pos.row(), _win_pos.col());

    curs_set(old_curs);
}

int WindowEntry::operator>>(int amount)
{
    move(amount);
    return _pos;
}

int WindowEntry::operator<<(int amount)
{
    move(amount * -1);
    return _pos;
}

void WindowEntry::move(int amount)
{
    if (_data.empty())
        return;

    if ((amount < 0) && ((_pos + amount) < 0))
        amount = -1 * _pos;
    else if ((_pos + amount) > (int)_data.size())
        amount = _data.size() - _pos;

    if (amount == 0)
        return;

    _win_pos >> amount;
    _pos += amount;

    wmove(_win, _win_pos.row(), _win_pos.col());
}

void WindowEntry::mpos(int pos)
{
    move(pos - _pos);
}

void WindowEntry::mbegin(void)
{
    move(0 - _pos);
}

void WindowEntry::mend(void)
{
    move(_data.size() - _pos);
}

void WindowEntry::insert(char ch)
{
    _data.insert(_data.begin() + _pos, ch);

    update();
}

void WindowEntry::insert(const string& s)
{
    if (s.empty())
        return;

    _data.insert(_data.begin() + _pos, s.begin(), s.end());

    _pos += (s.size() - 1);
    _win_pos >> (s.size() - 1);

    update();
}

void WindowEntry::remove(int count) 
{
    if ((count == 0) || _data.empty() || (_pos == (int)_data.size()))
        return;

    if (count > 0)
    {
        _data.erase(_data.begin() + _pos, _data.begin() + (_pos + count));
    }
    else
    {
        _data.erase(_data.begin() + ((_pos + 1) + count), _data.begin() + (_pos + 1));
        _pos += (count + 1);
        _win_pos >> (count + 1);
    }

    update();
}

void WindowEntry::remove(void)
{
    remove(1);
}

void WindowEntry::replace(char ch)
{
    if (_data.empty() || (_pos == (int)_data.size()))
        return;

    _data[_pos] = ch;

    update();
}

void WindowEntry::replace(const string& s)
{
    if (_data.empty() || (_pos == (int)_data.size()))
        return;

    int rcount = s.size();
    if (rcount > ((int)_data.size() - _pos))
        rcount = _data.size() - _pos;

    for (int i = 0; i < rcount; i++)
    {
        _data[_pos] = s[i];
        move(1);
    }

    if (rcount < (int)s.size())
        insert(string(s.begin() + rcount, s.end()));
    else
        update();
}

void WindowEntry::clear(void)
{
    if (_data.empty())
        return;

    mbegin();
    remove(_data.size());
}

void WindowEntry::pclear(void)
{
    if (_data.empty() || (_pos == (int)_data.size()))
        return;

    remove(_data.size() - _pos);
}

void WindowEntry::vstart(void)
{
    _vstart = _pos;
}

void WindowEntry::vstop(void)
{
    if (_vstart == -1)
        return;

    int spos = _pos;

    vmove(_vstart - _pos);

    _win_pos >> (spos - _vstart);
    _pos = spos;

    _vstart = -1;

    wmove(_win, _win_pos.row(), _win_pos.col());
}

void WindowEntry::vhighlight(void)
{
    chtype ch = mvwinch(_win, _win_pos.row(), _win_pos.col());
    attr_t attrs = ch & A_ATTRIBUTES;
    short color = PAIR_NUMBER(attrs);

    attrs ^= A_REVERSE;
    //attrs ^= A_STANDOUT;

    mvwchgat(_win, _win_pos.row(), _win_pos.col(), 1, attrs, color, NULL);
}

void WindowEntry::vmove(int amount)
{
    if (_vstart == -1)
        return;

    if (_data.empty())
        return;

    if ((_pos + amount) < 0)
        amount = -1 * _pos;
    else if ((_pos + amount) >= (int)_data.size())
        amount = (_data.size() - 1) - _pos;

    if (amount == 0)
        return;

    // moving toward _vstart    : move change
    // moving away from _vstart : change move
    int inc = (amount > 0) ? 1 : -1;

    // Moving toward _vstart
    if ((((_vstart - _pos) < 0) && (inc < 0))
            || (((_vstart - _pos) > 0) && (inc > 0)))
    {
        while (amount && (_pos != _vstart))
        {
            _pos += inc;
            _win_pos >> inc;

            vhighlight();

            amount -= inc;
        }
    }

    // Moving away from _vstart
    while (amount)
    {
        vhighlight();

        _pos += inc;
        _win_pos >> inc;

        amount -= inc;
    }

    wmove(_win, _win_pos.row(), _win_pos.col());
}

void WindowEntry::vmpos(int pos)
{
    vmove(pos - _pos);
}

void WindowEntry::vmbegin(void)
{
    vmove(0 - _pos);
}

void WindowEntry::vmend(void)
{
    vmove(_data.size() - _pos - 1);
}

void WindowEntry::vtoggle(void)
{
    if (_pos == _vstart)
        return;

    int tpos = _pos;

    vhighlight();

    _win_pos >> (_vstart - _pos);
    _pos = _vstart;

    vhighlight();

    _vstart = tpos;
}

void WindowEntry::vremove(void)
{
    if (_vstart > _pos)
        remove((_vstart - _pos) + 1);
    else
        remove((_vstart - (_pos + 1)));

    _vstart = -1;
}

void WindowEntry::vreplace(char ch)
{
    if (_vstart > _pos)
    {
        for (int i = _pos; i <= _vstart; i++)
            _data[i] = ch;
    }
    else
    {
        for (int i = _vstart; i <= _pos; i++)
            _data[i] = ch;
    }

    mpos(_vstart);
    _vstart = -1;

    update();
}

void WindowEntry::vreplace(const string& s)
{
    vremove();
    insert(s);
    _pos--;
    _win_pos << 1;
    set();
}

void WindowEntry::vclear(void)
{
    if (_data.empty())
        return;

    vstop();
    clear();
}

string WindowEntry::data(void)
{
    return _data;
}

string WindowEntry::pdata(void)
{
    return string(_data.begin() + _pos, _data.end());
}

string WindowEntry::vdata(void)
{
    string s;

    if (_pos > _vstart)
    {
        for (int i = _vstart; i <= _pos; i++)
            s.push_back(_data[i]);
    }
    else
    {
        for (int i = _pos; i <= _vstart; i++)
            s.push_back(_data[i]);
    }

    return s;
}

int WindowEntry::position(void)
{
    return _pos;
}

size_t WindowEntry::length(void)
{
    return _data.size();
}

size_t WindowEntry::size(void)
{
    return _data.size();
}

bool WindowEntry::empty(void)
{
    return _data.size() == 0;
}

Cursor& WindowEntry::cursor(void)
{
    return _win_pos;
}

size_t WindowEntry::width(void)
{
    return _data.size();
}

void WindowEntry::width(size_t w)
{
    if (w <= _data.size())
        return;

    size_t append = w - _data.size();
    for (size_t i = 0; i < append; i++)
        _data.push_back(' ');
}

void WindowEntry::get_attrs(int ch, AttrColor& ac)
{
}

string WindowEntry::info(void)
{
    string s("Window entry:\n");
    s += "  position: " + to_string(_pos)
        + ", row: " + to_string(_win_pos.row())
        + ", col: " + to_string(_win_pos.col())
        + ", width: " + to_string(_data.size())
        + "\n  Data: " + _data;
    return s;
}

HeaderEntry::HeaderEntry(void)
    : WindowEntry()
{
}

HeaderEntry::HeaderEntry(WINDOW* w, const Cursor& c)
    : WindowEntry(w, c)
{
}

HeaderEntry::HeaderEntry(WINDOW* w, const Cursor& c, string s)
    : WindowEntry(w, c, s)
{
}

HeaderEntry& HeaderEntry::operator=(const HeaderEntry& rhs)
{
    if (this != &rhs)
        WindowEntry::operator=(rhs);

    return *this;
}

HeaderEntry& HeaderEntry::operator=(const string& rhs)
{
    if (rhs.empty())
        return *this;

    clear();
    insert(rhs);

    return *this;
}

void HeaderEntry::get_attrs(int ch, AttrColor& ac)
{
    ac._attrs |= A_BOLD;
}

string HeaderEntry::info(void)
{
    string s("Header entry:\n");
    s += "  position: " + to_string(_pos)
        + ", row: " + to_string(_win_pos.row())
        + ", col: " + to_string(_win_pos.col())
        + "\n  Data: " + _data;
    return s;
}

EvalEntry::EvalEntry(void)
    : WindowEntry()
{
}

EvalEntry::EvalEntry(WINDOW* w, const Cursor& c)
    : WindowEntry(w, c)
{
}

EvalEntry::EvalEntry(WINDOW* w, const Cursor& c, string s)
    : WindowEntry(w, c, s)
{
}

EvalEntry& EvalEntry::operator=(const EvalEntry& rhs)
{
    if (this != &rhs)
        WindowEntry::operator=(rhs);

    return *this;
}

EvalEntry& EvalEntry::operator=(const string& rhs)
{
    if (rhs.empty())
        return *this;

    clear();
    insert(rhs);

    return *this;
}

bool eval_special(int ch)
{
    switch (ch)
    {
        case '(':
        case ')':
        case 'e':
            return true;

        default:
            break;
    }

    return false;
}

#if 0
void EvalEntry::draw(void)
{
    for (size_t i = 0; i < _data.size(); i++)
    {
        if (eval_special(_data[i]))
            waddch(_win, (chtype)_data[i] | A_BOLD);
        else
            waddch(_win, _data[i]);
    }
}
#endif

void EvalEntry::get_attrs(int ch, AttrColor& ac)
{
#if 0
    static ColorPair ecp(COLOR_RED, COLOR_WHITE);

    attr_t attrs;
    short pair, fg, bg;

    wattr_get(_win, &attrs, &pair, NULL);
    pair_content(pair, &fg, &bg);
    if (ecp.bg() != bg)
        ecp.bg(bg);

    if (eval_special(ch))
    {
        ac._attrs |= A_BOLD;
    }
    else if (isdigit(ch))
    {
        ac._attrs |= A_BOLD;
        ac._color_pair = ecp.pair_number();
    }
#endif
    if (eval_special(ch))
        ac._attrs |= A_BOLD;
}

string EvalEntry::info(void)
{
    string s("Eval entry:\n");
    s += "  position: " + to_string(_pos)
        + ", row: " + to_string(_win_pos.row())
        + ", col: " + to_string(_win_pos.col())
        + "\n  Data: " + _data;
    return s;
}

MatrixEntry::MatrixEntry(void)
    : WindowEntry(), _width(1), _mep(MEP_ONE)
{
    _win_pos = Cursor(0, _width + 2);
}

MatrixEntry::MatrixEntry(WINDOW* w, const Cursor& c, MatrixEntryPosition mep)
    : WindowEntry(w, c), _width(1), _mep(mep)
{
    _win_pos >> (_width + 2);
}

MatrixEntry::MatrixEntry(WINDOW* w, const Cursor& c, size_t width, MatrixEntryPosition mep)
    : WindowEntry(w, c), _width(width), _mep(mep)
{
    if (_width == 0)
        _width = 1;

    _win_pos >> (_width + 2);
}

MatrixEntry::MatrixEntry(WINDOW* w, const Cursor& c, string s, MatrixEntryPosition mep)
    : WindowEntry(w, c, s), _mep(mep)
{
    if (_data.empty())
        _width = 1;
    else
        _width = _data.size();

    _win_pos >> (_width + 2);
    //_win_pos >> 2;
}

MatrixEntry::MatrixEntry(WINDOW* w, const Cursor& c, size_t width, string s, MatrixEntryPosition mep)
    : WindowEntry(w, c, s), _width(width), _mep(mep)
{
    if (_data.empty() && (_width == 0))
        _width = 1;
    else if (_width < _data.size())
        _width = _data.size();

    _win_pos >> (_width + 2);
    //_win_pos >> ((_width - _data.size()) + 2);
}

MatrixEntry& MatrixEntry::operator=(const MatrixEntry& rhs)
{
    if (this != &rhs)
    {
        WindowEntry::operator=(rhs);
        _width = rhs._width;
        _mep = rhs._mep;
    }

    return *this;
}

MatrixEntry& MatrixEntry::operator=(const string& rhs)
{
    if (rhs.empty())
        return *this;

    clear();
    insert(rhs);

    return *this;
}

void MatrixEntry::draw(void)
{
    if (_mep == MEP_ONE || _mep == MEP_START)
        waddch(_win, (chtype)'[' | A_BOLD);
    else
        waddch(_win, (chtype)'|' | A_BOLD);

    waddch(_win, ' ');

    for (size_t i = 0; i < (_width - _data.size()); i++)
        waddch(_win, ' ');

    WindowEntry::draw();

    waddch(_win, ' ');

    if (_mep == MEP_ONE || _mep == MEP_END)
        waddch(_win, (chtype)']' | A_BOLD);
}

void MatrixEntry::update(void)
{
    int old_curs = curs_set(0);
    int move_amount = ((_width - _data.size()) + _pos) + 2;

    wmove(_win, _win_pos.row(), _win_pos.col() - move_amount);

    draw();

    wmove(_win, _win_pos.row(), _win_pos.col());

    curs_set(old_curs);
}

#if 0
void MatrixEntry::update(void)
{
    int old_curs = curs_set(0);
    int move_amount = ((_width - _data.size()) + _pos);

    wmove(_win, _win_pos.row(), _win_pos.col() - move_amount);

    draw();

    wmove(_win, _win_pos.row(), _win_pos.col());

    curs_set(old_curs);
}
#endif

void MatrixEntry::insert(char ch)
{
    _data.insert(_data.begin() + _pos, ch);
    _pos++;

    if (_data.size() > _width)
        _width++;

    update();
}

void MatrixEntry::insert(const string& s)
{
    if (s.empty())
        return;

    _data.insert(_data.begin() + _pos, s.begin(), s.end());

    _pos += s.size();

    if (_data.size() > _width)
        _width = _data.size();

    update();
}

void MatrixEntry::remove(int count) 
{
    if ((count == 0) || _data.empty() || (_pos == (int)_data.size()))
        return;

    if (count > 0)
    {
        _data.erase(_data.begin() + _pos, _data.begin() + (_pos + count));
        _win_pos >> count;
    }
    else
    {
        _data.erase(_data.begin() + ((_pos + 1) + count), _data.begin() + (_pos + 1));
        _pos += (count + 1);
        _win_pos >> 1;
    }

    update();
}

void MatrixEntry::remove(void)
{
    remove(1);
}

size_t MatrixEntry::width(void)
{
    return _width;
}

void MatrixEntry::width(size_t w)
{
    if (w < _data.size())
        return;

    _width = w;
}

bool matrix_special(int ch)
{
    switch (ch)
    {
        case '[':
        case ']':
        case '(':
        case ')':
        case '|':
        case '=':
        case 'e':
            return true;

        default:
            break;
    }

    return false;
}

void MatrixEntry::get_attrs(int ch, AttrColor& ac)
{
#if 0
    static ColorPair mcp(COLOR_RED, COLOR_WHITE);

    attr_t attrs;
    short pair, fg, bg;

    wattr_get(_win, &attrs, &pair, NULL);
    pair_content(pair, &fg, &bg);
    if (mcp.bg() != bg)
        mcp.bg(bg);

    if (matrix_special(ch))
    {
        ac._attrs |= A_BOLD;
    }
    else if (isdigit(ch))
    {
        ac._attrs |= A_BOLD;
        ac._color_pair = mcp.pair_number();
    }
#endif

    if (matrix_special(ch))
        ac._attrs |= A_BOLD;
}

string MatrixEntry::info(void)
{
    string s("Matrix entry:\n");
    s += "  position: " + to_string(_pos)
        + ", row: " + to_string(_win_pos.row())
        + ", col: " + to_string(_win_pos.col())
        + ", width: " + to_string(_width)
        + "\n  Data: " + _data;
    return s;
}

WindowPane::WindowPane(MatrixEditor* me, WINDOW* w)
    : _me(me), _win(w), _color_pair(ColorPair(COLOR_BLACK, COLOR_WHITE)),
    _clear_toggle(1), _yank_toggle(1)
{
    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));
}

WindowPane::WindowPane(const WindowPane& rhs)
    : _me(rhs._me), _win(rhs._win), _color_pair(rhs._color_pair),
    _clear_toggle(rhs._clear_toggle), _yank_toggle(rhs._yank_toggle),
    _replaced(rhs._replaced)
{
    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));
}

WindowPane& WindowPane::operator=(const WindowPane& rhs)
{
    if (this == &rhs)
        return *this;

    _me = rhs._me;
    _win = rhs._win;
    _color_pair = rhs._color_pair;
    _clear_toggle = rhs._clear_toggle;
    _yank_toggle = rhs._yank_toggle;
    _replaced = rhs._replaced;

    return *this;
}

WINDOW* WindowPane::window(void)
{
    return _win;
}

ColorPair& WindowPane::color(void)
{
    return _color_pair;
}

WindowPane::KeyAction WindowPane::key_action(int ch, EditorMode m)
{
    if (_key_mode_actions.find(ch) == _key_mode_actions.end())
    {
        if ((m == MODE_INSERT) && is_print(ch))
            return &WindowPane::insert;
        else if ((m == MODE_REPLACE) && is_print(ch))
            return &WindowPane::replace;
    }
    else
    {
        switch (m)
        {
            case MODE_INSERT:
                return _key_mode_actions[ch].insert_mode_action;
            case MODE_EDIT:
                return _key_mode_actions[ch].edit_mode_action;
            case MODE_VISUAL:
                return _key_mode_actions[ch].visual_mode_action;
            case MODE_REPLACE:
                return _key_mode_actions[ch].replace_mode_action;
        }
    }

    return &WindowPane::no_op;
}

void WindowPane::we_addstr(string s)
{
    for (size_t i = 0; i < s.size(); i++)
        waddch(_win, s[i]);
}

bool WindowPane::is_print(int ch)
{
    return (isgraph(ch) || isspace(ch));
}

void WindowPane::clear(void)
{
    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));
    wclear(_win);
}

void WindowPane::redraw(void)
{
    this->refresh();
}

void WindowPane::refresh(void)
{
    wrefresh(_win);
}

void WindowPane::insert_mode(int ch)
{
    _me->set_mode(MODE_INSERT);
}

void WindowPane::edit_mode(int ch)
{
    _me->set_mode(MODE_EDIT);
}

void WindowPane::visual_mode(int ch)
{
    _me->set_mode(MODE_VISUAL);
}

void WindowPane::replace_mode(int ch)
{
    _replaced.clear();
    _me->set_mode(MODE_REPLACE);
}

void WindowPane::command_mode(int ch)
{
    _me->command(ch);
}

void WindowPane::cancel(int ch)
{
    _clear_toggle = 1;
    _yank_toggle = 1;

    _me->set_mode(MODE_EDIT);
}

void WindowPane::visual_cancel(int ch)
{
    _me->set_mode(MODE_EDIT);
}

void WindowPane::replace_cancel(int ch)
{
    _replaced.clear();
    _me->set_mode(MODE_EDIT);
}

void WindowPane::move(int ch)
{
}

void WindowPane::insert(int ch)
{
}

void WindowPane::remove(int ch)
{
}

void WindowPane::enter(int ch)
{
}

void WindowPane::replace(int ch)
{
}
void WindowPane::clear(int ch)
{
}
void WindowPane::clear_to_eol(int ch)
{
}
void WindowPane::yank(int ch)
{
}
void WindowPane::paste(int ch)
{
}

void WindowPane::replace_move(int ch)
{
}

void WindowPane::visual_move(int ch)
{
}
void WindowPane::visual_remove(int ch)
{
}
void WindowPane::visual_replace(int ch)
{
}
void WindowPane::visual_clear(int ch)
{
}
void WindowPane::visual_yank(int ch)
{
}
void WindowPane::visual_paste(int ch)
{
}
void WindowPane::visual_toggle_position(int ch)
{
}

MatrixPane::MatrixPane(size_t n, MatrixEditor* me, WINDOW* w)
    : WindowPane(me, w), _extra_data(0), _mp(MP_COEFFICIENT),
    _n(n), _c_i(1), _c_j(1), _s_i(1), _v_i(1), _e_i(0),
    _c_col_width(10), _s_col_width(3), _v_col_width(3), _cur_hist_entry(0)
{
    _c_matrix.resize(_n);

    size_t win_col = 0;
    for (size_t i = 0; i < _n; i++)
    {
        _c_matrix[i].resize(_n);

        win_col = 0;
        for (size_t j = 0; j < _n; j++)
        {
            MatrixEntry::MatrixEntryPosition mep;

            //win_col = (j + 1) * (_col_spacing + _pad + _c_col_width) + j;
            //win_col = j + _c_col_width;

            if (j == 0)
                mep = MatrixEntry::MEP_START;
            else if (j < (_n - 1))
                mep = MatrixEntry::MEP_MIDDLE;
            else
                mep = MatrixEntry::MEP_END;

            _c_matrix[i][j] = MatrixEntry(_win, Cursor(i+_header_rows, win_col), _c_col_width, mep);

            win_col += _c_matrix[i][j].width() + 3;
        }
    }

    _s_vector.resize(_n);
    //win_col += _col_spacing + _vector_spacing + _col_spacing + _pad + _s_col_width + 1;
    win_col += _vector_spacing + 1;
    for (size_t i = 0; i < _n; i++)
        _s_vector[i] = MatrixEntry(_win, Cursor(i+_header_rows, win_col), _s_col_width);

    _v_vector.resize(_n);
    //win_col += _col_spacing + _vector_spacing + _col_spacing + _pad + _v_col_width + 1;
    win_col += _s_vector[0].width() + 3 + _vector_spacing + 1;
    for (size_t i = 0; i < _n; i++)
        _v_vector[i] = MatrixEntry(_win, Cursor(i+_header_rows, win_col), _v_col_width);
}

#if 0
MatrixPane::MatrixPane(size_t n, MatrixEditor* me, WINDOW* w)
    : WindowPane(me, w), _extra_data(0), _mp(MP_COEFFICIENT),
    _n(n), _c_i(1), _c_j(1), _s_i(1), _v_i(1), _e_i(0),
    _c_col_width(10), _s_col_width(3), _v_col_width(3), _cur_hist_entry(0)
{
    _c_matrix.resize(_n);

    size_t win_col;
    for (size_t i = 0; i < _n; i++)
    {
        _c_matrix[i].resize(_n);

        for (size_t j = 0; j < _n; j++)
        {
            win_col = (j + 1) * (_col_spacing + _pad + _c_col_width) + j;
            _c_matrix[i][j] = MatrixEntry(_win, Cursor(i+_header_rows, win_col), _c_col_width);
        }
    }

    _s_vector.resize(_n);
    win_col += _col_spacing + _vector_spacing + _col_spacing + _pad + _s_col_width + 1;
    for (size_t i = 0; i < _n; i++)
        _s_vector[i] = MatrixEntry(_win, Cursor(i+_header_rows, win_col), _s_col_width);

    _v_vector.resize(_n);
    win_col += _col_spacing + _vector_spacing + _col_spacing + _pad + _v_col_width + 1;
    for (size_t i = 0; i < _n; i++)
        _v_vector[i] = MatrixEntry(_win, Cursor(i+_header_rows, win_col), _v_col_width);
}
#endif

MatrixPane::MatrixPane(const MatrixPane& rhs)
    : WindowPane(rhs),
    _c_matrix(rhs._c_matrix), _s_vector(rhs._s_vector), _v_vector(rhs._v_vector), _mp(rhs._mp),
    _n(rhs._n), _c_i(rhs._c_i), _c_j(rhs._c_j), _s_i(rhs._s_i), _v_i(rhs._v_i), _e_i(rhs._e_i),
    _c_col_width(rhs._c_col_width), _s_col_width(rhs._s_col_width), _v_col_width(rhs._v_col_width),
    _pad(rhs._pad), _col_spacing(rhs._col_spacing), _vector_spacing(rhs._vector_spacing),
    _header_rows(rhs._header_rows), _history(rhs._history), _cur_hist_entry(rhs._cur_hist_entry),
    _valid_screen_chars(rhs._valid_screen_chars), _key_mode_actions(rhs._key_mode_actions)
{
    auto it = _extra_data.begin();
    while (it != _extra_data.end())
    {
        delete *it;
        it = _extra_data.erase(it);
    }

    for (auto r_it = rhs._extra_data.begin(); r_it != rhs._extra_data.end(); ++r_it)
        _extra_data.push_back(new WindowEntry(**r_it));
}

MatrixPane& MatrixPane::operator=(const MatrixPane& rhs)
{
    if (this == &rhs)
        return *this;

    WindowPane::operator=(rhs);

    _c_matrix = rhs._c_matrix;
    _s_vector = rhs._s_vector;
    _v_vector = rhs._v_vector;

    _mp = rhs._mp;

    _n = rhs._n;
    _c_i = rhs._c_i;
    _c_j = rhs._c_j;
    _s_i = rhs._s_i;
    _v_i = rhs._v_i;
    _e_i = rhs._e_i;

    _c_col_width = rhs._c_col_width;
    _s_col_width = rhs._s_col_width;
    _v_col_width = rhs._v_col_width;

    _pad = rhs._pad;
    _col_spacing = rhs._col_spacing;
    _vector_spacing = rhs._vector_spacing;
    _header_rows = rhs._header_rows;
    _history = rhs._history;
    _cur_hist_entry = rhs._cur_hist_entry;

    _valid_screen_chars = rhs._valid_screen_chars;
    _key_mode_actions = rhs._key_mode_actions;

    auto it = _extra_data.begin();
    while (it != _extra_data.end())
    {
        delete *it;
        it = _extra_data.erase(it);
    }

    for (auto r_it = rhs._extra_data.begin(); r_it != rhs._extra_data.end(); ++r_it)
        _extra_data.push_back(new WindowEntry(**r_it));

    return *this;
}

MatrixPane::~MatrixPane(void)
{
    auto it = _extra_data.begin();
    while (it != _extra_data.end())
    {
        delete *it;
        it = _extra_data.erase(it);
    }
}

bool MatrixPane::is_print(int ch)
{
    size_t i;
    for (i = 0; i < _valid_screen_chars.size(); i++)
    {
        if (_valid_screen_chars[i] == ch)
            break;
    }

    return (i != _valid_screen_chars.size());
}

WindowPane::KeyAction MatrixPane::key_action(int ch, EditorMode m)
{
    if (_key_mode_actions.find(ch) == _key_mode_actions.end())
    {
        if ((m == MODE_INSERT) && is_print(ch))
            return &WindowPane::insert;
        else if ((m == MODE_REPLACE) && is_print(ch))
            return &WindowPane::replace;
    }
    else
    {
        switch (m)
        {
            case MODE_INSERT:
                return _key_mode_actions[ch].insert_mode_action;
            case MODE_EDIT:
                return _key_mode_actions[ch].edit_mode_action;
            case MODE_VISUAL:
                return _key_mode_actions[ch].visual_mode_action;
            case MODE_REPLACE:
                return _key_mode_actions[ch].replace_mode_action;
        }
    }

    return &WindowPane::no_op;
}

void MatrixPane::draw(void)
{
    size_t erow = (_n - 1) / 2;
    size_t ecol = _vector_spacing / 2;
    int old_curs = curs_set(0);
    int y, x;
    getyx(_win, y, x);

    wmove(_win, 0, 0);
    wattr_on(_win, A_BOLD, NULL);
    wprintw(_win, "[ Coefficient Matrix ]-1 x [ Constants Vector ] = [ Unknowns Vector ]\n");
    wattr_off(_win, A_BOLD, NULL);

    for (size_t i = 0; i < _n; i++)
    {
        wmove(_win, i+_header_rows, 0);

        //waddch(_win, (chtype)'[' | A_BOLD);
        //waddch(_win, ' ');
        for (size_t j = 0; j < _n; j++)
        {
            _c_matrix[i][j].draw();

            if (j != (_n - 1))
            {
                //waddch(_win, ' ');
                //waddch(_win, (chtype)'|' | A_BOLD);
                //waddch(_win, ' ');
            }
        }
        //waddch(_win, ' ');
        //waddch(_win, (chtype)']' | A_BOLD);

        size_t k = 0;
        if (i == 0)
        {
            wattr_on(_win, A_BOLD, NULL);
            wprintw(_win, "-1");
            wattr_off(_win, A_BOLD, NULL);
            k = 2;
        }

        for (; k < _vector_spacing; k++)
            waddch(_win, ' ');

        //waddch(_win, (chtype)'[' | A_BOLD);
        //waddch(_win, ' ');

        _s_vector[i].draw();

        //waddch(_win, ' ');
        //waddch(_win, (chtype)']' | A_BOLD);

        for (k = 0; k < _vector_spacing; k++)
        {
            if ((i == erow) && (k == ecol))
                waddch(_win, (chtype)'=' | A_BOLD);
            else
                waddch(_win, ' ');
        }

        //waddch(_win, (chtype)'[' | A_BOLD);
        //waddch(_win, ' ');

        _v_vector[i].draw();

        //waddch(_win, ' ');
        //waddch(_win, (chtype)']' | A_BOLD);
    }

    _e_i = 0;
    auto it = _extra_data.begin();
    while (it != _extra_data.end())
    {
        delete *it;
        it = _extra_data.erase(it);
    }

    wclrtobot(_win);

    curs_set(old_curs);
    wmove(_win, y, x);
}

WindowEntry& MatrixPane::entry(size_t row, size_t col = 1)
{
    static WindowEntry not_found(_win, Cursor(1000, 1000));

    switch (_mp)
    {
        case MP_COEFFICIENT:
            if (row > _n)
                row = _n;

            if (col > _n)
                col = _n;
            else if (col == 0)
                col = 1;

            return _c_matrix[row-1][col-1];

        case MP_CONSTANTS:
            if (row > _n)
                row = _n;

            return _s_vector[row-1];

        case MP_UNKNOWNS:
            if (row > _n)
                row = _n;

            return _v_vector[row-1];

        case MP_EXTRA_DATA:
            if (!_extra_data.empty())
            {
                if (row > _extra_data.size())
                    row = _extra_data.size();
                else if (row == 0)
                    row = 1;

                return *_extra_data[row-1];
            }
    }

    return not_found;
}

WindowEntry& MatrixPane::entry(void)
{
    static WindowEntry not_found(_win, Cursor(1000, 1000));

    switch (_mp)
    {
        case MP_COEFFICIENT:
            return entry(_c_i, _c_j);

        case MP_CONSTANTS:
            return entry(_s_i);

        case MP_UNKNOWNS:
            return entry(_v_i);

        case MP_EXTRA_DATA:
            return entry(_e_i);
    }

    return not_found;
}

bool MatrixPane::adjust(size_t width)
{
    size_t inc;

    switch (_mp)
    {
        case MP_COEFFICIENT:
            if (width <= _c_col_width)
                return false;

            inc = width - _c_col_width;
            _c_col_width = width;

            for (size_t i = 0; i < _n; i++)
            {
                for (size_t j = 0; j < _n; j++)
                {
                    MatrixEntry& me = _c_matrix[i][j];

                    me.width(width);
                    me.cursor() >> (inc * (j+1));
                }
            }

            inc *= _n;
            for (size_t i = 0; i < _n; i++)
                _s_vector[i].cursor() >> inc;

            for (size_t i = 0; i < _n; i++)
                _v_vector[i].cursor() >> inc;

            break;

        case MP_CONSTANTS:
            if (width <= _s_col_width)
                return false;

            inc = width - _s_col_width;
            _s_col_width = width;

            for (size_t i = 0; i < _n; i++)
            {
                MatrixEntry& me = _s_vector[i];

                me.width(width);
                me.cursor() >> inc;
            }

            for (size_t i = 0; i < _n; i++)
                _v_vector[i].cursor() >> inc;

            break;

        case MP_UNKNOWNS:
            if (width <= _v_col_width)
                return false;

            inc = width - _v_col_width;
            _v_col_width = width;

            for (size_t i = 0; i < _n; i++)
            {
                MatrixEntry& me = _v_vector[i];

                me.width(width);
                me.cursor() >> inc;
            }

            break;

        case MP_EXTRA_DATA:
            return false;
    }

    return true;
}

void MatrixPane::set(void)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            entry(_c_i, _c_j).set();
            break;
        case MP_CONSTANTS:
            entry(_s_i).set();
            break;
        case MP_UNKNOWNS:
            entry(_v_i).set();
            break;
        case MP_EXTRA_DATA:
            entry(_e_i).set();
            break;
    }
}

void MatrixPane::redraw(void)
{
    WindowPane::clear();

    draw();
    set();

    WindowPane::refresh();
}

void MatrixPane::refresh(void)
{
    if (adjust(entry().width()))
        draw();

    set();

    WindowPane::refresh();
}

void MatrixPane::insert_mode(int ch)
{
    WindowPane::insert_mode(ch);

    if (ch == 'a')
        right();
}

void MatrixPane::edit_mode(int ch)
{
    WindowPane::edit_mode(ch);

    entry() << 1;
}

void MatrixPane::visual_mode(int ch)
{
    WindowPane::visual_mode(ch);

    entry().vstart();
}

void MatrixPane::replace_mode(int ch)
{
    if (ch != 'r')
    {
        WindowPane::replace_mode(ch);
        return;
    }

    while ((ch = wgetch(_win)) != 0)
    {
        if (ch == -1)
            break;

        if (ch == K_ESCAPE)
            break;

        if (is_print(ch))
        {
            entry().replace(ch);
            break;
        }
    }
}

void MatrixPane::insert(int ch)
{
    WindowEntry& e(entry());
    e.insert(ch);
}

void MatrixPane::remove(int ch)
{
    WindowEntry& e(entry());

    if ((ch == KEY_DC) || (ch == 'x'))
    {
        e.remove();
    }
    else
    {
        if (e.position() != 0)
        {
            e << 1;
            e.remove();
        }
    }
}

void MatrixPane::replace(int ch)
{
    WindowEntry& e(entry());

    _replaced.push_back(e[e.position()]);
    e.replace(ch);
    e >> 1;
}

void MatrixPane::replace_move(int ch)
{
    WindowEntry& e(entry());

    switch (ch)
    {
        case KEY_BACKSPACE:
        case K_BACKSPACE1:
        case K_BACKSPACE2:
            left();
            if (!_replaced.empty())
            {
                e.replace(_replaced.back());
                _replaced.pop_back();
            }
            break;

        case KEY_LEFT:
            left();
            _replaced.clear();
            break;

        case KEY_RIGHT:
            right();
            _replaced.clear();
            break;
    }
}

void MatrixPane::clear_to_eol(int ch)
{
    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.pdata());

    e.pclear();
    e << 1;
}

void MatrixPane::clear(int ch)
{
    _clear_toggle ^= 1;

    if (!_clear_toggle)
        return;

    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.data());

    e.clear();
}

void MatrixPane::next(void)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            if ((_c_i == _n) && (_c_j == _n))
            {
                _mp = MP_CONSTANTS;
                _s_i = 1;
            }
            else if (_c_j == _n)
            {
                _c_i++;
                _c_j = 1;
            }
            else
            {
                _c_j++;
            }
            break;

        case MP_CONSTANTS:
            if (_s_i == _n)
            {
                if (_v_vector[0].empty())
                {
                    _mp = MP_COEFFICIENT;
                    _c_i = _c_j = 1;
                }
                else
                {
                    _mp = MP_UNKNOWNS;
                    _v_i = 1;
                }
            }
            else
            {
                _s_i++;
            }
            break;

        case MP_UNKNOWNS:
            if (_v_i == _n)
            {
                if (_extra_data.empty())
                {
                    _mp = MP_COEFFICIENT;
                    _c_i = _c_j = 1;
                }
                else
                {
                    _mp = MP_EXTRA_DATA;
                    _e_i = 1;
                }
            }
            else
            {
                _v_i++;
            }
            break;

        case MP_EXTRA_DATA:
            if (_e_i == _extra_data.size())
            {
                _mp = MP_COEFFICIENT;
                _c_i = _c_j = 1;
            }
            else
            {
                _e_i++;
            }
            break;
    }

    set();
}

void MatrixPane::prev(void)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            if ((_c_i == 1) && (_c_j == 1))
            {
                if (_extra_data.empty())
                {
                    if (_v_vector[0].empty())
                    {
                        _mp = MP_CONSTANTS;
                        _s_i = _n;
                    }
                    else
                    {
                        _mp = MP_UNKNOWNS;
                        _v_i = _n;
                    }
                }
                else
                {
                    _mp = MP_EXTRA_DATA;
                    _e_i = _extra_data.size();
                }
            }
            else if (_c_j == 1)
            {
                _c_i--;
                _c_j = _n;
            }
            else
            {
                _c_j--;
            }
            break;

        case MP_CONSTANTS:
            if (_s_i == 1)
            {
                _mp = MP_COEFFICIENT;
                _c_i = _c_j = _n;
            }
            else
            {
                _s_i--;
            }
            break;

        case MP_UNKNOWNS:
            if (_v_i == 1)
            {
                _mp = MP_CONSTANTS;
                _s_i = _n;
            }
            else
            {
                _v_i--;
            }
            break;

        case MP_EXTRA_DATA:
            if (_e_i == 1)
            {
                _mp = MP_UNKNOWNS;
                _v_i = _n;
            }
            else
            {
                _e_i--;
            }
            break;
    }

    set();
}

void MatrixPane::down(void)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            if (_c_i < _n)
                _c_i++;
            break;
        case MP_CONSTANTS:
            if (_s_i < _n)
                _s_i++;
            break;
        case MP_UNKNOWNS:
            if (_v_i < _n)
                _v_i++;
            break;
        case MP_EXTRA_DATA:
            if (_e_i < _n)
                _e_i++;
            break;
    }

    set();
}

void MatrixPane::up(void)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            if (_c_i > 1)
                _c_i--;
            break;
        case MP_CONSTANTS:
            if (_s_i > 1)
                _s_i--;
            break;
        case MP_UNKNOWNS:
            if (_v_i > 1)
                _v_i--;
            break;
        case MP_EXTRA_DATA:
            if (_e_i > 1)
                _e_i--;
            break;
    }

    set();
}

void MatrixPane::first(void)
{
    _mp = MP_COEFFICIENT;
    _c_i = _c_j = 1;

    set();
}

void MatrixPane::last(void)
{
    if (_extra_data.empty())
    {
        if (_v_vector[0].empty())
        {
            _mp = MP_CONSTANTS;
            _s_i = _n;
        }
        else
        {
            _mp = MP_UNKNOWNS;
            _v_i = _n;
        }
    }
    else
    {
        _mp = MP_EXTRA_DATA;
        _e_i = _extra_data.size();
    }

    set();
}

void MatrixPane::left(void)
{
    entry() << 1;
}

void MatrixPane::right(void)
{
    WindowEntry& e(entry());

    if ((_me->mode() != MODE_INSERT)
            && (e.position() == ((int)e.size() - 1)))
        return;

    e >> 1;
}

void MatrixPane::begin(void)
{
    entry().mbegin();
}

void MatrixPane::end(void)
{
    WindowEntry& e(entry());

    e.mend();

    if ((_me->mode() != MODE_INSERT)
            && (e.position() == (int)e.size()))
        e << 1;
}

void MatrixPane::move(int ch)
{
    switch (ch)
    {
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case K_BACKSPACE1:
        case K_BACKSPACE2:
        case 'h':
            left();
            break;

        case KEY_RIGHT:
        case 'l':
            right();
            break;

        case K_TAB:
        case 'n':
            next();
            break;

        case KEY_BTAB:
        case 'b':
            prev();
            break;

        case 'j':
        case '+':
            down();
            break;

        case 'k':
        case '-':
            up();
            break;

        case K_NEWLINE:
        case KEY_ENTER:
            break;

        case KEY_HOME:
        case '^':
        case '_':
            begin();
            break;

        case '$':
            end();
            break;

        case 'g':
            first();
            break;

        case 'G':
            last();
            break;

        case KEY_UP:
            if (_history.empty())
                break;

            if (_cur_hist_entry >= 1)
                entry() = _history[--_cur_hist_entry];

            break;

        case KEY_DOWN:
            if (_history.empty())
                break;

            if (_cur_hist_entry < (_history.size() - 1))
                entry() = _history[++_cur_hist_entry];

            break;

        case 'e':
            if (_history.empty())
                break;

            _history.erase(_history.begin() + _cur_hist_entry);

            if (_history.empty())
            {
                entry().clear();
                break;
            }

            if (_cur_hist_entry == _history.size())
                _cur_hist_entry--;

            entry() = _history[_cur_hist_entry];

            break;
    }
}

void MatrixPane::yank(int ch)
{
    _yank_toggle ^= 1;

    if (!_yank_toggle)
        return;

    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.data());
}

void MatrixPane::paste(int ch)
{
    if (_me->yanked().empty())
        return;

    WindowEntry& e(entry());

    if (ch == 'p')
        e >> 1;

    e.insert(_me->yanked());

    if ((_me->mode() != MODE_INSERT)
            && (e.position() == (int)e.size()))
        e << 1;
}

void MatrixPane::vleft(void)
{
    entry().vmove(-1);
}

void MatrixPane::vright(void)
{
    WindowEntry& e(entry());

    if ((_me->mode() != MODE_INSERT)
            && (e.position() == ((int)e.size() - 1)))
        return;

    e.vmove(1);
}

void MatrixPane::vbegin(void)
{
    entry().vmbegin();
}

void MatrixPane::vend(void)
{
    entry().vmend();
}

void MatrixPane::visual_move(int ch)
{
    switch (ch)
    {
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case K_BACKSPACE1:
        case K_BACKSPACE2:
        case 'h':
            vleft();
            break;

        case KEY_RIGHT:
        case 'l':
            vright();
            break;

        case K_TAB:
        case 'n':
            //next();
            break;

        case KEY_BTAB:
        case 'b':
            //prev();
            break;

        case KEY_DOWN:
        case 'j':
        case '+':
            //down();
            break;

        case KEY_UP:
        case 'k':
        case '-':
            //up();
            break;

        case K_NEWLINE:
        case KEY_ENTER:
            break;

        case KEY_HOME:
        case '^':
        case '_':
            vbegin();
            break;

        case '$':
            vend();
            break;

        case 'g':
            //first();
            break;

        case 'G':
            //last();
            break;
    }
}

void MatrixPane::visual_done(void)
{
    entry().vstop();
    _me->set_mode(MODE_EDIT);
}

void MatrixPane::visual_cancel(int ch)
{
    visual_done();
}

void MatrixPane::visual_remove(int ch)
{
    WindowEntry& e(entry());

    _me->yanked(e.vdata());
    e.vremove();
    if (e.position() == (int)e.size())
        e << 1;

    visual_done();
}

void MatrixPane::visual_replace(int ch)
{
    while ((ch = wgetch(_win)) != 0)
    {
        if (ch == -1)
            break;

        if (ch == K_ESCAPE)
            break;

        if (is_print(ch))
        {
            entry().vreplace(ch);
            break;
        }
    }

    visual_done();
}

void MatrixPane::visual_clear(int ch)
{
    visual_done();
}

void MatrixPane::visual_yank(int ch)
{
    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.vdata());

    visual_done();
}

void MatrixPane::visual_paste(int ch)
{
    if (_me->yanked().empty())
        return;

    entry().vreplace(_me->yanked());

    visual_done();
}

void MatrixPane::visual_toggle_position(int ch)
{
    entry().vtoggle();
}

void MatrixPane::enter(int ch)
{
    solve();
}

void MatrixPane::we_addstr(string s)
{
    for (size_t i = 0; i < s.size(); i++)
    {
        if (matrix_special(s[i]))
            waddch(_win, (chtype)s[i] | A_BOLD);
        else
            waddch(_win, s[i]);
    }
}

void MatrixPane::solve(void)
{
    size_t i, j;
    int old_curs = curs_set(0);

    _e_i = 0;
    auto it = _extra_data.begin();
    while (it != _extra_data.end())
    {
        delete *it;
        it = _extra_data.erase(it);
    }

    for (i = 0; i < _n; i++)
    {
        for (j = 0; j < _n; j++)
        {
            if (_c_matrix[i][j].empty())
            {
                ostringstream oss;

                oss << "Entry [" << i+1 << "][" << j+1 << "] in the coefficient matrix is empty";
                _me->error(oss.str());
                cancel(K_ESCAPE);

                _c_i = i+1;
                _c_j = j+1;
                _mp = MP_COEFFICIENT;

                set();
                curs_set(old_curs);

                return;
            }
        }
    }

    for (i = 0; i < _n; i++)
    {
        if (_s_vector[i].empty())
        {
            ostringstream oss;

            oss << "Entry [" << i+1 << "] in the solution vector is empty";
            _me->error(oss.str());
            cancel(K_ESCAPE);

            _s_i = i+1;
            _mp = MP_CONSTANTS;

            set();
            curs_set(old_curs);

            return;
        }
    }

    Matrix<Scientific> A(_n);
    try
    {
        for (i = 0; i < _n; i++)
        {
            for (j = 0; j < _n; j++)
                A(i+1,j+1) = Number<Scientific>::parse_expression(_c_matrix[i][j].data());
        }
    }
    catch (NumberParsingException<Scientific>& e)
    {
        ostringstream oss;

        oss << "Error with expression in entry [" << i+1 << "][" << j+1 << "] of the coefficient matrix:\n";
        oss << "  " << e.str() << "\n";
        oss << "  " << setw(e.index()+1) << "^";

        _me->error(oss.str());
        cancel(K_ESCAPE);

        _c_i = i+1;
        _c_j = j+1;
        _mp = MP_COEFFICIENT;

        set();
        curs_set(old_curs);

        return;
    }

    Matrix<Scientific> s(_n, 1);
    try
    {
        for (i = 0; i < _n; i++)
            s(i+1,1) = Number<Scientific>::parse_expression(_s_vector[i].data());
    }
    catch (NumberParsingException<Scientific>& e)
    {
        ostringstream oss;

        oss << "Error with expression in entry [" << i+1 << "] of the solution vector:\n";
        oss << "  " << e.str() << "\n";
        oss << "  " << setw(e.index()+1) << "^";

        _me->error(oss.str());
        cancel(K_ESCAPE);

        _s_i = i+1;
        _mp = MP_CONSTANTS;

        set();
        curs_set(old_curs);

        return;
    }

    if ((A == _last_A) && (s == _last_s))
    {
        for (i = 0; i < _n; i++)
        {
            ostringstream oss;

            oss << _last_v(i+1,1);

            _v_vector[i] = oss.str();
            _v_vector[i] << 1;
        }

        set();
        curs_set(old_curs);

        return;
    }

    Matrix<Scientific> v;
    try
    {
        v = A.solve(s);

    }
    catch (MatrixSingularException<Scientific>& e)
    {
        ostringstream oss;

        oss << "Coefficient matrix is singular";

        _me->error(oss.str());
        cancel(K_ESCAPE);

        _mp = MP_COEFFICIENT;
        _c_i = _c_j = 1;

        set();
        curs_set(old_curs);

        return;
    }

    _last_A = A;
    _last_s = s;
    _last_v = v;

    for (i = 0; i < _n; i++)
    {
        for (j = 0; j < _n; j++)
        {
            string s(_c_matrix[i][j].data());

            _history.push_back(s);
            if (_history.size() == 100)
                _history.erase(_history.begin(), _history.begin() + 10);
            _cur_hist_entry = _history.size() - 1;
        }
    }

    for (i = 0; i < _n; i++)
    {
        string s(_s_vector[i].data());

        _history.push_back(s);
        if (_history.size() == 100)
            _history.erase(_history.begin(), _history.begin() + 10);
        _cur_hist_entry = _history.size() - 1;
    }

    bool need_adjust = false;

    _mp = MP_UNKNOWNS;
    _v_i = 1;

    for (i = 0; i < _n; i++)
    {
        ostringstream oss;

        oss << v(i+1,1);

        _v_vector[i] = oss.str();
        _v_vector[i] << 1;

        if (adjust(_v_vector[i].width()))
            need_adjust = true;
    }

    if (need_adjust)
        draw();

    int curs_row = _header_rows + _n + 1;

    wmove(_win, curs_row, 0);
    wclrtobot(_win);

    curs_set(old_curs);

    wattr_on(_win, A_BOLD, NULL);
    wprintw(_win, "View additional data? ");
    wattr_off(_win, A_BOLD, NULL);

    int ch = wgetch(_win);

    wmove(_win, curs_row, 0);
    wclrtoeol(_win);

    if ((ch != 'y') && (ch != 'Y'))
    {
        set();
        cancel(K_ESCAPE);

        return;
    }

    old_curs = curs_set(0);

    string str;
    size_t pos;

    _e_i = 0;

    // Determinant
    wattr_on(_win, A_BOLD, NULL);
    mvwaddstr(_win, curs_row, 0, "Determinant:");
    wattr_off(_win, A_BOLD, NULL);
    {
        ostringstream oss;
        oss << A.determinant();
        str = oss.str();

        _extra_data.push_back(new EvalEntry(_win, Cursor(++curs_row, 0), str));
    }

    wgetch(_win);

    curs_row++;

    // Cofactor matrix
    wattr_on(_win, A_BOLD, NULL);
    mvwaddstr(_win, ++curs_row, 0, "Cofactor Matrix:");
    wattr_off(_win, A_BOLD, NULL);
    {
        ostringstream oss;
        oss << A.cofactor_matrix();
        str = oss.str();

        while (!str.empty())
        {
            pos = str.find('\n');
            if (pos > 0)
                _extra_data.push_back(new MatrixEntry(_win, Cursor(++curs_row, 0), str.substr(0, pos)));
            str.erase(0, pos + 1);
        }
#if 0
        Matrix<Scientific> CF(A.cofactor_matrix());
        vector<MatrixEntry*> cofactor_matrix;
        size_t max_size = 0;
        for (size_t i = 1; i <= CF.rows(); i++)
        {
            size_t col = 0;
            for (size_t j = 1; j <= CF.cols(); j++)
            {
                ostringstream oss;
                oss << CF(i,j);
                str = oss.str();

                if (str.size() > max_size)
                    max_size = str.size();

                MatrixEntry::MatrixEntryPosition mep;
                if (j == 1)
                    mep = MatrixEntry::MEP_START;
                else if (j == CF.cols())
                    mep = MatrixEntry::MEP_END;
                else
                    mep = MatrixEntry::MEP_MIDDLE;

                MatrixEntry* me = new MatrixEntry(_win, Cursor(curs_row + i, col), str, mep);
                _cofacor_matrix.push_back(me);
                col += me->width() + 3;
            }
        }

        curs_row += CF.rows();
#endif
    }

    curs_row++;

    // Adjoint matrix
    wattr_on(_win, A_BOLD, NULL);
    mvwaddstr(_win, ++curs_row, 0, "Adjoint:");
    wattr_off(_win, A_BOLD, NULL);
    {
        ostringstream oss;
        oss << A.adjoint();
        str = oss.str();

        while (!str.empty())
        {
            pos = str.find('\n');
            if (pos > 0)
                _extra_data.push_back(new MatrixEntry(_win, Cursor(++curs_row, 0), str.substr(0, pos)));
            str.erase(0, pos + 1);
        }
    }

    curs_row++;

    // Inverse matrix
    wattr_on(_win, A_BOLD, NULL);
    mvwaddstr(_win, ++curs_row, 0, "Inverse:");
    wattr_off(_win, A_BOLD, NULL);
    {
        ostringstream oss;
        oss << A.inverse();
        str = oss.str();

        while (!str.empty())
        {
            pos = str.find('\n');
            if (pos > 0)
                _extra_data.push_back(new MatrixEntry(_win, Cursor(++curs_row, 0), str.substr(0, pos)));
            str.erase(0, pos + 1);
        }
    }

    curs_row++;

    // Cramer's rule matices
    wattr_on(_win, A_BOLD, NULL);
    mvwaddstr(_win, ++curs_row, 0, "Cramer's rule:");
    wattr_off(_win, A_BOLD, NULL);

    for (j = 1; j <= A.rows(); j++)
    {
        ostringstream coss;
        streambuf *cout_buf = cout.rdbuf();
        cout.rdbuf(coss.rdbuf());

        Matrix<Scientific> CR = A.cramers_rule(s, j);

        str = coss.str();
        cout.rdbuf(cout_buf);

        pos = str.find('\n');
        _extra_data.push_back(new MatrixEntry(_win, Cursor(++curs_row, 0), str.substr(0, pos)));

        ostringstream oss;
        oss << CR;
        str = oss.str();

        while (!str.empty())
        {
            pos = str.find('\n');
            if (pos > 0)
                _extra_data.push_back(new MatrixEntry(_win, Cursor(++curs_row, 0), str.substr(0, pos)));
            str.erase(0, pos + 1);
        }

        curs_row++;
    }

    wgetch(_win);

    int y, x, h, w;
    getbegyx(_win, y, x);
    getmaxyx(_win, h, w);

    _e_i = 0;
    int max_row = h - 1;
    int scrolled = 0;
    for (size_t i = 0; i < _extra_data.size(); i++)
    {
        Cursor& c(_extra_data[i]->cursor());

        if (c.row() >= (max_row - 1))
        {
            wattr_on(_win, A_BOLD, NULL);
            mvwprintw(_win, max_row, 0, "Press Enter to see more data...");
            wattr_off(_win, A_BOLD, NULL);
            wgetch(_win);
            wmove(_win, max_row, 0);
            wclrtoeol(_win);

            int diff = c.row() - (max_row - 1);

            wscrl(_win, diff - scrolled + 1);
            scrolled += diff - scrolled + 1;

            c - (diff + 1);
        }

        _extra_data[i]->update();
    }

    //wgetch(_win);
    //wscrl(_win, -5);
    //wgetch(_win);
    //wscrl(_win, 5);

    wgetch(_win);
    set();
    curs_set(old_curs);
    cancel(K_ESCAPE);
}

EvalPane::EvalPane(MatrixEditor* me, WINDOW* w)
    : WindowPane(me, w), _win_offset(2), _e_entry(EvalEntry(_win, Cursor(0, _win_offset))),
    _cur_hist_entry(0)
{
}

EvalPane::EvalPane(const EvalPane& rhs)
    : WindowPane(rhs), _win_offset(rhs._win_offset), _e_entry(rhs._e_entry),
    _history(rhs._history), _cur_hist_entry(rhs._cur_hist_entry)
{
}

EvalPane& EvalPane::operator=(const EvalPane& rhs)
{
    if (this == &rhs)
        return *this;

    _e_entry = rhs._e_entry;
    _win_offset = rhs._win_offset;
    _valid_screen_chars = rhs._valid_screen_chars;
    _key_mode_actions = rhs._key_mode_actions;
    _history = rhs._history;
    _cur_hist_entry = rhs._cur_hist_entry;

    return *this;
}

bool EvalPane::is_print(int ch)
{
    size_t i;
    for (i = 0; i < _valid_screen_chars.size(); i++)
    {
        if (_valid_screen_chars[i] == ch)
            break;
    }

    return (i != _valid_screen_chars.size());
}

WindowPane::KeyAction EvalPane::key_action(int ch, EditorMode m)
{
    if (_key_mode_actions.find(ch) == _key_mode_actions.end())
    {
        if ((m == MODE_INSERT) && is_print(ch))
            return &WindowPane::insert;
        else if ((m == MODE_REPLACE) && is_print(ch))
            return &WindowPane::replace;
    }
    else
    {
        switch (m)
        {
            case MODE_INSERT:
                return _key_mode_actions[ch].insert_mode_action;
            case MODE_EDIT:
                return _key_mode_actions[ch].edit_mode_action;
            case MODE_VISUAL:
                return _key_mode_actions[ch].visual_mode_action;
            case MODE_REPLACE:
                return _key_mode_actions[ch].replace_mode_action;
        }
    }

    return &WindowPane::no_op;
}

void EvalPane::draw(void)
{
    int old_curs = curs_set(0);
    int y, x;
    getyx(_win, y, x);

    wmove(_win, 0, 0);
    wclrtoeol(_win);

    waddch(_win, (chtype)'>' | A_BOLD);
    waddch(_win, ' ');

    _e_entry.draw();

    wmove(_win, y, x);

    curs_set(old_curs);
}

void EvalPane::set(void)
{
    _e_entry.set();
}

void EvalPane::redraw(void)
{
    WindowPane::clear();

    draw();
    set();

    WindowPane::refresh();
}

void EvalPane::refresh(void)
{
    set();

    WindowPane::refresh();
}

void EvalPane::insert_mode(int ch)
{
    WindowPane::insert_mode(ch);

    if (ch == 'a')
        right();
}

void EvalPane::edit_mode(int ch)
{
    WindowPane::edit_mode(ch);

    _e_entry << 1;
}

void EvalPane::visual_mode(int ch)
{
    WindowPane::visual_mode(ch);

    _e_entry.vstart();
}

void EvalPane::replace_mode(int ch)
{
    if (ch != 'r')
    {
        WindowPane::replace_mode(ch);
        return;
    }

    while ((ch = wgetch(_win)) != 0)
    {
        if (ch == -1)
            break;

        if (ch == K_ESCAPE)
            break;

        if (is_print(ch))
        {
            _e_entry.replace(ch);
            break;
        }
    }
}

void EvalPane::insert(int ch)
{
    _e_entry.insert(ch);
    _e_entry >> 1;
}

void EvalPane::remove(int ch)
{
    if ((ch == KEY_DC) || (ch == 'x'))
    {
        _e_entry.remove();
        if (_e_entry.position() == (int)_e_entry.size())
            _e_entry << 1;
    }
    else
    {
        if (_e_entry.position() != 0)
        {
            _e_entry << 1;
            _e_entry.remove();
        }
    }
}

void EvalPane::replace(int ch)
{
    _replaced.push_back(_e_entry[_e_entry.position()]);

    _e_entry.replace(ch);
    _e_entry >> 1;
}

void EvalPane::replace_move(int ch)
{
    switch (ch)
    {
        case KEY_BACKSPACE:
        case K_BACKSPACE1:
        case K_BACKSPACE2:
            left();
            if (!_replaced.empty())
            {
                _e_entry.replace(_replaced.back());
                _replaced.pop_back();
            }
            break;

        case KEY_LEFT:
            left();
            _replaced.clear();
            break;

        case KEY_RIGHT:
            right();
            _replaced.clear();
            break;
    }
}

void EvalPane::clear_to_eol(int ch)
{
    if (_e_entry.empty())
        return;

    _me->yanked(_e_entry.pdata());

    _e_entry.pclear();
    _e_entry << 1;
}

void EvalPane::clear(int ch)
{
    _clear_toggle ^= 1;

    if (!_clear_toggle)
        return;

    if (_e_entry.empty())
        return;

    _me->yanked(_e_entry.data());

    _e_entry.clear();
}

void EvalPane::left(void)
{
    _e_entry << 1;
}

void EvalPane::right(void)
{
    if ((_me->mode() != MODE_INSERT)
            && (_e_entry.position() == ((int)_e_entry.size() - 1)))
        return;

    _e_entry >> 1;
}

void EvalPane::begin(void)
{
    _e_entry.mbegin();
}

void EvalPane::end(void)
{
    _e_entry.mend();

    if ((_me->mode() != MODE_INSERT)
            && (_e_entry.position() == (int)_e_entry.size()))
        _e_entry << 1;
}

void EvalPane::move(int ch)
{
    switch (ch)
    {
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case K_BACKSPACE1:
        case K_BACKSPACE2:
        case 'h':
            left();
            break;

        case KEY_RIGHT:
        case 'l':
            right();
            break;

        case KEY_HOME:
        case '^':
        case '_':
        case 'g':
            begin();
            break;

        case '$':
        case 'G':
            end();
            break;

        case KEY_UP:
            if (_history.empty())
                break;

            if (_cur_hist_entry >= 1)
                _e_entry = _history[--_cur_hist_entry];

            break;

        case KEY_DOWN:
            if (_history.empty())
                break;

            if (_cur_hist_entry < (_history.size() - 1))
                _e_entry = _history[++_cur_hist_entry];

            break;

        case 'e':
            if (_history.empty())
                break;

            _history.erase(_history.begin() + _cur_hist_entry);

            if (_history.empty())
            {
                _e_entry.clear();
                break;
            }

            if (_cur_hist_entry == _history.size())
                _cur_hist_entry--;

            _e_entry = _history[_cur_hist_entry];

            break;
    }
}

void EvalPane::yank(int ch)
{
    _yank_toggle ^= 1;

    if (!_yank_toggle)
        return;

    if (_e_entry.empty())
        return;

    _me->yanked(_e_entry.data());
}

void EvalPane::paste(int ch)
{
    if (_me->yanked().empty())
        return;

    if (ch == 'p')
        _e_entry >> 1;

    _e_entry.insert(_me->yanked());

    if ((_me->mode() != MODE_INSERT)
            && (_e_entry.position() == (int)_e_entry.size()))
        _e_entry << 1;
}

void EvalPane::vleft(void)
{
    _e_entry.vmove(-1);
}

void EvalPane::vright(void)
{
    if ((_me->mode() != MODE_INSERT)
            && (_e_entry.position() == ((int)_e_entry.size() - 1)))
        return;

    _e_entry.vmove(1);
}

void EvalPane::vbegin(void)
{
    _e_entry.vmbegin();
}

void EvalPane::vend(void)
{
    _e_entry.vmend();
}

void EvalPane::visual_move(int ch)
{
    switch (ch)
    {
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case K_BACKSPACE1:
        case K_BACKSPACE2:
        case 'h':
            vleft();
            break;

        case KEY_RIGHT:
        case 'l':
            vright();
            break;

        case K_NEWLINE:
        case KEY_ENTER:
            break;

        case KEY_HOME:
        case '^':
        case '_':
            vbegin();
            break;

        case '$':
            vend();
            break;

        case 'g':
            vbegin();
            break;

        case 'G':
            vend();
            break;
    }
}

void EvalPane::visual_done(void)
{
    _e_entry.vstop();
    _me->set_mode(MODE_EDIT);
}

void EvalPane::visual_cancel(int ch)
{
    visual_done();
}

void EvalPane::visual_remove(int ch)
{
    _me->yanked(_e_entry.vdata());
    _e_entry.vremove();
    if (_e_entry.position() == (int)_e_entry.size())
        _e_entry << 1;

    visual_done();
}

void EvalPane::visual_replace(int ch)
{
    while ((ch = wgetch(_win)) != 0)
    {
        if (ch == -1)
            break;

        if (ch == K_ESCAPE)
            break;

        if (is_print(ch))
        {
            _e_entry.vreplace(ch);
            break;
        }
    }

    visual_done();
}

void EvalPane::visual_clear(int ch)
{
    visual_done();
}

void EvalPane::visual_yank(int ch)
{
    if (_e_entry.empty())
        return;

    _me->yanked(_e_entry.vdata());

    visual_done();
}

void EvalPane::visual_paste(int ch)
{
    if (_me->yanked().empty())
        return;

    _e_entry.vreplace(_me->yanked());

    visual_done();
}

void EvalPane::visual_toggle_position(int ch)
{
    _e_entry.vtoggle();
}

void EvalPane::enter(int ch)
{
    solve();
}

void EvalPane::we_addstr(string s)
{
    for (size_t i = 0; i < s.size(); i++)
    {
        if (eval_special(s[i]))
            waddch(_win, (chtype)s[i] | A_BOLD);
        else
            waddch(_win, s[i]);
    }
}

void EvalPane::solve(void)
{
    if (_e_entry.empty())
    {
        _me->error(string("Nothing to solve."));
        return;
    }

    string s(_e_entry.data());

    _history.push_back(s);
    if (_history.size() == 100)
        _history.erase(_history.begin(), _history.begin() + 10);
    _cur_hist_entry = _history.size() - 1;

    try
    {
        Number<Scientific> n = Number<Scientific>::parse_expression(s);

        wmove(_win, 2, 0);

        //ColorPair c(2,_color_pair.bg());
        //wcolor_set(_win, c.pair_number(), NULL);
        for (int i = 0; i < 3; i++)
        {
            ostringstream oss;

            wattr_on(_win, A_BOLD, NULL);
            if (i == 0)
            {
                wprintw(_win, "Scientific: ");
                oss << n;
            }
            else if (i == 1)
            {
                wprintw(_win, "  Rational: ");
                oss << n->toRational();
            }
            else if (i == 2)
            {
                wprintw(_win, "     Float: ");
                oss << n->toFloat();
            }
            wattr_off(_win, A_BOLD, NULL);

            we_addstr(oss.str());
            waddch(_win, '\n');
        }
        //wcolor_set(_win, _color_pair.pair_number(), NULL);
    }
    catch (NumberParsingException<Scientific>& e)
    {
        ostringstream oss;

        oss << "Error parsing expression:\n";
        oss << "  " << e.str() << "\n";
        oss << "  " << setw(e.index()+1) << "^";

        _me->error(oss.str());
        cancel(K_ESCAPE);

        _e_entry.mpos(e.index());

        return;
    }

    cancel(K_ESCAPE);
    _e_entry.mpos(_e_entry.size() - 1);
    set();
}


MatrixEditor* MatrixEditor::sig_handler = NULL;
bool MatrixEditor::ncurses_initialized = false;

MatrixEditor::MatrixEditor(void)
    : _initialized(false), _mode(MODE_EDIT), _current_matrix_pane(0),
    _current_eval_pane(0), _current_editor_window(NULL),
    _h(0), _w(0), _y(0), _x(0),
    _matrix_pane_header(NULL), _matrix_pane_window(NULL),
    _eval_pane_header(NULL), _eval_pane_window(NULL),
    _command_window(NULL), _error_window(NULL),
    _command_history(0), _exit_loop(false)

{
    // Nothing
}

MatrixEditor::~MatrixEditor(void)
{
    fini();
}

void MatrixEditor::fini(void)
{
    _initialized = false;

    _mode = MODE_EDIT;

    _current_matrix_pane = 0;
    _matrix_panes.clear();

    _current_eval_pane = 0;
    _eval_panes.clear();

    _current_editor_window = NULL;

    _h = _w = _y = _x = 0;

    _yanked.clear();
    _command_history.clear();

    _exit_loop = false;

    if (_matrix_pane_header != NULL)
    {
        delwin(_matrix_pane_header);
        _matrix_pane_header = NULL;
    }

    if (_matrix_pane_window != NULL)
    {
        delwin(_matrix_pane_window);
        _matrix_pane_window = NULL;
    }

    if (_eval_pane_header != NULL)
    {
        delwin(_eval_pane_header);
        _eval_pane_header = NULL;
    }

    if (_eval_pane_window != NULL)
    {
        delwin(_eval_pane_window);
        _eval_pane_window = NULL;
    }

    if (_command_window != NULL)
    {
        delwin(_command_window);
        _command_window = NULL;
    }

    if (_error_window != NULL)
    {
        delwin(_error_window);
        _error_window = NULL;
    }

    endwin();

    sig_handler = NULL;
}

bool MatrixEditor::init_signals(void)
{
    if (signal(SIGWINCH, sig_winch) == SIG_ERR)
    {
        cout << "Failed to set SIGWINCH signal handler." << endl;
        return false;
    }

    if (signal(SIGINT, sig_quit) == SIG_ERR)
    {
        cout << "Failed to set SIGINT signal handler." << endl;
        return false;
    }

    if (signal(SIGTERM, sig_quit) == SIG_ERR)
    {
        cout << "Failed to set SIGTERM signal handler." << endl;
        return false;
    }

    if (signal(SIGABRT, sig_quit) == SIG_ERR)
    {
        cout << "Failed to set SIGABRT signal handler." << endl;
        return false;
    }

    return true;
}

bool MatrixEditor::init_ncurses(void)
{
    if (ncurses_initialized)
    {
        refresh();

        getmaxyx(stdscr, _h, _w);
        getbegyx(stdscr, _y, _x);

        return true;
    }

    if (!init_signals())
        return false;

    if (initscr() == NULL)
        return false;

    start_color();
    use_default_colors();

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    _too_small_color = ColorPair(COLOR_WHITE, COLOR_RED);
    _black_bar_color = ColorPair(COLOR_WHITE, COLOR_BLACK);
    _error_window_color = ColorPair(COLOR_WHITE, COLOR_RED);

    getmaxyx(stdscr, _h, _w);
    getbegyx(stdscr, _y, _x);

    ncurses_initialized = true;

    return true;
}

bool MatrixEditor::init(void)
{
    if (sig_handler != NULL)
        return false;

    sig_handler = this;

    if (_initialized)
        return true;

    if (!init_ncurses())
        return false;

    int mpwh = get_matrix_pane_height();
    int starty = _y;

    // Matrix window ***********************************************************
    _matrix_pane_header = newwin(mpwhh, _w, starty, _x);
    if (_matrix_pane_header == NULL)
    {
        printw("Could not allocate matrix title window");
        getch();
        fini();
        return false;
    }

    starty += mpwhh;

    wbkgd(_matrix_pane_header, COLOR_PAIR(_black_bar_color.pair_number()));
    wattron(_matrix_pane_header, A_BOLD);
    wprintw(_matrix_pane_header, "SOLVE LINEAR EQUATIONS");
    wnoutrefresh(_matrix_pane_header);

    _matrix_pane_window = newwin(mpwh, _w, starty, _x);
    if (_matrix_pane_window == NULL)
    {
        printw("Could not allocate matrix window");
        getch();
        fini();
        return false;
    }

    _matrix_panes.push_back(MatrixPane(2, this, _matrix_pane_window));
    _current_matrix_pane = 0;

    scrollok(_matrix_pane_window, TRUE);
    keypad(_matrix_pane_window, TRUE);
    wnoutrefresh(_matrix_pane_window);

    starty += mpwh;
    // *************************************************************************

    // Evaluation window *******************************************************
    _eval_pane_header = newwin(epwhh, _w, starty, _x);
    if (_eval_pane_header == NULL)
    {
        printw("Could not allocate evaluation title window");
        getch();
        fini();
        return false;
    }

    wbkgd(_eval_pane_header, COLOR_PAIR(_black_bar_color.pair_number()));
    wattron(_eval_pane_header, A_BOLD);
    wprintw(_eval_pane_header, "EVALUATE EXPRESSION");
    wnoutrefresh(_eval_pane_header);

    starty += epwhh;

    _eval_pane_window = newwin(epwh, _w, starty, _x);
    if (_eval_pane_window == NULL)
    {
        printw("Could not allocate evaluation window");
        getch();
        fini();
        return false;
    }

    _eval_panes.push_back(EvalPane(this, _eval_pane_window));
    _current_eval_pane = 0;

    scrollok(_eval_pane_window, TRUE);
    keypad(_eval_pane_window, TRUE);
    wnoutrefresh(_eval_pane_window);

    starty += epwh;
    // *************************************************************************

    // Error window ************************************************************
    _error_window = newwin(ewh, _w, starty, _x);
    if (_error_window == NULL)
    {
        printw("Could not allocate error window");
        getch();
        fini();
        return false;
    }

    init_pair(_error_window_color.pair_number(),
            _error_window_color.fg(), _error_window_color.bg());
    wbkgd(_error_window, COLOR_PAIR(_error_window_color.pair_number()));
    keypad(_error_window, TRUE);
    wattron(_error_window, A_BOLD);
    wnoutrefresh(_error_window);

    starty += ewh;
    // *************************************************************************

    // Command window **********************************************************
    _command_window = newwin(cwh, _w, starty, _x);
    if (_command_window == NULL)
    {
        printw("Could not allocate command window");
        getch();
        fini();
        return false;
    }

    init_pair(_black_bar_color.pair_number(),
            _black_bar_color.fg(), _black_bar_color.bg());
    wbkgd(_command_window, COLOR_PAIR(_black_bar_color.pair_number()));
    keypad(_command_window, TRUE);
    wnoutrefresh(_command_window);

    starty += cwh;
    // *************************************************************************

    doupdate();

    _eval_panes[_current_eval_pane].redraw();
    _matrix_panes[_current_matrix_pane].redraw();

    _current_editor_window = &_matrix_panes[_current_matrix_pane];

#if 0
    attr_t attrs;
    short pair, fg, bg, r, g, b;
    attr_get(&attrs, &pair, NULL);
    pair_content(pair, &fg, &bg);
    color_content(240, &r, &g, &b);
#endif

    _initialized = true;

    return true;
}

int MatrixEditor::get_matrix_pane_height(void)
{
    int mpwh = _h;

    mpwh -= mpwhh;   // Matrix Pane header
    mpwh -= epwhh;   // Eval Pane header
    mpwh -= epwh;    // Eval Pane window
    mpwh -= ewh;     // Error window
    mpwh -= cwh;     // Command window

    return mpwh;
}

void MatrixEditor::sig_winch(int sig)
{
    if (MatrixEditor::sig_handler != NULL)
        MatrixEditor::sig_handler->resize_windows();
}

void MatrixEditor::sig_quit(int sig)
{
    if (MatrixEditor::sig_handler != NULL)
        MatrixEditor::sig_handler->quit();
}

void MatrixEditor::quit(void)
{
    if (!ncurses_initialized || (sig_handler != this))
        return;

    fini();
    exit(0);
}

bool MatrixEditor::window_too_small(void)
{
    static int save_cursor = -1;
    static short std_scr_color = 0;

    if (save_cursor < 0)
    {
        attr_t attrs;
        wattr_get(stdscr, &attrs, &std_scr_color, NULL);
        save_cursor = curs_set(0);
    }

    int mpwh = get_matrix_pane_height();

    if ((mpwh < mpw_minh) || (_w < mpw_minw))
    {
        wclear(stdscr);

        wbkgd(stdscr, COLOR_PAIR(_too_small_color.pair_number()));

        wattr_on(stdscr, A_BOLD | A_BLINK, NULL);
        wprintw(stdscr, "!!!");
        wattr_off(stdscr, A_BLINK, NULL);

        wprintw(stdscr, " Terminal window is too small ");

        wattr_on(stdscr, A_BLINK, NULL);
        wprintw(stdscr, "!!!");
        wattr_off(stdscr, A_BOLD | A_BLINK, NULL);

        wrefresh(stdscr);

        return true;
    }

    wbkgd(stdscr, COLOR_PAIR(std_scr_color));
    curs_set(save_cursor);

    return false;
}

void MatrixEditor::resize_windows(void)
{
    if (!ncurses_initialized || (sig_handler != this))
        return;

    endwin();
    refresh();

    getmaxyx(stdscr, _h, _w);
    getbegyx(stdscr, _y, _x);

    if (window_too_small())
    {
        flushinp();
        return;
    }

    // int wresize(window, lines, columns) OK or ERR
    // bool is_term_resized(lines, columns) TRUE or FALSE  checks if resize_term would modify window structures
    // int resize_term(lines, columns) OK or ERR
    // int resizeterm(lines, columns) OK or ERR  adds bookkeeping for SIGWINCH??
    
    if (!_initialized)
        return;

    int mpwh = get_matrix_pane_height();
    int starty = _y;

    wresize(_matrix_pane_header, mpwhh, _w);
    mvwin(_matrix_pane_header, starty, _x);
    wnoutrefresh(_matrix_pane_header);

    starty += mpwhh;

    wresize(_matrix_pane_window, mpwh, _w);
    mvwin(_matrix_pane_window, starty, _x);
    wnoutrefresh(_matrix_pane_window);

    starty += mpwh;

    wresize(_eval_pane_header, epwhh, _w);
    mvwin(_eval_pane_header, starty, _x);
    wnoutrefresh(_eval_pane_header);

    starty += epwhh;

    wresize(_eval_pane_window, epwh, _w);
    mvwin(_eval_pane_window, starty, _x);
    wnoutrefresh(_eval_pane_window);

    starty += epwh;

    wresize(_error_window, ewh, _w);
    mvwin(_error_window, starty, _x);
    wnoutrefresh(_error_window);

    starty += ewh;

    wresize(_command_window, cwh, _w);
    mvwin(_command_window, starty, _x);
    wnoutrefresh(_command_window);

    starty += cwh;

    doupdate();

    _eval_panes[_current_eval_pane].redraw();
    _matrix_panes[_current_matrix_pane].redraw();

    flushinp();
}

void MatrixEditor::loop(void)
{
    if (!_initialized)
    {
        cout << "Failed to initialize." << endl;
        return;
    }

    set_escdelay(25);

    int mpwh = get_matrix_pane_height();
    if ((mpwh < mpw_minh) || (_w < mpw_minw))
        raise(SIGWINCH);

    int ch;
    while ((ch = wgetch(_current_editor_window->window())) != 0)
    {
        if (_exit_loop)
            break;

        if (ch == ERR)
            continue;

        if (ch == KEY_RESIZE)
            continue;

        WindowPane::KeyAction ka(_current_editor_window->key_action(ch, _mode));
        MEW_CALL(*_current_editor_window, ka)(ch);

        if (_exit_loop)
            break;

        _current_editor_window->refresh();
    }
}

void MatrixEditor::yanked(const string& s)
{
    _yanked = s;
}

const string& MatrixEditor::yanked(void)
{
    return _yanked;
}

EditorMode MatrixEditor::mode(void)
{
    return _mode;
}

void MatrixEditor::set_mode(EditorMode mode)
{
    if (!_initialized)
        return;

    switch (mode)
    {
        case MODE_INSERT:
            wattron(_command_window, A_BOLD);
            wprintw(_command_window, "-- INSERT --");
            wattroff(_command_window, A_BOLD);
            wrefresh(_command_window);
            break;

        case MODE_EDIT:
            wclear(_command_window);
            wrefresh(_command_window);
            break;

        case MODE_VISUAL:
            wattron(_command_window, A_BOLD);
            wprintw(_command_window, "-- VISUAL --");
            wattroff(_command_window, A_BOLD);
            wrefresh(_command_window);
            break;

        case MODE_REPLACE:
            wattron(_command_window, A_BOLD);
            wprintw(_command_window, "-- REPLACE --");
            wattroff(_command_window, A_BOLD);
            wrefresh(_command_window);
            break;

        default:
            return;

    }

    _mode = mode;
}

void MatrixEditor::error(string e)
{
    if (!_initialized)
        return;

    int old_curs = curs_set(0);

    wprintw(_error_window, "%s", e.c_str());
    wrefresh(_error_window);

    (void)wgetch(_error_window);

    wclear(_error_window);
    wrefresh(_error_window);

    curs_set(old_curs);
}

void MatrixEditor::command(int ch)
{
    if (!_initialized)
        return;

    process_command(ch);

    wclear(_command_window);
    wrefresh(_command_window);
}

void trim_whitespace(string& s)
{
    auto it = s.begin();
    while (it != s.end())
    {
        if (!isspace(*it))
            break;

        it = s.erase(it);
    }
}

void MatrixEditor::process_command(int ch)
{
    static char buffer[256];
    static size_t command_index = 0;

    WINDOW* w = _command_window;

    waddch(w, ch);

    int y, x;
    int line_start = 1;
    int line_end = 1;

    getyx(w, y, x);

    while ((ch = wgetch(w)) != 0)
    {
        switch (ch)
        {
            case K_ESCAPE:
                if (line_end > line_start)
                {
                    mvwinnstr(w, y, line_start, buffer, line_end - line_start);
                    string s(buffer);
                    size_t i;
                    for (i = 0; i < _command_history.size(); i++)
                    {
                        if (_command_history[i] == s)
                            break;
                    }

                    if (i == _command_history.size())
                    {
                        _command_history.insert(_command_history.begin(), s);
                        command_index = 0;
                    }
                }
                return;

            case KEY_ENTER:
            case K_NEWLINE:
                if (line_end > line_start)
                {
                    mvwinnstr(w, y, line_start, buffer, line_end - line_start);
                    string s(buffer);
                    size_t i;
                    for (i = 0; i < _command_history.size(); i++)
                    {
                        if (_command_history[i] == s)
                            break;
                    }

                    if (i == _command_history.size())
                    {
                        _command_history.insert(_command_history.begin(), s);
                        command_index = 0;
                    }

                    trim_whitespace(s);
                    execute_command(s);
                }
                return;

            case KEY_BACKSPACE:
            case K_BACKSPACE1:
            case K_BACKSPACE2:
                if ((x > line_start) || (line_end == line_start))
                {
                    mvwdelch(w, y, x - 1);
                    line_end--;
                }
                break;

            case KEY_DC:
                if (x == line_end)
                    mvwdelch(w, y, x - 1);
                else
                    wdelch(w);
                line_end--;
                break;

            case KEY_LEFT:
                if (x > line_start)
                    wmove(w, y, x - 1);
                break;

            case KEY_RIGHT:
                if (x < line_end)
                    wmove(w, y, x + 1);
                break;

            case K_TAB:
                // cycle through possible commands that match beginning of string
                // This is a completion depending on what's there already
                break;

            case KEY_UP:
                if (!_command_history.empty())
                {
                    string s(_command_history[command_index]);
                    line_end = s.size() + 1;
                    wmove(w, y, line_start);
                    wclrtoeol(w);
                    mvwaddstr(w, y, line_start, s.c_str());

                    if (command_index < (_command_history.size() - 1))
                        command_index++;
                }
                break;
            case KEY_DOWN:
                if (!_command_history.empty())
                {
                    string s(_command_history[command_index]);
                    line_end = s.size() + 1;
                    wmove(w, y, line_start);
                    wclrtoeol(w);
                    mvwaddstr(w, y, line_start, s.c_str());

                    if (command_index > 0)
                        command_index--;
                }
                break;

            default:
                if (isprint(ch))
                {
                    winsch(w, ch);
                    wmove(w, y, x + 1);
                    line_end++;
                }
                break;
        }

        getyx(w, y, x);
        if (x == 0)
            break;
    }
}

void MatrixEditor::execute_command(const string& s)
{
    WINDOW* w = _command_window;

    string com;
    vector<string> args;
    size_t pos;

    if ((pos = s.find_first_of(" \t")) == string::npos)
    {
        com = s;
    }
    else
    {
        com = s.substr(0, pos);

        size_t apos = pos + 1;
        while ((pos = s.find_first_of(" \t", apos)) != string::npos)
        {
            args.push_back(s.substr(apos, pos - apos));
            apos = pos + 1;
        }

        args.push_back(s.substr(apos));
    }

    if (_command_actions.find(com) != _command_actions.end())
    {
        wclear(w);
        wrefresh(w);

        MEC_CALL(*this, _command_actions[com])(args);
    }
    else
    {
        error(string("INVALID COMMAND: ") + com.c_str());
    }
}

void MatrixEditor::quit_all(const vector<string>& args)
{
    _exit_loop = true;
}

void MatrixEditor::quit(const vector<string>& args)
{
    WINDOW* ew = _current_editor_window->window();

    if (ew == _matrix_pane_window)
    {
        _matrix_panes.erase(_matrix_panes.begin() + _current_matrix_pane);

        if (_matrix_panes.empty())
        {
            _exit_loop = true;
            return;
        }

        if (_current_matrix_pane == _matrix_panes.size())
            _current_matrix_pane = _matrix_panes.size() - 1;

        _current_editor_window = &_matrix_panes[_current_matrix_pane];
    }
    else if (ew == _eval_pane_window)
    {
        _eval_panes.erase(_eval_panes.begin() + _current_eval_pane);

        if (_eval_panes.empty())
        {
            _exit_loop = true;
            return;
        }

        if (_current_eval_pane == _eval_panes.size())
            _current_eval_pane = _eval_panes.size() - 1;

        _current_editor_window = &_eval_panes[_current_eval_pane];
    }

    _current_editor_window->redraw();
}

void MatrixEditor::color(const vector<string>& args)
{
    WINDOW* w = _command_window;
    WINDOW* ew = _current_editor_window->window();
    ColorPair& cp = _current_editor_window->color();
    int ch;
    short pair_number, fg, bg;
    int old_curs = curs_set(0);

    pair_number = cp.pair(&fg, &bg);

    wprintw(w, "Use left/right arrows to change foreground and up/down to change background ");

    while ((ch = wgetch(w)) != 0)
    {
        if (ch == -1)
        {
            wprintw(w, "Error getting character");
            continue;
        }

        if ((ch == K_ESCAPE) || (ch == K_NEWLINE) || (ch == KEY_ENTER))
            break;

        if (ch == KEY_LEFT)
        {
            if (fg > 0)
                fg--;
            else
                fg = COLORS - 1;
        }
        else if (ch == KEY_RIGHT)
        {
            if (fg == COLORS - 1)
                fg = 0;
            else
                fg++;
        }
        else if (ch == KEY_DOWN)
        {
            if (bg > 0)
                bg--;
            else
                bg = COLORS - 1;
        }
        else if (ch == KEY_UP)
        {
            if (bg == COLORS - 1)
                bg = 0;
            else
                bg++;
        }

        init_pair(pair_number, fg, bg);
        wbkgd(ew, COLOR_PAIR(pair_number));
        wrefresh(ew);

        wclear(w);
        mvwprintw(w, 0, 0, "Foreground: %d, Background: %d", fg, bg);
    }

    cp.pair(fg, bg);

    wclear(w);
    wrefresh(w);

    curs_set(old_curs);
}

void MatrixEditor::prev_pane(const vector<string>& args)
{
    WINDOW* ew = _current_editor_window->window();

    if (ew == _matrix_pane_window)
    {
        if (_matrix_panes.size() <= 1)
            return;

        if (_current_matrix_pane == 0)
            _current_matrix_pane = _matrix_panes.size() - 1;
        else
            _current_matrix_pane--;

        _current_editor_window = &_matrix_panes[_current_matrix_pane];
    }
    else if (ew == _eval_pane_window)
    {
        if (_eval_panes.size() <= 1)
            return;

        if (_current_eval_pane == 0)
            _current_eval_pane = _eval_panes.size() - 1;
        else
            _current_eval_pane--;

        _current_editor_window = &_eval_panes[_current_eval_pane];
    }

    _current_editor_window->redraw();
}

void MatrixEditor::next_pane(const vector<string>& args)
{
    WINDOW* ew = _current_editor_window->window();

    if (ew == _matrix_pane_window)
    {
        if (_matrix_panes.size() <= 1)
            return;

        if ((_current_matrix_pane + 1) == _matrix_panes.size())
            _current_matrix_pane = 0;
        else
            _current_matrix_pane++;

        _current_editor_window = &_matrix_panes[_current_matrix_pane];
    }
    else if (ew == _eval_pane_window)
    {
        if (_eval_panes.size() <= 1)
            return;

        if ((_current_eval_pane + 1) == _eval_panes.size())
            _current_eval_pane = 0;
        else
            _current_eval_pane++;

        _current_editor_window = &_eval_panes[_current_eval_pane];
    }

    _current_editor_window->redraw();
}

void MatrixEditor::matrix_pane(const vector<string>& args)
{
    if (!args.empty() && (args[0] == "n"))  // New matrix
    {
        int n;

        if (args.size() < 2)
        {
            n = 3;
        }
        else
        {
            size_t idx;
            n = stoi(args[1], &idx, 0);

            if (idx == 0)
                return;
        }

        _matrix_panes.push_back(MatrixPane(n, this, _matrix_pane_window));
        if (_matrix_panes.size() == 100)
            _matrix_panes.erase(_matrix_panes.begin(), _matrix_panes.begin() + 10);
        _current_matrix_pane = _matrix_panes.size() - 1;

        _matrix_panes[_current_matrix_pane].redraw();
    }

    _current_editor_window = &_matrix_panes[_current_matrix_pane];
}

void MatrixEditor::eval_pane(const vector<string>& args)
{
    if (!args.empty() && (args[0] == "n"))  // New matrix
    {
        _eval_panes.push_back(EvalPane(this, _eval_pane_window));
        if (_eval_panes.size() == 100)
            _eval_panes.erase(_eval_panes.begin(), _eval_panes.begin() + 10);
        _current_eval_pane = _eval_panes.size() - 1;

        _eval_panes[_current_eval_pane].redraw();
    }

    _current_editor_window = &_eval_panes[_current_eval_pane];
}

void MatrixEditor::new_pane(const vector<string>& args)
{
    WINDOW* ew = _current_editor_window->window();

    vector<string> nargs(args);
    nargs.insert(nargs.begin(), "n");

    if (ew == _matrix_pane_window)
        matrix_pane(nargs);
    else if (ew == _eval_pane_window)
        eval_pane(nargs);
}

void MatrixEditor::redraw(const vector<string>& args)
{
    _current_editor_window->redraw();
}

