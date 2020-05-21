#include "BTree.h"
#include <stdio.h>

using namespace std;

BTreeNode::BTreeNode() {}

NodeType BTreeNode::getNodeType() {
	return this->type;
}

BTreeInternalNode::BTreeInternalNode() {
	type = INTERNAL;
	weight = 0;
	int i = 0;
	while (i< NUM_KEYS){
		keys[i] = NULL; //배열초기화
		child[i++] = NULL;
	}
	child[i] = NULL;
}

BTreeInternalNode::~BTreeInternalNode() {}

long long BTreeInternalNode::getNthKey(int idx) {
	return this->keys[idx];
}

void BTreeInternalNode::setNthKey(int idx, long long value) {
	this->keys[idx] = value;
	return;
}

void BTreeInternalNode::makeMeRoot(BTreeNode* left, BTreeNode* right, long long newvalue) {
	this->child[0] = left;
	this->child[1] = right;
	this->keys[0] = newvalue;
}

int BTreeInternalNode::findPlace(long long fkey) { //fkey와 key배열 대소비교를 통해 child배열 idx를 반환(caller는 idx를 받아서 child[idx]를 다음으로 탐색)
	for (int i = 0; i < weight; i++) {
		if (fkey < keys[i])
			return i;
	}
	return weight; //마지막
}

long long BTreeInternalNode::split_internal(long long newvalue, int newvalue_idx, BTreeInternalNode* sibling, BTreeNode* rightchild) {
	//newinternal->split_internal로 호출. 
	//returns a value which will be inserted in caller object's parent node.
	//여기서 newvalue_idx는 rightchild가 sibling에 연결되어있던 child 배열 idx임. - child[newvalue_idx+1] = rightchild 해주면 됨. 
	
	this->weight = 4;
	sibling->weight = 6;
	
	//temparr로 keys복사. 
	long long temparr[11];
	for (int i = 0; i < newvalue_idx; i++)
		temparr[i] = sibling->keys[i];
	temparr[newvalue_idx] = newvalue;
	for (int i = newvalue_idx + 1; i < 11; i++)
		temparr[i] = sibling->keys[i - 1];

	//tempchildarr로 child복사, rightchild 포함하여 tempchildarr생성. (연결완료되었다고 볼 수 있음)
	BTreeNode* tempchildarr[12];
	for (int i = 0; i <= newvalue_idx; i++)
		tempchildarr[i] = sibling->child[i];
	tempchildarr[newvalue_idx + 1] = rightchild;
	for (int i = newvalue_idx + 2; i < 12; i++)
		tempchildarr[i] = sibling->child[i - 1];

	//sibling, this의 key 세팅
	for (int i = 0; i < sibling->weight; i++)
		sibling->keys[i] = temparr[i];
	for (int i = sibling->weight; i < NUM_KEYS; i++)
		sibling->keys[i] = NULL;
	
	for (int i = 0; i < this->weight; i++)
		this->keys[i] = temparr[i + sibling->weight + 1];
	for (int i = this->weight; i < NUM_KEYS; i++)
		this->keys[i] = NULL;

	//sibling, this의 child 세팅
	for (int i = 0; i <= sibling->weight; i++)
		sibling->child[i] = tempchildarr[i];
	for (int i = sibling->weight + 1; i < NUM_KEYS + 1; i++)
		sibling->child[i] = NULL;

	for (int i = 0; i <= this->weight; i++)
		this->child[i] = tempchildarr[i + sibling->weight + 1];
	for (int i = this->weight + 1; i < NUM_KEYS + 1; i++)
		this->child[i] = NULL;

	return temparr[sibling->weight];
	//리턴 후, 
	//1. history[i+1] 이 full인지 여부에 따라 다시 split 또는 그냥 right push
	//2. 이 함수를 불렀던 caller object는 새로운 rightchild가 됨. (sibling의 parent와 connection이 필요하다는 소리임~)
}

