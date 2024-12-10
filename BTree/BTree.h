#pragma once

#include <iostream>
#include <vector>
#include <fstream>

constexpr int d = 8;
constexpr int maxRecord = 2 * d;
constexpr int maxChild = 2 * d + 1;

int h = 0;
int nodes = 0;

class BTreeNode {
public:
	int size;
	int parentIndex;
	int keys[maxRecord];
	int recordIndex[maxRecord];
	int childIndexes[maxChild];

	BTreeNode(int parentIndex = -1):
		parentIndex(parentIndex)
	{
		size = 0;
	}

	void shiftRightFrom(int startIndex) {
		if (startIndex < 0 || startIndex >= maxRecord) {
			std::cout << "Invalid start index for shifting.";
			return;
		}

		// Shift keys and recordIndex to the right
		for (int i = size; i > startIndex; --i) {
			keys[i] = keys[i - 1];
			recordIndex[i] = recordIndex[i - 1];
		}

		// Shift childIndexes to the right
		for (int i = size + 1; i > startIndex + 1; --i) {
			childIndexes[i] = childIndexes[i - 1];
		}

		// Update size
		size++;
	}

	void shiftLeftFrom(int startIndex) {
		if (startIndex < 0 || startIndex >= maxRecord) {
			std::cout << "Invalid start index for shifting.";
			return;
		}

		// Shift keys and recordIndex to the left
		for (int i = startIndex; i < size - 1; ++i) {
			keys[i] = keys[i + 1];
			recordIndex[i] = recordIndex[i + 1];
		}

		// Shift childIndexes to the left
		for (int i = startIndex + 1; i < size; ++i) {
			childIndexes[i] = childIndexes[i + 1];
		}

		// Update size
		size--;
	}

	int findInsertPosition(int key) {
		int index = 0;
		while (index < size && keys[index] < key) {
			index++;
		}
		return index;
	}

	void insert(int key, int recordOffset, int leftPointer = -1)
	{
		int index = findInsertPosition(key);

		shiftRightFrom(index);

		keys[index] = key;
		recordIndex[index] = recordOffset;
		childIndexes[index + 1] = leftPointer;
	}
};

constexpr int sizeOfNodeInFile = sizeof(BTreeNode);

