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
#include <fstream>
using namespace std;

static const string g_default_file = "linear_circuits.txt";

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

void Cursor::row(int r)
{
    _row = r;
}

void Cursor::col(int c)
{
    _col = c;
}

WindowEntry::WindowEntry(void)
    : _pos(0), _vstart(-1), _win(NULL), _win_pos(Cursor(0, 0)),
      _default_attrs(AttrColor())
{
}

WindowEntry::WindowEntry(WINDOW* w, const Cursor& c)
    : _pos(0), _vstart(-1), _win(w), _win_pos(c),
      _default_attrs(AttrColor())
{
}

WindowEntry::WindowEntry(WINDOW* w, const Cursor& c, string s)
    : _data(s), _pos(0), _vstart(-1), _win(w), _win_pos(c),
      _default_attrs(AttrColor())
{
}

WindowEntry::WindowEntry(const WindowEntry& rhs)
    : _data(rhs._data), _pos(rhs._pos), _vstart(rhs._vstart),
      _win(rhs._win), _win_pos(rhs._win_pos),
      _default_attrs(rhs._default_attrs)
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
        _data.erase(_data.begin() + ((_pos + 1) + count),
                _data.begin() + (_pos + 1));
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

string WindowEntry::data(void) const
{
    return _data;
}

string WindowEntry::pdata(void) const
{
    return _data.substr(_pos);
}

string WindowEntry::vdata(void) const
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

int WindowEntry::position(void) const
{
    return _pos;
}

size_t WindowEntry::length(void) const
{
    return _data.size();
}

size_t WindowEntry::size(void) const
{
    return _data.size();
}

bool WindowEntry::empty(void) const
{
    return _data.size() == 0;
}

Cursor& WindowEntry::cursor(void)
{
    return _win_pos;
}

size_t WindowEntry::width(void) const
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

void WindowEntry::read(istream& is)
{
    is >> _data;
}

istream& operator>>(istream& is, WindowEntry& rhs)
{
    rhs.read(is);
    return is;
}

void WindowEntry::write(ostream& os) const
{
    os << _data;
}

ostream& operator<<(ostream& os, const WindowEntry& rhs)
{
    rhs.write(os);
    return os;
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

MatrixEntry::MatrixEntry(WINDOW* w, const Cursor& c,
        size_t width, MatrixEntryPosition mep)
    : WindowEntry(w, c), _width(width), _mep(mep)
{
    if (_width == 0)
        _width = 1;

    _win_pos >> (_width + 2);
}

MatrixEntry::MatrixEntry(WINDOW* w, const Cursor& c,
        string s, MatrixEntryPosition mep)
    : WindowEntry(w, c, s), _mep(mep)
{
    if (_data.empty())
        _width = 1;
    else
        _width = _data.size();

    _pos = _data.size();
    _win_pos >> (_width + 2);
    //_win_pos >> 2;
}

MatrixEntry::MatrixEntry(WINDOW* w, const Cursor& c,
        size_t width, string s, MatrixEntryPosition mep)
    : WindowEntry(w, c, s), _width(width), _mep(mep)
{
    if (_data.empty() && (_width == 0))
        _width = 1;
    else if (_width < _data.size())
        _width = _data.size();

    _pos = _data.size();
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
    //int move_amount = ((_width - _data.size()) + _pos);
    int move_amount = ((_width - _data.size()) + _pos) + 2;

    wmove(_win, _win_pos.row(), _win_pos.col() - move_amount);

    draw();

    wmove(_win, _win_pos.row(), _win_pos.col());

    curs_set(old_curs);
}

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
        _data.erase(_data.begin() + ((_pos + 1) + count),
                _data.begin() + (_pos + 1));
        _pos += (count + 1);
        _win_pos >> 1;
    }

    update();
}

void MatrixEntry::remove(void)
{
    remove(1);
}

size_t MatrixEntry::width(void) const
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

//******************************************************************************
//* WindowPane *****************************************************************
//******************************************************************************
WindowPane::WindowPane(MatrixEditor* me, WINDOW* w)
    : _me(me), _win(w), _color_pair(ColorPair(COLOR_BLACK, COLOR_WHITE)),
      _clear_toggle(1), _yank_toggle(1), _begin_toggle(1),
      _history(0), _cur_hist_entry(0), _cur_col(0)
{
    int height, width;

    getmaxyx(_win, height, width);

    _win_start = Cursor(0, 0);
    _win_end = Cursor(height, width);

    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));
}

WindowPane::WindowPane(MatrixEditor* me, WINDOW* w,
        const Cursor& start, const Cursor& end)
    : _me(me), _win(w), _win_start(start), _win_end(end),
      _color_pair(ColorPair(COLOR_BLACK, COLOR_WHITE)),
      _clear_toggle(1), _yank_toggle(1), _begin_toggle(1),
      _history(0), _cur_hist_entry(0), _cur_col(0)
{
    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));
}

WindowPane::WindowPane(const WindowPane& rhs)
    : _me(rhs._me), _win(rhs._win),
      _win_start(rhs._win_start), _win_end(rhs._win_end),
      _color_pair(rhs._color_pair), _clear_toggle(rhs._clear_toggle),
      _yank_toggle(rhs._yank_toggle), _begin_toggle(rhs._begin_toggle),
      _replaced(rhs._replaced), _history(rhs._history),
      _cur_hist_entry(rhs._cur_hist_entry), _cur_col(rhs._cur_col)
{
    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));
}

WindowPane& WindowPane::operator=(const WindowPane& rhs)
{
    if (this == &rhs)
        return *this;

    _me = rhs._me;
    _win = rhs._win;
    _win_start = rhs._win_start;
    _win_end = rhs._win_end;
    _color_pair = rhs._color_pair;
    _clear_toggle = rhs._clear_toggle;
    _yank_toggle = rhs._yank_toggle;
    _begin_toggle = rhs._begin_toggle;
    _replaced = rhs._replaced;
    _history = rhs._history;
    _cur_hist_entry = rhs._cur_hist_entry;
    _cur_col = rhs._cur_col;

    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));

    return *this;
}

void WindowPane::key_action(int ch, EditorMode m)
{
    if (_key_mode_actions.find(ch) == _key_mode_actions.end())
    {
        if ((m == MODE_INSERT) && is_print(ch))
            insert(ch);
        else if ((m == MODE_REPLACE) && is_print(ch))
            replace(ch);
    }
    else
    {
        switch (m)
        {
            case MODE_INSERT:
                MEW_CALL(*this, _key_mode_actions[ch].insert_mode_action)(ch);
                break;
            case MODE_EDIT:
                MEW_CALL(*this, _key_mode_actions[ch].edit_mode_action)(ch);
                break;
            case MODE_VISUAL:
                MEW_CALL(*this, _key_mode_actions[ch].visual_mode_action)(ch);
                break;
            case MODE_REPLACE:
                MEW_CALL(*this, _key_mode_actions[ch].replace_mode_action)(ch);
                break;
            default:
                no_op(ch);
                break;
        }
    }
}

