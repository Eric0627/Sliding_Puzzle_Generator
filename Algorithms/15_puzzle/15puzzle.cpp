#include<iostream>
#include<fstream>
#include<ctime>
#include<vector>
#include<queue>
#include<string>
#include<sstream>
#include<stdlib.h>
#include<math.h>
#include<set>

#define N 10
#define L 16//maximum level number

using namespace std;

/*
The game generator for 15-puzzle
*/

ifstream readFile("../input/15puzzle/input1.txt");
ifstream part1("../input/part1.txt"), part2("../input/part2.txt");//game codes in JavaScript
ofstream writeFile("../collection1.txt");
stringstream ss;//string stream for formating the game codes

int flag = false;//indicator: whether finding the optimal solution
int row, col, map[N][N], iniMap[N][N];//map: 0-space 1-obstacle 2-block
int num;//number of blocks
int levNo = 0;//number of levels
int maxLev = L;

int x[4] = { -1,1,0,0 },
y[4] = { 0,0,-1,1 };//moving direction: up, down, left, right

set<string> visited;//set (Red-Black Tree) for visited states

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
	int getKey() {
		return (x - 1) * col + y;//get key for hashing
	}
};

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
	int type;//index for different block shapes (only one type in 15-puzzle: 1x1 block)
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
	void setBlank(Point p) {//set the position of the blank in 15-puzzle
		blank = p;
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
	Point getBlank() {//get the blank position
		return blank;
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
	bool isEquivalent(State* s) {//check if two states are the same
		vector<Point> p = s->getPositions();
		for (int i = 0;i < num;i++)
			if (!points[i].isEquale(p[i])) return false;
		return true;
	}
	string getKey() {//get the state key for avoiding collision
		string key = "";
		for (int i = 0;i < num;i++)
			key += to_string(points[i].getKey()) + ",";
		return key;
	}
private:
	int step;//number of steps to the parent state
	int level;//levels to the root (the least number of steps)
	State* parent;//parent state
	int moved;//index of the moved block
	int direction;//moving direction
	vector<Point> points;//positions of all blocks
	int grid[N][N];//grid of types for pruning
	Point blank;//position of the blank
};


State* first = new State();//the starting node (the only end configurations in 15-puzzle)

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
			if (output[i][j] == 0) cout << 0;
			else cout << output[i][j] - 1;
			cout << "\t";
		}
		cout << endl;
	}
	cout << endl;
}

//Record the expanded states in each level
vector< vector<State*> > level;
void initLevel() {
	for (int i = 0;i <= maxLev;i++) {
		vector<State*> temp;
		temp.clear();
		level.push_back(temp);
	}
}

/*
	Definition of hash table and hash operations
*/

//hash table
int m;//length of hashtable (different for different sliding puzzles)
vector< vector<State*> > hashTable;
void initHashTable() {
	m = 3801313;//the prime number can distribute the hash values more evenly
	for (int i = 0;i < m;i++) {
		vector<State*> temp;
		temp.clear();
		hashTable.push_back(temp);
	}
}

//Compute the number of inversions in the configuration/state
int getInversion(State* s, int idx) {
	vector<Point> points = s->getPositions();
	int cnt = 0, key = points[idx].getKey();
	for (int i = 0;i < idx;i++)
		if (key < points[i].getKey()) cnt++;
	return cnt;
}

int hashKey[20] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181 };//Fabonacci Sequence
int Hash(State* s) {//hash function for 15-puzzle
	/*int res = 0;
	vector<Point> points = s->getPositions();
	for (int i = 0;i < num;i++) res ^= hashKey[i] * (points[i].getKey()*i);
	return res % m;*/
	int res = 0;
	vector<Point> points = s->getPositions();
	for (int i = 0;i < num;i++) {
		int inv = getInversion(s, i);
		int fac = hashKey[points[i].getKey() - 1];
		res += (inv * fac) % m;
	}
	res = (res + 1) % m;
	return res;
}

void insert(State* s) {//insert the state into the hash table
	int value = Hash(s);
	State* temp;
	hashTable[value].push_back(s);
}

bool search(State* s) {//check if the state is in the hash table
	int value = Hash(s);
	int num = hashTable[value].size();
	for (int i = 0;i < num;i++) {
		if (s->isEquivalent(hashTable[value][i])) return true;
	}
	return false;
}

/*
	Construct the end configuration (starting node)
*/

//Get the end configuration from the text file
void readin() {
	string type;
	int t;
	//build the map
	row = col = 4, num = 15;//numbers of rows and columns of the gameboard
	for (int i = 0;i < row;i++)
		for (int j = 0;j < col;j++)
			map[i][j] = 0;
	//set the blocks
	for (int i = 0;i < num;i++) {
		int a = 1, b = 1, x = i / 4 + 1, y = i % 4 + 1;
		//set the block type
		t = 1;
		first->addPosition(Point(x, y));
		Block* block = new Block(Point(x, y), a, b, t);//set block point & size
		//set shape of block
		int temp = 1;
		for (int i = 0;i < a;i++) {
			for (int j = 0;j < b;j++) {
				block->setShape(i, j, temp);
			}
		}
		blocks.push_back(block);
	}
	first->setGrid();
}

