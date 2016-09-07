/* A Star search
 * By Nuno Das Neves
 * Date 5/3/2016 - 9/9/2016
 * A* path plan based off SLAM occupancy grid
 */

#include "Astar.h"

int main (int argc, char *argv[]) {
    
    ros::init(argc, argv, "owr_astar");
    //std::cout << "ros init'd" << std::endl;
    Astar pathFinder("owr_auton_pathing");
    pathFinder.spin();
    return 0;
}

// TODO fix for transform
Astar::Astar(const std::string topic) : node(), tfSubscriber(node, "map", 1), tfFilter(tfSubscriber, tfListener, "base_link", 1) {
    getMap();
    mapSubscriber = node.subscribe<nav_msgs::OccupancyGrid>("map", 2, &Astar::mapCallback, this);
    
    mapPublisher = node.advertise<nav_msgs::OccupancyGrid>("astarOccupancyGrid", 2, true);
    pathPublisher = node.advertise<nav_msgs::Path>("astarPath", 2, true);
    
    goalSubscriber = node.subscribe<geometry_msgs::PointStamped>("owr_auton_pathing/astargoal", 2, &Astar::setGoalCallback, this);
    
    // TODO fix this: should be false by default; use callback to update
    go = true;
    //goSubscriber = node.subscribe<std_msgs::Bool>("owr_auton_pathing/astarstart", 2, &Astar::setGoCallback, this);
    
    // TODO this is for getting the tranform PUT THIS BACK!
    tfFilter.registerCallback(boost::bind(&Astar::tfCallback, this, _1));
}



// gets a transform from SLAM (between map and base_link)
void Astar::tfCallback(const nav_msgs::OccupancyGrid::ConstPtr& gridData) {
    
    tfListener.lookupTransform("map","base_link", gridData->header.stamp, transform);
   
    start.x = (int)round(transform.getOrigin().getX() + GRID_OFFSET)*GRID_FACTOR;
    start.x = (int)round(transform.getOrigin().getY() + GRID_OFFSET)*GRID_FACTOR;    
    start.isStart = true;
    
    ROS_INFO("base_link at (%f, %f)", transform.getOrigin().getX(), transform.getOrigin().getY());
    ROS_INFO("Rover tf at (%d, %d)", start.x, start.y);
    
    // this gets the map from SLAM, which we're not doing anymore
    //makeGrid((int8_t*)gridData->data.data(), gridData->info);
    
    finalPath.header = gridData->header;
    finalPath.header.stamp = ros::Time::now();
    
    //TODO make sure this works
    // ??? geotiff origin? a pose in the real world of the centre of the map
    outputGrid.info.origin.position = gridData->info.origin.position;
    outputGrid.header = gridData->header;
    
    if (go == true) {
        doSearch();
    }
}

// just gets a map from SLAM
void Astar::mapCallback(const nav_msgs::OccupancyGrid::ConstPtr& gridData) {
    
    ROS_INFO ("Got a SLAM map");
    start.isStart = true;
    finalPath.header = gridData->header;
    finalPath.header.stamp = ros::Time::now();
    
    outputGrid.info.origin.position = gridData->info.origin.position;
    outputGrid.header = gridData->header;
    
    if (go == true) {
        doSearch();
    }
}

void Astar::setGoalCallback(const geometry_msgs::PointStamped::ConstPtr& thePoint) {
    
    goal.isGoal = true;
    goal.x = (1/GRID_SCALE)*(thePoint->point.x + GRID_OFFSET);
    goal.y = (1/GRID_SCALE)*(thePoint->point.y + GRID_OFFSET);
    ROS_INFO("Astar goal at (%d, %d)", goal.x, goal.y);
    
    if (go == true) {
        doSearch();
    }
    
}

void Astar::setGoCallback(const std_msgs::Bool::ConstPtr& goOrNo) {
    
    go = goOrNo->data;
    if (go == false) {
        ROS_INFO("Astar paused");
    } else {
        ROS_INFO("Astar doing");
        doSearch();
    }
}

void Astar::doSearch() {
    
    // if it's a nogo, don't go!
    if (go == true && goal.isGoal && start.isStart) {
        
        // publish the occupancyGrid we generated
        ROS_INFO ("Publishing a map");
        mapPublisher.publish(outputGrid);
        
        ROS_INFO("Attempting to find path...");
        if (!findPath()) { // do aStar algorithm, get the path
            ROS_ERROR("No path found!");
            return;
        }
        //printGrid();        // testing only
        convertPath();      // convert aStarPath to the path output we need
        
        clearPaths();
        ROS_INFO("Publishing a path");
        pathPublisher.publish(finalPath);
    }
}

