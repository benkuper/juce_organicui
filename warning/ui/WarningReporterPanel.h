#pragma once

#include "../WarningReporter.h"

class WarningReporterItem :
	public Component,
	public Button::Listener
{
public:
	WarningReporterItem(WeakReference<WarningTarget> target);
	~WarningReporterItem();

	Label itemLabel;
	Label warningLabel;
	TextButton resolveBT;

	WeakReference<WarningTarget> target;

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

	void addTarget(WeakReference<WarningTarget> t);
	void removeTarget(WeakReference<WarningTarget> t);

	WarningReporterItem * getUIForTarget(WeakReference<WarningTarget> t);

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

