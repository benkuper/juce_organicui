/*
  ==============================================================================

    OSCRemoteControl.h
    Created: 23 Apr 2018 5:00:30pm
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class OSCRemoteControl :
	public EnablingControllableContainer,
	public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public: 
	juce_DeclareSingleton(OSCRemoteControl, true);

	OSCRemoteControl();
	~OSCRemoteControl();

	IntParameter * localPort;

	OSCReceiver receiver;

	void setupReceiver();

	void processMessage(const OSCMessage &m);

	void onContainerParameterChanged(Parameter * p) override;
	
	void oscMessageReceived(const OSCMessage &m) override;
	void oscBundleReceived(const OSCBundle &b) override;
};

