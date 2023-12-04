#include <bits/stdc++.h>
#include <windows.h>
#include <conio.h>
#include <random>
#include <ctime>
#include <chrono>

#define INITIAL_FEVER 20000
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
            consoleInfo.dwSize.Y = 40;

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

long long gettime() {
    const auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return millis.count();
}

long long timer = gettime();
long long softtimer = gettime();
long long fevertimer = gettime();

mt19937 engine((unsigned int)gettime());
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
int boardrotation = -1;
wchar_t box = L'\u25A0', gbox = L'\u25A1';
int fevertypenow = 0;
int FPS = 30;
int score = 0;
int fevergauge = INITIAL_FEVER;
int totalfevercount = 0;
long long feverend = gettime() - 999;
int feverstate = 0;
int repeat = 1;
int i;
long long inittime = gettime();

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
    KEY_X = 120,    // X
    KEY_DOWN = 80,  // down
    KEY_LEFT = 75,  // left
    KEY_RIGHT = 77, // right
    KEY_SPACE = 32, // space
    KEY_Z = 122,    // Z
    KEY_C = 99,     // C
};
const int keylist[8] = {
    -1,
    eKeyCode::KEY_X,
    eKeyCode::KEY_DOWN,
    eKeyCode::KEY_LEFT,
    eKeyCode::KEY_RIGHT,
    eKeyCode::KEY_SPACE,
    eKeyCode::KEY_Z,
    eKeyCode::KEY_C,
};
const string keyname[7] = {
    "  X  ", "down ", "left ", "right", "space", "  Z  ", "  C  ",
};
int key[7] = { 1, 2, 3, 4, 5, 6, 7 }; // index of keylist

void boardclear() { // clear board
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 10; j++) board[i][j] = 0;
    }
}

int cof[4][2][3] = {
    { {1, 0, 0}, {0, 1, 0} },
    { {0, 1, -2}, {-1, 0, 29} },
    { {-1, 0, 27}, {0, -1, 31} },
    { {0, -1, 29}, {1, 0, 2} },
};

void _cout(int x, int y, char c, bool rotated = false) { // ASCII console output
    int r = rotated ? max(0, boardrotation) : 0; // rotate false or boardrotation is -1 => r = 0
    COORD coord{ 0, };
    DWORD dw = 0;
    coord.X = cof[r][0][0] * x + cof[r][0][1] * y + cof[r][0][2];
    coord.Y = cof[r][1][0] * x + cof[r][1][1] * y + cof[r][1][2]; // rotation matrix
    SetConsoleCursorPosition(doubleConsole::console.hBuffer[doubleConsole::console.nCurBuffer], coord);
    WriteFile(doubleConsole::console.hBuffer[doubleConsole::console.nCurBuffer],
        &c, sizeof(c), &dw, NULL);
}

void _wcout(int x, int y, wchar_t c, bool rotated = false) { // unicode console output
    int r = rotated ? max(0, boardrotation) : 0; // rotate false or boardrotation is -1 => r = 0
    COORD coord{ 0, };
    DWORD dw = 0;
    coord.X = cof[r][0][0] * x + cof[r][0][1] * y + cof[r][0][2];
    coord.Y = cof[r][1][0] * x + cof[r][1][1] * y + cof[r][1][2]; // rotation matrix
    SetConsoleCursorPosition(doubleConsole::console.hBuffer[doubleConsole::console.nCurBuffer], coord);
    WriteConsoleOutputCharacterW(doubleConsole::console.hBuffer[doubleConsole::console.nCurBuffer],
        &c, 1, coord, &dw);
}

void printstr(int h, int w, char* str) {
    int lengthofstr = strlen(str);
    int i = w;
    for (i = w; i < w + lengthofstr; i++) {
        _wcout(i, h, str[i - w]);
    }
}

void printint(int h, int w, long long x, int k) {
    int length = 0;
    int i, j;
    long long xcpy = x;
    int p = 1;
    while (k > 0) {
        p *= 10;
        k--;
    }
    if (x > p) {
        if (!x) _wcout(w, h, '0');
        while (xcpy > 0) {
            length += 1;
            xcpy /= 10;
        }
        int i = 0;
        int remainder = 0;
        for (i = 0; i < length + 3; i++) {
            _wcout(w + i, h, ' ');
        }
        for (i = length - 1; i >= 0; i--) {
            remainder = x % 10;
            x /= 10;
            _wcout(w + i, h, '0' + remainder);
        }
    }
    else if (x != 0) {
        while (xcpy > 0) {
            length += 1;
            xcpy /= 10;
        }
        for (i = p / 10, j = 0; i > 0; i /= 10, j++) {
            _wcout(w + j, h, '0' + (x / i));
            x %= i;
        }
    }
    else {
        for (i = p / 10, j = 0; i > 0; i /= 10, j++) {
            _wcout(w + j, h, '0');
        }
    }

}

