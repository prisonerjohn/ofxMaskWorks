#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	//ofSetLogLevel(OF_LOG_VERBOSE);

	maskBuilder.setMaskSize(512, 424);

	loadSettings();
}

//--------------------------------------------------------------
void ofApp::exit()
{
	saveSettings();
}

//--------------------------------------------------------------
void ofApp::update()
{
	maskBuilder.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	const auto maskSize = maskBuilder.getMaskSize();
	const auto padding = glm::vec2((ofGetWidth() - maskSize.x * 2) / 3.0f, (ofGetHeight() - maskSize.y) / 2.0f);

	maskBuilder.setControlBounds(ofRectangle(padding.x, padding.y, maskSize.x, maskSize.y));
	maskBuilder.draw(padding.x, padding.y, maskSize.x, maskSize.y);

	maskBuilder.getMaskTexture().draw(padding.x * 2 + maskSize.x, padding.y);

	std::ostringstream oss;
	oss << "ofxMaskWorks" << std::endl
		<< "* SHIFT+M / toggle editing mode" << std::endl
		<< "* LEFT-CLICK / add corner point OR select point" << std::endl
		<< "* RIGHT-CLICK / add curve point OR convert point" << std::endl
		<< "* DRAG MOUSE  / move selected point" << std::endl
		<< "* UP,DOWN,LEFT,RIGHT / nudge selected point" << std::endl
		<< "* DELETE / delete selected point" << std::endl
		<< std::endl
		<< "ofApp" << std::endl
		<< "* SHIFT+L / load settings" << std::endl
		<< "* SHIFT+S / save settings";
	ofDrawBitmapStringHighlight(oss.str(), 10, 20);
}

//--------------------------------------------------------------
void ofApp::saveSettings() const
{
	nlohmann::json json;
	maskBuilder.serialize(json);
	ofSavePrettyJson("settings.json", json);
}

//--------------------------------------------------------------
void ofApp::loadSettings()
{
	const auto json = ofLoadJson("settings.json");
	maskBuilder.deserialize(json);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == 'S')
	{
		saveSettings();
	}
	else if (key == 'L')
	{
		loadSettings();
	}
}
