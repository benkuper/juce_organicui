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
	Point2DParameter(const String &niceName, const String &description, bool enabled = true);
	~Point2DParameter() {}

	float x, y;
	FloatParameter::UIType defaultUI;

	void setPoint(Point<float> value);
	void setPoint(float x, float y);
	UndoableAction * setUndoablePoint(Point<float> oldPoint, Point<float> newPoint, bool onlyReturnAction = false);
	UndoableAction* setUndoablePoint(float oldX, float oldY, float newX, float newY, bool onlyReturnAction = false);

	void setValueInternal(var  &_value) override;
	void setBounds(float _minX, float _minY, float _maxX, float _maxY);
	
	virtual void setAttribute(String name, var val) override;

	virtual StringArray getValuesNames() override;

	Point<float> getPoint();
	virtual var getLerpValueTo(var targetValue, float weight) override;
	virtual void setWeightedValue(Array<var> values, Array<float> weights) override;

	bool checkValueIsTheSame(var newValue, var oldValue) override;

	ControllableUI * createDefaultUI() override;

	static Point2DParameter * create() { return new Point2DParameter("New Point2D Parameter", ""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Point2D"; }



protected:
	virtual var getCroppedValue(var originalValue) override;
};
