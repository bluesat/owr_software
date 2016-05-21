/* A Star search
 * By Nuno Das Neves
 * Date 5/3/2016 - 19/3/2016
 * A* path plan based off LIDAR point-cloud
 */

#include "Astar.h"


Astar::Astar(const std::string topic) {
    // shouldn't need to set any values..?
    
    // check topic names!
    aStarSubscriber = node.subscribe<nav_msgs::OccupancyGrid>("map", 2, &Astar::aStarCallback, this);
    //goalSubscriber = node.subscribe<geometry_msgs>("goal", 2, &Astar::setGoalCallback, this);
    //tfSubscriber = node.subscribe<geometry_msgs>("tf", 2, &Astar::setStartCallback, this);
}

void Astar::aStarCallback(const nav_msgs::OccupancyGrid::ConstPtr& gridData) {
    
    goal.isGoal = true;
    goal.x = 5;
    goal.y = 5;
    
    start.isStart = true;
    start.x = 0;
    start.y = 0;
    
    if (!goal.isGoal) {
        ROS_ERROR("Can't find goal!");
        return;
    }
    if (!start.isStart) {
        ROS_ERROR("Can't find start!");
        return;
    }
    
    // interpret data and put it in the occupancyGrid
    makeGrid((int8_t*)gridData->data.data(), gridData->info);
    
    findPath();         // do aStar algorithm, get the path
    convertPath();      //convert aStarPath to the path output we need
    
    pathPublisher.publish(finalPath);
}

void Astar::setGoalCallback(const geometry_msgs::Point::ConstPtr& thePoint) {
    
    //-------- test data ------
    goal.isGoal = true;
    goal.x = 5;
    goal.y = 5;
    //-------------------------
    
}

void Astar::setStartCallback(const geometry_msgs::Transform::ConstPtr& theTF) {
    // ---------- test only
    start.isStart = true;
    start.x = 0;
    start.y = 0;
    // ----------------
}

//main loop?
void Astar::spin() {
    while(ros::ok()) {
        ros::spinOnce();
    }
}

/* //test main
int main(int argc, char **argv) {
    std::cout << "A* search test" << std::endl;
    
    Astar test;
    
    test.setGridEndPoints(0,0,83,83);
    test.setGridStepCosts("middle");
    
    //test.printGrid();
    
    test.findPath();
    
    test.printGrid();
    
    return 0;
}
*/

void Astar::makeGrid(int8_t data[], nav_msgs::MapMetaData info) {
    
    if(info.width <= SIZE_OF_GRID && info.height <= SIZE_OF_GRID) {
        ROS_ERROR("nav_msgs::OccupancyGrid is too big!");
        return;
    }
    
    //set entire grid to impassable
    for(unsigned int i = 0; i < occupancyGrid.size(); ++i) {
        for(unsigned int j = 0; j < occupancyGrid[i].size(); ++j) {
            occupancyGrid[i][j] = IMPASS;
        }
    }
    
    int x = 0;
    int y = 0;
    char value;
    
    for (unsigned int i = 0; i < sizeof(data)/sizeof(int8_t); ++i) {
        value = data[i];
        if (value < 0) {
            value = IMPASS;    // set unknowns to impassable
        }
        occupancyGrid[x][y] = value;    // set the value
        
        x ++;   //always increment x
        
        // if we're at a multiple of the width, move to the next line
        if ((i % (info.width-1)) == 0) {    // need to subtract 1 because the i will be 1 less
            y ++;
            x = 0;  // reset x to the start of the row
        }
    }
}

void Astar::convertPath() {
    aStarPath;
    finalPath;
}


void Astar::setGridEndPoints(int sx, int sy, int gx,int gy) {
    start.isStart = true;
    start.x = sx;
    start.y = sy;

    goal.isGoal = true;
    goal.x = gx;
    goal.y = gy;
}

