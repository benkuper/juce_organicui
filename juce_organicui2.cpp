/*
  ==============================================================================

   

  ==============================================================================
*/

#include "juce_organicui.h"

//==============================================================================



#include "engine/Engine.cpp"
#include "engine/EngineFileDocument.cpp"

#include "settings/GlobalSettings.cpp"
#include "settings/ProjectSettings.cpp"
#include "settings/ui/GlobalSettingsEditor.cpp"

#include "inspectable/Inspectable.cpp"
#include "inspectable/InspectableContent.cpp"
#include "inspectable/InspectableSelectionManager.cpp"

#include "help/HelpBox.cpp"

#include "inspectable/ui/InspectableContentComponent.cpp"
#include "inspectable/ui/InspectableEditor.cpp"
#include "inspectable/ui/InspectableSelector.cpp"
#include "inspectable/ui/Inspector.cpp"

#include "logger/CustomLogger.cpp"
#include "logger/ui/CustomLoggerUI.cpp"

#include "manager/Factory.h"
#include "manager/BaseItem.cpp"
#include "manager/GenericControllableManager.cpp"
#include "manager/GenericControllableItem.cpp"
#include "manager/ui/GenericControllableItemEditor.cpp" 
#include "manager/ui/BaseItemEditor.cpp"

#include "outliner/Outliner.cpp"

#include "progress/ProgressNotifier.cpp"
#include "progress/ui/ProgressWindow.cpp"

#include "script/ScriptTarget.cpp"
#include "script/Script.cpp"
#include "script/ScriptManager.cpp"
#include "script/ScriptUtil.cpp"
#include "script/ui/ScriptEditor.cpp"

#include "ui/AssetManager.cpp"
#include "ui/LookAndFeelOO.cpp"
#include "ui/Grabber.cpp"

#include "ui/shapeshifter/GapGrabber.cpp"
#include "ui/shapeshifter/ShapeShifter.cpp"
#include "ui/shapeshifter/ShapeShifterContainer.cpp"
#include "ui/shapeshifter/ShapeShifterContent.cpp"
#include "ui/shapeshifter/ShapeShifterFactory.cpp"
#include "ui/shapeshifter/ShapeShifterManager.cpp"
#include "ui/shapeshifter/ShapeShifterPanel.cpp"
#include "ui/shapeshifter/ShapeShifterPanelHeader.cpp"
#include "ui/shapeshifter/ShapeShifterPanelTab.cpp"
#include "ui/shapeshifter/ShapeShifterWindow.cpp"

#include "help/HelpPanel.cpp"
#include "undo/UndoMaster.cpp"
#include "updater/AppUpdater.cpp"


#include "app/CrashHandler.cpp"
#include "app/OrganicMainComponent.cpp"
#include "app/OrganicMainComponentCommands.cpp"
#include "app/OrganicApplication.cpp"

#include "remotecontrol/OSCRemoteControl.cpp"

#include "warning/WarningTarget.cpp"
#include "warning/WarningReporter.cpp"
#include "warning/ui/WarningTargetUI.cpp"
#include "warning/ui/WarningReporterPanel.cpp"