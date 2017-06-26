#pragma warning(disable: 4996)

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>

using namespace std;

#define MIN_ORDER 2
#define BUCKET_SIZE 128
#define BUCKET_SIZE2 147
#define DEFAULT_BUCKET_SIZE 2
#define DEFAULT_SLOT_SIZE 128
#define DEFAULT_SLOT_SIZE2 147
#define MM 64 // B+트리의 차수
#define M (MM*2+1) // 홀수 차수,key개수 128개로 만듬
#define MM2 73 // B+트리의 차수
#define M2 (MM2*2+1) // 홀수 차수,key개수 128개로 만듬


#define MAX 500 // 큐의 크기
#define TR 500 // 스택의 크기

//4096 /32 =128 // 4096 / 28 147

int skeysize = 0;
int pkeysize = 0;
int Qsize = 0;

typedef struct Node {
	int count;
	int sid[M - 1]; // 학생id 중복경우에 누구 것인지 확인하기 위함
	double score[M - 1]; // score
	int p[M]; // 리프노드면 블록위치 저장
	struct Node* branch[M]; // 주소
}node;

typedef struct Node2 {
	int count2;
	int sid2[M - 1]; // 학생id 중복경우에 누구 것인지 확인하기 위함
	int score2[M - 1]; // score
	int p2[M]; // 리프노드면 블록위치 저장
	struct Node2* branch2[M]; // 주소
}node2;

node* root; // 루트 node
int front = 0, rear = 0; // 큐를 구성하기 위함
node* queue[MAX];
int findcnt;
int leafnum = 0; // leaf노드 개수구하기 위함

int* search(double k); // key 탐색 함수
int* insertItem(double k, int sid, int blocknum);
node* bput(node* k); // 큐 입력 함수
node* get();
int isEmpty(); // 큐 공백 체크

void indexNodePrint(node* t);

unsigned int LIST_SIZE;
unsigned int LIST_SIZE2;

node2* root2;//·çÆ® node
int front2 = 0, rear2 = 0; //Å¥¸¦ ±¸¼ºÇÏ±â À§ÇÔ
node2* queue2[MAX];
int findcnt2;
int leafnum2 = 0;//leaf³ëµå °³¼ö±¸ÇÏ±â À§ÇÔ

int* search2(int k); //key Å½»ö ÇÔ¼ö
int* insertItem2(int k, int sid, int blocknum);
node2* bput2(node2* k);//Å¥ ÀÔ·Â ÇÔ¼ö
node2* get2();
int isEmpty2(); //Å¥ °ø¹é Ã¼Å©

void indexNodePrint2(node2* t);


///////////////////////////////////////////////////////////////////////////////
// DYNAMIC HASHING FUNCTIONS
// 1. StElement -> Students Element를 나타내는 Class
// 2. PrElement -> Professors Element를 나타내는 Class
///////////////////////////////////////////////////////////////////////////////

class StElement {
	char name[20];
	unsigned int studentID;
	float score;
	unsigned int advisorID;
public:
	// set class elements
	void setName(char* _name) { strcpy(name, _name); }
	void setSID(unsigned int _studentID) { studentID = _studentID; }
	void setScore(float _score) { score = _score; };
	void setAID(unsigned int _advisorID) { advisorID = _advisorID; }

	// get class elements
	char* getName() { return name; }
	unsigned int getSID() { return studentID; }
	float getScore() { return score; }
	unsigned int getAID() { return advisorID; }

	// functions
	void setElement(char*, unsigned int, float, unsigned int);
};
class PrElement {
	char name[20];
	unsigned int ProfID;
	int salary;
public:
	// set class elements
	void setPName(char* _name) { strcpy(name, _name); }
	void setPID(unsigned int _ProfID) { ProfID = _ProfID; }
	void setSalary(int _salary) { salary = _salary; };

	// get class elements
	char* getPName() { return name; }
	unsigned int getPID() { return ProfID; }
	int getSalary() { return salary; }


	// functions
	void setPElement(char*, unsigned int, int);
};
void StElement::setElement(char* _name, unsigned int _studentID, float _score, unsigned int _advisorID) {
	setName(_name);
	setSID(_studentID);
	setScore(_score);
	setAID(_advisorID);
}
void PrElement::setPElement(char* _name, unsigned int _ProfID, int _salary) {
	setPName(_name);
	setPID(_ProfID);
	setSalary(_salary);
}

///////////////////////////////////////////////////////////////////////////////
// DYNAMIC HASHING FUNCTIONS
// 1. Leaf -> Bucket을 나타내는 자료구조
// 2. Directory -> Bucket Address Table을 나타내는 자료구조
///////////////////////////////////////////////////////////////////////////////

typedef struct {
	int header;
	int count;  // Leaf Node에 들어있는 Record 숫자
	StElement** pRecord; // Leaf Node에 연결된 Record들
} StLeaf;

typedef struct {
	int header;
	int count;  // Leaf Node에 들어있는 Record 숫자
	PrElement** pRecord; // Leaf Node에 연결된 Record들
} PrLeaf;

typedef struct {
	int header;
	int divCount;  // Bucket Address Table의 분할 횟수를 나타낸다
	StLeaf** entry;
} StDirectory;

typedef struct
{
	int header;
	int divCount;  // Bucket Address Table의 분할 횟수를 나타낸다
	PrLeaf** entry;
} PrDirectory;

///////////////////////////////////////////////////////////////////////////////
// DYNAMIC HASHING FUNCTIONS
// 1. pow_2 -> 입력된 p번 2배 곱하는 함수
// 2. StMakePseudoKey, PrMakePseudoKey -> Students, Professors의 PseudoKey를 생성함
// 3. StNReturn, PrNReturn -> Students, Professors의  n bit를 Return
///////////////////////////////////////////////////////////////////////////////

// Á¦°öÀ» ÇØÁÖ´Â ÇÔ¼ö
int pow_2(int p) {
	int i;
	int x = 1;
	for (i = 0; i < p; i++) x *= 2;
	return x;
}

// pseudokey¸¦ ¸¸µé¾î¼­ return ÇÑ´Ù.
// KEY ±æÀÌ¸¸Å­ Á¦°ö ÈÄ mod ¿¬»êÀ» ÅëÇØ ÇÏÀ§ KEY_LENGTH Å°¸¸Å­ Àß¶ó³½´Ù
int StMakePseudoKey(int key) { return key % pow_2(skeysize); }
int PrMakePseudoKey(int key) { return key % pow_2(pkeysize); }

//  »óÀ§ n bit¸¦ µ¹·ÁÁØ´Ù.
int StNReturn(int k, int n) { return k / pow_2(skeysize - n); }
int PrNReturn(int k, int n) { return k / pow_2(pkeysize - n); }

