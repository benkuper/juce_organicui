/*
  ==============================================================================

    OSCRemoteControl.h
    Created: 23 Apr 2018 5:00:30pm
    Author:  Ben

  ==============================================================================
*/

#pragma once


#if ORGANICUI_USE_SERVUS
#include "servus/servus.h"
#endif

class OSCRemoteControl :
	public EnablingControllableContainer,
#if ORGANICUI_USE_SERVUS
	public Thread,
#endif
	public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>

{
public: 
	juce_DeclareSingleton(OSCRemoteControl, true);

	OSCRemoteControl();
	~OSCRemoteControl();

	IntParameter * localPort;
	OSCReceiver receiver;

	void setupReceiver();

#if ORGANICUI_USE_SERVUS
	servus::Servus servus;
	void setupZeroconf();
#endif

	void processMessage(const OSCMessage &m);

	void onContainerParameterChanged(Parameter * p) override;
	
	void oscMessageReceived(const OSCMessage &m) override;
	void oscBundleReceived(const OSCBundle &b) override;

#if ORGANICUI_USE_SERVUS
	void run() override;
#endif

	class RemoteControlListener
	{
	public:
        virtual ~RemoteControlListener(){}
		virtual void processMessage(const OSCMessage &m) {}
	};

	ListenerList<RemoteControlListener> remoteControlListeners;
	void addRemoteControlListener(RemoteControlListener* e) { remoteControlListeners.add(e); }
	void removeRemoteControlListener(RemoteControlListener* e) { remoteControlListeners.remove(e); }

};

