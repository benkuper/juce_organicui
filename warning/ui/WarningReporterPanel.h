#pragma once

#include "../WarningReporter.h"

class WarningReporterItem :
	public Component,
	public Button::Listener
{
public:
	WarningReporterItem(WarningTarget* target);
	~WarningReporterItem();

	Label itemLabel;
	Label warningLabel;
	TextButton resolveBT;

	WarningTarget* target;

	void resized() override;
	virtual void buttonClicked(Button*) override;
};

class WarningReporterContainer :
	public Component,
	public WarningReporter::AsyncListener
{
public:
	WarningReporterContainer();
	~WarningReporterContainer();

	OwnedArray<WarningReporterItem> items;

	const int lineHeight = 20;

	void resized() override;
	int getContentHeight();

	void addTarget(WarningTarget* t);
	void removeTarget(WarningTarget* t);

	WarningReporterItem * getUIForTarget(WarningTarget* t);

	void newMessage(const WarningReporter::WarningReporterEvent& e) override;

};

class WarningReporterPanel :
	public ShapeShifterContentComponent
{
public:
	WarningReporterPanel(StringRef name);
	~WarningReporterPanel();

	Viewport viewport;
	WarningReporterContainer container;

	void resized() override;

	static WarningReporterPanel * create(const String& name) { return new WarningReporterPanel(name); }
};

