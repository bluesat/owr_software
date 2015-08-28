/*
	Class header for AutoGUI
*/

#ifndef AUTOGUI_H
#define AUTOGUI_H

#include "ListNode.h"
#include <list>
#include "GLUTWindow.h"
#include "GPSInputManager.h"

#define SCALE 40000

#define INPUT_BUFFER_SIZE 30
#define INPUT_DISABLED 0
#define INPUT_LAT 1
#define INPUT_LON 2

class AutoGUI : public GLUTWindow {
	public:
		AutoGUI(int width, int height, int *argc, char *argv[], double destPos[3][2]);
		void updateInfo(ListNode cur);
	private:
		void idle();
		void display();
		void keydown(unsigned char key, int x, int y);
		
		double targetLat;
		double targetLon;
		bool haveTargetLat;
		bool haveTargetLon;
		
		GPSInputManager *keymanager;
		
		void drawFullMap(double refLat, double refLon);
		void drawGPSPos();
		
		bool arrows[4];
		double dests[3][2];
		double mapCentre[2];
		vector3D currentPos;
		
		std::list<ListNode> path;
		std::list<ListNode> obstacles;
		
		// draws map for left side of the window
		void drawOverviewMap();
		// draws map for right side of the window
		void drawTrackingMap();
		
		// pointer to the ROS handler
		void *autoNode;
};

#endif // AUTOGUI_H