///////////////////////////////////////////////////////////////////////////////
// DYNAMIC HASHING FUNCTIONS
// 1. StGetBitSize, PrGetBitSize -> ¾ó¸¶ÀÇ bit°¡ ÇÊ¿äÇÒ °ÍÀÎÁö ¹Ì¸® °è»ê(2)
// 2. StGetBlockSize, PrGetBlockSize -> ¾ó¸¶ÀÇ blockÀÌ ÇÊ¿äÇÒ °ÍÀÎÁö ¹Ì¸® °è»ê(1)
///////////////////////////////////////////////////////////////////////////////

void StGetBitSize(int nCount) {
	++skeysize;
	if (nCount <= 0) {
		skeysize = skeysize - 1;
		return;
	}
	else StGetBitSize(nCount = nCount / 2);        //ÀÎÀÚ·Î ¹ÞÀº nCount¿¡ -1À» ÁÙ¿©¼­ È£ÃâÇÑ´Ù.
}
void PrGetBitSize(int nCount) {
	++pkeysize;
	if (nCount <= 0) {
		pkeysize = pkeysize - 1;
		return;
	}
	else PrGetBitSize(nCount = nCount / 2);        //ÀÎÀÚ·Î ¹ÞÀº nCount¿¡ -1À» ÁÙ¿©¼­ È£ÃâÇÑ´Ù.
}

void StGetBlockSize(vector<StElement> a) {
	int b = a.size();
	StGetBitSize(b / 128);
}
void PrGetBlockSize(vector<PrElement> a) {
	int b = a.size();
	PrGetBitSize(b / 147);
}


///////////////////////////////////////////////////////////////////////////////
// DYNAMIC HASHING FUNCTIONS
// Exact Query Processing 
// 1. StExactQuery -> Student Exact Query Processing
// 2. PrExactQuery -> Professor Exact Query Processing 
///////////////////////////////////////////////////////////////////////////////

