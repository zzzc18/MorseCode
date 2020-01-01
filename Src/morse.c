#include "morse.h"
#include <string.h>
#include "feed_back.h"
#include "gpio.h"
#include "main.h"
#include "queue.h"
#include "tim.h"
#include "usart.h"

uint8_t map[6000];  // for every two bits 00:void 01:dot 10:line
uint16_t invmap[200];
uint8_t BUF[110];
uint8_t TBUF[1000];
uint8_t len;
uint8_t Input_Error_BUF[] = "Ooops invalid input morse code";
queue que;

uint8_t Caps_Trans(uint8_t ch) {  // a->A
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 'A';
    return ch;
}

void Morse_init() {
    map[6] = 'A', invmap['A'] = 6;          // A ._
    map[149] = 'B', invmap['B'] = 149;      // B  _...
    map[153] = 'C', invmap['C'] = 153;      // C _._.
    map[37] = 'D', invmap['D'] = 37;        // D _..
    map[1] = 'E', invmap['E'] = 1;          // E .
    map[89] = 'F', invmap['F'] = 89;        // F .._.
    map[41] = 'G', invmap['G'] = 41;        // G __.
    map[85] = 'H', invmap['H'] = 85;        // H ....
    map[5] = 'I', invmap['I'] = 5;          // I ..
    map[106] = 'J', invmap['J'] = 106;      // J .___
    map[38] = 'K', invmap['K'] = 38;        // K _._
    map[101] = 'L', invmap['L'] = 101;      // L ._..
    map[10] = 'M', invmap['M'] = 10;        // M __
    map[9] = 'N', invmap['N'] = 9;          // N _.
    map[42] = 'O', invmap['O'] = 42;        // O ___
    map[105] = 'P', invmap['P'] = 105;      // P .__.
    map[166] = 'Q', invmap['Q'] = 166;      // Q __._
    map[25] = 'R', invmap['R'] = 25;        // R ._.
    map[21] = 'S', invmap['S'] = 21;        // S ...
    map[2] = 'T', invmap['T'] = 2;          // T _
    map[22] = 'U', invmap['U'] = 22;        // U .._
    map[86] = 'V', invmap['V'] = 86;        // V ..._
    map[26] = 'W', invmap['W'] = 26;        // W .__
    map[150] = 'X', invmap['X'] = 150;      // X _.._
    map[154] = 'Y', invmap['Y'] = 154;      // Y _.__
    map[165] = 'Z', invmap['Z'] = 165;      // Z __..
    map[682] = '0', invmap['0'] = 682;      // 0 _____
    map[426] = '1', invmap['1'] = 426;      // 1 .____
    map[362] = '2', invmap['2'] = 362;      // 2 ..___
    map[346] = '3', invmap['3'] = 346;      // 3 ...__
    map[342] = '4', invmap['4'] = 342;      // 4 ...._
    map[341] = '5', invmap['5'] = 341;      // 5 .....
    map[597] = '6', invmap['6'] = 597;      // 6 _....
    map[661] = '7', invmap['7'] = 661;      // 7 __...
    map[677] = '8', invmap['8'] = 677;      // 8 ___..
    map[681] = '9', invmap['9'] = 681;      // 9 ____.
    map[1638] = '.', invmap['.'] = 1638;    //. ._._._
    map[2709] = ':', invmap[':'] = 2709;    //: ___...
    map[2650] = ',', invmap[','] = 2650;    //, __..__
    map[2457] = ';', invmap[';'] = 2457;    //; _._._.
    map[1445] = '?', invmap['?'] = 1445;    //? ..__..
    map[598] = '=', invmap['='] = 598;      //= _..._
    map[1705] = '\'', invmap['\''] = 1705;  //' .____.
    map[601] = '/', invmap['/'] = 601;      /// _.._.
    map[2458] = '!', invmap['!'] = 2458;    //! _._.__
    map[2390] = '-', invmap['-'] = 2390;    //- _...._
    map[1446] = '_', invmap['_'] = 1446;    //_ ..__._
    map[1625] = '"', invmap['"'] = 1625;    //" ._.._.
    map[617] = '(', invmap['('] = 617;      //( _.__.
    map[2470] = ')', invmap[')'] = 2470;    //) _.__._
    map[5526] = '$', invmap['$'] = 5526;    //$ ..._.._
    // map[85] = '&', invmap['&'] = 85;        //& ....
    map[1689] = '@', invmap['@'] = 1689;  //@ .__._.
    memset(BUF, 0, sizeof(BUF));
    len = 0;
    clear(&que);
}