void BTreeInternalNode::pushRight(long long newvalue, int newvalue_idx, BTreeNode* right) {
	//keys[newvalue_idx]에 right->keys[0] 집어넣고, child[newvalue_idx]=left, child[newvalue_idx+1]=right.
	//이 함수를 부르는 객체가 overflow가 아닌 경우에만 호출 가능. 
	//이때 left, right는 internal일수도, leaf일수도!
	for (int i = this->weight; i > newvalue_idx; i--)
		keys[i] = keys[i - 1];
	for (int i = this->weight + 1; i > newvalue_idx + 1; i--)
		child[i] = child[i - 1];
	keys[newvalue_idx] = newvalue;
	child[newvalue_idx + 1] = right;
	weight++;
}

void BTreeInternalNode::pushLeft(int deleteidx) {
	//deleteIdx는 keys 배열의 idx임. keys[idx], child[idx+1]부터 삭제됨. 
	//한 세트가 아예 없어지는 경우(key, child)
	for (int i = deleteidx; i < weight - 1; i++)
		keys[i] = keys[i + 1];
	keys[weight - 1] = NULL;

	for (int i = deleteidx+1; i < weight; i++)
		child[i] = child[i + 1];
	child[weight] = NULL;
	/*
	if (deleteidx != 0) {
		if (child[deleteidx]->getNodeType() == INTERNAL)
			keys[deleteidx - 1] = ((BTreeInternalNode*)(child[deleteidx]))->getNthKey(0);
		else
			keys[deleteidx - 1] = ((BTreeLeafNode*)(child[deleteidx]))->getNthKey(0);
	}
	*/
	weight--;
}

void BTreeInternalNode::deleteChildOnFront() {
	for (int i = 0; i < weight-1; i++) 
		keys[i] = keys[i + 1];
	keys[weight-1] = NULL;
	for (int i = 0; i < weight; i++)
		child[i] = child[i + 1];
	child[weight] = NULL;

	weight--;
}

void BTreeInternalNode::pushChildOnFront(BTreeNode* newchild) {
	for (int i = weight; i > 0; i--)
		keys[i] = keys[i - 1];
	for (int i = weight + 1; i > 0; i--)
		child[i] = child[i - 1];
	child[0] = newchild;
	keys[0] = ((BTreeLeafNode*)child[1])->getNthKey(0);
	weight++;
}

BTreeNode* BTreeInternalNode::getNthChild(int idx) {
	return this->child[idx];
}

BTreeLeafNode* BTreeInternalNode::findLeafNode(long long fkey) {
	//findplace함수 이용
	//주어진fkey값을 가질 수 있는 leaf node를 반환. 
	int idx = this->findPlace(fkey);
	BTreeNode* obj = this->child[idx];
	while (obj->getNodeType() == INTERNAL) {
		idx = ((BTreeInternalNode*)obj)->findPlace(fkey);
		obj = ((BTreeInternalNode*)obj)->child[idx];
	}
	//이제 obj는 leaf node임
	return (BTreeLeafNode*)obj;
}

