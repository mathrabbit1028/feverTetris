#include <bits/stdc++.h>
#include <windows.h>
#include <conio.h>
#include <random>
#include <ctime>
using namespace std;

// prevent flickering
namespace doubleConsole {

    struct stRect
    {
        int nWidth;
        int nHeight;
    };

    struct stConsole
    {
        // Console Handler
        HANDLE hConsole;
        // Console Rect Data
        stRect rtConsole;
        // Console Buffer Handler
        HANDLE hBuffer[2];
        // Current Console Buffer Index
        int nCurBuffer;

        stConsole()
            : hConsole(nullptr), hBuffer{ nullptr, }, nCurBuffer(0)
        {}
    };

    stConsole console;

    void InitGame(bool bInitConsole = true)
    {
        // Initialize Console Data
        if (bInitConsole) {
            console.hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            console.nCurBuffer = 0;

            CONSOLE_CURSOR_INFO consoleCursor{ 1, FALSE };
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo{ 0, };
            GetConsoleScreenBufferInfo(console.hConsole, &consoleInfo);
            consoleInfo.dwSize.X = 40;
            consoleInfo.dwSize.Y = 30;

            console.rtConsole.nWidth = consoleInfo.srWindow.Right - consoleInfo.srWindow.Left;
            console.rtConsole.nHeight = consoleInfo.srWindow.Bottom - consoleInfo.srWindow.Top;

            console.hBuffer[0] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
            SetConsoleScreenBufferSize(console.hBuffer[0], consoleInfo.dwSize);
            SetConsoleWindowInfo(console.hBuffer[0], TRUE, &consoleInfo.srWindow);
            SetConsoleCursorInfo(console.hBuffer[0], &consoleCursor);

            console.hBuffer[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
            SetConsoleScreenBufferSize(console.hBuffer[1], consoleInfo.dwSize);
            SetConsoleWindowInfo(console.hBuffer[1], TRUE, &consoleInfo.srWindow);
            SetConsoleCursorInfo(console.hBuffer[1], &consoleCursor);
        }
    }

    void DestroyGame() {
        if (console.hBuffer[0] != nullptr) {
            CloseHandle(console.hBuffer[0]);
        }
        if (console.hBuffer[1] != nullptr) {
            CloseHandle(console.hBuffer[1]);
        }
    }

    void ScreenClear() {
        COORD pos{ 0, };
        DWORD dwWritten = 0;
        unsigned size = console.rtConsole.nWidth * console.rtConsole.nHeight;
        FillConsoleOutputCharacter(console.hConsole, ' ', size, pos, &dwWritten);
        SetConsoleCursorPosition(console.hConsole, pos);
    }

    void BufferFlip() {
        SetConsoleActiveScreenBuffer(console.hBuffer[console.nCurBuffer]);
        console.nCurBuffer = console.nCurBuffer ? 0 : 1;
    }
}

// garbage 8, empty 15
// I, O, Z, S, J, L, T
// 9, 6, 4, 2, 1, 13, 5

time_t timer = time(NULL);
time_t softtimer = time(NULL);

mt19937 engine((unsigned int)time(NULL));
uniform_int_distribution<int> distribution(0, 5039); // 7-bag generator
uniform_int_distribution<int> distribution2(0, 9); // garbage generator
uniform_int_distribution<int> distribution3(0, 49); // key shuffle generator
auto generator = bind(distribution, engine);
auto generator2 = bind(distribution2, engine);
auto generator3 = bind(distribution3, engine);

int board[30][10], ghost[30][10], ans = 0, cnt = 0, att = 0;
deque<int> nextlist, temp;
int holdblock = 0; // type of holdblock
bool ishold, isharddrop;
const int nextcount = 5; // counts of next block

//const int color[9] = { 15, 9, 6, 4, 2, 1, 6, 13, 8 }; // color of each mino
//const char print[9] = { ' ', 'I', 'O', 'Z', 'S', 'J', 'L', 'T', 'X' }; // print ASCII char
const int first[3] = { 21, 4, 0 }; // initial summon position

// BLOCK table : block[type][rotation], store relative position of four block of one mino
const int block[8][4][4][2] = {
    {
        {
            {0, 0}, {0, 0}, {0, 0}, {0, 0}
        },
        {
            {0, 0}, {0, 0}, {0, 0}, {0, 0}
        },
        {
            {0, 0}, {0, 0}, {0, 0}, {0, 0}
        },
        {
            {0, 0}, {0, 0}, {0, 0}, {0, 0}
        }
    },

    // I
    {
        {
            {0, -1}, {0, 0}, {0, 1}, {0, 2}
        },
        {
            {1, 1}, {0, 1}, {-1, 1}, {-2, 1}
        },
        {
            {-1, -1}, {-1, 0}, {-1, 1}, {-1, 2}
        },
        {
            {1, 0}, {0, 0}, {-1, 0}, {-2, 0}
        }
    },

    // O
    {
        {
            {0, 0}, {0, 1}, {1, 0}, {1, 1}
        },
        {
            {0, 0}, {0, 1}, {1, 0}, {1, 1}
        },
        {
            {0, 0}, {0, 1}, {1, 0}, {1, 1}
        },
        {
            {0, 0}, {0, 1}, {1, 0}, {1, 1}
        }
    },

    // Z
    {
        {
            {1, -1}, {1, 0}, {0, 0}, {0, 1}
        },
        {
            {1, 1}, {0, 1}, {0, 0}, {-1, 0}
        },
        {
            {-1, 1}, {-1, 0}, {0, 0}, {0, -1}
        },
        {
            {-1, -1}, {0, -1}, {0, 0}, {1, 0}
        }
    },

    // S
    {
        {
            {0, -1}, {1, 0}, {0, 0}, {1, 1}
        },
        {
            {1, 0}, {0, 1}, {0, 0}, {-1, 1}
        },
        {
            {0, 1}, {-1, 0}, {0, 0}, {-1, -1}
        },
        {
            {-1, 0}, {0, -1}, {0, 0}, {1, -1}
        }
    },

    // J
    {
        {
            {1, 1}, {0, 1}, {0, 0}, {0, -1}
        },
        {
            {-1, 1}, {-1, 0}, {0, 0}, {1, 0}
        },
        {
            {-1, -1}, {0, -1}, {0, 0}, {0, 1}
        },
        {
            {1, -1}, {1, 0}, {0, 0}, {-1, 0}
        }
    },

    // L
    {
        {
            {1, -1}, {0, -1}, {0, 0}, {0, 1}
        },
        {
            {1, 1}, {1, 0}, {0, 0}, {-1, 0}
        },
        {
            {-1, 1}, {0, 1}, {0, 0}, {0, -1}
        },
        {
            {-1, -1}, {-1, 0}, {0, 0}, {1, 0}
        }
    },

    // T
    {
        {
            {0, 0}, {0, -1}, {1, 0}, {0, 1}
        },
        {
            {0, 0}, {1, 0}, {0, 1}, {-1, 0}
        },
        {
            {0, 0}, {0, 1}, {-1, 0}, {0, -1}
        },
        {
            {0, 0}, {-1, 0}, {0, -1}, {1, 0}
        }
    }
};

// SRS table: srs[rotation][diraction][order]
// diration 0 : clockwise / diration 1 : counter clockwise
const pair<int, int> srsI[4][2][5] = {

    {
        { {0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, 2} },
        { {0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1} },
    },

    {
        { {0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1} },
        { {0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2} },
    },

    {
        { {0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2} },
        { {0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1} },
    },

    {
        { {0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1} },
        { {0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2} },
    },

};
const pair<int, int> srs[4][2][5] = {

    {
        { {0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2} },
        { {0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2} },
    },

    {
        { {0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2} },
        { {0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2} },
    },

    {
        { {0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2} },
        { {0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2} },
    },

    {
        { {0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2} },
        { {0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2} },
    },

};

struct blockinfo { // struct of block
    int h, w, rotation, type;
};

enum eKeyCode {
    KEY_UP = 72,    // up
    KEY_DOWN = 80,  // down
    KEY_LEFT = 75,  // left
    KEY_RIGHT = 77, // right
    KEY_SPACE = 32, // space
    KEY_X = 120,    // X
    KEY_C = 99,     // C
};
const int keylist[8] = {
    -1,
    eKeyCode::KEY_UP,
    eKeyCode::KEY_DOWN,
    eKeyCode::KEY_LEFT,
    eKeyCode::KEY_RIGHT,
    eKeyCode::KEY_SPACE,
    eKeyCode::KEY_X,
    eKeyCode::KEY_C,
};
const string keyname[7] = {
    " up  ", "down ", "left ", "right", "space", "  X  ", "  C  ",
};
int key[7] = {1, 2, 3, 4, 5, 6, 7}; // index of keylist

void boardclear() { // clear board
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 10; j++) board[i][j] = 0;
    }
}

