#pragma once

class Grabber : public Component
{
public:

	enum Direction { VERTICAL, HORIZONTAL };
	Grabber(Direction d = HORIZONTAL) : dir(d) {}
	
	~Grabber() {}

	Direction dir;

	void paint(Graphics &g) override;
};
