#include "JuceHeader.h"
#include "OSCHelpers.h"

void OSCHelpers::logOSCFormatError(const char* message, int length)
{
	LOGERROR("OSC Error : " << String(message, length));
}



OSCArgument OSCHelpers::varToArgument(const var& v, BoolMode bm)
{
	if (v.isBool())
	{
		switch (bm)
		{
		case TF: return OSCArgument((bool)v); break;
		case Int: return OSCArgument((int)v); break;
		case Float: return OSCArgument((float)v); break;
		}
	}
	else if (v.isInt()) return OSCArgument((int)v);
	else if (v.isInt64()) return OSCArgument((int)v);
	else if (v.isDouble()) return OSCArgument((float)v);
	else if (v.isString()) return OSCArgument(v.toString());
	else if (v.isArray() && v.size() == 4)
	{
		int col = 0;
		for (int i = 0; i < v.size(); ++i) col += (int)((float)v[i] * 255) << ((3 - i) * 8);

		return OSCArgument(OSCColour::fromInt32(col));
	}
	else if (v.isUndefined()) return OSCArgument();
	else if (v.isVoid()) return OSCArgument();

	jassert(false);

	return OSCArgument("error");
}

OSCArgument OSCHelpers::varToColorArgument(const var& v)
{
	if (v.isBool()) return OSCArgument(OSCHelpers::getOSCColour((bool)v ? Colours::white : Colours::black));
	else if (v.isInt() || v.isInt64() || v.isDouble())
	{
		int iv = (int)v;
		OSCColour c = OSCColour::fromInt32(iv);// >> 24 & 0xFF | iv >> 16 & 0xFF | iv >> 8 & 0xFF | iv & 0xFF);
		return OSCArgument(c);
	}
	else if (v.isString())
	{
		Colour c = Colour::fromString(v.toString());
		return OSCArgument(OSCHelpers::getOSCColour(c));
	}
	else if (v.isArray() && v.size() >= 3)
	{
		Colour c = Colour::fromFloatRGBA(v[0], v[1], v[2], v.size() >= 4 ? (float)v[3] : 1.0f);
		return OSCArgument(OSCHelpers::getOSCColour(c));
	}

	jassert(false);
	return OSCArgument("error");
}

var OSCHelpers::argumentToVar(const OSCArgument& a)
{
	if (a.isFloat32()) return var(a.getFloat32());
	if (a.isInt32()) return var(a.getInt32());
	if (a.isInt64()) return var((int)a.getInt64());
	if (a.isString()) return var(a.getString());
	if (a.isTorF()) return var(a.getBool());
	if (a.isImpulse()) return var();
	return var("error");
}

void OSCHelpers::addArgumentsForParameter(OSCMessage& m, Parameter* p, BoolMode bm, ColorMode cm, var forceVar)
{
	var val = forceVar.isVoid() ? p->getValue() : forceVar;

	switch (p->type)
	{

	case Controllable::BOOL: OSCHelpers::addBoolArgumentToMessage(m, (bool)val, bm); break;
	case Controllable::INT: m.addInt32((int)val); break;
	case Controllable::FLOAT: m.addFloat32((float)val); break;
	case Controllable::STRING: m.addString(val.toString()); break;
	case Controllable::COLOR:
	{
		Colour c = Colour::fromFloatRGBA(val[0], val[1], val[2], val[3]);
		OSCHelpers::addColorArgumentToMessage(m, c, cm);
		break;
	}
	case Controllable::POINT2D:
	case Controllable::POINT3D:
		for (int i = 0; i < val.size(); i++) m.addFloat32(val[i]);
		break;

	case Controllable::ENUM:
	{
		m.addString(((EnumParameter*)p)->getValueKey());
	}
	break;

	case Controllable::TARGET:
		m.addString(val.toString());
		break;

	default:
		jassertfalse;
		if (p->isComplex()) for (int i = 0; i < val.size(); i++) m.addArgument(varToArgument(val[i], bm));
		else m.addArgument(varToArgument(p->getValue(), bm));
		break;
	}
}

