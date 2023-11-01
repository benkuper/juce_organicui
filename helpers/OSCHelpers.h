#pragma once

class Controllable;
class ControllableContainer;
class Parameter;

class OSCHelpers
{
public:
	enum ColorMode { ColorRGBA, Float3, Float4 };
	enum BoolMode { Int, Float, TF };

	static void logOSCFormatError(const char* message, int length);

	static juce::OSCArgument varToArgument(const juce::var& v, BoolMode bm);
	static juce::OSCArgument varToColorArgument(const juce::var& v);
	static juce::var argumentToVar(const juce::OSCArgument& a);

	static void addArgumentsForParameter(juce::OSCMessage& m, Parameter* p, BoolMode bm = Int, ColorMode cm = ColorRGBA, juce::var forceVar = juce::var());
	static void addBoolArgumentToMessage(juce::OSCMessage& m, bool value, BoolMode boolMode);
	static void addColorArgumentToMessage(juce::OSCMessage& m, const juce::Colour& c, ColorMode colorMode);

	static bool getBoolArg(juce::OSCArgument a);
	static float getFloatArg(juce::OSCArgument a);
	static int getIntArg(juce::OSCArgument a);
	static juce::String getStringArg(juce::OSCArgument a);
	static juce::OSCColour getOSCColour(juce::Colour c);
	static juce::Point<float> getP2DArg(const juce::OSCMessage& m, int startIndex = 0);
	static juce::Vector3D<float> getP3DArg(const juce::OSCMessage& m, int startIndex = 0);

	static juce::Colour getColourFromOSC(juce::OSCColour c);

	static juce::String getLastAddressPart(const juce::OSCMessage& m);

	static juce::OSCMessage getOSCMessageForControllable(Controllable* p, ControllableContainer* addressRelativeTo = nullptr, BoolMode bm = Int, ColorMode cm = ColorRGBA);

	static Controllable* findControllableAndHandleMessage(ControllableContainer* root, const juce::OSCMessage& m, int dataOffset = 0);
	static Controllable* findControllable(ControllableContainer* root, const juce::OSCMessage& m, int dataOffset = 0);
	static ControllableContainer* findParentContainer(ControllableContainer* root, const juce::OSCMessage& m, int dataOffset = 0);

	static void handleControllableForOSCMessage(Controllable* c, const juce::OSCMessage& m, int dataOffset = 0);

	template<class T>
	static T* getItemForArgument(BaseManager<T>* manager, const juce::OSCMessage& m, int argIndex)
	{
		if (m.size() <= argIndex) return nullptr;

		if (m[0].isInt32())
		{
			int index = m[0].getInt32();
			if (manager->items.size() <= index) return nullptr;
			return manager->items[index];
		}

		return manager->getItemWithName(getStringArg(m[0]), true, true);
	}

	static juce::String messageToString(const juce::OSCMessage& m);
};


class OSCQueryHelpers
{
public:

	class OSCQueryValueContainer :
		public EnablingControllableContainer
	{
	public:
		OSCQueryValueContainer(const juce::String& name, bool canListen = true, bool canSync = true);
		~OSCQueryValueContainer();

		BoolParameter* enableListen;
		BoolParameter* syncContent;

		InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
	};

	static void updateContainerFromData(ControllableContainer* cc, juce::var data, bool useAddressForNaming = false, std::function<OSCQueryValueContainer* (const juce::String& name)> createContainerFunc = nullptr);
	
	static void createOrUpdateControllableFromData(ControllableContainer* parentCC, Controllable* c, juce::StringRef name, juce::var data, bool useAddresForNaming = false);



	class OSCQueryValueContainerEditor :
		public EnablingControllableContainerEditor
	{
	public:
		OSCQueryValueContainerEditor(OSCQueryValueContainer* cc, bool isRoot);
		~OSCQueryValueContainerEditor();

		std::unique_ptr<BoolToggleUI> enableListenUI;
		std::unique_ptr<BoolToggleUI> syncUI;

		void resizedInternalHeader(juce::Rectangle<int>& r) override;
	};
};
