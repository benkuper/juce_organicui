#pragma once

class Controllable;
class ControllableContainer;
class Parameter;

class OSCHelpers
{
public:
	 
	static void logOSCFormatError(const char* message, int length);
	
	static OSCArgument varToArgument(const var& v);
	static OSCArgument varToColorArgument(const var& v);
	static var argumentToVar(const OSCArgument& a);

	static float getFloatArg(OSCArgument a);
	static int getIntArg(OSCArgument a);
	static String getStringArg(OSCArgument a);
	static OSCColour getOSCColour(Colour c);

	static Colour getColourFromOSC(OSCColour c);


	static OSCMessage getOSCMessageForParameter(Parameter* p, ControllableContainer* addressRelativeTo = nullptr);
	static void addArgumentsForParameters(OSCMessage& m, Parameter* p);

	static Controllable * findControllableAndHandleMessage(ControllableContainer* root, const OSCMessage& m, int dataOffset = 0);

	static void handleControllableForOSCMessage(Controllable* c, const OSCMessage& m, int dataOffset = 0);
};
