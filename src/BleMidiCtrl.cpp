#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// BLE-MIDIのインスタンス
BLEMIDI_CREATE_INSTANCE("EleHulusi", MIDI);

// 接続したか？
static bool IsConnected = false;

// ノートナンバー
static byte note_number = 0;

// 接続時の処理
static void OnConnected()
{
  IsConnected = true;
  Serial.println("Connected!");
}
// 切断時の処理
static void OnDisconnected()
{
  IsConnected = false;
  Serial.println("Disconnected!");
}
// ノートオン時の処理
static void OnNoteOn(byte channel, byte note, byte velocity)
{
}
// ノートオフ時の処理
static void OnNoteOff(byte channel, byte note, byte velocity)
{
}

// MIDIの初期化
void BleMidiCtrl_begin()
{
    MIDI.begin();
    BLEMIDI.setHandleConnected(OnConnected);
    BLEMIDI.setHandleDisconnected(OnDisconnected);
    //MIDI.setHandleNoteOn(OnNoteOn);
    //MIDI.setHandleNoteOff(OnNoteOff);
}

// MIDIのメインループ処理
void BleMidiCtrl_loop()
{
    MIDI.read();
}

// ノートオン
void BleMidiCtrl_noteOn(int octave, int key, int vol, byte ch)
{
    byte velo = vol*4;
    MIDI.sendNoteOff(note_number, velo, ch);
    note_number = (byte)((octave+1)*12 + key);
    MIDI.sendNoteOn(note_number, velo, ch);
}

void BleMidiCtrl_noteOn(int vol, byte ch)
{
    byte velo = vol*4;
    MIDI.sendNoteOn(note_number, velo, ch);
}

// ノートオフ
void BleMidiCtrl_noteOff(int vol, byte ch)
{
    byte velo = vol*4;
    MIDI.sendNoteOff(note_number, velo, ch);
}