/*
  ==============================================================================

    DashboardUI.h
    Created: 19 Apr 2017 11:06:43pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemUI :
	public BaseItemMinimalUI<DashboardItem>,
	public Parameter::AsyncListener,
	public SettableTooltipClient
{
public:
	DashboardItemUI(DashboardItem * item);
	~DashboardItemUI();

	virtual void paint(Graphics& g) override;
	virtual void paintOverChildren(Graphics& g) override;
	virtual void resized() override;
	virtual void resizedDashboardItemInternal() {}

	virtual void mouseDown(const MouseEvent& e) override;
	virtual void mouseEnter(const MouseEvent& e) override;
	virtual void mouseDrag(const MouseEvent& e) override;
	virtual void mouseUp(const MouseEvent& e) override;

	virtual void updateEditMode();
	virtual void updateEditModeInternal(bool editMode) {}

	BaseItemUI<DashboardItem>::ItemResizerComponent resizer;

	virtual void newMessage(const Parameter::ParameterEvent& e) override;

};