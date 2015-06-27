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

class EditorWindow
{
    public:
        EditorWindow(MatrixEditor* me, WINDOW* w);
        EditorWindow(const EditorWindow& rhs);
        EditorWindow& operator=(const EditorWindow& rhs);
        ~EditorWindow(void) {};

        virtual void insert_mode(int ch);
        virtual void edit_mode(int ch);
        virtual void visual_mode(int ch);
        virtual void replace_mode(int ch);
        virtual void command_mode(int ch);

        virtual void cancel(int ch);
        virtual void visual_cancel(int ch);
        virtual void replace_cancel(int ch);

        virtual void move(int ch);
        virtual void insert(int ch);
        virtual void remove(int ch);
        virtual void replace(int ch);
        virtual void clear(int ch);
        virtual void clear_to_eol(int ch);
        virtual void yank(int ch);
        virtual void paste(int ch);
        virtual void replace_move(int ch);
        virtual void enter(int ch);

        virtual void visual_move(int ch);
        virtual void visual_remove(int ch);
        virtual void visual_replace(int ch);
        virtual void visual_clear(int ch);
        virtual void visual_yank(int ch);
        virtual void visual_paste(int ch);
        virtual void visual_toggle_position(int ch);

        virtual void no_op(int ch) {}

        virtual void redraw(void);
        virtual void refresh(void);
        virtual bool is_print(int ch);
        virtual void we_addstr(string s);

        typedef void (EditorWindow::*KeyAction)(int ch);
#define MEW_CALL(object, member_func)   ((object).*(member_func))
        struct KeyModeAction
        {
            KeyAction insert_mode_action;
            KeyAction edit_mode_action;
            KeyAction visual_mode_action;
            KeyAction replace_mode_action;
        };

        virtual KeyAction key_action(int ch, EditorMode m);

        WINDOW* window(void);
        ColorPair& color(void);

    protected:
        MatrixEditor* _me;
        WINDOW* _win;
        ColorPair _color_pair;
        int _clear_toggle;
        int _yank_toggle;
        string _replaced;

