/*
  ==============================================================================

    ParrotRecord.cpp
    Created: 4 Oct 2021 12:51:48pm
    Author:  bkupe

  ==============================================================================
*/


ParrotRecord::ParrotRecord() :
    BaseItem(getTypeString()),
    totalTime(0)
{
   // showInspectorOnSelect = false;
}

ParrotRecord::~ParrotRecord()
{
}

void ParrotRecord::clearRecord()
{
    dataMap.clear();
    totalTime = 0;
}

void ParrotRecord::addData(Controllable* c, double time, var value)
{
    if (!dataMap.contains(c)) dataMap.set(c, Array<RecordValue>());
    dataMap.getReference(c).add({ time, value });
    totalTime = jmax(time, totalTime);
}

var ParrotRecord::getJSONData()
{
    var data = BaseItem::getJSONData();
    var vData = new DynamicObject();
    HashMap<Controllable*, Array<RecordValue>>::Iterator it(dataMap);
    while (it.next())
    {
        var vValues;
        Array<RecordValue> values = it.getValue();
        for (auto& v : values)
        {
            var val;
            val.append(v.time);
            val.append(v.value);
            vValues.append(val);
        }
        vData.getDynamicObject()->setProperty(it.getKey()->getControlAddress(), vValues);
    }

    data.getDynamicObject()->setProperty("data", vData);
    data.getDynamicObject()->setProperty("totalTime", totalTime);
    return data;
}

void ParrotRecord::loadJSONDataItemInternal(var data)
{
    clearRecord();
    var dataVar = data.getProperty("data", var());
    if (dataVar.isObject())
    {
        NamedValueSet nv = dataVar.getDynamicObject()->getProperties();
        for (auto& n : nv)
        {
            if (Controllable* c = Engine::mainEngine->getControllableForAddress(n.name.toString()))
            {
                for (int i = 0; i < n.value.size(); i++)
                {
                    addData(c, n.value[i][0], n.value[i][1]);
                }
            }
        }
    }

    totalTime = data.getProperty("totalTime", totalTime);
}
