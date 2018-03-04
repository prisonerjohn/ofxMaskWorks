#pragma once

#include "ofEvents.h"
#include "ofFbo.h"
#include "ofJson.h"
#include "ofPath.h"
#include "ofPolyline.h"
#include "ofRectangle.h"
#include "ofVectorMath.h"

namespace ofxMaskWorks
{
	struct Point
	{
		glm::vec2 pos;
		bool curve;
	};

	class Builder
	{
	public:
		Builder();
		~Builder();

		void update();
		bool rebuildMask();

		void draw(int x, int y) const;
		void draw(int x, int y, int width, int height) const;
		void draw(const ofRectangle & drawBounds) const;

		const ofTexture & getMaskTexture() const;

		const ofPath & getMaskPath() const;

		void setMaskSize(int width, int height);
		const glm::ivec2 & getMaskSize() const;

		void setControlBounds(int x, int y, int width, int height);
		void setControlBounds(const ofRectangle & controlBounds);
		const ofRectangle & getControlBounds() const;

		void setEditing(bool editing);
		bool isEditing() const;

		void serialize(nlohmann::json & json, const std::string & name = "ofxMaskWorks") const;
		void deserialize(const nlohmann::json & json, const std::string & name = "ofxMaskWorks");

	private:
		static const float kNearThreshold;
		static const float kMoveThreshold;
		static const float kRenderPointSize;
		static const float kNudgePointAmount;

	private:
		bool onKeyPressed(ofKeyEventArgs & args);

		bool onMousePressed(ofMouseEventArgs & args);
		bool onMouseDragged(ofMouseEventArgs & args);
		bool onMouseReleased(ofMouseEventArgs & args);

	private:
		ofFbo maskFbo;
		ofFbo canvasFbo;

		bool editing;
		ofRectangle controlBounds;

		std::vector<Point> points;
		size_t addedIdx;
		size_t focusIdx;

		glm::vec2 pressCursor;
		glm::vec2 dragCursor;

		ofPath maskPath;
		bool maskDirty;
	};
}
