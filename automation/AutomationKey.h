/*
  ==============================================================================

    AutomationKey.h
    Created: 11 Dec 2016 1:22:20pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationKeyBase :
	public BaseItem
{
public:
	AutomationKeyBase();
	virtual ~AutomationKeyBase();

	FloatParameter * position;
	Parameter* value;

	int numDimensions;

	EnumParameter * easingType;

	std::unique_ptr<EasingBase> easingBase;
	virtual void setEasing(EasingBase::Type t) = 0;

	virtual Array<float> getValues() = 0;

	float getRelativePosition(float pos1, float pos2, float weight) const;

	void setSelectionManager(InspectableSelectionManager * ism) override;
	void onContainerParameterChangedInternal(Parameter *) override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationKeyBase)
};


template<class T>
class AutomationKey :
	public AutomationKeyBase
{
public:
	AutomationKey();
	virtual ~AutomationKey() {}

	Easing<T>* easing;

	virtual void setEasing(EasingBase::Type t) override;
	T getValue() const;
	T getValue(AutomationKey<T>* nextKey, const float& targetPos) const;
	virtual Array<float> getValues() override;

};



template<class T>
AutomationKey<T>::AutomationKey() :
	AutomationKeyBase()
{
	if (std::is_same<T, float>)
	{
		value = addFloatParameter("Value", "Value of the key", 0);
		numDimensions = 1;
	}
	else if (std::is_same < T, Point<float>)
	{
		value = addPoint2DParameter("Position", "Position of the key");
		numDimensions = 2;
	}
	else if (std::is_same < T, Vector3D<float>)
	{
		value = addPoint3DParameter("Position", "Position of the key");
		numDimensions = 3;
	}

	easing = easingBase.get();
}

template<class T>
void AutomationKey<T>::setEasing(EasingBase::Type t)
{
	if (easingBase != nullptr)
	{
		if (easingBase->type == t) return;
		removeChildControllableContainer(easingBase.get());
		easing = nullptr;
	}

	Easing<T> * e = nullptr;
	switch (t)
	{
	case EasingBase::LINEAR:
		e = new LinearEasing<T>();
		break;

	case EasingBase::HOLD:
		e = new HoldEasing<T>();
		break;

	/*
	case EasingBase::BEZIER:
		e = new CubicEasing<T>();
		break;

	case EasingBase::SINE:
		e = new  SineEasing<T>();
		break;
		*/
	}

	easingBase.reset(e);
	if (easing != nullptr)
	{
		easing = (Easing<T> *)easingBase.get();
		easing->setSelectionManager(selectionManager);
		addChildControllableContainer(easing);
	}
}

template<> float AutomationKey<float>::getValue() const { return value->floatValue(); }
template<> Point<float> AutomationKey<Point<float>>::getValue() const { return ((Point2DParameter*)value)->getPoint(); }
template<> Vector3D<float> AutomationKey<Vector3D<float>>::getValue() const { return ((Point3DParameter*)value)->getVector(); }

template<class T>
T AutomationKey<T>::getValue(AutomationKey<T>* nextKey, const float& position) const
{
	return easing->getValue(getValue(), nextKey->getValue(), getRelativePosition(position->floatValue(), nextKey->position->floatValue(), targetPos));
}

template<class T>
Array<float> AutomationKey<T>::getValues()
{
	return Array<float>();
}

template<>
Array<float> AutomationKey<float>::getValues()
{
	float val = getValue();

	Array<float> result;
	result.add(val);
	return  result;
}

template<>
Array<float> AutomationKey<Point<float>>::getValues()
{
	Point<float> val = getValue();

	Array<float> result;
	result.add(val.x,val.y);
	return  result;
}

template<>
Array<float> AutomationKey<Vector3D<float>>::getValues()
{
	Vector3D<float> val = getValue();

	Array<float> result;
	result.add(val.x,val.y,val.z);
	return  result;
}