BTreeLeafNode* BTreeInternalNode::findLeafNode(long long fkey, History* head) {
	//findplace함수 이용
	//주어진fkey값을 가질 수 있는 leaf node를 반환. 
	//history 링크드리스트 형태로 관리. head와 가까울수록 하위 노드임
	History* temp = new History;
	temp->node = this;
	temp->next = NULL;
	int idx = this->findPlace(fkey);
	temp->childidx = idx;
	head->next = temp;
	//root노드를 head의 next로 끼워넣음

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
	//이제 obj는 leaf node임
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

int BTreeLeafNode::findKey(long long fkey) { //keys배열을 돌면서 fkey의 값을 갖는 원소가 있는지 검색. 존재하면 1 반환
	for (int i = 0; i < weight; i++) {
		if (keys[i] == fkey)
			return 1;
	}
	return NULL; //fkey값이 없는 경우
}

int BTreeLeafNode::findInsertPlace(long long fkey) { //insert할 idx반환. 
	for (int i = 0; i < weight; i++) {
		if (fkey < keys[i])
			return i;
	}
	return weight; 
}

void BTreeLeafNode::changeMyRightSibling(BTreeLeafNode* deletee) {
	//deletee: 이제 지워질 애. 
	//caller 객체의 right sibling을 deletee의 right sibling으로 지정해줌. 
	this->right_sibling = deletee->right_sibling;
}

int BTreeLeafNode::findDeletePlace(long long fkey) { //delete할 key값의 idx반환. 
	for (int i = 0; i < weight; i++) {
		if (fkey == keys[i])
			return i;
	}
	printf("DELETE ERROR: NO SUCH KEY\n");
}

long long BTreeLeafNode::getNthKey(int idx) {
	return this->keys[idx];
}

void BTreeLeafNode::pushRight(long long newvalue, int newvalue_idx) { //newvalue를 keys[newvalue_idx]에 집어넣고 오른쪽으로 한칸 씩 밂. *이 함수는 split이 필요 없는 경우에만 사용 가능
	for (int i = weight; i > newvalue_idx; i--)
		keys[i] = keys[i - 1];
	keys[newvalue_idx] = newvalue;
	weight++;
}

void BTreeLeafNode::pushLeft(int deleteidx) { //deleteidx에 있는 값을 지우고, 그 뒤에 있는 keys를 왼쪽으로 한칸씩 shift, weight--
	for (int i = deleteidx; i < weight-1; i++)
		keys[i] = keys[i + 1];
	for (int i = weight - 1; i < NUM_KEYS; i++) {
		keys[i] = NULL;
	}
	weight--;
}

BTreeLeafNode* BTreeLeafNode::split(long long newvalue, int newvalue_idx) {  //right_sibling연결을 바꾸고 key값들 세팅해줌. split된 객체에서 호출
	BTreeLeafNode* newleaf = new BTreeLeafNode();
	
	newleaf->weight = 5; //새로운 노드에는 5개 key가 들어감. (n=11, 11/2의 반올림)		
	this->weight = 6;

	//temparr로 keys복사. 
	long long temparr[11];
	for (int i = 0; i < newvalue_idx; i++)
		temparr[i] = this->keys[i];
	temparr[newvalue_idx] = newvalue;
	for (int i = newvalue_idx + 1; i < 11; i++)
		temparr[i] = this->keys[i - 1];

	//newleaf, this의 key 초기화
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
		printf("%lld", keys[i]);
		if (i != weight - 1)
			printf(", ");
		else //마지막 원소 출력시 콤마 없이 개행
			printf("\n");
	}
}

