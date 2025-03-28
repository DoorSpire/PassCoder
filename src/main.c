#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define CHARSET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:'\",.<>?/`~"
#define CHARSET_SIZE (sizeof(CHARSET) - 1)

int getCharIndex(char c) {
    for (int i = 0; i < CHARSET_SIZE; i++) {
        if (CHARSET[i] == c) {
            return i;
        }
    }
    return -1;
}

char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 2);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

void writeFile(const char* path, const char* data) {
    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\" for writing.\n", path);
        exit(74);
    }

    size_t dataLength = strlen(data);
    size_t bytesWritten = fwrite(data, sizeof(char), dataLength, file);

    if (bytesWritten < dataLength) {
        fprintf(stderr, "Could not write to file \"%s\".\n", path);
        exit(74);
    }

    fclose(file);
}

int startsWith(const char *str, const char *prefix) {
    if (strlen(prefix) > strlen(str)) {
        return 0;
    }

    return strncmp(str, prefix, strlen(prefix)) == 0;
}

int endsWith(const char *str, const char *suffix) {
    size_t strLen = strlen(str);
    size_t suffixLen = strlen(suffix);

    if (suffixLen > strLen) {
        return 0;
    }

    return strcmp(str + (strLen - suffixLen), suffix) == 0;
}

char* generatePassword(int length) {
    int charset_len = strlen(CHARSET);

    if (length > charset_len) {
        printf("Error: Password length cannot exceed the number of unique characters available in the charset.\n");
        exit(1);
    }

    char *password = (char *)malloc(length + 1);
    if (password == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    int *used = (int *)calloc(charset_len, sizeof(int));
    if (used == NULL) {
        printf("Memory allocation for 'used' array failed!\n");
        exit(1);
    }

    srand(time(NULL));

    for (int i = 0; i < length; i++) {
        int randIndex;
        do {
            randIndex = rand() % charset_len;
        } while (used[randIndex] == 1);

        password[i] = CHARSET[randIndex];
        used[randIndex] = 1;
    }

    password[length] = '\0';
    free(used);
    return password;
}

void vigenereEncrypt(char *plaintext, char *key, char *ciphertext) {
    int textLen = strlen(plaintext);
    int keyLen = strlen(key);

    for (int i = 0, j = 0; i < textLen; i++) {
        int textIndex = getCharIndex(plaintext[i]);
        if (textIndex != -1) {
            int keyIndex = getCharIndex(key[j % keyLen]);
            int cipherIndex = (textIndex + keyIndex) % CHARSET_SIZE; 
            ciphertext[i] = CHARSET[cipherIndex];
            j++;
        } else {
            ciphertext[i] = plaintext[i];
        }
    }
    ciphertext[textLen] = '\0';
}

void vigenereDecrypt(char *ciphertext, char *key, char *plaintext) {
    int textLen = strlen(ciphertext);
    int keyLen = strlen(key);

    for (int i = 0, j = 0; i < textLen; i++) {
        int textIndex = getCharIndex(ciphertext[i]);
        if (textIndex != -1) {
            int keyIndex = getCharIndex(key[j % keyLen]);
            int plainIndex = (textIndex - keyIndex + CHARSET_SIZE) % CHARSET_SIZE;
            plaintext[i] = CHARSET[plainIndex];
            j++;
        } else {
            plaintext[i] = ciphertext[i];
        }
    }
    plaintext[textLen] = '\0';
}

int main() {
    printf("PASSCODER\n\n");
    printf("Keybinds:\n quit - exit the program\n access <key> - get your passwords\n generate <length> - generate a password with provided data\n add <service:password:username> - after you run the cmd, it will ask for the key and then it will add you thing\n");
    printf("WARNING: SERVICES MUST BE MORE THAN 7 CHARACTERS OR THE PROGRAM WILL BREAK IDK WHY LOL\n");

    while (1) {
        printf(">> ");
        char input[1024];
        int i = 0;
        int c = getchar();

        while (c != '\n' && i < 1023) {
            input[i] = c;
            i++;
            c = getchar();
        }

        input[i] = '\0';

        if (startsWith(input, "access ")) {
            char plainText[1024];
            char* key = input + strlen("access ");
            vigenereDecrypt(readFile("encoded.pass"), key, plainText);
            printf("%s\n", plainText);
        } else if (startsWith(input, "add ")) {
            char cipherText[1024];
            char* service = input + strlen("add ");
            printf("Key >> ");
            char key[1024];
            int ii = 0;
            int cc = getchar();

            while (cc != '\n' && ii < 1023) {
                key[ii] = cc;
                ii++;
                cc = getchar();
            }

            key[i] = '\0';
            char plainText[4096];
            char newPlainText[4096];

            vigenereDecrypt(readFile("encoded.pass"), key, plainText);
            snprintf(newPlainText, sizeof(newPlainText), "%s\n%s", plainText, service);
            vigenereEncrypt(newPlainText, key, cipherText);
            writeFile("encoded.pass", cipherText);
        } else if (startsWith(input, "generate ")) {
            int len = atoi(input + strlen("generate "));
            char* pass = generatePassword(len);
            printf("%s\n", pass);
            free(pass);
        } else if (strcmp(input, "quit") == 0) {
            break;
        } else {
            printf("Invalid command\n");
        }
    }

    return 0;
}