// ======================================================================
// FILE:        MyAI.cpp
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

#include "MyAI.hpp"

MyAI::MyAI ( int _rowDimension, int _colDimension, int _totalMines, int _agentX, int _agentY ) : Agent()
{
    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================
    rows = _rowDimension;
    cols = _colDimension;
    remain_mines = _totalMines;
    remain_tiles = rows * cols - 1;
    agentX = _agentX;
    agentY = _agentY;

    close = false;

    board = new Tile*[cols];
    for ( int index = 0; index < cols; ++index )
        board[index] = new Tile[rows];

    actionQueue.assign({});

    lastAction = UNCOVER;
    board[agentX][agentY].uncovered = false;
    uncoverNeighborTiles(agentX, agentY);

    srand( time(NULL) );

    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================
};

Agent::Action MyAI::getAction( int number )
{
    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================

    // precept the response from the environment
    if(lastAction == UNCOVER && board[agentX][agentY].uncovered){
        remain_tiles--;
        board[agentX][agentY].uncovered = false;
        board[agentX][agentY].number = number;
        if(number == 0) zeroTiles.push_back({agentX, agentY});
        else{ 
            edgeTiles.insert({agentX, agentY});
        }
    }
    else if(lastAction == FLAG && board[agentX][agentY].uncovered){
        remain_tiles--;
        remain_mines--;
        board[agentX][agentY].flag = true;
        board[agentX][agentY].visited = true;
    }

    // in case doing the same action multiple times.
    while(!actionQueue.empty() && history.find({actionQueue[0].x, actionQueue[0].y}) != history.end()){
        actionQueue.erase(actionQueue.begin());
    }

    // Uncover the neighbor tiles of "zero" tile first
    if(actionQueue.empty()){
        int size = zeroTiles.size();
        for(auto tile: zeroTiles)
            uncoverNeighborTiles(tile.first, tile.second);
        zeroTiles.erase(zeroTiles.begin(), zeroTiles.begin()+size);
    }

    // Uncover/Flag the easy boundary tiles (can be easly inferred by one edge tile)
    if(actionQueue.empty()){
        vector<pair<int, int>> blacklist;
        for(auto tile: edgeTiles){
            bool keep = checkBoundTiles(tile.first, tile.second);
            if(!keep) blacklist.push_back(tile);
        }
        for(auto tile: blacklist){
            edgeTiles.erase(tile);
        }
    }

    // Caculate all the configurations and make the best decision
    if(actionQueue.empty()){

        vector<vector<pair<int, int>>> segment;
        edgeTilesSegment(segment);

        map<pair<int, int>, float> mine_stat;
        int total_min = 0;
        for(int i=0; i<segment.size(); i++){

            int min_mine = INT_MAX;
            vector<vector<Action>> configs = findMinesConfig(segment[i], min_mine);
            total_min += (min_mine == INT_MAX) ? 0 : min_mine;

            if(configs.empty()) ;
            else if(configs.size() == 1){
                setConfig(configs[0]);
            }
            else{
                // caculate the possibility of mine for each edge tile
                caculateMineStat(mine_stat, configs);

                // check if have any 100% sure tile
                act2SafeTilebyStat(mine_stat);                
            }
        }

        // In this case, no 100% safe or 100% mine boundary tile. Make the best guess by probility
        if(actionQueue.empty()){
            bool correct = bestGuessbyStat(mine_stat, total_min);
            if(!correct){
                close = true;
                int min_mine = INT_MAX;
                vector<pair<int, int>> edgs (edgeTiles.begin(), edgeTiles.end());
                vector<vector<Action>> configs = findMinesConfig(edgs, min_mine);
                if(configs.size() == 1) setConfig(configs[0]);
                else{
                    caculateMineStat(mine_stat, configs);
                    act2SafeTilebyStat(mine_stat);                
                }
                if(actionQueue.empty()) bestGuessbyStat(mine_stat, total_min);
                close = false;
            }
        }
    }

    return popActionQueue();
    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================

}


// ======================================================================
// YOUR CODE BEGINS
// ======================================================================

bool MyAI::unexplore(int x, int y){
    if(!board[x][y].uncovered || board[x][y].flag) return false;
    for(auto edg: edgeTiles){
        if(neighbor(edg, {x, y})) return false;
    }
    return true;
}

bool MyAI::neighbor(pair<int, int> a, pair<int, int> b){
    int dist = abs(a.first - b.first) + abs(a.second - b.second);
    if(dist > 2) return false;
    else if(dist == 2){
        int x = abs(a.first - b.first);
        int y = abs(a.second - b.second);
        if(x == 2 || y == 2) return false;
    }
    return true;
}

bool MyAI::local(pair<int, int> a, pair<int, int> b){
    int dist = abs(a.first - b.first) + abs(a.second - b.second);
    if(dist > 3) return false;
    else if(dist == 3){
        int x = abs(a.first - b.first);
        int y = abs(a.second - b.second);
        if(x == 3 || y == 3) return false;
    }
    return true;
}