//ros's main loop thing?
void Astar::spin() {
    //std::cout << "spinning" << std::endl;
    while(ros::ok()) {
        ros::spinOnce();
    }
}


void Astar::getMap() {
    // load image from file
    cv::Mat image;
    image = cv::imread(IMG_PATH, CV_LOAD_IMAGE_GRAYSCALE);   // Read the file
    cv::Mat bimage = image.clone();
    cv::Mat fimage = image.clone();
    
    if(!image.data)                              // Check for invalid input
    {
        ROS_ERROR ("Could not open or find %s", IMG_PATH);
        return;
    }
    
    int tot;
    int howmany = 0;
    double alpha = 1.5f;
        
    for(int c=0; c < image.cols; ++c) {
        for(int r=0; r < image.rows; ++r) {
            image.at<uchar>(r,c) = cv::saturate_cast<uchar>(alpha*(image.at<uchar>(r,c)));
            if (image.at<uchar>(r,c) > 250) image.at<uchar>(r,c) = 0;
        }
    }
    
    //blur image with cv's GaussianBlur
    for ( int i = 1; i < 21; i += 2 ) cv::GaussianBlur(image, bimage, cv::Size(i, i), 0, 0);
    
    // This gives us the gradient/ differential map. Whatever its called. High pass filter?
    for(int c=0; c < image.cols; ++c) {
        for(int r=0; r < image.rows; ++r) {
            // loop through adjacent pixels
            for(int co=-11; co <= 11; ++co) {
                for(int ro=-11; ro <= 11; ++ro) {
                    // if within a circle around our pixel,
                    if (r+ro < image.rows && c+co < image.cols
                        && r+ro >= 0 && c+co >= 0 && (ro != 0 || co != 0)
                        && sqrt((ro*ro)+(co+co)) <= 11)
                    {
                        // count em up for cases where we go over the edge (so the average is computed accurately)
                        howmany ++;
                        // add to the total
                        tot += abs(bimage.at<uchar>(r,c) - bimage.at<uchar>(r+ro,c+co));
                    }
                }
            }
            // get average and stick in new image
            fimage.at<uchar>(r,c) = cv::saturate_cast<uchar>(tot/howmany);
            // reset stuff
            howmany = 0;
            tot = 0;
        }
    }
    ROS_INFO ("made diff map");
    
    // linear filter to bump up the whites
    for(int c=0; c < image.cols; ++c) {
        for(int r=0; r < image.rows; ++r) {
            fimage.at<uchar>(r,c) = cv::saturate_cast<uchar>(1.8f*(fimage.at<uchar>(r,c)));
        }
    }
    ROS_INFO ("brightened");
    
    
    // Do the radius thing - this will surround impassable pixels with almost impassable pixels 
    // This gives a buffer zone for the rover
    for(int c=0; c < image.cols; ++c) {
        for(int r=0; r < image.rows; ++r) {
            // if the pixel is over our threshold
            if (fimage.at<uchar>(r,c) > 100) {
                // loop through adjacent pixels
                for(int co=-6; co <= 6; ++co) {
                    for(int ro=-6; ro <= 6; ++ro) {
                        // make sure its within a circle around our pixel, and lower than the threshold
                        if (r+ro < image.rows && c+co < image.cols
                            && r+ro >= 0 && c+co >= 0 && (ro != 0 || co != 0)
                            && sqrt((ro*ro)+(co+co)) <= 6 && fimage.at<uchar>(r+ro,c+co) < 100) {
                            // set to threshold
                            fimage.at<uchar>(r+ro,c+co) = 100;
                        }
                    }
                }
            }
        }
    }
    ROS_INFO ("did radius thing");
    //cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
    //cv::imshow( "Display window", fimage );                   // Show our image inside it.
    //cv::waitKey(0);                                          // Wait for a keystroke in the window
    
    // free da memory
    image.release();
    bimage.release();
    
    // resize astarGrid
    astarGrid.resize(fimage.cols);
    //set entire grid to impassable
    for(unsigned int i = 0; i < astarGrid.size(); ++i) {
        astarGrid[i].resize(fimage.rows);
        for(unsigned int j = 0; j < astarGrid[i].size(); ++j) {
            astarGrid[i][j] = IMPASS;
        }
    }
    
    
    outputGrid.info.width=fimage.cols;
    outputGrid.info.height=fimage.rows;
    // each pixel is 0.090375m
    outputGrid.info.resolution = 0.090375;
    outputGrid.info.map_load_time = ros::Time::now();
    // resize the data vector to the right size (???????)
    std::vector<int8_t> vec;
    vec.resize(fimage.rows*fimage.cols);
    outputGrid.data = vec;
    
    // TODO remove these; use transform instead
    start.x = fimage.cols/2;
    start.y = fimage.rows/2;
    //start.isStart = true;
    
    int i = 0;
    unsigned char value;
    // loopy loop through each pixel in greyscale
    
    for(int r=0; r < fimage.rows; r++) {
        for(int c=0; c < fimage.cols; c++) {
            // pretty sure this is the right way around
            value = (int)fimage.at < uchar>(r,c);
            astarGrid[c][r] = value;
            // populate the grid we're going to publish
            outputGrid.data[i] = value;
            i++;
        }
    }
}

