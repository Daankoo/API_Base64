#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const string BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const int LINE_LENGTH = 76;

// Записує символ у файл, автоматично розбиваючи на рядки по 76 символів
void writeChar(ofstream& file, char c, int& linePos) {
    if (linePos == LINE_LENGTH) {
        file << '\n';
        linePos = 0;
    }
    file << c;
    linePos++;
}

void encode(const string& inputName, const string& outputName, const string& comment) {
    ifstream readFile(inputName, ios::binary);
    if (!readFile) {
        cout << "Error: could not open file \"" << inputName << "\"\n";
        return;
    }

    ofstream writeFile(outputName);
    if (!writeFile) {
        cout << "Error: could not create file \"" << outputName << "\"\n";
        return;
    }

    // Записуємо коментар якщо є
    if (!comment.empty()) {
        // Символ '-' + текст, не більше 76 символів загалом
        string commentLine = "-" + comment;
        if (commentLine.length() > LINE_LENGTH) {
            commentLine = commentLine.substr(0, LINE_LENGTH);
        }
        writeFile << commentLine << '\n';
    }

    unsigned char bytes[3];
    unsigned char ind1, ind2, ind3, ind4;
    int linePos = 0;

    while (true) {
        bytes[0] = 0;
        bytes[1] = 0;
        bytes[2] = 0;

        readFile.read((char*)bytes, 3);
        int readSize = readFile.gcount();

        if (readSize == 0) break;

        if (readSize == 3) {
            ind1 = bytes[0] >> 2;
            ind2 = ((bytes[0] & 3) << 4) | (bytes[1] >> 4);
            ind3 = ((bytes[1] & 15) << 2) | (bytes[2] >> 6);
            ind4 = bytes[2] & 63;

            writeChar(writeFile, BASE64_CHARS[ind1], linePos);
            writeChar(writeFile, BASE64_CHARS[ind2], linePos);
            writeChar(writeFile, BASE64_CHARS[ind3], linePos);
            writeChar(writeFile, BASE64_CHARS[ind4], linePos);
        }
        else if (readSize == 2) {
            ind1 = bytes[0] >> 2;
            ind2 = ((bytes[0] & 3) << 4) | (bytes[1] >> 4);
            ind3 = ((bytes[1] & 15) << 2);

            writeChar(writeFile, BASE64_CHARS[ind1], linePos);
            writeChar(writeFile, BASE64_CHARS[ind2], linePos);
            writeChar(writeFile, BASE64_CHARS[ind3], linePos);
            writeChar(writeFile, '=', linePos);
        }
        else { // readSize == 1
            ind1 = bytes[0] >> 2;
            ind2 = ((bytes[0] & 3) << 4);

            writeChar(writeFile, BASE64_CHARS[ind1], linePos);
            writeChar(writeFile, BASE64_CHARS[ind2], linePos);
            writeChar(writeFile, '=', linePos);
            writeChar(writeFile, '=', linePos);
        }
    }

    // Завершальний перенос рядка якщо файл не порожній
    if (linePos > 0) {
        writeFile << '\n';
    }

    writeFile.close();
    readFile.close();

    cout << "Encoding complete. Result saved to \"" << outputName << "\"\n";
}

// Повертає індекс символу в Base64 алфавіті, або -1 якщо символ не знайдено
int base64Index(char c) {
    size_t pos = BASE64_CHARS.find(c);
    if (pos == string::npos) return -1;
    return (int)pos;
}

void decode(const string& inputName, const string& outputName) {
    ifstream readFile(inputName);
    if (!readFile) {
        cout << "Error: could not open file \"" << inputName << "\"\n";
        return;
    }

    ofstream writeFile(outputName, ios::binary);
    if (!writeFile) {
        cout << "Error: could not create file \"" << outputName << "\"\n";
        return;
    }

    string line;

    while (getline(readFile, line)) {
        // Пропускаємо рядки-коментарі (починаються з '-')
        if (!line.empty() && line[0] == '-') {
            continue;
        }

        // Обробляємо рядок по 4 символи
        int i = 0;
        while (i < (int)line.size()) {
            // Зчитуємо блок з 4 символів
            char c1 = line[i];
            char c2 = (i + 1 < (int)line.size()) ? line[i + 1] : '=';
            char c3 = (i + 2 < (int)line.size()) ? line[i + 2] : '=';
            char c4 = (i + 3 < (int)line.size()) ? line[i + 3] : '=';
            i += 4;

            int v1 = base64Index(c1);
            int v2 = base64Index(c2);
            int v3 = (c3 == '=') ? 0 : base64Index(c3);
            int v4 = (c4 == '=') ? 0 : base64Index(c4);

            // Перший байт завжди є
            unsigned char byte1 = (v1 << 2) | (v2 >> 4);
            writeFile.put(byte1);

            // Другий байт — тільки якщо c3 не паддінг
            if (c3 != '=') {
                unsigned char byte2 = ((v2 & 0x0F) << 4) | (v3 >> 2);
                writeFile.put(byte2);
            }

            // Третій байт — тільки якщо c4 не паддінг
            if (c4 != '=') {
                unsigned char byte3 = ((v3 & 0x03) << 6) | v4;
                writeFile.put(byte3);
            }
        }
    }

    writeFile.close();
    readFile.close();

    cout << "Decoding complete. Result saved to \"" << outputName << "\"\n";
}

int main() {
    int choice;

    cout << "=== Base64 Encoder/Decoder ===\n\n";
    cout << "1. Encode\n";
    cout << "2. Decode\n";
    cout << "Choose: ";
    cin >> choice;
    cin.ignore();

    if (choice == 1) {
        string inputName, outputName, comment;

        cout << "\n--- Encoder ---\n";
        cout << "Enter input file name: ";
        getline(cin, inputName);

        cout << "Enter output file name (press Enter to skip): ";
        getline(cin, outputName);

        if (outputName.empty()) {
            outputName = inputName + ".base64";
            cout << "Output file: " << outputName << "\n";
        }

        cout << "Enter a comment for the file (press Enter to skip): ";
        getline(cin, comment);

        encode(inputName, outputName, comment);
    }
    else if (choice == 2) {
        string inputName, outputName;

        cout << "\n--- Decoder ---\n";
        cout << "Enter encoded file name: ";
        getline(cin, inputName);

        // Пропонуємо ім'я: якщо файл має розширення .base64 — прибираємо його
        string suggestedName = inputName;
        if (inputName.size() > 7 && inputName.substr(inputName.size() - 7) == ".base64") {
            suggestedName = inputName.substr(0, inputName.size() - 7);
            cout << "Suggested output file name: \"" << suggestedName << "\"\n";
        }

        cout << "Enter output file name: ";
        getline(cin, outputName);

        if (outputName.empty()) {
            outputName = suggestedName;
            cout << "Output file: " << outputName << "\n";
        }

        decode(inputName, outputName);
    }
    else {
        cout << "Invalid choice.\n";
    }

    return 0;
}