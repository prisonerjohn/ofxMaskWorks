#include "SdfGenerator.h"

#include "ofGraphicsConstants.h"

namespace ofxMaskWorks
{
	SdfGenerator::SdfGenerator()
		: pixels(nullptr)
	{}

	SdfGenerator::~SdfGenerator()
	{
		if (this->pixels != nullptr)
		{
			delete[] this->pixels;
			this->pixels = nullptr;
		}
	}
	
	void SdfGenerator::generate(const ofFloatPixels & srcPixels, ofFloatPixels & dstPixels)
	{
		if (srcPixels.getWidth() != dstPixels.getWidth() || 
			srcPixels.getHeight() != dstPixels.getHeight() ||
			srcPixels.getNumChannels() != dstPixels.getNumChannels())
		{
			ofLogError(__FUNCTION__) << "Source and destination must be the same size.";
			return;
		}

		if (srcPixels.getImageType() != OF_IMAGE_COLOR_ALPHA)
		{
			ofLogError(__FUNCTION__) << "Image type must be RGBA or BGRA.";
			return;
		}

		this->width = srcPixels.getWidth();
		this->height = srcPixels.getHeight();
		const int numPixels = this->width * this->height;
		this->pixels = new Pixel[numPixels];

		const auto srcData = srcPixels.getData();
		auto dstData = dstPixels.getData();
		
		const auto mode = static_cast<FillMode>(this->fillMode.get());
		ofFloatColor color = (mode == FillMode::White) ? ofFloatColor::white : ofFloatColor::black;

		if (this->maxInside > 0.0f)
		{
			for (int i = 0; i < numPixels; ++i)
			{
				this->pixels[i].alpha = 1.0f - srcData[i * 4 + 3];
			}

			this->computeEdgeGradients();
			this->generateDistanceTransform();
			if (this->postProcessDistance > 0.0f)
			{
				this->postProcess();
			}

			float scale = 1.0f / this->maxInside;
			for (int i = 0; i < numPixels; ++i)
			{
				color.a = glm::clamp(this->pixels[i].distance);
				dstData[i * 4 + 0] = color.r;
				dstData[i * 4 + 1] = color.g;
				dstData[i * 4 + 2] = color.b;
				dstData[i * 4 + 3] = color.a;
			}
		}

		if (this->maxOutside > 0.0f)
		{
			for (int i = 0; i < numPixels; ++i)
			{
				this->pixels[i].alpha = srcData[i * 4 + 3];
			}

			this->computeEdgeGradients();
			this->generateDistanceTransform();
			if (this->postProcessDistance > 0.0f)
			{
				this->postProcess();
			}

			const float scale = 1.0f / this->maxOutside;
			if (this->maxInside > 0.0f)
			{
				for (int i = 0; i < numPixels; ++i)
				{
					color.a = 0.5f + (dstData[i * 4 + 3] - glm::clamp(this->pixels[i].distance * scale)) * 0.5f;
					dstData[i * 4 + 0] = color.r;
					dstData[i * 4 + 1] = color.g;
					dstData[i * 4 + 2] = color.b;
					dstData[i * 4 + 3] = color.a;
				}
			}
			else
			{
				for (int i = 0; i < numPixels; ++i)
				{
					color.a = glm::clamp(1.0f - this->pixels[i].distance * scale);
					dstData[i * 4 + 0] = color.r;
					dstData[i * 4 + 1] = color.g;
					dstData[i * 4 + 2] = color.b;
					dstData[i * 4 + 3] = color.a;
				}
			}
		}

		if (mode == FillMode::Distance)
		{
			for (int i = 0; i < numPixels; ++i)
			{
				dstData[i * 4 + 0] = dstData[i * 4 + 3];
				dstData[i * 4 + 1] = dstData[i * 4 + 3];
				dstData[i * 4 + 2] = dstData[i * 4 + 3];
			}
		}
		else if (mode == FillMode::Source)
		{
			for (int i = 0; i < numPixels; ++i)
			{
				dstData[i * 4 + 0] = srcData[i * 4 + 0];
				dstData[i * 4 + 1] = srcData[i * 4 + 1];
				dstData[i * 4 + 2] = srcData[i * 4 + 2];
			}
		}

		delete[] this->pixels;
		this->pixels = nullptr;
	}

