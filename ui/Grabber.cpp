Grabber::Grabber(Direction d) : dir(d)
{
	setMouseCursor(dir == VERTICAL ? MouseCursor::LeftRightResizeCursor : MouseCursor::UpDownResizeCursor);
}

void Grabber::paint(Graphics & g)
{
	//juce::Rectangle<int> r = getLocalBounds();
	g.setColour(BG_COLOR.brighter(.6f));
	const int numLines = 3;
	for (int i = 0; i < numLines; ++i)
	{
		if (dir == HORIZONTAL)
		{
			float th = (i + 1)*(float)getHeight() / ((float)numLines + 1);
			g.drawLine(0, th, (float)getWidth(), th, 1);
		}
		else
		{
			float tw = (i + 1)*(float)getWidth() / ((float)numLines + 1);
			g.drawLine(tw, 0, tw, (float)getHeight(), 1);
		}

	}
}

