#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H 1

void InitKeyboardDriver();
void DeinitKeyboardDriver();
static void KbdPushback(char c);
static void KbdNAFPushback(char c);

void KeyboardDriverMain(uint8_t sc);

#endif /* PS2KEYBOARD_H */