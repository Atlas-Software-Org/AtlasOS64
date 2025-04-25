#include "language.h"

LanguageType cur_type;

void SetLanguage(LanguageType type) {
    cur_type = type;
}

LanguageType GetLanguage() {
    return cur_type;
}

char Username[16] = {0};

void SetUsername(char* Name) {
    int len = 16;
    for (int i = 0; i < 16; i++) {
        if (Name[i] == ' ') {
            len = i;
            break;
        }
    }
    for (int i = 0; i < len; i++) {
        Username[i] = Name[i];
    }
    Username[len] = '\0';  // Null-terminate the string
}

char* GetUsername() {
    return Username;
}