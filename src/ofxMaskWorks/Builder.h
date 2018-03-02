#pragma once

#include "ofEvents.h"
#include "ofFbo.h"
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

		const ofTexture & getMaskTexture() const;

		void setCanvasSize(int width, int height);
		const glm::ivec2 & getCanvasSize() const;

		void setEditing(bool editing);
		bool isEditing() const;

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
		ofFbo canvasFbo;
		ofFbo maskFbo;

		bool editing;
		ofRectangle drawBounds;

		std::vector<Point> points;
		size_t addedIdx;
		size_t focusIdx;

		glm::vec2 pressCursor;
		glm::vec2 dragCursor;

		bool maskDirty;
	};
}
