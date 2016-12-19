#include "AIShell.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#define mp(x,y) make_pair( (x), (y) )

const int INF = numeric_limits<int>::max();
const int NEG_INF = numeric_limits<int>::min();

using namespace std;

namespace{
	class Compare {
	public:
		bool operator() (const MoveScore &a, const MoveScore &b) {
			return a.second > b.second;
		}
	};

	template<class T>
	void printDepth(const T &t, int depth) {
		for (int i = -1; i < depth; i++) cout << "   ";
		cout << t << '\n';
	}

	void printGameState(int **game, int N, int M) {
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < N; j++) {
				if (game[i][j] == 0)
					printf("%2s ", " ");
				else
					printf("%2s ", game[i][j] == 1 ? "A" : "H");
			}
			printf("\n");
		}
		fflush(stdout);
	}

    Node ApplyDirection(Node cur, Node dir){
        return mp(cur.first + dir.first, cur.second + dir.second);
    }
}

AIShell::AIShell(int numCols, int numRows, bool gravityOn, int** gameState, Move lastMove) : aiPieces(), humanPieces()
{
    this->deadline=0;
    this->numRows=numRows;
    this->numCols=numCols;
    this->gravityOn=gravityOn;
    this->gameState=gameState;
    this->lastMove=lastMove;

    //Collect the pieces already in play.
    for (int col = 0; col < numCols; col++){
        for (int row = 0; row < numRows; row++){
            switch(gameState[col][row]){
                case AI_PIECE:
                    aiPieces.insert(make_pair(col, row));
                    break;
                case HUMAN_PIECE:
                    humanPieces.insert(make_pair(col, row));
                    break;
                case NO_PIECE:
                    if (gravityOn)
                        break;
                    break;

            }
        }
    }
}

AIShell::~AIShell()
{

    //delete the gameState variable.
    for (int i =0; i<numCols; i++){
        delete [] gameState[i];
    }

    delete [] gameState;
}

Move AIShell::makeMove(){
	time(&start);
    if (lastMove.col == -1 || lastMove.row == -1){
        return Move(numCols/2, numRows/2);
    }

  	MoveScore next_move;
    for (int mDepth = 1; mDepth < 10; mDepth++){
      	this->maxDepth = mDepth;
      	MoveScore rt = AlphaBeta(NEG_INF, INF);
		if (!DeadlineCheck()) {
			//cout << "Depth: " << mDepth - 1 << '\n';
			break;
		}
		if (rt.first.first != -1 && rt.first.second != -1)
		{
			next_move = rt;
		}
		//cout << "Depth: " << mDepth << "| " << rt.first.first << ' ' << rt.first.second << '\n';
    }
	
	if (next_move.first.first == -1 || next_move.first.second == -1) {
		for (int col = 0; col < numCols; col++) {
			for (int row = 0; row < numRows; row++) {
				if (gameState[col][row] == NO_PIECE)
					return Move(col, row);
			}
		}
	}

    return Move(next_move.first.first, next_move.first.second);
}

MoveList AIShell::GetAllActions(int distance){
	set<Node> to_return;
	if (this->gravityOn){
		vector<int> colsInUse;
		/*for (int i = 0; i < numCols; i++)
			if (gameState[i][0] != NO_PIECE) colsInUse.push_back(i);*/

		for (int col = 0; col < numCols; col++)
			colsInUse.push_back(col);

		for (const auto &col : colsInUse) {
			int i = 0;
			while (gameState[col][i] != NO_PIECE && i < numRows) i++;
			if (i < numRows) to_return.insert(mp(col, i));
		}

		return to_return;
	}
	else {
		queue<pair<Node, int> > q;
		set<Node> allPieces;
		allPieces.insert(aiPieces.begin(), aiPieces.end());
		allPieces.insert(humanPieces.begin(), humanPieces.end());
		for (const auto &piece : allPieces) {
			for (const auto &dir_pair : directions) {
				for (const auto &dir : dir_pair) {
					Node next_node = ApplyDirection(piece, dir);

					if (isValidNode(next_node) && gameState[next_node.first][next_node.second] == NO_PIECE) {
						q.push(mp(next_node, 1));
						to_return.insert(next_node);
					}
				}
			}
		}

		while (!q.empty()) {
			auto cur = q.front();
			q.pop();

			if (cur.second + 1 > distance) continue;

			for (const auto &dir_pair : directions) {
				for (const auto &dir : dir_pair) {
					Node next_node = ApplyDirection(cur.first, dir);
					if (!isValidNode(next_node) || to_return.count(next_node)) continue;

					q.push(mp(next_node, cur.second + 1));
					to_return.insert(next_node);
				}
			}
		}

		return to_return;
	}
}

