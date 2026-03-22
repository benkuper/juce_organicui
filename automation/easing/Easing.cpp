/*
  ==============================================================================

	Easing.cpp
	Created: 11 Dec 2016 1:29:02pm
	Author:  Ben

  ==============================================================================
*/

const String Easing::typeNames[Easing::TYPE_MAX]{ "Linear", "Bezier", "Hold","Sine", "Elastic","Bounce", "Steps", "Noise", "Perlin" };

Easing::Easing(Type type) :
	ControllableContainer("Easing"),
	type(type),
	length(0)
{
	showInspectorOnSelect = false;
}

Easing::~Easing()
{
	masterReference.clear();
}

void Easing::updateKeys(const Point<float>& _start, const Point<float>& _end, bool stretch)
{
	if ((start == _start && end == _end) || (_start.x > _end.x)) return;
	start = _start;
	end = _end;
	prevLength = length;
	length = end.x - start.x;

	updateKeysInternal();
}

EasingUI* Easing::createUI()
{
	return new EasingUI(this);
}

float Easing::getWeightForPos(float pos)
{
	return (pos - start.x) / (end.x - start.x);
}

Point<float> Easing::getClosestPointForPos(float pos)
{
	return Point<float>(pos, getValue(getWeightForPos(pos)));
}


LinearEasing::LinearEasing() : Easing(LINEAR) {}
HoldEasing::HoldEasing() : Easing(HOLD) {}


EasingUI* LinearEasing::createUI()
{
	return new LinearEasingUI(this);
}


EasingUI* HoldEasing::createUI()
{
	return new HoldEasingUI(this);
}

EasingUI* CubicEasing::createUI()
{
	return new CubicEasingUI(this);
}


float LinearEasing::getValue(const float& weight)
{
	return jmap(weight, start.y, end.y);
}

juce::Rectangle<float> LinearEasing::getBounds(bool includeHandles)
{
	return 	juce::Rectangle<float>(Point<float>(jmin(start.x, end.x), jmin(start.y, end.y)), Point<float>(jmax(start.x, end.x), jmax(start.y, end.y)));
}


float HoldEasing::getValue(const float& weight)
{
	if (weight < 1) return  start.y;
	return end.y;
}

juce::Rectangle<float> HoldEasing::getBounds(bool includeHandles)
{
	return 	juce::Rectangle<float>(Point<float>(jmin(start.x, end.x), jmin(start.y, end.y)), Point<float>(jmax(start.x, end.x), jmax(start.y, end.y)));
}


CubicEasing::CubicEasing() :
	Easing(BEZIER)
{
	anchor1 = addPoint2DParameter("Anchor 1", "Anchor 1 of the quadratic curve");
	anchor2 = addPoint2DParameter("Anchor 2", "Anchor 2 of the quadratic curve");
}


void CubicEasing::onContainerParameterChanged(Parameter* p)
{
	if (isClearing || isBeingDestroyed) return;
	if (p == anchor1 || p == anchor2) updateBezier();
}

Array<Point<float>> CubicEasing::getSplitControlPoints(float pos)
{
	float t = getBezierWeight(pos);

	Point<float> p1 = start;
	Point<float> p2 = start + anchor1->getPoint();
	Point<float> p3 = end + anchor2->getPoint();
	Point<float> p4 = end;

	Point<float> pp12 = p1 + (p2 - p1) * t;
	Point<float> pp23 = p2 + (p3 - p2) * t;
	Point<float> pp34 = p3 + (p4 - p3) * t;

	Point<float> pp123 = pp12 + (pp23 - pp12) * t;
	Point<float> pp234 = pp23 + (pp34 - pp23) * t;
	//Point<float> pp1234 = pp123 + (pp234 - pp123) * t;

	Array<Point<float>> result;
	result.add(pp12, pp123, pp234, pp34);
	return result;
}

juce::Rectangle<float> CubicEasing::getBounds(bool includeHandles)
{
	Bezier::AxisAlignedBoundingBox  bbox = bezier.aabb();

	Array<Point<float>> points;
	if (bbox.width() > 0 && bbox.height() > 0) points.add(Point<float>(bbox.minX(), bbox.minY()), Point<float>(bbox.maxX(), bbox.maxY()));
	if (includeHandles) points.add(anchor1->getPoint() + start, anchor2->getPoint() + end);

	return juce::Rectangle<float>::findAreaContainingPoints(points.getRawDataPointer(), points.size());
}


