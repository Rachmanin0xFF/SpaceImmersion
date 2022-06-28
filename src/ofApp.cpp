#include "ofApp.h"

// no flip = torus
// one flip = klein bottle (mobius cylinder)
// both flip = real projective plane
unsigned int ofApp::get_index(int x, int y) {
	while (x >= resx) {
		x -= resx;
		if (flip_y) y = resy - 1 - y;
	}
	while (x < 0) {
		x += resx;
		if (flip_y) y = resy - 1 - y;
	}
	while (y >= resy) {
		y -= resy;
		if (flip_x) x = resx - 1 - x;
	}
	while (y < 0) {
		y += resy;
		if (flip_x) x = resx - 1 - x;
	}
	return x * resy + y;
}

// spring force on p1 by p2
glm::vec3 spring(const glm::vec3 &p1, const glm::vec3 &p2, const float &len) {
	glm::vec3 a = p2 - p1;
	float d = glm::length(a);
	a /= d;
	a *= (d - len) * 0.1f;
	return a;
}

// repulsion force on p1 by p2
glm::vec3 repulse(const glm::vec3& p1, const glm::vec3& p2) {
	glm::vec3 a = p2 - p1;
	float d = glm::length(a);
	a /= d;
	a *= -100.f/d;
	return a;
}

//--------------------------------------------------------------
void ofApp::setup(){
	std::vector<ofFloatColor> cols;
	float rd = 500.f; // default 10
	for (int x = 0; x < resx; x++) {
		for (int y = 0; y < resy; y++) {
			float theta = TWO_PI*(float)x / (float)(resx-1);
			float phi = TWO_PI*(float)y / (float)(resy-1);

			// for torus
			glm::vec3 pos(
						(cos(theta)*100+300)*cos(phi),
						(cos(theta)*100+300)*sin(phi),
						sin(theta)*100
					);

			// for random sphere
			//pos = glm::vec3(rd + 10, 0, 0);
			//while (glm::length2(pos) > rd * rd) {
			//	pos = glm::vec3(ofRandom(-rd, rd), ofRandom(-rd, rd), ofRandom(-rd, rd));
			//}

			//pos = glm::vec3(x * 3, y * 3, (y - resy / 2.f)*(y-resy/2.f)/40.f + (x - resx / 2.f)*(x-resx/2.f)/40.f);

			grid.addVertex(pos);
			grid_lines.addVertex(pos);

			ofFloatColor c(cos(theta*2)*0.4+0.4, sin(theta*2)*0.4 + 0.4, sin(phi*2)*0.4+0.4);
			if (x == 100) c = ofFloatColor(1, 1, 1);
			if (y == 100) c = ofFloatColor(1, 1, 1);
			grid.addColor(c);
			grid_lines.addColor(c); // may be overwritten later
		}
	}
	std::vector<ofIndexType> faces;
	for (int x = 0; x < resx; x++) {
		for (int y = 0; y < resy; y++) {
			// 2 triangles for faces
			grid.addIndex(get_index(x, y));
			grid.addIndex(get_index(x+1, y));
			grid.addIndex(get_index(x+1, y+1));
			
			grid.addIndex(get_index(x, y));
			grid.addIndex(get_index(x+1, y+1));
			grid.addIndex(get_index(x, y+1));

			// lines -- ofMesh can't do quads :(
			// make degenerate triangles bc ofMesh draw modes don't have quads???
			grid_lines.addIndex(get_index(x, y));
			grid_lines.addIndex(get_index(x + 1, y));
			grid_lines.addIndex(get_index(x, y));

			if (x % 5 == 0) {
				grid_lines.addIndex(get_index(x, y));
				grid_lines.addIndex(get_index(x, y + 1));
				grid_lines.addIndex(get_index(x, y));
			}

			vels.push_back(glm::vec3(0.0));
		}
	}
	// EDGE 1
	// EDGE 2
}

