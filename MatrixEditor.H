#ifndef _MATRIX_EDITOR_H
#define _MATRIX_EDITOR_H

#include <Matrix.H>
#include <ncurses.h>
#include <list>
#include <vector>
#include <string>
#include <map>
#include <bitset>
using namespace std;

#define K_ESCAPE      0x1B
#define K_NEWLINE     0x0A
#define K_TAB         0x09
#define K_BACKSPACE1  0x08
#define K_BACKSPACE2  0x7F

enum EditorMode
{
    MODE_INSERT,
    MODE_EDIT,
    MODE_VISUAL,
    MODE_REPLACE
};

struct ColorPair
{
    public:
        ColorPair(void) : _fg(0), _bg(0) { _init(); }
        ColorPair(short fg, short bg) : _fg(fg), _bg(bg) { _init(); }
        ColorPair(const ColorPair& cp) { _copy(cp); }

        ColorPair& operator=(const ColorPair& rhs)
        {
            if (this != &rhs) { _copy(rhs); }
            return *this;
        }

        ~ColorPair(void) { if (_pair_number > 0) { color_pairs[_pair_number]--; } }

        short pair_number(void) { return _pair_number; }
        short fg(void) { return _fg; }
        void fg(short fg) { _fg = fg; init_pair(_pair_number, _fg, _bg); }
        short bg(void) { return _bg; }
        void bg(short bg) { _bg = bg; init_pair(_pair_number, _fg, _bg); }
        void pair(short fg, short bg) { _fg = fg; _bg = bg; init_pair(_pair_number, fg, bg); }
        short pair(short* fg, short* bg) { *fg = _fg; *bg = _bg; return _pair_number; }

    private:
        static vector<int> color_pairs;

        short _pair_number;
        short _fg, _bg;

        void _init(void)
        {
            size_t i;
            for (i = 1; i < color_pairs.size(); i++)
            {
                if (color_pairs[i] == 0)
                    break;
            }
            
            if (i == color_pairs.size())
                color_pairs.resize(i + 1);

            color_pairs[i]++;

            _pair_number = i;
            init_pair(_pair_number, _fg, _bg);
        }

        void _copy(const ColorPair& cp)
        {
            _pair_number = cp._pair_number;
            _fg = cp._fg;
            _bg = cp._bg;

            color_pairs[_pair_number]++;
        }
};

class Cursor
{
    public:
        Cursor(void);
        Cursor(int row, int col);
        Cursor(const Cursor& cursor);
        Cursor& operator=(const Cursor& rhs);
        int operator+(int amount);
        int operator-(int amount);
        int operator>>(int amount);
        int operator<<(int amount);
        int row(void);
        int col(void);

    private:
        int _row;
        int _col;
};

class WindowEntry
{
    public:
        WindowEntry(void);
        WindowEntry(WINDOW* w, const Cursor& c);
        WindowEntry(WINDOW* w, const Cursor& c, string s);
        WindowEntry& operator=(const WindowEntry& rhs);
        virtual ~WindowEntry(void) {}

        WindowEntry& operator=(const string& rhs);
        virtual char& operator[](size_t position);

        virtual void set(void);
        virtual void draw(void);
        virtual void update(void);

        virtual int operator>>(int amount);
        virtual int operator<<(int amount);

        virtual void move(int amount);
        virtual void mpos(int pos);
        virtual void mbegin(void);
        virtual void mend(void);

        virtual void insert(char ch);
        virtual void insert(const string& s);

        virtual void remove(int count);
        virtual void remove(void);

        virtual void replace(char ch);
        virtual void replace(const string& s);

        virtual void clear(void);
        virtual void pclear(void);

        virtual void vstart(void);
        virtual void vstop(void);
        virtual void vhighlight(void);
        virtual void vtoggle(void);

        virtual void vmove(int amount);
        virtual void vmpos(int pos);
        virtual void vmbegin(void);
        virtual void vmend(void);

        virtual void vremove(void);
        virtual void vreplace(char ch);
        virtual void vreplace(const string& s);
        virtual void vclear(void);

        virtual string data(void);
        virtual string pdata(void);
        virtual string vdata(void);

        virtual int position(void);
        virtual size_t length(void);
        virtual size_t size(void);
        virtual bool empty(void);
        virtual Cursor& cursor(void);

        virtual size_t width(void);
        virtual void width(size_t w);

        virtual string info(void);

        struct AttrColor
        {
            public:
                AttrColor(attr_t attrs = A_NORMAL, short color_pair = -1)
                    : _attrs(attrs), _color_pair(color_pair) {}

                attr_t _attrs;
                short _color_pair;
        };

        typedef void (WindowEntry::*wattr_func_t)(int ch, AttrColor& ac);
        typedef void (*attr_func_t)(int ch, AttrColor& ac);
#define ATTR_CALL  ((*this).*(&WindowEntry::get_attrs))

        virtual void get_attrs(int ch, AttrColor& ac);
        void default_attrs(AttrColor& ac) { _default_attrs = ac; }
        const AttrColor& default_attrs(void) { return _default_attrs; }

    protected:
        string _data;
        int _pos;
        int _vstart;
        WINDOW* _win;
        Cursor _win_pos;
        AttrColor _default_attrs;
};

class HeaderEntry : public WindowEntry
{
    public:
        HeaderEntry(void);
        HeaderEntry(WINDOW* w, const Cursor& c);
        HeaderEntry(WINDOW* w, const Cursor& c, string s);
        HeaderEntry& operator=(const HeaderEntry& rhs);
        //virtual ~HeaderEntry(void);

        HeaderEntry& operator=(const string& rhs);
        //virtual char& operator[](size_t position);

        //virtual void set(void);
        //virtual void draw(void);
        //virtual void update(void);

        //virtual int operator>>(int amount);
        //virtual int operator<<(int amount);

        //virtual void move(int amount);
        //virtual void mpos(int pos);
        //virtual void mbegin(void);
        //virtual void mend(void);

