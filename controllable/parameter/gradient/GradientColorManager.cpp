/*
  ==============================================================================

	GradientColorManager.cpp
	Created: 11 Apr 2017 11:40:41am
	Author:  Ben

  ==============================================================================
*/

GradientColorComparator GradientColorManager::comparator;

GradientColorManager::GradientColorManager(float maxPosition, bool addDefaultColors, bool dedicatedSelectionManager) :
	BaseManager("Colors"),
	allowKeysOutside(true)
{

	itemDataType = "GradientColor";

	editorIsCollapsed = true;
	selectItemWhenCreated = false;
	isSelectable = false;

	position = addFloatParameter("Position", "Position in the gradient", 0, 0, maxPosition);
	position->isSavable = false;
	position->hideInEditor = true;

	length = addFloatParameter("Length", "Length of the gradient", maxPosition, 0, INT32_MAX);

	currentColor = new ColorParameter("Color", "Current color depending on time", Colours::black);
	currentColor->isSavable = false;
	currentColor->setControllableFeedbackOnly(true);
	addParameter(currentColor);

	if (dedicatedSelectionManager)
	{
		customSelectionManager.reset(new InspectableSelectionManager(false));
		showInspectorOnSelect = false;
		selectionManager = customSelectionManager.get();
	}

	if (addDefaultColors)
	{
		addColorAt(0, Colours::green);
		addColorAt(1, Colours::yellow);
	}

	updateCurrentColor();

	scriptObject.getDynamicObject()->setMethod("getColorAtPosition", &GradientColorManager::getColorAtPositionFromScript);
	scriptObject.getDynamicObject()->setMethod("getKeyAtPosition", &GradientColorManager::getKeyAtPositionFromScript);
	scriptObject.getDynamicObject()->setMethod("getKeysBetween", &GradientColorManager::getKeysBetweenFromScript);
}

GradientColorManager::~GradientColorManager()
{
}

void GradientColorManager::setLength(float val, bool stretch, bool stickToEnd)
{
	updateKeyRanges();
	
	if (stretch)
	{
		float stretchFactor = val / length->floatValue();
		if (stretchFactor > 1) length->setValue(val); //if stretching, we have first to expand the length in case keys are not allowed outside
		for (auto& k : items) k->position->setValue(k->position->floatValue() * stretchFactor);
		if (stretchFactor < 1) length->setValue(val); //if reducing, we have to first reduce keys and then we can reduce the length, in case keys are not allowed outside
	}
	else
	{
		float lengthDiff = val - length->floatValue();
		length->setValue(val); // just change the value, nothing unusual
		if (stickToEnd) for (auto& k : items) k->position->setValue(k->position->floatValue() + lengthDiff);
	}

}

void GradientColorManager::setAllowKeysOutside(bool value)
{
	if (allowKeysOutside == value) return;
	allowKeysOutside = value;
	updateKeyRanges();
}

void GradientColorManager::updateKeyRanges()
{
	for (auto& i : items)
	{
		if (!allowKeysOutside) i->position->setRange(0, length->floatValue());
		else i->position->clearRange();
	}
}

void GradientColorManager::updateCurrentColor()
{
	currentColor->setColor(getColorForPosition(position->floatValue()));
}

Colour GradientColorManager::getColorForPosition(const float & time) const
{
	if (items.isEmpty()) return Colours::transparentBlack;
	if (time <= items[0]->position->floatValue()) return items[0]->itemColor->getColor();
	if (time >= items[items.size() - 1]->position->floatValue()) return items[items.size() - 1]->itemColor->getColor();

	GradientColor * nearest = getItemAt(time, true);

	if (nearest == nullptr) return Colours::purple;

	GradientColor::Interpolation interpolation = nearest->interpolation->getValueDataAsEnum<GradientColor::Interpolation>();

	switch (interpolation)
	{
	case GradientColor::NONE:
		return nearest->itemColor->getColor();
		break;

	case GradientColor::LINEAR:
	{
		GradientColor * next = items[items.indexOf(nearest) + 1];
		if (nearest == nullptr || next == nullptr || nearest->position->floatValue() == next->position->floatValue()) return nearest->itemColor->getColor();
		return nearest->itemColor->getColor().interpolatedWith(next->itemColor->getColor(), jmap<float>(time, nearest->position->floatValue(), next->position->floatValue(), 0, 1));
	}
	break;
	}

	return Colours::purple;
}

