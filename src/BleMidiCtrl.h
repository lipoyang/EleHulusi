#pragma once
#include <Arduino.h>

void BleMidiCtrl_begin();
void BleMidiCtrl_loop();
void BleMidiCtrl_noteOn(int octave, int key, int vol, byte ch);
void BleMidiCtrl_noteOn(int vol, byte ch);
void BleMidiCtrl_noteOff(int vol, byte ch);