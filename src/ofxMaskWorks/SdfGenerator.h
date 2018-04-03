#pragma once

#include "ofParameter.h"
#include "ofPixels.h"
#include "ofVectorMath.h"

namespace ofxMaskWorks
{
	class SdfGenerator
	{
	public:
		enum class FillMode
		{
			White,
			Black,
			Distance,
			Source
		};

	public:
		SdfGenerator();
		~SdfGenerator();

		void generate(const ofFloatPixels & srcPixels, ofFloatPixels & dstPixels);

	public:
		ofParameter<float> maxInside{ "Max Inside", 50.0f, 0.0f, 100.0f };
		ofParameter<float> maxOutside{ "Max Outside", 0.0f, 0.0f, 100.0f };
		ofParameter<float> postProcessDistance{ "Post-Process Distance", 0.0f, 0.0f, 100.0f };
		ofParameter<int> fillMode{ "Fill Mode", static_cast<int>(FillMode::White) };

		ofParameterGroup parameters{ "SDF Generator",
			maxInside, maxOutside,
			postProcessDistance,
			fillMode
		};

	private:
		struct Pixel
		{
			float alpha;
			float distance;
			glm::vec2 gradient;
			int dX;
			int dY;
		};

	private:
		void computeEdgeGradients();
		float approximateEdgeDelta(float gx, float gy, float a);
		void updateDistance(Pixel & p, int x, int y, int oX, int oY);
		void generateDistanceTransform();
		void postProcess();

	private:
		int width;
		int height;
		Pixel * pixels;
	};
}