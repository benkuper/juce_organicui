/*
  ==============================================================================

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_organicui
  vendor:           benkuper
  version:          5.3.2
  name:             Organic UI
  description:      A sub-framework that adds multiple functionalities such as moving panels, full controllable hierarchy, managers, editors...
  website:          https://github.com/benkuper/juce_organicui
  license:          GPLv3

  dependencies:     juce_core, juce_gui_basics, juce_gui_extra, juce_opengl, juce_osc

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define ORGANIC_UI_H_INCLUDED

//==============================================================================
#ifdef _MSC_VER
#pragma warning (push)
// Disable warnings for long class names, padding, and undefined preprocessor definitions.
#pragma warning (disable: 4251 4786 4668 4820)
#endif


/** Config: ORGANICUI_USE_SERVUS
	Enables the use of Zeroconf / DNS-SD / Bonjour for advertising the remote control OSC port and dashboard.

	You will need to link the Servus libraries in the exporters in order to compile with this option.
*/
#ifndef ORGANICUI_USE_SERVUS
#define ORGANICUI_USE_SERVUS 0
#endif

/** Config: ORGANICUI_USE_SHAREDTEXTURE
	Enables the use of Shared Texture. If enabled, you'll need the juce_sharedtexture module
	You will need to add the SpoutLibrary.dll alongside the windows .exe file
*/

#ifndef ORGANICUI_USE_SHAREDTEXTURE
#define ORGANICUI_USE_SHAREDTEXTURE 0
#endif

/** Config: ORGANICUI_USE_WEBSERVER
	Enables the use of the JUCE SimpleWeb module to expose the dashboard as a webserver
	You will need to add the juce_simpleweb in order to compile if you enable this option
*/
#ifndef ORGANICUI_USE_WEBSERVER
#define ORGANICUI_USE_WEBSERVER 0
#endif

/** Config: ORGANICUI_USE_DASHBOARDPANEL
	Enables the use of the DashboardPanel
*/
#ifndef ORGANICUI_USE_DASHBOARDPANEL
#define ORGANICUI_USE_DASHBOARDPANEL 1
#endif


/** Config: ORGANICUI_USE_PARROTPANEL
	Enables the use of the ParrotPanel
*/
#ifndef ORGANICUI_USE_PARROTPANEL
#define ORGANICUI_USE_PARROTPANEL 1
#endif

/** Config: ORGANICUI_USE_DETECTIVEPANEL
	Enables the use of the DetectivePanel
*/
#ifndef ORGANICUI_USE_DETECTIVEPANEL
#define ORGANICUI_USE_DETECTIVEPANEL 1
#endif

/** Config: ORGANICUI_USE_HELPPANEL
	Enables the use of the HelpPanel
*/
#ifndef ORGANICUI_USE_HELPPANEL
#define ORGANICUI_USE_HELPPANEL 1
#endif

/** Config: ORGANICUI_USE_LOGGERPANEL
	Enables the use of the LoggerReporterPanel
*/
#ifndef ORGANICUI_USE_LOGGERPANEL
#define ORGANICUI_USE_LOGGERPANEL 1
#endif

/** Config: ORGANICUI_USE_WARNINGPANEL
	Enables the use of the WarningReporterPanel
*/
#ifndef ORGANICUI_USE_WARNINGPANEL
#define ORGANICUI_USE_WARNINGPANEL 1
#endif


/** Config: ORGANICUI_USE_OUTLINERPANEL
* Enables the use of the Outliner
*/
#ifndef ORGANICUI_USE_OUTLINERPANEL
#define ORGANICUI_USE_OUTLINERPANEL 1
#endif


#include <juce_core/juce_core.h>
#include <juce_javascript/juce_javascript.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>
#include <juce_osc/juce_osc.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_javascript/juce_javascript.h>

#if ORGANICUI_USE_SERVUS
#include "servus/servus.h"
#endif

#if ORGANICUI_USE_WEBSERVER
#include <juce_simpleweb/juce_simpleweb.h>
#endif


#include "resources/BinaryData.h"


#include "ui/Style.h"
#include "ui/AssetManager.h"
#include "ui/LookAndFeelOO.h"
#include "ui/Grabber.h"
#include "ui/UITimers.h"

#include "helpers/RegexFunctions.h"
#include "engine/EngineListener.h"

#include "helpers/DebugHelpers.h"
#include "helpers/QueuedNotifier.h"
#include "helpers/StringUtil.h"
#include "helpers/crypto/hmac/SHA1.h"
#include "helpers/crypto/hmac/HMAC_SHA1.h"
#include "helpers/crypto/sha512/SHA512.h"
#include "helpers/PerlinNoise.hpp"

#include "undo/UndoMaster.h"
#include "manager/Factory.h"


#include "script/ScriptTarget.h"


#include "warning/WarningTarget.h"