void printline(int x1, int y1, int x2, int y2, char c, bool rotated = false) { // print one line
    if (x1 == x2) { // vertical line
        for (int j = min(y1, y2); j <= max(y1, y2); j++) {
            _cout(x1, j, c, rotated);
        }
    }
    else if (y1 == y2) { // horizontal line
        for (int i = min(x1, x2); i <= max(x1, x2); i++) {
            _cout(i, y1, c, rotated);
        }
    }
}

void printrect(int x1, int y1, int x2, int y2, bool rotated = false) {
    int r = rotated ? max(0, boardrotation) : 0; // rotate false or boardrotation is -1 => r = 0
    if (x1 > x2) swap(x1, x2);
    if (y1 > y2) swap(y1, y2);
    printline(x1 + 1, y1, x2 - 1, y1, (r % 2 ? '|' : '-'), rotated);
    printline(x1 + 1, y2, x2 - 1, y2, (r % 2 ? '|' : '-'), rotated);
    printline(x1, y1 + 1, x1, y2 - 1, (r % 2 ? '-' : '|'), rotated);
    printline(x2, y1 + 1, x2, y2 - 1, (r % 2 ? '-' : '|'), rotated); // change char; vertical or horizontal
    _wcout(x1, y1, L'\u25A0', rotated);
    _wcout(x1, y2, L'\u25A0', rotated);
    _wcout(x2, y1, L'\u25A0', rotated);
    _wcout(x2, y2, L'\u25A0', rotated);
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

void clearwindow() {
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 32; j++) {
            _cout(i, j, ' '); // make empty
        }
    }
}

void printboard() { // print board (without color)

    if (boardrotation >= 0) clearwindow(); // if board rotation is activated
    
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
    printrect(1, 8, 6, 11, true);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (ch[holdblock][i][j] == 1) {
                //_cout(j + 2, i + 9, print[holdblock]);
                _wcout(j + 2, i + 9, L'\u25A0', true);
            }
            else {
                _cout(j + 2, i + 9, ' ', true);
            }
        }
    }

    // board randering
    const int off1 = 7;
    printrect(off1 + 1, 8, off1 + 12, 28, true);
    for (int i = 25; i >= 0; i--) {
        for (int j = 0; j < 10; j++) {
            //_cout(j + 1 + off1, 28 - i, print[board[i][j]]);
            if (board[i][j] > 0) _wcout(j + 2 + off1, 27 - i, box, true);
            else if (ghost[i][j] == 1) _wcout(j + 2 + off1, 27 - i, gbox, true);
            else _cout(j + 2 + off1, 27 - i, ' ', true);
        }
    }

    // next randering
    const int off2 = 20;
    printrect(off2 + 1, 8, off2 + 6, 23, true);
    // get a next nextcount=5 block using deque
    for (int t = 0; t < nextcount; t++) {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 4; j++) {
                if (ch[nextlist.front()][i][j] == 1) {
                    _wcout(j + off2 + 2, i + 3 * t + 9, L'\u25A0', true);
                }
                else {
                    _cout(j + off2 + 2, i + 3 * t + 9, ' ', true);
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
    for (int i = off3 + 2; i <= off3 + 18; i++) _cout(i, 10, ' ');

    int tmp = 0; // check type of key shuffle
    for (int i = 0; i < 7; i++) {
        if (key[i] == 0) { // if key prevented
            for (int j = 0; j < 5; j++) _cout(off3 + 7 + j, 10, keyname[i][j]);
            _cout(off3 + 13, 10, 'X');
            break;
        }
        else if (key[i] != i + 1) {
            for (int j = 0; j < 5; j++) _cout(off3 + 3 + j + tmp, 10, keyname[i][j]);
            tmp = 10; // make offset of second
        }
    }
    if (tmp == 10) { // key swap
        _cout(off3 + 9, 10, '<');
        _cout(off3 + 10, 10, '-');
        _cout(off3 + 11, 10, '>');
    }
    if (boardrotation >= 0) { // rander string of board rotation event
        printstr(10, 33, (char*)"board rotation!");
    }
    if (box == L'\u00B7') { // rander string of blind event
        printstr(10, 35, (char*)"blind event!");
    }

    printstr(12, 32, (char*)"-----------------");
    printstr(14, 33, (char*)"Score | ");
    printint(14, 41, score, 7);
    printstr(18, 33, (char*)"Time  | ");
    printint(18, 41, ((gettime() - inittime) / 1000) / 60, 2);
    printstr(18, 43, (char*)" : ");
    printint(18, 46, ((gettime() - inittime) / 1000) % 60, 2);
    printstr(22, 33, (char*)"Fever | ");
    for (i = 0; i < 7; i++) {
        _wcout(41 + i, 22, L'\u25A1');
    }
    for (i = 0; ((i / 7.0) * INITIAL_FEVER < (INITIAL_FEVER - fevergauge)) && (i < 7); i++) {
        _wcout(41 + i, 22, L'\u25A0');
    }
    printstr(26, 33, (char*)"Level | ");
    printint(26, 41, 1 + (totalfevercount / 5), 2);

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

        if (nKey == keylist[key[0]]) { // press X
            ret = clockwise(h, w, rotation, type);
            if (boardrotation >= 0) {
                boardrotation = (boardrotation + 1) % 4;
            }
            timer = gettime();
        }
        else if (nKey == keylist[key[1]]) { // press down
            ret = softdrop(h, w, rotation, type);
            timer = gettime();
        }
        else if (nKey == keylist[key[2]]) { // press left
            ret = moveleft(h, w, rotation, type);
            timer = gettime();
        }
        else if (nKey == keylist[key[3]]) { // press right
            ret = moveright(h, w, rotation, type);
            timer = gettime();
        }
        else if (nKey == keylist[key[4]]) { // press space
            ret = harddrop(h, w, rotation, type);
            timer = gettime();
            isharddrop = true;
        }
        else if (nKey == keylist[key[5]]) { // press Z
            ret = counterclockwise(h, w, rotation, type);
            if (boardrotation >= 0) boardrotation = (boardrotation + 3) % 4;
            timer = gettime();
        }
        else if (nKey == keylist[key[6]]) { // press C
            if (!ishold) {
                ret = dohold(type);
                timer = gettime();
            }
            ishold = true;
        }

    }
    return ret;
}

