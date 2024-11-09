#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Definirea tipurilor de token
typedef enum {
    Keyword, Identificator, Separator, Operator, Comentariu, String, Numar, Eroare, End_of_file
} TokenTypes;

// Struct pentru token
typedef struct {
    TokenTypes type;
    char* value;
    int length;
    int line;
} Tokens;

// Functie care verifica daca un sir este keyword
int iskeyword(const char* sir) {
    const char* keyword[] = {"int", "string", "printf", "scanf", "if", "else", "return", NULL};
    for (int i = 0; keyword[i] != NULL; i++) {
        if (strcmp(sir, keyword[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Functie care transforma un token in sir de caractere
const char* tokenString(TokenTypes t) {
    switch (t) {
        case Keyword:
            return "Keyword";
        case Identificator:
            return "Identificator";
        case Separator:
            return "Separator";
        case Operator:
            return "Operator";
        case Comentariu:
            return "Comentariu";
        case String:
            return "String";
        case Numar:
            return "Numar";
        case Eroare:
            return "Eroare";
        case End_of_file:
            return "End_of_file";
        default:
            return "?";
    }
}

// Functia care citeste urmatorul token din fisier
Tokens nextToken(FILE* in, int* line) {
    static int lastChar = ' ';
    char buffer[255];
    int i = 0;

    // Sarim peste spatii
    while (isspace(lastChar)) {
        if (lastChar == '\n') {
            (*line)++;
        }
        lastChar = fgetc(in);
    }

    // Finalul fisierului
    if (lastChar == EOF) {
        return (Tokens){End_of_file, NULL, 0, *line};
    }

    // Verificam daca token-ul este numar
    if (isdigit(lastChar)) {
        do {
            buffer[i++] = lastChar;
            lastChar = fgetc(in);
        } while (isalnum(lastChar));
        buffer[i] = '\0';
        return (Tokens){Numar, strdup(buffer), i, *line};
    }

    // Verificam daca token-ul este keyword sau identificator
    if (isalpha(lastChar) || lastChar == '_') {
        do {
            buffer[i++] = lastChar;
            lastChar = fgetc(in);
        } while (isalnum(lastChar) || lastChar == '_');
        buffer[i] = '\0';
        if (iskeyword(buffer)) {
            return (Tokens){Keyword, strdup(buffer), i, *line};
        }
        else {
            return (Tokens){Identificator, strdup(buffer), i, *line};
        }
    }

    // Verificam daca token-ul este string
    if (lastChar == '"') {
        buffer[i++] = lastChar;
        lastChar = fgetc(in);
        while (lastChar != '"' && lastChar != EOF) {
            if (lastChar == '\\') {
                buffer[i++] = lastChar;
                lastChar = fgetc(in);
            }
            buffer[i++] = lastChar;
            lastChar = fgetc(in);
        }
        buffer[i++] = '"';
        lastChar = fgetc(in);
        buffer[i] = '\0';
        return (Tokens){String, strdup(buffer), i, *line};
    }

    // Verificam daca token-ul este comentariu
    if (lastChar == '/') {
        int nextChar = fgetc(in);
        if (nextChar == '*') {
            buffer[i++] = '/';
            buffer[i++] = '*';
            while (1) {
                lastChar = fgetc(in);
                if (lastChar == EOF) {
                    buffer[i] = '\0';
                    return (Tokens){Eroare, "Comentariu multi-linie neterminat", i, *line};
                }
                if (lastChar == '\n') (*line)++;
                buffer[i++] = lastChar;
                if (lastChar == '*' && (lastChar = fgetc(in)) == '/') {
                    buffer[i++] = '/';
                    break;
                }
            }
            buffer[i] = '\0';
            lastChar = fgetc(in);
            return (Tokens){Comentariu, strdup(buffer), i, *line};
        }
        else if (nextChar == '/') {
            buffer[i++] = '/';
            buffer[i++] = '/';
            while (lastChar != '\n' && lastChar != EOF) {
                lastChar = fgetc(in);
                buffer[i++] = lastChar;
            }
            buffer[i] = '\0';
            lastChar = fgetc(in);
            return (Tokens){Comentariu, strdup(buffer), i, *line};
        }
        else {
            lastChar = '/';
            return (Tokens){Operator, strdup("/"), 1, *line};
        }
    }

    // Verificam daca token-ul este operator
    if (strchr("+-*/%=&|!<>", lastChar)) {
        buffer[i++] = lastChar;
        int nextChar = fgetc(in);
        if ((buffer[0] == '+' && nextChar == '+') || (buffer[0] == '-' && nextChar == '-') || (buffer[0] == '=' && nextChar == '=') ||
            (buffer[0] == '!' && nextChar == '=') || (buffer[0] == '&' && nextChar == '&') || (buffer[0] == '|' && nextChar == '||')) {
            buffer[i++] = nextChar;
            lastChar = fgetc(in);
        }
        else {
            lastChar = nextChar;
        }
        buffer[i] = '\0';
        return (Tokens){Operator, strdup(buffer), i, *line};
    }

    // Verificam daca token-ul este separator
    if (strchr("(){};,", lastChar)) {
        buffer[i++] = lastChar;
        buffer[i] = '\0';
        lastChar = fgetc(in);
        return (Tokens){Separator, strdup(buffer), i, *line};
    }

    // Token-ul nu a fost identificat
    buffer[i++] = lastChar;
    lastChar = fgetc(in);
    buffer[i] = '\0';
    return (Tokens){Eroare, strdup(buffer), i, *line};
}

int main() {
    FILE* in = fopen("intrare.txt", "r");
    int line = 1;
    Tokens token;
    while ((token = nextToken(in, &line)).type != End_of_file) {
        printf("Token: \"%s\", Type: %s, Length: %d, Line: %d\n", token.value, tokenString(token.type), token.length, token.line);
    }
    fclose(in);
    return 0;
}