void BTreeLeafNode::printLeafNode(long long low, long long high) {
	BTreeLeafNode* temp = this;
	int breakflag = 0;
	while (temp != NULL && breakflag != 1) {
		for (int i = 0; i < temp->weight; i++) {
			if (temp->keys[i] < low)
				continue;
			if (temp->keys[i] >= low && temp->keys[i] < high)
				printf("%lld", temp->keys[i]);
				
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

bool BTreeLeafNode::checkIfLeftmost(History* head) {
	History* cur = head->next;
	int i = 0;
	while (cur != NULL && i < 2) {
		if (cur->childidx != 0)
			return false;
		cur = cur->next;
		i++;
	}
	return true;
}

BTree::BTree() {
	BTreeLeafNode* temp = new BTreeLeafNode();
	temp->weight = 0;
	root = temp;
	//초기 생성시 root는 leafnode의 형태이므로, leaf node 하나 생성 후 weight 0 으로 지정.
}

BTree::~BTree() {}

void BTree::insert(long long value) {
	//root type이 leaf인 경우 - weight확인 - 1. 삽입가능하면 삽입, 2. overflow면 split, InternalNode생성, root로 지정
	//root type이 internal인 경우 - traverse: keys확인하여 타고 내려가기. (while child type==Internal). 내려가서 삽입 후 weight확인 - 1. 삽입가능하면 삽입, 2. overflow면 split, InternalNode생성, insert() 
	if (root->getNodeType() == LEAF) { //leaf가 root인 경우
		BTreeLeafNode* tmp = (BTreeLeafNode*)root;
		
		int idx = tmp->findInsertPlace(value);
		
		if (tmp->weight == NUM_KEYS) { //split. internalnode생성. root와 연결 
			BTreeLeafNode* newleaf = tmp->split(value, idx);
			BTreeInternalNode* newinternal = new BTreeInternalNode();
			newinternal->weight = 1;
			long long newvalue = newleaf->getNthKey(0);
			newinternal->makeMeRoot(tmp, newleaf, newvalue);
			root = newinternal;
		}
		else { //split 필요없음
			tmp->pushRight(value, idx);
		}
	}
	else { //일반적인 case (ROOT != LEAF)
		History* head = new History;
		head->childidx = NULL;
		head->next = NULL;
		head->node = NULL;

		BTreeInternalNode* temp_root = (BTreeInternalNode*)root;
		BTreeLeafNode* leaf = temp_root->findLeafNode(value, head); 
		//value가 들어갈 노드를 찾아서 leaf에 저장
		//head에는 leaf를 찾는 과정에서 거친 internal node가 linked-list 형태로 저장되어 있음

		int idx = leaf->findInsertPlace(value);
		if (leaf->weight == NUM_KEYS) { //split필요
			BTreeLeafNode* newleaf = leaf->split(value, idx);
			History* current = head->next;
			BTreeInternalNode* parent_of_newnode = (BTreeInternalNode*)(current->node);
			//leaf, newleaf : Affected Nodes.
			//leaf node split done.			
			
			
			if (parent_of_newnode->weight == NUM_KEYS) { //internal node도 split 필요 - iterative하게 구현하자 (split_internal_according_to_history)
				/*
				***Internal Node Split - PROPAGATE***
				*/
				//leaf와 internal 연결하는 부분 우선 구현
				//internal이 타고 올라가면서 split하는 부분은 함수로 반복적으로 구현
				
				//1. internal node split, newinternal에 newleaf연결. newinternal 기억해두기
				//2. history[i+1] full 여부 확인. 반복. 
				long long insert_key = newleaf->getNthKey(0);
				BTreeInternalNode* newinternal = new BTreeInternalNode();
				long long next_key = newinternal->split_internal(insert_key, current->childidx, parent_of_newnode, newleaf);
				
				
				while (1) {
					if (current->next == NULL) {
						BTreeInternalNode* newroot = new BTreeInternalNode();
						newroot->weight = 1;
						newroot->makeMeRoot(parent_of_newnode, newinternal, next_key);
						this->root = newroot;
						break;
					}
					else {
						current = current->next;
						parent_of_newnode = (BTreeInternalNode*)(current->node);
						//newinternal은 그대로 사용(affected node)
						//이제 p_o_n의 full 여부를 확인해야 함. 

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
		else { //split필요없음
			leaf->pushRight(value, idx);
		}
	}

}

void BTree::remove(long long value) {
	if (root->getNodeType() == LEAF) { //leaf가 root인 경우
		BTreeLeafNode* tmp = (BTreeLeafNode*)root;

		int idx = tmp->findDeletePlace(value);

		tmp->pushLeft(idx);
	}
	else { //일반적인 case (ROOT != LEAF)
		History* head = new History;
		head->childidx = NULL;
		head->next = NULL;
		head->node = NULL;

		BTreeInternalNode* temp_root = (BTreeInternalNode*)root;
		BTreeLeafNode* leaf = temp_root->findLeafNode(value, head);
		BTreeInternalNode* parent = (BTreeInternalNode*)head->next->node;
		BTreeInternalNode* grand_parent;
		//삭제가 일어날 노드를 찾아서 leaf에 저장,
		//head에는 leaf를 찾는 과정에서 거친 internal node가 linked-list 형태로 저장되어 있음

		int idx = leaf->findDeletePlace(value);
		leaf->pushLeft(idx);

		if (idx == 0 && head->next->childidx != 0)
			parent->setNthKey(head->next->childidx-1, leaf->getNthKey(0));//노드의 최소값 변경된 case - 부모노드 key값 변경해주기
			

		if (leaf->weight < 5) {
			if (leaf->checkIfLeftmost(head)) { //내가 트리의 leftmost 인 경우 - 부모도 leftmost임 (무조건!)
				BTreeLeafNode* right_sibling = (BTreeLeafNode*)parent->getNthChild(1);


				if (right_sibling->weight - 1 >= 2) {
					//Borrow
					//right sibling의 최소값 가져오기, right sibling 노드에서 해당 삭제
					//가져온 값을 leaf에 삽입
					//parent의 key값 right_sibling->key[0]으로 바꿔주기. 
					long long borrow_value = right_sibling->getNthKey(0);
					right_sibling->pushLeft(0); //right의 weight 조절 완료
					leaf->pushRight(borrow_value, leaf->weight); //leaf의 weight 조절 완료
					parent->setNthKey(0, right_sibling->getNthKey(0));
				}
				else {
					//Merge
					//leaf를 통째로 right_sibling 앞에 삽입
					//parent는 left push. 
					for (int i = 0; i < leaf->weight; i++) {
						right_sibling->pushRight(leaf->getNthKey(leaf->weight - 1 - i) , 0);
					}
					

					///////////////////////////////////////////////////////////////nono//leaf는 이제 떨거지.. rightsibling 변경해주지 않아도 괜찮다. (어차피 부모노드에서 끊어내면 그만)


					parent->deleteChildOnFront();

					if (parent->weight < 5) { //leftpush 후, 부모가 underflow라면, 부모도 right sibling과 borrow 또는 merge 필요
						if (parent == root && parent->weight==0) {
							root = right_sibling;
							return;
						}
						
						grand_parent = (BTreeInternalNode*)head->next->next->node;
						BTreeInternalNode* parent_right_sibling = (BTreeInternalNode*)(grand_parent->getNthChild(1));
						if (parent_right_sibling->weight - 1 >= 5) {
							//borrow.
							//right의 leftmost child를 데려와서 parent의 맨 뒤에 붙이고 (rightpush 이용)
							//right는 left push
							//right의 새로운 leftmost key를 grandparent에
							parent->pushRight(((BTreeLeafNode*)parent_right_sibling)->getNthKey(0), parent->weight, parent_right_sibling->getNthChild(0));
							parent_right_sibling->deleteChildOnFront();
							grand_parent->setNthKey(head->next->next->childidx, ((BTreeLeafNode*)parent_right_sibling)->getNthKey(0));
						}
						else {
							//merge. 
							//parent의 key, child를 right의 앞에 끼워넣기 (pushchildonfront)
							for (int i = parent->weight; i >= 0; i--) {
								parent_right_sibling->pushChildOnFront(parent->getNthChild(i));
							}
							//grandparent left push
							grand_parent->deleteChildOnFront();
							if (grand_parent->weight == 0 && grand_parent==root) {
								root = parent_right_sibling;
							}
						}
						
					}

				}
			}
			else { //내가 트리의 leftmost가 아닌 경우. BUT parent는 leftmost일수도 있다. 
				BTreeLeafNode* left_sibling;
				BTreeInternalNode* parent_left_sibling; 
				if (head->next->childidx != 0) // 내가 parent의 leftmost가 아닐 때 - sibling은 바로 왼쪽노드
					left_sibling = (BTreeLeafNode*)parent->getNthChild(head->next->childidx - 1);
				else { //내가 parent의 leftmost인 경우일 때 - parent는 무조건 트리의 leftmost가 아님! 이 경우는 위 if문에서 다룬다 - sibling은 부모의 left sibling의 rightmost child.
					grand_parent = (BTreeInternalNode*)head->next->next->node;
					parent_left_sibling = (BTreeInternalNode*)(grand_parent->getNthChild(head->next->next->childidx - 1));
					left_sibling = (BTreeLeafNode*)parent_left_sibling->getNthChild(parent_left_sibling->weight);
				}



				if (left_sibling->weight - 1 >= 5) {
					//Borrow
					//left sibling의 최대값 가져오기, left sibling 노드에서 해당 삭제
					//가져온 값을 leaf에 삽입
					//parent의 key값 leaf[0]으로 바꿔주기. 
					long long borrow_value = left_sibling->getNthKey(left_sibling->weight - 1);
					left_sibling->pushLeft(left_sibling->weight - 1); //left의 weight 조절 완료
					leaf->pushRight(borrow_value, 0); //leaf의 weight 조절 완료
					if (head->next->childidx != 0)
						parent->setNthKey(head->next->childidx - 1, borrow_value);
				}
				else {
					//Merge
					//leaf를 통째로 left_sibling 뒤에 삽입
					//leaf가 parent의 leftmost라면, parent의 parent key값을 leaf[0]으로 변경
					//parent는 left push. 
					for (int i = 0; i < leaf->weight; i++) {
						left_sibling->pushRight(leaf->getNthKey(i), left_sibling->weight);
					}
					//leftsibling의 rightsibling을 leaf의 rightsibling으로 변경해줘야함. 
					left_sibling->changeMyRightSibling(leaf);

					if (parent == root && parent->weight - 1 == 0) {
						root = left_sibling;
						return;
					}

					grand_parent = (BTreeInternalNode*)head->next->next->node;
					if (head->next->childidx == 0) { //merge 후 지워진 노드가 parent의 leftmost인 경우
						grand_parent->setNthKey(head->next->next->childidx - 1, parent->getNthKey(0));
						parent->deleteChildOnFront();
					}
					else
						parent->pushLeft(head->next->childidx - 1);

					if (parent->weight < 5) { //leftpush 후, 부모가 underflow라면, 부모도 left sibling과 borrow 또는 merge 필요
						if (head->next->next->childidx == 0) { //parent가 트리의 leftmost인 경우
							BTreeInternalNode* parent_right_sibling = (BTreeInternalNode*)(grand_parent->getNthChild(1));
							if (parent_right_sibling->weight - 1 >= 5) {
								//borrow.
								//right의 leftmost child를 데려와서 parent의 맨 뒤에 붙이고 (rightpush 이용)
								//right는 left push
								//right의 새로운 leftmost key를 grandparent에
								parent->pushRight(((BTreeLeafNode*)parent_right_sibling)->getNthKey(0), parent->weight, parent_right_sibling->getNthChild(0));
								parent_right_sibling->deleteChildOnFront();
								grand_parent->setNthKey(head->next->next->childidx, ((BTreeLeafNode*)parent_right_sibling)->getNthKey(0));
							}
							else {
								//merge. 
								//parent의 key, child를 right의 앞에 끼워넣기 (pushchildonfront)
								for (int i = parent->weight; i >= 0; i--) {
									parent_right_sibling->pushChildOnFront(parent->getNthChild(i));
								}
								//grandparent left push
								grand_parent->deleteChildOnFront();
								if (grand_parent->weight == 0 && root==grand_parent) {
									root = parent_right_sibling;
								}
							}
						}
						else { //parent가 트리의 leftmost가 아닌 경우
							parent_left_sibling = (BTreeInternalNode*)(grand_parent->getNthChild(head->next->next->childidx - 1));
							if (parent_left_sibling->weight - 1 >= 5) {
								//borrow.
								//left의 rightmost child를 데려와서 parent의 맨 앞에 끼우고
								//left의 rightmost key를 grandparent에, grandparent의 key를 parent에. 
								parent->pushChildOnFront(parent_left_sibling->getNthChild(parent_left_sibling->weight));
								parent_left_sibling->pushLeft(parent_left_sibling->weight - 1);
								grand_parent->setNthKey(head->next->next->childidx - 1, ((BTreeLeafNode*)parent->getNthChild(0))->getNthKey(0));
							}
							else {
								//merge. 
								//parent의 key, child를 left의 뒤로 갖다 붙이기 (rightPush)
								for (int i = 0; i < parent->weight + 1; i++) {
									parent_left_sibling->pushRight(((BTreeLeafNode*)parent->getNthChild(i))->getNthKey(0), parent_left_sibling->weight, (BTreeLeafNode*)parent->getNthChild(i));
								}
								//grandparent left push
								grand_parent->pushLeft(head->next->next->childidx - 1);
								if (grand_parent->weight == 0 && root==grand_parent) {
									root = parent_left_sibling;
								}
							}
						}
					}
				
				}
			}
		}

	}
}

void BTree::printLeafNode(long long value) { // find the leaf node that contains 'value' and print all values in the leaf node.

	//value가 트리에 없는 경우는 고려하지 않음. 

	BTreeLeafNode* temp;

	if (this->root->getNodeType() == INTERNAL) {
		BTreeInternalNode* internal_temp = (BTreeInternalNode*)this->root;
		temp = internal_temp->findLeafNode(value);
	}
	else {
		temp = (BTreeLeafNode*)this->root;
	}

	if (temp->findKey(value) == NULL)
		printf("No Such Value\n");
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
		printf("%lld\n", value);
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


/****************STATUS******************/

//insert - DONE
//remove - DONE
//printLeafNode - DONE
//pointQuery - DONE
//rangeQuery - DONE
