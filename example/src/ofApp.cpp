#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	//ofSetLogLevel(OF_LOG_VERBOSE);

	maskBuilder.setCanvasSize(512, 424);
}

//--------------------------------------------------------------
void ofApp::update()
{
	maskBuilder.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	const auto maskSize = maskBuilder.getCanvasSize();
	const auto maskPos = glm::vec2(ofGetWidth() - maskSize.x, ofGetHeight() - maskSize.y) * 0.5f;
	maskBuilder.draw(maskPos.x, maskPos.y, maskSize.x, maskSize.y);

	std::ostringstream oss;
	oss << "ofxMaskWorks" << std::endl
		<< "* SHIFT+M / toggle editing mode" << std::endl
		<< "* LEFT-CLICK / add corner point OR select point" << std::endl
		<< "* RIGHT-CLICK / add curve point OR convert point" << std::endl
		<< "* DRAG MOUSE  / move selected point" << std::endl
		<< "* UP,DOWN,LEFT,RIGHT / nudge selected point" << std::endl
		<< "* DELETE / delete selected point";
	ofDrawBitmapStringHighlight(oss.str(), 10, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