        //virtual void insert(char ch);
        //virtual void insert(const string& s);

        //virtual void remove(int count);
        //virtual void remove(void);

        //virtual void replace(char ch);
        //virtual void replace(const string& s);

        //virtual void clear(void);
        //virtual void pclear(void);

        //virtual void vstart(void);
        //virtual void vstop(void);
        //virtual void vhighlight(void);
        //virtual void vtoggle(void);

        //virtual void vmove(int amount);
        //virtual void vmpos(int pos);
        //virtual void vmbegin(void);
        //virtual void vmend(void);

        //virtual void vremove(void);
        //virtual void vreplace(char ch);
        //virtual void vreplace(const string& s);
        //virtual void vclear(void);

        //virtual string data(void);
        //virtual string pdata(void);
        //virtual string vdata(void);

        //virtual int position(void);
        //virtual size_t length(void);
        //virtual size_t size(void);
        //virtual bool empty(void);
        //virtual Cursor& cursor(void);

        virtual string info(void);
        virtual void get_attrs(int ch, AttrColor& ac);

    private:
};

class EvalEntry : public WindowEntry
{
    public:
        EvalEntry(void);
        EvalEntry(WINDOW* w, const Cursor& c);
        EvalEntry(WINDOW* w, const Cursor& c, string s);
        EvalEntry& operator=(const EvalEntry& rhs);
        //virtual ~EvalEntry(void);

        EvalEntry& operator=(const string& rhs);
        //virtual char& operator[](size_t position);

        //virtual void set(void);
        //virtual void draw(void);
        //virtual void update(void);

        //virtual int operator>>(int amount);
        //virtual int operator<<(int amount);

        //virtual void move(int amount);
        //virtual void mpos(int pos);
        //virtual void mbegin(void);
        //virtual void mend(void);

        //virtual void insert(char ch);
        //virtual void insert(const string& s);

        //virtual void remove(int count);
        //virtual void remove(void);

        //virtual void replace(char ch);
        //virtual void replace(const string& s);

        //virtual void clear(void);
        //virtual void pclear(void);

        //virtual void vstart(void);
        //virtual void vstop(void);
        //virtual void vhighlight(void);
        //virtual void vtoggle(void);

        //virtual void vmove(int amount);
        //virtual void vmpos(int pos);
        //virtual void vmbegin(void);
        //virtual void vmend(void);

        //virtual void vremove(void);
        //virtual void vreplace(char ch);
        //virtual void vreplace(const string& s);
        //virtual void vclear(void);

        //virtual string data(void);
        //virtual string pdata(void);
        //virtual string vdata(void);

        //virtual int position(void);
        //virtual size_t length(void);
        //virtual size_t size(void);
        //virtual bool empty(void);
        //virtual Cursor& cursor(void);

        virtual string info(void);
        virtual void get_attrs(int ch, AttrColor& ac);

    private:
};

class MatrixEntry : public WindowEntry
{
    public:
        enum MatrixEntryPosition
        {
            MEP_ONE,
            MEP_START,
            MEP_MIDDLE,
            MEP_END,
        };

        MatrixEntry(void);
        MatrixEntry(WINDOW* w, const Cursor& c, MatrixEntryPosition mep = MEP_ONE);
        MatrixEntry(WINDOW* w, const Cursor& c, size_t width, MatrixEntryPosition mep = MEP_ONE);
        MatrixEntry(WINDOW* w, const Cursor& c, string s, MatrixEntryPosition mep = MEP_ONE);
        MatrixEntry(WINDOW* w, const Cursor& c, size_t width, string s, MatrixEntryPosition mep = MEP_ONE);
        MatrixEntry& operator=(const MatrixEntry& rhs);
        //virtual ~MatrixEntry(void);

        MatrixEntry& operator=(const string& rhs);
        //virtual char& operator[](size_t position);

        //virtual void set(void);
        virtual void draw(void);
        virtual void update(void);

        //virtual int operator>>(int amount);
        //virtual int operator<<(int amount);

        //virtual void move(int amount);
        //virtual void mpos(int pos);
        //virtual void mbegin(void);
        //virtual void mend(void);

        virtual void insert(char ch);
        virtual void insert(const string& s);

        virtual void remove(int count);
        virtual void remove(void);

        //virtual void replace(char ch);
        //virtual void replace(const string& s);

        //virtual void clear(void);
        //virtual void pclear(void);

        //virtual void vstart(void);
        //virtual void vstop(void);
        //virtual void vhighlight(void);
        //virtual void vtoggle(void);

        //virtual void vmove(int amount);
        //virtual void vmpos(int pos);
        //virtual void vmbegin(void);
        //virtual void vmend(void);

        //virtual void vremove(void);
        //virtual void vreplace(char ch);
        //virtual void vreplace(const string& s);
        //virtual void vclear(void);

        //virtual string data(void);
        //virtual string pdata(void);
        //virtual string vdata(void);

        //virtual int position(void);
        //virtual size_t length(void);
        //virtual size_t size(void);
        //virtual bool empty(void);
        //virtual Cursor& cursor(void);

        virtual string info(void);
        virtual void get_attrs(int ch, AttrColor& ac);

        size_t width(void);
        void width(size_t w);

    private:
        size_t _width;
        MatrixEntryPosition _mep;
        size_t _num_adornments;
};

class MatrixEditor;

class WindowPane
{
    public:
        WindowPane(MatrixEditor* me, WINDOW* w);
        WindowPane(const WindowPane& rhs);
        WindowPane& operator=(const WindowPane& rhs);
        ~WindowPane(void) {}

        typedef void (WindowPane::*KeyAction)(int ch);
#define MEW_CALL(object, member_func)   ((object).*(member_func))
        struct KeyModeAction
        {
            KeyAction insert_mode_action;
            KeyAction edit_mode_action;
            KeyAction visual_mode_action;
            KeyAction replace_mode_action;
        };

        virtual KeyAction key_action(int ch, EditorMode m);

