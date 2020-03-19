#pragma once

class AutomationKey2DUI :
	public BaseItemMinimalUI<AutomationKey>
{
public:
	AutomationKey2DUI(AutomationKey * key);
	~AutomationKey2DUI();

	class Easing2DHandle :
		public Component
	{
	public:
		Easing2DHandle(AutomationKey* key, bool isFirst);
		~Easing2DHandle();

		AutomationKey* key;
		bool isFirst;

		void paint(Graphics& g) override;
	};

	OwnedArray<Easing2DHandle> handles;

	void paint(Graphics& g) override;
	void updateHandles();

	void controllableFeedbackUpdateInternal(Controllable* c) override;
	void inspectableSelectionChanged(Inspectable*) override;


	class Key2DUIEvent
	{
	public:
		enum Type { HANDLES_REMOVED, HANDLES_ADDED, UI_SELECTION_CHANGED };

		Key2DUIEvent(Type t, AutomationKey2DUI* kui, Array<Easing2DHandle*> handles = Array<Easing2DHandle*>()) 
			: type(t), keyUI(kui), handles(handles) 
		{}

		Type type;
		AutomationKey2DUI* keyUI;
		Array<Easing2DHandle*> handles;
	};

	QueuedNotifier<Key2DUIEvent> keyUINotifier;
	typedef QueuedNotifier<Key2DUIEvent>::Listener AsyncListener;

	void addAsyncKey2DUIListener(AsyncListener* newListener) { keyUINotifier.addListener(newListener); }
	void addAsyncCoalescedKey2DUIListener(AsyncListener* newListener) { keyUINotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncKey2DUIListener(AsyncListener* listener) { keyUINotifier.removeListener(listener); }
};