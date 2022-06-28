#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		unsigned int get_index(int x, int y);

		// these two bools determine the surface topology
		// TODO: add support for spheres? (x/y swap)
		bool flip_y = false;
		bool flip_x = false;

		std::vector<glm::vec3> vels;
		
		ofEasyCam cam;
		ofMesh grid;
		ofMesh grid_lines;
		int resx = 200;
		int resy = 200;
		bool doSim = false;
};
