#include "JuceHeader.h"
#include "UITimers.h"

juce_ImplementSingleton(OrganicUITimers)

OrganicUITimers::OrganicUITimers()
{
	startTimer(ORGANICUI_DEFAULT_TIMER, 1000 / 30); //30 fps drawing
	startTimer(ORGANICUI_SLOW_TIMER, 1000 / 20); //20 fps drawing

	timerMap.set(ORGANICUI_DEFAULT_TIMER, {});
	timerMap.set(ORGANICUI_SLOW_TIMER, {});

#ifdef ORGANICUI_LOG_FPS_DEBUG
	startTimer(ORGANICUI_FPS_TIMER, 1000); //1 fps log
#endif
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
	if(timerID == ORGANICUI_FPS_TIMER){
		DBG("Default timer: " << fps[0] << "fps, slow timer: " << fps[1] << "fps");
		fps.set(0, 0);
		fps.set(1, 0);
		return;
	}
#endif

	if (timerMap.contains(timerID))
	{
		Array<WeakReference<UITimerTarget>> params = timerMap[timerID];

		if(lastRepaintTimes.contains(timerID)){
			juce::uint32 timeDiff = juce::Time::getMillisecondCounter() - lastRepaintTimes[timerID];

			if(timeDiff < (getTimerInterval(timerID) * 13 / 10)) return;
		}

#ifdef ORGANICUI_LOG_FPS_DEBUG
		fps.set(timerID, fps[timerID] + 1);
#endif

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

UITimerTarget::UITimerTarget(int timerID, juce::String _name) :
	shouldRepaint(false),
	paintingAsked(false),
	name(_name),
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
	paintingAsked = true;
	handlePaintTimerInternal();
	shouldRepaint = false;
}

void UITimerTarget::validatePaint(){
	if (paintingAsked && paintTimerID >= 0)
	{
		paintingAsked = false;
		OrganicUITimers::getInstance()->lastRepaintTimes.set(paintTimerID, juce::Time::getMillisecondCounter());
	}
}
