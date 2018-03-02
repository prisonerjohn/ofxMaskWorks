#include "Builder.h"

#include "ofGraphics.h"
#include "ofPath.h"

namespace ofxMaskWorks
{
	const float Builder::kNearThreshold = 10.0f;
	const float Builder::kMoveThreshold = 10.0f;
	const float Builder::kRenderPointSize = 5.0f;
	const float Builder::kNudgePointAmount = 0.5f;

	Builder::Builder()
		: editing(false)
		, focusIdx(-1)
		, maskDirty(false)
	{
		ofAddListener(ofEvents().keyPressed, this, &Builder::onKeyPressed);
	}

	Builder::~Builder()
	{
		ofRemoveListener(ofEvents().keyPressed, this, &Builder::onKeyPressed);
	}

	void Builder::update()
	{
		if (!this->canvasFbo.isAllocated()) return;

		if (this->maskDirty && this->rebuildMask())
		{
			this->maskDirty = false;
		}

		this->canvasFbo.begin();
		{
			ofBackground(ofColor::white);

			// Draw the mask.
			if (this->maskFbo.isAllocated())
			{
				this->maskFbo.draw(0, 0);
			}

			if (this->editing)
			{
				ofPushStyle();
				{
					// Draw points.
					for (int i = 0; i < this->points.size(); ++i)
					{
						if (this->focusIdx == i)
						{
							ofSetColor(ofColor::crimson);
						}
						else
						{
							ofSetColor(ofColor::seaGreen);
						}
						if (this->points[i].curve)
						{
							ofDrawCircle(this->points[i].pos, kRenderPointSize);
						}
						else
						{
							ofDrawRectangle(this->points[i].pos - glm::vec2(kRenderPointSize), kRenderPointSize * 2.0f, kRenderPointSize * 2.0f);
						}
					}

					// Draw border.
					ofSetColor(ofColor::crimson);
					ofNoFill();
					ofDrawRectangle(1, 1, this->canvasFbo.getWidth() - 1, this->canvasFbo.getHeight() - 1);
				}
				ofPopStyle();
			}
		}
		this->canvasFbo.end();
	}

