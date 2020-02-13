#pragma once

class AutomationKey2DUI :
	public BaseItemMinimalUI<AutomationKey>
{
public:
	AutomationKey2DUI(AutomationKey * key);
	~AutomationKey2DUI();

	void paint(Graphics& g) override;
};