/*
void GradientColorManager::rebuildGradient()
{
	gradientLock.enter();
	gradient.clearColours();
	if (items.size() > 0 && items[0]->position->floatValue() > 0) gradient.addColour(0, items[0]->color->getColor(), items[0]->interpolation->getValueDataAsEnum<ColourGradient::Interpolation>());

	for (auto &i : items)
	{
		i->gradientIndex = gradient.addColour(jlimit<float>(0,1,i->position->floatValue() / length->floatValue()), i->color->getColor(), i->interpolation->getValueDataAsEnum<ColourGradient::Interpolation>());
	}
	gradientLock.exit();

	updateCurrentColor();
	colorManagerListeners.call(&GradientColorManagerListener::gradientUpdated);
}
*/

GradientColor * GradientColorManager::addColorAt(float time, Colour color)
{
	if (items.isEmpty())  color = color.withAlpha(1.0f); //if only one color, force a non-transparent one to avoid confusion
	GradientColor * t = getItemAt(time);
	if (t == nullptr)
	{
		t = new GradientColor(time, color);
		BaseManager::addItem(t);
	}
	else
	{
		t->itemColor->setColor(color);
	}
	
	reorderItems();
	//rebuildGradient();
	return t;
}

GradientColor * GradientColorManager::getItemAt(float time, bool getNearestPreviousKeyIfNotFound) const
{
	GradientColor * nearestPrevious = nullptr;
	for (auto &t : items)
	{
		if (t->position->floatValue() == time) return t;
		if (t->position->floatValue() > time) break; //avoid looking for further keys, todo : implement dichotomy mechanism
		nearestPrevious = t;
	}

	return getNearestPreviousKeyIfNotFound ? nearestPrevious : nullptr;
}
Array<GradientColor*> GradientColorManager::getItemsInTimespan(float startTime, float endTime)
{
	Array<GradientColor*> result;

	for (auto& gc : items)
	{
		if (gc->position->floatValue() >= startTime && gc->position->floatValue() <= endTime)
		{
			result.add(gc);
		}
	}
	return result;
}

Array<UndoableAction*> GradientColorManager::getMoveKeysBy(float start, float offset)
{
	Array<UndoableAction*> actions;
	Array<GradientColor*> triggers = getItemsInTimespan(start, length->floatValue());
	for (auto& t : triggers) actions.add(t->position->setUndoableValue(t->position->floatValue(), t->position->floatValue() + offset, true));
	return actions;
}

Array<UndoableAction*> GradientColorManager::getRemoveTimespan(float start, float end)
{
	Array<UndoableAction*> actions;
	Array<GradientColor*> triggers = getItemsInTimespan(start, end);
	actions.addArray(getRemoveItemsUndoableAction(triggers));
	actions.addArray(getMoveKeysBy(end, start - end));
	return actions;
}



void GradientColorManager::addItemInternal(GradientColor * item, var data)
{
	//item->gradientIndex = gradient.addColour(item->position->floatValue() / length->floatValue(), item->itemColor->getColor());
	if(!allowKeysOutside) item->position->setRange(0, length->floatValue());
	item->selectionManager = selectionManager;
}

void GradientColorManager::removeItemInternal(GradientColor *)
{
	//rebuildGradient();
	updateCurrentColor();
}

