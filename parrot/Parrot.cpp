/*
  ==============================================================================

	Parrot.cpp
	Created: 4 Oct 2021 12:51:30pm
	Author:  bkupe

  ==============================================================================
*/

Parrot::Parrot() :
	BaseItem(getTypeString()),
	Thread("Parrot"),
	targetsCC("Targets"),
	currentRecord(nullptr),
	timeAtRecord(0)
{
	currentRecordEnum = addEnumParameter("Current Record", "The record to use for recording/playing");
	status = addEnumParameter("Status", "The status of this parrot");
	status->addOption("Idle", IDLE)->addOption("Recording", RECORDING)->addOption("Playing", PLAYING);
	status->setControllableFeedbackOnly(true);

	startRecordTrigger = addTrigger("Start Record", "");
	stopRecordTrigger = addTrigger("Stop Record", "");
	playTrigger = addTrigger("Play", "");
	pauseTrigger = addTrigger("Pause", "");
	stopTrigger = addTrigger("Stop", "");

	trimToFirstData = addBoolParameter("Trim To First Data", "If checked, this will shift all timings so it starts at the first animated parameter key.", false);
	trimToLastData = addBoolParameter("Trim to Last Data", "If checked, this will clip the length of this record to the last recorded data time", false);
	forceValueAtStartRecord = addBoolParameter("Force Start Value", "If checked, this will store a key for every parameter when starting recording", true);

	playProgression = addFloatParameter("Play Progression", "This is the time of the record playback", 0, 0);
	playProgression->setControllableFeedbackOnly(true);

	loop = addBoolParameter("Loop", "Loop indefinitely the playback", false);

	targetsCC.userCanAddControllables = true;
	targetsCC.userAddControllablesFilters.add(TargetParameter::getTypeStringStatic());
	addChildControllableContainer(&targetsCC);
	addChildControllableContainer(&recordManager);

	recordManager.addBaseManagerListener(this);

	updateControls();
}

Parrot::~Parrot()
{
	stopThread(1000);
	recordManager.removeBaseManagerListener(this);
}

void Parrot::setCurrentRecord(ParrotRecord* r)
{
	if (r == currentRecord) return;

	stopPlaying();
	if (currentRecord != nullptr)
	{
		Array<Controllable*> targets = getAllTargets();
		for (auto& c : targets)
		{
			c->isControlledByParrot = false;
			c->notifyStateChanged(); //force repaint
		}

		dataMapPlayIndex.clear();
	}

	currentRecord = r;

	if (currentRecord != nullptr)
	{
		resetDataPlayIndices();
		playProgression->setRange(0, currentRecord->totalTime);
	}

	playProgression->setValue(0);
	updateControls();
}

void Parrot::updateRecordOptions()
{
	currentRecordEnum->clearOptions();
	int index = 0;
	for (auto& i : recordManager.items) currentRecordEnum->addOption(i->niceName, index++, false);
	if (currentRecord != nullptr) currentRecordEnum->setValueWithKey(currentRecord->niceName);
}

double Parrot::getRelativeRecordTime()
{
	return Time::getMillisecondCounterHiRes() / 1000.0 - timeAtRecord;
}

Array<Controllable*> Parrot::getAllTargets()
{
	Array<Controllable*> result;
	for (auto& c : targetsCC.controllables)  if (((TargetParameter*)c)->target != nullptr) result.add(((TargetParameter*)c)->target);
	return result;
}

void Parrot::resetDataPlayIndices()
{
	dataMapPlayIndex.clear();

	if (currentRecord == nullptr) return;

	Array<Controllable*> targets = getAllTargets();
	for (auto& t : targets) dataMapPlayIndex.set(t, 0);
}

void Parrot::updateControls()
{
	Status s = status->getValueDataAsEnum<Status>();

	startRecordTrigger->setEnabled(s == IDLE);
	stopRecordTrigger->setEnabled(s == RECORDING);
	playTrigger->setEnabled(s == IDLE && currentRecord != nullptr);
	pauseTrigger->setEnabled(s == PLAYING);
	stopTrigger->setEnabled(s == PLAYING);
}

