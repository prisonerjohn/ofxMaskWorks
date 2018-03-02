#include "ofMain.h"
#include "ofApp.h"

//--------------------------------------------------------------
int main()
{
	ofGLFWWindowSettings settings;
	settings.setGLVersion(4, 1);
	settings.setSize(1280, 800);
	ofCreateWindow(settings);

	ofRunApp(new ofApp());
}
