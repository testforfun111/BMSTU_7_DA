#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET_SIZE 16

const int8_t ALPHABET[ALPHABET_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

void my_memcpy(int8_t *dst, const int8_t *src, int len)
{
    for (int i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }
}

int8_t find(const int8_t* nums, int8_t num, int size) {
    for (int i = 0; i < size; i++) {
        if (num == nums[i])
            return i;
    }
    return -1; 
}

// Rotor struct and methods
typedef struct {
    int8_t wiring[ALPHABET_SIZE];
} Rotor;

void initRotor(Rotor* rotor, const int8_t* wiring, int position) {
    for (int i = position; i < ALPHABET_SIZE; i++) {
        rotor->wiring[i - position] = wiring[i];
    }
    for (int i = 0; i < position; i++) {
        rotor->wiring[ALPHABET_SIZE - position + i] = wiring[i];
    }
}

int8_t rotorEncryptForward(Rotor* rotor, int8_t c) {
    int index = find(rotor->wiring, c, ALPHABET_SIZE);
    return ALPHABET[index];
}

int8_t rotorEncryptBackward(Rotor* rotor, int8_t c) {
    int index = find(ALPHABET, c, ALPHABET_SIZE);
    return rotor->wiring[index];
}

void rotorRotate(Rotor* rotor) {
    int8_t tmp = rotor->wiring[0];
    for (int i = 0; i < ALPHABET_SIZE - 1; i++) {
        rotor->wiring[i] = rotor->wiring[i + 1];
    }
    rotor->wiring[ALPHABET_SIZE - 1] = tmp;
}

// Commutation Panel struct and methods
typedef struct {
    int8_t wiring[ALPHABET_SIZE];
    int8_t originalWiring[ALPHABET_SIZE];
} CommutationPanel;

void initCommutationPanel(CommutationPanel* panel, const int8_t* wiring) {
    my_memcpy(panel->originalWiring, wiring, ALPHABET_SIZE);
    my_memcpy(panel->wiring, wiring, ALPHABET_SIZE);
    for (int i = 0; i < ALPHABET_SIZE; i += 2) {
        int8_t temp = panel->wiring[i];
        panel->wiring[i] = panel->wiring[i + 1];
        panel->wiring[i + 1] = temp;
    }
}

int8_t commutate(CommutationPanel* panel, int8_t c) {
    int idx = find(panel->originalWiring, c, ALPHABET_SIZE);
    if (idx != -1) {
        return panel->wiring[idx];
    }
    return c; // No change if not found
}

// Reflector struct and methods
typedef struct {
    int8_t wiring[ALPHABET_SIZE];
} Reflector;

void initReflector(Reflector* reflector, const int8_t* wiring) {
    my_memcpy(reflector->wiring, wiring, ALPHABET_SIZE);
    for (int i = 0; i < ALPHABET_SIZE; i += 2) {
        int8_t temp = reflector->wiring[i];
        reflector->wiring[i] = reflector->wiring[i + 1];
        reflector->wiring[i + 1] = temp;
    }
}

int8_t reflect(Reflector* reflector, int8_t c) {
    return reflector->wiring[find(ALPHABET, c, ALPHABET_SIZE)];
}

// Enigma Machine structure
typedef struct {
    Rotor* rotors;
    int rotorCount;
    Reflector reflector;
    CommutationPanel panel;
    int cnt;
} Enigma;

void initEnigma(Enigma* enigma, Rotor* rotors, int rotorCount, Reflector reflector, CommutationPanel panel) {
    enigma->rotors = rotors;
    enigma->rotorCount = rotorCount;
    enigma->reflector = reflector;
    enigma->panel = panel;
    enigma->cnt = 0;
}

void rotateRotors(Enigma* enigma) {
    enigma->cnt++;
    rotorRotate(&enigma->rotors[0]);
    if (enigma->cnt % ALPHABET_SIZE == 0) {
        rotorRotate(&enigma->rotors[1]);
    }
    if (enigma->cnt % (ALPHABET_SIZE * ALPHABET_SIZE) == 0) {
        rotorRotate(&enigma->rotors[2]);
    }
}

int8_t encryptChar(Enigma* enigma, int8_t p1, int8_t p2) {
    // Process first part of the byte
    p1 = commutate(&enigma->panel, p1);
    for (int i = 0; i < enigma->rotorCount; i++) {
        p1 = rotorEncryptForward(&enigma->rotors[i], p1);
    }
    p1 = reflect(&enigma->reflector, p1);
    for (int i = enigma->rotorCount - 1; i >= 0; i--) {
        p1 = rotorEncryptBackward(&enigma->rotors[i], p1);
    }
    p1 = commutate(&enigma->panel, p1);

    // Process second part of the byte
    p2 = commutate(&enigma->panel, p2);
    for (int i = 0; i < enigma->rotorCount; i++) {
        p2 = rotorEncryptForward(&enigma->rotors[i], p2);
    }
    p2 = reflect(&enigma->reflector, p2);
    for (int i = enigma->rotorCount - 1; i >= 0; i--) {
        p2 = rotorEncryptBackward(&enigma->rotors[i], p2);
    }
    p2 = commutate(&enigma->panel, p2);

    // Combine both parts into one byte
    return (p1 << 4) | p2;
}

char* encryptMessage(Enigma* enigma, const char* message) {
    int len = strlen(message);
    char* encryptedMessage = malloc((len + 1) * sizeof(char));  
    for (int i = 0; i < len; i++) {
        int8_t p1 = (message[i] & 0xf0) >> 4;
        int8_t p2 = (message[i] & 0x0f);
        encryptedMessage[i] = encryptChar(enigma, p1, p2);
        rotateRotors(enigma);
    }
    encryptedMessage[len] = '\0'; 
    return encryptedMessage;
}

void encryptFile(Enigma* enigma, const char* inputFile, const char* outputFile) {
    FILE* inFile = fopen(inputFile, "rb");
    FILE* outFile = fopen(outputFile, "wb");

    if (!inFile || !outFile) {
        printf("Cannot open files!\n");
        return;
    }

    int8_t buffer;
    while (fread(&buffer, sizeof(int8_t), 1, inFile)) {
        int8_t p1 = (buffer & 0xf0) >> 4;
        int8_t p2 = buffer & 0x0f;
        buffer = encryptChar(enigma, p1, p2);
        rotateRotors(enigma);
        fwrite(&buffer, sizeof(int8_t), 1, outFile);
    }

    fclose(inFile);
    fclose(outFile);
}
int main() {
    // Define wiring for rotors and reflector
    int8_t rotor1_wiring[ALPHABET_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    int8_t rotor2_wiring[ALPHABET_SIZE] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    int8_t rotor3_wiring[ALPHABET_SIZE] = {5, 2, 8, 4, 1, 6, 9, 11, 14, 13, 12, 10, 3, 7, 0, 15};
    int8_t reflector_wiring[ALPHABET_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    int8_t panel_wiring[ALPHABET_SIZE] = {3, 5, 7, 2, 8, 9, 11, 1, 0, 6, 12, 15, 13, 4, 14, 10};

    // Initialize components
    Rotor rotor1, rotor2, rotor3;
    initRotor(&rotor1, rotor1_wiring, 1);
    initRotor(&rotor2, rotor2_wiring, 1);
    initRotor(&rotor3, rotor3_wiring, 1);
    Rotor rotors[] = {rotor1, rotor2, rotor3};

    Reflector reflector;
    initReflector(&reflector, reflector_wiring);

    CommutationPanel panel;
    initCommutationPanel(&panel, panel_wiring);

    Enigma enigma;
    initEnigma(&enigma, rotors, 3, reflector, panel);

    //Enigma 1
    Rotor rotor11, rotor21, rotor31;
    initRotor(&rotor11, rotor1_wiring, 1);
    initRotor(&rotor21, rotor2_wiring, 1);
    initRotor(&rotor31, rotor3_wiring, 1);
    Rotor rotors1[] = {rotor11, rotor21, rotor31};

    Reflector reflector1;
    initReflector(&reflector1, reflector_wiring);

    CommutationPanel panel1;
    initCommutationPanel(&panel1, panel_wiring);
    Enigma enigma1;
    initEnigma(&enigma1, rotors1, 3, reflector1, panel1);

    //Encrypt message
    char *message = "Hello world";
    char *encryptedMessage, *tmp_message;
    encryptedMessage = encryptMessage(&enigma, message);
    printf("Message %s\n", message);
    printf("Encrypted message: %s\n", encryptedMessage);
    tmp_message = encryptMessage(&enigma1, encryptedMessage);
    printf("Encrypted message: %s\n", tmp_message);

    // //Encrypt file 
    // encryptFile(&enigma, "1.txt", "tmp");
    // encryptFile(&enigma1, "tmp", "2.txt");

    // //Encrypt file rar
    // encryptFile(&enigma, "Nodepay.rar", "tmp");
    // encryptFile(&enigma1, "tmp", "Nodepay1.rar");

    return 0;
}
