#include "BTree.h"
#include <stdio.h>

using namespace std;

BTreeNode::BTreeNode() {}

NodeType BTreeNode::getNodeType() {
	return type;
}

BTreeInternalNode::BTreeInternalNode() {
	type = INTERNAL;
	weight = 0;
	int i = 0;
	while (i< NUM_KEYS){
		keys[i] = NULL; //�迭�ʱ�ȭ
		child[i++] = NULL;
	}
	child[i] = NULL;
}

BTreeInternalNode::~BTreeInternalNode() {}

long long BTreeInternalNode::getNthKey(int idx) {
	return this->keys[idx];
}

void BTreeInternalNode::makeMeRoot(BTreeNode* left, BTreeNode* right, long long newvalue) {
	this->child[0] = left;
	this->child[1] = right;
	this->keys[0] = newvalue;
}

int BTreeInternalNode::findPlace(int fkey) { //fkey�� key�迭 ��Һ񱳸� ���� child�迭 idx�� ��ȯ(caller�� idx�� �޾Ƽ� child[idx]�� �������� Ž��)
	for (int i = 0; i < weight; i++) {
		if (fkey < keys[i])
			return i;
	}
	return weight; //������
}

/*
void connect(BTreeNode* left, BTreeNode* right, BTreeNode* parent){
	//split�Ǿ� ���� ���� internal ���(this)���� ȣ����. 
	//1. this�� keys, child ä��� ���� ��� keys, child ����. 
		//�̶�, right->child[0] key���� keys[0]�� ������ ������ left, right�� Ÿ�� ���� ����.
		//leaf�� �����ϴ� ��쿡�� �־��ְ�, 
		//internal�� �����ϴ� ��쿡�� ���� ����. 
	//2. ���� ���� 
	//3. right�� child �迭�� �־���. (���� �������ϱ�)
	//4. ���� �÷����� key
	//����/������ �ڽİ� �̵��� ���� �θ� ��� (���� ���� ���� ���� ��尡 �� ���) ���� ������ ��. 
	
}
*/

long long BTreeInternalNode::split_internal(long long newvalue, int newvalue_idx, BTreeInternalNode* sibling, BTreeNode* rightchild) {
	//newinternal->split_internal�� ȣ��. 
	//returns a value which will be inserted in caller object's parent node.
	//���⼭ newvalue_idx�� rightchild�� sibling�� ����Ǿ��ִ� child �迭 idx��. - child[newvalue_idx+1] = rightchild ���ָ� ��. 
	this->weight = 4;
	sibling->weight = 6;

	//temparr�� keys����. 
	long long temparr[11];
	for (int i = 0; i < newvalue_idx; i++)
		temparr[i] = sibling->keys[i];
	temparr[newvalue_idx] = newvalue;
	for (int i = newvalue_idx + 1; i < 11; i++)
		temparr[i] = sibling->keys[i - 1];

	//tempchildarr�� child����, rightchild �����Ͽ� tempchildarr����. (����Ϸ�Ǿ��ٰ� �� �� ����)
	BTreeNode* tempchildarr[12];
	for (int i = 0; i <= newvalue_idx; i++)
		tempchildarr[i] = sibling->child[i];
	tempchildarr[newvalue_idx + 1] = rightchild;
	for (int i = newvalue_idx + 2; i < 12; i++)
		tempchildarr[i] = sibling->child[i - 1];

	//sibling, this�� key ����
	for (int i = 0; i < sibling->weight; i++)
		sibling->keys[i] = temparr[i];
	for (int i = sibling->weight; i < NUM_KEYS; i++)
		sibling->keys[i] = NULL;
	
	for (int i = 0; i < this->weight; i++)
		this->keys[i] = temparr[i + sibling->weight + 1];
	for (int i = this->weight; i < NUM_KEYS; i++)
		this->keys[i] = NULL;

	//sibling, this�� child ����
	for (int i = 0; i <= sibling->weight; i++)
		sibling->child[i] = tempchildarr[i];
	for (int i = sibling->weight + 1; i < NUM_KEYS + 1; i++)
		sibling->child[i] = NULL;

	for (int i = 0; i <= this->weight; i++)
		this->child[i] = tempchildarr[i + sibling->weight + 2];
	for (int i = this->weight + 1; i < NUM_KEYS + 1; i++)
		this->child[i] = NULL;

	return temparr[sibling->weight];
	//���� ��, 
	//1. history[i+1] �� full���� ���ο� ���� �ٽ� split �Ǵ� �׳� right push
	//2. �� �Լ��� �ҷ��� caller object�� ���ο� rightchild�� ��. (sibling�� parent�� connection�� �ʿ��ϴٴ� �Ҹ���~)
}

