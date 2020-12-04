WarningTargetUI::WarningTargetUI(WarningTarget * target) :
	target(target)
{
	target->addAsyncWarningTargetListener(this);
	
	String s = target->getWarningMessage();
	setVisible(s.isNotEmpty());
	setTooltip(s);
}

WarningTargetUI::~WarningTargetUI()
{
	if(!target.wasObjectDeleted()) target->removeAsyncWarningTargetListener(this);
}


void WarningTargetUI::paint(Graphics& g)
{
	if (target.wasObjectDeleted()) return;
	g.drawImage(AssetManager::getInstance()->warningImage, getLocalBounds().toFloat());
}

void WarningTargetUI::newMessage(const WarningTarget::WarningTargetEvent& e)
{
	if (target.wasObjectDeleted()) return;

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