float CubicEasing::getValue(const float& weight)
{
	if (length == 0 || weight <= 0 || uniformLUT.size() == 0) return start.y;
	if (weight >= 1) return end.y;

	float indexF = weight * (uniformLUT.size() - 1);
	int index = (int)floor(indexF);
	float rel = indexF - index;
	float p1 = uniformLUT[index];
	float p2 = uniformLUT[index + 1];

	float p = p1 + (p2 - p1) * rel;
	return p;
}

Point<float> CubicEasing::getRawValue(const float& weight)
{
	Bezier::Point p = bezier.valueAt(weight);
	return { p.x, p.y };
}

float CubicEasing::getBezierWeight(const float& pos)
{
	if (length == 0) return 0;

	const int precision = length * 30;
	float closestT = getWeightForPos(pos);
	float minDist = INT32_MAX;
	for (int i = 0; i < precision; ++i)
	{
		float t = i * 1.0f / precision;
		Bezier::Point bp = bezier.valueAt(t);
		float dist = fabsf(bp.x - pos);
		if (dist < minDist)
		{
			closestT = t;
			minDist = dist;
		}
	}

	return closestT;
}

void CubicEasing::updateKeysInternal(bool stretch)
{
	if (length == 0) return;
	anchor1->setBounds(0, INT32_MIN, length, INT32_MAX);
	anchor2->setBounds(-length, INT32_MIN, 0, INT32_MAX);

	if (!anchor1->isOverriden && !anchor2->isOverriden)
	{
		anchor1->setPoint(length * .3f, 0);
		anchor2->setPoint(-length * .3f, 0);
	}

	if (stretch && prevLength != 0)
	{
		float stretchFactor = length / prevLength;
		anchor1->setPoint(anchor1->x * stretchFactor, anchor1->y * stretchFactor);
		anchor2->setPoint(anchor2->x * stretchFactor, anchor2->y * stretchFactor);
	}

	updateBezier();
}

void CubicEasing::updateBezier()
{
	if (length == 0) return;
	if (isClearing || isBeingDestroyed) return;

	Point<float> a1 = start + anchor1->getPoint();
	Point<float> a2 = end + anchor2->getPoint();
	bezier = Bezier::Bezier<3>({ {start.x, start.y},{a1.x, a1.y},{a2.x,a2.y},{end.x,end.y} });

	updateUniformLUT(1 + length * 20);
}

void CubicEasing::updateUniformLUT(int precision)
{
    if (isClearing || isBeingDestroyed || precision <= 0) return;

    uniformLUT.clear();
    Array<float, CriticalSection> arcLengths;
    arcLengths.add(0);

    Point<float> prevP = getRawValue(0);

    float dist = 0;
    for (int i = 1; i <= precision; ++i) {
        float rel = i * 1.0f / precision;
        Point<float> p = getRawValue(rel);

        dist += p.x - prevP.x;
        arcLengths.add(dist);
        prevP.setXY(p.x, p.y);
    }

    for (int i = 0; i <= precision; ++i)
    {
        float targetLength = (i * 1.0f / precision) * length;
        int low = 0;
        int high = precision;

        // Ensure binary search stays within bounds
        while (low < high)
        {
            int mid = low + (high - low) / 2;

            if (arcLengths[mid] < targetLength)
                low = mid + 1;
            else
                high = mid;
        }

        int index = jmax(0, jmin(low, precision - 1)); // Clamp index to valid range

        float lengthBefore = arcLengths[index];
        float lengthAfter = arcLengths[jmin(index + 1, precision)];

        float pos = 0;
        if (lengthBefore == targetLength)
            pos = index / static_cast<float>(precision);
        else if (lengthAfter > lengthBefore)
            pos = (index + (targetLength - lengthBefore) / (lengthAfter - lengthBefore)) / static_cast<float>(precision);

        Point<float> curPoint = getRawValue(pos);
        uniformLUT.add(curPoint.y);
    }
}


SineEasing::SineEasing() :
	Easing(SINE)
{
	freqAmp = addPoint2DParameter("Frequency Amplitude", "Frequency and amplitude of the sine wave");
	freqAmp->setPoint(1.f, .25f);
}

void SineEasing::updateKeysInternal(bool stretch)
{
	if (length == 0) return;

	if (stretch && prevLength != 0)
	{
		float stretchFactor = length / prevLength;
		freqAmp->setPoint(freqAmp->x * stretchFactor, freqAmp->y * stretchFactor);
	}
}