#include "inspectable/Inspectable.h"
#include "inspectable/InspectableContent.h"
#include "inspectable/InspectableSelectionManager.h"
#include "inspectable/ui/InspectableContentComponent.h"
#include "inspectable/ui/InspectableEditor.h"
#include "inspectable/ui/InspectableSelector.h"
#include "inspectable/ui/Inspector.h"

#include "warning/WarningReporter.h"
#include "warning/ui/WarningTargetUI.h"

#include "help/HelpBox.h"

#include "controllable/ControllableContainerListener.h"

#include "script/ExpressionListener.h"

#include "dashboard/DashboardItemProvider.h"

#include "controllable/Controllable.h"

#include "controllable/parameter/Parameter.h"

#include "controllable/ui/ControllableUI.h"
#include "controllable/ui/ControllableEditor.h"
#include "controllable/parameter/ui/ParameterUI.h"
#include "controllable/parameter/ui/ParameterEditor.h"

#include "script/ScriptExpression.h"

#include "controllable/ControllableFactory.h"
#include "controllable/ControllableHelpers.h"

#include "controllable/parameter/BoolParameter.h"
#include "controllable/parameter/ColorParameter.h"
#include "controllable/parameter/EnumParameter.h"
#include "controllable/parameter/FloatParameter.h"
#include "controllable/parameter/IntParameter.h"
#include "controllable/parameter/Point2DParameter.h"
#include "controllable/parameter/Point3DParameter.h"
#include "controllable/parameter/StringParameter.h"
#include "controllable/parameter/FileParameter.h"
#include "controllable/parameter/TargetParameter.h"

#include "controllable/parameter/ui/BetterStepper.h"
#include "controllable/parameter/ui/BoolToggleUI.h"
#include "controllable/parameter/ui/ColorParameterUI.h"
#include "controllable/parameter/ui/EnumParameterUI.h"
#include "controllable/parameter/ui/FloatParameterLabelUI.h"
#include "controllable/parameter/ui/IntParameterLabelUI.h"
#include "controllable/parameter/ui/FloatSliderUI.h"
#include "controllable/parameter/ui/P2DUI.h"
#include "controllable/parameter/ui/DoubleSliderUI.h"
#include "controllable/parameter/ui/FloatStepperUI.h"
#include "controllable/parameter/ui/IntSliderUI.h"
#include "controllable/parameter/ui/IntStepperUI.h"
#include "controllable/parameter/ui/StringParameterUI.h"
#include "controllable/parameter/ui/TargetParameterUI.h"
#include "controllable/parameter/ui/TripleSliderUI.h"
#include "controllable/parameter/ui/ColorStatusUI.h"

#include "controllable/Trigger.h"
#include "controllable/ui/TriggerUI.h"
#include "controllable/ui/TriggerBlinkUI.h"
#include "controllable/ui/TriggerButtonUI.h"
#include "controllable/ui/TriggerImageUI.h"

#include "controllable/ControllableContainer.h"
#include "controllable/ui/GenericControllableContainerEditor.h"

#include "controllable/ControllableUtil.h"



#include "updater/AppUpdater.h"

#include "progress/ProgressNotifier.h"
#include "progress/ui/ProgressWindow.h"

#include "warning/ui/WarningReporterPanel.h"

#include "engine/Engine.h"

#include "settings/GlobalSettings.h"
#include "settings/ui/GlobalSettingsEditor.h"
#include "settings/ProjectSettings.h"

#include "ui/shapeshifter/GapGrabber.h"
#include "ui/shapeshifter/ShapeShifter.h"
#include "ui/shapeshifter/ShapeShifterContainer.h"
#include "ui/shapeshifter/ShapeShifterContent.h"
#include "ui/shapeshifter/ShapeShifterFactory.h"
#include "ui/shapeshifter/ShapeShifterManager.h"
#include "ui/shapeshifter/ShapeShifterPanel.h"
#include "ui/shapeshifter/ShapeShifterPanelHeader.h"
#include "ui/shapeshifter/ShapeShifterPanelTab.h"
#include "ui/shapeshifter/ShapeShifterWindow.h"

#include "logger/CustomLogger.h"
#include "logger/ui/CustomLoggerUI.h"
#include "helpers/WakeOnLan.h"
#include "helpers/NetworkHelpers.h"

#include "remotecontrol/OSCRemoteControl.h"


#include "manager/BaseItemListener.h"
#include "manager/ManagerListener.h"
#include "manager/BaseItem.h"
#include "manager/ItemGroup.h"

#include "manager/ui/BaseItemMinimalUI.h"
#include "manager/ui/BaseItemUI.h"
#include "manager/ui/BaseItemEditor.h"
#include "manager/ui/GenericManagerEditor.h"

#include "manager/Manager.h"

#include "manager/ui/ManagerUI.h"
#include "manager/ui/ManagerViewMiniPane.h"
#include "manager/ui/ManagerViewUI.h"
#include "manager/ui/ManagerShapeShifterUI.h"

#include "manager/GenericControllableItem.h"
#include "manager/GenericControllableManager.h"
#include "manager/ui/GenericControllableItemEditor.h"

