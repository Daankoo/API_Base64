#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const string BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const int LINE_LENGTH = 76;

// Виводить символ у файл з автоматичним переносом рядка кожні 76 символів
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

    if (!comment.empty()) {
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
            // Два байти → три Base64-символи + один паддінг '='
            ind1 = bytes[0] >> 2;
            ind2 = ((bytes[0] & 3) << 4) | (bytes[1] >> 4);
            ind3 = ((bytes[1] & 15) << 2);

            writeChar(writeFile, BASE64_CHARS[ind1], linePos);
            writeChar(writeFile, BASE64_CHARS[ind2], linePos);
            writeChar(writeFile, BASE64_CHARS[ind3], linePos);
            writeChar(writeFile, '=', linePos);
        }
        else {
            // Один байт → два Base64-символи + два паддінги '=='
            ind1 = bytes[0] >> 2;
            ind2 = ((bytes[0] & 3) << 4);

            writeChar(writeFile, BASE64_CHARS[ind1], linePos);
            writeChar(writeFile, BASE64_CHARS[ind2], linePos);
            writeChar(writeFile, '=', linePos);
            writeChar(writeFile, '=', linePos);
        }
    }

    if (linePos > 0) {
        writeFile << '\n';
    }

    writeFile.close();
    readFile.close();

    cout << "Encoding complete. Result saved to \"" << outputName << "\"\n";
}

// Повертає індекс символу в Base64-алфавіті, або -1 якщо символ не входить до алфавіту
int base64Index(char c) {
    size_t pos = BASE64_CHARS.find(c);
    if (pos == string::npos) return -1;
    return (int)pos;
}

// Останній рядок визначається довжиною меншою за 76 або наявністю символу паддінгу '='
bool isLastLine(const string& line) {
    return (int)line.size() < LINE_LENGTH || line.find('=') != string::npos;
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
    int lineNum = 0;
    bool messageEnded = false;

    while (getline(readFile, line)) {
        lineNum++;

        // Рядки-коментарі починаються з '-' і пропускаються декодером
        if (!line.empty() && line[0] == '-') {
            continue;
        }

        // Будь-який не-коментарний рядок після кінця повідомлення є некоректним
        if (messageEnded) {
            cout << "Warning: data found after end of message (line " << lineNum << ")\n";
            break;
        }

        // Усі рядки крім останнього мають бути рівно 76 символів;
        // останній може бути коротшим або містити паддінг
        if ((int)line.size() != LINE_LENGTH) {
            if (line.size() % 4 != 0) {
                cout << "Line " << lineNum << ": Invalid line length (" << line.size() << ")\n";
                writeFile.close();
                readFile.close();
                return;
            }
        }

        // Посимвольна валідація рядка
        bool paddingStarted = false;
        for (int i = 0; i < (int)line.size(); i++) {
            char c = line[i];

            // Символ '-' допустимий тільки на першій позиції рядка (коментар)
            if (c == '-') {
                cout << "Line " << lineNum << ", symbol " << (i + 1) << ": Invalid input character (`-')\n";
                writeFile.close();
                readFile.close();
                return;
            }

            // Паддінг '=' допустимий тільки в останніх двох позиціях останнього блоку рядка
            if (c == '=') {
                int posInBlock = i % 4;
                bool isNearEnd = (i >= (int)line.size() - 2);

                if (!isNearEnd || posInBlock < 2) {
                    cout << "Line " << lineNum << ", symbol " << (i + 1) << ": Invalid padding usage\n";
                    writeFile.close();
                    readFile.close();
                    return;
                }
                paddingStarted = true;
                continue;
            }

            // Після символу паддінгу не може йти жоден інший символ
            if (paddingStarted) {
                cout << "Line " << lineNum << ", symbol " << (i + 1) << ": Invalid padding usage\n";
                writeFile.close();
                readFile.close();
                return;
            }

            if (base64Index(c) == -1) {
                cout << "Line " << lineNum << ", symbol " << (i + 1) << ": Invalid input character (`" << c << "')\n";
                writeFile.close();
                readFile.close();
                return;
            }
        }

        // Декодування блоками по 4 символи → до 3 байтів
        for (int i = 0; i < (int)line.size(); i += 4) {
            char c1 = line[i];
            char c2 = line[i + 1];
            char c3 = line[i + 2];
            char c4 = line[i + 3];

            int v1 = base64Index(c1);
            int v2 = base64Index(c2);
            int v3 = (c3 == '=') ? 0 : base64Index(c3);
            int v4 = (c4 == '=') ? 0 : base64Index(c4);

            writeFile.put((unsigned char)((v1 << 2) | (v2 >> 4)));

            if (c3 != '=')
                writeFile.put((unsigned char)(((v2 & 0x0F) << 4) | (v3 >> 2)));

            if (c4 != '=')
                writeFile.put((unsigned char)(((v3 & 0x03) << 6) | v4));
        }

        if (isLastLine(line)) {
            messageEnded = true;
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

        // Якщо файл має розширення .base64 — пропонується ім'я без нього
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