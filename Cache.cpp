
#include"Cache.h"



Cache::Cache(SearchEngine* s, ReplacementPolicy* r) :rp(r), s_engine(s) {}
Cache::~Cache() {
	
   //delete rp;
   //delete s_engine;
}
Data* Cache::read(int addr) {
	int idx = s_engine->search(addr);

	if (idx == -1)
	{
		return nullptr;
	}
	else
	{
		Elem* temp = rp->getElement(idx);
		rp->access(idx);
		rp->ensure_index(s_engine);

		//
		//rp->print_debugging();
		//s_engine->print(rp);

		return temp->data;
	}
}
Elem* Cache::put(int addr, Data* cont) {
	if (rp->isFull())
	{
		Elem* death = rp->getDeathElement();
		s_engine->deleteNode(death->addr);
		rp->remove();

		Elem* temp = new Elem(addr, cont, true);
		rp->insert(temp);
		int idx = rp->size() - 1;
		s_engine->insert(addr, idx);
		rp->ensure_index(s_engine);

		//
		//rp->print_debugging();
		//s_engine->print(rp);

		return death;
	}
	else
	{
		Elem* temp = new Elem(addr, cont, true);
		rp->insert(temp);
		int idx = rp->size() - 1;
		s_engine->insert(addr, idx);
		rp->ensure_index(s_engine);

		//
		//rp->print_debugging();
		//s_engine->print(rp);

		return nullptr;
	}
}
Elem* Cache::write(int addr, Data* cont) {
	int idx = s_engine->search(addr);
	if (idx != -1)
	{
		rp->replace(idx, cont);
		//rp->ensure_index(s_engine);

		/// Debug

		//rp->print_debugging();
		//s_engine->print(rp);

		return nullptr;
	}
	else
	{
		if (rp->isFull())
		{
			Elem* death = rp->getDeathElement();
			s_engine->deleteNode(death->addr);
			rp->remove();
			Elem* temp = new Elem(addr, cont, false);
			rp->insert(temp);
			int idx = rp->size() - 1;
			s_engine->insert(addr, idx);
			rp->ensure_index(s_engine);

			//================================
			//rp->print_debugging();
			//s_engine->print(rp);

			return death;
		}
		else
		{
			Elem* temp = new Elem(addr, cont, false);
			rp->insert(temp);
			int idx = rp->size() - 1;
			s_engine->insert(addr, idx);
			rp->ensure_index(s_engine);

			// 
			//rp->print_debugging();
			//s_engine->print(rp);

			return nullptr;
		}
	}
}
void Cache::printRP() {
    rp->print();
	//rp->print_debugging();
}
void Cache::printSE() {
    s_engine->print(rp);
}


void ReplacementPolicy::ensure_index(SearchEngine* s)
{
	int addr = -1;
	for (int i = 0; i < this->count; i++)
	{
		addr = this->myQueue->arr[i]->addr;
		s->ensure_index(addr, i);
	}
}

// FIFO
int FIFO::insert(Elem* e)
{
    myQueue->push_back(e);
    this->count++;
    return count - 1;
}

int FIFO::remove()
{
    myQueue->pop();
	this->count--;
    return 0;
}

//void FIFO::print()
//{
//    if (myQueue->empty())
//    {
//        return;
//    }
//    else
//    {
//        myQueue->print_elements();
//    }
//}

Elem* FIFO::getDeathElement()
{
	return this->getElement(0);
}

//=============================================
// MRU

int MRU::insert(Elem* e)
{
	myQueue->push_back(e);
	this->count++ ;
	myQueue->move_front(this->count - 1);
	return 0;
}

void MRU::access(int idx)
{
	myQueue->move_front(idx);
}

int MRU::remove()
{
	myQueue->pop();
	this->count--;
	return 0;
}

//void MRU::print()
//{
//	if (myQueue->empty()) { return; }
//	else { myQueue->print_elements(); }
//}

Elem* MRU::getDeathElement()
{
	return this->getElement(0);
}

// ================================================
// LRU

int LRU::remove() 
{
	myQueue->pop_back();
	this->count--;
	return 0;
}
Elem* LRU::getDeathElement()
{
	return this->getElement(this->count - 1);
}

//========================================================================
// LFU
int LFU::insert(Elem* e)
{
	myQueue->push_back(e);
	this->count++;
	this->freq_count[count - 1] = 1;
	int index = this->count - 1;
	this->reHeapUp(index, this->freq_count, this->myQueue);


	//this->print_debugging();


	return 0;
}

