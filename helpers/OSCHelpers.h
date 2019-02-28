#pragma once


class OSCHelpers
{
public:
	 
	static void logOSCFormatError(const char * message, int length)
	{
		LOGERROR("OSC Error : " << String(message, length));
	}

	static OSCArgument varToArgument(const var & v)
	{
		if (v.isBool()) return OSCArgument(((bool)v) ? 1 : 0);
		else if (v.isInt()) return OSCArgument((int)v);
		else if (v.isInt64()) return OSCArgument((int)v);
		else if (v.isDouble()) return OSCArgument((float)v);
		else if (v.isString()) return OSCArgument(v.toString());
		else if (v.isArray() && v.size() == 4)
		{
			int col = 0;
			for (int i = 0; i < v.size(); i++) col += (int)((float)v[i] * 255) << ((3-i) * 8);

			return OSCArgument(OSCColour::fromInt32(col));
		}

		jassert(false);

		return OSCArgument("error");
	}

	static var argumentToVar(const OSCArgument & a)
	{
		if (a.isFloat32()) return var(a.getFloat32());
		if (a.isInt32()) return var(a.getInt32());
		if (a.isString()) return var(a.getString());
		return var("error");
	}


	static float getFloatArg(OSCArgument a)
	{
		if (a.isFloat32()) return a.getFloat32();
		if (a.isInt32()) return (float)a.getInt32();
		if (a.isString()) return a.getString().getFloatValue();
		return 0;
	}

	static int getIntArg(OSCArgument a)
	{
		if (a.isInt32()) return a.getInt32();
		if (a.isFloat32()) return (int)roundf(a.getFloat32());
		if (a.isString()) return a.getString().getIntValue();
		return 0;
	}

	static String getStringArg(OSCArgument a)
	{
		if (a.isString()) return a.getString();
		if (a.isInt32()) return String(a.getInt32());
		if (a.isFloat32()) return String(a.getFloat32());
		return "";
	}

	static OSCColour getOSCColour(Colour c)
	{
		return OSCColour::fromInt32((int32)(c.getRed() << 24 | c.getGreen() << 16 | c.getBlue() << 8 | c.getAlpha()));
	}

	static Colour getColourFromOSC(OSCColour c)
	{
		return Colour(c.red, c.green, c.blue, c.alpha);
	}

};
