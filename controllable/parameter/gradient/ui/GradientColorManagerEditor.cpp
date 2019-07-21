#include "GradientColorManagerEditor.h"
#pragma once

GradientColorManagerEditor::GradientColorManagerEditor(GradientColorManager* manager, bool isRoot) :
	InspectableEditor(manager, isRoot),
	managerUI(manager)
{
	addAndMakeVisible(&managerUI);
	setSize(100, 60);
}

GradientColorManagerEditor::~GradientColorManagerEditor()
{
}

void GradientColorManagerEditor::resized()
{
	managerUI.setBounds(getLocalBounds());
}