void Astar::makeGrid(int8_t data[], nav_msgs::MapMetaData info) {

    if((info.width >= SIZE_OF_GRID) || (info.height >= SIZE_OF_GRID)) {
        ROS_ERROR("nav_msgs::OccupancyGrid is too big!");
        return;
    }
    astarGrid.resize(info.width);
    //set entire grid to impassable
    for(unsigned int i = 0; i < astarGrid.size(); ++i) {
        astarGrid[i].resize(info.height);
        for(unsigned int j = 0; j < astarGrid[i].size(); ++j) {
            astarGrid[i][j] = IMPASS;
        }
    }
    
    
    int x = 0;
    int y = 0;
    char value;
        
    for (unsigned int i = 0; i < (info.width * info.height); ++i) {
        value = data[i];
        
        if (value < 0) {
            value = 100;    // set unknowns to somewhat passable
        }
        astarGrid[x][y] = value;    // set the value
        
        // radius thingy thing
        if (value > IMPASS_THRESHOLD) {
            astarGrid[x][y] = IMPASS;
            // loop through all points potentially in the circle
            for (int c = -IMPASS_RADIUS+1; c < IMPASS_RADIUS; c ++) {
                for (int r = IMPASS_RADIUS-1; r > -IMPASS_RADIUS; r--) {
                    int cx = x+c;
                    int ry = y+r;
                    // make sure the point is in the circle and not outside the astarGrid
                    if (cx < astarGrid.size() && cx >= 0 && ry < astarGrid[0].size() && ry >=0
                        && sqrt((c*c)+(r*r)) <= IMPASS_RADIUS && astarGrid[cx][ry] < IMPASS_THRESHOLD) {
                        astarGrid[cx][ry] = IMPASS_THRESHOLD;
                    }
                }
            }
        }
        
        x ++;   //always increment x
        // if we're at a multiple of the width, move to the next line
        if (x == info.width) {
            y ++;
            x = 0;  // reset x to the start of the row
        }
    }
    
}

void Astar::setGridEndPoints(int sx, int sy, int gx,int gy) {
    start.isStart = true;
    start.x = sx;
    start.y = sy;

    goal.isGoal = true;
    goal.x = gx;
    goal.y = gy;
}

