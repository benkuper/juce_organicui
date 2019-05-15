#include "OrganicMainComponent.h"

ApplicationProperties& getAppProperties();
ApplicationCommandManager& getCommandManager();

OrganicMainContentComponent::OrganicMainContentComponent()
{
	setSize(800, 600);

	Engine::mainEngine->addEngineListener(this);
	
	lookAndFeelOO.reset(new LookAndFeelOO());
	LookAndFeel::setDefaultLookAndFeel(lookAndFeelOO.get());
	
	(&getCommandManager())->registerAllCommandsForTarget(this);
	(&getCommandManager())->setFirstCommandTarget(this);
	
	(&getCommandManager())->getKeyMappings()->resetToDefaultMappings();
	addKeyListener((&getCommandManager())->getKeyMappings());
	

#if JUCE_MAC
	setMacMainMenu(this, nullptr, "");
#else
	//done in Main.cpp as it's a method of DocumentWindow
#endif



#if JUCE_OPENGL && JUCE_WINDOWS
	openGLContext.setComponentPaintingEnabled(true);
	openGLContext.setContinuousRepainting(false);
	openGLContext.attachTo(*this);
#endif

}

OrganicMainContentComponent::~OrganicMainContentComponent()
{
#if JUCE_MAC
	setMacMainMenu(nullptr, nullptr, "");
#endif

	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
	ShapeShifterManager::deleteInstance();
}

void OrganicMainContentComponent::init()
{
	String lastVersion = getAppProperties().getUserSettings()->getValue("lastVersion", "0");
	if (lastVersion != getAppVersion())
	{
		ShapeShifterManager::getInstance()->loadDefaultLayoutFile(true);
	} else
	{
		ShapeShifterManager::getInstance()->loadLastSessionLayoutFile();
	}


	addAndMakeVisible(&ShapeShifterManager::getInstance()->mainContainer);
	grabKeyboardFocus();
}

void OrganicMainContentComponent::clear()
{

#if JUCE_OPENGL && JUCE_WINDOWS
	openGLContext.detach();
	openGLContext.setRenderer(nullptr);
#endif

	tooltipWindow.setMillisecondsBeforeTipAppears(300);
}


void OrganicMainContentComponent::paint(Graphics& g)
{
	g.fillAll(BG_COLOR.darker());
}

void OrganicMainContentComponent::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	ShapeShifterManager::getInstance()->mainContainer.setBounds(r);
}


void OrganicMainContentComponent::startLoadFile()
{

	if (fileProgressWindow != nullptr)
	{
		removeChildComponent(fileProgressWindow.get());
		fileProgressWindow = nullptr;
	}

	fileProgressWindow.reset(new ProgressWindow("Loading File...", Engine::mainEngine));
	addAndMakeVisible(fileProgressWindow.get());
	fileProgressWindow->setSize(getWidth(), getHeight());
}

void OrganicMainContentComponent::fileProgress(float percent, int state)
{
	if (fileProgressWindow != nullptr)
	{
		fileProgressWindow->setProgress(percent);
	} else
	{
		DBG("Window is null but still got progress");
	}
}

void OrganicMainContentComponent::endLoadFile()
{
	if (fileProgressWindow != nullptr)
	{
		removeChildComponent(fileProgressWindow.get());
		fileProgressWindow = nullptr;
	}
}
