#include<iostream>
#include<fstream>
#include<ctime>
#include<vector>
#include<queue>
#include<string>
#include<sstream>
#include<stdlib.h>
#include<math.h>

#define N 10
#define L 200

using namespace std;

/*
Student name: Li Chaohui
Student number: 55202995
Project code: 20CS071
Project title: A Game Generator for Sliding Puzzles

The game generator for Klotski (complete search)
*/

//Game codes in JavaScript (Vue.js)
ifstream part1("../input/Klotski/part1.txt"), part2("../input/Klotski/part2.txt");
ofstream writeFile("../collection.txt");//text output: generated game codes
stringstream ss;//string stream for formating the text output

int flag = false;//indicator: whether finding the optimal solution
int row, col, map[N][N], iniMap[N][N];//map: 0-space 1-obstacle 2-block
int num;//number of blocks
int levNo = 0;//number of levels (height/depth)
int hGenNum = 1;//number of horizontal generals (1x2 block) in Klotski
int levLimit = L;

int x[4] = { -1,1,0,0 },
	y[4] = { 0,0,-1,1 };//moving direction: up, down, left, right

/*
	Compatible and extensible data structures
*/
//Data structure - Point: position of each cell/unit
struct Point {
	int x, y;
	Point(int a = 0, int b = 0) :x(a), y(b) {};
	bool isEquale(Point p) {//check if two points are equal
		if (x == p.x&&y == p.y) return true;
		return false;
	}
	int getKey() {//get key for hashing
		return (x - 1) * col + y;
	}
};

Point des;//destination of the main block (the 2x2 block in Klotski)

//Data structure - Block: any shape of block/tile (composed of cells/units)
class Block {
public:
	Block() {
		p = Point();
		r = c = 0;
	}
	Block(Point p, int a, int b, int t) {
		this->p = p;
		r = a, c = b, type = t;
	}
	void setPosition(Point p) {//set the position of the block
		this->p = p;
	}
	void setSize(int a, int b) {//set the block size (number of rows & columns)
		r = a, c = b;
	}
	void setShape(int i, int j, int val) {//set the block shape (val: 1 - cell of the block; 0 - blank)
		shape[i][j] = val;
	}
	void printBlock() {//display the block by 0 and 1
		for (int i = 0;i < r;i++) {
			for (int j = 0;j < c;j++) {
				cout << shape[i][j] << " ";
			}
			cout << endl;
		}
	}
	int getRow() {//get the number of rows
		return r;
	}
	int getCol() {//get the number of columns
		return c;
	}
	int getType() {//get the block type 
		return type;
	}
	int getCell(int i, int j) {//check if one cell belongs to the block (or a blank)
		if (i < 0 || i >= r || j < 0 || j >= c) return -1;//invalid query: out of block boundaries
		return shape[i][j];
	}
	Point getPosition() {//get the position of the block
		return p;
	}
	bool isOverlap(Block* block) {//check if two blocks are overlapping
		Point point = block->getPosition();
		for (int i = 0;i < r;i++) {
			for (int j = 0;j < c;j++) {
				if (shape[i][j] == 1
					&& block->getCell(i + p.x - point.x, j + p.y - point.y) == 1) return true;
			}
		}
		return false;
	}
private:
	Point p;//block position: the position of the top-left cell
	int r, c;//the numbers of rows & columns
	int type;//4 types in Klotski: 1 - 1x1; 2 - 1x2; 3 - 2x1; 4 - 2x2
	int shape[N][N];//block shape
};

vector<Block*> blocks;//Store all blocks on the gameboard: only the positions keep changing