	bool Builder::rebuildMask()
	{
		if (!this->maskFbo.isAllocated())
		{
			ofLogError(__FUNCTION__) << "Mask FBO not allocated!";
			return false;
		}

		ofLogVerbose(__FUNCTION__) << "Rebuild with " << this->points.size() << " points";

		this->maskFbo.begin();
		{
			if (this->points.size() == 0)
			{
				// No points, just use the entire screen.
				ofBackground(ofColor::white);
			}
			else
			{
				ofBackground(ofColor::black);
				ofSetColor(ofColor::white);

				ofPath path;
				size_t numCurvePts = 0;
				for (int i = 0; i < this->points.size(); ++i)
				{
					const auto & currPt = this->points[i];
					if (i == 0)
					{
						path.moveTo(currPt.pos);
						ofLogVerbose(__FUNCTION__) << i << ": Path MOVE to " << currPt.pos;

						if (currPt.curve)
						{
							// Add a couple of curve points to get it going.
							path.curveTo(currPt.pos);
							ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/CURR (" << numCurvePts << ") to " << currPt.pos;
							++numCurvePts;

							path.curveTo(currPt.pos);
							ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/CURR (" << numCurvePts << ") to " << currPt.pos;
							++numCurvePts;
						}
					}
					else
					{
						if (currPt.curve)
						{
							if (numCurvePts == 0)
							{
								// First point in curve, add curve points coming from the previous point.
								const auto prevPt = this->points[i - 1];
								path.curveTo(prevPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/BEGIN (" << numCurvePts << ") to " << prevPt.pos;
								++numCurvePts;

								path.curveTo(prevPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/BEGIN (" << numCurvePts << ") to " << prevPt.pos;
								++numCurvePts;
							}

							path.curveTo(currPt.pos);
							ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/CURR (" << numCurvePts << ") to " << currPt.pos;
							++numCurvePts;
						}
						else
						{
							if (numCurvePts > 0)
							{
								// Previous was last point in curve, add curve points at the current point.
								path.curveTo(currPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/END (" << numCurvePts << ") to " << currPt.pos;
								++numCurvePts;

								path.curveTo(currPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/END (" << numCurvePts << ") to " << currPt.pos;
								++numCurvePts;
							}

							path.lineTo(currPt.pos);
							ofLogVerbose(__FUNCTION__) << i << ": Path LINE to " << currPt.pos;
							numCurvePts = 0;
						}

						if (i == this->points.size() - 1)
						{
							// Close the path by connecting the last point to the first.
							const auto firstPt = this->points[0];
							if (firstPt.curve)
							{
								if (numCurvePts == 0)
								{
									// First point in curve, add curve points coming from the current point.
									path.curveTo(currPt.pos);
									ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/BEGIN (" << numCurvePts << ") to " << currPt.pos;
									++numCurvePts;

									path.curveTo(currPt.pos);
									ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/BEGIN (" << numCurvePts << ") to " << currPt.pos;
									++numCurvePts;
								}
								
								// Add a couple of curve points to close it up.
								path.curveTo(firstPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/END (" << numCurvePts << ") to " << firstPt.pos;
								++numCurvePts;
						
								path.curveTo(firstPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/END (" << numCurvePts << ") to " << firstPt.pos;
								++numCurvePts;
							}
							else if (numCurvePts > 0)
							{
								// Previous was last point in curve, add curve points at the current point.
								path.curveTo(firstPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/END (" << numCurvePts << ") to " << currPt.pos;
								++numCurvePts;

								path.curveTo(firstPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path CURVE/END (" << numCurvePts << ") to " << currPt.pos;
								++numCurvePts;
							}
							else
							{
								path.lineTo(firstPt.pos);
								ofLogVerbose(__FUNCTION__) << i << ": Path LINE to " << firstPt.pos;
								numCurvePts = 0;
							}
						}
					}
				}

				if (this->editing)
				{
					for (auto & polyline : path.getOutline())
					{
						polyline.draw();
					}
				}
				else
				{
					path.draw();
				}
			}
		}
		this->maskFbo.end();

		return true;
	}

	void Builder::draw(int x, int y) const
	{
		this->draw(x, y, this->canvasFbo.getWidth(), this->canvasFbo.getHeight());
	}

	void Builder::draw(int x, int y, int width, int height) const
	{
		if (!this->canvasFbo.isAllocated()) return;

		auto & mutableBounds = const_cast<ofRectangle &>(this->drawBounds);
		mutableBounds.set(x, y, width, height);
		this->canvasFbo.draw(this->drawBounds);
	}

	const ofTexture & Builder::getMaskTexture() const
	{
		return this->maskFbo.getTexture();
	}

	void Builder::setCanvasSize(int width, int height)
	{
		this->canvasFbo.allocate(width, height);
		this->maskFbo.allocate(width, height);
		this->maskDirty = true;
	}

	const glm::ivec2 & Builder::getCanvasSize() const
	{
		return glm::ivec2(this->canvasFbo.getWidth(), this->canvasFbo.getHeight());
	}

	void Builder::setEditing(bool editing)
	{
		if (this->editing == editing)
		{
			return;
		}
		this->editing = editing;

		if (this->editing)
		{
			ofAddListener(ofEvents().mousePressed, this, &Builder::onMousePressed);
			ofAddListener(ofEvents().mouseDragged, this, &Builder::onMouseDragged);
			ofAddListener(ofEvents().mouseReleased, this, &Builder::onMouseReleased);
		}
		else
		{
			ofRemoveListener(ofEvents().mousePressed, this, &Builder::onMousePressed);
			ofRemoveListener(ofEvents().mouseDragged, this, &Builder::onMouseDragged);
			ofRemoveListener(ofEvents().mouseReleased, this, &Builder::onMouseReleased);
		}

		this->maskDirty = true;
	}

	bool Builder::isEditing() const
	{
		return this->editing;
	}

	bool Builder::onKeyPressed(ofKeyEventArgs & args)
	{
		if (args.key == 'M')
		{
			this->setEditing(!this->editing);
			ofLogNotice(__FUNCTION__) << "Set editing mode " << (this->editing ? "ON" : "OFF");
			return true;
		}
		else if (this->focusIdx < this->points.size())
		{
			if (args.key == OF_KEY_DEL)
			{
				this->points.erase(this->points.begin() + this->focusIdx);
				ofLogVerbose(__FUNCTION__) << "Delete point " << this->focusIdx;
				this->focusIdx = -1;

				this->maskDirty = true;
				return true;
			}
			else if (args.key == OF_KEY_UP)
			{
				this->points[this->focusIdx].pos.y -= kNudgePointAmount;
				ofLogVerbose(__FUNCTION__) << "Nudge point " << this->focusIdx << " UP to " << this->points[this->focusIdx].pos;

				this->maskDirty = true;
				return true;
			}
			else if (args.key == OF_KEY_DOWN)
			{
				this->points[this->focusIdx].pos.y += kNudgePointAmount;
				ofLogVerbose(__FUNCTION__) << "Nudge point " << this->focusIdx << " DOWN to " << this->points[this->focusIdx].pos;

				this->maskDirty = true;
				return true;
			}
			else if (args.key == OF_KEY_LEFT)
			{
				this->points[this->focusIdx].pos.x -= kNudgePointAmount;
				ofLogVerbose(__FUNCTION__) << "Nudge point " << this->focusIdx << " LEFT to " << this->points[this->focusIdx].pos;

				this->maskDirty = true;
				return true;
			}
			else if (args.key == OF_KEY_RIGHT)
			{
				this->points[this->focusIdx].pos.x += kNudgePointAmount;
				ofLogVerbose(__FUNCTION__) << "Nudge point " << this->focusIdx << " RIGHT to " << this->points[this->focusIdx].pos;

				this->maskDirty = true;
				return true;
			}
		}

		return false;
	}

	bool Builder::onMousePressed(ofMouseEventArgs & args)
	{
		if (!this->drawBounds.inside(args.x, args.y))
		{
			return false;
		}

		// Try to find a near point to focus.
		const auto currCursor = glm::vec2(args.x - this->drawBounds.x, args.y - this->drawBounds.y);
		size_t nearestIdx = -1;
		float nearestDist = std::numeric_limits<float>::max();
		for (int i = 0; i < this->points.size(); ++i)
		{
			const auto currDist = glm::distance(currCursor, points[i].pos);
			if (currDist < kNearThreshold && currDist < nearestDist)
			{
				nearestIdx = i;
				nearestDist = currDist;
			}
		}

		if (nearestIdx < this->points.size())
		{
			// Set focus point idx.
			this->focusIdx = nearestIdx;
			this->addedIdx = -1;
			ofLogVerbose(__FUNCTION__) << "Focus on point " << this->focusIdx << " at " << this->points[this->focusIdx].pos;
		}
		else // No focus point found.
		{
			if (this->focusIdx < this->points.size())
			{
				// Unfocus previously focused point.
				ofLogVerbose(__FUNCTION__) << "Unfocus from point " << this->focusIdx << " at " << this->points[this->focusIdx].pos;
				this->addedIdx = -1;
				this->focusIdx = -1;
			}
			else
			{
				// Add a new point.
				this->points.push_back(Point{ currCursor, false });
				this->addedIdx = this->points.size() - 1;
				this->focusIdx = this->addedIdx;
				ofLogVerbose(__FUNCTION__) << "Add new point " << this->focusIdx << " at " << this->points[this->focusIdx].pos;

				this->maskDirty = true;
			}
		}

		// Toggle the point type if we are using the right mouse button or the SHIFT key.
		if (this->focusIdx < this->points.size() && (args.button == 2 || ofGetKeyPressed(OF_KEY_SHIFT)))
		{
			this->points[this->focusIdx].curve ^= 1;
			ofLogVerbose(__FUNCTION__) << "Toggle point " << this->focusIdx << " to " << (this->points[this->focusIdx].curve ? "curve" : "corner");

			this->maskDirty = true;
		}

		this->pressCursor = currCursor;
		this->dragCursor = currCursor;

		return true;
	}

	bool Builder::onMouseDragged(ofMouseEventArgs & args)
	{
		if (this->focusIdx >= this->points.size())
		{
			return false;
		}

		const auto currCursor = glm::vec2(args.x - this->drawBounds.x, args.y - this->drawBounds.y);
		const auto deltaCursor = currCursor - this->dragCursor;
		this->points[this->focusIdx].pos += deltaCursor;
		ofLogVerbose(__FUNCTION__) << "Drag point " << this->focusIdx << " to " << this->points[this->focusIdx].pos;

		this->dragCursor = currCursor;

		this->maskDirty = true;
		return true;
	}

	bool Builder::onMouseReleased(ofMouseEventArgs & args)
	{
		if (this->addedIdx >= this->points.size() || this->addedIdx != this->focusIdx)
		{
			return false;
		}

		const auto currCursor = glm::vec2(args.x - this->drawBounds.x, args.y - this->drawBounds.y);
		const auto deltaCursor = currCursor - this->pressCursor;
		if (glm::length(deltaCursor) < kMoveThreshold)
		{
			// Unfocus previously focused point.
			ofLogVerbose(__FUNCTION__) << "Unfocus from point " << this->focusIdx << " at " << this->points[this->focusIdx].pos;
			this->addedIdx = -1;
			this->focusIdx = -1;
		}

		this->dragCursor = currCursor;

		return true;
	}
}
