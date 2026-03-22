#include "JuceHeader.h"

using namespace juce;

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

juce::String OSCHelpers::getLastAddressPart(const juce::OSCMessage& m)
{
	StringArray arr = StringArray::fromTokens(m.getAddressPattern().toString(), "/", "\"");
	return arr.isEmpty() ? "" : arr[arr.size() - 1];
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

ControllableContainer* OSCHelpers::findParentContainer(ControllableContainer* root, const juce::String &address, int dataOffset)
{

	if (root == nullptr) return nullptr;
	StringArray addSplit;
	addSplit.addTokens(address, "/", "\"");
	if(address.startsWith("/")) addSplit.remove(0); //remove first empty string
	addSplit.remove(addSplit.size() - 1);
	
	if (addSplit.isEmpty()) return Engine::mainEngine;

	return root->getControllableContainerForAddress(addSplit, true);
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


// OSCQUERY

void OSCQueryHelpers::updateContainerFromData(ControllableContainer* cc, var data, bool useAddressForNaming, std::function<OSCQueryValueContainer* (const String& name)> createContainerFunc)
{
	DynamicObject* dataObject = data.getProperty("CONTENTS", var()).getDynamicObject();

	Array<WeakReference<ControllableContainer>> containersToDelete = cc->getAllContainers();
	Array<WeakReference<Controllable>> controllablesToDelete = cc->getAllControllables();

	bool syncContent = true;
	if (OSCQueryHelpers::OSCQueryValueContainer* vc = dynamic_cast<OSCQueryHelpers::OSCQueryValueContainer*>(cc))
	{
		controllablesToDelete.removeAllInstancesOf(vc->enableListen);
		controllablesToDelete.removeAllInstancesOf(vc->syncContent);
		syncContent = vc->syncContent != nullptr ? vc->syncContent->boolValue() : true;
	}

	if (syncContent && dataObject != nullptr)
	{
		NamedValueSet nvSet = dataObject->getProperties();
		for (auto& nv : nvSet)
		{
			//int access = nv.value.getProperty("ACCESS", 1);
			bool isGroup = /*access == 0 || */nv.value.hasProperty("CONTENTS");
			if (isGroup) //group
			{
				String ccNiceName;
				if (!useAddressForNaming) ccNiceName = nv.value.getProperty("DESCRIPTION", "");
				if (ccNiceName.isEmpty()) ccNiceName = nv.name.toString();

				OSCQueryHelpers::OSCQueryValueContainer* childCC = dynamic_cast<OSCQueryHelpers::OSCQueryValueContainer*>(cc->getControllableContainerByName(ccNiceName, true));

				if (childCC == nullptr)
				{
					childCC = createContainerFunc != nullptr ? createContainerFunc(ccNiceName) : new OSCQueryHelpers::OSCQueryValueContainer(ccNiceName);
					childCC->saveAndLoadRecursiveData = true;
					childCC->setCustomShortName(nv.name.toString());
					childCC->editorIsCollapsed = true;
				}
				else
				{
					containersToDelete.removeAllInstancesOf(childCC);
					childCC->setNiceName(ccNiceName);
				}

				updateContainerFromData(childCC, nv.value, useAddressForNaming, createContainerFunc);

				if (childCC->parentContainer != cc) cc->addChildControllableContainer(childCC, true);
			}
			else
			{
				Controllable* c = cc->getControllableByName(nv.name.toString());
				if (c != nullptr) controllablesToDelete.removeAllInstancesOf(c);
				createOrUpdateControllableFromData(cc, c, nv.name, nv.value, useAddressForNaming);
			}
		}
	}

	for (auto& cd : controllablesToDelete) cc->removeControllable(cd);
	for (auto& ccd : containersToDelete) cc->removeChildControllableContainer(ccd);
}

void OSCQueryHelpers::createOrUpdateControllableFromData(ControllableContainer* parentCC, Controllable* sourceC, StringRef name, var data, bool useAddressForNaming)
{
	var val = data.getProperty("VALUE", var());

	if (name == StringRef("enabled"))
	{
		if (EnablingControllableContainer* ecc = dynamic_cast<EnablingControllableContainer*>(parentCC))
		{
			ecc->setCanBeDisabled(true);
			if (val.size() > 0) ecc->enabled->setValue(val[0]);
			return;
		}
	}

	Controllable* c = sourceC;

	String cNiceName;
	if (!useAddressForNaming) cNiceName = data.getProperty("DESCRIPTION", "");
	if (cNiceName.isEmpty()) cNiceName = name;

	String type = data.getProperty("TYPE", "").toString();
	var valRange = data.hasProperty("RANGE") ? data.getProperty("RANGE", var()) : var();
	int access = data.getProperty("ACCESS", 3);

	var value;
	var range;

	if (val.isArray()) value = val;
	else value.append(val);
	if (valRange.isArray()) range = valRange;
	else range.append(valRange);

	if (range.size() != value.size())
	{
		//DBG("Not the same : " << range.size() << " / " << value.size() << "\n" << data.toString());
		//NLOGWARNING(niceName, "RANGE and VALUE fields don't have the same size, skipping : " << cNiceName);
	}
	var minVal;
	var maxVal;
	for (int i = 0; i < range.size(); ++i)
	{
		minVal.append(range[i].getProperty("MIN", INT32_MIN));
		maxVal.append(range[i].getProperty("MAX", INT32_MAX));
	}

	Controllable::Type targetType = Controllable::Type::CUSTOM;

	if (type == "N" || type == "I") targetType = Controllable::TRIGGER;
	else if (type == "i" || type == "h") targetType = Controllable::INT;
	else if (type == "f" || type == "d") targetType = Controllable::FLOAT;
	else if (type == "ii" || type == "ff" || type == "hh" || type == "dd") targetType = Controllable::POINT2D;
	else if (type == "iii" || type == "fff" || type == "hhh" || type == "ddd") targetType = Controllable::POINT3D;
	else if (type == "ffff" || type == "dddd" || type == "iiii" || type == "hhhh" || type == "r") targetType = Controllable::COLOR;
	else if (type == "s" || type == "S" || type == "c")
	{
		if (range[0].isObject()) targetType = Controllable::ENUM;
		else targetType = Controllable::STRING;
	}
	else if (type == "T" || type == "F") targetType = Controllable::BOOL;


	if (c != nullptr && targetType != c->type)
	{
		parentCC->removeControllable(c);
		c = nullptr;
	}

	bool addToContainer = c == nullptr;

	switch (targetType)
	{
	case Controllable::TRIGGER:
	{
		if (c == nullptr) c = new Trigger(cNiceName, cNiceName);
	}
	break;

	case Controllable::INT:
	{
		if (c == nullptr) c = new IntParameter(cNiceName, cNiceName, value[0], minVal[0], maxVal[0]);
		else
		{
			((Parameter*)c)->setValue(value[0]);
			((Parameter*)c)->setRange(minVal[0], maxVal[0]);
		}
	}
	break;

	case Controllable::FLOAT:
	{
		if (c == nullptr) c = new FloatParameter(cNiceName, cNiceName, value[0], minVal[0], maxVal[0]);
		else
		{
			((Parameter*)c)->setValue(value[0]);
			((Parameter*)c)->setRange(minVal[0], maxVal[0]);
		}
	}
	break;

	case Controllable::POINT2D:
	{
		if (value.isVoid()) for (int i = 0; i < 2; ++i) value.append(0);
		if (c == nullptr) c = new Point2DParameter(cNiceName, cNiceName);
		if (value.size() >= 2) ((Point2DParameter*)c)->setValue(value);
		if (range.size() >= 2) ((Point2DParameter*)c)->setRange(minVal, maxVal);
	}
	break;

	case Controllable::POINT3D:
	{
		if (value.isVoid()) for (int i = 0; i < 3; ++i) value.append(0);
		if (c == nullptr) c = new Point3DParameter(cNiceName, cNiceName);
		if (value.size() >= 3) ((Point3DParameter*)c)->setValue(value);
		if (range.size() >= 3) ((Point3DParameter*)c)->setRange(minVal, maxVal);
	}
	break;

	case Controllable::COLOR:
	{
		Colour col = Colours::black;
		if (type == "ffff" || type == "dddd") col = value.size() >= 4 ? Colour::fromFloatRGBA(value[0], value[1], value[2], value[3]) : Colours::black;
		else if (type == "iiii" || type == "hhhh") col = value.size() >= 4 ? Colour::fromRGBA((int)value[0], (int)value[1], (int)value[2], (int)value[3]) : Colours::black;
		else if (type == "r")
		{
			// do not use Colour::fromString that parses a #ARGB color string while OSC "r" type is a RGBA format.
			const uint32 intValue = CharacterFunctions::HexParser<uint32>::parse(value[0].toString().getCharPointer());
			const OSCColour oscColor = OSCColour::fromInt32(intValue);
			col = Colour::fromRGBA(oscColor.red, oscColor.green, oscColor.blue, oscColor.alpha);
		}
		if (c == nullptr)  c = new ColorParameter(cNiceName, cNiceName, col);
		else ((ColorParameter*)c)->setColor(col);
	}
	break;

	case Controllable::STRING:
	{
		if (c == nullptr) c = new StringParameter(cNiceName, cNiceName, value[0]);
		else ((StringParameter*)c)->setValue(value[0]);
	}
	break;

	case Controllable::ENUM:
	{
		var options = range[0].getProperty("VALS", var());
		if (options.isArray())
		{
			if (c == nullptr) c = new EnumParameter(cNiceName, cNiceName);
			EnumParameter* ep = (EnumParameter*)c;
			ep->clearOptions();
			for (int i = 0; i < options.size(); ++i) ep->addOption(options[i], options[i], false);
			ep->setValueWithKey(value[0]);
		}
	}
	break;

	case Controllable::BOOL:
	{
		if (c == nullptr) c = new BoolParameter(cNiceName, cNiceName, value[0]);
		else ((BoolParameter*)c)->setValue(value[0]);
	}
	break;
	default:
		break;
	}

	if (c != nullptr)
	{
		c->setNiceName(cNiceName);
		c->setCustomShortName(name);
		if (access == 1) c->setControllableFeedbackOnly(true);
		if (addToContainer) parentCC->addControllable(c);
	}

}


OSCQueryHelpers::OSCQueryValueContainer::OSCQueryValueContainer(const String& name, bool canListen, bool canSync) :
	EnablingControllableContainer(name, false),
	enableListen(nullptr),
	syncContent(nullptr)
{
	if (canListen)
	{
		enableListen = addBoolParameter("Listen", "This will activate listening to this container", false);
		enableListen->hideInEditor = true;
	}

	if (canSync)
	{

		syncContent = addBoolParameter("Sync", "This will activate syncing on this container", true);
		syncContent->hideInEditor = true;
	}
}

OSCQueryHelpers::OSCQueryValueContainer::~OSCQueryValueContainer()
{
}

InspectableEditor* OSCQueryHelpers::OSCQueryValueContainer::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new OSCQueryValueContainerEditor(this, isRoot);
}


OSCQueryHelpers::OSCQueryValueContainerEditor::OSCQueryValueContainerEditor(OSCQueryValueContainer* cc, bool isRoot) :
	EnablingControllableContainerEditor(cc, isRoot)
{
	if (cc->enableListen != nullptr)
	{
		enableListenUI.reset(cc->enableListen->createToggle());
		addAndMakeVisible(enableListenUI.get());
	}

	if (cc->syncContent != nullptr)
	{
		syncUI.reset(cc->syncContent->createToggle());
		addAndMakeVisible(syncUI.get());
	}
}

OSCQueryHelpers::OSCQueryValueContainerEditor::~OSCQueryValueContainerEditor()
{
}

void OSCQueryHelpers::OSCQueryValueContainerEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	EnablingControllableContainerEditor::resizedInternalHeader(r);
	if (enableListenUI != nullptr)
	{

		enableListenUI->setVisible(container->controllables.size() > 1);
		enableListenUI->setBounds(r.removeFromRight(60).reduced(3));
	}

	if (syncUI != nullptr)
	{
		if (enableListenUI != nullptr) r.removeFromRight(16);
		syncUI->setVisible(container->controllables.size() > 1);
		syncUI->setBounds(r.removeFromRight(60).reduced(3));
	}
}