bool WindowPane::is_print(int ch)
{
    //return (isgraph(ch) || isspace(ch));
    return isprint(ch);
}

void WindowPane::we_addstr(string s)
{
    for (size_t i = 0; i < s.size(); i++)
        waddch(_win, s[i]);
}

void WindowPane::draw(void)
{
    entry().draw();
}

void WindowPane::set(void)
{
    entry().set();
}

void WindowPane::clear(void)
{
    wclear(_win);
    wrefresh(_win);
}

void WindowPane::redraw(void)
{
    clear();
    draw();
    set();
    wrefresh(_win);
}

void WindowPane::refresh(void)
{
    set();
    wrefresh(_win);
}

void WindowPane::insert_mode(int ch)
{
    _me->set_mode(MODE_INSERT);

    if (ch == 'a')
        right(ch);
    else if (ch == 'A')
        line_end(ch);
    else if (ch == 's')
        remove(ch);
    else if (ch == 'o')
        insert_line(ch);
    else if (ch == 'O')
        insert_line(ch);
}

void WindowPane::edit_mode(int ch)
{
    _me->set_mode(MODE_EDIT);
    left(ch);
}

void WindowPane::cancel(int ch)
{
    _clear_toggle = 1;
    _yank_toggle = 1;
    _begin_toggle = 1;

    _me->set_mode(MODE_EDIT);
}

void WindowPane::visual_mode(int ch)
{
    _me->set_mode(MODE_VISUAL);
    entry().vstart();
}

void WindowPane::visual_cancel(int ch)
{
    entry().vstop();
    _me->set_mode(MODE_EDIT);
}

void WindowPane::replace_mode(int ch)
{
    _me->set_mode(MODE_REPLACE);
    _replaced.clear();
}

void WindowPane::replace_cancel(int ch)
{
    _replaced.clear();
    _me->set_mode(MODE_EDIT);
}

void WindowPane::insert(int ch)
{
    WindowEntry& e(entry());

    e.insert(ch);
    e >> 1;

    _cur_col = e.cursor().col();
}

void WindowPane::insert_line(int ch)
{
}

void WindowPane::remove(int ch)
{
    WindowEntry& e(entry());

    e.remove();

    if (ch == 's')
        return;

    if (e.position() == (int)e.size())
        e << 1;

    _cur_col = e.cursor().col();
}

void WindowPane::remove_left(int ch)
{
    WindowEntry& e(entry());

    if (e.position() == 0)
        return;

    e << 1;
    e.remove();

    _cur_col = e.cursor().col();
}

void WindowPane::remove_line(int ch)
{
    _clear_toggle ^= 1;

    if (!_clear_toggle)
        return;

    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.data());

    e.clear();

    _cur_col = e.cursor().col();
}

void WindowPane::clear_to_eol(int ch)
{
    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.pdata());

    e.pclear();
    e << 1;

    _cur_col = e.cursor().col();
}

void WindowPane::clear_all(int ch)
{
    clear_to_eol(ch);
}

void WindowPane::yank(int ch)
{
    _yank_toggle ^= 1;

    if (!_yank_toggle)
        return;

    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.data());
}

void WindowPane::paste(int ch)
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

    _cur_col = e.cursor().col();
}

void WindowPane::move(int ch)
{
}

void WindowPane::left(int ch)
{
    WindowEntry& e(entry());

    e << 1;

    _cur_col = e.cursor().col();
}

void WindowPane::right(int ch)
{
    WindowEntry& e(entry());

    if ((_me->mode() != MODE_INSERT)
            && (e.position() == ((int)e.size() - 1)))
        return;

    e >> 1;

    _cur_col = e.cursor().col();
}

void WindowPane::up(int ch)
{
}

void WindowPane::down(int ch)
{
}

void WindowPane::prev(int ch)
{
}

void WindowPane::next(int ch)
{
}

void WindowPane::line_begin(int ch)
{
    WindowEntry& e(entry());

    e.mbegin();
    _cur_col = e.cursor().col();
}

void WindowPane::line_end(int ch)
{
    WindowEntry& e(entry());

    e.mend();

    if ((_me->mode() != MODE_INSERT)
            && (e.position() == (int)e.size()))
        e << 1;

    _cur_col = e.cursor().col();
}

void WindowPane::pane_begin(int ch)
{
    _begin_toggle ^= 1;

    if (!_begin_toggle)
        return;

    line_begin(ch);
}

void WindowPane::pane_end(int ch)
{
    line_end(ch);
}

void WindowPane::history_prev(int ch)
{
    if (_history.empty())
        return;

    if (_cur_hist_entry >= 1)
        entry() = _history[--_cur_hist_entry];
}

void WindowPane::history_next(int ch)
{
    if (_history.empty())
        return;

    if (_cur_hist_entry < (_history.size() - 1))
        entry() = _history[++_cur_hist_entry];
}

void WindowPane::history_delete(int ch)
{
    if (_history.empty())
        return;

    _history.erase(_history.begin() + _cur_hist_entry);

    if (_history.empty())
    {
        entry().clear();
        return;
    }

    if (_cur_hist_entry == _history.size())
        _cur_hist_entry--;

    entry() = _history[_cur_hist_entry];
}

void WindowPane::visual_move(int ch)
{
}

void WindowPane::visual_left(int ch)
{
    entry().vmove(-1);
}

void WindowPane::visual_right(int ch)
{
    WindowEntry& e(entry());

    if ((_me->mode() != MODE_INSERT)
            && (e.position() == ((int)e.size() - 1)))
        return;

    e.vmove(1);
}

void WindowPane::visual_up(int ch)
{
}

void WindowPane::visual_down(int ch)
{
}

void WindowPane::visual_line_begin(int ch)
{
    entry().vmbegin();
}

void WindowPane::visual_line_end(int ch)
{
    entry().vmend();
}

void WindowPane::visual_pane_begin(int ch)
{
    entry().vmbegin();
}

void WindowPane::visual_pane_end(int ch)
{
    entry().vmend();
}

void WindowPane::visual_remove(int ch)
{
    WindowEntry& e(entry());

    _me->yanked(e.vdata());
    e.vremove();
    if (e.position() == (int)e.size())
        e << 1;

    visual_cancel(ch);
}

void WindowPane::visual_replace(int ch)
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

    visual_cancel(ch);
}

void WindowPane::visual_clear(int ch)
{
    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.data());

    e.clear();

    visual_cancel(ch);
}

void WindowPane::visual_yank(int ch)
{
    WindowEntry& e(entry());

    if (e.empty())
        return;

    _me->yanked(e.vdata());

    visual_cancel(ch);
}

void WindowPane::visual_paste(int ch)
{
    if (_me->yanked().empty())
        return;

    entry().vreplace(_me->yanked());

    visual_cancel(ch);
}

void WindowPane::visual_toggle(int ch)
{
    entry().vtoggle();
}