void _cout(int x, int y, char c) { // ASCII console output
    COORD coord{ 0, };
    DWORD dw = 0;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(doubleConsole::console.hBuffer[doubleConsole::console.nCurBuffer], coord);
    WriteFile(doubleConsole::console.hBuffer[doubleConsole::console.nCurBuffer],
        &c, sizeof(c), &dw, NULL);
}

void _wcout(int x, int y, wchar_t c) { // unicode console output
    COORD coord{ 0, };
    DWORD dw = 0;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(doubleConsole::console.hBuffer[doubleConsole::console.nCurBuffer], coord);
    WriteConsoleOutputCharacterW(doubleConsole::console.hBuffer[doubleConsole::console.nCurBuffer],
        &c, 1, coord, &dw);
}

void printline(int x1, int y1, int x2, int y2, char c) { // print one line
    if (x1 == x2) { // vertical line
        for (int j = min(y1, y2); j <= max(y1, y2); j++) {
            _cout(x1, j, c);
        }
    }
    else if (y1 == y2) { // horizontal line
        for (int i = min(x1, x2); i <= max(x1, x2); i++) {
            _cout(i, y1, c);
        }
    }
}

void printrect(int x1, int y1, int x2, int y2, bool cover=true) {
    if (x1 > x2) swap(x1, x2);
    if (y1 > y2) swap(y1, y2);
    if (cover) printline(x1 + 1, y1, x2 - 1, y1, '-');
    printline(x1 + 1, y2, x2 - 1, y2, '-');
    printline(x1, y1 + 1, x1, y2 - 1, '|');
    printline(x2, y1 + 1, x2, y2 - 1, '|');
    _wcout(x1, y1, L'\u25A0');
    _wcout(x1, y2, L'\u25A0');
    _wcout(x2, y1, L'\u25A0');
    _wcout(x2, y2, L'\u25A0');
}

