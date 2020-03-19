/*
==============================================================================

Point3DParameter.h
Created: 30 Sep 2016 9:37:36am
Author:  bkupe

==============================================================================
*/

#pragma once

class Point3DParameter : public Parameter
{
public:
	Point3DParameter(const String &niceName, const String &description, bool enabled = true);
	~Point3DParameter() {}

	float x, y, z;
	FloatParameter::UIType defaultUI;


	void setVector(Vector3D<float> value);
	void setVector(float x, float y, float z);
	void setUndoableVector(Vector3D<float> oldVector, Vector3D<float> newVector);
	void setUndoableVector(float oldX, float oldY, float oldZ, float newX, float newY, float newZ);

	void setValueInternal(var & _value) override;

	void setBounds(float _minX, float _minY, float _minZ, float _maxX, float _maxY, float _maxZ);
	void clearRange() override;

	Vector3D<float> getVector();
	virtual var getLerpValueTo(var targetValue, float weight) override;
	virtual void setWeightedValue(Array<var> values, Array<float> weights) override;

	bool checkValueIsTheSame(var newValue, var oldValue) override;

	virtual StringArray getValuesNames() override;

	ControllableUI * createDefaultUI(Controllable * targetControllable = nullptr) override;

	static Point3DParameter * create() { return new Point3DParameter("New Point3D Parameter", ""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Point3D"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Point3DParameter)
};