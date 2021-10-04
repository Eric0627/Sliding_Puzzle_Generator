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

#include "imageInterpreter.h"

#define N 20
#define L 200

using namespace std;

/*
Student name: Li Chaohui
Student number: 55202995
Project code: 20CS071
Project title: A Game Generator for Sliding Puzzles

The game generator for Sokoban
*/

ifstream part1("../input/sokoban/part1.txt"), part2("../input/sokoban/part2.txt");//game codes in JavaScript
ofstream writeFile("../collection2.txt");
stringstream ss;//string stream for formating the game codes

set<string> visited;//set (Red-Black Tree) for visited states

int flag = false;//indicator: whether finding the optimal solution
int row, col, map[N][N], iniMap[N][N];//map: 0-space 1-obstacle 2-block
int num;//number of blocks
int levNo = 0;//number of levels

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
	int getKey() {
		return (x - 1) * col + y;//get key for hashing
	}
};

Point des[N];//destinations of all boxes in Sokoban

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
		if (i < 0 || i >= r || j < 0 || j >= c) return -1;
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
	int type;//index for different block shapes (only one type in Sokoban: 1x1 block)
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
		for (int i = 0;i < row;i++)
			for (int j = 0;j < col;j++)
				grid[i][j] = map[i][j];
		//copy blocks
		for (int n = 0;n < num;n++) {
			int r = points[n].x - 1, c = points[n].y - 1;
			Block* b = blocks[n];
			for (int i = 0;i < b->getRow();i++) {
				for (int j = 0;j < b->getCol();j++) {
					if (grid[r + i][c + j] == 0) {//obstacles cannot be overlapped
						grid[r + i][c + j] = b->getType() + 1;
						if (n > 0) grid[r + i][c + j]++;
					}
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
		int r1, c1, r2, c2;
		for (int i = 0;i < num;i++) {
			r1 = points[i].x - 1, c1 = points[i].y - 1;
			r2 = p[i].x - 1, c2 = p[i].y - 1;
			if (grid[r1][c1] != s->getGrid(r2, c2)) return false;
		}
		return true;
	}
	string getKey() {//get the state key for avoiding collision
		string key = "";
		stringstream temp;
		int output[N][N];
		//copy map
		for (int i = 0;i < row;i++)
			for (int j = 0;j < col;j++)
				output[i][j] = map[i][j];
		//copy blocks
		for (int n = 0;n < num;n++) {
			int r = points[n].x - 1, c = points[n].y - 1;
			Block* b = blocks[n];
			if (n > 0) output[r][c] = 3;
			else output[r][c] = 2;
		}
		for (int i = 0;i < row;i++) {
			for (int j = 0;j < col;j++) {
				string str;
				switch (output[i][j])
				{
				case 0: str = "0";break;
				case 1: str = "X";break;
				default: str = (output[i][j] - 1) + 48;break;
				}
				temp << str << ",";
			}
			temp << endl;
		}
		temp << endl;
		key = temp.str();
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
};


State* first = new State();;//the starting nodes (all the end configurations in Sokoban)

//print out a configuration on console
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
				if (output[r + i][c + j] == 0 && b->getCell(i, j) == 1) {//obstacles cannot be overlapped
					output[r + i][c + j] = 2;
					if (n > 0) output[r + i][c + j]++;
				}
			}
		}
	}
	for (int i = 0;i < row;i++) {
		for (int j = 0;j < col;j++) {
			string str;
			switch (output[i][j])
			{
			case 0: str = "0";break;
			case 1: str = "X";break;
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
	for (int i = 0;i <= L;i++) {
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
	m = 18013;//pow(3,12);//row * col;
	for (int i = 0;i < m;i++) {
		vector<State*> temp;
		temp.clear();
		hashTable.push_back(temp);
	}
}

int hashKey[100] = { 0 };//factors for hashing
int fab(int n) {//Fabonacci number
	if (hashKey[n] != 0) return hashKey[n];
	if (n < 1) return 0;
	else if (n == 1 || n == 2) return 1;
	int res = 1;
	int pre = 1;
	int temp = 0;
	for (int i = 3; i < n; i++) {
		temp = res;
		res = pre + res;
		pre = temp;
	}
	return hashKey[n] = res;
}

//int hashKey[20] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181 };//Fabonacci Sequence

//hash function
int Hash1(State* s) {//the primary hash function
	int res = 0;
	for (int i = 0;i < row;i++)
		for (int j = 0;j < col;j++) res ^= fab(i*col + j) * (s->getGrid(i, j));
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
	for (int i = 0;i < num;i++) {
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
	Construct the end configurations (starting nodes)
*/

void setNewState() {//set a new configuration
	State* newState = new State();
	for (int i = 0;i < num;i++) newState->addPosition(blocks[i]->getPosition());
	newState->setGrid();
	newState->setStep(0), newState->setLevel(0);//the end configuration is the root node
	newState->setParent(NULL), newState->setMovedBlockIdx(-1), newState->setDir(-1);
	string stateKey = newState->getKey();
	//cout << stateKey << endl;
	if (visited.find(stateKey) == visited.end()) {//check if the state has been visited
		visited.insert(stateKey);
		level[0].push_back(newState);//record the state at level 0 (the root level)
	}
}

//Get the end configuration from the text file
void readByText(char* path) {
	ifstream readFile(path);
	string type;
	int t;
	//build the map
	readFile >> row >> col;
	for (int i = 0;i < row;i++)
		for (int j = 0;j < col;j++)
			readFile >> map[i][j];
	readFile >> num;
	//initialize the player (a special 1x1 block)
	Block* player = new Block(Point(0, 0), 1, 1, 1);
	player->setShape(0, 0, 1);
	blocks.push_back(player);
	//set the blocks
	for (int i = 0;i < num;i++) {
		int a, b, x, y;
		readFile >> a >> b >> x >> y;
		//set destinations
		des[i] = Point(x, y);
		//set the block type
		if (a == 2 && b == 2) type = "Four", t = 4;
		else if (a == 2) type = "TowV", t = 3;
		else if (b == 2) type = "TowH", t = 2;
		else type = "One", t = 1;

		Block* block = new Block(Point(x, y), a, b, t);//set block point & size
		//set shape of block
		int temp;
		for (int i = 0;i < a;i++) {
			for (int j = 0;j < b;j++) {
				readFile >> temp;
				block->setShape(i, j, temp);
			}
		}
		blocks.push_back(block);
	}
	num++;//including the player
	//set the temporary map
	int board[N][N];
	//copy map
	for (int i = 0;i < row;i++)
		for (int j = 0;j < col;j++)
			board[i][j] = map[i][j];
	//copy blocks
	for (int n = 1;n < num;n++) {
		Block* b = blocks[n];
		Point p = b->getPosition();
		int r = p.x - 1, c = p.y - 1;
		for (int i = 0;i < b->getRow();i++) {
			for (int j = 0;j < b->getCol();j++) {
				//obstacles cannot be overlapped
				if (board[r + i][c + j] == 0 && b->getCell(i, j) == 1) board[r + i][c + j] = 2;
			}
		}
	}
	//find possible positions of the player (around the boxes)
	for (int i = 1;i < num;i++) {
		for (int j = 0;j < 4;j++) {
			Point p = blocks[i]->getPosition();
			int r = p.x + x[j], c = p.y + y[j];
			if (r < 1 || c < 1 || r > row || c > col) continue;//out of board
			if (board[r - 1][c - 1] != 0) continue;//collide the obstacle
			//update the player pos
			blocks[0]->setPosition(Point(r, c));
			setNewState();
		}
	}
	//cout << "Number of end configurations: " << level[0].size() << endl;
}

//Get the end configuration from the image file
void readByImg(char* path, int r, int c) {
	//read the image input
	int** res = imageToArray(path, r, c);
	row = r, col = c;
	int cnt = 0;
	//initialize the player (a special 1x1 block)
	Block* player = new Block(Point(0, 0), 1, 1, 1);
	player->setShape(0, 0, 1);
	blocks.push_back(player);
	//find all boxes (on destination)
	for (int i = 0;i < row;i++) {
		for (int j = 0;j < col;j++) {
			if (res[i][j] == 2) {
				map[i][j] = 0;
				des[cnt++] = Point(i + 1, j + 1);
				Block* block = new Block(Point(i+1, j+1), 1, 1, 1);//set block point & size
				//set shape of block
				block->setShape(0, 0, 1);
				blocks.push_back(block);
			}
			else map[i][j] = res[i][j];
		}	
	}
	num = cnt + 1;
	//set the temporary map
	int board[N][N];
	//copy map
	for (int i = 0;i < row;i++)
		for (int j = 0;j < col;j++)
			board[i][j] = map[i][j];
	//copy blocks
	for (int n = 1;n < num;n++) {
		Block* b = blocks[n];
		Point p = b->getPosition();
		int r = p.x - 1, c = p.y - 1;
		for (int i = 0;i < b->getRow();i++) {
			for (int j = 0;j < b->getCol();j++) {
				//obstacles cannot be overlapped
				if (board[r + i][c + j] == 0 && b->getCell(i, j) == 1) board[r + i][c + j] = 2;
			}
		}
	}
	//find possible positions of the player (around the boxes)
	for (int i = 1;i < num;i++) {
		for (int j = 0;j < 4;j++) {
			Point p = blocks[i]->getPosition();
			int r = p.x + x[j], c = p.y + y[j];
			if (r < 1 || c < 1 || r > row || c > col) continue;//out of board
			if (board[r - 1][c - 1] != 0) continue;//collide the obstacle
			//update the player pos
			blocks[0]->setPosition(Point(r, c));
			setNewState();
		}
	}
	//cout << "Number of end configurations: " << level[0].size() << endl;
}

/*
	Generate sliding puzzles by the pivotal algorithm
*/

//Check if a block can move a small step
bool moveBlock(int dir, int step, bool isBoxMoved) {//isBoxMoved: whether the box moves with the player
	//move the player
	Block* b = blocks[0];
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
	for (int i = 1;i < num;i++) {
		if (b->isOverlap(blocks[i])) {
			b->setPosition(p);//restore the previous position if overlap	
			return false;
		}
	}
	if (isBoxMoved) {
		int tempX = p.x - step * x[dir],
			tempY = p.y - step * y[dir];
		for (int i = 1;i < num;i++) {
			Point boxPos = blocks[i]->getPosition();
			if (boxPos.isEquale(Point(tempX, tempY))) {//find the box that can move with the player
				blocks[i]->setPosition(p);
				return true;
			}
		}
		return false;
	}
	return true;
}

//Backtrack and print the route on the console
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
		cout << "\nMove Block: " << s->getBlock() << " " << s->getStep() << " steps " << str << "\n\n";
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
	clock_t startTime, endTime;
	startTime = clock();//time onset
	int cnt = 0, len = level[0].size();
	State* front = level[0][0];//first state
	queue<State*> q;
	for (cnt = 0;cnt < len;cnt++) q.push(level[0][cnt]);//put all the end configurations into the queue
	while (!q.empty() && levNo < L) {
		front = q.front();
		q.pop();
		//insert(front);	
		//cout << "Level: " << levNo << endl;
		levNo = front->getLevel() + 1;
		for (int i = 0;i < 8;i++) {//4 directions x 2 situations (move box or not)
			for (int n = 0;n < num;n++) blocks[n]->setPosition(front->getPositions()[n]);//set the position of block n in the state
			int step = 0;
			bool isBoxMoved = i < 4 ? false : true;
			if (moveBlock(i % 4, 1, isBoxMoved)) step++;//check how many small steps
			if (step > 0) {
				State* child = new State();
				for (int j = 0;j < num;j++) {//set block positions in the child state
					child->addPosition(blocks[j]->getPosition());
				}
				child->setGrid();
				string childKey = child->getKey();//key for avoiding repetitive search
				if (visited.find(childKey) == visited.end()) {//check if the child is visited
					cnt++;//total number of states increases by 1
					//set attributes of the child state
					child->setParent(front);
					child->setMovedBlockIdx(isBoxMoved);
					child->setDir(i % 4);
					child->setStep(step);
					child->setLevel(front->getLevel() + 1);
					
					q.push(child);//push it into the queue
					visited.insert(childKey);//record the child state as visited
					level[levNo].push_back(child);//record it in the level list
				}
				else delete child;
			}
		}
	}
	endTime = clock();//time out
	//printRoute(front);
	//cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	//cout << "The no. of level: " << levNo << "\n";
	//cout << "The no. states: " << cnt << "\n";
	////levNo = getSteps(front);
	//cout << "The no. of steps: " << getSteps(front) << "\n";
	////printRoute(front);
}

/*
	Generate the game codes in JavaScript
*/

//Print game solution recursively
void generateSol(State* s) {
	State* parent = s->getParent();
	if (parent == NULL) return;
	vector<Point> points = s->getPositions();
	int i = points[0].x, j = points[0].y, dir = s->getDir();
	switch (dir)//get the reversed direction
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
	int output[N][N];
	//copy map
	for (int i = 0;i < row;i++) {
		for (int j = 0;j < col;j++) {
			output[i][j] = map[i][j];
			if (map[i][j] == 1) output[i][j] = 5;
		}
	}
	vector<Point> points = s->getPositions();
	//copy blocks
	for (int n = 0;n < num;n++) {
		int r = points[n].x - 1, c = points[n].y - 1;
		Block* b = blocks[n];
		for (int i = 0;i < b->getRow();i++) {
			for (int j = 0;j < b->getCol();j++) {
				if (output[r + i][c + j] == 0 && b->getCell(i, j) == 1) {//obstacles cannot be overlapped
					output[r + i][c + j] = 1;
					if (n > 0) output[r + i][c + j]++;
				}
			}
		}
	}
	//set blanks (ground) and targets
	//1: player, 2: box, 3 : ground, 4 : target, 5 : wall/obstacle, 6 : arrive (box on target)
	bool isGround = false;
	for (int i = 0;i < row;i++) {
		int left = 0, right = col - 1;
		for (int j = left;j < col;j++) {
			if (output[i][j] == 5) {
				left = j;
				break;
			}
		}
		for (int j = right;j >= 0;j--) {
			if (output[i][j] == 5) {
				right = j;
				break;
			}
		}
		for (int j = left;j <= right;j++) {
			if (output[i][j] == 0) output[i][j] = 3;
		}
	}
	for (int i = 0;i < num - 1;i++) {
		if (output[des[i].x - 1][des[i].y - 1] == 2) output[des[i].x - 1][des[i].y - 1] = 6;
		else output[des[i].x - 1][des[i].y - 1] = 4;
	}
	ss << "[";
	for (int i = 0;i < row;i++) {
		if (i == 0) ss << "[";
		else ss << ",[";
		for (int j = 0;j < col;j++) {
			ss << output[i][j];
			if (j == col - 1) ss << "]";
			else ss << ",";
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
	int fileType = atoi(argv[1]);
	//initialization
	initGame();
	if (fileType == 0)
		//read the text input and construct the starting nodes
		readByText(argv[2]);
	//read the image input and construct the starting nodes
	else readByImg(argv[2], atoi(argv[3]), atoi(argv[4]));//path, rows, cols
	//Generate Sokoban puzzles with solutions
	multiSourceBFS();

	while (level[levNo].size() == 0) levNo--;//prevent out of range
	int len = level[levNo].size();//number of states in the last level
	//printRoute(level[levNo][0]);

	//Generate the game codes
	string line;
	//while (getline(part1, line)) ss << line << endl;
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
	//while (getline(part2, line)) ss << line << endl;
	//writeFile << ss.str();
	/*ofstream outFile("../Vue_project/my-app/src/components/Sokoban.vue");
	outFile << ss.str();*/
	cout << "{" << ss.str() << "}";
	return 0;
}
