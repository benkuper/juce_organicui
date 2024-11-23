/*
  ==============================================================================

    Point2DParameter.h
    Created: 30 Sep 2016 9:37:36am
    Author:  bkupe

  ==============================================================================
*/
#pragma once

class Point2DParameter : public Parameter
{
public:
	Point2DParameter(const juce::String& niceName, const juce::String &description, bool enabled = true);
	~Point2DParameter() {}

	float x, y;
	FloatParameter::UIType defaultUI;
	int stringDecimals;
	bool showExtendedEditor;
	bool extendedEditorInvertX;
	bool extendedEditorInvertY;
	bool extendedEditorStretchMode;
	bool canShowExtendedEditor;

	void setPoint(juce::Point<float> value, bool setSimilarSelected = false);
	void setPoint(float x, float y, bool setSimilarSelected = false);
	juce::Array<juce::UndoableAction*> setUndoablePoint(juce::Point<float> newPoint, bool onlyReturnAction = false, bool setSimilarSelected = false);
	juce::Array<juce::UndoableAction*> setUndoablePoint(float newX, float newY, bool onlyReturnAction = false, bool setSimilarSelected = false);

	void setDefaultPoint(juce::Point<float> value, bool doResetValue = true);
	void setDefaultPoint(float x, float y, bool doResetValue = true);

	void setValueInternal(juce::var  &_value) override;
	void setBounds(float _minX, float _minY, float _maxX, float _maxY);
	
	virtual bool setAttributeInternal(juce::String name, juce::var val) override;
	virtual juce::StringArray getValidAttributes() const override;

	virtual juce::StringArray getValuesNames() override;

	juce::Point<float> getPoint();
	virtual juce::var getLerpValueTo(juce::var targetValue, float weight) override;
	virtual void setWeightedValue(juce::Array<juce::var> values, juce::Array<float> weights) override;

	void setShowExtendedEditor(bool value);

	bool checkValueIsTheSame(juce::var newValue, juce::var oldValue) override;

	ControllableUI * createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	static Point2DParameter * create() { return new Point2DParameter("New Point2D Parameter", ""); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Point2D"; }

	virtual juce::var getJSONDataInternal() override;
	virtual void loadJSONDataInternal(juce::var data) override;


protected:
	virtual juce::var getCroppedValue(juce::var originalValue) override;
};