void WindowPane::replace(int ch)
{
    WindowEntry& e(entry());

    if (e.position() == (int)e.size())
    {
        _replaced.push_back(0);
        e.insert(ch);
    }
    else
    {
        _replaced.push_back(e[e.position()]);
        e.replace(ch);
    }

    e >> 1;
}

void WindowPane::replace_move(int ch)
{
    WindowEntry& e(entry());

    switch (ch)
    {
        case KEY_BACKSPACE:
        case K_BACKSPACE1:
        case K_BACKSPACE2:
            left(ch);
            if (!_replaced.empty())
            {
                if (_replaced.back() == 0)
                    e.remove();
                else
                    e.replace(_replaced.back());
                _replaced.pop_back();
            }
            break;

        case KEY_LEFT:
            left(ch);
            _replaced.clear();
            break;

        case KEY_RIGHT:
            right(ch);
            _replaced.clear();
            break;
    }
}

void WindowPane::replace_one(int ch)
{
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

void WindowPane::update_color(short fg, short bg)
{
    _color_pair.pair(fg, bg);
    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));
    redraw();
}

void WindowPane::write(ostream& os) const
{
}

void WindowPane::read(istream& is)
{
}

bool WindowPane::empty(void) const
{
    return true;
}

istream& operator>>(istream& is, WindowPane& rhs)
{
    rhs.read(is);
    return is;
}

ostream& operator<<(ostream& os, const WindowPane& rhs)
{
    rhs.write(os);
    return os;
}

WINDOW* WindowPane::window(void)
{
    return _win;
}

ColorPair& WindowPane::color(void)
{
    return _color_pair;
}

//******************************************************************************
//* TextPane *******************************************************************
//******************************************************************************
TextPane::TextPane(MatrixEditor* me, WINDOW* w)
    : WindowPane(me, w), _lines(0), _cur_line(0)
{
    _lines.push_back(WindowEntry(_win, _win_start));
    _tilde_fill = ColorPair(COLOR_BLUE, _color_pair.bg());
}

TextPane::TextPane(MatrixEditor* me, WINDOW* w, vector<string> lines)
    : WindowPane(me, w), _lines(0), _cur_line(0)
{
    for (size_t i = 0; i < lines.size(); i++)
        (*this)[i] = lines[i];

    if (_lines.empty())
        _lines.push_back(WindowEntry(_win, _win_start));

    _tilde_fill = ColorPair(COLOR_BLUE, _color_pair.bg());
}

TextPane::TextPane(MatrixEditor* me, WINDOW* w,
        const Cursor& start, const Cursor& end)
    : WindowPane(me, w, start, end), _lines(0), _cur_line(0)
{
    _lines.push_back(WindowEntry(_win, _win_start));
    _tilde_fill = ColorPair(COLOR_BLUE, _color_pair.bg());
}

TextPane::TextPane(MatrixEditor* me, WINDOW* w, vector<string> lines,
        const Cursor& start, const Cursor& end)
    : WindowPane(me, w, start, end), _lines(0), _cur_line(0)
{
    for (size_t i = 0; i < lines.size(); i++)
        (*this)[i] = lines[i];

    if (_lines.empty())
        _lines.push_back(WindowEntry(_win, _win_start));

    _tilde_fill = ColorPair(COLOR_BLUE, _color_pair.bg());
}

TextPane::TextPane(const TextPane& rhs)
    : WindowPane(rhs), _lines(rhs._lines),
      _cur_line(rhs._cur_line), _tilde_fill(rhs._tilde_fill)
{
}

TextPane& TextPane::operator=(const TextPane& rhs)
{
    if (this == &rhs)
        return *this;

    WindowPane::operator=(rhs);

    _lines = rhs._lines;
    _cur_line = rhs._cur_line;
    _tilde_fill = rhs._tilde_fill;

    return *this;
}

WindowEntry& TextPane::operator[](size_t row)
{
    if (_lines.empty())
        _lines.push_back(WindowEntry(_win, _win_start));

    if (row >= _lines.size())
    {
        int new_rows = (int)row - ((int)_lines.size() - 1);

        for (size_t i = 0; i < new_rows; i++)
            insert_line('o');
    }

    _cur_line = row;

    return _lines[row];
}

void TextPane::key_action(int ch, EditorMode m)
{
    if (_key_mode_actions.find(ch) == _key_mode_actions.end())
    {
        if ((m == MODE_INSERT) && is_print(ch))
            insert(ch);
        else if ((m == MODE_REPLACE) && is_print(ch))
            replace(ch);
    }
    else
    {
        switch (m)
        {
            case MODE_INSERT:
                MEW_CALL(*this, _key_mode_actions[ch].insert_mode_action)(ch);
                break;
            case MODE_EDIT:
                MEW_CALL(*this, _key_mode_actions[ch].edit_mode_action)(ch);
                break;
            case MODE_VISUAL:
                MEW_CALL(*this, _key_mode_actions[ch].visual_mode_action)(ch);
                break;
            case MODE_REPLACE:
                MEW_CALL(*this, _key_mode_actions[ch].replace_mode_action)(ch);
                break;
            default:
                no_op(ch);
                break;
        }
    }
}

WindowEntry& TextPane::entry(size_t row, size_t col)
{
    if (row >= _lines.size())
        row = _lines.size() - 1;

    return _lines[row];
}

WindowEntry& TextPane::entry(void)
{
    return _lines[_cur_line];
}

void TextPane::draw(void)
{
    int bg = _color_pair.bg();
    if (bg != _tilde_fill.bg())
        _tilde_fill.bg(bg);

    for (size_t i = 0; i < _lines.size(); i++)
    {
        WindowEntry& e(_lines[i]);

        if ((e.cursor().row() >= _win_start.row())
                && (e.cursor().row() < _win_end.row()))
        {
            wmove(_win, e.cursor().row(), 0);
            wclrtoeol(_win);

            e.draw();
        }
    }

    if (_lines[_lines.size() - 1].cursor().row() < _win_end.row())
    {
        int start_line = _lines[_lines.size() - 1].cursor().row() + 1;
        for (int i = start_line; i < _win_end.row(); i++)
        {
            wmove(_win, i, 0);
            wclrtoeol(_win);
            waddch(_win, (chtype)'~' | COLOR_PAIR(_tilde_fill.pair_number()));
        }
    }
}

void TextPane::insert_line(int ch)
{
    WindowEntry& e(entry());
    Cursor c(e.cursor());
    string data;

    switch (ch)
    {
        case K_NEWLINE:
        case KEY_ENTER:
        default:
            data = e.pdata();
            e.pclear();
            // Fall through
        case 'o':
            _cur_line++;
            c + 1;
            break;

        case 'O':
            break;
    }

    c.col(0);

    _lines.insert(_lines.begin() + _cur_line, WindowEntry(_win, c, data));

    if (entry().cursor().row() == _win_end.row())
    {
        for (size_t i = 0; i <= _cur_line; i++)
            _lines[i].cursor() - 1;
    }
    else
    {
        for (size_t i = _cur_line + 1; i < _lines.size(); i++)
            _lines[i].cursor() + 1;
    }

    // XXX Could be optimized to draw only affected lines
    draw();

    _cur_col = entry().cursor().col();
}

