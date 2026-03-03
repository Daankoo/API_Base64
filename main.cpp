/*
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 + \
*/

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

using namespace std;

int main() {
	
	string Base64_char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	//string NameFile;
	//cout << "Enter name file";
	//cin >> NameFile;

	ifstream ReadFile("D:\\Study\\University\\API\\Вase64_fail\\read.txt", ios::binary);
	
	if (!ReadFile) {
		cout << "File is not open";
		return -1; // Завершення програми з помилкою
	}

	ofstream WriteFile("D:\\Study\\University\\API\\Вase64_fail\\write.txt");

	if (!WriteFile) {
		cout << "File is not found";
		return -1;
	}

	unsigned char byte[3];
	unsigned char ind1, ind2, ind3, ind4;

	while (true) {	

		byte[0] = 0;
		byte[1] = 0;
		byte[2] = 0;

		ReadFile.read((char*)byte, 3);
		int ReadSize = ReadFile.gcount();
		
		if (ReadSize == 0) {
			break;
		}

		if (ReadSize == 3) {
			ind1 = byte[0] >> 2;
			ind2 = ((byte[0] & 3) << 4) ^ (byte[1] >> 4);
			ind3 = ((byte[1] & 15) << 2) ^ (byte[2] >> 6);
			ind4 = byte[2] & 63;

			WriteFile << Base64_char[ind1] << Base64_char[ind2] << Base64_char[ind3] << Base64_char[ind4];
		}
		else if (ReadSize == 2) {
			ind1 = byte[0] >> 2;
			ind2 = ((byte[0] & 3) << 4) ^ (byte[1] >> 4);
			ind3 = ((byte[1] & 15) << 2);
			
			WriteFile << Base64_char[ind1] << Base64_char[ind2] << Base64_char[ind3] << "=";
		}
		else {
			ind1 = byte[0] >> 2;
			ind2 = ((byte[0] & 3) << 4);

			WriteFile << Base64_char[ind1] << Base64_char[ind2] << "==";
		}
	}

	WriteFile.close();

	return 0;
}