float SineEasing::getValue(const float& weight)
{
	return  start.y + (end.y - start.y) * weight + sinf(weight * length * MathConstants<float>::pi * 2 / freqAmp->x) * freqAmp->y;
}

juce::Rectangle<float> SineEasing::getBounds(bool includeHandles)
{
	Array<Point<float>> points;
	points.add(Point<float>(start.x, start.y - std::abs(freqAmp->y)), Point<float>(end.x, end.y + std::abs(freqAmp->y)), Point<float>(start.x, start.y + std::abs(freqAmp->y)), Point<float>(end.x, end.y - std::abs(freqAmp->y)));
	return juce::Rectangle<float>::findAreaContainingPoints(points.getRawDataPointer(), points.size());
}


EasingUI* SineEasing::createUI()
{
	return new SineEasingUI(this);
}



ElasticEasing::ElasticEasing() :
	Easing(ELASTIC)
{
	param = addPoint2DParameter("Frequency Amplitude", "Frequency and amplitude of the sine wave");
}


void ElasticEasing::updateKeysInternal(bool stretch)
{
	if (length == 0) return;
	if (!param->isOverriden) param->setPoint(length * .75f, 0);
	param->setBounds(0, 0, length, 0);

	if (prevLength == 0) return;
	float stretchFactor = length / prevLength;
	param->setPoint(param->x * stretchFactor, param->y * stretchFactor);
}

float ElasticEasing::getValue(const float& weight)
{
	const float c4 = jmap<float>(param->x / length, 10, 0);
	float p = pow(2, -10 * weight) * sin((weight * 10 - 0.75) * c4) + 1;
	return jmap<float>(p, start.y, end.y);
}

juce::Rectangle<float> ElasticEasing::getBounds(bool includeHandles)
{
	return juce::Rectangle<float>(Point<float>(start.x, jmin(start.y, end.y + (end.y - start.y))), Point<float>(end.x, jmax(start.y, end.y + (end.y - start.y))));
}


EasingUI* ElasticEasing::createUI()
{
	return new ElasticEasingUI(this);
}


BounceEasing::BounceEasing() :
	Easing(BOUNCE)
{
}

float BounceEasing::getValue(const float& weight)
{
	const float d1 = 7.5625f;
	const float n1 = 2.75f;

	float p = 0;
	float x = weight;

	if (x < 1 / n1) {
		p = d1 * x * x;
	}
	else if (x < 2 / n1) {
		p = d1 * (x - 1.5 / n1) * x + 0.75;
	}
	else if (x < 2.5 / n1) {
		p = d1 * (x - 2.25 / n1) * x + 0.9375;
	}
	else {
		p = d1 * (x - 2.625 / n1) * x + 0.984375;
	}

	return jmap<float>(p, start.y, end.y);
}

juce::Rectangle<float> BounceEasing::getBounds(bool includeHandles)
{
	return juce::Rectangle<float>(Point<float>(start.x, jmin(start.y, end.y)), Point<float>(end.x, jmax(start.y, end.y)));

}

void BounceEasing::updateKeysInternal(bool stretch)
{
	if (length == 0) return;

}

EasingUI* BounceEasing::createUI()
{
	return new BounceEasingUI(this);
}


StepEasing::StepEasing() :
	Easing(STEPS)
{
	param = addPoint2DParameter("Step Size", "Size of each step");
}


void StepEasing::updateKeysInternal(bool stretch)
{
	if (length == 0) return;
	if (!param->isOverriden) param->setPoint(length * .75f, 0);
	param->setBounds(0.01f, 0, length, 0);

	if (prevLength == 0) return;
	float stretchFactor = length / prevLength;
	param->setPoint(param->x * stretchFactor, param->y * stretchFactor);
}

float StepEasing::getValue(const float& weight)
{
	if(param->x == 0) return start.y;
	int numSteps = ceil(length / param->x);
	if(numSteps == 0 || start.y == end.y) return start.y;

	int curStep = floor(weight * numSteps);
	return jmap<float>(curStep, 0, numSteps, start.y, end.y);
}

juce::Rectangle<float> StepEasing::getBounds(bool includeHandles)
{
	return juce::Rectangle<float>(Point<float>(start.x, jmin(start.y, end.y + (end.y - start.y))), Point<float>(end.x, jmax(start.y, end.y + (end.y - start.y))));
}


EasingUI* StepEasing::createUI()
{
	return new GenericEasingUI(this, param);
}

