#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {

    ofGLWindowSettings settings;
    settings.setGLVersion(1, 2);  // Programmable pipeline
    settings.setSize(1024, 768);
    ofCreateWindow(settings);

    ofRunApp(new ofApp());
    ofLogToConsole();
}