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

int main() {
    string inputName, outputName, comment;

    cout << "=== Base64 Encoder ===\n\n";

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

    return 0;
}