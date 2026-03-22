/*
  ==============================================================================

	EnumParameterUI.cpp
	Created: 29 Sep 2016 5:35:12pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

EnumParameterUIBase::EnumParameterUIBase(Array<EnumParameter*> parameters) :
	ParameterUI(Inspectable::getArrayAs<EnumParameter, Parameter>(parameters)),
	eps(parameters),
	ep(parameters[0])
{
	ep->addAsyncEnumParameterListener(this);
	showEditWindowOnDoubleClick = false;
}

EnumParameterUIBase::~EnumParameterUIBase()
{
	if (parameter != nullptr && !parameter.wasObjectDeleted()) ep->removeAsyncEnumParameterListener(this);
}

void EnumParameterUIBase::addPopupMenuItemsInternal(PopupMenu* p)
{
	p->addItem(101, "Set Options...");
}

void EnumParameterUIBase::handleMenuSelectedID(int result)
{
	ParameterUI::handleMenuSelectedID(result);
	if (result == 101)
	{
		std::unique_ptr<Component> editComponent(new EnumOptionManager(ep));
		CallOutBox* box = &CallOutBox::launchAsynchronously(std::move(editComponent), localAreaToGlobal(getLocalBounds()), nullptr);
		box->setArrowSize(8);
	}
}


void EnumParameterUIBase::newMessage(const EnumParameter::EnumParameterEvent& e)
{
	updateFromParameter();
}



EnumParameterUI::EnumParameterUI(Array<EnumParameter*> parameters) :
	EnumParameterUIBase(parameters)
{
	cb.addListener(this);
	cb.setTextWhenNoChoicesAvailable("");
	cb.setTextWhenNothingSelected("Select an element");
	cb.setTooltip(tooltip);
	addAndMakeVisible(cb);

	cb.addMouseListener(this, true);

	prevValue = ep->getValueKey();


	updateUIParams();
}

EnumParameterUI::~EnumParameterUI()
{
	//cb.removeListener(this);
}

void EnumParameterUI::updateFromParameter()
{
	cb.clear(dontSendNotification);
	idKeyMap.clear();

	if (parameter.wasObjectDeleted()) return;

	int id = 1;
	GenericScopedLock lock(ep->enumValues.getLock());
	for (auto& ev : ep->enumValues)
	{
		if (ev == nullptr)
		{
			jassertfalse;
			continue;
		}

		cb.addItem(ev->key, id);
		idKeyMap.set(id, ev->key);
		keyIdMap.set(ev->key, id);
		id++;
	}

	cb.setSelectedId(keyIdMap[ep->getValueKey()], dontSendNotification);
	cb.setEnabled(isInteractable());

	updateTooltip();
}

String EnumParameterUI::getSelectedKey()
{
	return idKeyMap[cb.getSelectedId()];
}

void EnumParameterUI::resized()
{
	if (shouldBailOut()) return;
	cb.setBounds(getLocalBounds());
}

void EnumParameterUI::updateUIParamsInternal()
{
	if (shouldBailOut()) return;
	Colour bgColor = useCustomBGColor ? customBGColor : BG_COLOR;
	Colour fgColor = useCustomTextColor ? customTextColor : TEXT_COLOR;

	cb.setColour(cb.backgroundColourId, bgColor);
	cb.setColour(cb.outlineColourId, bgColor.brighter(.2f));
	cb.setColour(cb.focusedOutlineColourId, bgColor.brighter(.3f));
	cb.setColour(cb.textColourId, fgColor);
	cb.setColour(cb.buttonColourId, fgColor);
	cb.setColour(cb.arrowColourId, fgColor.darker(.2f));
	updateFromParameter();
}

void EnumParameterUI::valueChanged(const var& value)
{
	cb.setSelectedId(keyIdMap[ep->getValueKey()], dontSendNotification);
	prevValue = ep->getValueKey();
}

void EnumParameterUI::comboBoxChanged(ComboBox*)
{
	if (shouldBailOut()) return;
	ep->setUndoableValue(prevValue, getSelectedKey());

}

EnumOptionManager::EnumOptionManager(EnumParameter* ep) :
	ep(ep)
{
	int numRowsToDisplay = ep->enumValues.size() + 5;
	for (int i = 0; i < numRowsToDisplay; i++)
	{
		EnumOptionUI* ui = new EnumOptionUI(ep, i);
		optionsUI.add(ui);
		ui->keyLabel.addListener(this);
		ui->valueLabel.addListener(this);
		container.addAndMakeVisible(ui);
	}

	viewport.setScrollBarsShown(true, false);
	addAndMakeVisible(&viewport);
	viewport.setViewedComponent(&container, false);

	setSize(200, 140);
}

EnumOptionManager::~EnumOptionManager()
{
}

void EnumOptionManager::paint(Graphics& g)
{
	juce::Rectangle<int> hr = getLocalBounds().removeFromTop(20);
	g.setColour(TEXT_COLOR);
	g.drawText("Value", hr.removeFromRight(getWidth() / 2).reduced(2).toFloat(), Justification::centred, false);
	g.drawText("Key", hr.reduced(2).toFloat(), Justification::centred, false);
}

void EnumOptionManager::resized()
{
	juce::Rectangle<int> r = getLocalBounds().withHeight(20);
	for (int i = 0; i < optionsUI.size(); i++) optionsUI[i]->setBounds(r.translated(0, i * r.getHeight()));

	int th = optionsUI.size() * r.getHeight();
	container.setSize(getWidth() - 10, th);
	viewport.setBounds(getLocalBounds().withTrimmedTop(20));
}

void EnumOptionManager::labelTextChanged(Label* l)
{
	String key = ep->getValueKey();

	ep->clearOptions();

	for (auto& o : optionsUI)
	{
		String k = o->keyLabel.getText();
		String v = o->valueLabel.getText();
		if (k.isEmpty() || v.isEmpty()) continue;
		ep->addOption(k, v, false);
	}

	ep->setValueWithKey(key);
}

EnumOptionManager::EnumOptionUI::EnumOptionUI(EnumParameter* ep, int index) :
	ep(ep),
	index(index)
{
	if (index < ep->enumValues.size())
	{
		keyLabel.setText(ep->enumValues[index]->key, dontSendNotification);
		valueLabel.setText(ep->enumValues[index]->value.toString(), dontSendNotification);
	}


	keyLabel.setEditable(true);
	keyLabel.setBorderSize(BorderSize<int>(1));
	keyLabel.setColour(Label::backgroundColourId, BG_COLOR);
	keyLabel.setColour(Label::textColourId, TEXT_COLOR.darker(.2f));
	keyLabel.setColour(Label::textWhenEditingColourId, TEXT_COLOR);

	valueLabel.setEditable(true);
	valueLabel.setBorderSize(BorderSize<int>(1));
	valueLabel.setColour(Label::backgroundColourId, BG_COLOR);
	valueLabel.setColour(Label::textColourId, TEXT_COLOR.darker(.2f));
	valueLabel.setColour(Label::textWhenEditingColourId, TEXT_COLOR);


	addAndMakeVisible(&keyLabel);
	addAndMakeVisible(&valueLabel);
}

void EnumOptionManager::EnumOptionUI::resized()
{
	keyLabel.setBounds(getLocalBounds().removeFromLeft(getWidth() / 2).reduced(2));
	valueLabel.setBounds(getLocalBounds().removeFromRight(getWidth() / 2).reduced(2));
}

EnumParameterButtonBarUI::EnumParameterButtonBarUI(Array<EnumParameter*> parameters) :
	EnumParameterUIBase(parameters),
	isVertical(false)
{
	updateFromParameter();
}

EnumParameterButtonBarUI::~EnumParameterButtonBarUI()
{
}

void EnumParameterButtonBarUI::updateFromParameter()
{
	buttons.clear();
	Random r;
	int radioId = r.nextInt();

	String selectedKey = ep->getValueKey();
	for (auto& ev : ep->enumValues)
	{
		TextButton* tb = new TextButton(ev->key);
		tb->addListener(this);
		tb->setClickingTogglesState(true);
		tb->setRadioGroupId(radioId);

		if (ev->key == selectedKey) tb->setToggleState(true, dontSendNotification);

		buttons.add(tb);
		addAndMakeVisible(tb);
	}

	updateUIParams();
}

void EnumParameterButtonBarUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	if (buttons.size() == 0) return;

	int bSize = (isVertical ? r.getHeight() : r.getWidth()) / buttons.size();
	for (auto& tb : buttons)
	{
		tb->setBounds(isVertical ? r.removeFromTop(bSize) : r.removeFromLeft(bSize));
	}
}

void EnumParameterButtonBarUI::updateUIParamsInternal()
{
	Colour tColor = useCustomTextColor ? customTextColor : TEXT_COLOR;
	Colour bColor = useCustomBGColor ? customBGColor : BG_COLOR.brighter(.1f);
	Colour fColor = useCustomFGColor ? customFGColor : (isInteractable() ? HIGHLIGHT_COLOR : FEEDBACK_COLOR);

	for (int i = 0; i < buttons.size(); i++)
	{
		TextButton* tb = buttons[i];
		tb->setColour(TextButton::textColourOffId, tColor);
		tb->setColour(TextButton::textColourOnId, bColor);
		tb->setColour(TextButton::buttonColourId, bColor);
		tb->setColour(TextButton::buttonOnColourId, fColor);

		if (i == 0) tb->setConnectedEdges(isVertical ? Button::ConnectedOnBottom : Button::ConnectedOnRight);
		else if (i == buttons.size() - 1) tb->setConnectedEdges(isVertical ? Button::ConnectedOnTop : Button::ConnectedOnLeft);
		else tb->setConnectedEdges(isVertical ? (Button::ConnectedOnTop | Button::ConnectedOnBottom) : (Button::ConnectedOnLeft | Button::ConnectedOnRight));
	}
}

void EnumParameterButtonBarUI::valueChanged(const var&)
{
	String selectedKey = ep->getValueKey();
	for (auto& tb : buttons)
	{
		if (tb->getButtonText() == selectedKey) tb->setToggleState(true, dontSendNotification);
	}
}

void EnumParameterButtonBarUI::buttonClicked(Button* b)
{
	ep->setValueWithKey(b->getButtonText());
}
