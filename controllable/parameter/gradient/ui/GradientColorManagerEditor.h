#pragma once

class GradientColorManagerEditor :
	public InspectableEditor
{
public:
	GradientColorManagerEditor(GradientColorManager * manager, bool isRoot);
	~GradientColorManagerEditor();

	GradientColorManager* manager;
	GradientColorManagerUI managerUI;

	void resized() override;
};