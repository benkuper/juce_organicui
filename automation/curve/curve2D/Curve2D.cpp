/*
  ==============================================================================

	Curve2D.cpp
	Created: 21 Mar 2020 4:06:01pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

Curve2D::Curve2D(const String& name) :
	Manager(name),
	recorder(nullptr),
	showUIInEditor(true)
{
	isSelectable = false;

	editorCanBeCollapsed = false;
	showInspectorOnSelect = false;
	//userCanAddItemsManually = false;
	canInspectChildContainers = false;


	length = addFloatParameter("Length", "The length of the curve", 0, 0);
	length->hideInEditor = true;
	length->setControllableFeedbackOnly(true);

	position = addFloatParameter("Position", "The length of the curve", 0, 0, 1);
	position->hideInEditor = true;

	value = addPoint2DParameter("Value", "The current value of the curve at the current position");
	value->hideInEditor = true;
	value->setControllableFeedbackOnly(true);

	keySyncMode = addBoolParameter("Key Sync", "If checked, when moving 2d keys, the keys in the timeline will ajdust to keep the same timing relative to the 2D keys", false, false);
	keySyncMode->hideInEditor = true;

	focusRange = addFloatParameter("Focus Range", "This allow to focus on a specific part of the curve around the position.", 0, 0);
	focusRange->hideInEditor = true;
}

Curve2D::~Curve2D()
{

}

void Curve2D::setControlMode(ControlMode mode)
{
	controlMode = mode;
	position->setControlMode(mode == MANUAL ? Parameter::MANUAL : Parameter::AUTOMATION);
	if (position->controlMode == Parameter::AUTOMATION)
	{
		position->automation->setManualMode(true);
		position->automation->setLength(length->floatValue(), true);
		((Automation*)position->automation->automationContainer)->clear();
	}

	keySyncMode->setEnabled(position->controlMode == Parameter::AUTOMATION);
}

void Curve2D::addItemInternal(Curve2DKey* k, var)
{
	k->isFirst = getNumItems() == 1;
	if (!isManipulatingMultipleItems && !isCurrentlyLoadingData) updateCurve(false);
}

void Curve2D::addItemsInternal(Array<Curve2DKey*> keys, var)
{
	if (!isCurrentlyLoadingData) updateCurve();
}

void Curve2D::removeItemInternal(Curve2DKey*)
{
	if (!isManipulatingMultipleItems && !isCurrentlyLoadingData) updateCurve(false);
}

void Curve2D::removeItemsInternal(Array<Curve2DKey*>)
{
	if (!isCurrentlyLoadingData) updateCurve(false);
}

float Curve2D::addFromPointsAndSimplify(Array<Point<float>> sourcePoints, bool clearBeforeAdd, Array<float> pointTimes)
{
	if (clearBeforeAdd) clear();


	if (sourcePoints.size() == 0)
		return 0;

	float lengthBefore = length->floatValue();


	Array<float> points;
	for (auto& pp : sourcePoints) points.add(pp.x, pp.y);

	float* result;
	unsigned int resultNum = 0;
	unsigned int* corners = nullptr;
	unsigned int cornersLength = 0;
	unsigned int* cornerIndex = nullptr;
	unsigned int cornerIndexLength = 0;

	const float errorThreshold = 0.02f;

	curve_fit_corners_detect_fl(points.getRawDataPointer(), points.size() / 2, 2,
		errorThreshold / 4, errorThreshold * 4, 32, MathConstants<double>::pi / 8,
		&corners, &cornersLength);
	if (cornersLength == 0) corners = nullptr;

	DBG((int)cornersLength << " corners detected");

	curve_fit_cubic_to_points_fl(points.getRawDataPointer(), points.size() / 2, 2,
		errorThreshold, CURVE_FIT_CALC_HIGH_QUALIY,
		corners, cornersLength,
		&result, &resultNum,
		nullptr,
		&cornerIndex, &cornerIndexLength);

	int numPoints = ((int)resultNum);

	Array<Curve2DKey*> keys;

	CubicEasing2D* prevEasing = nullptr;
	Point<float> prevRP;

	float maxDist = 100;

	int numBadPoints = 0;
	for (int i = 0; i < numPoints; ++i)
	{
		int index = i * 6;
		Point<float> h1(result[index + 0], result[index + 1]);
		Point<float> rp(result[index + 2], result[index + 3]);
		Point<float> h2(result[index + 4], result[index + 5]);


		if (prevEasing != nullptr && h1.getDistanceFrom(rp) < maxDist)
		{
			prevEasing->anchor2->setPoint(h1 - rp);
		}


		if (i > 0 && rp.getDistanceFrom(prevRP) > maxDist)
		{
			numBadPoints++;
			continue;
		}

		Curve2DKey* k = new Curve2DKey();
		k->setPosition(rp);
		k->setNiceName("Key " + String(i));
		k->easingType->setValueWithData(Easing2D::BEZIER);
		CubicEasing2D* ce = (CubicEasing2D*)k->easing.get();
		if (h2.getDistanceFrom(rp) < maxDist) ce->anchor1->setPoint(h2 - rp);

		DBG("Add good point : " << k->getPosition().toString());
		keys.add(k);

		prevEasing = ce;
		prevRP.setXY(rp.x, rp.y);
	}
	DBG(numBadPoints << " bad points discarded");

	free(result);
	free(corners);
	free(cornerIndex);

	addItems(keys);

	float addedLength = length->floatValue() - lengthBefore;
	return addedLength;

	//TODO implement automation curve construction from times array (needs a getClosestPointForPosition in Easing2D)

}

void Curve2D::updateCurve(bool relativeAutomationKeySyncMode)
{
	if (isCurrentlyLoadingData || Engine::mainEngine->isClearing) return;

	Array<float> prevCurvePositions;
	float prevLength = length->floatValue();

	bounds = getNumItems() > 0 ? getFirstItem()->easing->getBounds() : Rectangle<float>(0, 0, 0, 0);

	float curLength = 0;
	int numItems = getNumItems();
	for (int i = 0; i < numItems; ++i)
	{

		if (i < numItems - 1) getItemAt(i)->setNextKey(getItemAt(i + i));
		else getItemAt(i)->setNextKey(nullptr);
		prevCurvePositions.add(getItemAt(i)->curvePosition);
		getItemAt(i)->curvePosition = curLength;
		if (i < numItems - 1) curLength += getItemAt(i)->getLength();

		bounds = bounds.getUnion(getItemAt(i)->easing->getBounds());
	}

	length->setValue(curLength);

	if (getNumItems() >= 2 && keySyncMode->boolValue() && prevLength > 0 && length->floatValue() > 0)
	{
		Automation* a = (Automation*)position->automation->automationContainer;
		a->callFunctionOnItems([&](auto k)
			{
				float kPrevPos = k->value->floatValue() * prevLength;

				if (relativeAutomationKeySyncMode)
				{
					Curve2DKey* prevKey = nullptr;
					int numItems = getNumItems();
					for (int j = numItems - 2; j >= 0; j--)
					{
						if (prevCurvePositions[j + 1] == prevCurvePositions[j]) continue;
						if (prevCurvePositions[j] <= kPrevPos)
						{
							prevKey = getItemAt(j);
							float relP = (kPrevPos - prevCurvePositions[j]) / (prevCurvePositions[j + 1] - prevCurvePositions[j]);
							float newPos = prevKey->curvePosition + relP * (getItemAt(j + 1)->curvePosition - prevKey->curvePosition);
							float newNormPos = newPos / length->floatValue();
							//DBG("kValue : " << k->value->floatValue() << ", prevLength : " << prevLength << ", length : " << length->floatValue() << ", prevCurvePos : " << prevCurvePositions[j] << ", nextCurvePos : " << prevCurvePositions[j + 1] << ", Prev Key : " << j << ", relP : " << relP << ", newPos " << newPos << " / normPos : " << newNormPos);
							k->value->setValue(newNormPos);
							break;
						}
					}
				}
				else
				{
					//DBG("prev length : " << prevLength << ", length : " << length->floatValue() << ", kPrevPos : " << kPrevPos);// << ", prevCurvePos : " << prevCurvePositions[j] << ", Prev Key : " << j << ", relP : " << relP << ", newPos " << newPos << " / normPos : " << newNormPos);
					k->value->setValue(kPrevPos / length->floatValue());
				}
			});
	}

	computeValue();
}

void Curve2D::computeValue()
{
	value->setPoint(getValueAtPosition(position->floatValue() * length->floatValue()));
}

Curve2DKey* Curve2D::getKeyForPosition(float pos)
{
	if (getNumItems() == 0) return nullptr;
	if (pos == 0) return getFirstItem();

	int numItems = getNumItems();
	for (int i = numItems - 1; i >= 0; i--)
	{
		if (getItemAt(i)->curvePosition <= pos) return getItemAt(i);
	}

	return nullptr;
}

Point<float> Curve2D::getValueAtNormalizedPosition(float pos)
{
	return getValueAtPosition(pos * length->floatValue());
}


Point<float> Curve2D::getValueAtPosition(float pos)
{
	if (getNumItems() == 0) return Point<float>();
	if (getNumItems() == 1) return getFirstItem()->viewUIPosition->getPoint();
	if (pos == length->floatValue())  return getLastItem()->viewUIPosition->getPoint();

	Curve2DKey* k = getKeyForPosition(pos);
	if (k == nullptr || k->easing == nullptr) return Point<float>();
	float normPos = (pos - k->curvePosition) / k->getLength();
	return k->easing->getValue(normPos);
}

void Curve2D::onContainerParameterChanged(Parameter* p)
{
	Manager::onContainerParameterChanged(p);
	if (p == position)
	{
		computeValue();
	}

}

void Curve2D::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	Manager::onControllableFeedbackUpdate(cc, c);

	if (Curve2DKey* k = dynamic_cast<Curve2DKey*>(cc))
	{
		updateCurve();
	}
}

void Curve2D::afterLoadJSONDataInternal()
{
	updateCurve();
}

InspectableEditor* Curve2D::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new Curve2DEditor(this, isRoot);
}