void MyAI::edgeTilesSegment(vector<vector<pair<int, int>>>& segment){
    if(edgeTiles.empty()) return;
    
    // index map to edgeTile 
    map<int, pair<int, int>> idx2tile;
    int idx = 0;
    for(auto edg: edgeTiles){
        idx2tile.insert({idx++, edg});
    }

    // Union Find
    vector<int> parent(idx, 0);
    for(int i=0; i<parent.size(); i++){
        parent[i] = i;
    }

    for(int i=0; i<parent.size()-1; i++){
        for(int j=i+1; j<parent.size(); j++){
            if(i == j) continue;
            if( local(idx2tile[i], idx2tile[j]) ){
                // merge two set
                if(parent[i] < parent[j]) parent[j] = parent[i];
                else parent[i] = parent[j];
            }
        }
    }

    // identify the group of each edge tile
    map<int, vector<int>> groups;
    for(int i=0; i<parent.size(); i++){
        int root = i;
        while(root != parent[root]) root = parent[root];
        if(groups.find(root) == groups.end())
            groups.insert( {root, {i}} );
        else
            groups[root].push_back(i);
    }

    int i = 0;
    for(auto iter: groups){
        segment.push_back({});
        for(auto idx: iter.second){
            segment[i].push_back(idx2tile[idx]);
        }
        i++;
    }

}

bool MyAI::bestGuessbyStat(map<pair<int, int>, float>& stat, int& total_min){

    // get the bound tile with smallest prob
    float min = INT_MAX;
    pair<int, int> tile;
    for(auto it:stat){
        if(it.second < min){ 
            tile = it.first;
            min = it.second;
        }
    }

    if(close){
        if(!stat.empty()) actionQueue.push_back({UNCOVER, tile.first, tile.second});
        return true;
    }


    // caculate the unexplored area
    int unexp_mines = remain_mines - total_min; // the largest number of mines in unexplored area
    vector<pair<int, int>> unexp_tiles; // the tiles in unexplored area
    for(int i=0; i<cols; i++){
        for(int j=0; j<rows; j++){
            if(unexplore(i, j)){
                unexp_tiles.push_back({i, j});
            }
        }
    }

    // game is finished
    if(stat.empty() && unexp_tiles.empty()) return true;

    // this is the special case when closing to the end of game
    if(unexp_tiles.empty() && unexp_mines > 0) return false;

    // all unexplore tile is safe
    if(!unexp_tiles.empty() && unexp_mines == 0){
        for(auto tile: unexp_tiles){
            actionQueue.push_back({UNCOVER, tile.first, tile.second});
        }
        return true;
    }

    // no unexplored tile
    if(unexp_tiles.empty()){
        actionQueue.push_back({UNCOVER, tile.first, tile.second});
        return true;
    }

    // no edgeTile ?
    if(stat.empty()){
        cout << "error" << endl;
        return false;
    }

    // unexplored area  vs  explored area
    float prob = (float) unexp_mines / unexp_tiles.size();
    if(min < prob){
        actionQueue.push_back({UNCOVER, tile.first, tile.second});
    }
    else{
        int ri = rand() % unexp_tiles.size();
        actionQueue.push_back({UNCOVER, unexp_tiles[ri].first, unexp_tiles[ri].second});
    }

    return true; 
}

void MyAI::act2SafeTilebyStat(map<pair<int, int>, float>& stat){
    for(auto it: stat){
        if(it.second == 0){
            int x = it.first.first;
            int y = it.first.second;
            actionQueue.push_back({UNCOVER, x, y});
        }
        else if(it.second == 1){
            int x = it.first.first;
            int y = it.first.second;
            actionQueue.push_back({FLAG, x, y});
        }
    }
}

void MyAI::caculateMineStat(map<pair<int, int>, float>& stat, vector<vector<Action>>& configs){
    set<pair<int, int>> set;
    for(auto config: configs){
        for(auto act: config){
            pair<int, int> key {act.x, act.y};
            if(stat.find(key) == stat.end()) stat.insert({key, 0});
            if(act.action == FLAG){
                stat[key]++;
                set.insert(key);
            }
        }
    }

    for(auto key: set){    
        stat[key] /= configs.size(); 
        //cout << stat[key] << endl; 
    }
}

void MyAI::setConfig(vector<Action> config){
    for(auto& act: config) 
        actionQueue.push_back(act);
}

void MyAI::printConfig(vector<Action> config){
    cout << "found config." << endl;
    for(auto act: config){
        cout << "action: ";
        if(act.action == UNCOVER) cout << "UNCOVER, ";
        else cout << "FLAG, ";
        cout << act.x+1 << ", " << act.y+1 << endl;
    }
    cout << endl << endl;
}

