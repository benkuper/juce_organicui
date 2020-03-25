/*
  ==============================================================================

    Curve2DEditor.cpp
    Created: 5 Feb 2017 2:58:27pm
    Author:  Ben

  ==============================================================================
*/

Curve2DEditor::Curve2DEditor(Curve2D * curve, bool isRoot) :
	GenericControllableContainerEditor(curve, isRoot),
	curve(curve),
	curveUI(nullptr)
{
	positionUI.reset(curve->position->createSlider());
	valueUI.reset((ParameterUI*)curve->value->createDefaultUI());
	valueUI->showLabel = false;

	addAndMakeVisible(positionUI.get());
	addAndMakeVisible(valueUI.get());

	if (curve->showUIInEditor)
	{
		curveUI.reset(new Curve2DUI(curve));

		curveUI->bgColor = BG_COLOR;
		curveUI->transparentBG = false;
		addAndMakeVisible(curveUI.get());
		setSize(100, 400);
	}

	
}

Curve2DEditor::~Curve2DEditor()
{
}

void Curve2DEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (r.getWidth() == 0 || r.getHeight() == 0) return;
	valueUI->setBounds(r.removeFromRight(100).reduced(3));
	r.removeFromRight(2);
	positionUI->setBounds(r.removeFromRight(100).reduced(3));
	GenericControllableContainerEditor::resizedInternalHeader(r);
}

void Curve2DEditor::resizedInternalContent(juce::Rectangle<int> &r)
{
	if (curveUI != nullptr)
	{
		if (curveUI != nullptr) curveUI->setBounds(r);
		r.translate(0, curveUI->getHeight());
	}
	
	r.setHeight(0);
}