int LFU::remove()
{
	myQueue->pop();
	this->count--;
	myQueue->move_front(this->count - 1);

	// assign root = last leaf
	this->freq_count[0] = this->freq_count[this->count];
	// remove count of death element
	this->freq_count[this->count] = this->DEFAULT_FREQ;
	this->reHeapDown(0, this->freq_count, this->myQueue);


	//this->print_debugging();
	return 0;

}

void LFU::access(int idx)
{
	this->freq_count[idx]++;
	this->reHeapDown(idx, this->freq_count, myQueue);


	//this->print_debugging();
}

//void LFU::print()
//{
//	if (myQueue->empty()) { return; }
//	else { myQueue->print_elements(); }
//
//	/*if (myQueue->empty()) { return; }
//	else
//	{
//		int height = (int)log2(this->count) + 1;
//		int left = -1;
//		int right = -1;
//		for (int i = 0; i < height; i++)
//		{
//			if (i == 0)
//			{
//				this->getElement(i)->print();
//			}
//			left = this->LEFT(i);
//			if (left < this->count) {
//				this->getElement(left)->print();
//			}
//			right = this->RIGHT(i);
//			if (right < this->count)
//			{
//				this->getElement(right)->print();
//			}
//		}
//	}*/
//
//
//
//	//this->print_debugging();
//	
//}

void LFU::print_debugging()
{
	printf("-------------------------------------\n");
	printf("| Debugging LFU\n");
	printf("-------------------------------------\n");
	printf("| Queue | Max = %5d | Size = %5d|\n", MAXSIZE, this->count);
	printf("-------------------------------------\n");
	printf("| Index | Addr | Freq | Data | Sync |\n");
	if (this->isEmpty()) { return; }
	else
	{
		for (int i = 0; i < count; i++)
		{
			printf("| %5d | %4d | %4d | %4s | %4d |\n",
				i, this->getElement(i)->addr, this->freq_count[i],
				this->getElement(i)->data->getValue().c_str(),
				this->getElement(i)->sync);
		}
	}
	printf("-------------------------------------\n");
	/*printf("Freq array\n");
	printf("-----------------\n");
	printf("| Index | Value |\n");
	for (int  i = 0; i < MAXSIZE; i++)
	{
		printf("| %5d | %5d |\n", i, freq_count[i]);
	}
	printf("---------------------------------\n");*/
}



void LFU::reHeapDown(int index, int *freq_count, Aqueue<Elem>*& q)
{
	int left = this->LEFT(index);
	int right = this->RIGHT(index);
	int min = index;
		
	if (left < this->count && freq_count[left] <= freq_count[index]) {
			
		min = left;
	}

	if (right < this->count && freq_count[right] <= freq_count[index]) {
			
		if (left < this->count && freq_count[left] <= freq_count[right])
		{
			min = left;
		}
		else
		{
			min = right;
		}
	}

		
	if (min != index)
	{
		swap(freq_count[index], freq_count[min]);
		q->swap(index, min);
		reHeapDown(min, freq_count, q);
	}
}

void LFU::reHeapUp(int index, int* freq_count, Aqueue<Elem>*& q)
{
	if (index > 0)
	{
		int parent = this->PARENT(index);
		//int a = freq_count[index];
		//int b = freq_count[parent];
		if (freq_count[index] < freq_count[parent])
		{
			std::swap(freq_count[index], freq_count[parent]);
			q->swap(index, parent);
			this->reHeapUp(parent,freq_count, myQueue);
		}
	}
}


//========================================================================
// DBHashing

int DBHashing::hash(int key, int i)
{
	int h1 = this->hash1(key);
	int h2 = this->hash2(key);
	int ans = (h1 + i * h2) % this->size;
	return ans;
}


int DBHashing::insert(int addr, int idx)
{
	int slot = 0;


	int pos = -1;
	//int prev = -1;
	//int next = 0;


	while (slot < this->size)
	{
		pos = this->hash(addr, slot);


		//printf("Hash: %d -> value: %d\n", addr, pos);


		if (this->status[pos] == STATUS_TYPE::NIL || this->status[pos] == STATUS_TYPE::DELETED)
		{
			this->addr[pos] = addr;
			this->idx[pos] = idx;
			this->status[pos] = STATUS_TYPE::NON_EMPTY;
			return pos;
		}

		/*if (pos == prev)
		{
			if (this->status[next] == STATUS_TYPE::NIL || this->status[next] == STATUS_TYPE::DELETED)
			{
				this->addr[next] = addr;
				this->idx[next] = idx;
				this->status[next] = STATUS_TYPE::NON_EMPTY;
				return next;
			}
		}
		prev = pos;
		next++;*/


		slot++;
	}
	return -1;
}


