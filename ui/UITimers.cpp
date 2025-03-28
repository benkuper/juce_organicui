#include "JuceHeader.h"
#include "UITimers.h"

juce_ImplementSingleton(OrganicUITimers)


OrganicUITimers::OrganicUITimers()
{
	timerMap.set(ORGANICUI_DEFAULT_TIMER, {});
	timerMap.set(ORGANICUI_SLOW_TIMER, {});

#ifdef ORGANICUI_LOG_FPS_DEBUG
	startTimer(ORGANICUI_FPS_TIMER, 1000); //1 fps log
#endif

	setupTimers();
}

void OrganicUITimers::setupTimers()
{
	stopTimer(ORGANICUI_DEFAULT_TIMER);
	stopTimer(ORGANICUI_SLOW_TIMER);

	startTimer(ORGANICUI_DEFAULT_TIMER, 1000 / GlobalSettings::getInstance()->uiRefreshRate->intValue());
	startTimer(ORGANICUI_SLOW_TIMER, 1500 / GlobalSettings::getInstance()->uiRefreshRate->intValue());
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
#ifdef ORGANICUI_LOG_FPS_DEBUG
	if (timerID == ORGANICUI_FPS_TIMER) {
		DBG("Default timer: " << fps[0] << "fps, slow timer: " << fps[1] << "fps");
		fps.set(0, 0);
		fps.set(1, 0);
		return;
	}
#endif

	if (timerMap.contains(timerID))
	{
		Array<WeakReference<UITimerTarget>> targets = timerMap[timerID];

#ifdef ORGANICUI_LOG_FPS_DEBUG
		fps.set(timerID, fps[timerID] + 1);
#endif

		int curTime = juce::Time::getMillisecondCounter();

		for (auto& t : targets)
		{
			if (t.wasObjectDeleted())
			{
				jassertfalse;
				continue;
			}


			if (t->safeRepaintCheck)
			{
				int timeDiff = curTime - t->lastRepaintTime;
				if (timeDiff < getTimerInterval(timerID) / 2) continue;
			}

			t->handlePaintTimer();
		}
	}
}

UITimerTarget::UITimerTarget(int timerID, juce::String _name, bool safeRepaintCheck) :
	shouldRepaint(false),
	name(_name),
	paintTimerID(timerID),
	safeRepaintCheck(safeRepaintCheck),
	lastRepaintTime(0)
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
	paintingAsked = true;
	handlePaintTimerInternal();
	shouldRepaint = false;
}

void UITimerTarget::validatePaint(){
	if (paintingAsked && paintTimerID >= 0)
	{
		paintingAsked = false;
		lastRepaintTime = Time::getMillisecondCounter();
	}
}
