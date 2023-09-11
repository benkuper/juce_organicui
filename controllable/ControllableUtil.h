#pragma once

class ControllableUtil
{
public:
	static juce::var  createDataForParam(const juce::String& type, const juce::String& name, const juce::String & description, juce::var value, juce::var minVal = INT32_MIN, juce::var maxVal = INT32_MAX, bool editable = true, bool hiddenInEditor = false)
	{
		juce::var v = juce::var(new juce::DynamicObject());
		v.getDynamicObject()->setProperty("type", type);
		v.getDynamicObject()->setProperty("niceName", name);
		v.getDynamicObject()->setProperty("description", description);
		v.getDynamicObject()->setProperty("value", value);
		v.getDynamicObject()->setProperty("minValue", minVal);
		v.getDynamicObject()->setProperty("maxValue", maxVal);
		v.getDynamicObject()->setProperty("editable", editable);
		v.getDynamicObject()->setProperty("hiddenInEditor", hiddenInEditor);
		return v;
	}

	//Helpers
	template<class T>
	static T * findParentAs(juce::WeakReference<Controllable> c, int maxLevel = -1)
	{
		int curLevel = 0;
		if (c == nullptr || c.wasObjectDeleted()) return nullptr;

		juce::WeakReference<ControllableContainer> cc = c->parentContainer;

		if (cc == nullptr || cc.wasObjectDeleted()) return nullptr;
		T * result = dynamic_cast<T *>(cc.get());

		while (result == nullptr && cc != nullptr)
		{
			cc = cc->parentContainer;
			
			if (cc == nullptr || cc.wasObjectDeleted()) return nullptr;

			result = dynamic_cast<T *>(cc.get());
			curLevel++;
			if (maxLevel != -1 && curLevel > maxLevel) return nullptr;
		}

		return result;
	}

	template<class T>
	static T* findParentAs(juce::WeakReference<ControllableContainer> _cc, int maxLevel = -1)
	{
		int curLevel = 0;
		if (_cc == nullptr || _cc.wasObjectDeleted()) return nullptr;

		juce::WeakReference<ControllableContainer> cc = _cc->parentContainer;

		if (cc == nullptr || cc.wasObjectDeleted()) return nullptr;
		T* result = dynamic_cast<T*>(cc.get());

		while (result == nullptr && cc != nullptr)
		{
			cc = cc->parentContainer;

			if (cc == nullptr || cc.wasObjectDeleted()) return nullptr;

			result = dynamic_cast<T*>(cc.get());
			curLevel++;
			if (maxLevel != -1 && curLevel > maxLevel) return nullptr;
		}

		return result;
	}
};