void addghost(int h, int w, int rotation, int type) { // add ghost block
    for (int i = 0; i < 4; i++) {
        ghost[h + block[type][rotation][i][0]][w + block[type][rotation][i][1]] = 1;
    }
}

void delghost(int h, int w, int rotation, int type) { // delete ghost block
    for (int i = 0; i < 4; i++) {
        ghost[h + block[type][rotation][i][0]][w + block[type][rotation][i][1]] = 0;
    }
}

void printboard() { // print board (without color)

    // nextblock shape
    const int ch[8][2][4] = {
        { {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {0, 0, 0, 0}, {1, 1, 1, 1} },
        { {0, 1, 1, 0}, {0, 1, 1, 0} },
        { {1, 1, 0, 0}, {0, 1, 1, 0} },
        { {0, 1, 1, 0}, {1, 1, 0, 0} },
        { {0, 0, 1, 0}, {1, 1, 1, 0} },
        { {1, 0, 0, 0}, {1, 1, 1, 0} },
        { {0, 1, 0, 0}, {1, 1, 1, 0} },
    };

    // hold randering
    printrect(1, 8, 6, 11);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (ch[holdblock][i][j] == 1) {
                //_cout(j + 2, i + 9, print[holdblock]);
                _wcout(j + 2, i + 9, L'\u25A0');
            }
            else {
                _cout(j + 2, i + 9, ' ');
            }
        }
    }

    // board randering
    const int off1 = 7;
    printrect(off1 + 1, 8, off1 + 12, 28, false);
    for (int i = 27; i >= 0; i--) {
        for (int j = 0; j < 10; j++) {
            //_cout(j + 1 + off1, 28 - i, print[board[i][j]]);
            if (board[i][j] > 0) _wcout(j + 2 + off1, 27 - i, L'\u25A0');
            else if (ghost[i][j] == 1) _wcout(j + 2 + off1, 27 - i, L'\u25A1');
            else _cout(j + 2 + off1, 27 - i, ' ');
        }
    }

    // next randering
    const int off2 = 20;
    printrect(off2 + 1, 8, off2 + 6, 23);
    // get a next nextcount=5 block using deque
    for (int t = 0; t < nextcount; t++) {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 4; j++) {
                if (ch[nextlist.front()][i][j] == 1) {
                    _wcout(j + off2 + 2, i + 3*t + 9, L'\u25A0');
                }
                else {
                    _cout(j + off2 + 2, i + 3*t + 9, ' ');
                }
            }
        }
        temp.push_back(nextlist.front());
        nextlist.pop_front();
    }
    for (int t = 0; t < nextcount; t++) {
        nextlist.push_front(temp.back());
        temp.pop_back();
    }

    // right box
    const int off3 = 30;
    printrect(off3 + 1, 8, off3 + 19, 28);

    int tmp = 0; // check type of key shuffle
    for (int i = 0; i < 7; i++) {
        if (key[i] == 0) { // if key prevented
            for (int j = 0; j < 5; j++) _cout(off3 + 7 + j, 11, keyname[i][j]);
            _cout(off3 + 13, 11, 'X');
            break;
        }
        else if (key[i] != i + 1) {
            for (int j = 0; j < 5; j++) _cout(off3 + 3 + j + tmp, 11, keyname[i][j]);
            tmp = 10; // make offset of second
        }
    }
    if (tmp == 10) { // key swap
        _cout(off3 + 9, 11, '<');
        _cout(off3 + 10, 11, '-');
        _cout(off3 + 11, 11, '>');
    }

}

