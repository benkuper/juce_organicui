/*
  ==============================================================================

   

  ==============================================================================
*/

#ifdef ORGANIC_UI_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "juce_organicui.h"

//==============================================================================

#include "helpers/StringUtil.cpp"
#include "helpers/OSCHelpers.cpp"
#include "helpers/crypto/hmac/SHA1.cpp"
#include "helpers/crypto/hmac/HMAC_SHA1.cpp"

#include "resources/BinaryData.cpp"
#include "script/ScriptExpression.cpp"
#include "controllable/Controllable.cpp"
#include "controllable/ControllableContainer.cpp"
#include "controllable/ControllableFactory.cpp"
#include "controllable/ControllableHelpers.cpp"
#include "controllable/parameter/BoolParameter.cpp"
#include "controllable/parameter/ColorParameter.cpp"
#include "controllable/parameter/EnumParameter.cpp"
#include "controllable/parameter/FloatParameter.cpp"
#include "controllable/parameter/FloatRangeParameter.cpp"
#include "controllable/parameter/IntParameter.cpp"
#include "controllable/parameter/IntRangeParameter.cpp"
#include "controllable/parameter/Parameter.cpp"
#include "controllable/parameter/Point2DParameter.cpp"
#include "controllable/parameter/Point3DParameter.cpp"
#include "controllable/parameter/StringParameter.cpp"
#include "controllable/parameter/FileParameter.cpp"
#include "controllable/parameter/TargetParameter.cpp"
#include "controllable/parameter/ui/BetterStepper.cpp"
#include "controllable/parameter/ui/BoolImageToggleUI.cpp"
#include "controllable/parameter/ui/BoolToggleUI.cpp"
#include "controllable/parameter/ui/ColorParameterUI.cpp"
#include "controllable/parameter/ui/DoubleSliderUI.cpp"
#include "controllable/parameter/ui/EnumParameterUI.cpp"
#include "controllable/parameter/ui/FloatParameterLabelUI.cpp"
#include "controllable/parameter/ui/IntParameterLabelUI.cpp"
#include "controllable/parameter/ui/FloatSliderUI.cpp"
#include "controllable/parameter/ui/FloatStepperUI.cpp"
#include "controllable/parameter/ui/IntSliderUI.cpp"
#include "controllable/parameter/ui/IntStepperUI.cpp"
#include "controllable/parameter/ui/ParameterUI.cpp"
#include "controllable/parameter/ui/ParameterEditor.cpp"
#include "controllable/parameter/ui/StringParameterUI.cpp"
#include "controllable/parameter/ui/TargetParameterUI.cpp"
#include "controllable/parameter/ui/TripleSliderUI.cpp"

#include "controllable/parameter/gradient/GradientColor.cpp"
#include "controllable/parameter/gradient/GradientColorManager.cpp"
#include "controllable/parameter/gradient/ui/GradientColorUI.cpp"
#include "controllable/parameter/gradient/ui/GradientColorManagerUI.cpp"
#include "controllable/parameter/gradient/ui/GradientColorManagerEditor.cpp"

#include "controllable/Trigger.cpp"
#include "controllable/ui/ControllableEditor.cpp"
#include "controllable/ui/ControllableUI.cpp"
#include "controllable/ui/GenericControllableContainerEditor.cpp"
#include "controllable/ui/TriggerBlinkUI.cpp"
#include "controllable/ui/TriggerButtonUI.cpp"
#include "controllable/ui/TriggerImageUI.cpp"
#include "controllable/ui/TriggerUI.cpp"


#pragma warning(push)
#pragma warning(disable:4067 4200)
#include "automation/common/fitting/intern/curve_fit_cubic.c";
#include "automation/common/fitting/intern/curve_fit_corners_detect.c";
#pragma warning(pop)

#include "automation/recorder/AutomationRecorder.cpp"
#include "automation/easing/Easing.cpp"
#include "automation/AutomationKey.cpp"
#include "automation/Automation.cpp"
#include "automation/parameter/ParameterAutomation.cpp"
#include "automation/easing/ui/EasingUI.cpp"
#include "automation/ui/AutomationKeyUI.cpp"
#include "automation/ui/AutomationMultiKeyTransformer.cpp"
#include "automation/ui/AutomationUI.cpp"
#include "automation/ui/AutomationEditor.cpp"
#include "automation/recorder/ui/AutomationRecorderEditor.cpp"
#include "automation/parameter/ui/ParameterAutomationEditor.cpp"
#include "automation/curve/curve2D/easing/Easing2D.cpp"
#include "automation/curve/curve2D/Curve2DKey.cpp"
#include "automation/curve/curve2D/Curve2D.cpp"
#include "automation/curve/curve2D/easing/ui/Easing2DUI.cpp"
#include "automation/curve/curve2D/ui/Curve2DKeyUI.cpp"
#include "automation/curve/curve2D/ui/Curve2DUI.cpp"
#include "automation/curve/curve2D/ui/Curve2DEditor.cpp"