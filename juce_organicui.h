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
  version:          1.0.0
  name:             Organic UI
  description:      A sub-framework that adds multiple functionalities such as moving panels, full controllable hierarchy, managers, editors...
  website:          https://github.com/benkuper/juce_organicui
  license:          GPLv3

  dependencies:     juce_core, juce_gui_basics, juce_gui_extra

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef ORGANIC_UI_H_INCLUDED
#define ORGANIC_UI_H_INCLUDED

//==============================================================================
#ifdef _MSC_VER
 #pragma warning (push)
 // Disable warnings for long class names, padding, and undefined preprocessor definitions.
 #pragma warning (disable: 4251 4786 4668 4820)
#endif



#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_gui_extra/juce_gui_extra.h"
#include "juce_opengl/juce_opengl.h"


using namespace juce;


#include "manager/Factory.h"

#include "resources/OrganicBinaryData.h"

#include "ui/Style.h"
#include "ui/AssetManager.h"
#include "ui/LookAndFeelOO.h"

#include "engine/EngineListener.h"

#include "helpers/DebugHelpers.h"
#include "helpers/QueuedNotifier.h"
#include "helpers/StringUtil.h"
#include "helpers/WakeOnLan.h"

#include "script/ScriptTarget.h"


#include "inspectable/Inspectable.h"
#include "inspectable/InspectableContent.h"
#include "inspectable/InspectableSelectionManager.h"
#include "inspectable/ui/InspectableContentComponent.h"
#include "inspectable/ui/InspectableEditor.h"
#include "inspectable/ui/InspectableSelector.h"
#include "inspectable/ui/Inspector.h"

#include "help/HelpBox.h"

#include "controllable/ControllableContainerListener.h"

#include "script/ScriptExpression.h"

#include "controllable/Controllable.h"

#include "controllable/ui/ControllableUI.h"
#include "controllable/ui/ControllableEditor.h"

#include "controllable/parameter/ui/ParameterUI.h"
#include "controllable/parameter/ui/ParameterEditor.h"
#include "controllable/parameter/Parameter.h"

#include "controllable/ControllableFactory.h"
#include "controllable/ControllableHelpers.h"

#include "controllable/parameter/BoolParameter.h"
#include "controllable/parameter/ColorParameter.h"
#include "controllable/parameter/EnumParameter.h"
#include "controllable/parameter/FloatParameter.h"
#include "controllable/parameter/FloatRangeParameter.h"
#include "controllable/parameter/IntParameter.h"
#include "controllable/parameter/IntRangeParameter.h"
#include "controllable/parameter/Point2DParameter.h"
#include "controllable/parameter/Point3DParameter.h"
#include "controllable/parameter/StringParameter.h"
#include "controllable/parameter/TargetParameter.h"

#include "controllable/parameter/ui/BetterStepper.h"
#include "controllable/parameter/ui/BoolImageToggleUI.h"
#include "controllable/parameter/ui/BoolToggleUI.h"
#include "controllable/parameter/ui/ColorParameterUI.h"
#include "controllable/parameter/ui/EnumParameterUI.h"
#include "controllable/parameter/ui/FloatParameterLabelUI.h"
#include "controllable/parameter/ui/FloatSliderUI.h"
#include "controllable/parameter/ui/DoubleSliderUI.h"
#include "controllable/parameter/ui/FloatStepperUI.h"
#include "controllable/parameter/ui/IntSliderUI.h"
#include "controllable/parameter/ui/IntStepperUI.h"
#include "controllable/parameter/ui/StringParameterUI.h"
#include "controllable/parameter/ui/TargetParameterUI.h"
#include "controllable/parameter/ui/TripleSliderUI.h"

#include "controllable/Trigger.h"
#include "controllable/ui/TriggerUI.h"
#include "controllable/ui/TriggerBlinkUI.h"
#include "controllable/ui/TriggerButtonUI.h"
#include "controllable/ui/TriggerImageUI.h"

#include "controllable/ControllableContainer.h"
#include "controllable/ui/GenericControllableContainerEditor.h"

#include "progress/ProgressNotifier.h"
#include "progress/ui/ProgressWindow.h"

#include "engine/Engine.h"


#include "controllable/preset/PresetChooser.h"
#include "controllable/preset/PresetManager.h"

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


#include "undo/UndoMaster.h"
#include "updater/AppUpdater.h"


#include "manager/ui/GenericManagerEditor.h"


#include "manager/BaseItem.h"
#include "manager/ui/BaseItemMinimalUI.h"
#include "manager/ui/BaseItemUI.h"
#include "manager/ui/BaseManagerUI.h"
#include "manager/ui/BaseManagerViewUI.h"
#include "manager/BaseManager.h"
#include "manager/ui/BaseItemEditor.h"
#include "manager/ui/BaseManagerShapeShifterUI.h"

#include "manager/GenericControllableItem.h"
#include "manager/GenericControllableManager.h"
#include "manager/ui/GenericControllableItemUI.h"

#include "dashboard/DashboardItem.h"
#include "dashboard/DashboardItemManager.h"
#include "dashboard/DashboardScriptItem.h"
#include "dashboard/DashboardPanel.h"
#include "dashboard/DashboardPanelManager.h"
#include "dashboard/Dashboard.h"
#include "dashboard/DashboardManager.h"


#include "dashboard/ui/DashboardItemUI.h"
#include "dashboard/ui/DashboardScriptItemUI.h"
#include "dashboard/ui/DashboardItemManagerUI.h"
#include "dashboard/ui/DashboardPanelUI.h"
#include "dashboard/ui/DashboardUI.h"
#include "dashboard/ui/DashboardManagerUI.h"
#include "dashboard/ui/DashboardView.h"
#include "dashboard/ui/DashboardManagerView.h"

#include "script/Script.h"
#include "script/ScriptManager.h"
#include "script/ScriptUtil.h"
#include "script/ui/ScriptUI.h"

#include "logger/CustomLogger.h"
#include "logger/ui/CustomLoggerUI.h"

#include "outliner/Outliner.h"

#include "help/HelpPanel.h"

#endif   // ORGANIC_UI_H_INCLUDED 
