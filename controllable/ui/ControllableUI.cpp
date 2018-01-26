#include "ControllableUI.h"
/*
  ==============================================================================

	ControllableUI.cpp
	Created: 9 Mar 2016 12:02:16pm
	Author:  bkupe

  ==============================================================================
*/
ControllableUI::ControllableUI(Controllable * controllable) :
	Component(controllable->niceName),
	controllable(controllable),
	showLabel(true),
	opaqueBackground(false),
	showMenuOnRightClick(true),
	forceFeedbackOnly(controllable->isControllableFeedbackOnly)
{
	jassert(controllable != nullptr);
	updateTooltip();
	controllable->addAsyncControllableListener(this);


	setEnabled(controllable->enabled);
	setAlpha(controllable->enabled ? 1 : .5f);

	setInterceptsMouseClicks(controllable->enabled, controllable->enabled);
}

ControllableUI::~ControllableUI()
{
	if (controllable.get())controllable->removeAsyncControllableListener(this);
}


void ControllableUI::mouseEnter(const MouseEvent & e)
{
	HelpBox::getInstance()->setOverData(controllable->helpID);
}

void ControllableUI::mouseExit(const MouseEvent & e)
{
	HelpBox::getInstance()->clearOverData(controllable->helpID);
}

void ControllableUI::mouseDown(const MouseEvent & e)
{
	if (e.mods.isRightButtonDown())
	{
		if (showMenuOnRightClick)
		{
			ScopedPointer<PopupMenu> p = new PopupMenu();
			addPopupMenuItems(p);

			
			if (controllable->includeInScriptObject)
			{
				p->addSeparator();
				p->addItem(-1, "Copy OSC Control Address");
				p->addItem(-2, "Copy Script Control Address");
			}

			if (p->getNumItems() == 0) return;

			int result = p->show();
			if (result > 0)
			{
				handleMenuSelectedID(result);
			} else if (result < 0)
			{
				switch (result)
				{
				case -1:
					SystemClipboard::copyTextToClipboard(controllable->controlAddress);
					break;
				case -2:
					SystemClipboard::copyTextToClipboard("root" + controllable->controlAddress.replaceCharacter('/', '.'));
					break;

				case -3:
					showEditWindow();
					break;
				default:
					DBG("Not handled : " << result);
				}
			}
		}
	} else
	{
		mouseDownInternal(e);
	}
}

void ControllableUI::mouseUp(const MouseEvent & e)
{
	if (e.mods.isRightButtonDown()) return;
	mouseUpInternal(e);

}

void ControllableUI::setOpaqueBackground(bool value)
{
	opaqueBackground = value;
}

void ControllableUI::setForceFeedbackOnly(bool value)
{
	setRepaintsOnMouseActivity(false);
	forceFeedbackOnly = value;
	setForceFeedbackOnlyInternal();
	repaint();
}

void ControllableUI::newMessage(const Controllable::ControllableEvent & e)
{
	switch (e.type)
	{
	case Controllable::ControllableEvent::CONTROLADDRESS_CHANGED:
		controllableControlAddressChanged();
		break;

	case Controllable::ControllableEvent::NAME_CHANGED:
		break;

	case Controllable::ControllableEvent::STATE_CHANGED:
		controllableStateChanged();
		break;

	case Controllable::ControllableEvent::CONTROLLABLE_REMOVED:
		break;

	case Controllable::ControllableEvent::FEEDBACK_STATE_CHANGED:
		feedbackStateChanged();
		break;

	default:
		//NOT HANDLED
		break;

	}
}

void ControllableUI::controllableStateChanged()
{
	setEnabled(controllable->enabled);
	setAlpha(controllable->enabled ? 1 : .5f);
	setInterceptsMouseClicks(controllable->enabled, controllable->enabled);
}

void ControllableUI::feedbackStateChanged()
{
	setForceFeedbackOnly(controllable->isControllableFeedbackOnly);
}

void ControllableUI::controllableControlAddressChanged()
{
	updateTooltip();
}

void ControllableUI::updateTooltip()
{
	tooltip = controllable->description + "\nControl Address : " + controllable->controlAddress;
	bool readOnly = false;
	if (controllable->type != Controllable::Type::TRIGGER)
	{
		tooltip += " (" + controllable->argumentsDescription + ")";
		if (((Parameter *)controllable.get())->isEditable == false) readOnly = true;

	}
	if (controllable->isControllableFeedbackOnly) readOnly = true;
	if (readOnly) tooltip += " (read only)";

	setTooltip(tooltip);
}
