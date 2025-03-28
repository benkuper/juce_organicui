/*
  ==============================================================================

    GapGrabber.cpp
    Created: 2 May 2016 5:40:46pm
    Author:  bkupe

  ==============================================================================
*/

GapGrabber::GapGrabber(Direction _direction) :
	UITimerTarget(ORGANICUI_DEFAULT_TIMER, "GapGrabber"),
	direction(_direction)
{
	setRepaintsOnMouseActivity(true);
	setMouseCursor(direction == HORIZONTAL ? MouseCursor::LeftRightResizeCursor : MouseCursor::UpDownResizeCursor);
}

GapGrabber::~GapGrabber()
{
}

void GapGrabber::paint(Graphics & g)
{
	int grabberSize = 100;
 juce::Rectangle<int> r = getLocalBounds().reduced(2);
 juce::Rectangle<int> tr = (direction == Direction::HORIZONTAL) ? r.withHeight(grabberSize) : r.withWidth(grabberSize);
	tr.setCentre(r.getCentre());
	Colour c = BG_COLOR.brighter(.1f);
	if (isMouseOver()) c = HIGHLIGHT_COLOR;
	if (isMouseButtonDown()) c = Colours::yellow;

	g.setColour(c);
	g.fillRoundedRectangle(tr.toFloat(), 2);
}

void GapGrabber::mouseDrag(const MouseEvent &e)
{
	position = direction == HORIZONTAL?e.getPosition().x:e.getPosition().y;
	shouldRepaint = true;
}

void GapGrabber::mouseUp(const MouseEvent &e)
{
	listeners.call(&Listener::grabberGrabEvent, this, false);
}

void GapGrabber::mouseDown(const MouseEvent &e)
{
	listeners.call(&Listener::grabberGrabEvent, this, true);
}

void GapGrabber::handlePaintTimerInternal()
{
	listeners.call(&Listener::grabberGrabUpdate, this, position);
}
