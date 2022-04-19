#pragma once

class Controllable;
class ControllableContainer;
class Parameter;

#include "../manager/BaseManager.h"

class OSCHelpers
{
public:
	enum ColorMode { ColorRGBA, Float3, Float4 };

	static void logOSCFormatError(const char* message, int length);
	
	static OSCArgument varToArgument(const var& v);
	static OSCArgument varToColorArgument(const var& v);
	static var argumentToVar(const OSCArgument& a);

	static void addArgumentsForParameter(OSCMessage& m, Parameter* p);
	static void addColorArgumentToMessage(OSCMessage& m, const Colour& c, ColorMode colorMode);

	static float getFloatArg(OSCArgument a);
	static int getIntArg(OSCArgument a);
	static String getStringArg(OSCArgument a);
	static OSCColour getOSCColour(Colour c);
	static Point<float> getP2DArg(const OSCMessage& m, int startIndex = 0);
	static Vector3D<float> getP3DArg(const OSCMessage& m, int startIndex = 0);

	static Colour getColourFromOSC(OSCColour c);


	static OSCMessage getOSCMessageForControllable(Controllable* p, ControllableContainer* addressRelativeTo = nullptr);

	static Controllable* findControllableAndHandleMessage(ControllableContainer* root, const OSCMessage& m, int dataOffset = 0);
	static Controllable * findControllable(ControllableContainer* root, const OSCMessage& m, int dataOffset = 0);

	static void handleControllableForOSCMessage(Controllable* c, const OSCMessage& m, int dataOffset = 0);

	template<class T>
	static T* getItemForArgument(BaseManager<T>* manager, const OSCMessage& m, int argIndex)
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
};
