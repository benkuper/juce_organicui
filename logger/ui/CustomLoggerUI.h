/*
  ==============================================================================

    CustomLoggerUI.h
    Created: 6 May 2016 2:13:35pm
    Author:  Martin Hermant

  ==============================================================================
*/

#ifndef CustomLoggerUI_H_INCLUDED
#define CustomLoggerUI_H_INCLUDED

class CustomLoggerUI : 
	public ShapeShifterContentComponent,
	public CustomLogger::Listener,
	public TextButton::Listener,
	public AsyncUpdater
{
public:
	CustomLoggerUI(const String& contentName, CustomLogger* l);
	~CustomLoggerUI();

	class LogList : public juce::TableListBoxModel
	{
	public:
		LogList(CustomLoggerUI* o);

		int getNumRows() override;

		void paintRowBackground(Graphics&,
			int rowNumber,
			int width, int height,
			bool rowIsSelected) override;

		void paintCell(Graphics&,
			int rowNumber,
			int columnId,
			int width, int height,
			bool rowIsSelected) override;
		String getCellTooltip(int /*rowNumber*/, int /*columnId*/)    override;


		

	private:
		CustomLoggerUI* owner;
	};

	CustomLogger* logger;

	void resized()override;
	LogList logList;
	TextButton clearB, copyB;
	ScopedPointer<TableListBox> logListComponent;
	int maxNumElement;

	void buttonClicked(Button*) override;


	OwnedArray<LogElement> logElements;
	void newMessage(const String&) override;

	static CustomLoggerUI * create(const String &contentName) { return new CustomLoggerUI(contentName, CustomLogger::getInstance()); }

private: 
	int totalLogRow;
	void updateTotalLogRow();
	const LogElement * getElementForRow(const int r) const;
	const String& getSourceForRow(const int r) const;
	const bool isPrimaryRow(const int r) const;
	const String& getContentForRow(const int r) const;
	const Colour getSeverityColourForRow(const int r)const;
	const String getTimeStringForRow(const int r) const;
	friend class LogList;

	void handleAsyncUpdate()override;

};



#endif  // CustomLoggerUI_H_INCLUDED