void OSCHelpers::addBoolArgumentToMessage(OSCMessage& m, bool val, BoolMode boolMode)
{
	switch (boolMode)
	{
	case Int: m.addInt32(val ? 1 : 0); break;
	case Float: m.addFloat32(val ? 1 : 0); break;
	case TF: m.addBool(val); break;
	}
}


void OSCHelpers::addColorArgumentToMessage(OSCMessage& m, const Colour& c, ColorMode colorMode)
{
	if (colorMode == ColorRGBA) m.addColour(OSCHelpers::getOSCColour(c));
	else
	{
		m.addFloat32(c.getFloatRed());
		m.addFloat32(c.getFloatGreen());
		m.addFloat32(c.getFloatBlue());
		if (colorMode == Float4) m.addFloat32(c.getFloatAlpha());
	}
}

bool OSCHelpers::getBoolArg(OSCArgument a)
{
	if (a.isFloat32()) return a.getFloat32() >= 1;
	if (a.isInt32()) return (float)a.getInt32() >= 1;
	if (a.isInt64()) return (float)a.getInt64() >= 1;
	if (a.isString()) return a.getString().getIntValue() >= 1;
	if (a.isTorF()) return a.getBool();
	if (a.isImpulse()) return true;
	return false;
}


float OSCHelpers::getFloatArg(OSCArgument a)
{
	if (a.isFloat32()) return a.getFloat32();
	if (a.isInt32()) return (float)a.getInt32();
	if (a.isInt64()) return (float)a.getInt64();
	if (a.isString()) return a.getString().getFloatValue();
	if (a.isTorF()) return a.getBool() ? 1.0f : 0.0f;
	if (a.isImpulse()) return 1;
	return 0;
}

int OSCHelpers::getIntArg(OSCArgument a)
{
	if (a.isInt32()) return a.getInt32();
	if (a.isInt64()) return (int)a.getInt64();
	if (a.isFloat32()) return (int)roundf(a.getFloat32());
	if (a.isString()) return a.getString().getIntValue();
	if (a.isTorF()) return a.getBool() ? 1 : 0;
	if (a.isImpulse()) return 1;
	return 0;
}

String OSCHelpers::getStringArg(OSCArgument a)
{
	if (a.isString()) return a.getString();
	if (a.isInt32()) return String(a.getInt32());
	if (a.isInt64()) return String(a.getInt64());
	if (a.isFloat32()) return String(a.getFloat32());
	if (a.isTorF()) return a.getBool() ? "True" : "False";
	if (a.isBlob()) return "[Blob : " + String(a.getBlob().getSize()) + " bytes]";
	if (a.isImpulse()) return var("");
	return "";
}

OSCColour OSCHelpers::getOSCColour(Colour c)
{
	return OSCColour::fromInt32((int32)(c.getRed() << 24 | c.getGreen() << 16 | c.getBlue() << 8 | c.getAlpha()));
}

Point<float> OSCHelpers::getP2DArg(const OSCMessage& m, int startIndex)
{
	if (m.size() <= startIndex + 1) return Point<float>();
	return Point<float>(getFloatArg(m[startIndex]), getFloatArg(m[startIndex + 1]));
}

Vector3D<float> OSCHelpers::getP3DArg(const OSCMessage& m, int startIndex)
{
	if (m.size() <= startIndex + 2) return Vector3D<float>();
	return Vector3D<float>(getFloatArg(m[startIndex]), getFloatArg(m[startIndex + 1]), getFloatArg(m[startIndex + 2]));
}


Colour OSCHelpers::getColourFromOSC(OSCColour c)
{
	return Colour(c.red, c.green, c.blue, c.alpha);
}

OSCMessage OSCHelpers::getOSCMessageForControllable(Controllable* c, ControllableContainer* addressRelativeTo, BoolMode bm, ColorMode cm)
{
	OSCMessage m(c->getControlAddress(addressRelativeTo));
	if (c->type != c->TRIGGER) addArgumentsForParameter(m, (Parameter*)c, bm, cm);
	return m;
}

