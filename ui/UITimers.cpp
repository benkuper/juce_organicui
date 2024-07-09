#include "JuceHeader.h"
#include "UITimers.h"

juce_ImplementSingleton(OrganicUITimers)

OrganicUITimers::OrganicUITimers()
{
#if JUCE_MAC
	startTimer(ORGANICUI_DEFAULT_TIMER, 1000 / 20); //20 fps drawing on mac
	startTimer(ORGANICUI_SLOW_TIMER, 1000 / 15); //15 fps drawing on mac
#else
	startTimer(ORGANICUI_DEFAULT_TIMER, 1000 / 30); //30 fps drawing
	startTimer(ORGANICUI_SLOW_TIMER, 1000 / 20); //20 fps drawing
#endif

	timerMap.set(ORGANICUI_DEFAULT_TIMER, {});
	timerMap.set(ORGANICUI_SLOW_TIMER, {});

}

void OrganicUITimers::registerTarget(int timerID, UITimerTarget* ui)
{
	if (timerMap.contains(timerID)) timerMap.getReference(timerID).addIfNotAlreadyThere(ui);
}

void OrganicUITimers::unregisterTarget(int timerID, UITimerTarget* ui)
{
	if (timerMap.contains(timerID)) timerMap.getReference(timerID).removeAllInstancesOf(ui);
}

void OrganicUITimers::timerCallback(int timerID)
{
	if (timerMap.contains(timerID))
	{
		Array<WeakReference<UITimerTarget>> params = timerMap[timerID];

		for (auto& p : params)
		{
			if (p.wasObjectDeleted())
			{
				jassertfalse;
				continue;
			}

			p->handlePaintTimer();
		}
	}
}

UITimerTarget::UITimerTarget(int timerID) :
	shouldRepaint(false),
	paintTimerID(timerID)
{
	if (paintTimerID >= 0) OrganicUITimers::getInstance()->registerTarget(paintTimerID, this);
}


UITimerTarget::~UITimerTarget()
{
	if (paintTimerID >= 0
		&& OrganicUITimers::getInstanceWithoutCreating() != nullptr) OrganicUITimers::getInstance()->unregisterTarget(paintTimerID, this);
	masterReference.clear();
}

void UITimerTarget::handlePaintTimer()
{
	if (!shouldRepaint) return;
	handlePaintTimerInternal();
	shouldRepaint = false;
}
