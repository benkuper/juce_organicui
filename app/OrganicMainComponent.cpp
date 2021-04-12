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

}

OrganicMainContentComponent::~OrganicMainContentComponent()
{
#if JUCE_MAC
	setMacMainMenu(nullptr, nullptr, "");
#endif

	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);

	//ShapeShifterManager::getInstance()->toggleTemporaryFullContent(nullptr);
	ShapeShifterManager::deleteInstance();
}

void OrganicMainContentComponent::init()
{
	setupOpenGL();

	String lastVersion = getAppProperties().getUserSettings()->getValue("lastVersion", "0");
	if (lastVersion != getAppVersion())
	{
		ShapeShifterManager::getInstance()->loadDefaultLayoutFile(true);
	} else
	{
		ShapeShifterManager::getInstance()->loadLastSessionLayoutFile();
	}


	addAndMakeVisible(&ShapeShifterManager::getInstance()->mainContainer);
	if(isShowing()) grabKeyboardFocus();
}

void OrganicMainContentComponent::setupOpenGL()
{
#if JUCE_OPENGL && JUCE_WINDOWS
	if (openGLContext == nullptr)
	{
		openGLContext.reset(new OpenGLContext());
		openGLContext->setComponentPaintingEnabled(true);
		openGLContext->setContinuousRepainting(false);

		setupOpenGLInternal();

		openGLContext->attachTo(*this);
	}
#endif
}

void OrganicMainContentComponent::clear()
{

#if JUCE_OPENGL && JUCE_WINDOWS
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
