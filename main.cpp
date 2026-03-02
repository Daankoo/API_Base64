/*
Абетка
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 + \
*/



#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
	
	// ofstream NewFile потрібно буде зробити для опції вибору створити чи використати уже існуючи, але це виведення

	ifstream ReadFile("D:\\Study\\University\\API\\Вase64_fail\\read.txt", ios::binary);
	
	if (!ReadFile) {
		cout << "File is not open";
		return -1; // Завершення програми з помилкою
	}

	unsigned char buffer[3];

	while (true) {	
		ReadFile.read((char*)buffer, 3);
		int ReadSize = ReadFile.gcount();
		
		if (ReadSize == 0) {
			break;
		}

		cout << "Saze: " << ReadSize << " -> ";
		for (int i = 0; i < ReadSize; i++) {
			cout << buffer[i];
		}
		cout << endl;
	}
	

	//while (ReadFile) {
	//	string s;
	//	ReadFile >> s;
	//	cout << s;
	//}

	return 0;
}