void summon(int h, int w, int rotation, int type) { // add block in board
    for (int i = 0; i < 4; i++) {
        board[h + block[type][rotation][i][0]][w + block[type][rotation][i][1]] = type;
    }
}

void dispawn(int h, int w, int rotation, int type) { // remove block in board
    for (int i = 0; i < 4; i++) {
        board[h + block[type][rotation][i][0]][w + block[type][rotation][i][1]] = 0;
    }
}

bool hit(int h, int w, int rotation, int type) { // hit detection
    bool ret = false;
    for (int i = 0; i < 4; i++) {
        // out of x bound
        if (w + block[type][rotation][i][1] < 0 || w + block[type][rotation][i][1] > 9) ret = true;
        // out of below bound
        else if (h + block[type][rotation][i][0] < 0) ret = true;
        // overlap of block
        else if (board[h + block[type][rotation][i][0]][w + block[type][rotation][i][1]] > 0) ret = true;
    }
    return ret;
}

void remove_line() { // remove line
    int rem = 0;
    for (int i = 0; i < 30; i++) {
        int f = 1;
        for (int j = 0; j < 10; j++) if (board[i][j] == 0) f = 0;
        if (f == 1) rem++;
        else {
            for (int j = 0; j < 10; j++) board[i - rem][j] = board[i][j];
        }
    }
    // simple attack score of TETRIO
    if (rem == 2) att += 1;
    if (rem == 3) att += 2;
    if (rem == 4) att += 4;
    ans += rem;
}