//Data structure - State: a configuration or arrangement of all blocks
class State {
public:
	void setStep(int i) {//set the step (number of small steps to the parent)
		step = i;
	}
	void setLevel(int i) {//set the level number
		level = i;
	}
	void setParent(State* p) {//set the parent state for backtracking
		parent = p;
	}
	void setMovedBlockIdx(int b) {//set the index of the moved block (compared with the parent state)
		moved = b;
	}
	void setDir(int d) {//set the moving direction
		direction = d;
	}
	void addPosition(Point p) {//set the positions of all blocks in this state/configuration
		points.push_back(p);
	}
	int getStep() {//get the step
		return step;
	}
	int getLevel() {//get the level
		return level;
	}
	int getDir() {//get the direction
		return direction;
	}
	int getBlock() {//get the moved block index
		return moved;
	}
	vector<Point> getPositions() {//get all the block positions
		return points;
	}
	State* getParent() {//get the parent state
		return parent;
	}
	void setGrid() {//set the grid in terms of block types (for pruning)
		//copy the gameboard
		for (int i = 0;i < row;i++)
			for (int j = 0;j < col;j++)
				grid[i][j] = map[i][j];
		//copy blocks
		for (int n = 0;n < num;n++) {
			int r = points[n].x - 1, c = points[n].y - 1;
			Block* b = blocks[n];
			for (int i = 0;i < b->getRow();i++) {
				for (int j = 0;j < b->getCol();j++) {
					if (grid[r + i][c + j] == 0)//obstacles cannot be overlapped
						grid[r + i][c + j] = b->getType();
				}
			}
		}
	}
	int getGrid(int i, int j) {//get the grid
		//if (i < 0 || i >= row || j < 0 || j >= col) return -1;
		return grid[i][j];
	}
	bool isEquivalent(State* s) {//check if two states are the same (with pruning)
		for (int i = 0;i < row;i++) {
			for (int j = 0;j < col;j++) {
				if (grid[i][j] != s->getGrid(i, j)) return false;
			}
		}
		return true;
	}

private:
	int step;//number of steps to the parent state
	int level;//levels to the root (the least number of steps)
	State* parent;//parent state
	int moved;//index of the moved block
	int direction;//moving direction
	vector<Point> points;//positions of all blocks
	int grid[N][N];//grid of types for pruning
};


State* first = new State();//the starting nodes (all the end configurations in Klotski)

//Print out a configuration on console
void printBoard(State* s) {
	int output[N][N];
	//copy map
	for (int i = 0;i < row;i++)
		for (int j = 0;j < col;j++)
			output[i][j] = map[i][j];
	vector<Point> points = s->getPositions();
	//copy blocks
	for (int n = 0;n < num;n++) {
		int r = points[n].x - 1, c = points[n].y - 1;
		Block* b = blocks[n];
		for (int i = 0;i < b->getRow();i++) {
			for (int j = 0;j < b->getCol();j++) {
				if (output[r + i][c + j] == 0)//obstacles cannot be overlapped
					output[r + i][c + j] = b->getCell(i, j) == 1 ? n + 2 : 0;
			}
		}
	}
	for (int i = 0;i < row;i++) {
		for (int j = 0;j < col;j++) {
			string str;
			switch (output[i][j])
			{
			case 0: str = "0";break;//blank
			case 1: str = "X";break;//the 10th block
			default: str = (output[i][j] - 1) + 48;break;
			}
			cout << str << " ";
		}
		cout << endl;
	}
	cout << endl;
}

//Record the expanded states in each level
vector< vector<State*> > level;
void initLevel() {
	for (int i = 0;i <= levLimit;i++) {
		vector<State*> temp;
		temp.clear();
		level.push_back(temp);
	}
}

/*
	Definition of hash table and hash operations
*/

//Hash table
int m;//size of the hash table
vector< vector<State*> > hashTable;
void initHashTable() {
	m = 18013;//the prime number can distribute the hash values more evenly
	for (int i = 0;i < m;i++) {
		vector<State*> temp;
		temp.clear();
		hashTable.push_back(temp);
	}
}

//Hash function
int hashKey[20] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181 };//Fabonacci Sequence
int Hash1(State* s) {//the primary hash function
	int res = 0;
	for (int i = 0;i < row;i++)
		for (int j = 0;j < col;j++) res ^= hashKey[i*col + j] * (s->getGrid(i, j));
	return res % m;
}
int Hash2(State* s) {//the supplementary hash function for pruning the symmetric configurations
	int res = 0;
	for (int i = 0;i < row;i++)
		for (int j = col - 1;j >= 0;j++) res ^= hashKey[i*col + j] * (s->getGrid(i, j));
	return res % m;
}

void insert(State* s) {//insert the state into the hash table
	int value = Hash1(s);
	State* temp;
	hashTable[value].push_back(s);
}