    private:
        map<int, KeyModeAction> _key_mode_actions = {
            {
                K_ESCAPE,
                { 
                    &EditorWindow::edit_mode,
                    &EditorWindow::cancel,
                    &EditorWindow::visual_cancel,
                    &EditorWindow::replace_cancel
                }
            },
            {
                KEY_BACKSPACE,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                K_BACKSPACE1,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                K_BACKSPACE2,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                KEY_DC,
                {
                    &EditorWindow::remove,
                    &EditorWindow::remove,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'x',
                {
                    &EditorWindow::insert,
                    &EditorWindow::remove,
                    &EditorWindow::visual_remove,
                    &EditorWindow::replace
                }
            },
            {
                'd',
                {
                    &EditorWindow::insert,
                    &EditorWindow::clear,
                    &EditorWindow::visual_remove,
                    &EditorWindow::replace
                }
            },
            {
                'D',
                {
                    &EditorWindow::insert,
                    &EditorWindow::clear_to_eol,
                    &EditorWindow::visual_clear,
                    &EditorWindow::replace
                }
            },
            {
                'p',
                {
                    &EditorWindow::insert,
                    &EditorWindow::paste,
                    &EditorWindow::visual_paste,
                    &EditorWindow::replace
                }
            },
            {
                'P',
                {
                    &EditorWindow::insert,
                    &EditorWindow::paste,
                    &EditorWindow::visual_paste,
                    &EditorWindow::replace
                }
            },
            {
                'r',
                {
                    &EditorWindow::insert,
                    &EditorWindow::replace_mode,
                    &EditorWindow::visual_replace,
                    &EditorWindow::replace
                }
            },
            {
                'R',
                {
                    &EditorWindow::insert,
                    &EditorWindow::replace_mode,
                    &EditorWindow::visual_replace,
                    &EditorWindow::replace
                }
            },
            {
                K_NEWLINE,
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                KEY_ENTER,
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::replace
                }
            },
            {
                KEY_HOME,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace,
                }
            },
            {
                '^',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                '_',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                '$',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                'g',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                'G',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                KEY_LEFT,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                'h',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                KEY_RIGHT,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                'l',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                KEY_UP,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                'k',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                '-',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                KEY_DOWN,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                'j',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                '+',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                K_TAB,
                {
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_BTAB,
                {
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                'i',
                {
                    &EditorWindow::insert,
                    &EditorWindow::insert_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::replace
                }
            },
            {
                'a',
                {
                    &EditorWindow::insert,
                    &EditorWindow::insert_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::replace
                }
            },
            {
                'o',
                {
                    &EditorWindow::insert,
                    &EditorWindow::no_op,
                    &EditorWindow::visual_toggle_position,
                    &EditorWindow::replace
                }
            },
            {
                'O',
                {
                    &EditorWindow::insert,
                    &EditorWindow::no_op,
                    &EditorWindow::visual_toggle_position,
                    &EditorWindow::replace
                }
            },
            {
                's',  // Deletes char at cursor, like KEY_DC then to insert mode
                {
                    &EditorWindow::insert,
                    &EditorWindow::insert_mode,
                    &EditorWindow::visual_remove,
                    &EditorWindow::replace
                }
            },
            {
                'v',
                {
                    &EditorWindow::insert,
                    &EditorWindow::visual_mode,
                    &EditorWindow::visual_cancel,
                    &EditorWindow::replace
                }
            },
            {
                'y',
                {
                    &EditorWindow::insert,
                    &EditorWindow::yank,
                    &EditorWindow::visual_yank,
                    &EditorWindow::replace
                }
            },
            {
                ':',
                {
                    &EditorWindow::insert,
                    &EditorWindow::command_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::replace
                }
            },
        };
};

class MatrixPane : public EditorWindow
{
    public:
        MatrixPane(size_t n, MatrixEditor* me, WINDOW* w);
        MatrixPane(const MatrixPane& rhs);
        MatrixPane& operator=(const MatrixPane& rhs);
        ~MatrixPane(void);

        virtual void insert_mode(int ch);
        virtual void edit_mode(int ch);
        virtual void visual_mode(int ch);
        virtual void replace_mode(int ch);
        //virtual void command_mode(int ch);

        //virtual void cancel(int ch);
        virtual void visual_cancel(int ch);
        //virtual void replace_cancel(int ch);

        virtual void move(int ch);
        virtual void insert(int ch);
        virtual void remove(int ch);
        virtual void replace(int ch);
        virtual void clear(int ch);
        virtual void clear_to_eol(int ch);
        virtual void yank(int ch);
        virtual void paste(int ch);
        virtual void replace_move(int ch);
        virtual void enter(int ch);

        virtual void visual_move(int ch);
        virtual void visual_remove(int ch);
        virtual void visual_replace(int ch);
        virtual void visual_clear(int ch);
        virtual void visual_yank(int ch);
        virtual void visual_paste(int ch);
        virtual void visual_toggle_position(int ch);

        //virtual void no_op(int ch) {}

        virtual void redraw(void);
        virtual void refresh(void);
        virtual bool is_print(int ch);
        virtual KeyAction key_action(int ch, EditorMode m);
        virtual void we_addstr(string s);

        void draw(void);
        void set(void);

        WindowEntry& entry(size_t row, size_t col);
        WindowEntry& entry(void);
        bool adjust(size_t width);

        void next(void);
        void prev(void);
        void up(void);
        void down(void);
        void right(void);
        void left(void);
        void begin(void);
        void end(void);
        void first(void);
        void last(void);

        void vright(void);
        void vleft(void);
        void vbegin(void);
        void vend(void);
        void visual_done(void);

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
        vector<string> _history;
        size_t _cur_hist_entry;
        Matrix<Scientific> _last_A, _last_s, _last_v;

        vector<int> _valid_screen_chars = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.',
            '(', ')', ' ',
            '-', '+', '*', '/',
            'e', 'E', '^'
        };

