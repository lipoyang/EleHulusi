// #include <M5Stack.h>
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include "SimpleYMF825.h"
#include "WindHulusi.h"

// FM音源
SimpleYMF825 ymf825;

// 旋律管のボタン
Adafruit_MCP23X17 fingersL;
Adafruit_MCP23X17 fingersR;

int master_vol = 63;     // マスター音量
int tone_no = 5;         // 音色
int scale;               // 音階(ハ長調からどれだけ上がるか下がるか)

static int key_state = KEY_OFF; // 打鍵状態
static int ch_num = 0;          // 発声するチャンネル
static bool droneL_on = false;  // ドローン管(左)オン
static bool droneR_on = false;  // ドローン管(右)オン
static int V_off = 0;           // ブレスセンサ出力のオフセット(12bitサンプル生値)

void DisplayUI_begin();
void DisplayUI_loop(int octave, int key12, int vol);
void DipslayUI_error(const char* error);

void finger_begin();
void finger_input(int &octave, int &key12);
void breath_begin();
void breath_input(int &vol);
void button_input();
void scale_calc(int &octave, int &key12);
void sound_output(int octave, int key12, int vol);

// 初期化
void setup()
{
    // UIの初期化
    DisplayUI_begin();
    // シリアルポートの初期化(デバッグ用)
//  Serial.begin(115200); // ← DisplayUI_begin()を実行する場合には不要
    
    // FM音源の初期化 (チャンネルに音色を割り当て)
    ymf825.begin(IOVDD_3V3, PIN_FM_RESET, PIN_FM_SS);
    ymf825.setMasterVolume(master_vol);

    // 旋律管の初期化
    finger_begin();
    
    // ドローン管の初期化
    pinMode(PIN_SW_L,  INPUT_PULLUP);
    pinMode(PIN_SW_R,  INPUT); // #36 はプルアップ不可 (外部プルアップする)
    pinMode(PIN_LED_L, OUTPUT);
    pinMode(PIN_LED_R, OUTPUT);
    digitalWrite(PIN_LED_L, LOW);
    digitalWrite(PIN_LED_R, LOW);
    
    // ブレスセンサの初期化
    breath_begin();
}

// メインループ
void loop()
{
    int octave;      // オクターブ
    int key12;       // 12音音階番号
    int vol;         // 音量
    
    // ボタン入力
    button_input();
    
    // 旋律管の入力
    finger_input(octave, key12);
    
    // ブレスセンサの入力
    breath_input(vol);
     
    // 調性の計算
    scale_calc(octave, key12);
    
    // サウンド出力
    sound_output(octave, key12, vol);
    
    // UIの処理
    DisplayUI_loop(octave, key12, vol);
    
    delay(5);
}

// 旋律管の初期化
void finger_begin()
{
    // 左手
    if (!fingersL.begin_I2C(ADDR_LEFT)) {
        Serial.println("Left Hand Error.");
        DipslayUI_error("Left Hand 1");
        while (1);
    }else{
        Serial.println("Left Hand OK.");
    }
    for(int i = 0; i < 8; i++){
        fingersL.pinMode(i, INPUT_PULLUP);
    }
    // 右手
    if (!fingersR.begin_I2C(ADDR_RIGHT)) {
        Serial.println("Right Hand Error.");
        DipslayUI_error("Right Hand 1");
        while (1);
    }else{
        Serial.println("Right Hand OK.");
    }
    for(int i = 0; i < 8; i++){
        fingersR.pinMode(i, INPUT_PULLUP);
    }
}

// 旋律管の入力
void finger_input(int &octave, int &key12)
{
    // 指使い入力の取得
    uint8_t data_l = fingersL.readGPIOA();
    uint8_t data_r = fingersR.readGPIOA();
    static bool wasError = false;
    bool isError = false;
    if(data_l == 0xFF){
        isError = true;
        DipslayUI_error("Left Hand 2");
        delay(500);
    }
    if(data_r == 0xFF){
        isError = true;
        DipslayUI_error("Right Hand 2");
        delay(500);
    }
    if(wasError && !isError){
        DipslayUI_error("");
    }
    wasError = isError;
    
    uint8_t finger = ((data_l & 0x0F) << 4) | (data_r & 0x0F);
    // Serial.printf("finger = %02X\n", finger);
    
    // 指使いの判定
    int octave_up = ((finger & 0x80) != 0) ? 1 : 0;
    finger &= 0x7F;
    int key = 12;
    for(int i = 0; i < 13; i++){
        if(finger <= FINGER_TABLE[i]){
            key = i;
            break;
        }
    }
    // 高いドの場合
    if(key == 12){
        octave_up++;
    }
    // 音階の決定
    octave = 4 + octave_up;
    key12 = KEY_TABLE[key];
}

// ブレスセンサの初期化
void breath_begin()
{
    // アナログ入力の初期化
    pinMode(PIN_BREATH, ANALOG);
    delay(100);
    
    // オフセット電圧の取得
    const int AVERAGE_TIME = 8;
    int V_acc = 0;
    for(int i = 0; i < AVERAGE_TIME; i++){
        int V = analogRead(PIN_BREATH);
        Serial.printf("V = %d\n",V);
        V_acc += V;
        delay(50);
    }
    V_off = V_acc / AVERAGE_TIME;
    Serial.printf("V_off = %d\n",V_off);
}