TargetParameter* Parrot::addTarget(Controllable* c)
{
	if (c != nullptr && c->isControllableFeedbackOnly)
	{
		NLOGWARNING(niceName, "This parameter is read only, so it can't be parroted");
		return nullptr;
	}

	if (TargetParameter* t = getTargetForControllable(c)) return t;

	TargetParameter* t = targetsCC.addTargetParameter(targetsCC.getUniqueNameInContainer("Target"), "");
	t->isRemovableByUser = true;
	t->setValueFromTarget(c);
	return t;
}

TargetParameter* Parrot::getTargetForControllable(Controllable* c)
{
	if (c == nullptr) return nullptr;

	for (auto& t : targetsCC.controllables)
	{
		if (((TargetParameter*)t)->target == c) return (TargetParameter*)t;
	}
	return nullptr;
}

void Parrot::removeTargetForControllable(Controllable* c)
{
	for (auto& t : targetsCC.controllables)
	{
		if (((TargetParameter*)t)->target == c)
		{
			targetsCC.removeControllable(t);
			return;
		}
	}
}

void Parrot::onContainerParameterChangedInternal(Parameter* p)
{
	BaseItem::onContainerParameterChangedInternal(p);
	if (p == playProgression)
	{
		processControllables();
	}
	else if (p == currentRecordEnum)
	{
		int rIndex = (int)currentRecordEnum->getValue();
		if (rIndex >= 0 && rIndex < recordManager.items.size()) setCurrentRecord(recordManager.items[rIndex]);
		else setCurrentRecord(nullptr);
	}
	else if (p == status)
	{
		Status s = status->getValueDataAsEnum<Status>();
		Array<Controllable*> targets = getAllTargets();

		if (s == RECORDING)
		{
			for (auto& c : targets)
			{
				if (c->type == Controllable::TRIGGER) ((Trigger*)c)->addTriggerListener(this);
				else ((Parameter*)c)->addParameterListener(this);
			}
		}
		else
		{
			for (auto& c : targets)
			{
				if (c->type == Controllable::TRIGGER) ((Trigger*)c)->removeTriggerListener(this);
				else ((Parameter*)c)->removeParameterListener(this);
			}
		}

		updateControls();
	}
}

void Parrot::onContainerTriggerTriggered(Trigger* t)
{
	BaseItem::onContainerTriggerTriggered(t);
	if (t == startRecordTrigger) startRecording();
	else if (t == stopRecordTrigger) stopRecording();
	else if (t == playTrigger) startPlaying();
	else if (t == pauseTrigger) pausePlaying();
	else if (t == stopTrigger) stopPlaying();
}

void Parrot::onExternalTriggerTriggered(Trigger* t)
{
	Status s = status->getValueDataAsEnum<Status>();
	if (s == RECORDING && currentRecord != nullptr)
	{
		if (currentRecord->dataMap.size() == 0 && trimToFirstData->boolValue()) timeAtRecord = Time::getMillisecondCounterHiRes() / 1000.0;
		currentRecord->addData(t, getRelativeRecordTime());
	}
}

void Parrot::onExternalParameterValueChanged(Parameter* p)
{
	Status s = status->getValueDataAsEnum<Status>();
	if (s == RECORDING && currentRecord != nullptr)
	{
		if (currentRecord->dataMap.size() == 0 && trimToFirstData->boolValue()) timeAtRecord = Time::getMillisecondCounterHiRes() / 1000.0;
		currentRecord->addData(p, getRelativeRecordTime(), p->value);
	}
}

void Parrot::itemAdded(ParrotRecord* r)
{
	if (isCurrentlyLoadingData) return;
	if (recordManager.items.size() == 1) setCurrentRecord(r);
	updateRecordOptions();
}

void Parrot::itemRemoved(ParrotRecord* r)
{
	if (currentRecord == r) setCurrentRecord(nullptr);
	updateRecordOptions();
}

void Parrot::startRecording()
{
	if (currentRecord == nullptr)
	{
		if (recordManager.items.size() == 0) recordManager.addItem();
		else
		{
			NLOGWARNING(niceName, "Can't start recording, Current Record is not set");
			return;
		}
	}

	currentRecord->clearRecord();
	timeAtRecord = Time::getMillisecondCounterHiRes() / 1000.0;
	status->setValueWithData(RECORDING);
	Array<Controllable*> targets = getAllTargets();
	if (forceValueAtStartRecord->boolValue())
	{
		for (auto& c : targets) if(c->type != Controllable::TRIGGER) currentRecord->addData(c, 0, ((Parameter*)c)->value);
	}
}