        map<int, EditorWindow::KeyModeAction> _key_mode_actions = {
            {
                K_ESCAPE,
                { 
                    &EditorWindow::edit_mode,
                    &EditorWindow::cancel,
                    &EditorWindow::visual_cancel,
                    &EditorWindow::replace_cancel
                }
            },
            {
                KEY_BACKSPACE,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                K_BACKSPACE1,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                K_BACKSPACE2,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                KEY_DC,
                {
                    &EditorWindow::remove,
                    &EditorWindow::remove,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'x',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::remove,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'd',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::clear,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'D',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::clear_to_eol,
                    &EditorWindow::visual_clear,
                    &EditorWindow::no_op
                }
            },
            {
                'p',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::paste,
                    &EditorWindow::visual_paste,
                    &EditorWindow::no_op
                }
            },
            {
                'P',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::paste,
                    &EditorWindow::visual_paste,
                    &EditorWindow::no_op
                }
            },
            {
                'r',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::replace_mode,
                    &EditorWindow::visual_replace,
                    &EditorWindow::no_op
                }
            },
            {
                'R',
                {
                    &EditorWindow::insert,
                    &EditorWindow::replace_mode,
                    &EditorWindow::visual_replace,
                    &EditorWindow::no_op
                }
            },
            {
                K_NEWLINE,
                {
                    &EditorWindow::enter,
                    &EditorWindow::enter,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_ENTER,
                {
                    &EditorWindow::enter,
                    &EditorWindow::enter,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_HOME,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                '^',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                '_',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                '$',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                'g',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                'G',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_LEFT,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                'h',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_RIGHT,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                'l',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_UP,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                'k',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                '-',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                KEY_DOWN,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::move
                }
            },
            {
                'j',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                '+',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                K_TAB,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::move
                }
            },
            {
                'n',
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::move
                }
            },
            {
                KEY_BTAB,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::move
                }
            },
            {
                'b',
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::move
                }
            },
            {
                'i',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::insert_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                'a',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::insert_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                'o',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::visual_toggle_position,
                    &EditorWindow::no_op
                }
            },
            {
                'O',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::visual_toggle_position,
                    &EditorWindow::no_op
                }
            },
            {
                's',  // Deletes char at cursor, like KEY_DC then to insert mode
                {
                    &EditorWindow::no_op,
                    &EditorWindow::insert_mode,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'v',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::visual_mode,
                    &EditorWindow::visual_cancel,
                    &EditorWindow::no_op
                }
            },
            {
                'y',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::yank,
                    &EditorWindow::visual_yank,
                    &EditorWindow::no_op
                }
            },
            {
                'e',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::replace,
                }
            },
            {
                ':',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::command_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
        };
};


class EvalPane : public EditorWindow
{
    public:
        EvalPane(MatrixEditor* me, WINDOW* w);
        EvalPane(const EvalPane& rhs);
        EvalPane& operator=(const EvalPane& rhs);
        ~EvalPane(void) {}

        virtual void insert_mode(int ch);
        virtual void edit_mode(int ch);
        virtual void visual_mode(int ch);
        virtual void replace_mode(int ch);
        //virtual void command_mode(int ch);

        //virtual void cancel(int ch);
        virtual void visual_cancel(int ch);
        //virtual void replace_cancel(int ch);

        virtual void move(int ch);
        virtual void insert(int ch);
        virtual void remove(int ch);
        virtual void replace(int ch);
        virtual void clear(int ch);
        virtual void clear_to_eol(int ch);
        virtual void yank(int ch);
        virtual void paste(int ch);
        virtual void replace_move(int ch);
        virtual void enter(int ch);

        virtual void visual_move(int ch);
        virtual void visual_remove(int ch);
        virtual void visual_replace(int ch);
        virtual void visual_clear(int ch);
        virtual void visual_yank(int ch);
        virtual void visual_paste(int ch);
        virtual void visual_toggle_position(int ch);

        //virtual void no_op(int ch) {}