/* //-------- testing only ------------
void Astar::setGridStepCosts(char *typeOfObstacle) {
    
    // set initial costs to 1
    for(unsigned int i = 0; i < occupancyGrid.size(); ++i) {
        for(unsigned int j = 0; j < occupancyGrid[i].size(); ++j) {
            occupancyGrid[i][j]=1;
        }
    }
    
    int size = occupancyGrid.size();
    
    // add obstacles based on grid size
    if (typeOfObstacle  == "middle"){   // add a block in the middle
        if ((size % 2) == 0) {
            int start = (size/2) - 1;
            int end = (size/2) + 1;
            for(unsigned int p = start; p < end; ++p) {
                occupancyGrid[p][start] = IMPASS;
                occupancyGrid[p][start+1] = IMPASS;
            }
        } else {
             occupancyGrid[size/2][size/2] = IMPASS;
        }
    } else if (typeOfObstacle == "line") {  // add a line in the middle
        int start = (size/3);
        int end = (start*2) - 1;
        for(unsigned int p = start; p < end; ++p) {
            occupancyGrid[size/2][p] = IMPASS;
        }
    }
    
}

void Astar::printGrid(){
    std::cout << std::endl;
    // this is how these loops should be set up for correct indexing (pretty sure) i = x; j = y
    for(unsigned int j = 0; j < occupancyGrid[0].size(); ++j) {
      for(unsigned int i = 0; i < occupancyGrid.size(); ++i) {
        point printPoint(i,j);
        if (start.x == i && start.y == j) {
            std::cout << "S ";      // S for start
        } else if (goal.x == i && goal.y == j) {
            std::cout << "G ";      // G for goal
        } else if (occupancyGrid[i][j] == IMPASS) {
            std::cout << "B ";      // B for block
        } else if(std::find(finalPath.begin(), finalPath.end(), printPoint) != finalPath.end()){
            std::cout << "# ";      // # for final path
        } else {
            std::cout << "- ";      // - for empty (passable) grid square
        }
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
}
------------------------------------*/

void Astar::findPath() {
    currentPos = start;           // set our current position to the start position
    
    while(currentPos != goal) {
        
        closedSet.push_back(currentPos);    // push the current point into closedSet
        computeNeighbors();                 // get neighbors of current position
        
        if(openSet.empty()) {               // or if there is no solution (no more valid points)
            ROS_ERROR("No path found!");
            return;
        }
        
        //std::cout << "openSet.top() xy = (" << openSet[0].x << ", " << openSet[0].y << ")" << std::endl;
        //std::cout << "-------------------------" << std::endl;
        //std::cout << "  openSet.size()  = " << openSet.size()  << std::endl;
        //std::cout << "  openSet.top().previndex = " << openSet.top().previndex << std::endl;
        
        currentPos = openSet[0];     // set current position to the next in the priority queue
        
        std::pop_heap(openSet.begin(), openSet.end(), comp); openSet.pop_back();
        
        //std::cout << "---------------------------" << std::endl;
        if(currentPos == goal) {        // if we're at the goal
            getPath();
            break;
        }
    }
}

void Astar::computeNeighbors () {
    std::vector<point> neighbors = getNeighbors(currentPos);
    point adjacent;
    long index;
    std::vector<point>::iterator it;
    double maybeCost;
    
    if (!(neighbors.empty())) {
        for (int i = 0; i < neighbors.size(); ++i) {
            
            adjacent = neighbors[i];
            maybeCost = currentPos.cost + adjacent.stepCost; // get the total cost up to this point
            it = std::find(openSet.begin(), openSet.end(), adjacent);
            
            if (it != openSet.end()){  // check if adjacent is in frontierSet 
                index = it - openSet.begin(); // should return the index of the point...? i guess..?
                
                if (maybeCost >= openSet[index].cost) {
                    continue;   // if in frontierSet (or openSet); continue
                }
                
                //std::cout << "----updating this point xy = (" << frontierSet[frontIndex].x << ", " << frontierSet[frontIndex].y << ")" << std::endl;
                //std::cout << "----with cost = " << maybeCost << std::endl;
                //std::cout << "----which is this index in frontierSet = " << frontIndex << std::endl;
                //std::cout << "----which has this adjacent xy= " << adjacent.x << ", " << adjacent.y << ")" << std::endl;
                openSet[index].cost = maybeCost;
                getF(openSet[index]);
                
            } else {
                adjacent.cost = maybeCost;
                getF(adjacent);                             // get the F cost etc
                openSet.push_back(adjacent); std::push_heap(openSet.begin(), openSet.end(), comp);        // push it to the openSet for future expansion
                
            }      
        }
    }
}