Array<GradientColor*> GradientColorManager::addItemsFromClipboard(bool showWarning)
{
	Array<GradientColor*> keys = BaseManager::addItemsFromClipboard(showWarning);

	if (keys.isEmpty()) return keys;
	if (keys[0] == nullptr) return Array<GradientColor*>();

	float minTime = keys[0]->position->floatValue();
	for (auto& b : keys)
	{
		if (b->position->floatValue() < minTime)
		{
			minTime = b->position->floatValue();
		}
	}

	float diffTime = position->floatValue() - minTime;

	if (keys.size() > 1)
	{
		Array<GradientColor*> keysBetween = getItemsInTimespan(keys[0]->position->floatValue() + diffTime, keys[keys.size() - 1]->position->floatValue() + diffTime);
		if (keysBetween.size() > 0)
		{
			Array<GradientColor*> keysToRemove;
			for (auto& k : keysBetween) if (!keys.contains(k)) keysToRemove.add(k);
			removeItems(keysToRemove);
		}
	}

	for (auto& k : keys) k->position->setValue(k->position->floatValue() + diffTime);

	reorderItems();

	return keys;
}

void GradientColorManager::reorderItems()
{
	items.sort(GradientColorManager::comparator, true);
	BaseManager::reorderItems();
}

void GradientColorManager::onContainerParameterChanged(Parameter * p)
{
	if (p == position)
	{
		updateCurrentColor();
	}
}

void GradientColorManager::onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{
	GradientColor * t = static_cast<GradientColor *>(cc);
	if (t != nullptr)
	{
		if (c == t->position)
		{
			int index = items.indexOf(t);
			if (index > 0 && t->position->floatValue() < items[index - 1]->position->floatValue())
			{
				items.swap(index, index - 1);
				baseManagerListeners.call(&ManagerListener::itemsReordered);
			}
			else if (index < items.size() - 1 && t->position->floatValue() > items[index + 1]->position->floatValue())
			{
				items.swap(index, index + 1);
				baseManagerListeners.call(&ManagerListener::itemsReordered);
			}

		}
		else if (c == t->itemColor) {
			//gradient.setColour(t->gradientIndex, t->color->getColor());
			colorManagerListeners.call(&GradientColorManagerListener::gradientUpdated);
		}
		else if (c == t->interpolation)
		{
			//gradient.setInterpolation(t->gradientIndex, t->interpolation->getValueDataAsEnum<ColourGradient::Interpolation>());
			colorManagerListeners.call(&GradientColorManagerListener::gradientUpdated);
		}

		//rebuildGradient();
		updateCurrentColor();
	}
}

var GradientColorManager::getColorAtPositionFromScript(const juce::var::NativeFunctionArgs& a)
{
	GradientColorManager* au = getObjectFromJS<GradientColorManager>(a);
	if (!checkNumArgs(au->niceName, a, 1)) return var();
	Colour c = au->getColorForPosition(a.arguments[0]);
	var result;
	result.append(c.getFloatRed());
	result.append(c.getFloatGreen());
	result.append(c.getFloatBlue());
	result.append(c.getFloatAlpha());
	return result;
}

var GradientColorManager::getKeyAtPositionFromScript(const juce::var::NativeFunctionArgs& a)
{
	GradientColorManager* au = getObjectFromJS<GradientColorManager>(a);
	if (!checkNumArgs(au->niceName, a, 1)) return var();
	return au->getItemAt(a.arguments[0])->getScriptObject();
}

var GradientColorManager::getKeysBetweenFromScript(const juce::var::NativeFunctionArgs& a)
{
	GradientColorManager* au = getObjectFromJS<GradientColorManager>(a);
	if (!checkNumArgs(au->niceName, a, 2)) return var();
	var result = var();
	Array<GradientColor* > keys = au->getItemsInTimespan(a.arguments[0], a.arguments[1]);
	for (auto& k : keys) result.append(k->getScriptObject());
	return result;
}


void GradientColorManager::loadJSONDataInternal(var data)
{
	BaseManager::loadJSONDataInternal(data);
	//updateKeyRanges();
	//rebuildGradient();
}

InspectableEditor* GradientColorManager::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new GradientColorManagerEditor(this, isRoot);
}

