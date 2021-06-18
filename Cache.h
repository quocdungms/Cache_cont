#ifndef CACHE_H
#define CACHE_H

#include "main.h"




template <class T>
class Aqueue
{
	friend class ReplacementPolicy;
protected:

	T** arr;
	int count;
	int capacity;
public:
	Aqueue(int capacity)
	{
		this->capacity = capacity;
		this->count = 0;
		arr = new T * [this->capacity];
	}
	~Aqueue()
	{
		for (int i = 0; i < this->count; i++)
		{
			delete arr[i];
		}
		delete[] arr;
		this->count = 0;
	}

	void clear();
	void print();
	void print_elements();
	bool full();
	bool empty();
	void push_back(T* e);
	void pop();
	void pop_back();
	void move_front(int index);
	void move_back(int index);
	void swap(int a, int b);
	T* replace(int index, T* e);
	T* front();
	T* back();
};










class ReplacementPolicy {
	
protected:
	int count = 0;
	Aqueue<Elem>* myQueue = nullptr;
	
public:
	virtual int insert(Elem* e) = 0;
	virtual void access(int idx) = 0;
	virtual int remove() = 0;
	virtual Elem* getDeathElement() = 0;
	virtual void print_debugging()
	{
		printf("-------------------------------------\n");
		printf("| Debugging ReplacementPolicy\n");
		printf("-------------------------------------\n");
		printf("| Queue | MAX = %d | size = %d\n", MAXSIZE, this->count);
		printf("-------------------------------------\n");
		printf("| Index | Addr | Data | Sync|\n");
		if (this->isEmpty()) { return; }
		else
		{
			for (int i = 0; i < count; i++)
			{
				printf("| %5d | %4d | %4s | %4d |\n",
					i, this->getElement(i)->addr, this->getElement(i)->data->getValue().c_str(),
					this->getElement(i)->sync);
			}
		}
		printf("-------------------------------------\n");
	}

	bool isFull() { return count == MAXSIZE; }
	bool isEmpty() { return count == 0; }
	int size() { return this->count; }
	void replace(int idx, Data* cont)
	{
		if (idx < 0 || idx >= count) {
			printf("Invalid replace position!\n");
		}
		else
		{
			this->myQueue->arr[idx]->data = cont;
			this->myQueue->arr[idx]->sync = false;
			this->access(idx);
		}
	}
	Elem* getElement(int idx)
	{
		if (idx != -1)
		{
			return myQueue->arr[idx];
		}
		return nullptr;
	}
	
	void ensure_index(SearchEngine* s);
	void print()
	{
		if (myQueue->empty()) { return; }
		else { myQueue->print_elements(); }
	}
};


//=================================================================================
class FIFO : public ReplacementPolicy {
public:
	FIFO() {
		this->count = 0;
		this->myQueue = new Aqueue<Elem>(MAXSIZE);
	}
	~FIFO() {
		this->myQueue->~Aqueue();
	}
	int insert(Elem* e);
	Elem* getDeathElement();
	int remove();
	//void print() override;
	void ensure_index(SearchEngine* s) {}
	void access(int index) {}
};


//===================================================================
class MRU : public ReplacementPolicy {
public:
	MRU() {
		this->count = 0;
		this->myQueue = new Aqueue<Elem>(MAXSIZE);
	}
	~MRU() {
		this->myQueue->~Aqueue();
	}
	int insert(Elem* e);
	void access(int idx);
	
	int remove();
	//void print();
	Elem* getDeathElement();
};


//============================================================================

class LRU : public MRU {
public:
	int remove() override;
	Elem* getDeathElement() override;
};





//========================================================================

class LFU : public ReplacementPolicy {

private:
	int DEFAULT_FREQ = INT32_MAX;

	int* freq_count;
	int PARENT(int index) { return (index - 1) / 2; }
	int LEFT(int index) { return 2 * index + 1; }
	int RIGHT(int index) { return 2 * index + 2; }

public:
	LFU() 
	{
		this->count = 0;
		this->myQueue = new Aqueue<Elem>(MAXSIZE);
		this->freq_count = new int[MAXSIZE];
		for (int i = 0; i < MAXSIZE; i++)
		{
			this->freq_count[i] = DEFAULT_FREQ;
		}
	}
	~LFU() { 
		this->myQueue->~Aqueue();
		delete[] freq_count;
	}
	int insert(Elem* e);
	void access(int idx);
	int remove();
	//void print();
	Elem* getDeathElement() { return this->getElement(0); };
	void reHeapUp(int index, int* freq_count, Aqueue<Elem>*& q);
	void reHeapDown(int index, int *freq_count, Aqueue<Elem>*& q);

