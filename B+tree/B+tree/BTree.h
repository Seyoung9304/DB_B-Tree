// Note: You need to add additional member variables and functions as you need.

#define NUM_KEYS 10 
// NUM_KEYS should be set to make each tree node as large as 4KB. 
// But for this assignment, we will set the degree of node to 10 just to make debugging and grading easy
// Please do not change this number. 
// In practice, DBMS employs 'slotted page structure' to store variable-length records in B+tree.
// But again, the purpose of this assignment is to make students familiar with B-tree itself. 

enum NodeType {
	ROOT,
	INTERNAL,
	LEAF
};


class BTreeNode {
protected:
	NodeType type;
public:
	BTreeNode();
	virtual ~BTreeNode() {}
	NodeType getNodeType();
};

typedef struct history {
	int childidx;
	BTreeNode* node;
	struct history* next;
}History;

class BTreeLeafNode :public BTreeNode {
private:
	long long keys[NUM_KEYS];
	BTreeLeafNode* right_sibling;
public:
	int weight;
	BTreeLeafNode();
	~BTreeLeafNode();
	int findKey(long long fkey);
	int findInsertPlace(long long fkey);
	long long getNthKey(int idx); //keys[idx]값 반환
	void pushLeft(int deleteidx);
	void pushRight(long long newvalue, int newvalue_idx);
	BTreeLeafNode* split(long long newvalue, int newvalue_idx); //right_sibling연결을 바꾸고 key값들 세팅해줌. split된 객체에서 호출. 새로 생긴 leaf node 리턴.
	void printLeafNode();// print all keys in the current leaf node, separated by comma.
	void printLeafNode(long long low, long long high);
	void changeMyRightSibling(BTreeLeafNode* deletee);
	int findDeletePlace(long long fkey);
	bool checkIfLeftmost(History* head);
};

class BTreeInternalNode :public BTreeNode {
private:
	long long keys[NUM_KEYS];
	BTreeNode* child[NUM_KEYS + 1];
public:
	int weight;
	BTreeInternalNode();
	~BTreeInternalNode();
	long long getNthKey(int idx); //keys[idx]값 반환
	void setNthKey(int idx, long long value); //keys[idx]=value;
	void makeMeRoot(BTreeNode* left, BTreeNode* right, long long newvalue);
	int findPlace(long long fkey);
	long long split_internal(long long newvalue, int newvalue_idx, BTreeInternalNode* sibling, BTreeNode* rightchild); // affected child연결까지.
	void pushRight(long long newvalue, int newvalue_idx, BTreeNode* right);
	void pushLeft(int deleteidx);
	void pushChildOnFront(BTreeNode* newchild); //key, child가 세트로 이동
	void deleteChildOnFront(); //leftmost child 삭제시, key와 child 모두 한칸씩 앞으로 이동
	//bool checkIfLeftmost(History* head);
	BTreeNode* getNthChild(int idx);
	BTreeLeafNode* findLeafNode(long long fkey);
	BTreeLeafNode* findLeafNode(long long fkey, History* linked_list);
};



class BTree {
private:
	BTreeNode* root;
public:
	// You are not allowed to change the interfaces of these public methods.
	BTree();
	~BTree();
	void insert(long long value);
	void remove(long long value);
	void printLeafNode(long long value);
	// find the leaf node that contains 'value' and print all values in the leaf node.
	void pointQuery(long long value);
	// print the found value or "NOT FOUND" if there is no value in the index
	void rangeQuery(long long low, long long high);
	// print all found keys (low <= keys < high), separated by comma (e.g., 10, 11, 13, 15\n) 
};