// ブレスセンサの入力
void breath_input(int &vol)
{
    static const int V_MAX   = 80; // 最大ゲージ圧
    static const int Vol_MAX = 31; // 最大音量
    
    // アナログ入力の取得
    const int AVERAGE_TIME = 16;
    int V = 0;
    for(int i = 0; i < AVERAGE_TIME; i++){
        int V_temp = analogRead(PIN_BREATH);
        V += V_temp;
    }
    V = V / AVERAGE_TIME - V_off;
    
    // 音量への換算
    vol = V * Vol_MAX / V_MAX;
    if(vol > Vol_MAX) vol = Vol_MAX;
    if(vol < 1) vol = 0;

    // Serial.printf("V = %4d, vol = %2d\n",V, vol);
}

// ボタン入力
void button_input()
{
    // ドローン管のボタン判定
    static uint8_t sw_l = 0x00;
    static uint8_t sw_r = 0x00;
    sw_l <<= 1;
    sw_r <<= 1;
    sw_l |= (digitalRead(PIN_SW_L) == LOW) ? 0x01 : 0x00;
    sw_r |= (digitalRead(PIN_SW_R) == LOW) ? 0x01 : 0x00;
    
    if(       (droneL_on == false) && ((sw_l & 0x0F) == 0x0F)){
        droneL_on = true;
        digitalWrite(PIN_LED_L, LOW);
        Serial.println("SW-L ON");
    }else if ((droneL_on == true ) && ((sw_l & 0x0F) == 0x00)){
        droneL_on = false;
        digitalWrite(PIN_LED_L, HIGH);
        Serial.println("SW-L OFF");
    }
    if(       (droneR_on == false) && ((sw_r & 0x0F) == 0x0F)){
        droneR_on = true;
        digitalWrite(PIN_LED_R, LOW);
        Serial.println("SW-R ON");
    }else if ((droneR_on == true ) && ((sw_r & 0x0F) == 0x00)){
        droneR_on = false;
        digitalWrite(PIN_LED_R, HIGH);
        Serial.println("SW-R OFF");
    }
    
    // マスター音量 (M5本体のボタンで設定)
    static int master_vol_old = -1;
    if(master_vol != master_vol_old){
        master_vol_old = master_vol;
        ymf825.setMasterVolume(master_vol);
    }
    
    // 音色 (M5本体のボタンで設定)
    static int tone_no_old = -1;
    if(tone_no != tone_no_old){
        tone_no_old = tone_no;
        for(int i=0; i<16; i++){
            ymf825.keyoff(i);
        }
        key_state = KEY_OFF;
    }
    
    // 音階 (M5本体のボタンで設定)
    ; // ここですることは特にない
}

// 調性の計算
void scale_calc(int &octave, int &key12)
{
#if 0
    // 調号
    int key_sign = KEY_TABLE[scale][key7];
    key12 += key_sign;
#endif
    key12 += scale;
    if(key12 >= 12){
        key12 -= 12;
        octave++;
    }
    else if(key12 < 0){
        key12 += 12;
        octave--;
    }
}

// サウンド出力
void sound_output(int octave, int key12, int vol)
{
    // 管楽器系
    if(tone_no >= 4)
    {
        switch(key_state){
            case KEY_OFF:
                if(vol > 0){
                    ch_num = (ch_num + 1) & 0x0F; // mod 16
                    ymf825.keyoff(ch_num);
                    ymf825.setTone(ch_num, TONE_TABLE[tone_no] );
                    ymf825.keyon(ch_num, octave, key12, vol);
                    //Serial.printf("Key On %d, %d, %d, %d\n", ch_num, octave, key12, vol);
                    key_state = KEY_ON1;
                }
                break;
            case KEY_ON1:
                if(vol == 0){
                    ymf825.keyoff(ch_num);
                    //Serial.println("Key Off");
                    key_state = KEY_OFF;
                }else{
                    ymf825.keyon(ch_num, octave, key12, vol);
                    //Serial.printf("Key On %d, %d, %d, %d\n", ch_num, octave, key12, vol);
                }
                break;
        }
    }
    // 弦楽器系
    else
    {
        static int vol_old = 0;
        static int keyon_cnt = 0;
        bool is_keyon = false;
        switch(key_state){
            case KEY_OFF:
                if(vol > STRINGS_MIN){
                    key_state = KEY_ON1;
                    vol_old = vol;
                    keyon_cnt = 0;
                }
                break;
            case KEY_ON1:
                if(vol <= STRINGS_MIN){
                    key_state = KEY_OFF;
                }
                keyon_cnt++;
                if(keyon_cnt > 20){
                    is_keyon = true;
                }else{
                    if(vol >= vol_old){
                        vol_old = vol;
                    }else{
                        is_keyon = true;
                    }
                }
                if(is_keyon){
                    ch_num = (ch_num + 1) & 0x0F; // mod 16
                    ymf825.keyoff(ch_num);
                    ymf825.setTone(ch_num, TONE_TABLE[tone_no] );
                    ymf825.keyon(ch_num, octave, key12, vol_old);
                    //Serial.printf("Key On %d, %d, %d, %d\n", ch_num, octave, key12, vol);
                    key_state = KEY_ON2;
                }
                break;
            case KEY_ON2:
                if(vol <= STRINGS_MIN){
                    //ymf825.keyoff(0);
                    key_state = KEY_OFF;
                }
                break;
        }
    }
}

