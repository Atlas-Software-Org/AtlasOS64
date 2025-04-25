#pragma once

typedef enum {
    ENGLISH = 0,
    FRENCH = 1,
    GERMAN = 2,
    SPANISH = 3
} LanguageType;

void SetLanguage(LanguageType type);
LanguageType GetLanguage();

void SetUsername(char* Name);
char* GetUsername();