template <class T> class BTree {
private:
	const char* bTreeFile = "BTree.dat";
	const char* recordFile = "Record.dat";

	int rootIndex;

	BTreeNode currentPage;
	BTreeNode tempPage1, tempPage2;
	int currentPageIndex;

	BTreeNode readNode(int index)
	{
		readWriteCounter++;
		std::ifstream file(bTreeFile, std::ios::binary);
		if (!file.is_open())
		{
			std::cout << "Unable to open BTree file.";
			return BTreeNode(-1);
		}

		BTreeNode node(-1);
		file.seekg(index * sizeOfNodeInFile, std::ios::beg);
		file.read(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
		file.close();

		return node;
	}

	T readRecord(int index)
	{
		std::ifstream file(recordFile, std::ios::binary);
		if (!file.is_open())
		{
			std::cout<<"Unable to open Record file.";
			return T();
		}

		T record;
		file.seekg(index * sizeof(T), std::ios::beg);
		file.read(reinterpret_cast<char*>(&record), sizeof(T));
		file.close();

		return record;
	}

	int writeNode(BTreeNode node)
	{
		readWriteCounter++;
		std::ofstream file(bTreeFile, std::ios::binary | std::ios::app);
		if (!file.is_open())
		{
			std::cout << "Unable to open BTree file.";
			return -1;
		}

		file.seekp(0, std::ios::end);
		int index = file.tellp() / sizeof(BTreeNode);
		file.write(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
		file.close();

		return index;
	}

	int writeRecord(T record)
	{
		std::ofstream file(recordFile, std::ios::binary | std::ios::app);
		if (!file.is_open())
		{
			std::cout << "Unable to open Record file.";
			return -1;
		}

		file.seekp(0, std::ios::end);
		int index = file.tellp() / sizeof(T);
		file.write(reinterpret_cast<const char*>(&record), sizeof(T));
		file.close();

		return index;
	}

	void updateNode(int index, BTreeNode node)
	{
		readWriteCounter++;
		std::fstream file(bTreeFile, std::ios::in | std::ios::out | std::ios::binary);
		if (!file.is_open())
		{
			std::cout << "Unable to open BTree file.";
			return;
		}

		file.seekp(index * sizeOfNodeInFile, std::ios::beg);
		file.write(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
		file.close();
	}

	void updateRecord(int index, T record)
	{
		std::fstream file(recordFile, std::ios::in | std::ios::out | std::ios::binary);
		if (!file.is_open())
		{
			std::cout << "Unable to open Record file.";
			return;
		}

		file.seekp(index * sizeof(T), std::ios::beg);
		file.write(reinterpret_cast<const char*>(&record), sizeof(T));
		file.close();
	}

	bool searchStep(int key)
	{
		int i = 0;
		while(i < currentPage.size && key > currentPage.keys[i])
		{
			i++;
		}

		if(i < currentPage.size && key == currentPage.keys[i])
		{
			return true;
		}
		else if(currentPage.childIndexes[i] != -1)
		{
			currentPageIndex = currentPage.childIndexes[i];
			currentPage = readNode(currentPage.childIndexes[i]);
			return searchStep(key);
		}
		else
		{
			return false;
		}
	}

	void insertIntoCurrentNode(int key, int recordOffset)
	{
		int index = currentPage.findInsertPosition(key);

		currentPage.shiftRightFrom(index);

		currentPage.keys[index] = key;
		currentPage.recordIndex[index] = recordOffset;
		currentPage.childIndexes[index + 1] = -1;

		updateNode(currentPageIndex, currentPage);
	}

	bool compensation(int key, int recordOffset)
	{
		if (currentPage.parentIndex != -1)
		{
			tempPage1 = readNode(currentPage.parentIndex);

			// find neighbours and check if they have space
			int index = tempPage1.findInsertPosition(key);

			// checking right neighbour
			if(index < 2*d+1 && tempPage1.childIndexes[index] >= 0)
			{
				int neigbourIndex = tempPage1.childIndexes[index - 1];
				tempPage2 = readNode(tempPage1.childIndexes[index]);
				if(tempPage2.size < 2 * d)
				{
					// right neighbour has space
					// move the key from parent to the current node
					int parentKey = tempPage1.keys[index];
					int parentRecordIndex = tempPage1.recordIndex[index];
					int parentChildIndex = tempPage1.childIndexes[index];

					//inserting into neighbour
					tempPage2.insert(parentKey, parentRecordIndex);

					//inserting into parent
					tempPage1.keys[index + 1] = currentPage.keys[2*d -1];
					tempPage1.recordIndex[index + 1] = currentPage.recordIndex[2*d-1];

					// inserting into currentNode
					index = currentPage.findInsertPosition(key);
					currentPage.shiftRightFrom(index);
					currentPage.keys[index] = key;
					currentPage.recordIndex[index] = recordOffset;

					updateNode(currentPageIndex, currentPage);
					updateNode(currentPage.parentIndex, tempPage1);
					updateNode(neigbourIndex, tempPage2);
					return true;
				}
			}
			// checking left neighbour
			if(index - 1 >= 0  && tempPage1.childIndexes[index - 1] >=0)
			{
				int neigbourIndex = tempPage1.childIndexes[index - 1];
				tempPage2 = readNode(tempPage1.childIndexes[index - 1]);
				if(tempPage2.size < 2 * d)
				{
					// left neighbour has space
					// move the key from parent to the current node
					int parentKey = tempPage1.keys[index - 1];
					int parentRecordIndex = tempPage1.recordIndex[index - 1];
					int parentChildIndex = tempPage1.childIndexes[index - 1];

					//inserting into neighbour
					tempPage2.insert(parentKey, parentRecordIndex);

					//inserting into parent
					tempPage1.keys[index - 1] = currentPage.keys[0];
					tempPage1.recordIndex[index - 1] = currentPage.recordIndex[0];

					// inserting into currentNode
					
					currentPage.shiftLeftFrom(0);
					index = currentPage.findInsertPosition(key);
					currentPage.keys[index] = key;
					currentPage.recordIndex[index] = recordOffset;
					currentPage.size++;

					updateNode(currentPageIndex, currentPage);
					updateNode(currentPage.parentIndex, tempPage1);
					updateNode(neigbourIndex, tempPage2);
					return true;
				}
			}

		}
		return false;
	}

	BTreeNode child;
	BTreeNode newParent;
	BTreeNode parent;

	BTreeNode newNeighbour;

	void splitNode(int key, int recordOffset, int linker = -1)
	{
		// if parent
		if(currentPage.parentIndex == -1)
		{
			h++;
			// create new parent
			newParent = BTreeNode(-1);

			// finding the middle element
			int index = currentPage.findInsertPosition(key);
			int bias = index < d ? -1 : 0;

			// add key and index to parent
			if(index == d)
			{
				newParent.keys[0] = key;
				newParent.recordIndex[0] = recordOffset;
				newParent.size++;
				rootIndex = writeNode(newParent);
			}
			else
			{
				newParent.keys[0] = currentPage.keys[d + bias];
				newParent.recordIndex[0] = currentPage.recordIndex[d + bias];
				newParent.size++;
				rootIndex = writeNode(newParent);
			}

			newNeighbour = BTreeNode(rootIndex);
			currentPage.parentIndex = rootIndex;

			if(index < 2)
			{
				// make currentPage smaller
				currentPage.size = d - 1;
				currentPage.parentIndex = rootIndex;

				// create new neighbour
				newNeighbour.size = d;
				std::copy(currentPage.keys + d, currentPage.keys + maxRecord, newNeighbour.keys);
				std::copy(currentPage.recordIndex + d, currentPage.recordIndex + maxRecord, newNeighbour.recordIndex);
				std::copy(currentPage.childIndexes + d, currentPage.childIndexes + maxChild, newNeighbour.childIndexes);

				// insert into currentPage
				insertIntoCurrentNode(key, recordOffset);
			}
			else if(index > 2)
			{
				// make currentPage smaller
				currentPage.size = d;
				currentPage.parentIndex = rootIndex;
				updateNode(currentPageIndex, currentPage);

				// create new neighbour
				newNeighbour.size = d - 1;
				std::copy(currentPage.keys + d + 1, currentPage.keys + maxRecord, newNeighbour.keys);
				std::copy(currentPage.recordIndex + d + 1, currentPage.recordIndex + maxRecord, newNeighbour.recordIndex);
				std::copy(currentPage.childIndexes + d + 1, currentPage.childIndexes + maxChild, newNeighbour.childIndexes);
				newNeighbour.insert(key, recordOffset,linker);
			}
			else
			{
				currentPage.size = d;
				currentPage.parentIndex = rootIndex;
				updateNode(currentPageIndex, currentPage);

				// create new neighbour
				newNeighbour.size = d;
				std::copy(currentPage.keys + d, currentPage.keys + maxRecord, newNeighbour.keys);
				std::copy(currentPage.recordIndex + d, currentPage.recordIndex + maxRecord, newNeighbour.recordIndex);
				std::copy(currentPage.childIndexes + d, currentPage.childIndexes + maxChild, newNeighbour.childIndexes);
			}

			int newNeighbourIndex = writeNode(newNeighbour);
			if (linker != -1)
			{
				for (int i = 0; i < d + 1; i++)
				{
					child = readNode(newNeighbour.childIndexes[i]);
					child.parentIndex = newNeighbourIndex;
					updateNode(newNeighbour.childIndexes[i], child);
				}
			}

			//setting children
			newParent.childIndexes[0] = currentPageIndex;
			newParent.childIndexes[1] = newNeighbourIndex;

			updateNode(rootIndex, newParent);
			return;
		}
		else
		{
			int nextKey;
			int nextRecordOffset;
			// finding the middle element
			int index = currentPage.findInsertPosition(key);
			int bias = index < d ? -1 : 0;

			// add key and index to parent
			if (index == d)
			{
				nextKey = key;
				nextRecordOffset = recordOffset;
			}
			else
			{
				nextKey = currentPage.keys[d + bias];
				nextRecordOffset = currentPage.recordIndex[d + bias];
			}
			int parentIndex = currentPage.parentIndex;
			parent = readNode(parentIndex);
			int currentIndexInParent = parent.findInsertPosition(nextKey);
			newNeighbour = BTreeNode(parentIndex);
			if (index < 2)
			{
				// make currentPage smaller
				currentPage.size = d - 1;
				//currentPage.parentIndex = rootIndex;

				// create new neighbour
				newNeighbour.size = d;
				std::copy(currentPage.keys + d, currentPage.keys + maxRecord, newNeighbour.keys);
				std::copy(currentPage.recordIndex + d, currentPage.recordIndex + maxRecord, newNeighbour.recordIndex);
				std::copy(currentPage.childIndexes + d, currentPage.childIndexes + maxChild, newNeighbour.childIndexes);
				newNeighbour.childIndexes[d] = linker;

				// insert into currentPage
				insertIntoCurrentNode(key, recordOffset);
			}
			else if (index > 2)
			{
				// make currentPage smaller
				currentPage.size = d;
				//currentPage.parentIndex = rootIndex;
				updateNode(currentPageIndex, currentPage);

				// create new neighbour
				newNeighbour.size = d - 1;
				std::copy(currentPage.keys + d + 1, currentPage.keys + maxRecord, newNeighbour.keys);
				std::copy(currentPage.recordIndex + d + 1, currentPage.recordIndex + maxRecord, newNeighbour.recordIndex);
				std::copy(currentPage.childIndexes + d + 1, currentPage.childIndexes + maxChild, newNeighbour.childIndexes);
				newNeighbour.insert(key, recordOffset);
				newNeighbour.childIndexes[d] = linker;
			}
			else
			{
				currentPage.size = d;
				//currentPage.parentIndex = rootIndex;
				updateNode(currentPageIndex, currentPage);

				// create new neighbour
				newNeighbour.size = d;
				std::copy(currentPage.keys + d, currentPage.keys + maxRecord, newNeighbour.keys);
				std::copy(currentPage.recordIndex + d, currentPage.recordIndex + maxRecord, newNeighbour.recordIndex);
				std::copy(currentPage.childIndexes + d, currentPage.childIndexes + maxChild, newNeighbour.childIndexes);
				newNeighbour.childIndexes[d] = linker;
			}
			int newNeighbourIndex = writeNode(newNeighbour);
			if(linker!=-1)
			{
				for (int i = 0; i < d + 1; i++)
				{
					child = readNode(newNeighbour.childIndexes[i]);
					child.parentIndex = newNeighbourIndex;
					updateNode(newNeighbour.childIndexes[i], child);
				}
			}
			


			if(parent.size < 2*d)
			{
				// there is space in the parent
				parent.insert(nextKey, nextRecordOffset);
				parent.childIndexes[currentIndexInParent + 1] = newNeighbourIndex;
				updateNode(parentIndex, parent);
				return;
			}
			else
			{
				// there is no space in the parent
				currentPage = parent;
				currentPageIndex = parentIndex;
				splitNode(nextKey, nextRecordOffset, newNeighbourIndex);
				return;
			}
		}
		
	
	}

public:
	int readWriteCounter = 0;

	BTree()
	{
		h = 1;
		nodes = 0;
		resetDB();
		rootIndex = 0;
		currentPageIndex = 0;
		BTreeNode root(-1);
		writeNode(root);
		
	}

	int insert(int key, T record)
	{
		readWriteCounter = 0;
		if(currentPageIndex == rootIndex)
		{
			currentPage = readNode(rootIndex);
		}
		else
		{
			currentPage = readNode(currentPageIndex);
		}
		if(!search(key))
		{
			int recordOffset = writeRecord(record);
			if(currentPage.size < 2 * d)
			{
				// there is space in the current node
				insertIntoCurrentNode(key, recordOffset);
				return 0;
			}
			else
			{
				// there is no space in the current node
				
				// if compensation -> our neighbour have space
				if(compensation(key, recordOffset))
				{
					return 0;
				}
				// if the compensation is not possible

				splitNode(key, recordOffset);
				return 0;
			}

		}
		else
		{
			// Key already exists
			return -1;
		}
	}

	void remove(int key)
	{
		if(!search(key))
		{
			std::cout << "Key not found\n";
			return;
		}
		throw "Not implemented";
	}

	bool search(int key)
	{
		currentPage = readNode(rootIndex);
		if(currentPage.size == 0)
		{
			return false;
		}
		return searchStep(key);
	}

	void print()
	{
		readWriteCounter = 0;
		printInOrder(rootIndex);
	}

	BTreeNode printerNode;

private:
	void printInOrder(int index) {
		if (index < 0) return;

		printerNode = readNode(index);

		for (int i = 0; i < printerNode.size; i++) {
			if (printerNode.childIndexes[i] >= 0)
			{
				printInOrder(printerNode.childIndexes[i]);
				printerNode = readNode(printerNode.parentIndex);
			}
			printf("%11d ", printerNode.keys[i]);
			auto record = readRecord(printerNode.recordIndex[i]);
			record.print();
			//printf("\n");
		}
		if (printerNode.childIndexes[printerNode.size] >= 0)
		{
			printInOrder(printerNode.childIndexes[printerNode.size]);
			printerNode = readNode(printerNode.parentIndex);
		}
	}

public:
	void resetDB()
	{
		// Usuniêcie zawartoœci pliku BTree.dat
		std::ofstream bFile(bTreeFile, std::fstream::trunc);
		if (!bFile.is_open()) {
			std::cout << "Unable to open BTree file.";
			return;
		}
		bFile.close();

		// Usuniêcie zawartoœci pliku Record.dat
		std::ofstream record_file(recordFile, std::ios::out);
		if (!record_file.is_open()) {
			std::cout << "Unable to open Record file.";
			return;
		}
		record_file.close();

		// Resetowanie indeksów i tworzenie nowego korzenia
		rootIndex = 0;
		currentPageIndex = 0;
		BTreeNode root(-1);
		writeNode(root);
	}
};
