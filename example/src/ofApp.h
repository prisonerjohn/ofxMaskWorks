#pragma once

#include "ofMain.h"
#include "ofxMaskWorks.h"
#include "ofxMaskWorks/SdfGenerator.h"

class ofApp 
	: public ofBaseApp 
{
public:
	void setup() override;
	void exit() override;

	void update() override;
	void draw() override;

	void saveSettings() const;
	void loadSettings();

	void keyPressed(int key) override;

	ofxMaskWorks::Builder maskBuilder;
	ofxMaskWorks::SdfGenerator sdfGenerator;

	ofFloatPixels maskPixels;
	ofFloatPixels sdfPixels;
	ofTexture sdfTexture;
	bool doSdf;
};
