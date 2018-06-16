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
	showMenuOnRightClick(true)
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
			showContextMenu();
		}
	} else
	{
		if(isInteractable()) mouseDownInternal(e);
	}
}

void ControllableUI::mouseUp(const MouseEvent & e)
{
	if (e.mods.isRightButtonDown()) return;
	if(isInteractable()) mouseUpInternal(e);

}

bool ControllableUI::isInteractable()
{
	return controllable->enabled && !controllable->isControllableFeedbackOnly;
}

void ControllableUI::showContextMenu()
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

	if (result != 0)
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
			handleMenuSelectedID(result);
		}
	}
}

void ControllableUI::setOpaqueBackground(bool value)
{
	opaqueBackground = value;
}

void ControllableUI::newMessage(const Controllable::ControllableEvent & e)
{
	switch (e.type)
	{
	case Controllable::ControllableEvent::CONTROLADDRESS_CHANGED:
	{
		updateTooltip();
		controllableControlAddressChanged();

	}
		break;

	case Controllable::ControllableEvent::NAME_CHANGED:
		break;

	case Controllable::ControllableEvent::STATE_CHANGED:
	{
		setEnabled(controllable->enabled);
		setAlpha(controllable->enabled ? 1 : .5f);
		setInterceptsMouseClicks(isInteractable(), isInteractable());
		controllableStateChanged();
	}
		break;

	case Controllable::ControllableEvent::CONTROLLABLE_REMOVED:
		break;

	case Controllable::ControllableEvent::FEEDBACK_STATE_CHANGED:
	{
		setInterceptsMouseClicks(isInteractable(), isInteractable());
		feedbackStateChanged();
	}
		break;

	default:
		//NOT HANDLED
		break;

	}
}

void ControllableUI::updateTooltip()
{
	tooltip = controllable->description + "\nControl Address : " + controllable->controlAddress;
	bool readOnly = false;
	if (controllable->type != Controllable::Type::TRIGGER)
	{
		tooltip += " (" + controllable->argumentsDescription + ")";
		if (((Parameter *)controllable.get())->isControllableFeedbackOnly == false) readOnly = true;

	}
	if (controllable->isControllableFeedbackOnly) readOnly = true;
	if (readOnly) tooltip += " (read only)";

	setTooltip(tooltip);
}