int DBHashing::deleteNode(int key)
{
	int pos = this->find_slot(key);
	if (pos != -1)
	{
		this->status[pos] = STATUS_TYPE::DELETED;
		return pos;
	}
	return -1;
}

int DBHashing::search(int key)
{
	int slot = 0;
	//int prev = -1;
	//int next = 0;
	while (slot < this->size)
	{
		int pos = hash(key, slot);
		if (this->status[pos] == STATUS_TYPE::NIL) { return -1; }
		if (this->status[pos] == STATUS_TYPE::NON_EMPTY)
		{
			if (this->addr[pos] == key)
			{
				//printf("Search %d: slot %d, index %d\n", key, pos, this->idx[pos]);
				return this->idx[pos];
			}
		}

		/*if (prev == pos)
		{
			if (this->status[next] == STATUS_TYPE::NIL) { return -1; }
			if (this->status[next] == STATUS_TYPE::NON_EMPTY)
			{
				if (this->addr[next] == key)
				{
					return this->idx[next];
				}
			}
		}
		prev = pos;
		next++;*/

		slot++;
	}
	return -1;
}

int DBHashing::find_slot(int addr)
{
	int slot = 0;
	//int prev = 0;
	//int next = 0;
	while (slot < this->size)
	{
		int pos = hash(addr, slot);
		if (this->status[pos] == STATUS_TYPE::NIL) { return -1; }
		if (this->status[pos] == STATUS_TYPE::NON_EMPTY)
		{
			if (this->addr[pos] == addr)
			{
				return pos;
			}
		}

		/*if (prev == pos)
		{
			if (this->status[next] == STATUS_TYPE::NIL) { return -1; }
			if (this->status[next] == STATUS_TYPE::NON_EMPTY)
			{
				if (this->addr[next] == addr)
				{
					return next;
				}
			}
		}
		prev = pos;
		next++;*/
		slot++;
	}
	return -1;
}

void DBHashing::print(ReplacementPolicy* r)
{
	printf("Prime memory:\n");
	int slot = 0;
	while (slot < this->size)
	{
		if (this->status[slot] == STATUS_TYPE::NON_EMPTY)
		{
			r->getElement(this->idx[slot])->print();
		}
		slot++;
	}


	///===========================================================
	// Debug
	//this->print_for_debugging();
}

string DBHashing::get_status(STATUS_TYPE stt)
{
	if (stt == STATUS_TYPE::NIL) { return "NIL"; }
	else if (stt == STATUS_TYPE::NON_EMPTY) { return "NON_EMPTY"; }
	else { return "DELETED"; }
}

void DBHashing::print_for_debugging()
{
	//
	printf("------------------------------------\n");
	printf("Debugging DBHashing\n");
	printf("------------------------------------\n");
	printf("Hash table | Size = %d\n", this->size);
	printf("------------------------------------\n");
	printf("| Index | Idx | Addr | Status type |\n");
	for (int i = 0; i < size; i++)
	{
		
		printf("| %5d | %3d | %4d | %11s |\n",
			i, this->idx[i], this->addr[i], this->get_status(status[i]).c_str());
		//printf("------------------------------------\n");
	}
	printf("------------------------------------\n");
}


void DBHashing::ensure_index(int addr, int idx)
{
	int pos = this->find_slot(addr);
	if(pos != -1)
	{
		if (status[pos] == STATUS_TYPE::NON_EMPTY)
		{
			this->idx[pos] = idx;
		}
		
	}
}


//========================================================================
// AVL
void AVL::ensure_index(int addr, int idx) {
	Node* temp = this->search_rec(addr);
	if (temp != nullptr) { temp->idx = idx; }
}


int AVL::search(int key)
{
	if (this->numOfNode == 0) { return -1; }
	else
	{
		Node* check = this->search_rec(key);
		if (check != nullptr) { return check->idx; }
		else { return -1; }
	}
}

void AVL::print(ReplacementPolicy* r)
{
	printf("Print AVL in inorder:\n");
	this->PRINT_LNR(r);
	printf("Print AVL in preorder:\n");
	this->PRINT_NLR(r);

	//this->DISPLAY();
}

int AVL::deleteNode(int key)
{
	this->REMOVE(key);
	return 1;
}

int AVL::insert(int key, int i)
{
	this->INSERT(key, i);
	return 1;
}









//=======================================================
// Aqueue
template <class T>
void Aqueue<T>::print_elements()
{
    if (this->empty())
    {
        return;
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            arr[i]->print();
        }
    }
}

