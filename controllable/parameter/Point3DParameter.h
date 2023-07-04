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
	Point3DParameter(const String& niceName, const String& description, bool enabled = true);
	~Point3DParameter() {}

	float x, y, z;
	FloatParameter::UIType defaultUI;
	int stringDecimals;


	void setVector(Vector3D<float> value);
	void setVector(float x, float y, float z);
	UndoableAction* setUndoableVector(Vector3D<float> oldVector, Vector3D<float> newVector, bool onlyReturnAction = false);
	UndoableAction* setUndoableVector(float oldX, float oldY, float oldZ, float newX, float newY, float newZ, bool onlyReturnAction = false);
	
	void setValueInternal(var& _value) override;
	void setBounds(float _minX, float _minY, float _minZ, float _maxX, float _maxY, float _maxZ);

	Vector3D<float> getVector();
	virtual var getLerpValueTo(var targetValue, float weight) override;
	virtual void setWeightedValue(Array<var> values, Array<float> weights) override;

	bool checkValueIsTheSame(var newValue, var oldValue) override;

	virtual bool setAttributeInternal(String name, var val) override;
	virtual StringArray getValidAttributes() const override;

	virtual StringArray getValuesNames() override;

	ControllableUI* createDefaultUI(Array<Controllable*> controllables = {}) override;

	static Point3DParameter* create() { return new Point3DParameter("New Point3D Parameter", ""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Point3D"; }

protected:
	var getCroppedValue(var originalValue) override;
};