#include "controllable/detective/ControllableDetectiveWatcher.h"
#include "controllable/parameter/detective/ParameterDetectiveWatcher.h"
#include "controllable/detective/Detective.h"
#include "controllable/detective/ui/ControllableDetectiveWatcherUI.h"
#include "controllable/parameter/detective/ui/ParameterDetectiveWatcherUI.h"
#include "controllable/detective/ui/DetectivePanel.h"


#include "controllable/parameter/gradient/GradientColor.h"
#include "controllable/parameter/gradient/GradientColorManager.h"
#include "controllable/parameter/gradient/ui/GradientColorUI.h"
#include "controllable/parameter/gradient/ui/GradientColorManagerUI.h"
#include "controllable/parameter/gradient/ui/GradientColorManagerEditor.h"


#include "automation/recorder/AutomationRecorder.h"
#include "automation/easing/Easing.h"
#include "automation/AutomationKey.h"
#include "automation/Automation.h"
#include "automation/parameter/ParameterAutomation.h"
#include "automation/easing/ui/EasingUI.h"
#include "automation/ui/AutomationKeyUI.h"
#include "automation/ui/AutomationMultiKeyTransformer.h"
#include "automation/ui/AutomationUI.h"
#include "automation/ui/AutomationEditor.h"
#include "automation/recorder/ui/AutomationRecorderEditor.h"
#include "automation/parameter/ui/ParameterAutomationEditor.h"
#include "automation/curve/curve2D/easing/Easing2D.h"
#include "automation/curve/curve2D/Curve2DKey.h"
#include "automation/curve/curve2D/Curve2D.h"
#include "automation/curve/curve2D/easing/ui/Easing2DUI.h"
#include "automation/curve/curve2D/ui/Curve2DKeyUI.h"
#include "automation/curve/curve2D/ui/Curve2DUI.h"
#include "automation/curve/curve2D/ui/Curve2DEditor.h"

#include "comment/Comment.h"
#include "comment/CommentManager.h"
#include "comment/ui/CommentUI.h"
#include "comment/ui/CommentManagerViewUI.h"

#include "dashboard/DashboardFeedbackBroadcaster.h"
#include "dashboard/DashboardItem.h"
#include "dashboard/DashboardItemFactory.h"
#include "dashboard/DashboardItemManager.h"
#include "dashboard/DashboardCommentItem.h"
#include "dashboard/DashboardGroupItem.h"
#include "dashboard/DashboardIFrameItem.h"

#include "dashboard/Dashboard.h"
#include "dashboard/DashboardManager.h"
#include "dashboard/ui/DashboardItemManagerUI.h"
#include "dashboard/ui/DashboardUI.h"
#include "dashboard/ui/DashboardItemUI.h"
#include "dashboard/ui/DashboardCommentItemUI.h"
#include "dashboard/ui/DashboardGroupItemUI.h"
#include "dashboard/ui/DashboardIFrameItemUI.h"
#include "dashboard/ui/DashboardManagerUI.h"
#include "dashboard/ui/DashboardManagerView.h"

#include "inspectable/dashboard/DashboardInspectableItem.h"
#include "controllable/dashboard/DashboardControllableItem.h"
#include "controllable/dashboard/DashboardTriggerItem.h"
#include "controllable/dashboard/DashboardCCItem.h"
#include "inspectable/dashboard/ui/DashboardInspectableItemUI.h"
#include "controllable/dashboard/ui/DashboardControllableItemUI.h"
#include "controllable/dashboard/ui/DashboardTriggerItemUI.h"
#include "controllable/dashboard/ui/DashboardCCItemUI.h"
#include "controllable/parameter/dashboard/DashboardParameterItem.h"
#include "controllable/parameter/dashboard/ui/DashboardParameterItemUI.h"


#include "dashboard/DashboardLinkItem.h"
#include "dashboard/ui/DashboardLinkItemUI.h"

#include "parrot/ParrotRecord.h"
#include "parrot/ParrotRecordManager.h"
#include "parrot/Parrot.h"
#include "parrot/ParrotManager.h"
#include "parrot/ui/ParrotUI.h"
#include "parrot/ui/ParrotPanel.h"

#include "script/Script.h"
#include "script/ScriptManager.h"
#include "script/ScriptUtil.h"
#include "script/ui/ScriptEditor.h"
#include "script/ui/ScriptManagerEditor.h"


#include "helpers/OSCHelpers.h"

#include "outliner/Outliner.h"

#include "help/HelpPanel.h"

#include "app/CrashHandler.h"
#include "app/OrganicMainComponent.h"
#include "app/OrganicApplication.h"




#if ORGANICUI_USE_SHAREDTEXTURE
#include <juce_sharedtexture/juce_sharedtexture.h>
#include "dashboard/SharedTextureDashboardItem.h"
#include "dashboard/ui/SharedTextureDashboardItemUI.h"
#endif

