// ======================================================================
// FILE:        MyAI.hpp
//
// AUTHOR:      Jian Li
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================

#ifndef MINE_SWEEPER_CPP_SHELL_MYAI_HPP
#define MINE_SWEEPER_CPP_SHELL_MYAI_HPP

#include "Agent.hpp"
#include <iostream> // temporary use
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <climits>
#include <math.h>

using namespace std;

class MyAI : public Agent
{
public:
    MyAI ( int _rowDimension, int _colDimension, int _totalMines, int _agentX, int _agentY );

    Action getAction ( int number ) override;


    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================
private:
	// Tile structure
    struct Tile{
        bool uncovered  = true;		// the tile uncovered or not
        bool flag       = false; 	// the tile has been flag or not
        int  number     = -1;     	// records number of bombs around
        bool visited	= false; 	// records if already visited or not (mostly for dfs)
    };

    int remain_mines;		// the remaining unflagged mines
    int rows;
    int cols;
    Tile**	board;	// the board MyAI percepts so far

	int agentX;		// current position, note: here the index is from 0 ~ cols-1 (not 1 ~ cols)
	int agentY;
	int lastAction;

	int startX;		// the range for configuration (instead of the whole board)
	int startY;
	int endX;
	int endY;

	set<pair<int, int>> history;

	vector<Action> actionQueue;			// the buffer of actions waiting to be executed
	vector<pair<int, int>> zeroTiles;
	//vector<pair<int, int>> edgeTiles;
	set<pair<int, int>> edgeTiles;

	void 					uncoverNeighborTiles(int c, int r);			// uncover the neghbor tiles
	bool 					checkBoundTiles(int x, int y); 				// uncover/flag the neighbor tiles if for sure, return false if no neighbor tiles

	bool 					neighbor(pair<int, int> a, pair<int, int> b);
	void 					edgeTilesSegment(vector<vector<pair<int, int>>>& segment);
	void 					act2SafeTilebyStat(map<pair<int, int>, float>& s);
	void 					caculateMineStat(map<pair<int, int>, float>& s, 
											vector<vector<Action>>& c);
	void 					bestGuessbyStat(map<pair<int, int>, float>& s);

	vector<vector<Action>> 	findMinesConfig(vector<pair<int, int>>& a);	// find all the possible configurations
	void 					setConfig(vector<Action> config);			// push the config to actionQueue
	void 					printConfig(vector<Action> config);
	vector<vector<int>> 	getCombination(vector<int> ary);			// generate combination of array, note: the input ary must be lexicographicaly sorted
	void 					dfsMines(vector<vector<Action>>& configs, 	// dfs for finding mines configuration	
									vector<Action>& config, 
									vector<pair<int, int>>& edgTiles,
									int index, int flagged);

    Action 					popActionQueue();							// do the first action in actionQueue
    bool 					isInBounds ( int c, int r );
    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================
};

#endif //MINE_SWEEPER_CPP_SHELL_MYAI_HPP