void TextPane::remove(int ch)
{
    WindowEntry& e(entry());

    if ((ch != KEY_DC) || (_me->mode() != MODE_INSERT))
    {
        WindowPane::remove(ch);
        return;
    }

    if ((_cur_line < (_lines.size() - 1))
            && (e.position() == (int)e.size()))
    {
        WindowEntry& next(entry(_cur_line + 1));
        string s(next.data());
        int pos = e.position();

        e.insert(s);
        e.mpos(pos);

        _lines.erase(_lines.begin() + _cur_line + 1);

        for (size_t i = _cur_line + 1; i < _lines.size(); i++)
            _lines[i].cursor() - 1;

        // XXX Could be optimized to draw only affected lines
        draw();

        return;
    }

    e.remove();

    _cur_col = e.cursor().col();
}

void TextPane::remove_left(int ch)
{
    WindowEntry& e(entry());

    if ((_cur_line == 0) || (e.position() != 0))
    {
        WindowPane::remove_left(ch);
        return;
    }

    string s(e.data());
    int cur_row = e.cursor().row();

    WindowEntry& prev(entry(_cur_line - 1));
    int pos;

    prev.mend();
    pos = prev.position();
    prev.insert(s);
    prev.mpos(pos);

    _lines.erase(_lines.begin() + _cur_line);

    if (cur_row == _win_start.row())
    {
        for (size_t i = 0; i < _cur_line; i++)
            _lines[i].cursor() + 1;
    }
    else
    {
        for (size_t i = _cur_line; i < _lines.size(); i++)
            _lines[i].cursor() - 1;
    }

    _cur_line--;

    // XXX Could be optimized to draw only affected lines
    draw();

    _cur_col = entry().cursor().col();
}

void TextPane::remove_line(int ch)
{
    if (_lines.size() == 1)
    {
        WindowPane::remove_line(ch);
        return;
    }

    _clear_toggle ^= 1;

    if (!_clear_toggle)
        return;

    WindowEntry& e(entry());
    string s(e.data());
    int cur_row = e.cursor().row();

    WindowEntry& last(entry(_lines.size() - 1));

    if (last.cursor().row() < _win_end.row())
    {
        wmove(_win, last.cursor().row(), 0);
        wclrtoeol(_win);
    }

    _lines.erase(_lines.begin() + _cur_line);

    if ((cur_row == _win_start.row()) && (_cur_line == _lines.size()))
    {
        for (size_t i = 0; i < _cur_line; i++)
            _lines[i].cursor() + 1;
    }
    else
    {
        for (size_t i = _cur_line; i < _lines.size(); i++)
            _lines[i].cursor() - 1;
    }

    if (_cur_line == _lines.size())
        _cur_line--;

    draw();

    entry().mbegin();

    _cur_col = entry().cursor().col();

    if (!s.empty())
        _me->yanked(s);
}

void TextPane::clear_all(int ch)
{
    _lines.clear();
    _cur_line = 0;
    _lines.push_back(WindowEntry(_win, _win_start));

    draw();
}

void TextPane::up(int ch)
{
    if (_cur_line == 0)
        return;

    _cur_line--;

    if (_lines[_cur_line].cursor().row() < _win_start.row())
    {
        for (size_t i = 0; i < _lines.size(); i++)
            _lines[i].cursor() + 1;

        draw();
    }

    WindowEntry& e(entry());

    e.mpos(_cur_col);
    if ((_me->mode() != MODE_INSERT)
            && (e.position() == (int)e.size()))
    {
        e << 1;
    }
}

void TextPane::down(int ch)
{
    if ((_cur_line + 1) >= _lines.size())
        return;

    _cur_line++;

    if (_lines[_cur_line].cursor().row() == _win_end.row())
    {
        for (size_t i = 0; i < _lines.size(); i++)
            _lines[i].cursor() - 1; 

        draw();
    }

    WindowEntry& e(entry());

    e.mpos(_cur_col);
    if ((_me->mode() != MODE_INSERT)
            && (e.position() == (int)e.size()))
    {
        e << 1;
    }
}

void TextPane::pane_begin(int ch)
{
    _begin_toggle ^= 1;

    if (!_begin_toggle)
        return;

    WindowEntry& e(entry(0));

    if (e.cursor().row() < _win_start.row())
    {
        int start_row = _win_start.row();

        for (size_t i = 0; i < _lines.size(); i++)
            _lines[i].cursor().row(start_row + i);

        draw();
    }

    _cur_line = 0;
    line_begin(ch);
}

void TextPane::pane_end(int ch)
{
    WindowEntry& e(entry(_lines.size() - 1));

    if (e.cursor().row() >= _win_end.row())
    {
        int start_row = _win_end.row() - (int)_lines.size();

        for (int i = 0; i < (int)_lines.size(); i++)
            _lines[i].cursor().row(start_row + i);

        draw();
    }

    _cur_line = _lines.size() - 1;
    line_end(ch);
}

void TextPane::write(ostream& os) const
{
    if (empty())
        return;

    for (size_t i = 0; i < _lines.size(); i++)
        os << _lines[i] << '\n';
}

void TextPane::read(istream& is)
{
    static char buffer[1024];

    _lines.clear();

    size_t i = 0;
    while (is.getline(buffer, sizeof(buffer)))
        (*this)[i++] = string(buffer);
}

bool TextPane::empty(void) const
{
    size_t i;
    for (i = 0; i < _lines.size(); i++)
    {
        if (!_lines[i].empty())
            break;
    }

    return i == _lines.size();
}

//******************************************************************************
//* MatrixPane *****************************************************************
//******************************************************************************
MatrixPane::MatrixPane(size_t n, MatrixEditor* me, WINDOW* w)
    : WindowPane(me, w), _mpp(MPP_MATRIX), _mp(MP_COEFFICIENT), _mtp(MTP_TEXT),
      _n(n), _c_i(0), _c_j(0), _s_i(0), _v_i(0),
      _c_col_width(10), _s_col_width(6), _v_col_width(6)
{
    _init();
}

MatrixPane::MatrixPane(size_t n, MatrixEditor* me, WINDOW* w,
        const Cursor& start, const Cursor& end)
    : WindowPane(me, w, start, end), _mpp(MPP_MATRIX),
      _mp(MP_COEFFICIENT), _mtp(MTP_TEXT),
      _n(n), _c_i(0), _c_j(0), _s_i(0), _v_i(0),
      _c_col_width(10), _s_col_width(6), _v_col_width(6)
{
    _init();
}