	void SdfGenerator::computeEdgeGradients()
	{
		static const float kSqrt2 = sqrt(2.0f);

		for (int y = 1; y < this->height - 1; ++y)
		{
			for (int x = 1; x < this->width - 1; ++x)
			{
				int idx = y * this->width + x;
				auto & p = pixels[idx];
				if (p.alpha > 0.0f && p.alpha < 1.0f)
				{
					// Estimate gradient of edge pixel using surrounding pixels.
					int nwdx = (y - 1) * this->width + (x - 1);
					int swdx = (y + 1) * this->width + (x - 1);
					int nedx = (y - 1) * this->width + (x + 1);
					int sedx = (y + 1) * this->width + (x + 1);
					float g = 
						- pixels[nwdx].alpha
						- pixels[swdx].alpha
						+ pixels[nedx].alpha
						+ pixels[sedx].alpha;

					int eedx = (y + 0) * this->width + (x + 1);
					int wwdx = (y + 0) * this->width + (x - 1);
					int ssdx = (y + 1) * this->width + (x + 0);
					int nndx = (y - 1) * this->width + (x + 0);
					p.gradient.x = g + (pixels[eedx].alpha - pixels[wwdx].alpha) * kSqrt2;
					p.gradient.y = g + (pixels[ssdx].alpha - pixels[nndx].alpha) * kSqrt2;
					p.gradient = glm::normalize(p.gradient);
				}
			}
		}
	}

	float SdfGenerator::approximateEdgeDelta(float gx, float gy, float a)
	{
		// (gx, gy) can be either the local pixel gradient or the direction to the pixel.

		if (gx == 0.0f || gy == 0.0f)
		{
			// Linear function is correct if both gx and gy are 0
			// and still fair if only one of them is 0.
			return 0.5f - a;
		}

		// Normalize (gx, gy)
		float length = sqrt(gx * gx + gy * gy);
		gx = gx / length;
		gy = gy / length;

		// Reduce symmetrical equation to first octant only.
		// gx >= 0, gy >= 0, gx >= gy
		gx = fabs(gx);
		gy = fabs(gy);
		if (gx < gy)
		{
			std::swap(gx, gy);
		}

		// Compute delta.
		float a1 = 0.5f * gy / gx;
		if (a < a1)
		{
			// 0 <= a < a1
			return 0.5f * (gx + gy) - sqrt(2.0f * gx * gy * a);
		}
		if (a < (1.0f - a1))
		{
			// a1 <= a <= 1 - a1
			return (0.5f - a) * gx;
		}
		// 1 - a1 < a <= 1
		return -0.5f * (gx + gy) + sqrt(2.0f * gx * gy * (1.0f - a));
	}

	void SdfGenerator::updateDistance(Pixel & p, int x, int y, int oX, int oY)
	{
		int ndx = (y + oY) * this->width + (x + oX);
		auto & neighbor = this->pixels[ndx];
		int cdx = (y + oY - neighbor.dY) * this->width + (x + oX - neighbor.dX);
		auto & closest = pixels[cdx];

		if (closest.alpha == 0.0f || &closest == &p)
		{
			// Neighbor has no closest yet
			// or neighbor's closest is p itself.
			return;
		}

		int dX = neighbor.dX - oX;
		int dY = neighbor.dY - oY;
		float distance = sqrt(dX * dX + dY * dY) + this->approximateEdgeDelta(dX, dY, closest.alpha);
		if (distance < p.distance)
		{
			p.distance = distance;
			p.dX = dX;
			p.dY = dY;
		}
	}

