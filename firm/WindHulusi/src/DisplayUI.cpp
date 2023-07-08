#include <M5Stack.h>

// 外部参照
extern int master_vol;      // マスター音量
extern int tone_no;         // 音色
extern int scale;           // 音階(ハ長調からどれだけ上がるか下がるか)

// 音色番号の最大値
#define TONE_MAX 7

// カーソル位置
#define POS_NORMAL  0   // 通常状態
#define POS_TONE    1   // 音色変更
#define POS_SCALE   2   // 調性変更
#define POS_VOLUME  3   // 音量変更
#define POS_MAX     POS_VOLUME
static int cursol_pos = POS_NORMAL;

// 描画位置
#define X_TONE      10
#define Y_TONE      10
#define W_TONE      300
#define X_SCALE     10
#define Y_SCALE     170
#define W_SCALE     140
#define X_VOLUME    170
#define Y_VOLUME    170
#define W_VOLUME    140
#define X_KEY       120
#define Y_KEY       80
#define W_KEY       100
#define H_KEY       64
#define H_CHAR      48
#define P_CHAR      24
#define R_CHAR      5

// スプライト
TFT_eSprite sprite = TFT_eSprite(&M5.Lcd);

// サブルーチン
static void DisplayUI_frame();
static void DisplayUI_settings();
static void DisplayUI_sound(int octave, int key12, int vol);

// 初期化
void DisplayUI_begin()
{
    M5.begin(true, false, true); // LCD:ON, SDカード:OFF, シリアル:ON
//  M5.Power.begin();
    M5.Lcd.fillScreen(BLACK);

    // スプライトの作成
    sprite.setColorDepth(16);
    sprite.createSprite(W_KEY, H_KEY);

    DisplayUI_frame();
    DisplayUI_settings();
}

// メインループ処理
void DisplayUI_loop(int octave, int key12, int vol)
{
    M5.update();
    
    // ボタン入力判定とそれに対する処理
    if (M5.BtnA.wasPressed()) {
        cursol_pos++;
        if(cursol_pos > POS_MAX) cursol_pos = POS_NORMAL;
        DisplayUI_settings();
    }
    if (cursol_pos != POS_NORMAL)
    {
        int change_val = 0;
        if (M5.BtnB.wasPressed()){
            change_val = 1;
        }else if(M5.BtnC.wasPressed()){
            change_val = -1;
        }
        if(change_val != 0){
            switch(cursol_pos){
            case POS_TONE:
                tone_no += change_val;
                if(tone_no < 0) tone_no = 0;
                if(tone_no > TONE_MAX) tone_no = TONE_MAX;
                break;
            case POS_SCALE:
                scale += change_val;
                if(scale < -12) scale = -12;
                if(scale >  12) scale =  12; 
                break;
            case POS_VOLUME:
                master_vol += change_val;
                if(master_vol == 31) master_vol = 0;
                if(master_vol ==  1) master_vol = 32;
                if(master_vol <   0) master_vol = 0;
                if(master_vol >  63) master_vol = 63;
                break;
            }
            DisplayUI_settings();
        }
    }
    
    // サウンド出力の描画
    static int cnt = 0;
    cnt++;
    if(cnt >= 10){
        cnt = 0;
        DisplayUI_sound(octave, key12, vol);
    }
}

// 枠線の描画
static void DisplayUI_frame()
{
    M5.Lcd.drawRoundRect(
        X_TONE - R_CHAR, Y_TONE - R_CHAR,
        W_TONE + R_CHAR * 2, H_CHAR + R_CHAR * 2,
        R_CHAR, WHITE);//YELLOW); 
    M5.Lcd.drawRoundRect(
        X_SCALE - R_CHAR, Y_SCALE - R_CHAR,
        W_SCALE + R_CHAR * 2, H_CHAR + R_CHAR * 2,
        R_CHAR, WHITE);//YELLOW); 
    M5.Lcd.drawRoundRect(
        X_VOLUME - R_CHAR, Y_VOLUME - R_CHAR,
        W_VOLUME + R_CHAR * 2, H_CHAR + R_CHAR * 2,
        R_CHAR, WHITE);//YELLOW); 
}