/*
	Generate sliding puzzles by the pivotal algorithm
*/

//Check if a block can move a small step
bool moveBlock(int idx, int dre, int step) {
	Block* b = blocks[idx];
	Point p = b->getPosition();
	int dx = p.x + step * x[dre],
		dy = p.y + step * y[dre];//coordinates after movement
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

//Backtrack and print the route on the console by recursion
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
		cout << "\nMove Block " << s->getBlock() + 1 << " " << s->getStep() << " steps " << str << "\n\n";
		printRoute(parent);
	}
	else {
		printBoard(s);
		//cout << "Reach the destination!" << endl;
	}
}

//Get total steps along the path
int getSteps(State* s) {
	State* parent = s->getParent();
	if (parent != NULL) return s->getStep() + getSteps(parent);
	else return s->getStep();
}

//Multi-source BFS
void multiSourceBFS() {
	int rec = 0;//record the last level number
	clock_t startTime, endTime;
	startTime = clock();//time onset
	//initialize the first state
	int cnt = 0, len = level[0].size();
	first->setStep(0), first->setLevel(0), first->setBlank(Point(row, col));
	first->setParent(NULL), first->setMovedBlockIdx(-1), first->setDir(0);
	State* front = first;
	//printBoard(first);
	queue<State*> q;
	q.push(first), level[0].push_back(first);
	//insert(first);
	visited.insert(first->getKey());//record the first state as visited
	while (!q.empty() && levNo < maxLev) {
		//if (levNo != rec) {//update the level number
		//	rec = levNo;
		//	cout << "Level: " << rec << " Count: " << cnt << endl;
		//	cnt = 0;
		//}
		front = q.front();
		q.pop();
		for (int n = 0;n < num;n++) blocks[n]->setPosition(front->getPositions()[n]);//update positions of all blocks
		//cout << "Level: " << levNo << endl;
		levNo = front->getLevel() + 1;
		for (int n = 0;n < num;n++) blocks[n]->setPosition(front->getPositions()[n]);//set the point of block in the state
		for (int i = 0;i < 4;i++) {//move the blank in 4 directions
			int idx = -1;
			Point blank = front->getBlank();
			Point neighbor = Point(blank.x + x[i], blank.y + y[i]);//find the position nearby the blank
			for (int n = 0;n < num;n++) {
				if (neighbor.isEquale(blocks[n]->getPosition())) {
					idx = n;
					break;
				}
			}
			if (idx == -1) continue;
			int step = 0;
			int dir = -1;
			switch (i) {//reverse the moving direction
			case 0: dir = 1;break;
			case 1: dir = 0;break;
			case 2: dir = 3;break;
			case 3: dir = 2;break;
			}
			State* child = new State();
			child->setBlank(blocks[idx]->getPosition());
			if (moveBlock(idx, dir, 1)) step++;//check how many small steps
			if (step > 0) {
				for (int j = 0;j < num;j++) {//set block positions in the child state
					child->addPosition(blocks[j]->getPosition());
				}
				child->setGrid();
				//if (!search(child)) {
				if (visited.find(child->getKey()) == visited.end()) {//check if the child is visited
					cnt++;//total number of states increases by 1
					//set attributes of the child state
					child->setParent(front);
					child->setMovedBlockIdx(idx);
					child->setDir(dir);
					child->setStep(step);
					child->setLevel(front->getLevel() + 1);

					visited.insert(child->getKey());//record the child state as visited
					q.push(child);//push it into the queue
					level[levNo].push_back(child);//record it in the level list
				}
				else delete child;//If visited, delete the child state
			}
			blocks[idx]->setPosition(front->getPositions()[idx]);//update the block positions
		}
	}
	//endTime = clock();//time out
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
	int i = points[idx].x, j = points[idx].y;
	ss << "[" << i - 1 << "," << j - 1 << "]";//[i,j,]
	if (parent != NULL) {
		State* grand = parent->getParent();
		if (grand != NULL) ss << ",";
		generateSol(parent);
	}
}

//Print the game board
void generateMap(State* s) {
	ss << "[";
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
		if (i != 0) ss << ",";
		ss << "[";
		for (int j = 0;j < col;j++) {
			if (output[i][j] == 0) ss << 0;
			else ss << output[i][j] - 1;
			if (j < col - 1) ss << ",";
		}
		ss << "]";
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
	int numOfPuzzles = 20;
	if (argc > 2) {
		maxLev = atoi(argv[1]) < L ? atoi(argv[1]) + 1 : L;
		numOfPuzzles = atoi(argv[2]);
	}
	else numOfPuzzles = atoi(argv[1]);
	//initialization
	initGame();
	//construct the starting state
	readin();
	//Generate the 15-puzzles with solutions
	multiSourceBFS();
	if (argc > 2) levNo = maxLev - 1;
	while (level[levNo].size() == 0) levNo--;//prevent out of range
	int len = level[levNo].size();//number of states in the last level
	if (len > numOfPuzzles) len = numOfPuzzles;
	//printRoute(level[levNo][0]);
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
	//writeFile << ss.str();
	cout << "{" << ss.str() << "}";
	return 0;
}
