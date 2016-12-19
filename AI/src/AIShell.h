#ifndef AISHELL_H
#define AISHELL_H

#pragma once
#include "Move.h"
#include <iostream>
#include <vector>
#include <utility>
#include <set>
#include <limits>
#include <queue>
#define mp(x,y) make_pair( (x), (y) )

using namespace std;


// A new AIShell will be created for every move request.
typedef pair<int, int> Node;
typedef pair<Node, int> MoveScore;
typedef set<Node> MoveList;
typedef vector<vector<Node> > DirectionList;


class AIShell{
public:
    enum PIECE_STATE{
        AI_PIECE        = 1,
        HUMAN_PIECE     = -1,
        NO_PIECE        = 0
    };


public:
    enum DIRECTIONS{
        NEGATIVE_DIAGONAL,
        POSITIVE_DIAGONAL,
        UP_DOWN,
        LEFT_RIGHT,
        DIRECTIONS_SIZE
    };

    struct ChainData{
        int size;
        bool cutOff;
    };

    const DirectionList directions = {
        {mp(-1, -1), mp(1, 1)},
        {mp(-1, 1), mp(1, -1)},
        {mp(-1, 0), mp(1, 0)},
        {mp(0, -1), mp(0, 1)}};

	time_t start;
    int maxDepth;
	int numRows; 
	int numCols; 
	int **gameState; 
	bool gravityOn; 
	Move lastMove; 
	vector<MoveScore> sortedMoves;
    set<Node> aiPieces, humanPieces;

    MoveList GetAllActions(int distance = 1);
    MoveScore AlphaBeta(int alpha, int beta, PIECE_STATE turn = AI_PIECE, int depth = 0);
	int GetFitness();
    void ApplyAction(Node move, PIECE_STATE turn);
    void RemoveAction(Node move, PIECE_STATE turn);
    ChainData ChainSizeCount(Node current_node, const Node &dir, set<Node> &seen, PIECE_STATE turn);
    bool isValidNode(Node node);
    int TerminalTest();
	bool DeadlineCheck();

public:
	int deadline; 
	int k;       

	AIShell(int numCols, int numRows, bool gravityOn, int** gameState, Move lastMove);
	~AIShell();
	Move makeMove();
};

#endif //AISHELL_H