	void print_debugging();

};





//=================================================================

class SearchEngine {
public:
	// return index of key in ReplacementPolicy, return -1 if not found
	virtual int search(int key) = 0; 
	virtual int insert(int key, int idx) = 0;
	virtual	int deleteNode(int key) = 0;
	virtual void print(ReplacementPolicy* r) = 0;
	// ensure index in Search engine when ReplacementPolicy has any changes
	virtual void ensure_index(int addr, int idx) = 0;
};


enum class STATUS_TYPE { 
	NIL, 
	NON_EMPTY, 
	DELETED

};


class DBHashing : public SearchEngine {
private:
	// hash function
	int (*hash1)(int);
	int (*hash2)(int);

	// defaut value for address array
	int DEFAULT_VALUE = -1;
	int* addr;
	int* idx;
	STATUS_TYPE* status;
	int size;

	bool check_prime(int n)
	{
		if (n <= 1) return false;
		if (n == 2 || n == 3) return true;
		bool check = true;
		for (int i = 2; i < n; i++)
		{
			if (n % i == 0) { check = false; }
		}
		return check;
	}

	int ensure_size(int n)
	{
		int temp = n * 1.2 + 1;
		while (true)
		{
			if (check_prime(temp) == true)
			{
				return temp;
			}
			temp++;
		}
	}

public:

	DBHashing(int (*h1)(int), int (*h2)(int), int size) {
		this->hash1 = h1;
		this->hash2 = h2;
		if (size < MAXSIZE)
		{
			size = ensure_size(MAXSIZE);
		}
		this->size = size;
		this->addr = new int[size];
		this->idx = new int[size];
		this->status = new STATUS_TYPE[size];
		for (int i = 0; i < size; i++)
		{
			addr[i] = DEFAULT_VALUE;
			idx[i] = -1;
			status[i] = STATUS_TYPE::NIL;
		}
	}
	~DBHashing() {
		delete[] addr;
		delete[] idx;
		delete[] status;
	}

	
	
	int hash(int key, int i);
	int insert(int addr, int idx);
	int find_slot(int addr);
	int deleteNode(int key);
	void print(ReplacementPolicy* r);
	int search(int key);
	void ensure_index(int addr, int idx);

	//for debugging
	void print_for_debugging();
	string get_status(STATUS_TYPE stt);
	
};




class AVL : public SearchEngine {
public:
	
	int insert(int key, int idx);
	int deleteNode(int key);
	void print(ReplacementPolicy* r);
	int search(int key);
	void ensure_index(int addr, int idx);


public:
	class Node;

private:
	Node* root;
	int numOfNode;
public:
	AVL() {
		this->root = NULL;
		this->numOfNode = 0;
	}
	~AVL() {
		this->Clear(root);
	}
	//void run(string filename);
	int get_height_rec(Node* node);
	int get_balance_factor(Node* node);
	int size();
	Node* insert_rec(Node*& node, int addr, int index);
	void INSERT(int addr, int index);
	Node* remove(Node*& node, int addr);
	void REMOVE(int addr);
	Node* search_rec(int addr);
	//void SEARCH(Elem &element);
	Node* get_root();
	Node* balance(Node* node);
	Node* ll_rotation(Node* parent);
	Node* rr_rotation(Node* parent);
	Node* lr_rotation(Node* parent);
	Node* rl_rotation(Node* parent);
	Node* max_node_rec(Node* node);
	Node* min_node_rec(Node* node);

	// preOrder
	void printNLR(Node* node, ReplacementPolicy * r);
	void PRINT_NLR(ReplacementPolicy *r);

	// inOrder
	void printLNR(Node* node, ReplacementPolicy* r);
	void PRINT_LNR(ReplacementPolicy* r);


	void _display(Node* node, int level);
	void DISPLAY();
	void Clear(AVL::Node* node);


	
	public: class Node
	{

	protected:

		// storage address and it's index in ReplacementPolicy
		int idx;
		int addr;
		Node* left;
		Node* right;
		friend class AVL;

	public:
		Node(int addr,int idx) {
			this->left = this->right = NULL;
			this->idx = idx;
			this->addr = addr;
		}
	};
};


#endif