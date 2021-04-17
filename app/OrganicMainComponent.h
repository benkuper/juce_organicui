#pragma once

class OrganicMainContentComponent   : 
	public Component, 
	public ApplicationCommandTarget, 
	public MenuBarModel, 
	public EngineListener
{
public:
   	OrganicMainContentComponent();
    virtual ~OrganicMainContentComponent();

	TooltipWindow tooltipWindow; //just declare one here

#if JUCE_OPENGL
	std::unique_ptr<OpenGLContext> openGLContext;
#endif

	std::unique_ptr<LookAndFeelOO> lookAndFeelOO;
	std::unique_ptr<ProgressWindow> fileProgressWindow;
	
	virtual void init();
	void setupOpenGL();
	virtual void setupOpenGLInternal() {}
	virtual void clear();

    virtual void paint (Graphics&) override;
    virtual void resized() override;


	//engine
	virtual void startLoadFile() override;
	virtual void fileProgress(float percent, int state) override;
	virtual void endLoadFile() override;


	//Override panel create functions
	virtual Outliner* createOutliner(const String& contentName) { return new Outliner(contentName); }

	// inherited from MenuBarModel , ApplicationCommandTarget
	ApplicationCommandTarget* getNextCommandTarget() override { return findFirstTargetParentComponent(); }
	virtual void getAllCommands(Array<CommandID>& commands) override;
	virtual void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
	virtual bool perform(const InvocationInfo& info) override;
	virtual StringArray getMenuBarNames() override;
	virtual PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName) override;
	virtual void fillFileMenuInternal(PopupMenu &p) {}
	virtual void menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/) override;


private:
    //==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrganicMainContentComponent)

};