MoveScore AIShell::AlphaBeta(int alpha, int beta, PIECE_STATE turn, int depth) {
	MoveScore to_return = mp(mp(-1, -1), turn == AI_PIECE ? NEG_INF : INF);
	vector<Node> reducedActions;

	if (depth == maxDepth) {
		to_return.second = GetFitness();
		return to_return;
	}

	if (!DeadlineCheck()) return to_return;

	if (depth == 0 && !sortedMoves.empty()) {
		sort(sortedMoves.begin(), sortedMoves.end(), Compare());
		for (const auto &i : sortedMoves)
			reducedActions.push_back(i.first);
		sortedMoves.clear();
	}
	else {
		auto allActions = GetAllActions(turn);
		if (allActions.empty()) {
			to_return.second = GetFitness();
			return to_return;
		}

		for (const auto &action : allActions) {
			ApplyAction(action, turn);
			int rt = TerminalTest();
			if (rt == 1) {
				if (turn == AI_PIECE) {
					to_return.second = INF;
					to_return.first = action;
					RemoveAction(action, turn);
					return to_return;
				}
			}
			else if (rt == -1) {
				if (turn == HUMAN_PIECE) {
					to_return.second = NEG_INF;
					to_return.first = action;
					RemoveAction(action, turn);
					return to_return;
				}
			}
			else {
				reducedActions.push_back(action);
			}
			RemoveAction(action, turn);
		}
	}

    for (const auto &action: reducedActions){
        ApplyAction(action, turn);
        MoveScore rt = AlphaBeta(alpha, beta, turn == AI_PIECE ? HUMAN_PIECE : AI_PIECE, depth+1);
		if (depth == 0) {
			//cout << '\t' << action.first << ' ' << action.second << ' ' << rt.second << '\n';
			sortedMoves.push_back(make_pair(action, rt.second));
		}
		if (!DeadlineCheck()) return to_return;
        if (turn == AI_PIECE){
            if (rt.second > to_return.second){
                to_return.second = rt.second;
                to_return.first = action;
            }

            alpha = max(alpha, rt.second);
        }
        else{
            if(rt.second < to_return.second){
                to_return.second = rt.second;
                to_return.first = action;
                beta = rt.second;
            }
            beta = min(beta, rt.second);
        }
		RemoveAction(action, turn);
        if (alpha >= beta)
        	return to_return;
    }

    return to_return;
}

void AIShell::ApplyAction(Node move, PIECE_STATE turn){
    gameState[move.first][move.second] = turn;
    if (turn == AI_PIECE) aiPieces.insert(move);
    else humanPieces.insert(move);
}

void AIShell::RemoveAction(Node move, PIECE_STATE turn){
    gameState[move.first][move.second] = NO_PIECE;
    if (turn == AI_PIECE) aiPieces.erase(move);
    else humanPieces.erase(move);
}
//Returns -1 if the other plays wins, return 1 if we won, or 0 if the game is not at a terminal state yet
int AIShell::TerminalTest(){
    set<Node> seen[DIRECTIONS_SIZE];
    fill(seen, seen+DIRECTIONS_SIZE, set<Node>());
  
    for (const auto &piece : aiPieces){
        for (int dir = 0; dir < DIRECTIONS_SIZE; dir++){
            if (seen[dir].count(piece)) continue;

            seen[dir].insert(piece);
            auto leftChain = ChainSizeCount(ApplyDirection(piece, directions[dir][0]), directions[dir][0], seen[dir], AI_PIECE);
            auto rightChain = ChainSizeCount(ApplyDirection(piece, directions[dir][1]), directions[dir][1], seen[dir], AI_PIECE);			
			int temp = 1 + leftChain.size + rightChain.size;
			if (temp >= k)
            	return 1;
		}
    }
  
    for (int dir = 0; dir < DIRECTIONS_SIZE; dir++)
        seen[dir].clear();

	for (const auto &piece : humanPieces){
        for (int dir = 0; dir < DIRECTIONS_SIZE; dir++){
            if (seen[dir].count(piece)) continue;

            seen[dir].insert(piece);
            auto leftChain = ChainSizeCount(ApplyDirection(piece, directions[dir][0]), directions[dir][0], seen[dir], HUMAN_PIECE);
            auto rightChain = ChainSizeCount(ApplyDirection(piece, directions[dir][1]), directions[dir][1], seen[dir], HUMAN_PIECE);
			int temp = 1 + leftChain.size + rightChain.size;
			if (temp >= k)
				return -1;
			else if (temp >= (k - 1) && !leftChain.cutOff && !rightChain.cutOff) {
				return -1;
			}
        }
    }
  	return 0;
}