        virtual void redraw(void);
        virtual void refresh(void);
        virtual bool is_print(int ch);
        virtual KeyAction key_action(int ch, EditorMode m);
        virtual void we_addstr(string s);

        void draw(void);
        void set(void);
        void right(void);
        void left(void);
        void begin(void);
        void end(void);

        void vright(void);
        void vleft(void);
        void vbegin(void);
        void vend(void);
        void visual_done(void);

        void solve(void);

    private:
        int _win_offset;
        EvalEntry _e_entry;
        vector<string> _history;
        size_t _cur_hist_entry;

        vector<int> _valid_screen_chars = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.',
            '(', ')', ' ',
            '-', '+', '*', '/',
            'e', 'E', '^'
        };

        map<int, EditorWindow::KeyModeAction> _key_mode_actions = {
            {
                K_ESCAPE,
                { 
                    &EditorWindow::edit_mode,
                    &EditorWindow::cancel,
                    &EditorWindow::visual_cancel,
                    &EditorWindow::replace_cancel
                }
            },
            {
                KEY_BACKSPACE,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                K_BACKSPACE1,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                K_BACKSPACE2,
                {
                    &EditorWindow::remove,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                KEY_DC,
                {
                    &EditorWindow::remove,
                    &EditorWindow::remove,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'x',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::remove,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'd',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::clear,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'D',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::clear_to_eol,
                    &EditorWindow::visual_clear,
                    &EditorWindow::no_op
                }
            },
            {
                'p',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::paste,
                    &EditorWindow::visual_paste,
                    &EditorWindow::no_op
                }
            },
            {
                'P',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::paste,
                    &EditorWindow::visual_paste,
                    &EditorWindow::no_op
                }
            },
            {
                'r',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::replace_mode,
                    &EditorWindow::visual_replace,
                    &EditorWindow::no_op
                }
            },
            {
                'R',
                {
                    &EditorWindow::insert,
                    &EditorWindow::replace_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                K_NEWLINE,
                {
                    &EditorWindow::enter,
                    &EditorWindow::enter,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_ENTER,
                {
                    &EditorWindow::enter,
                    &EditorWindow::enter,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_HOME,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                '^',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace
                }
            },
            {
                '_',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                '$',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                'g',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                'G',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_LEFT,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                'h',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_RIGHT,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::replace_move
                }
            },
            {
                'l',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::move,
                    &EditorWindow::visual_move,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_UP,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                'k',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                '-',
                {
                    &EditorWindow::insert,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::replace,
                }
            },
            {
                KEY_DOWN,
                {
                    &EditorWindow::move,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                'j',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                '+',
                {
                    &EditorWindow::insert,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::replace
                }
            },
            {
                K_TAB,
                {
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                KEY_BTAB,
                {
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                'i',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::insert_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                'a',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::insert_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
                }
            },
            {
                'o',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::visual_toggle_position,
                    &EditorWindow::no_op
                }
            },
            {
                'O',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::no_op,
                    &EditorWindow::visual_toggle_position,
                    &EditorWindow::no_op
                }
            },
            {
                's',  // Deletes char at cursor, like KEY_DC then to insert mode
                {
                    &EditorWindow::no_op,
                    &EditorWindow::insert_mode,
                    &EditorWindow::visual_remove,
                    &EditorWindow::no_op
                }
            },
            {
                'v',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::visual_mode,
                    &EditorWindow::visual_cancel,
                    &EditorWindow::no_op
                }
            },
            {
                'y',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::yank,
                    &EditorWindow::visual_yank,
                    &EditorWindow::no_op
                }
            },
            {
                'e',
                {
                    &EditorWindow::insert,
                    &EditorWindow::move,
                    &EditorWindow::no_op,
                    &EditorWindow::replace,
                }
            },
            {
                ':',
                {
                    &EditorWindow::no_op,
                    &EditorWindow::command_mode,
                    &EditorWindow::no_op,
                    &EditorWindow::no_op
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

        EditorWindow* _current_editor_window;

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
