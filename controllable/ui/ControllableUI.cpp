/*
  ==============================================================================

	ControllableUI.cpp
	Created: 9 Mar 2016 12:02:16pm
	Author:  bkupe

  ==============================================================================
*/

bool ControllableUI::showOSCControlAddressOption = true;
bool ControllableUI::showScriptControlAddressOption = true;
bool ControllableUI::showDetectiveOption = true;
bool ControllableUI::showDashboardOption = true;

std::function<void(ControllableUI*)> ControllableUI::customShowContextMenuFunc = nullptr;
std::function<void(ControllableUI *, PopupMenu *)> ControllableUI::customAddToContextMenuFunc = nullptr;
std::function<bool(ControllableUI *, int)> ControllableUI::handleCustomContextMenuResultFunc = nullptr;
std::function<void(ControllableUI*)> ControllableUI::customShowEditWindowFunction = nullptr;

ControllableUI::ControllableUI(Controllable * controllable) :
	Component(controllable->niceName),
	controllable(controllable),
	showLabel(true),
	opaqueBackground(false),
	showMenuOnRightClick(true),
	useCustomTextColor(false),
    customContourThickness(1)
{
	jassert(controllable != nullptr);
	updateTooltip();
	controllable->addAsyncControllableListener(this);

	setEnabled(controllable->enabled);
	setAlpha(controllable->enabled ? 1 : .5f);

	updateUIParams();
}

ControllableUI::~ControllableUI()
{
	if (controllable != nullptr && !controllable.wasObjectDeleted()) controllable->removeAsyncControllableListener(this);
}


void ControllableUI::paintOverChildren(Graphics& g)
{
	if (useCustomContour) drawContour(g);

}

void ControllableUI::mouseEnter(const MouseEvent & e)
{
	if (controllable != nullptr && !controllable.wasObjectDeleted()) HelpBox::getInstance()->setOverData(controllable->helpID);
}

void ControllableUI::mouseExit(const MouseEvent & e)
{
	setTooltip(tooltip); //restore tooltip when leaving
	if (controllable != nullptr && !controllable.wasObjectDeleted()) HelpBox::getInstance()->clearOverData(controllable->helpID);
}

void ControllableUI::mouseDown(const MouseEvent & e)
{
	if (controllable == nullptr && controllable.wasObjectDeleted()) return;

	setTooltip("");  //force not showing tooltip after click
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

void ControllableUI::drawContour(Graphics &g)
{
	g.setColour(customContourColor);
	g.drawRoundedRectangle(getLocalBounds().toFloat(), 2, customContourThickness);
}

bool ControllableUI::isInteractable(bool falseIfFeedbackOnly)
{
	if (controllable == nullptr || controllable.wasObjectDeleted()) return false;
	if (!controllable->enabled) return false;
	if (falseIfFeedbackOnly && controllable->isControllableFeedbackOnly) return false;
	return true;
}

void ControllableUI::showContextMenu()
{
	if (controllable == nullptr || controllable.wasObjectDeleted()) return;

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

	if (controllable->userCanSetReadOnly)
	{
		p->addSeparator();
		p->addItem(-11, "Read Only", true, controllable->isControllableFeedbackOnly);
	}

	if (controllable->includeInScriptObject)
	{
		p->addSeparator();
		if(showOSCControlAddressOption) p->addItem(-1, "Copy OSC Control Address");
		if(showScriptControlAddressOption) p->addItem(-2, "Copy Script Control Address");
	}


	p->addSeparator();

	if (showDetectiveOption)
	{
		
		if (controllable->type == Controllable::FLOAT ||
			controllable->type == Controllable::INT ||
			controllable->type == Controllable::POINT2D ||
			controllable->type == Controllable::POINT3D ||
			controllable->type == Controllable::COLOR ||
			controllable->type == Controllable::BOOL)
		{
			p->addItem(-10, "Watch this with The Detective");
		}
	}
	
	if (showDashboardOption)
	{
		PopupMenu dashboardMenu;
		int index = 0;
		for (auto& di : DashboardManager::getInstance()->items)
		{
			dashboardMenu.addItem(index + 10000, di->niceName);
			index++;
		}
		p->addSubMenu("Send to Dashboard", dashboardMenu);
	}
	

	if (p->getNumItems() == 0) return;

	int result = p->show();



	if (result != 0)
	{
		if (controllable == nullptr || controllable.wasObjectDeleted()) return; //in cas it has been deleted while menu was out...

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

		case -10:
			Detective::getInstance()->watchControllable(controllable);
			break;

		case -11:
			controllable->setControllableFeedbackOnly(!controllable->isControllableFeedbackOnly);
			break;

		default:
			if (result >= 10000)
			{
				DashboardManager::getInstance()->items[result - 10000]->itemManager.addItem(controllable->createDashboardItem());
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

void ControllableUI::updateUIParams()
{
	setInterceptsMouseClicks(isInteractable(false), isInteractable(false));
	updateUIParamsInternal();
}

void ControllableUI::setOpaqueBackground(bool value)
{
	opaqueBackground = value;
	updateUIParams();
}

void ControllableUI::showEditWindow()
{
	if (!isInteractable()) return;

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
		//setInterceptsMouseClicks(isInteractable(), isInteractable());
		controllableStateChanged();
		feedbackStateChanged();
		repaint();
	}
	break;

	case Controllable::ControllableEvent::CONTROLLABLE_REMOVED:
		break;

	case Controllable::ControllableEvent::FEEDBACK_STATE_CHANGED:
	{
		//setInterceptsMouseClicks(isInteractable(), isInteractable());
		feedbackStateChanged();
		repaint();
	}
	break;

	default:
		//NOT HANDLED
		break;

	}
}

void ControllableUI::controllableStateChanged()
{
	updateUIParams();
}

void ControllableUI::feedbackStateChanged()
{
	updateUIParams();
}

void ControllableUI::updateTooltip()
{
	if (controllable == nullptr || controllable.wasObjectDeleted()) return;

	if (customDescription.isNotEmpty())
	{
		tooltip = customDescription;
	}
	else
	{
		tooltip = controllable->description + "\nControl Address : " + controllable->controlAddress;
		if (controllable->type != Controllable::Type::TRIGGER) tooltip += " (" + controllable->argumentsDescription + ")";
		if (controllable->isControllableFeedbackOnly) tooltip += " (read only)";
	}
	
	setTooltip(tooltip);
}