void MatrixPane::_init(void)
{
    _c_matrix.resize(_n);

    size_t win_row = _win_start.row();
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

            Cursor c(win_row + _header_rows + i, win_col);
            _c_matrix[i][j] = MatrixEntry(_win, c, _c_col_width, mep);

            win_col += _c_matrix[i][j].width() + 3;
        }
    }

    _s_vector.resize(_n);
    //win_col += _col_spacing + _vector_spacing + _col_spacing
    //    + _pad + _s_col_width + 1;
    win_col += _vector_spacing + 1;
    for (size_t i = 0; i < _n; i++)
    {
        Cursor c(win_row + _header_rows + i, win_col);
        _s_vector[i] = MatrixEntry(_win, c, _s_col_width);
    }

    _v_vector.resize(_n);
    //win_col += _col_spacing + _vector_spacing + _col_spacing
    //    + _pad + _v_col_width + 1;
    win_col += _s_vector[0].width() + 3 + _vector_spacing + 1;
    for (size_t i = 0; i < _n; i++)
    {
        Cursor c(win_row + _header_rows + i, win_col);
        _v_vector[i] = MatrixEntry(_win, c, _v_col_width);
    }

    Cursor start_pane(win_row + _header_rows + _n + 1, 0);
    Cursor end_pane(_win_end.row() - 1, 0);
    //_text_panes[MTP_MATRIX_DATA] = TextPane(_me, _win, start_pane, end_pane);
    //_text_panes[MTP_TEXT] = TextPane(_me, _win, start_pane, end_pane);
    _text_panes.insert(
            pair<MatrixTextPart, TextPane>(MTP_MATRIX_DATA,
                TextPane(_me, _win, start_pane, end_pane)));
    _text_panes.insert(
            pair<MatrixTextPart, TextPane>(MTP_TEXT,
                TextPane(_me, _win, start_pane, end_pane)));
}

#if 0
MatrixPane::MatrixPane(size_t n, MatrixEditor* me, WINDOW* w)
    : WindowPane(me, w), _extra(0), _mp(MP_COEFFICIENT),
    _n(n), _c_i(0), _c_j(0), _s_i(0), _v_i(0), _e_i(0),
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
            _c_matrix[i][j] =
                MatrixEntry(_win, Cursor(i+_header_rows, win_col), _c_col_width);
        }
    }

    _s_vector.resize(_n);
    win_col += _col_spacing + _vector_spacing + _col_spacing
        + _pad + _s_col_width + 1;
    for (size_t i = 0; i < _n; i++)
    {
        _s_vector[i] =
            MatrixEntry(_win, Cursor(i+_header_rows, win_col), _s_col_width);
    }

    _v_vector.resize(_n);
    win_col += _col_spacing + _vector_spacing + _col_spacing
        + _pad + _v_col_width + 1;
    for (size_t i = 0; i < _n; i++)
    {
        _v_vector[i] =
            MatrixEntry(_win, Cursor(i+_header_rows, win_col), _v_col_width);
    }
}
#endif

MatrixPane::MatrixPane(const MatrixPane& rhs)
    : WindowPane(rhs),
    _c_matrix(rhs._c_matrix), _s_vector(rhs._s_vector), _v_vector(rhs._v_vector),
    _text_panes(rhs._text_panes), _mpp(rhs._mpp), _mp(rhs._mp), _mtp(rhs._mtp),
    _n(rhs._n), _c_i(rhs._c_i), _c_j(rhs._c_j), _s_i(rhs._s_i), _v_i(rhs._v_i),
    _c_col_width(rhs._c_col_width), _s_col_width(rhs._s_col_width),
    _v_col_width(rhs._v_col_width), _pad(rhs._pad), _col_spacing(rhs._col_spacing),
    _vector_spacing(rhs._vector_spacing), _header_rows(rhs._header_rows),
    _valid_screen_chars(rhs._valid_screen_chars),
    _key_mode_actions(rhs._key_mode_actions)
{
}

MatrixPane& MatrixPane::operator=(const MatrixPane& rhs)
{
    if (this == &rhs)
        return *this;

    WindowPane::operator=(rhs);

    _c_matrix = rhs._c_matrix;
    _s_vector = rhs._s_vector;
    _v_vector = rhs._v_vector;

    _text_panes = rhs._text_panes;

    _mpp = rhs._mpp;
    _mp = rhs._mp;
    _mtp = rhs._mtp;

    _n = rhs._n;
    _c_i = rhs._c_i;
    _c_j = rhs._c_j;
    _s_i = rhs._s_i;
    _v_i = rhs._v_i;

    _c_col_width = rhs._c_col_width;
    _s_col_width = rhs._s_col_width;
    _v_col_width = rhs._v_col_width;

    _pad = rhs._pad;
    _col_spacing = rhs._col_spacing;
    _vector_spacing = rhs._vector_spacing;
    _header_rows = rhs._header_rows;

    _valid_screen_chars = rhs._valid_screen_chars;
    _key_mode_actions = rhs._key_mode_actions;

    return *this;
}

MatrixPane::~MatrixPane(void)
{
}

void MatrixPane::key_action(int ch, EditorMode m)
{
    if ((_mpp != MPP_MATRIX) && (ch != ''))
    {
        _text_panes.find(_mtp)->second.key_action(ch, m);
    }
    else
    {
        if (_key_mode_actions.find(ch) == _key_mode_actions.end())
        {
            if ((m == MODE_INSERT) && is_print(ch))
                insert(ch);
            else if ((m == MODE_REPLACE) && is_print(ch))
                replace(ch);
        }
        else
        {
            switch (m)
            {
                case MODE_INSERT:
                    MEW_CALL(*this, _key_mode_actions[ch].insert_mode_action)(ch);
                    break;
                case MODE_EDIT:
                    MEW_CALL(*this, _key_mode_actions[ch].edit_mode_action)(ch);
                    break;
                case MODE_VISUAL:
                    MEW_CALL(*this, _key_mode_actions[ch].visual_mode_action)(ch);
                    break;
                case MODE_REPLACE:
                    MEW_CALL(*this, _key_mode_actions[ch].replace_mode_action)(ch);
                    break;
                default:
                    no_op(ch);
                    break;
            }
        }
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

void MatrixPane::draw(void)
{
    size_t erow = (_n - 1) / 2;
    size_t ecol = _vector_spacing / 2;
    int old_curs = curs_set(0);
    int y, x;
    getyx(_win, y, x);

    wmove(_win, _win_start.row(), 0);
    wattr_on(_win, A_BOLD, NULL);
    wprintw(_win,
            "[ Coefficient Matrix ]-1 x [ Constants Vector ] "
            "= [ Unknowns Vector ]\n");
    wattr_off(_win, A_BOLD, NULL);

    for (size_t i = 0; i < _n; i++)
    {
        wmove(_win, _win_start.row()+_header_rows+i, 0);

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

    //wclrtobot(_win);

    _text_panes.find(_mtp)->second.draw();

    curs_set(old_curs);
    wmove(_win, y, x);
}

WindowEntry& MatrixPane::entry(size_t row, size_t col)
{
    static WindowEntry not_found(_win, Cursor(1000, 1000));

    switch (_mp)
    {
        case MP_COEFFICIENT:
            if (row >= _n)
                row = _n - 1;

            if (col >= _n)
                col = _n - 1;

            return _c_matrix[row][col];

        case MP_CONSTANTS:
            if (row >= _n)
                row = _n - 1;

            return _s_vector[row];

        case MP_UNKNOWNS:
            if (row >= _n)
                row = _n - 1;

            return _v_vector[row];
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
    }

    return true;
}

void MatrixPane::refresh(void)
{
    if (_mpp == MPP_MATRIX)
    {
        if (adjust(entry().width()))
            draw();

        set();
    }
    else
    {
        _text_panes.find(_mtp)->second.set();
    }

    wrefresh(_win);
}

void MatrixPane::insert(int ch)
{
    entry().insert(ch);
}

void MatrixPane::remove(int ch)
{
    entry().remove();
}

void MatrixPane::clear_all(int ch)
{
    for (size_t i = 0; i < _n; i++)
    {
        for (size_t j = 0; j < _n; j++)
            _c_matrix[i][j].clear();
    }

    for (size_t i = 0; i < _n; i++)
        _s_vector[i].clear();

    for (size_t i = 0; i < _n; i++)
        _v_vector[i].clear();

    _c_i = _c_j = _s_i = _v_i = 0;
    _mp = MP_COEFFICIENT;

    set();
}

void MatrixPane::up(int ch)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            if (_c_i > 0)
                _c_i--;
            break;
        case MP_CONSTANTS:
            if (_s_i > 0)
                _s_i--;
            break;
        case MP_UNKNOWNS:
            if (_v_i > 0)
                _v_i--;
            break;
    }

    set();
}

void MatrixPane::down(int ch)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            if (_c_i < (_n - 1))
                _c_i++;
            break;
        case MP_CONSTANTS:
            if (_s_i < (_n - 1))
                _s_i++;
            break;
        case MP_UNKNOWNS:
            if (_v_i < (_n - 1))
                _v_i++;
            break;
    }

    set();
}

