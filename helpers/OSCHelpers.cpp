#include "OSCHelpers.h"

void OSCHelpers::logOSCFormatError(const char* message, int length)
{
	LOGERROR("OSC Error : " << String(message, length));
}

OSCArgument OSCHelpers::varToArgument(const var& v)
{
	if (v.isBool()) return OSCArgument(((bool)v) ? 1 : 0);
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
	if (a.isString()) return var(a.getString());
	return var("error");
}

void OSCHelpers::addArgumentsForParameter(OSCMessage& m, Parameter* p)
{
	switch (p->type)
	{

	case Controllable::BOOL: m.addInt32(p->intValue()); break;
	case Controllable::INT: m.addInt32(p->intValue()); break;
	case Controllable::FLOAT: m.addFloat32(p->floatValue()); break;
	case Controllable::STRING: m.addString(p->stringValue()); break;
	case Controllable::COLOR: m.addColour(OSCHelpers::getOSCColour(((ColorParameter*)p)->getColor())); break;
	case Controllable::POINT2D: 
		m.addFloat32(((Point2DParameter*)p)->x);
		m.addFloat32(((Point2DParameter*)p)->y);
		break;
	case Controllable::POINT3D:
		m.addFloat32(((Point3DParameter*)p)->x);
		m.addFloat32(((Point3DParameter*)p)->y);
		m.addFloat32(((Point3DParameter*)p)->z);
		break;

	case Controllable::ENUM: m.addString(((EnumParameter*)p)->getValueKey()); break;

	default:
		jassertfalse;
		if (p->isComplex()) for (int i = 0; i < p->value.size(); i++) m.addArgument(varToArgument(p->value[i]));
		else m.addArgument(varToArgument(p->value));
		break;
	}
}

float OSCHelpers::getFloatArg(OSCArgument a)
{
	if (a.isFloat32()) return a.getFloat32();
	if (a.isInt32()) return (float)a.getInt32();
	if (a.isString()) return a.getString().getFloatValue();
	return 0;
}

int OSCHelpers::getIntArg(OSCArgument a)
{
	if (a.isInt32()) return a.getInt32();
	if (a.isFloat32()) return (int)roundf(a.getFloat32());
	if (a.isString()) return a.getString().getIntValue();
	return 0;
}

String OSCHelpers::getStringArg(OSCArgument a)
{
	if (a.isString()) return a.getString();
	if (a.isInt32()) return String(a.getInt32());
	if (a.isFloat32()) return String(a.getFloat32());
	return "";
}

OSCColour OSCHelpers::getOSCColour(Colour c)
{
	return OSCColour::fromInt32((int32)(c.getRed() << 24 | c.getGreen() << 16 | c.getBlue() << 8 | c.getAlpha()));
}

Point<float> OSCHelpers::getP2DArg(const OSCMessage& m, int startIndex)
{
	if (m.size() <= startIndex + 1) return Point<float>();
	return Point<float>(getFloatArg(m[startIndex]), getFloatArg(m[startIndex+1]));
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

OSCMessage OSCHelpers::getOSCMessageForControllable(Controllable* c, ControllableContainer* addressRelativeTo)
{
	OSCMessage m(c->getControlAddress(addressRelativeTo));
	if(c->type != c->TRIGGER) addArgumentsForParameter(m, (Parameter *)c);
	return m;
}

Controllable * OSCHelpers::findControllableAndHandleMessage(ControllableContainer* root, const OSCMessage& m, int dataOffset)
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
	else if(Parameter* p = dynamic_cast<Parameter*>(c))
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
			else if (m.size() < dataOffset+3) LOG("Parameter " << p->niceName << " requires at least 3 arguments");
			else static_cast<ColorParameter*>(p)->setColor(Colour::fromFloatRGBA(getFloatArg(m[dataOffset]), getFloatArg(m[dataOffset+1]), getFloatArg(m[dataOffset+2]), m.size() >= 4 ? getFloatArg(m[dataOffset+3]) : 1));
			break;

		case Controllable::POINT2D:
			if (m.size() < dataOffset + 2) LOG("Parameter " << p->niceName << " requires at least 2 arguments");
			else static_cast<Point2DParameter*>(p)->setPoint(getFloatArg(m[dataOffset]), getFloatArg(m[dataOffset+1]));
			break;

		case Controllable::POINT3D:
			if (m.size() < dataOffset + 3) LOG("Parameter " << p->niceName << " requires at least 3 arguments");
			else static_cast<Point3DParameter*>(p)->setVector(getFloatArg(m[dataOffset]), getFloatArg(m[dataOffset+1]), getFloatArg(m[dataOffset+2]));
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
