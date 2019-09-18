/*
  ==============================================================================

	ControllableUI.cpp
	Created: 9 Mar 2016 12:02:16pm
	Author:  bkupe

  ==============================================================================
*/

std::function<void(ControllableUI*)> ControllableUI::customShowContextMenuFunc = nullptr;
std::function<void(ControllableUI *, PopupMenu *)> ControllableUI::customAddToContextMenuFunc = nullptr;
std::function<bool(ControllableUI *, int)> ControllableUI::handleCustomContextMenuResultFunc = nullptr;
std::function<void(ControllableUI*)> ControllableUI::customShowEditWindowFunction = nullptr;

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
	if (controllable != nullptr) HelpBox::getInstance()->setOverData(controllable->helpID);
}

void ControllableUI::mouseExit(const MouseEvent & e)
{
	if (controllable != nullptr) HelpBox::getInstance()->clearOverData(controllable->helpID);
}

void ControllableUI::mouseDown(const MouseEvent & e)
{
	if (e.mods.isRightButtonDown())
	{
		if (showMenuOnRightClick)
		{
			showContextMenu();
		}
	}
	else
	{
		if (isInteractable()) mouseDownInternal(e);
	}
}

void ControllableUI::mouseUp(const MouseEvent & e)
{
	if (e.mods.isRightButtonDown()) return;
	if (isInteractable()) mouseUpInternal(e);

}

bool ControllableUI::isInteractable()
{
	return controllable->enabled && !controllable->isControllableFeedbackOnly;
}

void ControllableUI::showContextMenu()
{
	if (customShowContextMenuFunc != nullptr)
	{
		customShowContextMenuFunc(this); //full override
		return;
	}
	
	
	std::unique_ptr<PopupMenu> p(new PopupMenu());
	
	
	addPopupMenuItems(p.get());

	
	if (ControllableUI::customAddToContextMenuFunc != nullptr)
	{
		p->addSeparator();
		ControllableUI::customAddToContextMenuFunc(this, p.get());
	}

	if (controllable->includeInScriptObject)
	{
		p->addSeparator();
		p->addItem(-1, "Copy OSC Control Address");
		p->addItem(-2, "Copy Script Control Address");
	}

	p->addSeparator();
	PopupMenu dashboardMenu;
	int index = 0;
	for (auto &di : DashboardManager::getInstance()->items)
	{
		dashboardMenu.addItem(index + 10000, di->niceName);
		index++;
	}
	p->addSubMenu("Send to Dashboard", dashboardMenu);


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
			if (result >= 10000)
			{
				DashboardManager::getInstance()->items[result - 10000]->itemManager.addItem(new DashboardTargetItem(controllable));
			}
			else if (ControllableUI::handleCustomContextMenuResultFunc != nullptr)
			{
				bool handled = ControllableUI::handleCustomContextMenuResultFunc(this, result);
				if (!handled) handleMenuSelectedID(result);
			}
			else
			{
				handleMenuSelectedID(result);
			}
		}
	}
}

void ControllableUI::setOpaqueBackground(bool value)
{
	opaqueBackground = value;
}

void ControllableUI::showEditWindow()
{
	if (customShowEditWindowFunction != nullptr) customShowEditWindowFunction(this);
	else showEditWindowInternal();
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
		repaint();
	}
	break;

	case Controllable::ControllableEvent::CONTROLLABLE_REMOVED:
		break;

	case Controllable::ControllableEvent::FEEDBACK_STATE_CHANGED:
	{
		setInterceptsMouseClicks(isInteractable(), isInteractable());
		feedbackStateChanged();
		repaint();
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
	if (controllable->type != Controllable::Type::TRIGGER) tooltip += " (" + controllable->argumentsDescription + ")";
	if (controllable->isControllableFeedbackOnly) tooltip += " (read only)";

	setTooltip(tooltip);
}