void BTreeInternalNode::pushRight(int newvalue, int newvalue_idx, BTreeNode* right) {
	//keys[newvalue_idx]�� right->keys[0] ����ְ�, child[newvalue_idx]=left, child[newvalue_idx+1]=right.
	//�� �Լ��� �θ��� ��ü�� overflow�� �ƴ� ��쿡�� ȣ�� ����. 
	//�̶� left, right�� internal�ϼ���, leaf�ϼ���!
	for (int i = this->weight; i > newvalue_idx; i--)
		keys[i] = keys[i - 1];
	for (int i = this->weight + 1; i > newvalue_idx + 1; i--)
		child[i] = child[i - 1];
	keys[newvalue_idx] = newvalue;
	child[newvalue_idx + 1] = right;
	weight++;
}

BTreeLeafNode* BTreeInternalNode::findLeafNode(int fkey) { 
	//findplace�Լ� �̿�
	//�־���fkey���� ���� �� �ִ� leaf node�� ��ȯ. 
	int idx = this->findPlace(fkey);
	BTreeNode* obj = this->child[idx];
	while (obj->getNodeType() == INTERNAL) {
		idx = ((BTreeInternalNode*)obj)->findPlace(fkey);
		obj = ((BTreeInternalNode*)obj)->child[idx];
	}
	//���� obj�� leaf node��
	return (BTreeLeafNode*)obj;
}

BTreeLeafNode* BTreeInternalNode::findLeafNode(int fkey, History* head) {
	//findplace�Լ� �̿�
	//�־���fkey���� ���� �� �ִ� leaf node�� ��ȯ. 
	//history ��ũ�帮��Ʈ ���·� ����. head�� �������� ���� �����
	History* temp = new History;
	temp->node = this;
	temp->next = NULL;
	int idx = this->findPlace(fkey);
	temp->childidx = idx;
	head->next = temp;
	//root��带 head�� next�� ��������

	BTreeNode* obj = this->child[idx];
	while (obj->getNodeType() == INTERNAL) {
		idx = ((BTreeInternalNode*)obj)->findPlace(fkey);
		
		History* new_temp = new History;
		new_temp->node = obj;
		new_temp->next = head->next;
		new_temp->childidx = idx;
		head->next = new_temp;
		
		obj = ((BTreeInternalNode*)obj)->child[idx];
	}
	//���� obj�� leaf node��
	return (BTreeLeafNode*)obj;
}

BTreeLeafNode::BTreeLeafNode() {
	type = LEAF;
	weight = 0;
	int i = NUM_KEYS;
	right_sibling = NULL;
	while (i--)
		keys[i] = NULL;
}

BTreeLeafNode::~BTreeLeafNode() {}

int BTreeLeafNode::findKey(int fkey) { //keys�迭�� ���鼭 fkey�� ���� ���� ���Ұ� �ִ��� �˻�. �����ϸ� 1 ��ȯ
	for (int i = 0; i < weight; i++) {
		if (keys[i] == fkey)
			return 1;
	}
	return NULL; //fkey���� ���� ���
}

int BTreeLeafNode::findInsertPlace(int fkey) { //insert�� idx��ȯ. 
	for (int i = 0; i < weight; i++) {
		if (fkey < keys[i])
			return i;
	}
	return weight; 
}

long long BTreeLeafNode::getNthKey(int idx) {
	return this->keys[idx];
}

void BTreeLeafNode::pushRight(int newvalue, int newvalue_idx) { //newvalue�� keys[newvalue_idx]�� ����ְ� ���������� ��ĭ �� ��. *�� �Լ��� split�� �ʿ� ���� ��쿡�� ��� ����
	for (int i = weight; i > newvalue_idx; i--)
		keys[i] = keys[i - 1];
	keys[newvalue_idx] = newvalue;
	weight++;
}

