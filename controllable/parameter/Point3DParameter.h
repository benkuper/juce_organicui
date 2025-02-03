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
	Point3DParameter(const juce::String& niceName, const juce::String& description, bool enabled = true);
	~Point3DParameter() {}

	float x, y, z;
	FloatParameter::UIType defaultUI;
	int stringDecimals;


	void setVector(juce::Vector3D<float> value);
	void setVector(float x, float y, float z);
	juce::Array<juce::UndoableAction*> setUndoableVector(juce::Vector3D<float> newVector, bool onlyReturnAction = false, bool setSimilarSelected = false);
	juce::Array<juce::UndoableAction*> setUndoableVector(float newX, float newY, float newZ, bool onlyReturnAction = false, bool setSimilarSelected = false);
	
	void setValueInternal(juce::var& _value) override;
	void setBounds(float _minX, float _minY, float _minZ, float _maxX, float _maxY, float _maxZ);

	juce::Vector3D<float> getVector();
	virtual juce::var getLerpValueTo(juce::var targetValue, float weight) override;
	virtual void setWeightedValue(juce::Array<juce::var> values, juce::Array<float> weights) override;

	bool checkValueIsTheSame(juce::var newValue, juce::var oldValue) override;

	virtual bool setAttributeInternal(juce::String name, juce::var val) override;
	virtual juce::StringArray getValidAttributes() const override;

	virtual juce::StringArray getValuesNames() override;

	ControllableUI* createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	static Point3DParameter* create() { return new Point3DParameter("New Point3D Parameter", ""); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Point3D"; }

protected:
	juce::var getCroppedValue(juce::var originalValue) override;
};