bool search(State* s) {//check if the state is in the hash table
	int value = Hash1(s);
	int num = hashTable[value].size();
	for (int i = 0;i < num;i++) {//check if the state has been visited if collision happens
		if (s->isEquivalent(hashTable[value][i])) return true;
	}
	// value = Hash2(s);
	// num = hashTable[value].size();
	// for (int i = 0;i < num;i++) {
	// 	if (s->isEquivalent(hashTable[value][i])) return true;
	// }
	return false;
}


/*
	Find all end configurations (assume the 2x2 block is at the position (4,2))
*/

//1. Find all permutations of blocks except the main block (1: 1x1 block, 2: 1x2 block, 3: 2x1 block)
vector<vector<int>> per;
bool isSwap(int arr[], int index, int len) {//ignore the repetitive numbers
	for (int i = index + 1;i < len;i++)
		if (arr[index] == arr[i]) return false;
	return true;
}

void swap(int arr[], int a, int b) {//swap arr[a] and arr[b]
	int temp = arr[a];
	arr[a] = arr[b];
	arr[b] = temp;
}

void permutations(int arr[], int index, int len) {
	if (index == len) {//find a permutation
		vector<int> list(arr, arr + len);
		per.push_back(list);
	}
	else {
		for (int i = index;i < len;i++) {
			if (isSwap(arr, i, len)) {
				swap(arr, index, i);
				permutations(arr, index + 1, len);//find permutations of trailing numbers
				swap(arr, index, i);
			}
		}
	}
}

//2. Find all valid configurations for each permutation
Point getBlank(int type, int start) {//get a blank position based on the given block type
	for (int i = 0;i < row;i++) {
		for (int j = 0;j < col;j++) {
			if (i*col + j + 1 > start && iniMap[i][j] == 0) {
				if (type == 1) return Point(i + 1, j + 1);
				else if (j + 1 < col && type == 2 && iniMap[i][j + 1] == 0) return Point(i + 1, j + 1);
				else if (i + 1 < row && type == 3 && iniMap[i + 1][j] == 0) return Point(i + 1, j + 1);
			}
		}
	}
	return Point(0, 0);
}

//Assuming the positions of two blanks are fixed, find a valid end configuration
bool findEndConfig(int arr[], int len, int blank1, int blank2) {
	for (int i = 0;i < row;i++)
		for (int j = 0;j < col;j++)
			iniMap[i][j] = 0;
	//set the main block at the position (4,2)
	for (int i = 0;i < 2;i++) {
		for (int j = 0;j < 2;j++) {
			int key = des.getKey() + i * col + j;
			if (blank1 == key || blank2 == key) return false;
			iniMap[des.x - 1 + i][des.y - 1 + j] = 4;//4: 2x2 block
		}
	}
	//set two blanks
	int c1 = (blank1 - 1) % col, r1 = (blank1 - c1) / col,
		c2 = (blank2 - 1) % col, r2 = (blank2 - c2) / col;
	iniMap[r1][c1] = iniMap[r2][c2] = -1;//cannot be occupied
	//put other blocks on the game board
	int start = 0;//starting position for next block
	int idx, sodlier = 6, generalH = 1, generalV = hGenNum + 1;//index for different blocks
	for (int n = 0;n < len;n++) {
		int type = arr[n];
		Point pos = getBlank(type, start);//get a valid position for the block
		if (pos.x == 0 && pos.y == 0) {
			//cout << "Invalid permutation!" << endl;
			return false;//invalid configuration
		}
		//find the index for the block
		if (type == 1) idx = sodlier++;
		else if (type == 2) idx = generalH++;
		else idx = generalV++;
		//set block
		Block* block = blocks[idx];
		int r = block->getRow(), c = block->getCol();
		for (int i = 0;i < r;i++) {
			for (int j = 0;j < c;j++) {
				iniMap[pos.x - 1 + i][pos.y - 1 + j] = type;
			}
		}
		block->setPosition(pos);
		start = pos.getKey();//update the starting position for next block
	}
	iniMap[r1][c1] = iniMap[r2][c2] = 0;//set the blanks
	return true;//get a valid end configuration
}

void setNewState() {//set a new configuration
	State* newState = new State();
	for (int i = 0;i < num;i++) newState->addPosition(blocks[i]->getPosition());
	newState->setGrid();
	newState->setStep(0), newState->setLevel(0);//the end configuration is the root node
	newState->setParent(NULL), newState->setMovedBlockIdx(-1), newState->setDir(0);
	if (!search(newState)) {//check if the state has been visited
		insert(newState);
		level[0].push_back(newState);//record the state at level 0 (the root level)
	}
}

