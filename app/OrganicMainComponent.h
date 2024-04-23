#pragma once

class OrganicMainContentComponent   : 
	public juce::Component, 
	public juce::ApplicationCommandTarget, 
	public juce::MenuBarModel, 
	public EngineListener
#if ORGANICUI_USE_SHAREDTEXTURE
	,public juce::OpenGLRenderer
#endif
{
public:
   	OrganicMainContentComponent();
    virtual ~OrganicMainContentComponent();

	juce::TooltipWindow tooltipWindow; //just declare one here

#if JUCE_OPENGL
	std::unique_ptr<juce::OpenGLContext> openGLContext;
#endif

	std::unique_ptr<LookAndFeelOO> lookAndFeelOO;
	std::unique_ptr<ProgressWindow> fileProgressWindow;
	
	virtual void init();
	virtual void afterInit();
	virtual void setupOpenGL();
	virtual void setupOpenGLInternal() {}
	virtual void clear();

    virtual void paint (juce::Graphics&) override;
    virtual void resized() override;

#if ORGANICUI_USE_SHAREDTEXTURE
	virtual void newOpenGLContextCreated() override;
	virtual void renderOpenGL() override;
	virtual void openGLContextClosing() override;
#endif

	//engine
	virtual void startLoadFile() override;
	virtual void fileProgress(float percent, int state) override;
	virtual void endLoadFile() override;


	//Override panel create functions
	virtual Outliner* createOutliner(const juce::String& contentName) { return new Outliner(contentName); }

	// inherited from MenuBarModel , ApplicationCommandTarget
	ApplicationCommandTarget* getNextCommandTarget() override { return findFirstTargetParentComponent(); }
	virtual void getAllCommands(juce::Array<juce::CommandID>& commands) override;
	virtual void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
	virtual bool perform(const InvocationInfo& info) override;
	virtual juce::StringArray getMenuBarNames() override;
	virtual juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
	virtual void fillFileMenuInternal(juce::PopupMenu &p) {}
	virtual void menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/) override;


private:
    //==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrganicMainContentComponent)

};