template <class T>
void Aqueue<T>::print()
{
    printf("========== Queue ==========\n");
    printf("Size = %d, Capacity = %d\n", this->count, this->capacity);
    if (empty()) {
        printf("Queue empty!\n");
    }
    else
    {
        printf("Index | Address | Data |  Memory  |\n");
        for (int i = 0; i < count; i++)
        {
            printf("%5d | %7d | %4s | %8d |\n", i, arr[i]->addr, arr[i]->data->getValue().c_str(), (int)arr[i]);
        }
    }
}

template <class T>
bool Aqueue<T>::empty()
{
    return this->count == 0;
}

template <class T>
bool Aqueue<T>::full()
{
    return this->count == this->capacity;
}


template <class T>
void Aqueue<T>::clear()
{
    for (int i = 0; i < count; i++)
    {
        delete arr[i];
    }
    this->count = 0;
}


template <class T>
T* Aqueue<T>::front()
{
    return this->arr[0];
}

template <class T>
T* Aqueue<T>::back()
{
    return this->arr[this->count - 1];
}

template <class T>
void Aqueue<T>::push_back(T* e)
{
    if (this->full())
    {
        printf("Queue overflow!\n");
    }
    else
    {
        arr[count] = e;
        count++;
    }
}


template <class T>
void Aqueue<T>::pop()
{
    if (this->empty())
    {
        printf("Queue underflow!\n");
    }
    else if(count == 1)
    {
        this->clear();
    }
    else
    {
        T* death = arr[0];
        for (int i = 0; i < count; i++)
        {
			if (i == count - 1)
			{
				arr[i] = nullptr;
			}
			else
			{
				arr[i] = arr[i + 1];
			}
            
        }
        delete death;
        count--;
    }
}


template <class T>
void Aqueue<T>::pop_back()
{
    if (this->empty())
    {
        printf("Queue underflow!\n");
    }
    else if (count == 1)
    {
        this->clear();
    }
    else
    {
        Elem* death = arr[this->count - 1];
        delete death;
        count--;
    }
}


template <class T>
void Aqueue<T>::move_front(int index)
{
    if (index < 0 || index >= count) {
        printf("Invalid move position!\n");
    }
    else
    {
        if (index == 0)
        {
            return;
        }
        else
        {
            T* temp = arr[index];
            for (int  i = index; i > 0; i--)
            {
                arr[i] = arr[i - 1];
            }
            arr[0] = temp;
        }
    }
}


template <class T>
void Aqueue<T>::move_back(int index)
{
	if (index < 0 || index >= count) {
		printf("Invalid move position!\n");
	}
	else
	{
		if (index == this->count - 1)
		{
			return;
		}
		else
		{
			T* temp = arr[index];
			for (int i = index; i < this->count - 1; i++)
			{
				arr[i] = arr[i + 1];
			}
			arr[this->count - 1] = temp;
		}
	}
}

template <class T>
void Aqueue<T>::swap(int a, int b)
{
	try
	{
		Elem* temp = this->arr[a];
		this->arr[a] = this->arr[b];
		this->arr[b] = temp;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}
	
}


template <class T>
T* Aqueue<T>::replace(int index, T* e)
{
	if (index < 0 || index >= count) {
		printf("Invalid replace position\n");
	}
	else
	{
		T* death = arr[index];
		this->arr[index] = e;
		return death;
	}
}


//====================================================================
// AVL tree

int AVL::size() {
	return this->numOfNode;
}

void AVL::Clear(Node* node) {
	if (node != NULL) {
		Clear(node->left);
		Clear(node->right);
		delete node;
	}
	node = NULL;
	numOfNode = 0;
}


int AVL::get_height_rec(Node* node) {
	if (node == NULL)
		return 0;
	int left_height = get_height_rec(node->left);
	int right_height = get_height_rec(node->right);

	return left_height > right_height ? left_height + 1 : right_height + 1;
}


int AVL::get_balance_factor(Node* node) {
	int left_height = get_height_rec(node->left);
	int right_height = get_height_rec(node->right);
	return left_height - right_height;
}



typename AVL::Node* AVL::balance(Node* node) {
	int balance_factor = get_balance_factor(node);

	if (balance_factor > 1) {
		if (get_balance_factor(node->left) > 0) {
			node = ll_rotation(node);
		}
		else {
			node = lr_rotation(node);
		}
	}
	else if (balance_factor < -1) {
		if (get_balance_factor(node->right) > 0) {
			node = rl_rotation(node);
		}
		else {
			node = rr_rotation(node);
		}
	}
	return node;
}