void MatrixPane::prev(int ch)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            if ((_c_i == 0) && (_c_j == 0))
            {
                if (_v_vector[0].empty())
                {
                    _mp = MP_CONSTANTS;
                    _s_i = _n - 1;
                }
                else
                {
                    _mp = MP_UNKNOWNS;
                    _v_i = _n - 1;
                }
            }
            else if (_c_j == 0)
            {
                _c_i--;
                _c_j = _n - 1;
            }
            else
            {
                _c_j--;
            }
            break;

        case MP_CONSTANTS:
            if (_s_i == 0)
            {
                _mp = MP_COEFFICIENT;
                _c_i = _c_j = _n - 1;
            }
            else
            {
                _s_i--;
            }
            break;

        case MP_UNKNOWNS:
            if (_v_i == 0)
            {
                _mp = MP_CONSTANTS;
                _s_i = _n - 1;
            }
            else
            {
                _v_i--;
            }
            break;
    }

    set();
}

void MatrixPane::next(int ch)
{
    switch (_mp)
    {
        case MP_COEFFICIENT:
            if ((_c_i == (_n - 1)) && (_c_j == (_n - 1)))
            {
                _mp = MP_CONSTANTS;
                _s_i = 0;
            }
            else if (_c_j == (_n - 1))
            {
                _c_i++;
                _c_j = 0;
            }
            else
            {
                _c_j++;
            }
            break;

        case MP_CONSTANTS:
            if (_s_i == (_n - 1))
            {
                if (_v_vector[0].empty())
                {
                    _mp = MP_COEFFICIENT;
                    _c_i = _c_j = 0;
                }
                else
                {
                    _mp = MP_UNKNOWNS;
                    _v_i = 0;
                }
            }
            else
            {
                _s_i++;
            }
            break;

        case MP_UNKNOWNS:
            if (_v_i == (_n - 1))
            {
                _mp = MP_COEFFICIENT;
                _c_i = _c_j = 0;
            }
            else
            {
                _v_i++;
            }
            break;
    }

    set();
}

void MatrixPane::pane_begin(int ch)
{
    _mp = MP_COEFFICIENT;
    _c_i = _c_j = 0;

    set();
}

void MatrixPane::pane_end(int ch)
{
    if (_v_vector[0].empty())
    {
        _mp = MP_CONSTANTS;
        _s_i = _n - 1;
    }
    else
    {
        _mp = MP_UNKNOWNS;
        _v_i = _n - 1;
    }

    set();
}

void MatrixPane::special(int ch)
{
    if (ch == '')
    {
        ch = wgetch(_win);

        switch (ch)
        {
            case KEY_UP:
                _mpp = MPP_MATRIX;
                set();
                break;

            case KEY_DOWN:
                _mpp = MPP_TEXT;
                draw();
                break;

            case KEY_LEFT:
                _mtp = MTP_MATRIX_DATA;
                draw();
                break;

            case KEY_RIGHT:
                _mtp = MTP_TEXT;
                draw();
                break;
        }
    }
    else
    {
        solve();
    }
}

void MatrixPane::update_color(short fg, short bg)
{
    _color_pair.pair(fg, bg);

    auto mit = _text_panes.find(MTP_MATRIX_DATA);
    mit->second.color().pair(fg, bg);

    auto tit = _text_panes.find(MTP_TEXT);
    tit->second.color().pair(fg, bg);

    wbkgd(_win, COLOR_PAIR(_color_pair.pair_number()));
    redraw();
}