        virtual bool is_print(int ch);
        virtual void we_addstr(string s);

        virtual WindowEntry& entry(size_t row, size_t col = 1) = 0;
        virtual WindowEntry& entry(void) = 0;

        virtual void set(void);
        virtual void draw(void);
        virtual void clear(void);
        virtual void redraw(void);
        virtual void refresh(void);

        virtual void insert_mode(int ch);
        virtual void edit_mode(int ch);
        virtual void visual_mode(int ch);
        virtual void replace_mode(int ch);
        virtual void command_mode(int ch);

        virtual void cancel(int ch);

        virtual void insert(int ch);
        virtual void remove(int ch);
        virtual void remove_left(int ch);
        virtual void clear_line(int ch);
        virtual void clear_to_eol(int ch);
        virtual void yank(int ch);
        virtual void paste(int ch);

        virtual void move(int ch);
        virtual void left(int ch);
        virtual void right(int ch);
        virtual void up(int ch);
        virtual void down(int ch);
        virtual void prev(int ch);
        virtual void next(int ch);
        virtual void line_begin(int ch);
        virtual void line_end(int ch);
        virtual void pane_begin(int ch);
        virtual void pane_end(int ch);

        virtual void history_prev(int ch);
        virtual void history_next(int ch);
        virtual void history_delete(int ch);

        virtual void visual_cancel(int ch);
        virtual void visual_move(int ch);
        virtual void visual_left(int ch);
        virtual void visual_right(int ch);
        virtual void visual_up(int ch);
        virtual void visual_down(int ch);
        virtual void visual_line_begin(int ch);
        virtual void visual_line_end(int ch);
        virtual void visual_pane_begin(int ch);
        virtual void visual_pane_end(int ch);

        virtual void visual_remove(int ch);
        virtual void visual_replace(int ch);
        virtual void visual_clear(int ch);
        virtual void visual_yank(int ch);
        virtual void visual_paste(int ch);
        virtual void visual_toggle(int ch);

        virtual void replace_cancel(int ch);
        virtual void replace(int ch);
        virtual void replace_move(int ch);
        virtual void replace_one(int ch);

        virtual void special(int ch) {}
        virtual void no_op(int ch) {}

        WINDOW* window(void);
        ColorPair& color(void);

    protected:
        MatrixEditor* _me;
        WINDOW* _win;
        ColorPair _color_pair;
        int _clear_toggle;
        int _yank_toggle;
        string _replaced;
        vector<string> _history;
        size_t _cur_hist_entry;

    private:
        map<int, KeyModeAction> _key_mode_actions = {
            {
                K_ESCAPE,
                { 
                    &WindowPane::edit_mode,
                    &WindowPane::cancel,
                    &WindowPane::visual_cancel,
                    &WindowPane::replace_cancel
                }
            },
            {
                KEY_BACKSPACE,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::move
                }
            },
            {
                K_BACKSPACE1,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::move
                }
            },
            {
                K_BACKSPACE2,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::move
                }
            },
            {
                KEY_DC,
                {
                    &WindowPane::remove,
                    &WindowPane::remove,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'x',
                {
                    &WindowPane::insert,
                    &WindowPane::remove,
                    &WindowPane::visual_remove,
                    &WindowPane::replace
                }
            },
            {
                'd',
                {
                    &WindowPane::insert,
                    &WindowPane::clear_line,
                    &WindowPane::visual_remove,
                    &WindowPane::replace
                }
            },
            {
                'D',
                {
                    &WindowPane::insert,
                    &WindowPane::clear_to_eol,
                    &WindowPane::visual_clear,
                    &WindowPane::replace
                }
            },
            {
                'p',
                {
                    &WindowPane::insert,
                    &WindowPane::paste,
                    &WindowPane::visual_paste,
                    &WindowPane::replace
                }
            },
            {
                'P',
                {
                    &WindowPane::insert,
                    &WindowPane::paste,
                    &WindowPane::visual_paste,
                    &WindowPane::replace
                }
            },
            {
                'r',
                {
                    &WindowPane::insert,
                    &WindowPane::replace_one,
                    &WindowPane::visual_replace,
                    &WindowPane::replace
                }
            },
            {
                'R',
                {
                    &WindowPane::insert,
                    &WindowPane::replace_mode,
                    &WindowPane::visual_replace,
                    &WindowPane::replace
                }
            },
            {
                K_NEWLINE,
                {
                    &WindowPane::insert,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::replace
                }
            },
            {
                KEY_ENTER,
                {
                    &WindowPane::insert,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::replace
                }
            },
            {
                KEY_HOME,
                {
                    &WindowPane::line_begin,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::replace,
                }
            },
            {
                '^',
                {
                    &WindowPane::insert,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::replace
                }
            },
            {
                '_',
                {
                    &WindowPane::insert,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::replace
                }
            },
            {
                '$',
                {
                    &WindowPane::insert,
                    &WindowPane::line_end,
                    &WindowPane::visual_line_end,
                    &WindowPane::replace
                }
            },
            {
                'g',
                {
                    &WindowPane::insert,
                    &WindowPane::pane_begin,
                    &WindowPane::visual_pane_begin,
                    &WindowPane::replace
                }
            },
            {
                'G',
                {
                    &WindowPane::insert,
                    &WindowPane::pane_end,
                    &WindowPane::visual_pane_end,
                    &WindowPane::replace
                }
            },
            {
                KEY_LEFT,
                {
                    &WindowPane::left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::move
                }
            },
            {
                'h',
                {
                    &WindowPane::insert,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace
                }
            },
            {
                KEY_RIGHT,
                {
                    &WindowPane::right,
                    &WindowPane::right,
                    &WindowPane::visual_right,
                    &WindowPane::move
                }
            },
            {
                'l',
                {
                    &WindowPane::insert,
                    &WindowPane::right,
                    &WindowPane::visual_right,
                    &WindowPane::replace
                }
            },
            {
                KEY_UP,
                {
                    &WindowPane::up,
                    &WindowPane::up,
                    &WindowPane::visual_up,
                    &WindowPane::move
                }
            },
            {
                'k',
                {
                    &WindowPane::insert,
                    &WindowPane::up,
                    &WindowPane::visual_up,
                    &WindowPane::replace
                }
            },
            {
                '-',
                {
                    &WindowPane::insert,
                    &WindowPane::up,
                    &WindowPane::visual_up,
                    &WindowPane::replace
                }
            },
            {
                KEY_DOWN,
                {
                    &WindowPane::down,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::move
                }
            },
            {
                'j',
                {
                    &WindowPane::insert,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::replace
                }
            },
            {
                '+',
                {
                    &WindowPane::insert,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::replace
                }
            },
            {
                K_TAB,
                {
                    &WindowPane::insert,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                KEY_BTAB,
                {
                    &WindowPane::insert,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                'i',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                'a',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                'o',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::visual_toggle,
                    &WindowPane::replace
                }
            },
            {
                'O',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::visual_toggle,
                    &WindowPane::replace
                }
            },
            {
                's',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::visual_remove,
                    &WindowPane::replace
                }
            },
            {
                'v',
                {
                    &WindowPane::insert,
                    &WindowPane::visual_mode,
                    &WindowPane::visual_cancel,
                    &WindowPane::replace
                }
            },
            {
                'y',
                {
                    &WindowPane::insert,
                    &WindowPane::yank,
                    &WindowPane::visual_yank,
                    &WindowPane::replace
                }
            },
            {
                ':',
                {
                    &WindowPane::insert,
                    &WindowPane::command_mode,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
        };
};