// 設定の描画
static void DisplayUI_settings()
{
    static const char TONE_NAME[][13] = {
        "Grand Piano",
        "Tinkle Bell",
        "Nylon Guiter",
        "Harpsichord",
        "Church Organ",
        "Flute",
        "Rock Organ",
        "Harmonica"
    };
  
//    M5.Lcd.setTextFont(4);
//    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextSize(3);

    // 音色
    if(cursol_pos == POS_TONE){
        M5.Lcd.setTextColor(RED); //, BLACK);
    }else{
        M5.Lcd.setTextColor(YELLOW); //, BLACK);
    }
    M5.Lcd.fillRect(X_TONE,Y_TONE,W_TONE,H_CHAR,BLACK);
    M5.Lcd.setCursor(X_TONE+P_CHAR,Y_TONE);
    M5.Lcd.print(TONE_NAME[tone_no]);

    // 調性
    if(cursol_pos == POS_SCALE){
        M5.Lcd.setTextColor(RED); //, BLACK);
    }else{
        M5.Lcd.setTextColor(YELLOW); //, BLACK);
    }
    M5.Lcd.fillRect(X_SCALE,Y_SCALE,W_SCALE,H_CHAR,BLACK);
    M5.Lcd.setCursor(X_SCALE+P_CHAR,Y_SCALE);
    if(scale > 0){
        M5.Lcd.printf("+%d", scale);
    }else{
        M5.Lcd.printf("%d", scale);
    }

    // 音量
    if(cursol_pos == POS_VOLUME){
        M5.Lcd.setTextColor(RED); //, BLACK);
    }else{
        M5.Lcd.setTextColor(YELLOW); //, BLACK);
    }
    M5.Lcd.fillRect(X_VOLUME,Y_VOLUME,W_VOLUME,H_CHAR,BLACK);
    M5.Lcd.setCursor(X_VOLUME+P_CHAR,Y_VOLUME);
    int vol = (master_vol >= 32) ? master_vol - 31 : 0;
    M5.Lcd.printf("%d", vol);
}

// サウンドの描画
static void DisplayUI_sound(int octave, int key12, int vol)
{
    // オクターブごとの表示色のRGB値 (M5の色定数は5:6:5の16ビットカラー)
    static const uint16_t COLOR_TABLE[][3] = {
        { (ORANGE >> 11) & 0x1F, (ORANGE >> 5) & 0x3F, ORANGE & 0x1F }, // O3 オレンジ色
        { (YELLOW >> 11) & 0x1F, (YELLOW >> 5) & 0x3F, YELLOW & 0x1F }, // O4 黄色
        { (GREEN  >> 11) & 0x1F, (GREEN  >> 5) & 0x3F, GREEN  & 0x1F }, // O5 緑色
        { (CYAN   >> 11) & 0x1F, (CYAN   >> 5) & 0x3F, CYAN   & 0x1F }, // O6 水色
    };
    // 音階の名前
    static const char KEY_NAME[12][3] = {
      "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" 
    };
    
    // 音量によって明度を変える
    int index = octave - 3;
    uint16_t r = COLOR_TABLE[index][0];
    uint16_t g = COLOR_TABLE[index][1];
    uint16_t b = COLOR_TABLE[index][2];
    r = r * vol / 31;
    g = g * vol / 31;
    b = b * vol / 31;
    uint16_t color = (r << 11) | (g << 5) | b;
    
#if 0
    M5.Lcd.fillRect(X_KEY,Y_KEY,W_KEY,H_KEY,BLACK);
    M5.Lcd.setTextColor(color);//, BLACK);
    M5.Lcd.setCursor(X_KEY,Y_KEY);
    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print(KEY_NAME[key12]);
#else
    sprite.fillRect(0,0,W_KEY,H_KEY,BLACK);
    sprite.setTextColor(color);//, BLACK);
    sprite.setCursor(0,0);
    sprite.setTextFont(2);
    sprite.setTextSize(4);
    sprite.print(KEY_NAME[key12]);
    sprite.pushSprite(X_KEY, Y_KEY); 
#endif
}