#ifndef Master_Mode
void DEBUG(uint8_t x) {
    char tmp[5];
    sprintf(tmp, "%c", x);
    tmp[1] = '<';
    tmp[2] = '=';
    HAL_UART_Transmit(&huart1, tmp, 3, 0xfff);
}

void DEBUGint(int x) {
    char tmp[5];
    sprintf(tmp, "%d", x);
    int len = strlen(tmp);
    tmp[len] = '<';
    tmp[len + 1] = '=';
    HAL_UART_Transmit(&huart1, tmp, len + 2, 0xfff);
}
#endif

void Clear_BUF() {
    memset(BUF, 0, sizeof(BUF));
    len = 0;
}

uint8_t convert_m2l() {  // morse to letter
    unsigned int hash = 0;
    if (len == 0) {
        Clear_BUF();
        return Input_Empty;
    }
    if (len > 7) {
        Clear_BUF();
        return Input_Error;
    }
    for (int i = 0; i < len; i++) {
        hash <<= 2;
        if (BUF[i] == '.')
            hash ^= 1;
        else if (BUF[i] == '_')
            hash ^= 2;
    }
    Clear_BUF();
    if (hash > map_max) return Input_Error;
    if (!map[hash]) return Input_Error;
    return map[hash];
}

void convert_l2m(uint8_t* str, uint8_t ch) {
    uint8_t tmp[10];
    int pos = 0;
    memset(str, 0, 10);
#ifndef Master_Mode
    HAL_UART_Transmit(&huart1, str, 10, 0xfff);
#endif
    int hash = invmap[ch];
    while (hash) {
        uint8_t now = hash & 3;
        if (now == 1) tmp[pos++] = '.';
        if (now == 2) tmp[pos++] = '_';
        hash >>= 2;
    }
    for (int i = 0; i < pos; i++) str[pos - i - 1] = tmp[i];
}

void flush() {
    if (empty(&que)) return;
    int cnt = 0;
    while (!empty(&que)) {
        TBUF[cnt++] = front(&que);
        pop(&que);
    }
    clear(&que);
    HAL_UART_Transmit(&huart1, TBUF, cnt, 0xfff);
}

void inc(uint8_t* x) {  // max==100
    if ((*x) < 100) (*x)++;
}

void Dot() {
#ifndef Master_Mode
    HAL_UART_Transmit(&huart1, ".", 1, 0xfff);
#endif
    BUF[len] = '.';
    inc(&len);
}

void Line() {
#ifndef Master_Mode
    HAL_UART_Transmit(&huart1, "_", 1, 0xfff);
#endif
    BUF[len] = '_';
    inc(&len);
}

void Min_Blank() {
#ifndef Master_Mode
    HAL_UART_Transmit(&huart1, "Min_Blank", 9, 0xfff);
#endif
}

void Letter_Blank() {
    uint8_t ch = convert_m2l();
    if (ch == Input_Empty) {
        return;
    }
    if (ch == Input_Error) {
        HAL_UART_Transmit(&huart1, Input_Error_BUF, sizeof(Input_Error_BUF) - 1,
                          0xfff);
        Error_Light();
        return;
    }
    push(&que, ch);
#ifndef Master_Mode
    HAL_UART_Transmit(&huart1, "Letter_Blank", 12, 0xfff);
#endif
}

void Word_Blank() {
    uint8_t ch = convert_m2l();
    if (ch == Input_Empty) {
        return;
    }
    if (ch == Input_Error) {
        HAL_UART_Transmit(&huart1, Input_Error_BUF, sizeof(Input_Error_BUF) - 1,
                          0xfff);
        Error_Light();
        return;
    }
    push(&que, ch);
    push(&que, ' ');
    flush();
#ifndef Master_Mode
    HAL_UART_Transmit(&huart1, "Word_Blank", 10, 0xfff);
#endif
}