void StExactQuery(StDirectory* dir, int targetID, ofstream &of) {
	int hashResult = 0;
	int i;
	StLeaf** blockPointer;
	hashResult = StMakePseudoKey(targetID);
	blockPointer = &(dir->entry[hashResult]);
	for (int i = 0; i < (dir->entry[hashResult]->count); i++) {
		if (targetID == (dir->entry[hashResult]->pRecord[i]->getSID())) {
			of << dir->entry[hashResult]->pRecord[i]->getName() << " ";
			of << dir->entry[hashResult]->pRecord[i]->getSID() << " ";
			of << dir->entry[hashResult]->pRecord[i]->getScore() << " ";
			of << dir->entry[hashResult]->pRecord[i]->getAID() << endl;
			break;
		}
	}
}
void PrExactQuery(PrDirectory* dir, int targetID, ofstream &of) {
	int hashResult = 0;
	int i;
	PrLeaf** blockPointer;
	hashResult = PrMakePseudoKey(targetID);
	blockPointer = &(dir->entry[hashResult]);
	for (int i = 0; i < (dir->entry[hashResult]->count); i++) {
		if (targetID == (dir->entry[hashResult]->pRecord[i]->getPID())) {
			of << dir->entry[hashResult]->pRecord[i]->getPID() << " ";
			of << dir->entry[hashResult]->pRecord[i]->getPName() << " ";
			of << dir->entry[hashResult]->pRecord[i]->getSalary() << endl;
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// DYNAMIC HASHING FUNCTIONS
// Insert Record at bucket 
// 1. StInsertRecord -> Student Insert Record
// 2. PrInsertRecord -> Professor Insert Record
///////////////////////////////////////////////////////////////////////////////

int StInsertRecord(StElement* rec, StDirectory* dir) {
	int i, ind;
	int key = rec->getSID();        // »ðÀÔµÇ´Â recordÀÇ key
	int pseudokey = StMakePseudoKey(key);    // pseudokey
	int index = StNReturn(pseudokey, dir->header); // entryÀÇ ¹øÈ£¸¦ Ã£´Â´Ù.
	StLeaf* bucket = dir->entry[index];    // »ðÀÔÇÏ°íÀÚ ÇÏ´Â leaf

										   // bucketÀ» È®ÀÎÇØ¼­ »ðÀÔÇÏ°íÀÚ ÇÏ´Â Å°°¡ ÀÌ¹Ì Á¸ÀçÇÏ´ÂÁö È®ÀÎ
	for (i = 0; i<bucket->count; i++) {
		if (bucket->pRecord[i]->getSID() == key) {
			printf("\n\nThe key is already exist!!!\n");
			return 0;  // ºñÁ¤»ó Á¾·á
		}
	}

	// bucket¿¡ ÀúÀåÇÒ °ø°£ÀÌ ³²¾ÆÀÖ´Ù¸é ºóÄ­¿¡ ÀúÀåÇÑ´Ù.
	if (bucket->count < BUCKET_SIZE) {
		bucket->pRecord[bucket->count] = rec;
		bucket->count++;
		return 1;   // Á¤»óÁ¾·á
	}

	// bucketÀÌ ´Ù Ã¡´Ù¸é overflow Ã³¸®
	while (1) {
		int n;
		StLeaf* newBucket;  // »õ·Î ¸¸µé bucket;
							//////  d < t+1ÀÎ°æ¿ì => ¸ÕÀú µð·ºÅä¸®¸¦ µÎ¹è ´Ã¸°´Ù
		if (dir->header < bucket->header + 1) {
			int numEntry;    // »õ·Î ¸¸µé¾îÁú entryÀÇ ¼ö
			StLeaf** newEntry;
			dir->header++;
			numEntry = pow_2(dir->header);
			newEntry = (StLeaf**)malloc(sizeof(StLeaf*)*numEntry);
			for (i = 0; i<numEntry / 2; i++)  // entry¸¦ ´Ã¾î³­ ¸¸Å­ ºÐ¹è
				newEntry[i * 2] = newEntry[i * 2 + 1] = dir->entry[i];
			dir->divCount = 0;
			free(dir->entry);
			dir->entry = newEntry;
		}
		// overflow°¡ »ý±ä leaf¸¦ splitÇÑ´Ù.
		// »õ·Î¿î bucket »ý¼º
		newBucket = (StLeaf*)malloc(sizeof(StLeaf));
		newBucket->header = bucket->header;
		newBucket->count = 0;
		newBucket->pRecord = (StElement**)malloc(sizeof(StElement*) * BUCKET_SIZE);

		// bucket³»ÀÇ record ºÐ¹è
		bucket->count = 0;
		for (i = 0; i<BUCKET_SIZE; i++) {
			if (StNReturn(StMakePseudoKey(bucket->pRecord[i]->getSID()), bucket->header + 1) % 2 == 0) {
				bucket->pRecord[bucket->count] = bucket->pRecord[i];
				bucket->count++;
			}
			else {
				newBucket->pRecord[newBucket->count] = bucket->pRecord[i];
				newBucket->count++;
			}
		}
		bucket->header++;
		newBucket->header++;
		if (bucket->header == dir->header)
			dir->divCount++;

		// entry -> leaf ·ÎÀÇ pointer Á¶Àý
		n = pow_2(dir->header - bucket->header + 1);      // ³ª´©¾îÁú entry ¼ö
		ind = StNReturn(StMakePseudoKey(key), bucket->header - 1) * n;   // ³ª´©¾îÁú entryÁß Ã¹¹øÂ° entryÀÇ index

		for (i = 0; i<n / 2; i++, ind++)        // µÎ°³ÀÇ bucketÀ¸·Î ³ª´®
			dir->entry[ind] = bucket;
		for (i = 0; i<n / 2; i++, ind++)
			dir->entry[ind] = newBucket;

		// ´Ù½Ã »ðÀÔµÉ ³ëµå¸¦ ÀúÀåÇÑ´Ù.
		index = StNReturn(pseudokey, dir->header);
		bucket = dir->entry[index];
		if (bucket->count < BUCKET_SIZE) // ÀúÀåÇÒ °÷ÀÇ bucketÀÌ fullÀÌ ¾Æ´Ï¸é ÀúÀå
		{
			bucket->pRecord[bucket->count] = rec;
			bucket->count++;
			return 1;   // Á¤»óÁ¾·á
		}
		// ÀúÀåÇÒ °÷ÀÌ fullÀÌ ¾Æ´Ï¸é 2ÀÇ Ã³À½À¸·Î µ¹¾Æ°¡¼­ ´Ù½Ã directory¸¦ ´Ã¸°´Ù.
	}
}
int PrInsertRecord(PrElement* rec, PrDirectory* dir) {
	int i, ind;
	int key = rec->getPID();        // »ðÀÔµÇ´Â recordÀÇ key
	int pseudokey = PrMakePseudoKey(key);    // pseudokey
	int index = PrNReturn(pseudokey, dir->header); // entryÀÇ ¹øÈ£¸¦ Ã£´Â´Ù.
	PrLeaf* bucket2 = dir->entry[index];    // »ðÀÔÇÏ°íÀÚ ÇÏ´Â leaf

											// bucketÀ» È®ÀÎÇØ¼­ »ðÀÔÇÏ°íÀÚ ÇÏ´Â Å°°¡ ÀÌ¹Ì Á¸ÀçÇÏ´ÂÁö È®ÀÎ
	for (i = 0; i<bucket2->count; i++) {
		if (bucket2->pRecord[i]->getPID() == key) {
			printf("\n\nThe key is already exist!!!\n");
			return 0;  // ºñÁ¤»ó Á¾·á
		}
	}

	// bucket¿¡ ÀúÀåÇÒ °ø°£ÀÌ ³²¾ÆÀÖ´Ù¸é ºóÄ­¿¡ ÀúÀåÇÑ´Ù.
	if (bucket2->count < BUCKET_SIZE2) {
		bucket2->pRecord[bucket2->count] = rec;
		bucket2->count++;
		return 1;   // Á¤»óÁ¾·á
	}

	// bucketÀÌ ´Ù Ã¡´Ù¸é overflow Ã³¸®
	while (1) {
		int n;
		PrLeaf* newBucket;  // »õ·Î ¸¸µé bucket;
							//////  d < t+1ÀÎ°æ¿ì => ¸ÕÀú µð·ºÅä¸®¸¦ µÎ¹è ´Ã¸°´Ù
		if (dir->header < bucket2->header + 1) {
			int numEntry;    // »õ·Î ¸¸µé¾îÁú entryÀÇ ¼ö
			PrLeaf** newEntry;
			dir->header++;
			numEntry = pow_2(dir->header);
			newEntry = (PrLeaf**)malloc(sizeof(PrLeaf*)*numEntry);
			for (i = 0; i<numEntry / 2; i++)  // entry¸¦ ´Ã¾î³­ ¸¸Å­ ºÐ¹è
				newEntry[i * 2] = newEntry[i * 2 + 1] = dir->entry[i];
			dir->divCount = 0;
			free(dir->entry);
			dir->entry = newEntry;
		}
		// overflow°¡ »ý±ä leaf¸¦ splitÇÑ´Ù.
		// »õ·Î¿î bucket »ý¼º
		newBucket = (PrLeaf*)malloc(sizeof(PrLeaf));
		newBucket->header = bucket2->header;
		newBucket->count = 0;
		newBucket->pRecord = (PrElement**)malloc(sizeof(PrElement*) * BUCKET_SIZE2);

		// bucket³»ÀÇ record ºÐ¹è
		bucket2->count = 0;
		for (i = 0; i<BUCKET_SIZE2; i++) {
			if (PrNReturn(PrMakePseudoKey(bucket2->pRecord[i]->getPID()), bucket2->header + 1) % 2 == 0) {
				bucket2->pRecord[bucket2->count] = bucket2->pRecord[i];
				bucket2->count++;
			}
			else {
				newBucket->pRecord[newBucket->count] = bucket2->pRecord[i];
				newBucket->count++;
			}
		}
		bucket2->header++;
		newBucket->header++;
		if (bucket2->header == dir->header)	dir->divCount++;

		// entry -> leaf ·ÎÀÇ pointer Á¶Àý
		n = pow_2(dir->header - bucket2->header + 1);      // ³ª´©¾îÁú entry ¼ö
		ind = PrNReturn(PrMakePseudoKey(key), bucket2->header - 1) * n;   // ³ª´©¾îÁú entryÁß Ã¹¹øÂ° entryÀÇ index

		for (i = 0; i<n / 2; i++, ind++)        // µÎ°³ÀÇ bucketÀ¸·Î ³ª´®
			dir->entry[ind] = bucket2;
		for (i = 0; i<n / 2; i++, ind++)
			dir->entry[ind] = newBucket;

		// ´Ù½Ã »ðÀÔµÉ ³ëµå¸¦ ÀúÀåÇÑ´Ù.
		index = PrNReturn(pseudokey, dir->header);
		bucket2 = dir->entry[index];
		if (bucket2->count < BUCKET_SIZE2) { // ÀúÀåÇÒ °÷ÀÇ bucketÀÌ fullÀÌ ¾Æ´Ï¸é ÀúÀå
			bucket2->pRecord[bucket2->count] = rec;
			bucket2->count++;
			return 1;   // Á¤»óÁ¾·á
		}
		// ÀúÀåÇÒ °÷ÀÌ fullÀÌ ¾Æ´Ï¸é 2ÀÇ Ã³À½À¸·Î µ¹¾Æ°¡¼­ ´Ù½Ã directory¸¦ ´Ã¸°´Ù.
	}
}

///////////////////////////////////////////////////////////////////////////////
// DYNAMIC HASHING FUNCTIONS
// Print DB record
// 1. StPrintDB, PrPrintDB -> ofstreamÀ¸·Î DB Ãâ·Â
// 2. StPrintTable, PrPrintTable -> ÆÄÀÏ·Î hash table Ãâ·Â(Students.hash, Professors.hash) 
// 3. StGetToken, PrGetToken -> ÆÄÀÏÀ» ÀÐ¾î¿Í¼­ ´Ü¾îº°·Î ºÐ¸® ÇÏ±âÀ§ÇÑ ÇÔ¼ö
///////////////////////////////////////////////////////////////////////////////

void StPrintDB(StDirectory* dir, ofstream& os) {
	int i, j;
	StLeaf* cBucket;
	StLeaf* pBucket = NULL;
	for (i = 0; i<pow_2(dir->header); i++) {
		int c = i;
		os << c;
		cBucket = dir->entry[i];
		if (cBucket != pBucket) {
			os << "\t" << cBucket->count << "\t" << endl;
			for (j = 0; j<cBucket->count; j++) {
				os << cBucket->pRecord[j]->getName() << ", ";
				os << cBucket->pRecord[j]->getSID() << ", ";
				os << cBucket->pRecord[j]->getScore() << ", ";
				os << cBucket->pRecord[j]->getAID() << endl;
			}
		}
		else { os << "\t" << "P" << "\t" << endl; }
		pBucket = cBucket;
	}
}

void StprintBinaryDB(StDirectory* dir) {
	FILE *fp;
	fp = fopen("Students.DB", "wb");

	int i, j;
	int tempInt;
	char* tempName;
	float tempFloat;

	StLeaf* cBucket;
	StLeaf* pBucket = NULL;
	for (i = 0; i<pow_2(dir->header); i++) {
		int c = i;
		fwrite(&c, sizeof(int), 1, fp);
		cBucket = dir->entry[i];
		if (cBucket != pBucket) {
			tempInt = cBucket->count;
			fwrite(&tempInt, sizeof(int), 1, fp);
			for (j = 0; j < cBucket->count; j++) {
				tempName = cBucket->pRecord[j]->getName();
				fwrite(&tempName, 20, 1, fp);

				tempInt = cBucket->pRecord[j]->getSID();
				fwrite(&tempInt, sizeof(int), 1, fp);

				tempFloat = cBucket->pRecord[j]->getScore();
				fwrite(&tempFloat, sizeof(float), 1, fp);

				tempInt = cBucket->pRecord[j]->getAID();
				fwrite(&tempInt, sizeof(int), 1, fp);
			}
		}
		else {
			tempInt = 300;
			fwrite(&tempInt, sizeof(int), 1, fp);
		}
		pBucket = cBucket;
	}
}

void PrPrintDB(PrDirectory* dir, ofstream& os) {
	int i, j;
	PrLeaf* cBucket;
	PrLeaf* pBucket = NULL;
	for (i = 0; i < pow_2(dir->header); i++) {
		int c = i;
		os << c;
		cBucket = dir->entry[i];
		if (cBucket != pBucket) {
			os << "\t" << cBucket->count << "\t" << endl;
			for (j = 0; j<cBucket->count; j++) {
				os << cBucket->pRecord[j]->getPName() << ", ";
				os << cBucket->pRecord[j]->getPID() << ", ";
				os << cBucket->pRecord[j]->getSalary() << endl;;
			}
		}
		else { os << "\t" << "P" << "\t" << endl; }
		pBucket = cBucket;
	}
}

void PrprintBinaryDB(PrDirectory* dir) {
	FILE *fp;
	fp = fopen("Professors.DB", "wb");

	int i, j;
	int tempInt;
	char* tempName;

	PrLeaf* cBucket;
	PrLeaf* pBucket = NULL;
	for (i = 0; i < pow_2(dir->header); i++) {
		int c = i;
		fwrite(&c, sizeof(int), 1, fp);
		cBucket = dir->entry[i];
		if (cBucket != pBucket) {
			tempInt = cBucket->count;
			fwrite(&tempInt, sizeof(int), 1, fp);
			for (j = 0; j < cBucket->count; j++) {
				tempName = cBucket->pRecord[j]->getPName();
				fwrite(&tempName, 20, 1, fp);

				tempInt = cBucket->pRecord[j]->getPID();
				fwrite(&tempInt, sizeof(int), 1, fp);


				tempInt = cBucket->pRecord[j]->getSalary();
				fwrite(&tempInt, sizeof(int), 1, fp);
			}
		}
		else {
			tempInt = 300;
			fwrite(&tempInt, sizeof(int), 1, fp);
		}
		pBucket = cBucket;
	}
}

void StPrintTable(vector<StElement> origin) {
	FILE *fp;
	fp = fopen("Students.hash", "wb");
	int getInt;
	int getPseudo;
	for (int i = 0; i < LIST_SIZE; i++) {
		StElement* rec = &origin[i];
		getInt = rec->getSID();
		fwrite(&getInt, sizeof(int), 1, fp);
		fwrite(" ", sizeof(" "), 1, fp);
		getPseudo = StMakePseudoKey(rec->getSID());
		fwrite(&getPseudo, sizeof(int), 1, fp);
		fwrite("\n", sizeof("\n"), 1, fp);
	}
	fclose(fp);
}

void PrPrintTable(vector<PrElement> origin) {
	FILE *fp;
	fp = fopen("Professors.hash", "wb");
	int getInt;
	int getPseudo;
	for (int i = 0; i < LIST_SIZE2; i++) {
		PrElement* rec = &origin[i];
		getInt = rec->getPID();
		fwrite(&getInt, sizeof(int), 1, fp);
		fwrite(" ", sizeof(" "), 1, fp);
		getPseudo = PrMakePseudoKey(rec->getPID());
		fwrite(&getPseudo, sizeof(int), 1, fp);
		fwrite("\n", sizeof("\n"), 1, fp);
	}
	fclose(fp);
}

StElement StGetToken(char* convStr) {
	StElement bufElement;
	char* tokList;

	tokList = strtok(convStr, ",");
	bufElement.setName(tokList);

	tokList = strtok(NULL, ",");
	bufElement.setSID(atoi(tokList));

	tokList = strtok(NULL, ",");
	bufElement.setScore(atof(tokList));

	tokList = strtok(NULL, ",");
	bufElement.setAID(atoi(tokList));

	return bufElement;
}
PrElement PrGetToken(char* convStr) {
	PrElement bufElement;
	char* tokList;

	tokList = strtok(convStr, ",");
	bufElement.setPName(tokList);

	tokList = strtok(NULL, ",");
	bufElement.setPID(atoi(tokList));

	tokList = strtok(NULL, ",");
	bufElement.setSalary(atof(tokList));

	return bufElement;
}



int* search(double k) {//key°ªÀÌ ¾îµð¿¡ À§Ä¡ÇØ¾ßÇÏ´ÂÁö Ã£´Â´Ù
	node* p = root;
	int path;
	if (p == NULL)
		return NULL;
	while (1) {
		int j;
		for (j = 0; j < p->count%M; j++) {
			if (p->score[j] >= k)
			{
				path = j;
				break;
			}
		}
		if (j == p->count%M) path = p->count%M;
		if (p->count / M != 1)findcnt++;
		if (p->count / M == 1) break;
		p = p->branch[j];
	}
	if (p->score[path] == k&& p->count%M != path) {
		return (int*)p->branch[path + 1];
	}
	else return NULL;
}

void sequencialSearch(StDirectory* StDir, float k1, float k2, ofstream &of)
{
	int path, j;
	node* p = root;
	findcnt = 0;

	if (p != NULL)
	{
		while (1)   // p°¡ leaf³ëµå ÀÏ¶§±îÁö Å½»ö
		{
			int j;
			for (j = 0; j < p->count%M; j++)   // ÇÑ ³ëµå¿¡¼­ °æ·Î¸¦ °áÁ¤
			{
				if (p->score[j] >= k1)
				{
					path = j;
					break;
				}
			}
			if (j == p->count%M)
				path = p->count%M;
			if (p->count / M == 1)
				break;
			p = p->branch[j];
		}
		if (p->score[path] >= k1)   // k°¡ 0ÀÌ¸é minimumºÎÅÍ ´Ù Ãâ·ÂÇÑ´Ù.
		{
			int Qnum = 0;
			int que = 0;
			while (p != NULL)
			{
				for (j = 0; j < p->count%M; j++)
				{
					if (p->score[j] <= k2) {
						StExactQuery(StDir, p->sid[j], of);
						Qnum++;
					}
					else {
						of << Qnum << endl;
						que = 1;
						break;
					}
				}
				if (que == 1)break;
				p = p->branch[0];
			}
		}
	}
}

int* insertItem(double score, int sid, int blocknum)
{
	node* trace[TR];// »ðÀÔµÉ °æ·Î¸¦ ÀúÀåÇÒ ½ºÅÃ¿ëµµÀÇ ¹è¿­
	int dir[TR];
	int i;
	int Block;
	double Key;
	int Sid;

	node* upRight, *p;
	int* insertFileLocation = (int*)malloc(sizeof(int));
	i = 0; // trace[]ÀÇ index
	p = root;//p¸¦ °¡Áö°í »ðÀÔµÉ À§Ä¡¸¦ Å½»ö

			 //*(int*)upRight = k;
	if (score <= 0)
	{
		printf("key error");
		return NULL;
	}

	if (root == NULL) {
		root = (node*)malloc(sizeof(node));
		root->branch[0] = NULL;
		root->score[0] = score;
		root->sid[0] = sid;
		root->p[0] = blocknum;
		root->count = M + 1;
		return insertFileLocation;
	}

	while (1) {
		int j;
		trace[i] = p;
		for (j = 0; j<p->count%M; j++)
			if (p->score[j] >= score)
			{
				dir[i] = j;
				break;
			}
		if (j == p->count%M) dir[i] = p->count%M;
		if (p->count / M == 1) break;
		p = p->branch[j];
		i++;
	}//ÀÌ ·çÇÁ¿¡¼­ ³ª¿À¸é p´Â key°ªÀÌ »ðÀÔµÉ ³ëµå

	 //º»°Ý »ðÀÔ
	Key = score;
	Sid = sid;
	Block = blocknum;
	while (i != -1) {
		int path = dir[i];
		p = trace[i];
		if (p->count%M != M - 1)//»ðÀÔÇØµµ overflow»ý±âÁö ¾ÊÀ½
		{
			int m;
			for (m = p->count%M; m > path; m--)//»ðÀÔµÉ Ä­ºÎÅÍ ³¡±îÁö ÇÑÄ­¾¿ BACK
			{
				p->score[m] = p->score[m - 1];
				p->sid[m] = p->sid[m - 1];
				p->p[m] = p->p[m - 1];
				p->branch[m + 1] = p->branch[m];
			}
			p->score[path] = Key;//key »ðÀÔ
			p->sid[path] = Sid;
			p->p[path] = Block;
			p->branch[path + 1] = upRight; // branch °ü¸®
			p->count++;
			break;
		}
		else //»ðÀÔ½Ã overflow ¹ß»ý
		{
			double nodeKey[M];
			int nodeSid[M];
			int nodep[M];
			node* nodeBranch[M + 1];
			node* newNode;
			int j, j2;
			newNode = (node*)malloc(sizeof(node));

			nodeBranch[0] = p->branch[0];
			for (j = 0, j2 = 0; j < M; j++, j2++)//ÀÓ½Ã·Î Å©±â M+1ÀÎ ³ëµå¿¡ ¼ø¼­´ë·Î º¹»ç
			{
				if (j == path)
				{
					nodeKey[j] = Key;
					nodeSid[j] = Sid;
					nodep[j] = Block;
					nodeBranch[j + 1] = upRight;
					j++;
					if (j >= M) break;
				}
				nodeKey[j] = p->score[j2];
				nodeSid[j] = p->sid[j2];
				nodep[j] = p->p[j2];
				nodeBranch[j + 1] = p->branch[j2 + 1];
			}
			for (j = 0; j < M / 2; j++)
			{
				p->score[j] = nodeKey[j];
				p->sid[j] = nodeSid[j];
				p->p[j] = nodep[j];
				p->branch[j + 1] = nodeBranch[j + 1];
			}
			newNode->branch[0] = nodeBranch[M / 2 + 1];
			for (j = 0; j < M / 2; j++)//°¡¿îµ¥ key ´ÙÀ½ºÎÅÍ´Â »õ·Î»ý±ä ³ëµå¿¡ º¹»ç
			{
				newNode->score[j] = nodeKey[M / 2 + 1 + j];
				newNode->sid[j] = nodeSid[M / 2 + 1 + j];
				newNode->p[j] = nodep[M / 2 + 1 + j];
				newNode->branch[j + 1] = nodeBranch[M / 2 + 2 + j];
			}
			//¸¸¾à p°¡ ¸®ÇÁ³ëµåÀÌ¸é ¾à°£ ¼öÁ¤
			if (p->count / M == 1)
			{
				newNode->branch[0] = p->branch[0]; //sequencial pointer°ü¸®
				p->branch[0] = newNode;
				p->score[M / 2] = nodeKey[M / 2]; // ¿Ã¸± key°ªÀ» ¸®ÇÁ³ëµå¿¡µµ ³²±ä´Ù.
				p->sid[M / 2] = nodeSid[M / 2];
				p->p[M / 2] = nodep[M / 2];
				p->branch[M / 2 + 1] = nodeBranch[M / 2 + 1];
				p->count = M + M / 2 + 1;
				newNode->count = M + M / 2;
				leafnum++;
			}
			else
			{
				p->count = newNode->count = M / 2;
				p->branch[0] = nodeBranch[0];
			}
			Key = nodeKey[M / 2];//°¡¿îµ¥ Å°¸¦ ¿Ã¸®´Â key·Î
			Sid = nodeSid[M / 2];
			upRight = newNode;//»õ·Î ¸¸µç node¸¦ ¿Ã¸®´Â °ªÀÇ ¿À¸¥ÂÊ ÀÚ½ÄÀ¸·Î
		}
		i--;
	}
	if (i == -1)//root¿¡¼­ overflow°¡ »ý±ä°æ¿ì
	{
		root = (node*)malloc(sizeof(node));
		root->count = 1;
		root->branch[0] = trace[0];
		root->branch[1] = upRight;
		root->score[0] = Key;
		root->sid[0] = Sid;
		root->p[0] = Block;
	}
	return insertFileLocation;
}

node* bput(node* k)
{
	queue[rear] = k;
	rear = ++rear % MAX;
	return k;
}

node* get()
{
	node* i;
	if (front == rear)
	{
		printf("\n Queue underflow.");
		return NULL;
	}
	i = queue[front];
	front = ++front % MAX;
	return i;
}

int isEmpty()
{
	return (front == rear);
}

int isEmpty2()
{
	return (front2 == rear2);
}

void indexNodePrint(node* t)
{
	if (t == NULL)
	{
		printf(" NULLÀÔ´Ï´Ù.");
	}
	else
	{
		ofstream os;
		os.open("Student_score.idx");
		bput(t);
		while (!isEmpty())
		{
			int i;
			t = get();
			if (t->count / M != 1) {
				//printf("(");
				for (i = 0; i < t->count%M - 1; i++)
				{
					//printf("%.1f, ", t->score[i]);
					os.write((char*)&t->score[i], sizeof(double));
					os.write((char*)&t->branch[i], sizeof(Node*));
				}
				//printf("%.1f)\n ", t->score[t->count%M - 1]);
				os.write((char*)&t->score[t->count%M - 1], sizeof(double));
				os.write((char*)&t->branch[i], sizeof(Node*));
			}
			if (t->count / M != 1)
				for (i = 0; i <= t->count; i++)
					bput(t->branch[i]);
		}
		os.close();
	}
}


int* search2(int k) {//key°ªÀÌ ¾îµð¿¡ À§Ä¡ÇØ¾ßÇÏ´ÂÁö Ã£´Â´Ù
	node2* p2 = root2;
	int path;
	if (p2 == NULL)
		return NULL;
	while (1) {
		int j;
		for (j = 0; j < p2->count2%M; j++) {
			if (p2->score2[j] >= k)
			{
				path = j;
				break;
			}
		}
		if (j == p2->count2%M) path = p2->count2%M;
		if (p2->count2 / M != 1)findcnt2++;
		if (p2->count2 / M == 1) break;
		p2 = p2->branch2[j];
	}
	if (p2->score2[path] == k&& p2->count2%M != path) {
		return (int*)p2->branch2[path + 1];
	}
	else return NULL;
}

void sequencialSearch2(PrDirectory* PrDir, int k1, int k2, ofstream& of)
{
	int path, j;
	node2* p2 = root2;
	findcnt2 = 0;

	if (p2 != NULL)
	{
		while (1)   // p°¡ leaf³ëµå ÀÏ¶§±îÁö Å½»ö
		{
			int j;
			for (j = 0; j < p2->count2%M; j++)   // ÇÑ ³ëµå¿¡¼­ °æ·Î¸¦ °áÁ¤
			{
				if (p2->score2[j] >= k1)
				{
					path = j;
					break;
				}
			}
			if (j == p2->count2%M)
				path = p2->count2%M;
			if (p2->count2 / M == 1)
				break;
			p2 = p2->branch2[j];
		}
		if (p2->score2[path] >= k1)   // k°¡ 0ÀÌ¸é minimumºÎÅÍ ´Ù Ãâ·ÂÇÑ´Ù.
		{
			int Qnum = 0;
			int que = 0;
			while (p2 != NULL)
			{
				for (j = 0; j < p2->count2%M; j++)
				{
					if (p2->score2[j] <= k2) {
						PrExactQuery(PrDir, p2->sid2[j], of);
						Qnum++;
					}
					else {
						of << Qnum << endl;
						que = 1;
						break;
					}
				}
				if (que == 1)break;
				p2 = p2->branch2[0];
			}
		}
	}
}

int* insertItem2(int score, int sid, int blocknum)
{
	node2* trace2[TR];// »ðÀÔµÉ °æ·Î¸¦ ÀúÀåÇÒ ½ºÅÃ¿ëµµÀÇ ¹è¿­
	int dir2[TR];
	int i;
	int Block;
	double Key;
	int Sid;

	node2* upRight2, *p2;
	int* insertFileLocation2 = (int*)malloc(sizeof(int));
	i = 0; // trace[]ÀÇ index
	p2 = root2;//p¸¦ °¡Áö°í »ðÀÔµÉ À§Ä¡¸¦ Å½»ö

			   //*(int*)upRight = k;
	if (score <= 0)
	{
		printf("key error");
		return NULL;
	}

	if (root2 == NULL) {
		root2 = (node2*)malloc(sizeof(node2));
		root2->branch2[0] = NULL;
		root2->score2[0] = score;
		root2->sid2[0] = sid;
		root2->p2[0] = blocknum;
		root2->count2 = M + 1;
		return insertFileLocation2;
	}

	while (1) {
		int j;
		trace2[i] = p2;
		for (j = 0; j<p2->count2%M; j++)
			if (p2->score2[j] >= score)
			{
				dir2[i] = j;
				break;
			}
		if (j == p2->count2%M) dir2[i] = p2->count2%M;
		if (p2->count2 / M == 1) break;
		p2 = p2->branch2[j];
		i++;
	}//ÀÌ ·çÇÁ¿¡¼­ ³ª¿À¸é p´Â key°ªÀÌ »ðÀÔµÉ ³ëµå

	 //º»°Ý »ðÀÔ
	Key = score;
	Sid = sid;
	Block = blocknum;
	while (i != -1) {
		int path2 = dir2[i];
		p2 = trace2[i];
		if (p2->count2%M != M - 1)//»ðÀÔÇØµµ overflow»ý±âÁö ¾ÊÀ½
		{
			int m;
			for (m = p2->count2%M; m > path2; m--)//»ðÀÔµÉ Ä­ºÎÅÍ ³¡±îÁö ÇÑÄ­¾¿ BACK
			{
				p2->score2[m] = p2->score2[m - 1];
				p2->sid2[m] = p2->sid2[m - 1];
				p2->p2[m] = p2->p2[m - 1];
				p2->branch2[m + 1] = p2->branch2[m];
			}
			p2->score2[path2] = Key;//key »ðÀÔ
			p2->sid2[path2] = Sid;
			p2->p2[path2] = Block;
			p2->branch2[path2 + 1] = upRight2; // branch °ü¸®
			p2->count2++;
			break;
		}
		else //»ðÀÔ½Ã overflow ¹ß»ý
		{
			double nodeKey[M];
			int nodeSid[M];
			int nodep[M];
			node2* nodeBranch[M + 1];
			node2* newNode;
			int j, j2;
			newNode = (node2*)malloc(sizeof(node2));

			nodeBranch[0] = p2->branch2[0];
			for (j = 0, j2 = 0; j < M; j++, j2++)//ÀÓ½Ã·Î Å©±â M+1ÀÎ ³ëµå¿¡ ¼ø¼­´ë·Î º¹»ç
			{
				if (j == path2)
				{
					nodeKey[j] = Key;
					nodeSid[j] = Sid;
					nodep[j] = Block;
					nodeBranch[j + 1] = upRight2;
					j++;
					if (j >= M) break;
				}
				nodeKey[j] = p2->score2[j2];
				nodeSid[j] = p2->sid2[j2];
				nodep[j] = p2->p2[j2];
				nodeBranch[j + 1] = p2->branch2[j2 + 1];
			}
			for (j = 0; j < M / 2; j++)
			{
				p2->score2[j] = nodeKey[j];
				p2->sid2[j] = nodeSid[j];
				p2->p2[j] = nodep[j];
				p2->branch2[j + 1] = nodeBranch[j + 1];
			}
			newNode->branch2[0] = nodeBranch[M / 2 + 1];
			for (j = 0; j < M / 2; j++)//°¡¿îµ¥ key ´ÙÀ½ºÎÅÍ´Â »õ·Î»ý±ä ³ëµå¿¡ º¹»ç
			{
				newNode->score2[j] = nodeKey[M / 2 + 1 + j];
				newNode->sid2[j] = nodeSid[M / 2 + 1 + j];
				newNode->p2[j] = nodep[M / 2 + 1 + j];
				newNode->branch2[j + 1] = nodeBranch[M / 2 + 2 + j];
			}
			//¸¸¾à p°¡ ¸®ÇÁ³ëµåÀÌ¸é ¾à°£ ¼öÁ¤
			if (p2->count2 / M == 1)
			{
				newNode->branch2[0] = p2->branch2[0]; //sequencial pointer°ü¸®
				p2->branch2[0] = newNode;
				p2->score2[M / 2] = nodeKey[M / 2]; // ¿Ã¸± key°ªÀ» ¸®ÇÁ³ëµå¿¡µµ ³²±ä´Ù.
				p2->sid2[M / 2] = nodeSid[M / 2];
				p2->p2[M / 2] = nodep[M / 2];
				p2->branch2[M / 2 + 1] = nodeBranch[M / 2 + 1];
				p2->count2 = M + M / 2 + 1;
				newNode->count2 = M + M / 2;
				leafnum2++;
			}
			else
			{
				p2->count2 = newNode->count2 = M / 2;
				p2->branch2[0] = nodeBranch[0];
			}
			Key = nodeKey[M / 2];//°¡¿îµ¥ Å°¸¦ ¿Ã¸®´Â key·Î
			Sid = nodeSid[M / 2];
			upRight2 = newNode;//»õ·Î ¸¸µç node¸¦ ¿Ã¸®´Â °ªÀÇ ¿À¸¥ÂÊ ÀÚ½ÄÀ¸·Î
		}
		i--;
	}
	if (i == -1)//root¿¡¼­ overflow°¡ »ý±ä°æ¿ì
	{
		root2 = (node2*)malloc(sizeof(node2));
		root2->count2 = 1;
		root2->branch2[0] = trace2[0];
		root2->branch2[1] = upRight2;
		root2->score2[0] = Key;
		root2->sid2[0] = Sid;
		root2->p2[0] = Block;
	}
	return insertFileLocation2;
}

node2* bput2(node2* k)
{
	queue2[rear2] = k;
	rear2 = ++rear2 % MAX;
	return k;
}

node2* get2()
{
	node2* i;
	if (front2 == rear2)
	{
		printf("\n Queue underflow.");
		return NULL;
	}
	i = queue2[front2];
	front2 = ++front2 % MAX;
	return i;
}


void indexNodePrint2(node2* t)
{
	if (t == NULL)
	{
		printf(" NULLÀÔ´Ï´Ù.");
	}
	else
	{
		ofstream os;
		os.open("Professor_score.idx");
		bput2(t);
		while (!isEmpty2())
		{
			int i;
			t = get2();
			if (t->count2 / M != 1) {
				//printf("(");
				for (i = 0; i < t->count2%M - 1; i++)
				{
					//printf("%d, ", t->score2[i]);
					os.write((char*)&t->score2[i], sizeof(int));
					os.write((char*)&t->branch2[i], sizeof(Node2*));
				}
				//printf("%d)\n ", t->score2[t->count2%M - 1]);
				os.write((char*)&t->score2[t->count2%M - 1], sizeof(int));
				os.write((char*)&t->branch2[i], sizeof(Node2*));
			}
			if (t->count2 / M != 1)
				for (i = 0; i <= t->count2; i++)
					bput2(t->branch2[i]);
		}
		os.close();
	}
}

///////////////////////////////////////////////////////////////////////////////
// DYNAMIC HASHING FUNCTIONS
// 1. student_start -> student_data.csv¸¦ ÀÐ¾î¿Í¼­ Students.DB¿¡ DB ÀúÀå
// 2. professor_start -> prof_data.csv¸¦ ÀÐ¾î¿Í¼­ Professors.DB¿¡ DB ÀúÀå
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////

int main() {
	ifstream mainIf;
	ofstream mainOf;
	int queryNum = 0;

	// Exact Query variable 
	string typeString = "";

	char* tableString;
	char* attrString;

	tableString = new char[100];
	attrString = new char[100];

	char* tokList = "";
	char comma;

	int ExTarget;

	//////////////////////////////////////////
	//////////////////////////////////////////
	ifstream StIS;
	ofstream StOS;
	int StTarget = 0;
	StIS.open("student_data.csv");
	StOS.open("Students.DB");
	if (StIS.is_open() == false) {
		cout << "File open Failed" << endl;
	}

	char StEnter;
	char* StConvStr;
	string StBufStr;
	StElement StBufElement;
	vector<StElement> sData;
	vector<unsigned int> StHashList;

	// get LIST_SIZE
	getline(StIS, StBufStr);
	StConvStr = new char[sizeof(StBufStr)];

	strcpy(StConvStr, StBufStr.c_str());
	StConvStr = strtok(StConvStr, ",");
	LIST_SIZE = atoi(StConvStr);

	for (int i = 0; i < LIST_SIZE; i++) {
		getline(StIS, StBufStr);
		strcpy(StConvStr, StBufStr.c_str());
		StBufElement = StGetToken(StConvStr);
		sData.push_back(StBufElement);
	}
	StGetBlockSize(sData);

	StDirectory* StDir;
	StDir = (StDirectory*)malloc(sizeof(StDirectory));
	StDir->header = MIN_ORDER;
	StDir->divCount = pow_2(MIN_ORDER - 1);
	StDir->entry = (StLeaf**)malloc(sizeof(StLeaf*) * pow_2(MIN_ORDER));
	for (int i = 0; i<pow_2(MIN_ORDER); i++) {
		StDir->entry[i] = (StLeaf*)malloc(sizeof(StLeaf));
		StDir->entry[i]->header = 2;
		StDir->entry[i]->count = 0;
		StDir->entry[i]->pRecord = (StElement**)malloc(sizeof(StElement*)*BUCKET_SIZE);
	}

	for (int i = 0; i < LIST_SIZE; i++) {
		StElement* StRec = &sData[i];
		StInsertRecord(StRec, StDir);
		/* printf("\n***  key : %d, name : %s ÀÎ record¸¦ ÀúÀå, pseudokey : %d ***\n",
		rec->getSID(), rec->getName(), makePseudokey(rec->getSID())); */
	}
	StprintBinaryDB(StDir);
	StPrintTable(sData);

	///////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	ifstream PrIS;
	ofstream PrOS;
	int PrTarget = 0;
	PrIS.open("prof_data.csv");
	PrOS.open("Professors.DB");
	if (PrIS.is_open() == false) {
		cout << "File open Failed" << endl;
	}

	char PrEnter;
	char* PrConvStr;
	string PrBufStr;
	PrElement PrBufElement;
	vector<PrElement> pData;
	vector<unsigned int> PrHashList;

	// get LIST_SIZE
	getline(PrIS, PrBufStr);
	PrConvStr = new char[sizeof(PrBufStr)];

	strcpy(PrConvStr, PrBufStr.c_str());
	PrConvStr = strtok(PrConvStr, ",");
	LIST_SIZE2 = atoi(PrConvStr);

	for (int i = 0; i < LIST_SIZE2; i++) {
		getline(PrIS, PrBufStr);
		strcpy(PrConvStr, PrBufStr.c_str());
		PrBufElement = PrGetToken(PrConvStr);
		pData.push_back(PrBufElement);
	}
	PrGetBlockSize(pData);

	PrDirectory* PrDir;
	PrDir = (PrDirectory*)malloc(sizeof(PrDirectory));
	PrDir->header = MIN_ORDER;
	PrDir->divCount = pow_2(MIN_ORDER - 1);
	PrDir->entry = (PrLeaf**)malloc(sizeof(PrLeaf*) * pow_2(MIN_ORDER));
	for (int i = 0; i<pow_2(MIN_ORDER); i++) {
		PrDir->entry[i] = (PrLeaf*)malloc(sizeof(PrLeaf));
		PrDir->entry[i]->header = 2;
		PrDir->entry[i]->count = 0;
		PrDir->entry[i]->pRecord = (PrElement**)malloc(sizeof(PrElement*)*BUCKET_SIZE2);
	}

	for (int i = 0; i < LIST_SIZE2; i++) {
		PrElement* PrRec = &pData[i];
		PrInsertRecord(PrRec, PrDir);
	}
	PrprintBinaryDB(PrDir);
	PrPrintTable(pData);
	///////////////////////////////////////////
	////////ÇÐ»ý B+Æ®¸® »ý¼º////////////////////
	root = NULL; //root³ëµå ÃÊ±âÈ­
	ifstream fi;
	float k1, k2;
	int num;
	int bnum;
	int sid1;
	int aid1;
	float score1;
	char name1[20];
	fi.open("Students.DB", ios::binary);
	string bufstr;
	while (!fi.eof()) {
		fi.read((char*)&num, sizeof(int));
		fi.read((char*)&bnum, sizeof(int));
		if (bnum != 300) {
			int h = bnum;
			for (int i = 0; i < h; i++) {
				fi.read((char*)&name1, sizeof(name1));
				fi.read((char*)&sid1, sizeof(int));
				fi.read((char*)&score1, sizeof(float));
				fi.read((char*)&aid1, sizeof(int));
				insertItem(score1, sid1, num);
			}
		}
		else continue;
	}
	indexNodePrint(root);

	fi.close();
	////////////////////////////////////////////
	////////±³¼ö B+Æ®¸® »ý¼º////////////////////
	root2 = NULL; //root³ëµå ÃÊ±âÈ­
	int num2;
	int k3, k4;
	int bnum2;
	int sid2;
	int aid2;
	char name2[20];
	fi.open("Professors.DB", ios::binary);
	while (!fi.eof()) {
		fi.read((char*)&num2, sizeof(int));
		fi.read((char*)&bnum2, sizeof(int));
		if (bnum != 300) {
			int h2 = bnum2;
			for (int i = 0; i < h2; i++) {
				fi.read((char*)&name2, sizeof(name2));
				fi.read((char*)&sid2, sizeof(int));
				fi.read((char*)&aid2, sizeof(int));
				insertItem2(aid2, sid2, num2);
			}
		}
		else continue;
	}
	indexNodePrint2(root2);

	fi.close();
	/////////////////////////////////////////////////////////////////////////////
	//Query Ã³¸®/////////////////////////////////////////////////////////////////

	char* myConvStr;
	char* myToken = " ";
	string myStr;

	myConvStr = new char[100];

	mainIf.open("query.dat");
	mainOf.open("query.res");
	mainIf >> queryNum;
	for (int i = 0; i < queryNum; i++) {
		mainIf >> myStr;
		strcpy(myConvStr, myStr.c_str());
		myToken = strtok(myConvStr, ",");
		if (!strcmp(myToken, "Search-Exact")) {
			myToken = strtok(NULL, ",");
			if (!strcmp(myToken, "Students")) {
				attrString = strtok(NULL, ",");
				ExTarget = atoi(strtok(NULL, ","));
				StExactQuery(StDir, ExTarget, mainOf);
			}
			else if (!strcmp(myToken, "Professors")) {
				attrString = strtok(NULL, ",");
				ExTarget = atoi(strtok(NULL, ","));
				PrExactQuery(PrDir, ExTarget, mainOf);
			}
			else {
				cout << "Àß¸øµÈ Table ¿äÃ»ÀÔ´Ï´Ù" << endl;
				return 0;
			}
		}
		else if (!strcmp(myToken, "Search-Range")) {
			myToken = strtok(NULL, ",");
			if (!strcmp(myToken, "Students")) {
				myToken = strtok(NULL, ",");
				float min = atof(strtok(NULL, ","));
				float max = atof(strtok(NULL, ","));
				sequencialSearch(StDir, min, max, mainOf);
			}
			else if (!strcmp(myToken, "Professors")) {
				myToken = strtok(NULL, ",");
				int min = atoi(strtok(NULL, ","));
				int max = atoi(strtok(NULL, ","));
				sequencialSearch2(PrDir, min, max, mainOf);
			}
		}
		else if (!strcmp(myToken, "Join")) {
			for (int i = 0; i < (StDir->divCount * 2); i++) {	// Bucket address table size
				for (int j = 0; j < (StDir->entry[i]->count); j++) { // Bucket table
					mainOf << StDir->entry[i]->pRecord[j]->getName() << " " << StDir->entry[i]->pRecord[j]->getSID()
						<< " " << StDir->entry[i]->pRecord[j]->getScore() << " ";
					PrExactQuery(PrDir, StDir->entry[i]->pRecord[j]->getAID(), mainOf);
				}
			}
		}
		else {
			cout << "잘못된 Query 요청입니다" << endl;
			return 0;
		}
	}

	StOS.close();
	StIS.close();
	PrOS.close();
	PrIS.close();
}