//Initialize all blocks in order
void setBlocks() {
	row = 5, col = 4;//rows and columns of the gameboard
	blocks.clear();
	num = 10;//number of blocks
	des.x = 4, des.y = 2;//destination
	//set the main block (2x2 block)
	Block* block = new Block(Point(des.x, des.y), 2, 2, 4);//located at the destination (end configurations)
	for (int i = 0;i < 2;i++) {
		for (int j = 0;j < 2;j++) {
			block->setShape(i, j, 1);
		}
	}
	blocks.push_back(block);
	//set other blocks
	Point pos(0, 0);//initial position
	int r, c, type;
	for (int i = 0;i < 9;i++) {
		//set block type
		if (i < hGenNum) r = 1, c = 2, type = 2;//hGenNum: the number of the 1x2 blocks
		else if (i < 5) r = 2, c = 1, type = 3;
		else r = 1, c = 1, type = 1;

		block = new Block(pos, r, c, type);
		for (int i = 0;i < r;i++) {
			for (int j = 0;j < c;j++) {
				block->setShape(i, j, 1);
			}
		}
		blocks.push_back(block);
	}
}

/*
	Generate sliding puzzles by the pivotal algorithm
*/

//Check if a block can move a small step
bool moveBlock(int idx, int dir, int step) {
	Block* b = blocks[idx];
	Point p = b->getPosition();
	int dx = p.x + step * x[dir],
		dy = p.y + step * y[dir];//coordinates after movement
	if (dx < 1 || dy < 1 || dx + b->getRow() - 1 > row || dy + b->getCol() - 1 > col) return false;//out of board
	for (int i = 0;i < b->getRow();i++) {
		for (int j = 0;j < b->getCol();j++) {
			if (map[i + dx - 1][j + dy - 1] == 1 && b->getCell(i, j) == 1) return false;//collide the obstacle
		}
	}
	b->setPosition(Point(dx, dy));//update point of moved block
	for (int i = 0;i < num;i++) {
		if (i == idx) continue;
		if (b->isOverlap(blocks[i])) {
			b->setPosition(p);//restore the previous position if overlap	
			return false;
		}
	}
	return true;
}

//Backtrack and print the route  on the console by recursion
void printRoute(State* s) {
	State* parent = s->getParent();
	if (parent != NULL) {
		printBoard(s);
		string str;
		switch (s->getDir() + 1)
		{
		case 1: str = "down";break;
		case 2: str = "up";break;
		case 3: str = "right";break;
		case 4: str = "left";break;
		default: break;
		}
		//cout << s << " " << parent << endl;
		cout << "\nMove Block" << s->getBlock() + 1 << " " << s->getStep() << " steps " << str << "\n\n";
		printRoute(parent);
	}
	else {
		printBoard(s);
		//cout << "Reach the destination!" << endl;
	}
}

//Get total steps along the path by recursion
int getSteps(State* s) {
	State* parent = s->getParent();
	if (parent != NULL) return s->getStep() + getSteps(parent);
	else return s->getStep();
}

//Multi-source BFS
void multiSourceBFS() {
	clock_t startTime, endTime;
	startTime = clock();//time onset
	int cnt = 0, len = level[0].size();
	State* front = level[0][cnt];//first state
	queue<State*> q;
	for (cnt = 0;cnt < len;cnt++) q.push(level[0][cnt]);//put all the end configurations into the queue
	while (!q.empty() && levNo < levLimit) {
		front = q.front();
		q.pop();
		insert(front);
		for (int n = 0;n < num;n++) blocks[n]->setPosition(front->getPositions()[n]);//update positions of all blocks
		//cout << "Level: " << levNo << endl;
		levNo = front->getLevel() + 1;
		for (int n = 0;n < num;n++) {//traverse all the children (one step)
			blocks[n]->setPosition(front->getPositions()[n]);//set the position of block n in the state
			for (int i = 0;i < 4;i++) {//four moving directions
				int step = 0;
				if (moveBlock(n, i, 1)) step++;//check how many small steps
				if (step > 0) {
					State* child = new State();
					for (int j = 0;j < num;j++) {//set block positions in the child state
						child->addPosition(blocks[j]->getPosition());
					}
					child->setGrid();
					if (!search(child)) {//check if the child state has been visited
						cnt++;//total number of states increases by 1
						//set attributes of the child state
						child->setParent(front);
						child->setMovedBlockIdx(n);
						child->setDir(i);
						child->setStep(step);
						child->setLevel(front->getLevel() + 1);

						insert(child);//insert the child state into hash table
						q.push(child);//push it into the queue
						level[levNo].push_back(child);//record it in the level list
					}
					else delete child;//If visited, delete the child state
				}
				blocks[n]->setPosition(front->getPositions()[n]);//update the block positions
			}
		}
	}
	endTime = clock();//time out
	////printRoute(front);
	//cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	//cout << "The no. of level: " << levNo << "\n";
	//cout << "The no. states: " << cnt << "\n";
	////levNo = getSteps(front);
	//cout << "The no. of steps: " << getSteps(front) << "\n";
}

