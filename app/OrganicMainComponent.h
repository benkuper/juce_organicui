/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef ORGANICMAINCOMPONENT_H_INCLUDED
#define ORGANICMAINCOMPONENT_H_INCLUDED


ApplicationProperties& getAppProperties();
ApplicationCommandManager& getCommandManager();

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class OrganicMainContentComponent   : public Component, public ApplicationCommandTarget, public MenuBarModel, public EngineListener
{
public:
    //==============================================================================
   	OrganicMainContentComponent();
    virtual ~OrganicMainContentComponent();

	virtual void init();
	virtual void clear() {}

	TooltipWindow tooltipWindow; // to add tooltips to an application, you
								 // just need to create one of these and leave it
								 // there to do its work..

	ScopedPointer<LookAndFeelOO> lookAndFeelOO;
	ScopedPointer<ProgressWindow> fileProgressWindow;


    virtual void paint (Graphics&) override;
    virtual void resized() override;


	//engine
	virtual void startLoadFile() override;
	virtual void fileProgress(float percent, int state) override;
	virtual void endLoadFile() override;

	// inherited from MenuBarModel , ApplicationCommandTarget
	ApplicationCommandTarget* getNextCommandTarget() override { return findFirstTargetParentComponent(); }
	virtual void getAllCommands(Array<CommandID>& commands) override;
	virtual void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
	virtual bool perform(const InvocationInfo& info) override;
	virtual StringArray getMenuBarNames() override;
	virtual PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName) override;
	virtual void menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/) override;

private:
    //==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrganicMainContentComponent)

};



#endif  // ORGANICMAINCOMPONENT_H_INCLUDED