void setrotate() {
    boardrotation = 0; // initial diraction
}

void resetrotate() {
    clearwindow();
    doubleConsole::BufferFlip();
    clearwindow();
    doubleConsole::BufferFlip();
    boardrotation = -1; // nagative value means rotate is not activated
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

void makelarge() {
    box = L'\u25A0';
    gbox = L'\u25A1';
}

void makesmall() {
    box = L'\u00B7';
    gbox = L'\u0020';
}

int fevercalculate(int fevergauge) {
    int i, j, ccnntt;
    int decreasecount = 5;
    ccnntt = 0;
    for (i = 0; i < 20; i++) {
        ccnntt = 0;
        for (j = 0; j < 10; j++) {
            ccnntt += !(!board[i][j]);
        }
        if (!ccnntt) {
            decreasecount++;
        }
    }
    return fevergauge - decreasecount;
}

long long feveractivate(int fevercount) {
    int feverlength = 5000 * (fevercount % 3) + 2500 * (fevercount / 3) + 10000;
    int fevertype = generator() % 3;
    switch (fevertype) {
    case 0:
        setrotate();
        break;
    case 1:
        shufflekey();
        break;
    case 2:
        makesmall();
        break;
    }
    fevertypenow = fevertype;
    return gettime() + feverlength;
}

/*
         <key binding>
    왼쪽 화살표 : 왼쪽 이동
    오른쪽 화살표 : 오른쪽 이동
    X 키 : 시계 방향 회전
    아래쪽 화살표 : softdrop
    스페이스 바 : harddrop
    Z 키 : 반시계 방향 회전
    C 키 : hold
*/

int main() {

    doubleConsole::InitGame(); // make double-buffer console

    boardclear(); // board initalization
    getnextblock(); // make list of block
    resetkey(); // reset keybind
    resetrotate(); // reset boardrotation

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

            if (gettime() - softtimer > max(0, 500 - 10 * totalfevercount)) { // gravity
                info = softdrop(info.h, info.w, info.rotation, info.type);
                softtimer = gettime();
            }

            if (((gettime() - fevertimer) > 1000 / FPS) && (feverend < gettime()) && !feverstate) {
                for (i = 0; i < repeat; i++) {
                    fevergauge = fevercalculate(fevergauge);
                }
                if (fevergauge <= 0) {
                    feverend = feveractivate(totalfevercount++);
                    feverstate = 1;
                }
                fevertimer = gettime();
            }
            score += 1 + feverstate + (totalfevercount / 5);
            if (feverstate && (feverend < gettime())) {
                switch (fevertypenow) {
                case 0:
                    resetrotate();
                    break;
                case 1:
                    resetkey();
                    break;
                case 2:
                    makelarge();
                    break;
                }
                fevergauge = INITIAL_FEVER;
                feverstate = 0;
                if (totalfevercount % 5 == 0) repeat += 1;

            }

            if (hit(info.h - 1, info.w, info.rotation, info.type)) { // if hit a floor
                if (gettime() - timer > max(0, 500 - 10 * totalfevercount)) break;
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