/*
	Generate the game codes in JavaScript	
*/

//Print game solution recursively
void generateSol(State* s) {
	State* parent = s->getParent();
	if (parent == NULL) return;
	vector<Point> points = s->getPositions();
	int idx = s->getBlock();
	int i = points[idx].x, j = points[idx].y, dir = s->getDir();
	switch (dir)//find the reversed direction
	{
	case 0: dir = 1;break;
	case 1:dir = 0;break;
	case 2:dir = 3;break;
	case 3:dir = 2;break;
	}
	ss << "[" << i - 1 << "," << j - 1 << "," << dir << "]";//[i,j,dir]
	if (parent != NULL) {
		State* grand = parent->getParent();
		if (grand != NULL)ss << ",";
		generateSol(parent);
	}
}

//Print the game board
void generateMap(State* s) {
	ss << "[";
	for (int i = 0;i < row;i++) {
		if (i == 0) ss << "[";
		else ss << ", [";
		for (int j = 0;j < col;j++) {
			ss << s->getGrid(i, j);
			if (j == col - 1) ss << "]";
			else ss << ", ";
		}
	}
	ss << "]";
}

/*
	Initialization
*/
void initGame() {
	delete first;
	first = new State();
	initHashTable();
	initLevel();
}

int main(int argc, char** argv) {
	int maxLev = 0, type;
	if (argc > 2) {
		maxLev = atoi(argv[1]);
		type = atoi(argv[2]);
	}
	else type = atoi(argv[1]);
	if (maxLev > 0) levLimit = levLimit < maxLev ? levLimit : maxLev + 1;
	hGenNum = type;//number of the 1x2 block
	int arr[9] = { 2,3,3,3,3,1,1,1,1 };//block types
	for (int i = 0;i < 5;i++) {
		if (i < hGenNum) arr[i] = 2;
		else arr[i] = 3;
	}
	permutations(arr, 0, 9);//find all the permutations
	//cout << "Number of permutations: " << per.size() << endl;
	initGame();

	//Find all end configurations (starting nodes)
	setBlocks();//set all blocks
	int temp[9];
	bool flag = true;
	for (int n = 0;n < per.size();n++) {
		//find valid configurations for the permutation
		vector<int> list = per[n];
		for (int i = 0;i < list.size();i++) {
			temp[i] = list[i];
		}
		for (int i = 1;i <= 20;i++) {
			for (int j = i;j <= 20;j++) {
				flag = findEndConfig(temp, 9, i, j);
				if (flag) {
					setNewState();//find a new end configuration
				}
			}
		}
	}
	//cout << "Number of starting nodes: " << level[0].size() << endl;
	//Generate the Klotski puzzles with solutions by complete search
	multiSourceBFS();
	if (maxLev > 0) levNo = maxLev;
	while (level[levNo].size() == 0) levNo--;
	int len = level[levNo].size();//number of states in the last level
	if (len > 20) len = 20;
	//Generate the game codes
	ss << "\"MAP\":[";
	for (int i = 0;i < len;i++) {
		State* s = level[levNo][i];
		generateMap(s);
		if (i < len - 1) ss << ",";
	}
	ss << "], \"sol\":[";
	for (int i = 0;i < len;i++) {
		ss << "[";
		State* s = level[levNo][i];
		generateSol(s);
		ss << "]";
		if (i < len - 1) ss << ",";
	}
	ss << "]";
	//writeFile << ss.str();//Update the game codes for rendering
	cout << "{" << ss.str() << "}";
	return 0;
}