//--------------------------------------------------------------
void ofApp::update(){
	glm::vec3 mean;
	for (int i = 0; i < resx * resy; i++) {
		mean += grid.getVertex(i);
	}
	mean /= (float)(resx * resy);

	if (!doSim) return;

	float springlen = 25.f;
	for (int x = 0; x < resx; x++) {
		for (int y = 0; y < resy; y++) {

			float theta = TWO_PI * (float)x / (float)(resx - 1);
			float phi = TWO_PI * (float)y / (float)(resy - 1);

			// smooth (continuous if no flip)
			float smTheta = sin(theta)*0.5+0.5;
			float smPhi = sin(theta) * 0.5 + 0.5;

			glm::vec3 &v0 = grid.getVertex(get_index(x, y));
			glm::vec3 &v1 = grid.getVertex(get_index(x+1, y));
			glm::vec3 &v2 = grid.getVertex(get_index(x, y+1));
			glm::vec3 &v3 = grid.getVertex(get_index(x-1, y));
			glm::vec3 &v4 = grid.getVertex(get_index(x, y-1));

			
			// fabric sim
			vels[get_index(x, y)] += spring(v0, v1, springlen*smPhi) // the multiplication here makes the zoomy
								   + spring(v0, v2, springlen) 
				                   + spring(v0, v3, springlen) 
				                   + spring(v0, v4, springlen);
			
			// curvature-encouraging spring network
			vels[get_index(x, y)] += spring(v0, (v1 + v2 + v3 + v4) / 4.0, 2);

			// repulsion from center
			vels[get_index(x, y)] += 0.02*repulse(v0, glm::vec3(0));
			

			// minimal surface approx.
			//vels[get_index(x, y)] += ((v1 + v2 + v3 + v4) / 4.0 - v0)*0.7;
			
			// set color to mean curvature
			glm::vec3 r = (v1 + v2 + v3 + v4) / 4.0 - v0;
			float area = glm::length(glm::cross((v1 - v3), (v2 - v4)));
			float rr = 500.f*glm::length2(r)/area;

			grid.setColor(get_index(x, y), ofFloatColor(0.01*rr, 0.02*rr, 0.07*rr));
			
			// behold... cpu shading
			//glm::vec3 n = glm::normalize(glm::cross((v1 - v3), (v2 - v4)));
			//grid.setColor(get_index(x, y), fabs(glm::dot(n, glm::vec3(1, 0, 0))));
		}
	}
	// smooth out colors
	
	for(int k = 0; k < 2; k++)
	for (int x = 0; x < resx; x++) {
		for (int y = 0; y < resy; y++) {
			grid.setColor(get_index(x, y), (grid.getColor(get_index(x + 1, y)) / 4.0 +
				grid.getColor(get_index(x - 1, y)) / 4.0 +
				grid.getColor(get_index(x, y+1)) / 4.0 +
				grid.getColor(get_index(x, y-1)) / 4.0));
		}
	}
	// repel random vertices
	
	for(int k = 0; k < 10; k++)
	for (int i = 0; i < resx * resy; i++) {
		int i2 = floor(ofRandom(resx * resy));
		if (i != i2)
			vels[i] += 0.1*repulse(grid.getVertex(i), grid.getVertex(i2));
	}
	// integrate pos
	for (int i = 0; i < resx * resy; i++) {
		grid.setVertex(i, grid.getVertex(i) + vels[i] * 0.15);
		vels[i] *= 0.985;
	}

	//glm::vec3 mmax(-1, -1, -1);
	//glm::vec3 mmin(-1, -1, -1);

	// set mesh vertices
	for (int i = 0; i < resx * resy; i++) {
		grid.setVertex(i, grid.getVertex(i) - mean);
		grid_lines.setVertex(i, grid.getVertex(i) * 1.0);
		//mmax = glm::max(mmax, grid.getVertex(i));
		//mmin = glm::min(mmin, grid.getVertex(i));
	}
	/*
	// normalize scale (prevent collapse)
	glm::vec3 size = mmax - mmin;
	for (int i = 0; i < resx * resy; i++) {
		grid.setVertex(i, 400 * grid.getVertex(i) / fmax(size.x, fmax(size.y, size.z)));
		grid_lines.setVertex(i, grid.getVertex(i)*1.0);
	}
	*/

}
float z = 0;
//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);
	ofEnableDepthTest();
	cam.begin();

	ofPushMatrix();
	//ofRotateY(z); z++; // auto camera control
	grid.draw();
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	grid_lines.drawWireframe();
	ofDisableBlendMode();
	ofPopMatrix();

	cam.end();
	ofDisableDepthTest();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	doSim = !doSim;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
