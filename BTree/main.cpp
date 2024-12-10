#include <iostream>
#include "BTree.h"
#include "Student.h"

class IntegerBTree
{
public:
	int value;
	void print()
	{
		std::cout << value << std::endl;
	
	}
};

int countNodesInFile(const char* filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cout << "Unable to open file: " << filename << std::endl;
		return -1;
	}

	std::streamsize fileSize = file.tellg();
	file.close();

	return fileSize / sizeOfNodeInFile;
}

void printMenu() {
	std::cout << "Instrukcje:\n";
	std::cout << "i - insert\n";
	std::cout << "s - search\n";
	std::cout << "p - print\n";
	std::cout << "q - quit\n";
}

int main()
{
	setlocale(LC_ALL, "");


	BTree<IntegerBTree> b_tree;
	IntegerBTree record;
	record.value = 10;

	/*//b_tree.insert(1, record);
	b_tree.insert(20, record);
	b_tree.insert(50, record);
	b_tree.insert(30, record);

	b_tree.insert(35, record);

	b_tree.insert(36, record);
	b_tree.insert(37, record);
	b_tree.insert(21, record);

	b_tree.insert(38, record);

	b_tree.insert(39, record);
	b_tree.insert(41, record);

	b_tree.insert(42, record);
	b_tree.insert(43, record);

	b_tree.print();
	printf("\n\n\n");

	b_tree.insert(44, record);*/

	for(int i = 0;i< 1000;i++)
	{
		b_tree.insert(i, record);
		//b_tree.print();
	}
	std::cout << " number of read and write opertions: " << b_tree.readWriteCounter;


	b_tree.print();
	std::cout << " number of read and write opertions: " << b_tree.readWriteCounter;
	std::cout << "Number of nodes in file: " << countNodesInFile("BTree.dat") << std::endl;

	/*BTree<Student> b_tree;
	char command;
	char name[30];
	short kol1, kol2, kol3;
	int key;

	printMenu();

	while (true) {
		std::cout << "Podaj instrukcję: ";
		std::cin >> command;

		switch (command) {
		case 'i':
			std::cout << "Podaj klucz: ";
			std::cin >> key;
			std::cout << "Podaj imię: ";
			std::cin >> name;
			std::cout << "Podaj kol1: ";
			std::cin >> kol1;
			std::cout << "Podaj kol2: ";
			std::cin >> kol2;
			std::cout << "Podaj kol3: ";
			std::cin >> kol3;
			{
				Student student(name, kol1, kol2, kol3);
				if (student.isValid()) {
					b_tree.insert(key, student);
				}
				else {
					std::cout << "Nieprawidłowe dane studenta.\n";
				}
			}
			break;
		case 's':
			std::cout << "Podaj klucz do wyszukania: ";
			std::cin >> key;
			if (b_tree.search(key)) {
				std::cout << "Klucz " << key << " znaleziony.\n";
			}
			else {
				std::cout << "Klucz " << key << " nie znaleziony.\n";
			}
			break;
		case 'p':
			b_tree.print();
			break;
		case 'q':
			return 0;
		default:
			std::cout << "Nieznana instrukcja.\n";
			printMenu();
			break;
		}
	}*/

	return 0;
}