class TextPane : public WindowPane
{
    public:
        TextPane(MatrixEditor* me, WINDOW* w);
        TextPane(const TextPane& rhs);
        TextPane& operator=(const TextPane& rhs);
        ~TextPane(void) {}

        virtual KeyAction key_action(int ch, EditorMode m);

        //virtual bool is_print(int ch);
        //virtual void we_addstr(string s);

        virtual WindowEntry& entry(size_t row, size_t col = 1);
        virtual WindowEntry& entry(void);

        //virtual void set(void);
        virtual void draw(void);
        //virtual void clear(void);
        //virtual void redraw(void);
        //virtual void refresh(void);

        //virtual void insert_mode(int ch);
        //virtual void edit_mode(int ch);
        //virtual void visual_mode(int ch);
        //virtual void replace_mode(int ch);
        //virtual void command_mode(int ch);

        //virtual void cancel(int ch);

        //virtual void insert(int ch);
        //virtual void remove(int ch);
        //virtual void remove_left(int ch);
        //virtual void clear_line(int ch);
        //virtual void clear_to_eol(int ch);
        //virtual void yank(int ch);
        //virtual void paste(int ch);

        //virtual void move(int ch);
        //virtual void left(int ch);
        //virtual void right(int ch);
        virtual void up(int ch);
        virtual void down(int ch);
        //virtual void prev(int ch);
        //virtual void next(int ch);
        //virtual void line_begin(int ch);
        //virtual void line_end(int ch);
        virtual void pane_begin(int ch);
        virtual void pane_end(int ch);

        //virtual void history_prev(int ch);
        //virtual void history_next(int ch);
        //virtual void history_delete(int ch);

        //virtual void visual_cancel(int ch);
        //virtual void visual_move(int ch);
        //virtual void visual_left(int ch);
        //virtual void visual_right(int ch);
        //virtual void visual_up(int ch);
        //virtual void visual_down(int ch);
        //virtual void visual_line_begin(int ch);
        //virtual void visual_line_end(int ch);
        //virtual void visual_pane_begin(int ch);
        //virtual void visual_pane_end(int ch);

        //virtual void visual_remove(int ch);
        //virtual void visual_replace(int ch);
        //virtual void visual_clear(int ch);
        //virtual void visual_yank(int ch);
        //virtual void visual_paste(int ch);
        //virtual void visual_toggle(int ch);

        //virtual void replace_cancel(int ch);
        //virtual void replace(int ch);
        //virtual void replace_move(int ch);
        //virtual void replace_one(int ch);

        virtual void special(int ch);
        //virtual void no_op(int ch) {}

    private:
        vector<WindowEntry> _entries;
        size_t _cur_entry;