bool AIShell::isValidNode(Node node){
    return !(node.first < 0 || node.first >= numCols || node.second < 0 || node.second >= numRows);
}

AIShell::ChainData AIShell::ChainSizeCount(Node current_node, const Node &dir, set<Node> &seen, PIECE_STATE turn){
    if (!isValidNode(current_node)){
        return {0, true};
    }
    else if (gameState[current_node.first][current_node.second] != turn){
        return {0, gameState[current_node.first][current_node.second] == NO_PIECE ? false: true};
    }
    seen.insert(current_node);
    auto rt =  ChainSizeCount(ApplyDirection(current_node, dir), dir, seen, turn);
    rt.size++;
    return rt;
}

int AIShell::GetFitness(){
	int aiFitness = 0, humanFitness = 0;
	int aiC, huC;
	aiC = huC = 1;
    set<Node> seen[DIRECTIONS_SIZE];
    fill(seen, seen+DIRECTIONS_SIZE, set<Node>());

  	//AI_PIECE chain computation
    for (const auto &piece : aiPieces){
        for (int dir = 0; dir < DIRECTIONS_SIZE; dir++){
            if (seen[dir].count(piece)) continue;

            seen[dir].insert(piece);
            auto leftChain = ChainSizeCount(ApplyDirection(piece, directions[dir][0]), directions[dir][0], seen[dir], AI_PIECE);
            auto rightChain = ChainSizeCount(ApplyDirection(piece, directions[dir][1]), directions[dir][1], seen[dir], AI_PIECE);

            //Only intested in chains longer than 1
            if (leftChain.size == 0 && rightChain.size == 0) continue;
			aiC++;
			int temp = 1 + leftChain.size + rightChain.size;

            //Both sides are cutoff
            if (leftChain.cutOff && rightChain.cutOff){
                aiFitness += temp;
            }
            //Boths sides are not cutoff
            else if (!leftChain.cutOff && !rightChain.cutOff){
				aiFitness += pow(temp, 3);
            }
            //One side is cutoff
            else{
				aiFitness += pow(temp, 2);
            }
        }
    }

    //Clearing the direction so we can search for the human's fitness
    for (int dir = 0; dir < DIRECTIONS_SIZE; dir++)
        seen[dir].clear();

  	//Human chain computation
	for (const auto &piece : humanPieces){
        for (int dir = 0; dir < DIRECTIONS_SIZE; dir++){
            if (seen[dir].count(piece)) continue;

            seen[dir].insert(piece);
            auto leftChain = ChainSizeCount(ApplyDirection(piece, directions[dir][0]), directions[dir][0], seen[dir], HUMAN_PIECE);
            auto rightChain = ChainSizeCount(ApplyDirection(piece, directions[dir][1]), directions[dir][1], seen[dir], HUMAN_PIECE);

            //Only intested in chains longer than 1
            if (leftChain.size == 0 && rightChain.size == 0) continue;
			huC++;
			int temp = 1 + leftChain.size + rightChain.size;

            //Both sides are cutoff
            if (leftChain.cutOff && rightChain.cutOff){
                humanFitness += temp;
            }
            //Boths sides are not cutoff
            else if (!leftChain.cutOff && !rightChain.cutOff){
                humanFitness += temp * temp * temp;
            }
            //One side is cutoff
            else{
                humanFitness += temp * temp;
            }
        }
    }
    return (aiFitness << 11)/aiC - (humanFitness << 11)/huC;
}

bool AIShell::DeadlineCheck() {
	time_t cur;
	time(&cur);

	double seconds = difftime(cur, start);
	return ((seconds * 1000) < (deadline - 50));
}

