/*
  ==============================================================================

    AutomationKeyUI.h
    Created: 11 Dec 2016 1:22:27pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationKeyUI :
	public BaseItemMinimalUI<AutomationKey>
{
public:
	AutomationKeyUI(AutomationKey *);
	virtual ~AutomationKeyUI();
    
	OwnedArray<EasingUI> easingsUI;

    Array<int> keyYPos1;
    Array<int> keyYPos2;
    
    float posAtMouseDown;
    float valueAtMouseDown;

	bool showHandles;
	
	const static int handleSize = 6;
	const static int handleClickZone = 10;

    void setShowHandles(bool value);

	class Handle :
		public Component
	{
	public:
		Handle(int dimensionIndex, Colour c);
		bool highlight;
		Colour color;
		int dimensionIndex;
		void paint(Graphics &g) override;
	};

    OwnedArray<Handle> handles;
	Handle* draggingHandle;


	void paint(Graphics &) override; //avoid default item painting

	void updateEasingsUI();

	void setKeyPositions(const Array<int> k1, const Array<int> k2);

	void showKeyEditorWindow();
	
	void resized() override;

	bool hitTest(int tx, int ty) override;

	void mouseDown(const MouseEvent &e) override;
	void mouseUp(const MouseEvent &e) override;

	void controllableFeedbackUpdateInternal(Controllable * c) override;

	void inspectableSelectionChanged(Inspectable *) override;
	void inspectablePreselectionChanged(Inspectable *) override;
};