        map<int, KeyModeAction> _key_mode_actions = {
            {
                K_ESCAPE,
                { 
                    &WindowPane::edit_mode,
                    &WindowPane::cancel,
                    &WindowPane::visual_cancel,
                    &WindowPane::replace_cancel
                }
            },
            {
                KEY_BACKSPACE,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::move
                }
            },
            {
                K_BACKSPACE1,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::move
                }
            },
            {
                K_BACKSPACE2,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::move
                }
            },
            {
                KEY_DC,
                {
                    &WindowPane::remove,
                    &WindowPane::remove,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'x',
                {
                    &WindowPane::insert,
                    &WindowPane::remove,
                    &WindowPane::visual_remove,
                    &WindowPane::replace
                }
            },
            {
                'd',
                {
                    &WindowPane::insert,
                    &WindowPane::clear_line,
                    &WindowPane::visual_remove,
                    &WindowPane::replace
                }
            },
            {
                'D',
                {
                    &WindowPane::insert,
                    &WindowPane::clear_to_eol,
                    &WindowPane::visual_clear,
                    &WindowPane::replace
                }
            },
            {
                'p',
                {
                    &WindowPane::insert,
                    &WindowPane::paste,
                    &WindowPane::visual_paste,
                    &WindowPane::replace
                }
            },
            {
                'P',
                {
                    &WindowPane::insert,
                    &WindowPane::paste,
                    &WindowPane::visual_paste,
                    &WindowPane::replace
                }
            },
            {
                'r',
                {
                    &WindowPane::insert,
                    &WindowPane::replace_one,
                    &WindowPane::visual_replace,
                    &WindowPane::replace
                }
            },
            {
                'R',
                {
                    &WindowPane::insert,
                    &WindowPane::replace_mode,
                    &WindowPane::visual_replace,
                    &WindowPane::replace
                }
            },
            {
                K_NEWLINE,
                {
                    &WindowPane::special,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::replace
                }
            },
            {
                KEY_ENTER,
                {
                    &WindowPane::special,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::replace
                }
            },
            {
                KEY_HOME,
                {
                    &WindowPane::line_begin,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::replace,
                }
            },
            {
                '^',
                {
                    &WindowPane::insert,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::replace
                }
            },
            {
                '_',
                {
                    &WindowPane::insert,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::replace
                }
            },
            {
                '$',
                {
                    &WindowPane::insert,
                    &WindowPane::line_end,
                    &WindowPane::visual_line_end,
                    &WindowPane::replace
                }
            },
            {
                'g',
                {
                    &WindowPane::insert,
                    &WindowPane::pane_begin,
                    &WindowPane::visual_pane_begin,
                    &WindowPane::replace
                }
            },
            {
                'G',
                {
                    &WindowPane::insert,
                    &WindowPane::pane_end,
                    &WindowPane::visual_pane_end,
                    &WindowPane::replace
                }
            },
            {
                KEY_LEFT,
                {
                    &WindowPane::left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::move
                }
            },
            {
                'h',
                {
                    &WindowPane::insert,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace
                }
            },
            {
                KEY_RIGHT,
                {
                    &WindowPane::right,
                    &WindowPane::right,
                    &WindowPane::visual_right,
                    &WindowPane::move
                }
            },
            {
                'l',
                {
                    &WindowPane::insert,
                    &WindowPane::right,
                    &WindowPane::visual_right,
                    &WindowPane::replace
                }
            },
            {
                KEY_UP,
                {
                    &WindowPane::up,
                    &WindowPane::up,
                    &WindowPane::visual_up,
                    &WindowPane::move
                }
            },
            {
                'k',
                {
                    &WindowPane::insert,
                    &WindowPane::up,
                    &WindowPane::visual_up,
                    &WindowPane::replace
                }
            },
            {
                '-',
                {
                    &WindowPane::insert,
                    &WindowPane::up,
                    &WindowPane::visual_up,
                    &WindowPane::replace
                }
            },
            {
                KEY_DOWN,
                {
                    &WindowPane::down,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::move
                }
            },
            {
                'j',
                {
                    &WindowPane::insert,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::replace
                }
            },
            {
                '+',
                {
                    &WindowPane::insert,
                    &WindowPane::down,
                    &WindowPane::visual_down,
                    &WindowPane::replace
                }
            },
            {
                K_TAB,
                {
                    &WindowPane::insert,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                KEY_BTAB,
                {
                    &WindowPane::insert,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                'i',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                'a',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                'o',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::visual_toggle,
                    &WindowPane::replace
                }
            },
            {
                'O',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::visual_toggle,
                    &WindowPane::replace
                }
            },
            {
                's',
                {
                    &WindowPane::insert,
                    &WindowPane::insert_mode,
                    &WindowPane::visual_remove,
                    &WindowPane::replace
                }
            },
            {
                'v',
                {
                    &WindowPane::insert,
                    &WindowPane::visual_mode,
                    &WindowPane::visual_cancel,
                    &WindowPane::replace
                }
            },
            {
                'y',
                {
                    &WindowPane::insert,
                    &WindowPane::yank,
                    &WindowPane::visual_yank,
                    &WindowPane::replace
                }
            },
            {
                ':',
                {
                    &WindowPane::insert,
                    &WindowPane::command_mode,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
        };
};

class MatrixPane : public WindowPane
{
    public:
        MatrixPane(size_t n, MatrixEditor* me, WINDOW* w);
        MatrixPane(const MatrixPane& rhs);
        MatrixPane& operator=(const MatrixPane& rhs);
        virtual ~MatrixPane(void);

        virtual KeyAction key_action(int ch, EditorMode m);

        virtual bool is_print(int ch);
        virtual void we_addstr(string s);

        virtual WindowEntry& entry(size_t row, size_t col = 1);
        virtual WindowEntry& entry(void);

        virtual void set(void);
        virtual void draw(void);
        //virtual void clear(void);
        //virtual void redraw(void);
        virtual void refresh(void);

        bool adjust(size_t width);

        //virtual void insert_mode(int ch);
        //virtual void edit_mode(int ch);
        //virtual void visual_mode(int ch);
        //virtual void replace_mode(int ch);
        //virtual void command_mode(int ch);

        //virtual void cancel(int ch);

        virtual void insert(int ch);
        virtual void remove(int ch);
        //virtual void remove_left(int ch);
        //virtual void clear_line(int ch);
        //virtual void clear_to_eol(int ch);
        //virtual void yank(int ch);
        //virtual void paste(int ch);

        //virtual void move(int ch);
        //virtual void left(int ch);
        //virtual void right(int ch);
        virtual void up(int ch);
        virtual void down(int ch);
        virtual void prev(int ch);
        virtual void next(int ch);
        //virtual void line_begin(int ch);
        //virtual void line_end(int ch);
        virtual void pane_begin(int ch);
        virtual void pane_end(int ch);

        //virtual void history_prev(int ch);
        //virtual void history_next(int ch);
        //virtual void history_delete(int ch);

        //virtual void visual_cancel(int ch);
        //virtual void visual_move(int ch);
        //virtual void visual_left(int ch);
        //virtual void visual_right(int ch);
        //virtual void visual_up(int ch);
        //virtual void visual_down(int ch);
        //virtual void visual_line_begin(int ch);
        //virtual void visual_line_end(int ch);
        //virtual void visual_pane_begin(int ch);
        //virtual void visual_pane_end(int ch);

        //virtual void visual_remove(int ch);
        //virtual void visual_replace(int ch);
        //virtual void visual_clear(int ch);
        //virtual void visual_yank(int ch);
        //virtual void visual_paste(int ch);
        //virtual void visual_toggle(int ch);

        //virtual void replace_cancel(int ch);
        //virtual void replace(int ch);
        //virtual void replace_move(int ch);
        //virtual void replace_one(int ch);

        //virtual void replace_cancel(int ch);

        //virtual void no_op(int ch) {}
        virtual void special(int ch);
        void solve(void);

        enum MatrixPanePart
        {
            MP_COEFFICIENT,
            MP_CONSTANTS,
            MP_UNKNOWNS,
            MP_EXTRA_DATA,
        };

    private:
        vector< vector<MatrixEntry> > _c_matrix;
        vector<MatrixEntry> _s_vector;
        vector<MatrixEntry> _v_vector;
        vector<WindowEntry*> _extra_data;
        MatrixPanePart _mp;
        size_t _n, _c_i, _c_j, _s_i, _v_i, _e_i;
        size_t _c_col_width, _s_col_width, _v_col_width;
        size_t _pad = 1;
        size_t _col_spacing = 1;
        size_t _vector_spacing = 3;
        size_t _header_rows = 2;
        Matrix<Scientific> _last_A, _last_s, _last_v;

        vector<int> _valid_screen_chars = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.',
            '(', ')', ' ',
            '-', '+', '*', '/',
            'e', 'E', '^'
        };

        map<int, WindowPane::KeyModeAction> _key_mode_actions = {
            {
                K_ESCAPE,
                { 
                    &WindowPane::edit_mode,
                    &WindowPane::cancel,
                    &WindowPane::visual_cancel,
                    &WindowPane::replace_cancel
                }
            },
            {
                KEY_BACKSPACE,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace_move
                }
            },
            {
                K_BACKSPACE1,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace_move
                }
            },
            {
                K_BACKSPACE2,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace_move
                }
            },
            {
                KEY_DC,
                {
                    &WindowPane::remove,
                    &WindowPane::remove,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'x',
                {
                    &WindowPane::no_op,
                    &WindowPane::remove,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'd',
                {
                    &WindowPane::no_op,
                    &WindowPane::clear_line,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'D',
                {
                    &WindowPane::no_op,
                    &WindowPane::clear_to_eol,
                    &WindowPane::visual_clear,
                    &WindowPane::no_op
                }
            },
            {
                'p',
                {
                    &WindowPane::no_op,
                    &WindowPane::paste,
                    &WindowPane::visual_paste,
                    &WindowPane::no_op
                }
            },
            {
                'P',
                {
                    &WindowPane::no_op,
                    &WindowPane::paste,
                    &WindowPane::visual_paste,
                    &WindowPane::no_op
                }
            },
            {
                'r',
                {
                    &WindowPane::no_op,
                    &WindowPane::replace_one,
                    &WindowPane::visual_replace,
                    &WindowPane::no_op
                }
            },
            {
                'R',
                {
                    &WindowPane::insert,
                    &WindowPane::replace_mode,
                    &WindowPane::visual_replace,
                    &WindowPane::no_op
                }
            },
            {
                K_NEWLINE,
                {
                    &WindowPane::special,
                    &WindowPane::special,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                KEY_ENTER,
                {
                    &WindowPane::special,
                    &WindowPane::special,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                KEY_HOME,
                {
                    &WindowPane::line_begin,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::no_op
                }
            },
            {
                '^',
                {
                    &WindowPane::insert,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::replace
                }
            },
            {
                '_',
                {
                    &WindowPane::no_op,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::no_op
                }
            },
            {
                '$',
                {
                    &WindowPane::no_op,
                    &WindowPane::line_end,
                    &WindowPane::visual_line_end,
                    &WindowPane::no_op
                }
            },
            {
                'g',
                {
                    &WindowPane::no_op,
                    &WindowPane::pane_begin,
                    &WindowPane::visual_pane_begin,
                    &WindowPane::no_op
                }
            },
            {
                'G',
                {
                    &WindowPane::no_op,
                    &WindowPane::pane_end,
                    &WindowPane::visual_pane_end,
                    &WindowPane::no_op
                }
            },
            {
                KEY_LEFT,
                {
                    &WindowPane::left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace_move
                }
            },
            {
                'h',
                {
                    &WindowPane::no_op,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::no_op
                }
            },
            {
                KEY_RIGHT,
                {
                    &WindowPane::right,
                    &WindowPane::right,
                    &WindowPane::visual_right,
                    &WindowPane::replace_move
                }
            },
            {
                'l',
                {
                    &WindowPane::no_op,
                    &WindowPane::right,
                    &WindowPane::visual_right,
                    &WindowPane::no_op
                }
            },
            {
                KEY_UP,
                {
                    &WindowPane::history_prev,
                    &WindowPane::history_prev,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'k',
                {
                    &WindowPane::no_op,
                    &WindowPane::up,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                '-',
                {
                    &WindowPane::insert,
                    &WindowPane::up,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                KEY_DOWN,
                {
                    &WindowPane::history_next,
                    &WindowPane::history_next,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'j',
                {
                    &WindowPane::no_op,
                    &WindowPane::down,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                '+',
                {
                    &WindowPane::insert,
                    &WindowPane::down,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                K_TAB,
                {
                    &WindowPane::next,
                    &WindowPane::next,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'n',
                {
                    &WindowPane::next,
                    &WindowPane::next,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                }
            },
            {
                KEY_BTAB,
                {
                    &WindowPane::prev,
                    &WindowPane::prev,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'b',
                {
                    &WindowPane::prev,
                    &WindowPane::prev,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'i',
                {
                    &WindowPane::no_op,
                    &WindowPane::insert_mode,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'a',
                {
                    &WindowPane::no_op,
                    &WindowPane::insert_mode,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'o',
                {
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::visual_toggle,
                    &WindowPane::no_op
                }
            },
            {
                'O',
                {
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::visual_toggle,
                    &WindowPane::no_op
                }
            },
            {
                's',
                {
                    &WindowPane::no_op,
                    &WindowPane::insert_mode,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'v',
                {
                    &WindowPane::no_op,
                    &WindowPane::visual_mode,
                    &WindowPane::visual_cancel,
                    &WindowPane::no_op
                }
            },
            {
                'y',
                {
                    &WindowPane::no_op,
                    &WindowPane::yank,
                    &WindowPane::visual_yank,
                    &WindowPane::no_op
                }
            },
            {
                'e',
                {
                    &WindowPane::insert,
                    &WindowPane::history_delete,
                    &WindowPane::no_op,
                    &WindowPane::replace,
                }
            },
            {
                ':',
                {
                    &WindowPane::no_op,
                    &WindowPane::command_mode,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
        };
};


class EvalPane : public WindowPane
{
    public:
        EvalPane(MatrixEditor* me, WINDOW* w);
        EvalPane(const EvalPane& rhs);
        EvalPane& operator=(const EvalPane& rhs);
        virtual ~EvalPane(void) {}

        virtual KeyAction key_action(int ch, EditorMode m);

        virtual bool is_print(int ch);
        virtual void we_addstr(string s);

        virtual WindowEntry& entry(size_t row, size_t col = 1);
        virtual WindowEntry& entry(void);

        //virtual void set(void);
        virtual void draw(void);
        //virtual void clear(void);
        //virtual void redraw(void);
        //virtual void refresh(void);

        //virtual void insert_mode(int ch);
        //virtual void edit_mode(int ch);
        //virtual void visual_mode(int ch);
        //virtual void replace_mode(int ch);
        //virtual void command_mode(int ch);

        //virtual void cancel(int ch);

        //virtual void insert(int ch);
        //virtual void remove(int ch);
        //virtual void remove_left(int ch);
        //virtual void clear_line(int ch);
        //virtual void clear_to_eol(int ch);
        //virtual void yank(int ch);
        //virtual void paste(int ch);

        //virtual void move(int ch);
        //virtual void left(int ch);
        //virtual void right(int ch);
        //virtual void up(int ch);
        //virtual void down(int ch);
        //virtual void prev(int ch);
        //virtual void next(int ch);
        //virtual void line_begin(int ch);
        //virtual void line_end(int ch);
        //virtual void pane_begin(int ch);
        //virtual void pane_end(int ch);

        //virtual void history_prev(int ch);
        //virtual void history_next(int ch);
        //virtual void history_delete(int ch);

        //virtual void visual_cancel(int ch);
        //virtual void visual_move(int ch);
        //virtual void visual_left(int ch);
        //virtual void visual_right(int ch);
        //virtual void visual_up(int ch);
        //virtual void visual_down(int ch);
        //virtual void visual_line_begin(int ch);
        //virtual void visual_line_end(int ch);
        //virtual void visual_pane_begin(int ch);
        //virtual void visual_pane_end(int ch);

        //virtual void visual_remove(int ch);
        //virtual void visual_replace(int ch);
        //virtual void visual_clear(int ch);
        //virtual void visual_yank(int ch);
        //virtual void visual_paste(int ch);
        //virtual void visual_toggle(int ch);

        //virtual void replace_cancel(int ch);
        //virtual void replace(int ch);
        //virtual void replace_move(int ch);
        //virtual void replace_one(int ch);

        //virtual void no_op(int ch) {}
        virtual void special(int ch);

        void solve(void);

    private:
        int _win_offset;
        EvalEntry _e_entry;

        vector<int> _valid_screen_chars = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.',
            '(', ')', ' ',
            '-', '+', '*', '/',
            'e', 'E', '^'
        };

        map<int, WindowPane::KeyModeAction> _key_mode_actions = {
            {
                K_ESCAPE,
                { 
                    &WindowPane::edit_mode,
                    &WindowPane::cancel,
                    &WindowPane::visual_cancel,
                    &WindowPane::replace_cancel
                }
            },
            {
                KEY_BACKSPACE,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace_move
                }
            },
            {
                K_BACKSPACE1,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace_move
                }
            },
            {
                K_BACKSPACE2,
                {
                    &WindowPane::remove_left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace_move
                }
            },
            {
                KEY_DC,
                {
                    &WindowPane::remove,
                    &WindowPane::remove,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'x',
                {
                    &WindowPane::no_op,
                    &WindowPane::remove,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'd',
                {
                    &WindowPane::no_op,
                    &WindowPane::clear_line,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'D',
                {
                    &WindowPane::no_op,
                    &WindowPane::clear_to_eol,
                    &WindowPane::visual_clear,
                    &WindowPane::no_op
                }
            },
            {
                'p',
                {
                    &WindowPane::no_op,
                    &WindowPane::paste,
                    &WindowPane::visual_paste,
                    &WindowPane::no_op
                }
            },
            {
                'P',
                {
                    &WindowPane::no_op,
                    &WindowPane::paste,
                    &WindowPane::visual_paste,
                    &WindowPane::no_op
                }
            },
            {
                'r',
                {
                    &WindowPane::no_op,
                    &WindowPane::replace_one,
                    &WindowPane::visual_replace,
                    &WindowPane::no_op
                }
            },
            {
                'R',
                {
                    &WindowPane::insert,
                    &WindowPane::replace_mode,
                    &WindowPane::visual_replace,
                    &WindowPane::no_op
                }
            },
            {
                K_NEWLINE,
                {
                    &WindowPane::special,
                    &WindowPane::special,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                KEY_ENTER,
                {
                    &WindowPane::special,
                    &WindowPane::special,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                KEY_HOME,
                {
                    &WindowPane::line_begin,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::no_op
                }
            },
            {
                '^',
                {
                    &WindowPane::insert,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::replace
                }
            },
            {
                '_',
                {
                    &WindowPane::no_op,
                    &WindowPane::line_begin,
                    &WindowPane::visual_line_begin,
                    &WindowPane::no_op
                }
            },
            {
                '$',
                {
                    &WindowPane::no_op,
                    &WindowPane::line_end,
                    &WindowPane::visual_line_end,
                    &WindowPane::no_op
                }
            },
            {
                'g',
                {
                    &WindowPane::no_op,
                    &WindowPane::pane_begin,
                    &WindowPane::visual_pane_begin,
                    &WindowPane::no_op
                }
            },
            {
                'G',
                {
                    &WindowPane::no_op,
                    &WindowPane::pane_end,
                    &WindowPane::visual_pane_end,
                    &WindowPane::no_op
                }
            },
            {
                KEY_LEFT,
                {
                    &WindowPane::left,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::replace_move
                }
            },
            {
                'h',
                {
                    &WindowPane::no_op,
                    &WindowPane::left,
                    &WindowPane::visual_left,
                    &WindowPane::no_op
                }
            },
            {
                KEY_RIGHT,
                {
                    &WindowPane::right,
                    &WindowPane::right,
                    &WindowPane::visual_right,
                    &WindowPane::replace_move
                }
            },
            {
                'l',
                {
                    &WindowPane::no_op,
                    &WindowPane::right,
                    &WindowPane::visual_right,
                    &WindowPane::no_op
                }
            },
            {
                KEY_UP,
                {
                    &WindowPane::history_prev,
                    &WindowPane::history_prev,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'k',
                {
                    &WindowPane::no_op,
                    &WindowPane::up,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                '-',
                {
                    &WindowPane::insert,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::replace,
                }
            },
            {
                KEY_DOWN,
                {
                    &WindowPane::history_next,
                    &WindowPane::history_next,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'j',
                {
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                '+',
                {
                    &WindowPane::insert,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::replace
                }
            },
            {
                K_TAB,
                {
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                KEY_BTAB,
                {
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'i',
                {
                    &WindowPane::no_op,
                    &WindowPane::insert_mode,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'a',
                {
                    &WindowPane::no_op,
                    &WindowPane::insert_mode,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
            {
                'o',
                {
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::visual_toggle,
                    &WindowPane::no_op
                }
            },
            {
                'O',
                {
                    &WindowPane::no_op,
                    &WindowPane::no_op,
                    &WindowPane::visual_toggle,
                    &WindowPane::no_op
                }
            },
            {
                's',
                {
                    &WindowPane::no_op,
                    &WindowPane::insert_mode,
                    &WindowPane::visual_remove,
                    &WindowPane::no_op
                }
            },
            {
                'v',
                {
                    &WindowPane::no_op,
                    &WindowPane::visual_mode,
                    &WindowPane::visual_cancel,
                    &WindowPane::no_op
                }
            },
            {
                'y',
                {
                    &WindowPane::no_op,
                    &WindowPane::yank,
                    &WindowPane::visual_yank,
                    &WindowPane::no_op
                }
            },
            {
                'e',
                {
                    &WindowPane::insert,
                    &WindowPane::history_delete,
                    &WindowPane::no_op,
                    &WindowPane::replace,
                }
            },
            {
                ':',
                {
                    &WindowPane::no_op,
                    &WindowPane::command_mode,
                    &WindowPane::no_op,
                    &WindowPane::no_op
                }
            },
        };
};

class MatrixEditor
{
    public:
        explicit MatrixEditor(void);
        ~MatrixEditor(void);

        bool init(void);
        void fini(void);
        void loop(void);
        void error(string e);
        void set_mode(EditorMode mode);
        EditorMode mode(void);
        void command(int ch);

        void yanked(const string& s);
        const string& yanked(void);

        static void sig_winch(int sig);
        void resize_windows(void);

        static void sig_quit(int sig);
        void quit(void);

    protected:
        void process_command(int ch);
        void execute_command(const string& s);
        void prev_pane(const vector<string>& args);
        void next_pane(const vector<string>& args);
        void quit_all(const vector<string>& args);
        void quit(const vector<string>& args);
        void color(const vector<string>& args);
        void matrix_pane(const vector<string>& args);
        void eval_pane(const vector<string>& args);
        void new_pane(const vector<string>& args);
        void redraw(const vector<string>& args);

        bool init_signals(void);
        bool init_ncurses(void);
        bool window_too_small(void);
        int get_matrix_pane_height(void);

    private:
        MatrixEditor& operator=(const MatrixEditor& rhs);

        static MatrixEditor* sig_handler;
        static bool ncurses_initialized;

        bool _initialized;

        EditorMode _mode;

        size_t _current_matrix_pane;
        vector<MatrixPane> _matrix_panes;

        size_t _current_eval_pane;
        vector<EvalPane> _eval_panes;

        WindowPane* _current_editor_window;

        int _h, _w, _y, _x;
        WINDOW* _matrix_pane_header;
        WINDOW* _matrix_pane_window;
        WINDOW* _eval_pane_header;
        WINDOW* _eval_pane_window;
        WINDOW* _command_window;
        WINDOW* _error_window;

        ColorPair _too_small_color;
        ColorPair _black_bar_color;
        ColorPair _error_window_color;

        string _yanked;

        vector<string> _command_history;

        bool _exit_loop;

        const int mpwhh = 1;
        const int epwhh = 1;
        const int epwh = 6;
        const int ewh = 4;
        const int cwh = 1;
        const int mpw_minh = 20;
        const int mpw_minw = 80;

        typedef void (MatrixEditor::*CommandAction)(const vector<string>& args);
#define MEC_CALL(object, member_func)   ((object).*(member_func))
        map<string, CommandAction> _command_actions = {

            { "q", &MatrixEditor::quit },
            { "qa", &MatrixEditor::quit_all },
            { "bp", &MatrixEditor::prev_pane },
            { "bn", &MatrixEditor::next_pane },
            { "color", &MatrixEditor::color },
            { "m", &MatrixEditor::matrix_pane },
            { "e", &MatrixEditor::eval_pane },
            { "n", &MatrixEditor::new_pane },
            { "r", &MatrixEditor::redraw },
        };
};

#endif
