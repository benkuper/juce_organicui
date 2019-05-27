#pragma once

class ControllableUtil
{
public:
	static var  createDataForParam(const String & type, const String & name, const String & description, var value, var minVal = INT32_MIN, var maxVal = INT32_MAX, bool editable = true, bool hiddenInEditor = false)
	{
		var v = var(new DynamicObject());
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
	static T * findParentAs(WeakReference<Controllable> c, int maxLevel = -1)
	{
		int curLevel = 0;
		if (c == nullptr || c.wasObjectDeleted()) return nullptr;

		WeakReference<ControllableContainer> cc = c->parentContainer;

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
	static T* findParentAs(WeakReference<ControllableContainer> _cc, int maxLevel = -1)
	{
		int curLevel = 0;
		if (c == nullptr || c.wasObjectDeleted()) return nullptr;

		WeakReference<ControllableContainer> cc = _cc->parentContainer;

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