BTreeLeafNode* BTreeLeafNode::split(int newvalue, int newvalue_idx) {  //right_sibling������ �ٲٰ� key���� ��������. split�� ��ü���� ȣ��
	BTreeLeafNode* newleaf = new BTreeLeafNode();
	newleaf->weight = 5; //���ο� ��忡�� 5�� key�� ��. (n=11, 11/2�� �ݿø�)		
	this->weight = 6;
	
	//temparr�� keys����. 
	long long temparr[11];
	for (int i = 0; i < newvalue_idx; i++)
		temparr[i] = this->keys[i];
	temparr[newvalue_idx] = newvalue;
	for (int i = newvalue_idx + 1; i < 11; i++)
		temparr[i] = this->keys[i - 1];

	//newleaf, this�� key �ʱ�ȭ
	for (int i = 0; i < this->weight; i++)
		this->keys[i] = temparr[i];
	for (int i = this->weight; i < NUM_KEYS; i++)
		this->keys[i] = NULL;

	for (int i = 0; i < newleaf->weight; i++)
		newleaf->keys[i] = temparr[this->weight+i];
	for (int i = newleaf->weight; i < NUM_KEYS; i++)
		newleaf->keys[i] = NULL;
	
	newleaf->right_sibling = this->right_sibling;
	this->right_sibling = newleaf;

	return newleaf;
}

void BTreeLeafNode::printLeafNode() {
	for (int i = 0; i < weight ; i++) {
		printf("%ld", keys[i]);
		if (i != weight - 1)
			printf(", ");
		else //������ ���� ��½� �޸� ���� ����
			printf("\n");
	}
}

void BTreeLeafNode::printLeafNode(int low, int high) {
	BTreeLeafNode* temp = this;
	int breakflag = 0;
	while (temp != NULL && breakflag != 1) {
		for (int i = 0; i < temp->weight; i++) {
			if (temp->keys[i] < low)
				continue;
			if (temp->keys[i] >= low && temp->keys[i] < high)
				printf("%ld", temp->keys[i]);
				
			if ((i == temp->weight - 1 && temp->right_sibling == NULL) ||  (i < temp->weight - 1 && temp->keys[i + 1] >= high) || (i == temp->weight - 1 && temp->right_sibling != NULL && temp->right_sibling->keys[0] >= high)) {
				breakflag = 1;
				printf("\n");
				break;
			}
			printf(", ");
		}
		temp = temp->right_sibling;
	}
}

BTree::BTree() {
	BTreeLeafNode* temp = new BTreeLeafNode();
	temp->weight = 0;
	root = temp;
	//�ʱ� ������ root�� leafnode�� �����̹Ƿ�, leaf node �ϳ� ���� �� weight 0 ���� ����.
}

BTree::~BTree() {}