void make_garbage(int num) { // add garbage
    int pos = generator2(); // find x-coord of empty cell
    for (int i = 29 - num; i >= 0; i--) {
        for (int j = 0; j < 10; j++) board[i + num][j] = board[i][j];
    }
    for (int i = 0; i < num; i++) {
        for (int j = 0; j < 10; j++) {
            if (j != pos) board[i][j] = 8;
            else board[i][j] = 0;
        }
    }
}

void sevenbag() { // add 7-block to queue using 7-bag
    int per = generator(); // 0 ~ 5039 : matching each permutation
    int arr[7];
    int ch[7] = { 0, 0, 0, 0, 0, 0, 0 };
    for (int n = 7; n >= 1; n--) {
        int block = 0, idx = 0;
        while (1) {
            if (ch[block] == 0) {
                if (idx == per % n) break;
                idx++;
            }
            block++;
        }

        ch[block] = 1;
        block++;
        per = per / n;
        nextlist.push_back(block); // push 7-bag to queue
    }
}

int getnextblock() { // get next block, remove line, and game over detection
    if (nextlist.size() <= 7) sevenbag(); // nextlist not enough long
    int block = nextlist.front();
    nextlist.pop_front();
    //cout << block << "\n";

    if (hit(first[0], first[1], first[2], block)) { // if game end
        cout << "Game Over\n";
        cout << "pieces: " << cnt << "\n";
        cout << "break " << ans << " lines\n";
        cout << "attacks " << att << " lines\n";
        exit(0);
    }

    remove_line(); // delete line
    cnt++;

    return block;
}

blockinfo clockwise(int h, int w, int rotation, int type) { // clockwise rotation
    if (type == 1) {
        for (int i = 0; i < 5; i++) { // SRS of I mino
            int new_h = h + srsI[rotation][0][i].first, new_w = w + srsI[rotation][0][i].second;
            if (!hit(new_h, new_w, (rotation + 1) % 4, type)) return { new_h, new_w, (rotation + 1) % 4, type };
        }
    }
    if (type >= 3) {
        for (int i = 0; i < 5; i++) { // SRS of rest
            int new_h = h + srs[rotation][0][i].first, new_w = w + srs[rotation][0][i].second;
            if (!hit(new_h, new_w, (rotation + 1) % 4, type)) return { new_h, new_w, (rotation + 1) % 4, type };
        }
    }
    return { h, w, rotation, type };
}

blockinfo counterclockwise(int h, int w, int rotation, int type) { // counter clockwise rotation
    if (type == 1) { // SRS of I mino
        for (int i = 0; i < 5; i++) {
            int new_h = h + srsI[rotation][1][i].first, new_w = w + srsI[rotation][1][i].second;
            if (!hit(new_h, new_w, (rotation + 3) % 4, type)) return { new_h, new_w, (rotation + 3) % 4, type };
        }
    }
    if (type >= 3) { // SRS of rest
        for (int i = 0; i < 5; i++) {
            int new_h = h + srs[rotation][1][i].first, new_w = w + srs[rotation][1][i].second;
            if (!hit(new_h, new_w, (rotation + 3) % 4, type)) return { new_h, new_w, (rotation + 3) % 4, type };
        }
    }
    return { h, w, rotation, type };
}

blockinfo moveleft(int h, int w, int rotation, int type) { // move left
    if (hit(h, w - 1, rotation, type)) return { h, w, rotation, type };
    return { h, w - 1, rotation, type }; // move left one block
}

blockinfo moveright(int h, int w, int rotation, int type) {
    if (hit(h, w + 1, rotation, type)) return { h, w, rotation, type };
    return { h, w + 1, rotation, type }; // move right 1 block
}

blockinfo softdrop(int h, int w, int rotation, int type) {
    if (hit(h - 1, w, rotation, type)) return { h, w, rotation, type };
    return { h - 1, w, rotation, type }; // move down 1 block
}

blockinfo harddrop(int h, int w, int rotation, int type) {
    while (true) { // find most below position
        if (hit(h - 1, w, rotation, type)) break;
        h--;
    }
    return { h, w, rotation, type };
}