void MatrixPane::solve(void)
{
    size_t i, j;
    int old_curs = curs_set(0);

    for (i = 0; i < _n; i++)
    {
        for (j = 0; j < _n; j++)
        {
            if (_c_matrix[i][j].empty())
            {
                ostringstream oss;

                oss << "Entry [" << i+1 << "][" << j+1
                    << "] in the coefficient matrix is empty";
                _me->error(oss.str());
                cancel(K_ESCAPE);

                _c_i = i;
                _c_j = j;
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

            _s_i = i;
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
            {
                A(i+1,j+1) =
                    Number<Scientific>::parse_expression(_c_matrix[i][j].data());
            }
        }
    }
    catch (NumberParsingException<Scientific>& e)
    {
        ostringstream oss;

        oss << "Error with expression in entry [" << i+1 << "]["
            << j+1 << "] of the coefficient matrix:\n";
        oss << "  " << e.str() << "\n";
        oss << "  " << setw(e.index()+1) << "^";

        _me->error(oss.str());
        cancel(K_ESCAPE);

        _c_i = i;
        _c_j = j;
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

        oss << "Error with expression in entry [" << i+1
            << "] of the solution vector:\n";
        oss << "  " << e.str() << "\n";
        oss << "  " << setw(e.index()+1) << "^";

        _me->error(oss.str());
        cancel(K_ESCAPE);

        _s_i = i;
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
        _c_i = _c_j = 0;

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
    _v_i = 0;

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

    _mtp = MTP_MATRIX_DATA;
    TextPane& mdata(_text_panes.find(_mtp)->second);

    mdata.clear_all('C');
    curs_row = 0;

    // Determinant
    str = "Determinant: ";
    {
        ostringstream oss;
        oss << A.determinant();
        str += oss.str();
    }
    mdata[curs_row++] = str;

    mdata[curs_row++] = "";

    // Cofactor matrix
    mdata[curs_row++] = "Cofactor Matrix:";
    {
        ostringstream oss;
        oss << A.cofactor_matrix();
        str = oss.str();

        while (!str.empty())
        {
            pos = str.find('\n');
            if (pos > 0)
                mdata[curs_row++] = str.substr(0, pos);
            str.erase(0, pos + 1);
        }
    }

    mdata[curs_row++] = "";

    // Adjoint matrix
    mdata[curs_row++] = "Adjoint:";
    {
        ostringstream oss;
        oss << A.adjoint();
        str = oss.str();

        while (!str.empty())
        {
            pos = str.find('\n');
            if (pos > 0)
                mdata[curs_row++] = str.substr(0, pos);
            str.erase(0, pos + 1);
        }
    }

    mdata[curs_row++] = "";

    // Inverse matrix
    mdata[curs_row++] = "Inverse:";
    {
        ostringstream oss;
        oss << A.inverse();
        str = oss.str();

        while (!str.empty())
        {
            pos = str.find('\n');
            if (pos > 0)
                mdata[curs_row++] = str.substr(0, pos);
            str.erase(0, pos + 1);
        }
    }

    mdata[curs_row++] = "";

    // Cramer's rule matices
    mdata[curs_row++] = "Cramer's rule:";
    for (j = 1; j <= A.rows(); j++)
    {
        ostringstream coss;
        streambuf *cout_buf = cout.rdbuf();
        cout.rdbuf(coss.rdbuf());

        Matrix<Scientific> CR = A.cramers_rule(s, j);

        str = coss.str();
        cout.rdbuf(cout_buf);

        pos = str.find('\n');
        mdata[curs_row++] = str.substr(0, pos);

        ostringstream oss;
        oss << CR;
        str = oss.str();

        while (!str.empty())
        {
            pos = str.find('\n');
            if (pos > 0)
                mdata[curs_row++] = str.substr(0, pos);
            str.erase(0, pos + 1);
        }

        mdata[curs_row++] = "";
    }

    mdata.pane_begin('g');
    mdata.pane_begin('g');
    mdata.draw();
    set();
    curs_set(old_curs);
    cancel(K_ESCAPE);
}

bool MatrixPane::empty(void) const
{
    for (size_t i = 0; i < _n; i++)
    {
        for (size_t j = 0; j < _n; j++)
        {
            if (!_c_matrix[i][j].empty())
                return false;
        }

        if (!_s_vector[i].empty())
            return false;

        if (!_v_vector[i].empty())
            return false;
    }

    return _text_panes.find(MTP_TEXT)->second.empty();
}

void MatrixPane::write(ostream& os) const
{
    if (empty())
        return;

    os << "[";
    for (size_t i = 0; i < _n; i++)
    {
        os << "[";
        for (size_t j = 0; j < _n; j++)
        {
            os << _c_matrix[i][j];
            if (j < (_n - 1))
                os << ",";
        }
        os << "]";
    }
    os << "]";

    os << "[";
    for (size_t i = 0; i < _n; i++)
        os << "[" << _s_vector[i] << "]";
    os << "]";

    os << "[";
    for (size_t i = 0; i < _n; i++)
        os << "[" << _v_vector[i] << "]";
    os << "]";

    os << "\n";

    os << _text_panes.find(MTP_TEXT)->second;

    os << '' << '\n';  // End of transmission block character 0x17
}

void MatrixPane::read(istream& is)
{
    static char buffer[1024];

    MatrixPane np(*this);

    is.getline(buffer, sizeof(buffer));
    string s(buffer);

    if (s.empty())
        return;

    size_t mstart = s.find("[[");
    size_t mend = s.find("]]");

    if (mstart == string::npos)
        return;

    size_t n = 1;
    size_t pos;
    string cm(s.substr(mstart, mend - mstart));
    while ((pos = cm.find("][", mstart)) != string::npos)
    {
        n++;
        mstart = pos + 2;
    }

    np._n = n;
    np._c_i = np._c_j = np._s_i = np._v_i = 0;
    np._mpp = MPP_MATRIX;
    np._mp = MP_COEFFICIENT;
    np._mtp = MTP_TEXT;

    np._c_matrix.clear();
    np._s_vector.clear();
    np._v_vector.clear();

    vector< vector<string> > c_matrix;
    vector<string> s_vector;
    vector<string> v_vector;

    np._c_matrix.resize(n);
    c_matrix.resize(n);
    for (size_t i = 0; i < n; i++)
        np._c_matrix[i].resize(n);

    np._s_vector.resize(n);
    np._v_vector.resize(n);

    mstart = s.find("[[");
    mend = s.find("]]");
    size_t index = 0;
    for (pos = mstart + 1; pos < mend + 1; pos++)
    {
        switch (s[pos])
        {
            case '[':
                mstart = pos + 1;
                break;
            case ',':
                c_matrix[index].push_back(s.substr(mstart, pos - mstart));
                mstart = pos + 1;
                break;
            case ']':
                c_matrix[index++].push_back(s.substr(mstart, pos - mstart));
                break;
        }
    }

    mstart = s.find("[[", ++pos);
    mend = s.find("]]", pos);
    for (pos = mstart + 1; pos < mend + 1; pos++)
    {
        switch (s[pos])
        {
            case '[':
                mstart = pos + 1;
                break;
            case ']':
                s_vector.push_back(s.substr(mstart, pos - mstart));
                break;
        }
    }

    mstart = s.find("[[", ++pos);
    mend = s.find("]]", pos);
    for (pos = mstart + 1; pos < mend + 1; pos++)
    {
        switch (s[pos])
        {
            case '[':
                mstart = pos + 1;
                break;
            case ']':
                v_vector.push_back(s.substr(mstart, pos - mstart));
                break;
        }
    }

    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = 0; j < n; j++)
        {
            if (c_matrix[i][j].size() > np._c_col_width)
                np._c_col_width = c_matrix[i][j].size();
        }

        if (s_vector[i].size() > np._s_col_width)
            np._s_col_width = s_vector[i].size();

        if (v_vector[i].size() > np._v_col_width)
            np._v_col_width = v_vector[i].size();
    }

    size_t win_col, win_row = np._win_start.row();
    for (size_t i = 0; i < n; i++)
    {
        win_col = 0;
        for (size_t j = 0; j < n; j++)
        {
            MatrixEntry::MatrixEntryPosition mep;

            //win_col = (j + 1) * (_col_spacing + _pad + _c_col_width) + j;
            //win_col = j + _c_col_width;

            if (j == 0)
                mep = MatrixEntry::MEP_START;
            else if (j < (n - 1))
                mep = MatrixEntry::MEP_MIDDLE;
            else
                mep = MatrixEntry::MEP_END;

            Cursor c(win_row + np._header_rows + i, win_col);
            np._c_matrix[i][j] = MatrixEntry(np._win, c, np._c_col_width, c_matrix[i][j], mep);

            win_col += np._c_matrix[i][j].width() + 3;
        }
    }

    win_col += np._vector_spacing + 1;
    for (size_t i = 0; i < n; i++)
    {
        Cursor c(win_row + np._header_rows + i, win_col);
        np._s_vector[i] = MatrixEntry(np._win, c, np._s_col_width, s_vector[i]);
    }

    win_col += np._s_vector[0].width() + 3 + np._vector_spacing + 1;
    for (size_t i = 0; i < n; i++)
    {
        Cursor c(win_row + np._header_rows + i, win_col);
        np._v_vector[i] = MatrixEntry(np._win, c, np._v_col_width, v_vector[i]);
    }

    Cursor start_pane(win_row + np._header_rows + n + 1, 0);
    Cursor end_pane(np._win_end.row() - 1, 0);

    np._text_panes.erase(np._text_panes.find(MTP_TEXT));

    vector<string> lines;
    while (is.peek() != '')
    {
        is.getline(buffer, sizeof(buffer));
        lines.push_back(string(buffer));
    }

    is.getline(buffer, sizeof(buffer));

    np._text_panes.insert(
            pair<MatrixTextPart, TextPane>(MTP_TEXT,
                TextPane(np._me, np._win, lines, start_pane, end_pane)));

    *this = np;
}

