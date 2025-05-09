#ifndef LANGUAGE_H
#define LANGUAGE_H 1

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

#endif /* LANGUAGE_H */