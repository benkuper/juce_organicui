#pragma once

class Grabber : public juce::Component
{
public:

	enum Direction { VERTICAL, HORIZONTAL };
	Grabber(Direction d = HORIZONTAL);
	
	~Grabber() {}

	Direction dir;

	void paint(juce::Graphics &g) override;
};
