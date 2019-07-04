WarningTargetUI::WarningTargetUI(WarningTarget* target) :
	target(target)
{
	String s = target->getWarningMessage();

	target->addAsyncWarningTargetListener(this);

	setVisible(s.isNotEmpty());
	setTooltip(s);
}

WarningTargetUI::~WarningTargetUI()
{
	target->removeAsyncWarningTargetListener(this);
}


void WarningTargetUI::paint(Graphics& g)
{
	g.drawImage(AssetManager::getInstance()->warningImage, getLocalBounds().toFloat());
}

void WarningTargetUI::newMessage(const WarningTarget::WarningTargetEvent& e)
{
	switch (e.type)
	{
	case WarningTarget::WarningTargetEvent::WARNING_CHANGED:
	{
		if (target == nullptr) return;
		String s = target->getWarningMessage();
		setVisible(s.isNotEmpty());
		setTooltip(s);
	}
	break;
	}
}
