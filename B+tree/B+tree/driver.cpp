#include <iostream>
#include <fstream>
#include "BTree.h"

using namespace std;

int main(int argc, char* argv[]) {

	BTree bt;
	char command;
	long long value, value2;

	//ifstream input(argv[1]);
	bool flag = true;

	for (int i = 0; i < 18; i++) {
		bt.insert(i + 1);
	}


	while (flag) {
		cin >> command;

		switch (command) {
		case 'i':
			cin >> value;
			bt.insert(value);
			break;
		case 's':
			cin >> value;
			bt.pointQuery(value);
			break;
		case 'r':
			cin >> value >> value2;
			bt.rangeQuery(value, value2);
			break;
		case 'd':
			cin >> value;
			bt.remove(value);
			break;
		case 'p':
			cin >> value;
			bt.printLeafNode(value);
			break;
		case 'q':
			flag = false;
			break;
		}
	}

	return 0;
}