vector<vector<int>> MyAI::getCombination(vector<int> ary){
    //sort(ary.begin(), ary.end());
    vector<vector<int>> res;
    do{
        res.push_back(ary);
    }while ( std::next_permutation(ary.begin(),ary.end()) );
    return res;
}

void MyAI::dfsMines(vector<vector<Action>>& configs, vector<Action>& config, vector<pair<int, int>>& edgTiles, int index, int flagged, int& min_mine){
    if(flagged > remain_mines) return;
    if(index == edgTiles.size()){
        //printConfig(config);
        if(!config.empty() && close && flagged != remain_mines) return;

        if(!config.empty()) {
            configs.push_back(config);

            int count = 0;
            for(auto a: config) if(a.action == FLAG) count++;
            if(count < min_mine) min_mine = count;
        }
        return;
    }

    int x = edgTiles[index].first;
    int y = edgTiles[index].second;
    int number = board[x][y].number; 

    // find the boundary tiles
    int mines = 0;
    vector<pair<int, int>> tiles;
    for(int j=min(y+1, rows-1); j>=max(y-1, 0); j--){
        for(int i=max(x-1, 0); i<=min(x+1, cols-1); i++){
            if(board[i][j].flag) mines++;
            if(board[i][j].uncovered && !board[i][j].visited){
                tiles.push_back({i, j});
            }
        }
    }

    if(mines > number) return;
    if(mines + tiles.size() < number) return;
    if(tiles.empty()){ 
        dfsMines(configs, config, edgTiles, index+1, flagged, min_mine);
        return;
    }

    // caculate the number of mines need to be set
    int remain = number - mines;
    vector<int> ary (tiles.size(), 0);
    for(int i=0; i<remain; i++) ary[ary.size()-1-i] = 1;


    vector<vector<int>> combinations = getCombination(ary);
    int tx, ty;
    for(auto& comb: combinations){
        for(int i=0; i<tiles.size(); i++){
            tx = tiles[i].first;
            ty = tiles[i].second;
            if(comb[i] == 1){ // mine
                config.push_back({FLAG, tx, ty});
                board[tx][ty].flag = true;
                board[tx][ty].visited = true;
            }
            else{ // no mine
                config.push_back({UNCOVER, tx, ty});
                board[tx][ty].visited = true;
            }
        }
        dfsMines(configs, config, edgTiles, index+1, flagged+remain, min_mine);
        for(int i=0; i<tiles.size(); i++){
            tx = tiles[i].first;
            ty = tiles[i].second;
            board[tx][ty].visited = false;
            board[tx][ty].flag = false;
            config.pop_back();
        }
    }

}

vector<vector<Agent::Action>> MyAI::findMinesConfig(vector<pair<int, int>>& area, int& min_mine){
    vector<vector<Action>> configs;
    vector<Action> config;
    vector<pair<int, int>> edgTiles (area.begin(), area.end());
    dfsMines(configs, config, edgTiles, 0, 0, min_mine);
    //edgeTiles.clear();
    return configs;
}

bool MyAI::checkBoundTiles(int x, int y){
    // find the boundary tiles
    int number = board[x][y].number;
    int mines = 0;
    vector<pair<int, int>> tiles;

    for(int j=y+1; j>=y-1; j--){
        for(int i=x-1; i<=x+1; i++){
            if(!isInBounds(i, j)) continue;
            if(board[i][j].flag) mines++;
            else if(board[i][j].uncovered && !board[i][j].visited){
                tiles.push_back({i, j});
            }
        }
    }

    int size = tiles.size();
    if(size == 0) // if have no uncovered bound tiles around the edge tile, discard it
        return false;

    else if(mines == number){  // if all the boundary tiles are not mine for sure, do it right now
        for(auto tile: tiles) 
            actionQueue.push_back({UNCOVER, tile.first, tile.second});
    }
    else if(size + mines == number){ // if all the boundary tiles are mine for sure, do it right now
        for(auto tile: tiles) 
            actionQueue.push_back({FLAG, tile.first, tile.second});
    }

    return true;

}

void MyAI::uncoverNeighborTiles(int c, int r){
    for(int j=min(r+1, rows-1); j>=max(r-1, 0); j--){
        for(int i=max(c-1, 0); i<=min(c+1, cols-1); i++){
            if(board[i][j].uncovered && !board[i][j].visited){
                actionQueue.push_back({UNCOVER, i, j});
                board[i][j].visited = true;
            }
        }
    }
};

Agent::Action MyAI::popActionQueue(){
    if(actionQueue.empty()) return {LEAVE,-1,-1};

    Action act;
    act = actionQueue.front();
    actionQueue.erase(actionQueue.begin());
    
    agentX = act.x;
    agentY = act.y;
    lastAction = act.action;
    history.insert({act.x, act.y});
    return act;
}

bool MyAI::isInBounds ( int c, int r )
{
    return ( 0 <= c && c < cols && 0 <= r && r < rows );
}


// ======================================================================
// YOUR CODE ENDS
// ======================================================================