Controllable* OSCHelpers::findControllableAndHandleMessage(ControllableContainer* root, const OSCMessage& m, int dataOffset)
{
	Controllable* c = findControllable(root, m, dataOffset);
	if (c == nullptr)
	{
		String addr = m.getAddressPattern().toString();

		if (addr.endsWith("enabled"))
		{
			String cAdd = addr.substring(0, addr.length() - 8);
			c = root->getControllableForAddress(cAdd);

			if (c != nullptr)
			{
				if (m.size() > dataOffset) c->setEnabled(OSCHelpers::getIntArg(m[dataOffset]) > 0);
				return c;
			}
		}
	}

	handleControllableForOSCMessage(c, m, dataOffset);

	return c;

}

Controllable* OSCHelpers::findControllable(ControllableContainer* root, const OSCMessage& m, int dataOffset)
{
	if (root == nullptr) return nullptr;
	String address = m.getAddressPattern().toString();
	return root->getControllableForAddress(address);
}

void OSCHelpers::handleControllableForOSCMessage(Controllable* c, const OSCMessage& m, int dataOffset)
{
	if (c == nullptr) return;

	if (c->type == Controllable::TRIGGER)
	{
		if (m.size() <= dataOffset || (m.size() > dataOffset && OSCHelpers::getIntArg(m[dataOffset]) > 0)) static_cast<Trigger*>(c)->trigger();
	}
	else if (Parameter* p = dynamic_cast<Parameter*>(c))
	{
		switch (p->type)
		{

		case Controllable::BOOL:
			if (m.size() <= dataOffset) LOG("Parameter " << p->niceName << " requires 1 argument");
			else p->setValue(getIntArg(m[dataOffset]) > 0);
			break;

		case Controllable::INT:
			if (m.size() <= dataOffset) LOG("Parameter " << p->niceName << " requires 1 argument");
			else p->setValue(getIntArg(m[dataOffset]));
			break;

		case Controllable::FLOAT:
			if (m.size() <= dataOffset) LOG("Parameter " << p->niceName << " requires 1 argument");
			else p->setValue(getFloatArg(m[dataOffset]));
			break;

		case Controllable::STRING:
			if (m.size() <= dataOffset) LOG("Parameter " << p->niceName << " requires 1 argument");
			else p->setValue(getStringArg(m[dataOffset]));
			break;

		case Controllable::COLOR:
			if (m[dataOffset].isColour()) static_cast<ColorParameter*>(p)->setColor(getColourFromOSC(m[dataOffset].getColour()));
			else if (m.size() < dataOffset + 3) LOG("Parameter " << p->niceName << " requires at least 3 arguments");
			else static_cast<ColorParameter*>(p)->setColor(Colour::fromFloatRGBA(getFloatArg(m[dataOffset]), getFloatArg(m[dataOffset + 1]), getFloatArg(m[dataOffset + 2]), m.size() >= 4 ? getFloatArg(m[dataOffset + 3]) : 1));
			break;

		case Controllable::POINT2D:
			if (m.size() < dataOffset + 2) LOG("Parameter " << p->niceName << " requires at least 2 arguments");
			else static_cast<Point2DParameter*>(p)->setPoint(getFloatArg(m[dataOffset]), getFloatArg(m[dataOffset + 1]));
			break;

		case Controllable::POINT3D:
			if (m.size() < dataOffset + 3) LOG("Parameter " << p->niceName << " requires at least 3 arguments");
			else static_cast<Point3DParameter*>(p)->setVector(getFloatArg(m[dataOffset]), getFloatArg(m[dataOffset + 1]), getFloatArg(m[dataOffset + 2]));
			break;

		case Controllable::ENUM:
			if (m.size() < dataOffset + 1) LOG("Parameter " << p->niceName << " requires at least 1 argument");
			else static_cast<EnumParameter*>(p)->setValueWithKey(getStringArg(m[dataOffset]));
			break;

		default:
			LOG("Type not handled : " << c->getTypeString());
			break;
		}
	}
}

String OSCHelpers::messageToString(const OSCMessage& m)
{
	String s = m.getAddressPattern().toString();
	for (auto& a : m)
	{
		s += "\n" + OSCHelpers::getStringArg(a);
	}

	return s;
}
