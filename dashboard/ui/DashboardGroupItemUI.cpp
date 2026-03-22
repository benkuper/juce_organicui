DashboardGroupItemUI::DashboardGroupItemUI(DashboardGroupItem* group) :
	DashboardItemUI(group),
	group(group)
{
	setName(group->niceName);
	managerUI.reset(new DashboardItemManagerUI(&group->itemManager));
	managerUI->canNavigate = false;
	managerUI->canZoom = false;
	managerUI->transparentBG = true;
	managerUI->setDisableDefaultMouseEvents(true);
	managerUI->autoSelectWithChildRespect = DashboardManager::getInstance()->editMode->boolValue();
	addAndMakeVisible(managerUI.get(), 0);
}

DashboardGroupItemUI::~DashboardGroupItemUI()
{
}

void DashboardGroupItemUI::paint(Graphics& g)
{
	if (group->backgroundColor->enabled)
	{
		Colour bc = group->backgroundColor->getColor();
		g.setColour(bc);
		g.fillAll(bc);
	}

	DashboardItemUI::repaint();

	float borderWidth = group->borderWidth->floatValue();
	Colour borderC = group->borderColor->getColor();
	if (borderWidth > 0 && borderC.getAlpha() > 0)
	{
		g.setColour(borderC);
		g.drawRect(getLocalBounds(), borderWidth);
	}
}

void DashboardGroupItemUI::resizedDashboardItemInternal()
{
	juce::Rectangle<int> r = getLocalBounds().reduced(4);

	managerUI->manager->viewOffset = -r.getCentre(); //top left anchor
	managerUI->setBounds(r);
	
	group->itemManager.canvasSize->setPoint(r.getWidth(), r.getHeight());
}

void DashboardGroupItemUI::updateEditModeInternal(bool editMode)
{
	managerUI->autoSelectWithChildRespect = editMode;
}

void DashboardGroupItemUI::mouseDown(const MouseEvent& e)
{
	if (e.eventComponent == managerUI.get())
	{
		if(e.mods.isRightButtonDown() || e.mods.isShiftDown()) managerUI->mouseDown(e); //transfer right mouse click and shift for selection
		else
		{
			managerUI->autoFilterHitTestOnItems = true;
			managerUI->validateHitTestOnNoItem = false;
			
			if(DashboardManager::getInstance()->editMode->boolValue()) item->selectThis();

			DashboardItemUI::mouseDown(e);
		}
	}
	
	DashboardItemUI::mouseDown(e);
}

void DashboardGroupItemUI::mouseDrag(const MouseEvent& e)
{
	if(e.eventComponent == managerUI.get()) DashboardItemUI::mouseDrag(e.getEventRelativeTo(this));
	else DashboardItemUI::mouseDrag(e);
}

void DashboardGroupItemUI::mouseUp(const MouseEvent& e)
{
	DashboardItemUI::mouseUp(e);

	managerUI->autoFilterHitTestOnItems = false;
	managerUI->validateHitTestOnNoItem = true;
}

void DashboardGroupItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	DashboardItemUI::controllableFeedbackUpdateInternal(c);
	if (c == group->backgroundColor || c == group->borderWidth || c == group->borderColor) repaint();
}

bool DashboardGroupItemUI::canStartDrag(const MouseEvent& e)
{
	return DashboardItemUI::canStartDrag(e) || e.eventComponent == managerUI.get();
}
