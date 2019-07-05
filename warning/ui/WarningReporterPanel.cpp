#include "WarningReporterPanel.h"

WarningReporterPanel::WarningReporterPanel(StringRef name) :
	ShapeShifterContentComponent(name)
{
	addAndMakeVisible(&viewport);
	viewport.setViewedComponent(&container);
}

WarningReporterPanel::~WarningReporterPanel()
{

}

void WarningReporterPanel::resized()
{
	container.setSize(getWidth(), container.getContentHeight());
	viewport.setBounds(getLocalBounds());
}


WarningReporterContainer::WarningReporterContainer()
{
	WarningReporter::getInstance()->addAsyncWarningReporterListener(this);

	for (auto& i : WarningReporter::getInstance()->targets)
	{
		addTarget(i);
	}

	resized();
}

WarningReporterContainer::~WarningReporterContainer()
{
	WarningReporter::getInstance()->removeAsyncWarningReporterListener(this);
}

void WarningReporterContainer::resized()
{
	juce::Rectangle<int> r = getLocalBounds().withHeight(lineHeight);

	for (auto& i : items)
	{
		i->setBounds(r.reduced(2));
		r.translate(0, lineHeight);
	}

	setSize(getWidth(), r.getY());
}

int WarningReporterContainer::getContentHeight()
{
	return items.size() * lineHeight;
}

void WarningReporterContainer::addTarget(WarningTarget* t)
{
	WarningReporterItem* item = getUIForTarget(t); 
	if (item != nullptr) return;
	item = new WarningReporterItem(t);
	items.add(item);
	addAndMakeVisible(item);
}

void WarningReporterContainer::removeTarget(WarningTarget* t)
{
	WarningReporterItem* item = getUIForTarget(t);
	if (item == nullptr) return;
	removeChildComponent(item);
	items.removeObject(item);
	DBG("Warning Reporter, num items now : " << items.size());
}

WarningReporterItem * WarningReporterContainer::getUIForTarget(WarningTarget* t)
{
	for (auto& i : items) if (i->target == t) return i;
	return nullptr;
}

void WarningReporterContainer::newMessage(const WarningReporter::WarningReporterEvent& e)
{
	switch (e.type)
	{
	case WarningReporter::WarningReporterEvent::WARNING_REGISTERED:
		addTarget(e.target);
		resized();
		break;

	case WarningReporter::WarningReporterEvent::WARNING_UNREGISTERED:
		removeTarget(e.target);
		resized();
		break;
	}
}

WarningReporterItem::WarningReporterItem(WarningTarget* target) :
	itemLabel("ItemLabel", target->getWarningTargetName()),
	warningLabel("WarningLabel", target->getWarningMessage()),
	resolveBT("Resolve"),
	target(target)
{
	itemLabel.setFont(12);
	warningLabel.setFont(12);

	addAndMakeVisible(&itemLabel);
	addAndMakeVisible(&warningLabel);
	addAndMakeVisible(&resolveBT);
	resolveBT.addListener(this);
}

WarningReporterItem::~WarningReporterItem()
{
}

void WarningReporterItem::resized()
{
	juce::Rectangle<int> r = getLocalBounds();

	itemLabel.setBounds(r.removeFromLeft(160));
	r.removeFromLeft(8);
	resolveBT.setBounds(r.removeFromRight(80));
	r.removeFromRight(8);
	warningLabel.setBounds(r);
}

void WarningReporterItem::buttonClicked(Button* b)
{
	if (b == &resolveBT) target->resolveWarning();
}
