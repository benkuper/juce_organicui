#include "JuceHeader.h"
#include "OrganicMainComponent.h"

ApplicationProperties& getAppProperties();
ApplicationCommandManager& getCommandManager();
OrganicApplication::MainWindow* getMainWindow();

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

#if ORGANICUI_USE_SHAREDTEXTURE
	SharedTextureManager::getInstance(); //create the main instance
#endif

}

OrganicMainContentComponent::~OrganicMainContentComponent()
{
#if JUCE_MAC
	setMacMainMenu(nullptr, nullptr, "");
#endif

	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);

	//ShapeShifterManager::getInstance()->toggleTemporaryFullContent(nullptr);
	if(ShapeShifterManager::getInstanceWithoutCreating() != nullptr) ShapeShifterManager::deleteInstance();
	ParameterUITimers::deleteInstance();

#if ORGANICUI_USE_SHAREDTEXTURE
	SharedTextureManager::deleteInstance();
#endif
}

void OrganicMainContentComponent::init()
{
	setupOpenGL();

	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Inspector", &InspectorUI::create));

	std::function<ShapeShifterContent* (const String&)> outlinerFunc = std::bind(&OrganicMainContentComponent::createOutliner, this, std::placeholders::_1);
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Outliner", outlinerFunc));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Dashboard", &DashboardManagerView::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Logger", &CustomLoggerUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Parrots", &ParrotManagerUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("The Detective", &DetectivePanel::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Warnings", &WarningReporterPanel::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Help", &HelpPanel::create));


	addAndMakeVisible(&ShapeShifterManager::getInstance()->mainContainer);


}

void OrganicMainContentComponent::afterInit()
{
	if (ShapeShifterManager::getInstance()->openedPanels.isEmpty())
	{

		String lastVersion = getAppProperties().getUserSettings()->getValue("lastVersion", "0");
		if (lastVersion != getAppVersion())
		{
			ShapeShifterManager::getInstance()->loadDefaultLayoutFile(true);
		}
		else
		{
			ShapeShifterManager::getInstance()->loadLastSessionLayoutFile();
		}
	}


	if (isShowing()) grabKeyboardFocus();
}

void OrganicMainContentComponent::setupOpenGL()
{
#if JUCE_OPENGL
if (GlobalSettings::getInstance()->useGLRenderer->boolValue())
{
	if (openGLContext == nullptr)
	{
		openGLContext.reset(new OpenGLContext());
		openGLContext->setComponentPaintingEnabled(true);
		openGLContext->setContinuousRepainting(false);

		setupOpenGLInternal();

#if ORGANICUI_USE_SHAREDTEXTURE
		openGLContext->setRenderer(this);
#endif

		openGLContext->attachTo(*this);
	}
}
#endif
}

void OrganicMainContentComponent::clear()
{
#if JUCE_OPENGL
	if (openGLContext)
	{
		openGLContext->detach();
		openGLContext->setRenderer(nullptr);
		openGLContext.reset();
	}
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

#if ORGANICUI_USE_SHAREDTEXTURE
void OrganicMainContentComponent::newOpenGLContextCreated()
{
	glDisable(GL_DEBUG_OUTPUT);
	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->initGL();
}

void OrganicMainContentComponent::renderOpenGL()
{
	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->renderGL();
}

void OrganicMainContentComponent::openGLContextClosing()
{
	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->clearGL();
}
#endif


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
	}
	else
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

	Component* fullScreenComponent = getMainWindow();

	if (ProjectSettings::getInstance()->showDashboardOnStartup->enabled)
	{
		Dashboard* d = dynamic_cast<Dashboard*>(ProjectSettings::getInstance()->showDashboardOnStartup->targetContainer.get());

		if (d != nullptr)
		{
			/*
			fullScreenComponent = new DashboardItemManagerUI(&d->itemManager);
			fullScreenComponent->setVisible(true);
			fullScreenComponent->toFront(true);
			fullScreenComponent->addToDesktop(0);
			*/

			ShapeShifterContent* dContent = ShapeShifterManager::getInstance()->getContentForType<DashboardManagerView>();

			if (ProjectSettings::getInstance()->fullScreenOnStartup->boolValue() && dContent != nullptr && dContent != ShapeShifterManager::getInstance()->temporaryFullContent)
			{
				ShapeShifterManager::getInstance()->toggleTemporaryFullContent(dContent); //only if fullScreen, otherwise it's weird
			}

			d->selectThis();

		}
		else
		{
			ShapeShifterManager::getInstance()->toggleTemporaryFullContent(nullptr);
		}
	}

	Desktop::getInstance().setKioskModeComponent(ProjectSettings::getInstance()->fullScreenOnStartup->boolValue() ? fullScreenComponent : nullptr);

#if JUCE_MAC
	MenuBarModel::handleAsyncUpdate(); //force refresh menu
#endif
}
