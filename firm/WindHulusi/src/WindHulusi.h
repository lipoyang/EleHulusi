#pragma once
#include <stdint.h>

// ピン番号定数
#define PIN_SW_L        26  // ドローン管(左)のスイッチ
#define PIN_SW_R        36  // ドローン管(右)のスイッチ
#define PIN_LED_L       5   // ドローン管(左)のLED
#define PIN_LED_R       2   // ドローン管(右)のLED

#define PIN_FM_SS       16   // FM音源のSPIスレーブセレクト
#define PIN_FM_RESET    17   // FM音源のリセット

#define PIN_BREATH      35  // ブレスセンサのアナログ入力

// 旋律管のIOエキスパンダのI2Cスレーブアドレス
#define ADDR_LEFT       0x20    // 左手
#define ADDR_RIGHT      0x21    // 右手

// 7音音階番号
#define KEY7_C          0
#define KEY7_D          1
#define KEY7_E          2
#define KEY7_F          3
#define KEY7_G          4
#define KEY7_A          5
#define KEY7_B          6

// 打鍵状態
#define KEY_OFF         0
#define KEY_ON1         1
#define KEY_ON2         2

// 弦楽器の最小音量閾値
#define STRINGS_MIN     5

// 指使いのテーブル(親指以外)
const uint8_t FINGER_TABLE[14] = {
    0x00, // C
    0x01, // D
    0x02, // D#
    0x03, // E
    0x07, // F
    0x0B, // F#
    0x0E, // C# ← 変則的だがひとまずここにおく
    0x0F, // G
    0x17, // G#
    0x1F, // A
    0x2F, // A#
    0x3F, // B
    0x5F, // C
    0x6F, // D
};

// 音階のテーブル
int KEY_TABLE[14] = {
    KEY_C,
    KEY_D,
    KEY_D_SHARP,
    KEY_E,
    KEY_F,
    KEY_F_SHARP,
    KEY_C_SHARP, // ← 変則的だがひとまずここにおく
    KEY_G,
    KEY_G_SHARP,
    KEY_A,
    KEY_A_SHARP,
    KEY_B,
    KEY_C,
    KEY_D,
};

#if 0
// 調号テーブル (-1:♭ +1:#)
const int KEY_TABLE[13][7] = {
//    C   D   E   F   G   A   B
    {-1, -1, -1,  0, -1, -1, -1}, // [0]:G♭ (変ト長調)
    { 0, -1, -1,  0, -1, -1, -1}, // [1]:D♭ (変ニ長調)
    { 0, -1, -1,  0,  0, -1, -1}, // [2]:A♭ (変イ長調)
    { 0,  0, -1,  0,  0, -1, -1}, // [3]:E♭ (変ホ長調)
    { 0,  0, -1,  0,  0,  0, -1}, // [4]:B♭ (変ロ長調)
    { 0,  0,  0,  0,  0,  0, -1}, // [5]:F (ヘ長調)
    { 0,  0,  0,  0,  0,  0,  0}, // [6]:C (ハ長調)
    { 0,  0,  0, +1,  0,  0,  0}, // [7]:G (ト長調)
    {+1,  0,  0, +1,  0,  0,  0}, // [8]:D (ニ長調)
    {+1,  0,  0, +1, +1,  0,  0}, // [9]:A (イ長調)
    {+1, +1,  0, +1, +1,  0,  0}, // [10]:E (ホ長調)
    {+1, +1,  0, +1, +1, +1,  0}, // [11]:B (ロ長調)
    {+1, +1, +1, +1, +1, +1,  0}, // [12]:F# (嬰ヘ長調)
};
#endif

// 音色テーブル
const int TONE_TABLE[8] = {
    // 弦楽器系
    GRAND_PIANO,
    TNKL_BELL,
    NYLON_GUITER,
    HARPSICHORD,
    // 管楽器系
    CHURCH_ORGAN,
    FLUTE,
    ROCK_ORGAN,
    HARMONICA
};
