#pragma once

#include "../WarningReporter.h"

class WarningReporterItem :
	public juce::Component,
	public juce::Button::Listener
{
public:
	WarningReporterItem(juce::WeakReference<WarningTarget> target);
	~WarningReporterItem();

	juce::Label itemLabel;
	juce::Label warningLabel;
	juce::TextButton resolveBT;

	juce::WeakReference<WarningTarget> target;

	void resized() override;
	virtual void buttonClicked(juce::Button*) override;
};

class WarningReporterContainer :
	public juce::Component,
	public WarningReporter::AsyncListener
{
public:
	WarningReporterContainer();
	~WarningReporterContainer();

	juce::OwnedArray<WarningReporterItem> items;

	const int lineHeight = 20;

	void resized() override;
	int getContentHeight();

	void addTarget(juce::WeakReference<WarningTarget> t);
	void removeTarget(juce::WeakReference<WarningTarget> t);

	WarningReporterItem * getUIForTarget(juce::WeakReference<WarningTarget> t);

	void newMessage(const WarningReporter::WarningReporterEvent& e) override;

};

class WarningReporterPanel :
	public ShapeShifterContentComponent
{
public:
	WarningReporterPanel(juce::StringRef name);
	~WarningReporterPanel();

	juce::Viewport viewport;
	WarningReporterContainer container;

	void resized() override;

	static WarningReporterPanel * create(const juce::String& name) { return new WarningReporterPanel(name); }
};

