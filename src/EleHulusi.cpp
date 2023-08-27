// #include <M5Stack.h>
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include "SimpleYMF825.h"
#include "EleHulusi.h"

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

static int droneL_octave = 4;     // ドローン管(左)のオクターブ
static int droneL_key    = KEY_G; // ドローン管(左)の音階
static int droneR_octave = 4;     // ドローン管(右)のオクターブ
static int droneR_key    = KEY_C; // ドローン管(右)の音階

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
    digitalWrite(PIN_LED_L, HIGH);
    digitalWrite(PIN_LED_R, HIGH);
    
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
    
    // 左親指開放で1オクターブ上がる
    int octave_up = ((finger & 0x80) != 0) ? 1 : 0;
    finger &= 0x7F;

    // 低い音の変則的な指使い
    if((finger != 00) && ((finger & 0x0F) == 0x00))
    {
        int key = FINGER_TABLE2_SIZE - 1;
        for(int i = 0; i < FINGER_TABLE2_SIZE; i++){
            if(finger <= FINGER_TABLE2[i]){
                key = i;
                break;
            }
        }
        // 中央ド以上の場合
        if(key >= MIDDLE_C){
            octave_up++;
        }
        key12 = KEY_TABLE2[key];
        octave = 3 + octave_up;
    }
    // 通常指使い
    else{
        int key = FINGER_TABLE_SIZE - 1;
        for(int i = 0; i < FINGER_TABLE_SIZE; i++){
            if(finger <= FINGER_TABLE[i]){
                key = i;
                break;
            }
        }
        // 高いド以上の場合
        if(key >= HIGH_C){
            octave_up++;
        }
        key12 = KEY_TABLE[key];
        octave = 4 + octave_up;
    }
}

// ブレスセンサの初期化
void breath_begin()
{
    // アナログ入力の初期化
//  pinMode(PIN_BREATH, ANALOG);
//  delay(100);

    // A/Dコンバータの設定
    Wire.beginTransmission(ADDR_MCP3425);
    Wire.write(CONFIG_MCP3425);
    Wire.endTransmission();
    delay(50);
    
    // オフセット電圧の取得
    const int AVERAGE_TIME = 8;
    int V_acc = 0;
    for(int i = 0; i < AVERAGE_TIME; i++){
//      int V = analogRead(PIN_BREATH);
        
        Wire.requestFrom(ADDR_MCP3425, 2);
        int V = (Wire.read() << 8 ) + Wire.read();
        
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
    static const int V_MAX   = 180; // 最大ゲージ圧 要調整
    static const int Vol_MAX = 31; // 最大音量
    
    // アナログ入力の取得
//  const int AVERAGE_TIME = 16;
//  int V = 0;
//  for(int i = 0; i < AVERAGE_TIME; i++){
//      int V_temp = analogRead(PIN_BREATH);
//      V += V_temp;
//  }
//  V = V / AVERAGE_TIME - V_off;

    Wire.requestFrom(ADDR_MCP3425, 2);
    int V = (Wire.read() << 8 ) + Wire.read();
    V = V - V_off;
    
    // 音量への換算
    vol = V * Vol_MAX / V_MAX;
    if(vol > Vol_MAX) vol = Vol_MAX;
    if(vol < 1) vol = 0;
    if(vol == 1) vol = 2; // ?

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
    sw_l |= (digitalRead(PIN_SW_L) == LOW) ? 0x00 : 0x01;
    sw_r |= (digitalRead(PIN_SW_R) == LOW) ? 0x00 : 0x01;
    
    if(       (droneL_on == false) && (sw_l == 0xF0)){
        droneL_on = true;
        digitalWrite(PIN_LED_L, LOW);
        Serial.println("SW-L ON");
    }else if ((droneL_on == true ) && (sw_l == 0xF0)){
        droneL_on = false;
        digitalWrite(PIN_LED_L, HIGH);
        Serial.println("SW-L OFF");
    }
    if(       (droneR_on == false) && (sw_r == 0xF0)){
        droneR_on = true;
        digitalWrite(PIN_LED_R, LOW);
        Serial.println("SW-R ON");
    }else if ((droneR_on == true ) && (sw_r == 0xF0)){
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

    // ドローン管(左)
    droneL_octave = 4;
    droneL_key = KEY_G;
    droneL_key += scale;
    if(droneL_key >= 12){
        droneL_key -= 12;
        droneL_octave++;
    }
    else if(droneL_key < 0){
        droneL_key += 12;
        droneL_octave--;
    }

    // ドローン管(右)
    droneR_octave = 4;
    droneR_key = KEY_G;
    droneR_key += scale;
    if(droneR_key >= 12){
        droneR_key -= 12;
        droneR_octave++;
    }
    else if(droneR_key < 0){
        droneR_key += 12;
        droneR_octave--;
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
                    ch_num = (ch_num + 3) & 0x0F; // mod 16

                    ymf825.keyoff(ch_num);
                    ymf825.setTone(ch_num, TONE_TABLE[tone_no] );
                    ymf825.keyon(ch_num, octave, key12, vol);
                    key_state = KEY_ON1;

                    if(droneL_on){
                        int ch_drone = (ch_num + 1) & 0x0F;
                        ymf825.keyoff(ch_drone);
                        ymf825.setTone(ch_drone, TONE_TABLE[tone_no] );
                        ymf825.keyon(ch_drone, droneL_octave, droneL_key, vol);
                    }
                    if(droneR_on){
                        int ch_drone = (ch_num + 2) & 0x0F;
                        ymf825.keyoff(ch_drone);
                        ymf825.setTone(ch_drone, TONE_TABLE[tone_no] );
                        ymf825.keyon(ch_drone, droneR_octave, droneR_key, vol);
                    }
                    //Serial.printf("Key On %d, %d, %d, %d\n", ch_num, octave, key12, vol);
                }
                break;
            case KEY_ON1:
                if(vol == 0){
                    ymf825.keyoff(ch_num);
                    ymf825.keyoff((ch_num + 1) & 0x0F);
                    ymf825.keyoff((ch_num + 2) & 0x0F);
                    key_state = KEY_OFF;
                    //Serial.println("Key Off");
                }else{
                    ymf825.keyon(ch_num, octave, key12, vol);
                    if(droneL_on){
                        int ch_drone = (ch_num + 1) & 0x0F;
                        ymf825.keyon(ch_drone, droneL_octave, droneL_key, vol);
                    }
                    if(droneR_on){
                        int ch_drone = (ch_num + 2) & 0x0F;
                        ymf825.keyon(ch_drone, droneR_octave, droneR_key, vol);
                    }
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