blockinfo dohold(int type) {
    if (holdblock == 0) { // if there isn't hold block
        holdblock = type;
        return { first[0], first[1], first[2], getnextblock() };
    }
    swap(holdblock, type); // if there hold block
    return { first[0], first[1], first[2], type };
}

blockinfo keybind(int h, int w, int rotation, int type) {
    int nKey = 0;
    blockinfo ret = { h, w, rotation, type };

    // keyinput detected
    if (_kbhit() > 0) {
        // get input key
        nKey = _getch();

        if (nKey == keylist[key[0]]) { // press up
            ret = clockwise(h, w, rotation, type);
            timer = time(NULL);
        }
        else if (nKey == keylist[key[1]]) { // press down
            ret = softdrop(h, w, rotation, type);
            timer = time(NULL);
        }
        else if (nKey == keylist[key[2]]) { // press left
            ret = moveleft(h, w, rotation, type);
            timer = time(NULL);
        }
        else if (nKey == keylist[key[3]]) { // press right
            ret = moveright(h, w, rotation, type);
            timer = time(NULL);
        }
        else if (nKey == keylist[key[4]]) { // press space
            ret = harddrop(h, w, rotation, type);
            timer = time(NULL);
            isharddrop = true;
        }
        else if (nKey == keylist[key[5]]) { // press X
            ret = counterclockwise(h, w, rotation, type);
            timer = time(NULL);
        }
        else if (nKey == keylist[key[6]]) { // press C
            if (!ishold) {
                ret = dohold(type);
                timer = time(NULL);
            }
            ishold = true;
        }

    }
    return ret;
}

void shufflekey() {
    int per = generator3(); // get number between 0 ~ 48
    int k1 = per % 7, k2 = per / 7; // target
    if (k1 == k2) { // if k1 == k2, delete k1
        key[k1] = 0;
    }
    else { // if k1 != k2, swap(k1, k2)
        swap(key[k1], key[k2]);
    }
}

void resetkey() {
    for (int i = 0; i < 7; i++) {
        key[i] = i + 1; // initial state
    }
}

/*
         <key binding>
    왼쪽 화살표 : 왼쪽 이동
    오른쪽 화살표 : 오른쪽 이동
    위쪽 화살표 : 시계 방향 회전
    아래쪽 화살표 : softdrop
    스페이스 바 : harddrop
    X 키 : 반시계 방향 회전
    C 키 : hold
*/

int main() {

    doubleConsole::InitGame(); // make double-buffer console

    boardclear(); // board initalization
    resetkey(); // reset keybind

    time_t softtimer = time(NULL); // timer for softdrop

    while (true) {

        blockinfo info;
        info.type = getnextblock();
        info.h = first[0], info.w = first[1], info.rotation = first[2]; // make info of next block

        ishold = false;
        isharddrop = false;

        while (true) {
            doubleConsole::ScreenClear();
            doubleConsole::BufferFlip(); // dobule-buffering

            dispawn(info.h, info.w, info.rotation, info.type); // hide before block

            if (time(NULL) - softtimer > 0.5) { // gravity
                info = softdrop(info.h, info.w, info.rotation, info.type);
                softtimer = time(NULL);
            }



            if (hit(info.h - 1, info.w, info.rotation, info.type)) { // if hit a floor
                if (time(NULL) - timer > 1.0) break;
                if (isharddrop) break;
            }
            info = keybind(info.h, info.w, info.rotation, info.type); // result pos by keyboard input

            blockinfo ghost = harddrop(info.h, info.w, info.rotation, info.type); // find ghost
            addghost(ghost.h, ghost.w, ghost.rotation, ghost.type); // add ghost
            summon(info.h, info.w, info.rotation, info.type); // show now block

            printboard(); // randering
            delghost(ghost.h, ghost.w, ghost.rotation, ghost.type); // delete ghost
        }

        summon(info.h, info.w, info.rotation, info.type); // update board
    }

    doubleConsole::DestroyGame(); // destory console

    return 0;
}