void Parrot::stopRecording()
{
	status->setValueWithData(IDLE);
	if (currentRecord == nullptr) return;
	if (!trimToLastData->boolValue()) currentRecord->totalTime = getRelativeRecordTime();
	playProgression->setRange(0, currentRecord->totalTime);

}

void Parrot::startPlaying()
{
	status->setValueWithData(PLAYING);
	Array<Controllable*> targets = getAllTargets();
	for (auto& c : targets)
	{
		c->isControlledByParrot = true;
		c->notifyStateChanged(); //force repaint
	}
	startThread();
}

void Parrot::pausePlaying()
{
	stopThread(1000);
	status->setValueWithData(IDLE);
	Array<Controllable*> targets = getAllTargets();
	for (auto& c : targets)
	{
		c->isControlledByParrot = false;
		c->notifyStateChanged(); //force repaint
	}
}

void Parrot::stopPlaying()
{
	stopThread(1000);
	playProgression->setValue(0);
	resetDataPlayIndices();
	status->setValueWithData(IDLE);

	Array<Controllable*> targets = getAllTargets();
	for (auto& c : targets)
	{
		c->isControlledByParrot = false;
		c->notifyStateChanged(); //force repaint
	}
}

void Parrot::run()
{
	double timeAtLastUpdate = Time::getMillisecondCounterHiRes() / 1000.0;
	while (!threadShouldExit())
	{
		if (currentRecord == nullptr) break;

		double curTime = Time::getMillisecondCounterHiRes() / 1000.0;
		double delta = curTime - timeAtLastUpdate;
		float progression = playProgression->floatValue() + delta;
		if (progression > currentRecord->totalTime)
		{
			resetDataPlayIndices();

			if (loop->boolValue())
			{
				progression = fmodf(progression, currentRecord->totalTime);
			}
			else
			{
				playProgression->setValue(0);
				break;
			}
		}

		playProgression->setValue(progression);

		timeAtLastUpdate = curTime;
		wait(10); //100 fps for now
	}

	status->setValueWithData(IDLE);
}

void Parrot::processControllables()
{
	if (currentRecord == nullptr) return;

	float curTime = playProgression->floatValue();

	Array<Controllable*> targets = getAllTargets();
	for (auto& c : targets) {
		Array<ParrotRecord::RecordValue> vals = currentRecord->dataMap[c];
		int nextIndex = dataMapPlayIndex[c];
		//LOG(c->niceName << " : " << it.getValue()->size() << " keys");
		while (nextIndex < vals.size())
		{
			ParrotRecord::RecordValue rv = vals[nextIndex];
			if (rv.time <= curTime)
			{
				processControllable(c, rv.value);
				nextIndex++;
			}
			else break;
		}

		dataMapPlayIndex.set(c, nextIndex);
	}
}

void Parrot::processControllable(Controllable* c, var val)
{
	if (c->type == Controllable::TRIGGER) ((Trigger*)c)->trigger();
	else ((Parameter*)c)->setValue(val);
}

var Parrot::getJSONData()
{
	var data = BaseItem::getJSONData();

	var targetData;
	for (auto& c : targetsCC.controllables)
	{
		TargetParameter* t = (TargetParameter*)c;
		targetData.append(t->value);
	}

	data.getDynamicObject()->setProperty("targets", targetData);

	data.getDynamicObject()->setProperty(recordManager.shortName, recordManager.getJSONData());
	return data;
}

void Parrot::loadJSONDataItemInternal(var data)
{
	var tData = data.getProperty("targets", var());
	for (int i = 0; i < tData.size(); i++)
	{
		Controllable* c = Engine::mainEngine->getControllableForAddress(tData[i]);
		addTarget(c);
	}

	recordManager.loadJSONData(data.getProperty(recordManager.shortName, var()));

	updateRecordOptions();
	int rIndex = (int)currentRecordEnum->getValue();
	if (rIndex >= 0 && rIndex < recordManager.items.size()) setCurrentRecord(recordManager.items[rIndex]);
	else setCurrentRecord(nullptr);
}
