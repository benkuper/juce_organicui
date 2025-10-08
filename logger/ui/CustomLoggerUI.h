/*
 ==============================================================================

 Copyright © Organic Orchestra, 2017

 This file is part of LGML. LGML is a software to manipulate sound in real-time

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation (version 3 of the License).

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 ==============================================================================
 */

#pragma once

// no true performance gain proved atm
#define USE_CACHED_GLYPH 0

#define LOGGER_USE_LABEL 0

class CustomLoggerUI : public ShapeShifterContentComponent,
	public CustomLogger::LoggerListener,
	public juce::TextButton::Listener,
	public juce::Timer
{
public:
	CustomLoggerUI(const juce::String& contentName, CustomLogger * l);
	~CustomLoggerUI();

	class LogList : public juce::TableListBoxModel
	{
	public:
		LogList(CustomLoggerUI* o);
		virtual ~LogList() {};
		int getNumRows() override;

		void paintRowBackground(juce::Graphics&,
			int rowNumber,
			int width, int height,
			bool rowIsSelected) override;
#if LOGGER_USE_LABEL
		Component * refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
			Component* existingComponentToUpdate) override;
#endif
		void paintCell(juce::Graphics&,
			int rowNumber,
			int columnId,
			int width, int height,
			bool rowIsSelected) override;

		juce::String getTextAt(int rowNumber, int columnId);

		juce::String getCellTooltip(int /*rowNumber*/, int /*columnId*/)    override;

	private:
#if USE_CACHED_GLYPH
		juce::HashMap<juce::String, juce::LowLevelGraphicsSoftwareRenderer::CachedGlyph > cachedG;
		void cleanUnusedGlyphs();
#endif


		int minRow, maxRow;
		CustomLoggerUI* owner;
		friend class CustomLoggerUI;
	};
	
	CustomLogger* logger;

	void resized()override;
	LogList logList;
	juce::TextButton clearB, copyB, autoScrollB;
	std::unique_ptr<juce::TableListBox> logListComponent;
	juce::OwnedArray<CustomLogger::LogEvent, juce::CriticalSection> logEvents;
	int maxNumElement;

	void buttonClicked(juce::Button*) override;

	void newMessage(const CustomLogger::LogEvent&) override;

	void clearLogger();

	static CustomLoggerUI * create(const juce::String &contentName) { return new CustomLoggerUI(contentName, CustomLogger::getInstance()); }

private:
	juce::MouseCursor  getMouseCursor() override;
	void mouseDown(const juce::MouseEvent&) override;
	void mouseDrag(const juce::MouseEvent&) override;

	const juce::Colour logNoneColor = TEXTNAME_COLOR;
	const juce::Colour logDbgColor = BLUE_COLOR.withSaturation(.2f).darker(.3f);

	juce::Atomic<int> totalLogRow;
	void updateTotalLogRow();
	const CustomLogger::LogEvent* getElementForRow(const int r) const;
	const juce::String& getSourceForRow(const int r) const;
	const bool isPrimaryRow(const int r) const;
	const juce::String& getContentForRow(const int r) const;
	const juce::Colour& getSeverityColourForRow(const int r)const;
	const juce::String getTimeStringForRow(const int r) const;
	friend class LogList;

	juce::int64 lastUpdateTime;
	void timerCallback()override;
	   
};