void BTree::insert(long long value) {
	//root type�� leaf�� ��� - weightȮ�� - 1. ���԰����ϸ� ����, 2. overflow�� split, InternalNode����, root�� ����
	//root type�� internal�� ��� - traverse: keysȮ���Ͽ� Ÿ�� ��������. (while child type==Internal). �������� ���� �� weightȮ�� - 1. ���԰����ϸ� ����, 2. overflow�� split, InternalNode����, insert() 
	if (root->getNodeType() == LEAF) { //leaf�� root�� ���
		BTreeLeafNode* tmp = (BTreeLeafNode*)root;
		
		int idx = tmp->findInsertPlace(value);
		
		if (tmp->weight == NUM_KEYS) { //split. internalnode����. root�� ���� 
			BTreeLeafNode* newleaf = tmp->split(value, idx);
			BTreeInternalNode* newinternal = new BTreeInternalNode();
			newinternal->weight = 1;
			long long newvalue = newleaf->getNthKey(0);
			newinternal->makeMeRoot(tmp, newleaf, newvalue);
			root = newinternal;
		}
		else { //split �ʿ����
			tmp->pushRight(value, idx);
		}
	}
	else { //�Ϲ����� case (ROOT != LEAF)
		History* head = new History;
		head->childidx = NULL;
		head->next = NULL;
		head->node = NULL;

		BTreeInternalNode* temp_root = (BTreeInternalNode*)root;
		BTreeLeafNode* leaf = temp_root->findLeafNode(value, head); 
		//value�� �� ��带 ã�Ƽ� leaf�� ����
		//head���� leaf�� ã�� �������� ��ģ internal node�� linked-list ���·� ����Ǿ� ����

		int idx = leaf->findInsertPlace(value);
		if (leaf->weight == NUM_KEYS) { //split�ʿ�
			BTreeLeafNode* newleaf = leaf->split(value, idx);
			History* current = head->next;
			BTreeInternalNode* parent_of_newnode = (BTreeInternalNode*)(current->node);
			//leaf, newleaf : Affected Nodes.
			//leaf node split done.			
			
			
			if (parent_of_newnode->weight == NUM_KEYS) { //internal node�� split �ʿ� - iterative�ϰ� �������� (split_internal_according_to_history)
				/*
				***Internal Node Split - PROPAGATE***
				*/
				//leaf�� internal �����ϴ� �κ� �켱 ����
				//internal�� Ÿ�� �ö󰡸鼭 split�ϴ� �κ��� �Լ��� �ݺ������� ����
				
				//1. internal node split, newinternal�� newleaf����. newinternal ����صα�
				//2. history[i+1] full ���� Ȯ��. �ݺ�. 
				long long insert_key = newleaf->getNthKey(0);
				BTreeInternalNode* newinternal = new BTreeInternalNode();
				long long next_key = newinternal->split_internal(insert_key, current->childidx, parent_of_newnode, newleaf);
				
				
				while (1) {
					if (current->next == NULL) {
						BTreeInternalNode* newroot = new BTreeInternalNode();
						newroot->makeMeRoot(parent_of_newnode, newinternal, next_key);
						break;
					}
					else {
						current = current->next;
						parent_of_newnode = (BTreeInternalNode*)(current->node);
						//newinternal�� �״�� ���(affected node)
						//���� p_o_n�� full ���θ� Ȯ���ؾ� ��. 

						if (parent_of_newnode->weight == NUM_KEYS) {
							//split
							BTreeInternalNode* newnewinternal = new BTreeInternalNode();
							next_key = newnewinternal->split_internal(next_key, current->childidx, parent_of_newnode, newinternal);
							newinternal = newnewinternal;
						}
						else {
							parent_of_newnode->pushRight(next_key, current->childidx, newinternal);
							break;
						}
					}
				}
			}
			else {
				parent_of_newnode->pushRight(newleaf->getNthKey(0), head->next->childidx, newleaf);
			}
		}
		else { //split�ʿ����
			leaf->pushRight(value, idx);
		}
	}

}

void BTree::remove(long long value) {

}

void BTree::printLeafNode(long long value) { // find the leaf node that contains 'value' and print all values in the leaf node.

	//value�� Ʈ���� ���� ���� ������� ����. 

	BTreeLeafNode* temp;

	if (this->root->getNodeType() == INTERNAL) {
		BTreeInternalNode* internal_temp = (BTreeInternalNode*)this->root;
		temp = internal_temp->findLeafNode(value);
	}
	else {
		temp = (BTreeLeafNode*)this->root;
	}

	if (temp->findKey(value) == NULL)
		printf("ERROR OCCURED\n");
	else 
		temp->printLeafNode();
	
}

void BTree::pointQuery(long long value) {// print the found value or "NOT FOUND" if there is no value in the index
	
	BTreeLeafNode* temp;

	if (this->root->getNodeType() == INTERNAL) {
		BTreeInternalNode* internal_temp = (BTreeInternalNode*)this->root;
		temp = internal_temp->findLeafNode(value);
	}
	else {
		temp = (BTreeLeafNode*)this->root;
	}

	if (temp->findKey(value) == NULL)
		printf("NOT FOUND\n");
	else
		printf("%ld\n", value);
}

void BTree::rangeQuery(long long low, long long high) {// print all found keys (low <= keys < high), separated by comma (e.g., 10, 11, 13, 15\n) 
	BTreeLeafNode* temp_low, *temp_high;

	if (this->root->getNodeType() == INTERNAL) {
		BTreeInternalNode* internal_temp = (BTreeInternalNode*)this->root;
		temp_low = internal_temp->findLeafNode(low);
		//temp_high = internal_temp->findLeafNode(high);
	}
	else {
		temp_low = (BTreeLeafNode*)this->root;
		//temp_high = (BTreeLeafNode*)this->root;
	}

	temp_low->printLeafNode(low, high);
}


//void insert(long long value);
//void remove(long long value);
//void printLeafNode(long long value); - DONE
//void pointQuery(long long value); - DONE
//void rangeQuery(long long low, long long high); - DONE