NoiseEasing::NoiseEasing() :
	Easing(NOISE)
{
	taper1 = addPoint2DParameter("Taper 1", "Influence of the noise close to the start");
	taper2 = addPoint2DParameter("Taper 2", "Influence of the noise close to the end");
}


void NoiseEasing::updateKeysInternal(bool stretch)
{
	if (length == 0) return;
	if (!taper1->isOverriden)
	{
		taper1->setPoint(length * .25f, .2f);
		taper2->setPoint(-.25f * length, 0);
	}

	taper1->setBounds(0, INT32_MIN, length, INT32_MAX);
	taper2->setBounds(-length, 0, 0, 0);

	if (stretch && prevLength > 0)
	{
		float stretchFactor = length / prevLength;
		taper1->setPoint(taper1->x * stretchFactor, taper1->y * stretchFactor);
		taper2->setPoint(taper2->x * stretchFactor, taper2->y * stretchFactor);
	}
}

float NoiseEasing::getValue(const float& weight)
{
	float n = r.nextFloat();

	float t1 = taper1->x / length;
	float t2Diff = -taper2->x / length;
	float t2 = 1 - t2Diff;

	float amplitude = taper1->y;

	if (weight < t1)
	{
		float w = (weight / t1);
		float sineW = -(cosf(MathConstants<float>::pi * w) - 1) / 2;
		amplitude *= sineW;
	}

	if (weight > t2)
	{
		float w = (weight - t2) / t2Diff;
		float sineW = -(cosf(MathConstants<float>::pi * w) - 1) / 2;
		amplitude *= 1 - sineW;
	}


	float baseVal = start.y + (end.y - start.y) * weight;
	return baseVal + (n - .5f) * 2 * amplitude;
}

juce::Rectangle<float> NoiseEasing::getBounds(bool includeHandles)
{
	return juce::Rectangle<float>(Point<float>(start.x, jmin(start.y, end.y + (end.y - start.y))), Point<float>(end.x, jmax(start.y, end.y + (end.y - start.y))));
}


EasingUI* NoiseEasing::createUI()
{
	return new GenericEasingUI(this, taper1, taper2);
}



PerlinEasing::PerlinEasing() :
	Easing(PERLIN)
{
	taper1 = addPoint2DParameter("Taper 1", "Influence of the noise close to the start. Vertical is the seed");
	taper2 = addPoint2DParameter("Taper 2", "Influence of the noise close to the end. Vertical is octaves");
	offset = addFloatParameter("Offset", "Offset of the perlin", 0);
}


void PerlinEasing::updateKeysInternal(bool stretch)
{
	if (length == 0) return;

	if (!taper1->isOverriden)
	{
		taper1->setPoint(length * .25f, .2f);
		taper2->setPoint(-.25f * length, 0);
	}

	taper1->setBounds(0, INT32_MIN, length, INT32_MAX);
	taper2->setBounds(-length, INT32_MIN, 0, INT32_MAX);


	if (stretch && prevLength > 0)
	{
		float stretchFactor = length / prevLength;
		taper1->setPoint(taper1->x * stretchFactor, taper1->y * stretchFactor);
		taper2->setPoint(taper2->x * stretchFactor, taper2->y * stretchFactor);
	}
}

float PerlinEasing::getValue(const float& weight)
{
	float n = perlin.octaveNoise0_1(weight * length, offset->floatValue(), 1 + (int32_t)(abs(taper2->y) * 10));

	float t1 = taper1->x / length;
	float t2Diff = -taper2->x / length;
	float t2 = 1 - t2Diff;

	float amplitude = taper1->y;

	if (weight < t1)
	{
		float w = (weight / t1);
		float sineW = -(cosf(MathConstants<float>::pi * w) - 1) / 2;
		amplitude *= sineW;
	}

	if (weight > t2)
	{
		float w = (weight - t2) / t2Diff;
		float sineW = -(cosf(MathConstants<float>::pi * w) - 1) / 2;
		amplitude *= 1-sineW;
	}
	

	float baseVal = start.y + (end.y - start.y) * weight;
	return baseVal + (n - .5f) * 2 * amplitude;
}

juce::Rectangle<float> PerlinEasing::getBounds(bool includeHandles)
{
	return juce::Rectangle<float>(Point<float>(start.x, jmin(start.y, end.y + taper2->y)), Point<float>(end.x, jmax(start.y, end.y - taper2->y)));
}


EasingUI* PerlinEasing::createUI()
{
	return new GenericEasingUI(this, taper1, taper2, {offset} );
}
