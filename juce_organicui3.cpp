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

#include "controllable/detective/ControllableDetectiveWatcher.cpp"
#include "controllable/parameter/detective/ParameterDetectiveWatcher.cpp"
#include "controllable/detective/Detective.cpp"
#include "controllable/detective/ui/ControllableDetectiveWatcherUI.cpp"
#include "controllable/parameter/detective/ui/ParameterDetectiveWatcherUI.cpp"
#include "controllable/detective/ui/DetectivePanel.cpp"

#include "help/HelpBox.cpp"

#include "inspectable/ui/InspectableContentComponent.cpp"
#include "inspectable/ui/InspectableEditor.cpp"
#include "inspectable/ui/InspectableSelector.cpp"
#include "inspectable/ui/Inspector.cpp"

#include "logger/CustomLogger.cpp"
#include "logger/ui/CustomLoggerUI.cpp"

#include "manager/Factory.h"
#include "manager/BaseItem.cpp"
#include "manager/ItemGroup.cpp"

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
#include "script/ui/ScriptManagerEditor.cpp"

#include "updater/AppUpdater.cpp"
#include "app/CrashHandler.cpp"
#include "app/OrganicMainComponent.cpp"
#include "app/OrganicMainComponentCommands.cpp"
#include "app/OrganicApplication.cpp"