EvalPane::EvalPane(MatrixEditor* me, WINDOW* w)
    : WindowPane(me, w), _win_offset(2)
{
    Cursor c(_win_start);
    c >> _win_offset;
    _e_entry = EvalEntry(_win, c);
}

EvalPane::EvalPane(MatrixEditor* me, WINDOW* w,
        const Cursor& start, const Cursor& end)
    : WindowPane(me, w, start, end), _win_offset(2)
{
    Cursor c(_win_start);
    c >> _win_offset;
    _e_entry = EvalEntry(_win, c);
}

EvalPane::EvalPane(const EvalPane& rhs)
    : WindowPane(rhs), _win_offset(rhs._win_offset), _e_entry(rhs._e_entry)
{
}

EvalPane& EvalPane::operator=(const EvalPane& rhs)
{
    if (this == &rhs)
        return *this;

    WindowPane::operator=(rhs);

    _e_entry = rhs._e_entry;
    _win_offset = rhs._win_offset;
    _valid_screen_chars = rhs._valid_screen_chars;
    _key_mode_actions = rhs._key_mode_actions;

    return *this;
}

void EvalPane::key_action(int ch, EditorMode m)
{
    if (_key_mode_actions.find(ch) == _key_mode_actions.end())
    {
        if ((m == MODE_INSERT) && is_print(ch))
            insert(ch);
        else if ((m == MODE_REPLACE) && is_print(ch))
            replace(ch);
    }
    else
    {
        switch (m)
        {
            case MODE_INSERT:
                MEW_CALL(*this, _key_mode_actions[ch].insert_mode_action)(ch);
                break;
            case MODE_EDIT:
                MEW_CALL(*this, _key_mode_actions[ch].edit_mode_action)(ch);
                break;
            case MODE_VISUAL:
                MEW_CALL(*this, _key_mode_actions[ch].visual_mode_action)(ch);
                break;
            case MODE_REPLACE:
                MEW_CALL(*this, _key_mode_actions[ch].replace_mode_action)(ch);
                break;
            default:
                no_op(ch);
                break;
        }
    }
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

WindowEntry& EvalPane::entry(size_t row, size_t col)
{
    return _e_entry;
}

WindowEntry& EvalPane::entry(void)
{
    return _e_entry;
}

void EvalPane::draw(void)
{
    int old_curs = curs_set(0);
    int y, x;
    getyx(_win, y, x);

    wmove(_win, _win_start.row(), _win_start.col());
    wclrtoeol(_win);

    waddch(_win, (chtype)'>' | A_BOLD);
    waddch(_win, ' ');

    _e_entry.draw();

    wmove(_win, y, x);

    curs_set(old_curs);
}

void EvalPane::special(int ch)
{
    solve();
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

        wmove(_win, _win_start.row() + 2, 0);

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
        MatrixEditor::sig_handler->quit(sig);
}

void MatrixEditor::quit(int sig)
{
    if (!ncurses_initialized || (sig_handler != this))
        return;

    fini();
    exit(sig);
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
    // bool is_term_resized(lines, columns) TRUE or FALSE
    //   checks if resize_term would modify window structures
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

        if ((_mode == MODE_EDIT) && (ch == _cmd_char))
            process_command(_cmd_char);
        else
            _current_editor_window->key_action(ch, _mode);

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

void MatrixEditor::info(string e)
{
    if (!_initialized)
        return;

    int old_curs = curs_set(0);

    wclear(_error_window);
    wprintw(_error_window, "%s", e.c_str());
    wrefresh(_error_window);

    curs_set(old_curs);
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

    if (!_initialized)
        return;

    WINDOW* w = _command_window;

    waddch(w, ch);

    int y, x;
    int line_start = 1;
    int line_end = 1;
    bool exit_loop = false;

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

                exit_loop = true;
                break;

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

                exit_loop = true;
                break;

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

        if (exit_loop)
            break;

        getyx(w, y, x);
        if (x == 0)
            break;
    }

    wclear(_command_window);
    wrefresh(_command_window);
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
    int ch;
    short fg, bg;
    int old_curs = curs_set(0);

    (void)_current_editor_window->color().pair(&fg, &bg);

    wprintw(w, "Use left/right arrows to change foreground "
            "and up/down to change background ");

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

        _current_editor_window->update_color(fg, bg);

        wclear(w);
        mvwprintw(w, 0, 0, "Foreground: %d, Background: %d", fg, bg);
    }

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

void MatrixEditor::open(const vector<string>& args)
{
    string filename;

    if (!args.empty())
        filename = args[0];
    else
        filename = g_default_file;

    ifstream ifs(filename);

    if (!ifs.is_open())
    {
        error("Could not open file: \"" + filename + "\".");
        return;
    }

    //info("Opening file: \"" + filename + "\".");

    size_t next_pane = _matrix_panes.size();
    MatrixPane mp(2, this, _matrix_pane_window);
    while (ifs >> mp)
        _matrix_panes.push_back(mp);

    ifs.close();

    if (next_pane < _matrix_panes.size())
    {
        _current_matrix_pane = next_pane;
        _current_editor_window = &_matrix_panes[_current_matrix_pane];
        _current_editor_window->redraw();
    }
}

void MatrixEditor::write(const vector<string>& args)
{
    string filename;

    if (!args.empty())
        filename = args[0];
    else
        filename = g_default_file;

    ofstream ofs(filename, ios_base::app);

    if (!ofs.is_open())
    {
        error("Could not open file: \"" + filename + "\" to write to.");
        return;
    }

    for (size_t i = 0; i < _matrix_panes.size(); i++)
        ofs << _matrix_panes[i];

    //info("Finished writing to file: \"" + filename + "\".");

    ofs.close();
}

