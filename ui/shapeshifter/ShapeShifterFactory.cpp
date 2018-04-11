/*
  ==============================================================================

    ShapeShifterFactory.cpp
    Created: 18 May 2016 11:45:57am
    Author:  bkupe

  ==============================================================================
*/

juce_ImplementSingleton(ShapeShifterFactory)

ShapeShifterFactory::ShapeShifterFactory()
{
	defs.add(new ShapeShifterDefinition("Inspector", &InspectorUI::create));
	defs.add(new ShapeShifterDefinition("Outliner", &Outliner::create));
	defs.add(new ShapeShifterDefinition("Dashboard", &DashboardManagerView::create));
	defs.add(new ShapeShifterDefinition("Logger", &CustomLoggerUI::create));
	defs.add(new ShapeShifterDefinition("Help", &HelpPanel::create));
}