std::vector<point> Astar::getNeighbors(point point1) {
    
    std::vector<point> neighbors;
    point adjacent;
    
    for(int i = point1.x-1; ((i <= point1.x+1) && (i < (int)occupancyGrid.size())); ++i) {  // need to typecast cos i can be negative
        for(int j = point1.y-1;((j <= point1.y+1) && (j < (int)occupancyGrid.size())); ++j) {
            
            adjacent.x = i;
            adjacent.y = j;
            
            if(std::find(closedSet.begin(), closedSet.end(), adjacent) == closedSet.end() && (i >= 0) && (j >= 0)) {
                
                //std::cout << "i,j = (" << i << ", " << j << ")" << std::endl;

                if (occupancyGrid[i][j] < IMPASS) {   // if its passable..
                    
                    if ((point1.x == i) || (point1.y == j)) {  
                        //std::cout << "adding a neighbor; adjacent" << std::endl;
                        adjacent.stepCost = occupancyGrid[i][j];    // if adjacent, use normal stepCost
                        neighbors.push_back(adjacent);
                        
                    } else if (canGoDiagonal(point1, adjacent) == true) {
                        //std::cout << "adding a neighbor; diag" << std::endl;
                        // otherwise just use the diagonal stepCost
                        adjacent.stepCost = occupancyGrid[i][j]*sqrt(2);    // if diagonal, multiply stepCost by sqrt(2) because that's how geometry works
                        neighbors.push_back(adjacent);
                        
                    }
                } else { // if the point is impassible, push it to the closedSet (we never want to expand it)
                    closedSet.push_back(adjacent);
                }
            }
        }
    }
    return neighbors;
}

bool Astar::canGoDiagonal(point point1, point point2) {
    // ---this section checks blocks next to adjacent and point1 to see if they're blocked; if both are then it treats adjacent as impassable
    int xset = point1.x+(point2.x - point1.x);
    int yset = point1.y+(point2.y - point1.y);
    // are xset and yset within the grid (are they real points)
    if ((xset >= 0 && xset < occupancyGrid.size()) && (yset >= 0 && yset < occupancyGrid.size())) {
        // now, are those points both impassable? if so, return false
        if ((occupancyGrid[xset][point1.y] == IMPASS) && (occupancyGrid[point1.x][yset] == IMPASS)) {
            return false;
        }
    }
    return true;
}

void Astar::getF (point &point1) {
    point1.goalDist = getDist(point1, goal);      // get distance to goal (our only heuristic right now)
    //point1.cost = currentPos.cost + point1.stepCost;  // get the total cost up to this point (now done in getNeighbors)
    point1.weight =  point1.cost + point1.goalDist;   // f = g + h
    point1.previndex = std::find(closedSet.begin(), closedSet.end(), currentPos) - closedSet.begin(); // returns index of a point matching x,y of currentPos in closedSet
    //std::cout << " " << std::endl;
    //std::cout << "adjacent xy = (" << point1.x << ", " << point1.y << ")" << std::endl;
    //std::cout << "  currentPos.cost = " << currentPos.cost << std::endl;
    //std::cout << "  cost = " << point1.cost << std::endl;
    //std::cout << "  goalDist = " << point1.goalDist << std::endl;
    //std::cout << "  weight = " << point1.weight << std::endl;
    //std::cout << "  previndex xy = (" << closedSet[point1.previndex].x << ", " << closedSet[point1.previndex].y << ")" << std::endl;
}

double Astar::getDist (point point1, point point2) {
    double dist = sqrt(pow((point1.x - point2.x), 2) + pow((point1.y - point2.y), 2)); // euclidean distance
    return dist;
}

void Astar::getPath() {
    while(currentPos.previndex != -1){
        aStarPath.push_back(currentPos);
        //std::cout << "finalpath xy = (" << currentPos.x << ", " << currentPos.y << ")" << std::endl;
        //std::cout << "  finalpath previndex = " << currentPos.previndex << std::endl;
        currentPos = closedSet[currentPos.previndex];
    }
    //std::cout << "finalpath xy = (" << currentPos.x << ", " << currentPos.y << ")" << std::endl;
    //std::cout << "  finalpath previndex = " << currentPos.previndex << std::endl;
}