/* //------ testing mainly ------------
void Astar::setGridStepCosts(char *typeOfObstacle) {
    
    // set initial costs to 1
    for(unsigned int i = 0; i < astarGrid.size(); ++i) {
        for(unsigned int j = 0; j < astarGrid[i].size(); ++j) {
            astarGrid[i][j]=1;
        }
    }
    
    int size = astarGrid.size();
    
    // add obstacles based on grid size
    if (typeOfObstacle  == "middle"){   // add a block in the middle
        if ((size % 2) == 0) {
            int start = (size/2) - 1;
            int end = (size/2) + 1;
            for(unsigned int p = start; p < end; ++p) {
                astarGrid[p][start] = IMPASS;
                astarGrid[p][start+1] = IMPASS;
            }
        } else {
             astarGrid[size/2][size/2] = IMPASS;
        }
    } else if (typeOfObstacle == "line") {  // add a line in the middle
        int start = (size/3);
        int end = (start*2) - 1;
        for(unsigned int p = start; p < end; ++p) {
            astarGrid[size/2][p] = IMPASS;
        }
    }
    
}
*/
void Astar::printGrid(){
    std::cout << std::endl;
    // this is how these loops should be set up for correct indexing (pretty sure) i = x; j = y
    /*for(unsigned int j = 0; j < astarGrid[0].size(); ++j) {
      for(unsigned int i = 0; i < astarGrid.size(); ++i) {
        point printPoint(i,j);
        if (start.x == i && start.y == j) {
            std::cout << "S ";      // S for start
        } else if (goal.x == i && goal.y == j) {
            std::cout << "G ";      // G for goal
        } else if (astarGrid[i][j] == IMPASS) {
            std::cout << "B ";      // B for block
        } else if(std::find(aStarPath.begin(), aStarPath.end(), printPoint) != aStarPath.end()){
            std::cout << "# ";      // # for final path
        } else {
            std::cout << "- ";      // - for empty (passable) grid square
        }
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
    */
    // print out the actual path
    std::cout << "aStarPath = [";
    for (unsigned int i = 0; i < aStarPath.size(); ++i) {
        std::cout << "(" << aStarPath[i].x << "," << aStarPath[i].y << ")";
        if (i < aStarPath.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}
/*------------------------------------*/

bool Astar::findPath() {
    currentPos = start;           // set our current position to the start position
    
    printf ("start = (%d,%d)\n",start.x,start.y);
    printf ("goal = (%d,%d)\n",goal.x,goal.y);
    
    while(currentPos != goal) {
        
        closedSet.push_back(currentPos);    // push the current point into closedSet
        computeNeighbors();                 // get neighbors of current position
        
        if(openSet.empty()) {               // or if there is no solution (no more valid points)
            return false;
        }
        
        currentPos = openSet[0];     // set current position to the next in the priority queue
        
        std::pop_heap(openSet.begin(), openSet.end(), comp); openSet.pop_back();
        
        //std::cout << "---------------------------" << std::endl;
        if(currentPos == goal) {        // if we're at the goal
            getPath();
            break;
        }
    }
    return true;
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
    
    for(int i = point1.x-1; ((i <= point1.x+1) && (i < (int)astarGrid.size())); ++i) {  // need to typecast cos i can be negative
        for(int j = point1.y-1;((j <= point1.y+1) && (j < (int)astarGrid.size())); ++j) {
            
            adjacent.x = i;
            adjacent.y = j;
            
            if(std::find(closedSet.begin(), closedSet.end(), adjacent) == closedSet.end() && (i >= 0) && (j >= 0)) {
                
                //std::cout << "i,j = (" << i << ", " << j << ")" << std::endl;

                if (astarGrid[i][j] < IMPASS) {   // if its passable..
                    
                    if ((point1.x == i) || (point1.y == j)) {  
                        //std::cout << "adding a neighbor; adjacent" << std::endl;
                        adjacent.stepCost = astarGrid[i][j];    // if adjacent, use normal stepCost
                        neighbors.push_back(adjacent);
                        
                    } else if (canGoDiagonal(point1, adjacent) == true) {
                        //std::cout << "adding a neighbor; diag" << std::endl;
                        // otherwise just use the diagonal stepCost
                        adjacent.stepCost = astarGrid[i][j]*sqrt(2);    // if diagonal, multiply stepCost by sqrt(2) because that's how geometry works
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
    if ((xset >= 0 && xset < astarGrid.size()) && (yset >= 0 && yset < astarGrid.size())) {
        // now, are those points both impassable? if so, return false
        if ((astarGrid[xset][point1.y] == IMPASS) && (astarGrid[point1.x][yset] == IMPASS)) {
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
    std::vector<point>::iterator it;
    
    while(currentPos.previndex != -1){
        it = aStarPath.begin();
        
        aStarPath.insert(it, currentPos);
        
        currentPos = closedSet[currentPos.previndex];
    }
    //std::cout << "finalpath xy = (" << currentPos.x << ", " << currentPos.y << ")" << std::endl;
    //std::cout << "  finalpath previndex = " << currentPos.previndex << std::endl;
}

void Astar::convertPath() {
    geometry_msgs::PoseStamped thisPose;
    thisPose.header = finalPath.header;
    finalPath.poses.resize(aStarPath.size());
    for (unsigned int i = 0; i < aStarPath.size(); ++i) {
        thisPose.header.seq = i;
        // math/meth
        thisPose.pose.position.x = ((double)aStarPath[i].x/(1/GRID_SCALE)) - GRID_OFFSET;
        thisPose.pose.position.y = ((double)aStarPath[i].y/(1/GRID_SCALE)) - GRID_OFFSET;
        
        finalPath.poses[i] = thisPose;
    }
}

void Astar::clearPaths() {
    closedSet.clear();
    openSet.clear();
    frontierSet.clear();
    aStarPath.clear();
}