	void SdfGenerator::generateDistanceTransform()
	{
		// Perform anti-aliased Euclidean distance transform.
		
		// Initialize distances.
		for (int i = 0; i < this->width * this->height; ++i)
		{
			auto & p = this->pixels[i];
			p.dX = 0;
			p.dY = 0;
			if (p.alpha <= 0.0f)
			{
				// Outside.
				p.distance = std::numeric_limits<float>::max();
			}
			else if (p.alpha < 1.0f)
			{
				// On the edge.
				p.distance = this->approximateEdgeDelta(p.gradient.x, p.gradient.y, p.alpha);
			}
			else
			{
				// Inside.
				p.distance = 0.0f;
			}
		}

		// Perform 8SSED (eight-points signed sequential Euclidean distance transform).
		
		// Scan up.
		for (int y = 1; y < this->height; ++y)
		{
			// |P.
			// |XX
			{
				int idx = y * this->width + 0;
				auto & p = pixels[idx];
				if (p.distance > 0.0f)
				{
					this->updateDistance(p, 0, y, 0, -1);
					this->updateDistance(p, 0, y, 1, -1);
				}
			}
			// -->
			// XP.
			// XXX
			for (int x = 1; x < this->width - 1; ++x)
			{
				int idx = y * this->width + x;
				auto & p = pixels[idx];
				if (p.distance > 0.0f) 
				{
					this->updateDistance(p, x, y, -1,  0);
					this->updateDistance(p, x, y, -1, -1);
					this->updateDistance(p, x, y,  0, -1);
					this->updateDistance(p, x, y,  1, -1);
				}
			}
			// XP|
			// XX|
			{
				int idx = y * this->width + (this->width - 1);
				auto & p = pixels[idx];
				if (p.distance > 0.0f)
				{
					this->updateDistance(p, width - 1, y, -1,  0);
					this->updateDistance(p, width - 1, y, -1, -1);
					this->updateDistance(p, width - 1, y,  0, -1);
				}
			}
			// <--
			// .PX
			{
				for (int x = width - 2; x >= 0; --x)
				{
					int idx = y * this->width + x;
					auto & p = pixels[idx];
					if (p.distance > 0.0f)
					{
						this->updateDistance(p, x, y, 1, 0);
					}
				}
			}
		}

		// Scan down.
		for (int y = height - 2; y >= 0; --y)
		{
			// XX|
			// .P|
			{
				int idx = y * this->width + (this->width - 1);
				auto & p = pixels[idx];
				if (p.distance > 0.0f)
				{
					this->updateDistance(p, width - 1, y,  0, 1);
					this->updateDistance(p, width - 1, y, -1, 1);
				}
			}
			// <--
			// XXX
			// .PX
			for (int x = width - 2; x > 0; --x)
			{
				int idx = y * this->width + x;
				auto & p = pixels[idx];
				if (p.distance > 0.0f)
				{
					this->updateDistance(p, x, y,  1, 0);
					this->updateDistance(p, x, y,  1, 1);
					this->updateDistance(p, x, y,  0, 1);
					this->updateDistance(p, x, y, -1, 1);
				}
			}
			// |XX
			// |PX
			{
				int idx = y * this->width + 0;
				auto & p = pixels[idx];
				if (p.distance > 0.0f)
				{
					this->updateDistance(p, 0, y, 1, 0);
					this->updateDistance(p, 0, y, 1, 1);
					this->updateDistance(p, 0, y, 0, 1);
				}
			}
			// -->
			// XP.
			for (int x = 1; x < width; ++x)
			{
				int idx = y * this->width + x;
				auto & p = pixels[idx];
				if (p.distance > 0.0f)
				{
					this->updateDistance(p, x, y, -1, 0);
				}
			}
		}
	}

	void SdfGenerator::postProcess()
	{
		// Adjust distances near edges based on the local edge gradient.
		for (int y = 0; y < this->height; ++y)
		{
			for (int x = 0; x < this->width; ++x)
			{
				int idx = y * this->width + x;
				auto & p = this->pixels[idx];
				if ((p.dX == 0 && p.dY == 0) || p.distance >= this->postProcessDistance)
				{
					// Ignore edge, inside, and beyond max distance.
					continue;
				}

				float dX = p.dX;
				float dY = p.dY;
				int cdx = (y - p.dY) * width + (x - p.dX);
				const auto & closest = this->pixels[cdx];
				if (closest.gradient.x == 0.0f && closest.gradient.y == 0.0f)
				{
					// Ignore unknown gradients (inside).
					continue;
				}

				// Compute hit point offset on gradient inside pixel.
				float df = this->approximateEdgeDelta(closest.gradient.x, closest.gradient.y, closest.alpha);
				float t =  dY * closest.gradient.x - dX * closest.gradient.y;
				float u = -df * closest.gradient.x +  t * closest.gradient.y;
				float v = -df * closest.gradient.y -  t * closest.gradient.x;

				// Use hit point to compute distance.
				if (fabs(u) <= 0.5f && fabs(v) <= 0.5f)
				{
					p.distance = sqrt((dX + u) * (dX + u) + (dY + v) * (dY + v));
				}
			}
		}
	}
}