typename AVL::Node* AVL::ll_rotation(Node* parent) {
	Node* current;
	current = parent->left;
	parent->left = current->right;
	current->right = parent;

	return current;
}


typename AVL::Node* AVL::rr_rotation(Node* parent) {
	Node* current;
	current = parent->right;
	parent->right = current->left;
	current->left = parent;
	return current;
}



typename AVL::Node* AVL::lr_rotation(Node* parent) {
	Node* current;
	current = parent->left;
	parent->left = this->rr_rotation(current);
	return this->ll_rotation(parent);
}

typename AVL::Node* AVL::rl_rotation(Node* parent) {
	Node* current;
	current = parent->right;
	parent->right = this->ll_rotation(current);
	return this->rr_rotation(parent);
}


typename AVL::Node* AVL::search_rec(int address) {
	Node* current = root;
	while (current != NULL)
	{
		if (current->addr == address) {
			return current;
		}
		address > current->addr ? current = current->right : current = current->left;
	}
	return nullptr;
}


typename AVL::Node* AVL::insert_rec(Node*& node, int addr, int idx) {
	if (node == NULL) {
		Node* temp = new Node(addr, idx);
		numOfNode++;
		node = temp;
		node->left = NULL;
		node->right = NULL;
		return node;
	}
	else if (node->addr > addr) {
		node->left = insert_rec(node->left, addr, idx);
		node = balance(node);
	}
	else if (node->addr <= addr) {
		node->right = insert_rec(node->right, addr, idx);
		node = balance(node);
	}

	return node;
}


void AVL::INSERT(int addr, int index) {
	this->insert_rec(root, addr, index);
}



typename AVL::Node* AVL::remove(Node*& node, int addr) {
	
	if (node == NULL) return NULL;
	else if (addr > node->addr) node->right = remove(node->right, addr);
	else if (addr < node->addr) node->left = remove(node->left, addr);
	// element found and has 2 children
	else if (node->left != NULL && node->right != NULL)
	{
		Node* temp = nullptr;
		// get min node of right sub-tree
		temp = this->min_node_rec(node->right);
		// assign data of current node to min
		node->addr = temp->addr;
		node->idx = temp->idx;
		int duplicate = node->addr;

		node->right = remove(node->right, duplicate);
	}
	else
	{
		Node* temp = nullptr;
		temp = node;
		if (node->left == NULL) node = node->right;
		else if (node->right == NULL) node = node->left;
		delete temp;
		numOfNode--;
	}
	if (node == NULL) return node;
	node = this->balance(node);
	return node;
}


void AVL::REMOVE(int address) {
	if (this->search_rec(address) != NULL) {
		this->remove(root, address);
	}
}

void AVL::printNLR(Node* node, ReplacementPolicy* r)
{
	if (node == NULL) return;
	r->getElement(node->idx)->print();
	printNLR(node->left, r);
	printNLR(node->right, r);

}
void AVL::PRINT_NLR(ReplacementPolicy* r) {
	this->printNLR(this->root, r);
}

// inOrder
void AVL::printLNR(Node* node, ReplacementPolicy* r)
{
	if (node == NULL) return;

	printLNR(node->left, r);
	//printf("%d ", node->id);
	r->getElement(node->idx)->print();
	printLNR(node->right, r);


}

void AVL::PRINT_LNR(ReplacementPolicy* r)
{
	printLNR(this->root, r);
}



typename AVL::Node* AVL::get_root() {
	return this->root;
}



void AVL::_display(Node* node, int level) {

	int i;
	if (node != NULL)
	{
		_display(node->right, level + 1);
		printf("\n");
		if (node == root)
			std::cout << "Root -> ";
		for (i = 0; i < level && node != root; i++)
			std::cout << "      ";
		std::cout << node->addr;
		_display(node->left, level + 1);
	}
}

void AVL::DISPLAY()
{
	printf("\n-----------------------------------------\n");
	printf("Size: %d", this->numOfNode);
	printf("\n-----------------------------------------\n");
	this->_display(root, 1);
	printf("\n-----------------------------------------\n");
}

typename AVL::Node* AVL::max_node_rec(Node* node) {
	if (node == NULL) {
		return NULL;
	}
	else if (node->right == NULL) {
		return node;
	}
	else {
		return max_node_rec(node->right);
	}
}



typename AVL::Node* AVL::min_node_rec(Node* node) {
	if (node == NULL) {
		return NULL;
	}
	else if (node->left == NULL) {
		return node;
	}
	else {
		return min_node_rec(node->left);
	}
}