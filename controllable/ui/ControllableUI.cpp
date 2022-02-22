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
bool ControllableUI::showParrotOption = true;
bool ControllableUI::drawContourOnInspectableHighlighted = false;

std::function<void(ControllableUI*)> ControllableUI::customShowContextMenuFunc = nullptr;
std::function<void(ControllableUI*, PopupMenu*)> ControllableUI::customAddToContextMenuFunc = nullptr;
std::function<bool(ControllableUI*, int)> ControllableUI::handleCustomContextMenuResultFunc = nullptr;
std::function<void(ControllableUI*)> ControllableUI::customShowEditWindowFunction = nullptr;

ControllableUI::ControllableUI(Array<Controllable*> controllables) :
	Component(controllables.size() > 0 ? controllables[0]->niceName : "[notset]"),
	controllables(Inspectable::getWeakArray(controllables)),
	controllable(controllables.size() > 0 ? controllables[0] : nullptr),
	showLabel(true),
	opaqueBackground(false),
	showMenuOnRightClick(true),
	forceFeedbackOnly(false),
	useCustomTextColor(false),
	customContourThickness(1)
{
	jassert(controllable != nullptr);
	updateTooltip();
	controllable->addAsyncControllableListener(this);
	if (drawContourOnInspectableHighlighted) controllable->addAsyncInspectableListener(this);

	setEnabled(controllable->enabled);
	setAlpha(controllable->enabled ? 1 : .5f);

	updateUIParams();
}

ControllableUI::~ControllableUI()
{
	if (controllable != nullptr && !controllable.wasObjectDeleted())
	{
		controllable->removeAsyncControllableListener(this);
		controllable->removeAsyncInspectableListener(this);
	}
}


void ControllableUI::paintOverChildren(Graphics& g)
{
	drawContour(g);
}

void ControllableUI::mouseEnter(const MouseEvent& e)
{
	if (controllable != nullptr && !controllable.wasObjectDeleted())
	{
		HelpBox::getInstance()->setOverData(controllable->helpID);
		if (drawContourOnInspectableHighlighted) controllable->highlightLinkedInspectables(true);
	}
}

void ControllableUI::mouseExit(const MouseEvent& e)
{
	if (controllable != nullptr && !controllable.wasObjectDeleted())
	{
		HelpBox::getInstance()->clearOverData(controllable->helpID);
		if (drawContourOnInspectableHighlighted) controllable->highlightLinkedInspectables(false);
		setTooltip(tooltip); //restore tooltip when leaving
	}
}

void ControllableUI::mouseDown(const MouseEvent& e)
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

void ControllableUI::mouseUp(const MouseEvent& e)
{
	if (e.mods.isRightButtonDown()) return;
	if (isInteractable()) mouseUpInternal(e);
}

void ControllableUI::drawContour(Graphics& g)
{
	if (controllable == nullptr || controllable.wasObjectDeleted()) return;
	bool isHighlighted = controllable->isHighlighted && ControllableUI::drawContourOnInspectableHighlighted;
	if (!(useCustomContour || isHighlighted)) return;
	g.setColour(isHighlighted ? Colours::purple : customContourColor);
	g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 2, customContourThickness);
}

bool ControllableUI::isInteractable(bool falseIfFeedbackOnly)
{
	if (controllable == nullptr || controllable.wasObjectDeleted()) return false;
	if (!controllable->enabled) return false;
	if (forceFeedbackOnly) return false;
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

	if (controllable->canBeDisabledByUser)
	{
		p->addSeparator();
		p->addItem(-12, "Enabled", true, controllable->enabled);
	}

	if (controllable->includeInScriptObject)
	{
		p->addSeparator();
		if (showOSCControlAddressOption) p->addItem(-1, "Copy OSC Control Address");
		if (showScriptControlAddressOption) p->addItem(-2, "Copy Script Control Address");
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

	if (showParrotOption)
	{
		PopupMenu parrotMenu;
		int index = 0;
		for (auto& pa : ParrotManager::getInstance()->items)
		{
			bool isInParrot = pa->getTargetForControllable(controllable) != nullptr;
			parrotMenu.addItem(index + 20000, pa->niceName, true, isInParrot);
			index++;
		}

		if (index > 0) parrotMenu.addSeparator();
		parrotMenu.addItem(19999, "Create new Parrot");

		p->addSubMenu("Set in Parrot", parrotMenu);
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

		if (index > 0) dashboardMenu.addSeparator();
		dashboardMenu.addItem(9999, "Create new Dashboard");

		p->addSubMenu("Send to Dashboard", dashboardMenu);
	}


	if (p->getNumItems() == 0) return;

	p->showMenuAsync(PopupMenu::Options(), [this](int result)
		{
			if (result == 0) return;

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
			{
				bool tVal = !controllable->isControllableFeedbackOnly;
				for (auto& c : controllables) c->setControllableFeedbackOnly(tVal);
			}
			break;

			case -12:
			{

				bool tVal = !controllable->enabled;
				for (auto& c : controllables) c->setEnabled(tVal);
			}
			break;

			case 9999:
			{
				Dashboard* d = new Dashboard();
				d->itemManager.addItem(controllable->createDashboardItem());
				DashboardManager::getInstance()->addItem(d);
			}
			break;

			case 19999:
			{
				Parrot* pa = new Parrot();
				pa->addTarget(controllable);
				ParrotManager::getInstance()->addItem(pa);
			}
			break;

			default:
				if (result >= 10000 && result <= 10100)
				{
					DashboardManager::getInstance()->items[result - 10000]->itemManager.addItem(controllable->createDashboardItem());
				}
				else if (result >= 20000 && result <= 20100)
				{
					Parrot* pa = ParrotManager::getInstance()->items[result - 20000];
					bool isInParrot = pa->getTargetForControllable(controllable) != nullptr;
					if (isInParrot) pa->removeTargetForControllable(controllable);
					else pa->addTarget(controllable);
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
	);
}

void ControllableUI::updateUIParams()
{
	setInterceptsMouseClicks(isInteractable(false), isInteractable(false));
	updateUIParamsInternal();
	resized();
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

void ControllableUI::newMessage(const Controllable::ControllableEvent& e)
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

void ControllableUI::newMessage(const Inspectable::InspectableEvent& e)
{
	if (e.type == Inspectable::InspectableEvent::HIGHLIGHT_CHANGED)
	{
		if (drawContourOnInspectableHighlighted) repaint();
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
