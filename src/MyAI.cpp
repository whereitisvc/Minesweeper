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
    agentX = _agentX;
    agentY = _agentY;

    board = new Tile*[cols];
        for ( int index = 0; index < cols; ++index )
            board[index] = new Tile[rows];

    actionQueue.assign({});

    lastAction = UNCOVER;
    board[agentX][agentY].uncovered = false;
    uncoverNeighborTiles(agentX, agentY);

    // startX = cols;
    // startY = -1;
    // endX = -1;
    // endY = rows;
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
        board[agentX][agentY].uncovered = false;
        board[agentX][agentY].number = number;
        if(number == 0) zeroTiles.push_back({agentX, agentY});
        else{ 
            edgeTiles.insert({agentX, agentY});
            // startX = min(agentX, startX);
            // startY = max(agentY, startY);
            // endX = max(agentX, endX);
            // endY = min(agentY, endY);
            // cout << "agent: " << agentX+1 << ", " << agentY+1 << endl;
            // cout << "start: " << startX+1 << ", " << startY+1 << endl;
            // cout << "end: " << endX+1 << ", " << endY+1 << endl;
        }
    }
    else if(lastAction == FLAG && board[agentX][agentY].uncovered){
        remain_mines--;
        board[agentX][agentY].flag = true;
        board[agentX][agentY].visited = true;
    }

    // in case doing the same action multiple times.
    while(!actionQueue.empty() && history.find({actionQueue[0].x, actionQueue[0].y}) != history.end()){
        actionQueue.erase(actionQueue.begin());
    }


    cout << "****************** AI INFO START ***********************" << endl;
    cout << "actionQueue size = " << actionQueue.size() << endl;

    // Uncover the neighbor tiles of "zero" tile first
    if(actionQueue.empty()){
        cout << endl << "-- zero Tiles case --" << endl;
        cout << "zeroTiles size = " << zeroTiles.size() << endl;
        int size = zeroTiles.size();
        for(auto tile: zeroTiles)
            uncoverNeighborTiles(tile.first, tile.second);
        zeroTiles.erase(zeroTiles.begin(), zeroTiles.begin()+size);
    }

    // Uncover/Flag the easy boundary tiles (can be easly inferred by one edge tile)
    if(actionQueue.empty()){
        cout << endl << "-- edge Tiles easy case --" << endl;
        cout << "edgeTiles size = " << edgeTiles.size() << endl;
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

        cout << endl << "-- edge Tiles medium case --" << endl;
        cout << "edgeTiles size = " << edgeTiles.size() << endl;

        vector<vector<pair<int, int>>> segment;
        edgeTilesSegment(segment);

        map<pair<int, int>, float> mine_stat;
        for(int i=0; i<segment.size(); i++){
            cout << endl << "area " << i  << " size = " << segment[i].size() << endl;

            vector<vector<Action>> configs = findMinesConfig(segment[i]);

            if(configs.empty()) 
                cout << "no config found" << endl;

            else if(configs.size() == 1){
                cout << "only one configuration found, great!" << endl;
                setConfig(configs[0]);
            }

            else{
                cout << "possible configs: " << configs.size() << endl;

                // caculate the possibility of mine for each edge tile
                caculateMineStat(mine_stat, configs);

                // check if have any 100% sure tile
                act2SafeTilebyStat(mine_stat);                
            }
        }

        // In this case, no 100% safe or 100% mine boundary tile. Make the best guess by probility
        if(actionQueue.empty()){
            cout << endl << "-- edge Tiles hard case --" << endl;
            bestGuessbyStat(mine_stat);
        }
    }

    cout << "****************** AI INFO END *************************" << endl << endl << endl << endl;
    return popActionQueue();
    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================

}


// ======================================================================
// YOUR CODE BEGINS
// ======================================================================

bool MyAI::neighbor(pair<int, int> a, pair<int, int> b){
    float dist = sqrt(  pow(a.first-b.first, 2) + pow(a.second-b.second, 2) );
    if(dist <= sqrt(2)) return true;
    return false;
}

void MyAI::edgeTilesSegment(vector<vector<pair<int, int>>>& segment){
    for(auto edg: edgeTiles){
        bool found = false;
        for(int i=0; i<segment.size(); i++){
            for(auto s: segment[i])
                if(neighbor(edg, s)){ 
                    segment[i].push_back(edg);
                    found = true;
                    break;
                }
            if(found) break;
        }
        if(!found) segment.push_back({edg});
    }
}

void MyAI::bestGuessbyStat(map<pair<int, int>, float>& stat){
    if(stat.empty()) return;
    float min = INT_MAX;
    pair<int, int> tile;
    for(auto it:stat){
        if(it.second < min){ 
            tile = it.first;
            min = it.second;
        }
    }
    cout << "min = " << min << ", (" << tile.first+1 << ", " << tile.second+1 << ")" << endl;
    actionQueue.push_back({UNCOVER, tile.first, tile.second});
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

void MyAI::dfsMines(vector<vector<Action>>& configs, vector<Action>& config, vector<pair<int, int>>& edgTiles, int index, int flagged){
    if(flagged > remain_mines) return;
    if(index == edgTiles.size()){
        //printConfig(config);
        if(!config.empty()) configs.push_back(config);
        return;
    }

    int x = edgTiles[index].first;
    int y = edgTiles[index].second;
    int number = board[x][y].number; 
    //cout << "x = " << x+1 << ", y = " << y+1 << ", number = " << number << endl;

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
        dfsMines(configs, config, edgTiles, index+1, flagged);
        return;
    }

    // caculate the number of mines need to be set
    int remain = number - mines;
    vector<int> ary (tiles.size(), 0);
    for(int i=0; i<remain; i++) ary[ary.size()-1-i] = 1;

    // for(auto a: ary) cout << a;
    // cout << endl;

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
        dfsMines(configs, config, edgTiles, index+1, flagged+remain);
        for(int i=0; i<tiles.size(); i++){
            tx = tiles[i].first;
            ty = tiles[i].second;
            board[tx][ty].visited = false;
            board[tx][ty].flag = false;
            config.pop_back();
        }
    }

}

vector<vector<Agent::Action>> MyAI::findMinesConfig(vector<pair<int, int>>& area){
    vector<vector<Action>> configs;
    vector<Action> config;
    vector<pair<int, int>> edgTiles (area.begin(), area.end());
    dfsMines(configs, config, edgTiles, 0, 0);
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

    int size = tiles.size(); //cout << x+1 << ", " << y+1 << " => " << size << endl;
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
