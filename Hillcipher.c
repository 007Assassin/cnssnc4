#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MOD 26
int SIZE;

// Proper mod function to handle negatives
int mod26(int x) {
    return (x % MOD + MOD) % MOD;
}

// Calculate determinant (recursive)
int determinant(int m[10][10], int n) {
    if (n == 1) return mod26(m[0][0]);

    int det = 0, temp[10][10];
    int sign = 1;

    for (int f = 0; f < n; f++) {
        int subi = 0;
        for (int i = 1; i < n; i++) {
            int subj = 0;
            for (int j = 0; j < n; j++) {
                if (j == f) continue;
                temp[subi][subj++] = m[i][j];
            }
            subi++;
        }
        det += sign * m[0][f] * determinant(temp, n - 1);
        sign = -sign;
    }
    return mod26(det);
}

// Find modular inverse of determinant
int modInverse(int a) {
    for (int x = 1; x < MOD; x++) {
        if (mod26(a * x) == 1) return x;
    }
    return -1;
}

// Calculate adjoint (cofactor transpose)
void cofactorTranspose(int m[10][10], int adj[10][10], int n) {
    if (n == 1) {
        adj[0][0] = 1;
        return;
    }

    int temp[10][10], sign;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int subi = 0, subj = 0;
            for (int row = 0; row < n; row++) {
                for (int col = 0; col < n; col++) {
                    if (row != i && col != j) {
                        temp[subi][subj++] = m[row][col];
                        if (subj == n - 1) {
                            subj = 0;
                            subi++;
                        }
                    }
                }
            }
            sign = ((i + j) % 2 == 0) ? 1 : -1;
            adj[j][i] = mod26(sign * determinant(temp, n - 1));
        }
    }
}

// Matrix inverse for decryption
int matrixInverse(int key[10][10], int inv[10][10]) {
    int det = determinant(key, SIZE);
    int detInv = modInverse(det);
    if (detInv == -1) {
        printf("Error: Key matrix is not invertible.\n");
        return 0;
    }

    int adj[10][10];
    cofactorTranspose(key, adj, SIZE);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            inv[i][j] = mod26(adj[i][j] * detInv);
        }
    }

    return 1;
}

// Multiply key * message vector
void matrixMultiply(int result[10][1], int matrix[10][10], int vector[10][1]) {
    for (int i = 0; i < SIZE; i++) {
        result[i][0] = 0;
        for (int j = 0; j < SIZE; j++) {
            result[i][0] += matrix[i][j] * vector[j][0];
        }
        result[i][0] = mod26(result[i][0]);
    }
}

// Preprocess message: uppercase + remove non-alpha + pad
void processPlaintext(char* raw, char* clean) {
    int j = 0;
    for (int i = 0; raw[i]; i++) {
        if (isalpha(raw[i])) {
            clean[j++] = toupper(raw[i]);
        }
    }
    while (j % SIZE != 0) {
        clean[j++] = 'X';  // Padding
    }
    clean[j] = '\0';
}

// Encrypt using Hill Cipher
void HillCipherEncrypt(char message[], int keyMatrix[10][10]) {
    char clean[100], CipherText[100];
    processPlaintext(message, clean);
    int len = strlen(clean);
    int messageVector[10][1], cipherMatrix[10][1];

    for (int i = 0; i < len; i += SIZE) {
        for (int j = 0; j < SIZE; j++) {
            messageVector[j][0] = (clean[i + j] - 'A') % 26;
        }

        matrixMultiply(cipherMatrix, keyMatrix, messageVector);

        for (int j = 0; j < SIZE; j++) {
            CipherText[i + j] = (cipherMatrix[j][0] + 'A');
        }
    }
    CipherText[len] = '\0';
    printf("\nCiphertext: %s\n", CipherText);
}

// Decrypt using Hill Cipher
void HillCipherDecrypt(char cipher[], int keyMatrix[10][10]) {
    int invKey[10][10];
    if (!matrixInverse(keyMatrix, invKey)) {
        return;
    }

    int len = strlen(cipher);
    int messageVector[10][1], plainMatrix[10][1];
    char PlainText[100];

    for (int i = 0; i < len; i += SIZE) {
        for (int j = 0; j < SIZE; j++) {
            messageVector[j][0] = (cipher[i + j] - 'A') % 26;
        }

        matrixMultiply(plainMatrix, invKey, messageVector);

        for (int j = 0; j < SIZE; j++) {
            PlainText[i + j] = (plainMatrix[j][0] + 'A');
        }
    }

    PlainText[len] = '\0';
    printf("Decrypted Text: %s\n", PlainText);
}

// Main Function
int main() {
    char message[100], cipher[100];
    int keyMatrix[10][10];

    printf("Enter the order of the matrix (e.g., 2 or 3): ");
    scanf("%d", &SIZE);

    if (SIZE < 2 || SIZE > 10) {
        printf("Error: Matrix size must be between 2 and 10.\n");
        return 0;
    }

    getchar();  // Clear buffer

    printf("Enter the plaintext: ");
    fgets(message, sizeof(message), stdin);
    size_t len = strlen(message);
    if (len > 0 && message[len - 1] == '\n') {
        message[len - 1] = '\0';
    }

    printf("Enter the %dx%d key matrix (values 0-25):\n", SIZE, SIZE);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("Enter key[%d][%d]: ", i, j);
            scanf("%d", &keyMatrix[i][j]);
            keyMatrix[i][j] = mod26(keyMatrix[i][j]);
        }
    }

    HillCipherEncrypt(message, keyMatrix);

    printf("\nEnter ciphertext to decrypt: ");
    scanf("%s", cipher);

    HillCipherDecrypt(cipher, keyMatrix);

    return 0;
}
