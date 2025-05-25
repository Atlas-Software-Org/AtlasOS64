#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H 1

void InitKeyboardDriver();
void DeinitKeyboardDriver();
static void KbdPushback(char c);
static void KbdNAFPushback(char c);

void __attribute__((interrupt)) KeyboardDriverMain(int *__unused);

#endif /* PS2KEYBOARD_H */