DashboardCommentItemUI::DashboardCommentItemUI(DashboardCommentItem* comment) :
	DashboardItemUI(comment),
	comment(comment)
{
	bgColor = comment->itemColor->getColor().darker().withAlpha(comment->bgAlpha->floatValue());

	//textUI.setEditable(false, true, false);
	textUI.setColour(textUI.textColourId, comment->itemColor->getColor());
	textUI.setColour(textUI.backgroundColourId, Colours::transparentBlack);
	textUI.setColour(TextEditor::outlineColourId, Colours::transparentBlack); // non-transparent red
	textUI.setColour(TextEditor::shadowColourId, Colours::transparentBlack); // non-transparent red
	textUI.setColour(CaretComponent::caretColourId, Colours::orange);

	//textUI.setColour(textUI.backgroundWhenEditingColourId, Colours::darkgrey.withAlpha(.2f));

	textUI.setFont(FontOptions(comment->size->floatValue()));
	textUI.setMultiLine(true, false);
	textUI.setText(comment->text->stringValue(), dontSendNotification);
	textUI.setReturnKeyStartsNewLine(false);
	//textUI.setShiftReturnKeyStartsNewLine(true);
	textUI.setScrollbarsShown(false);

	textUI.addListener(this);

	disableTextEditor();

	textUI.setFont(FontOptions(comment->size->floatValue()));
	addAndMakeVisible(&textUI);

	setSize(item->viewUISize->x, item->viewUISize->y);
	item->viewUISize->setPoint(getWidth(), getHeight());
	removeChildComponent(&resizer);

	resized();
}

DashboardCommentItemUI::~DashboardCommentItemUI()
{
}

void DashboardCommentItemUI::resizedDashboardItemInternal()
{
	textUI.setBounds(getLocalBounds());
}

void DashboardCommentItemUI::updateEditModeInternal(bool editMode)
{

	textUI.setReadOnly(!editMode);
	disableTextEditor();
}

void DashboardCommentItemUI::mouseDown(const MouseEvent& e)
{
	ItemMinimalUI::mouseDown(e);
}

void DashboardCommentItemUI::mouseDrag(const MouseEvent& e)
{
	if (e.eventComponent == &textUI)
	{
		//avoid dragging the item if selecting text inside editor
	}
	else
	{
		ItemMinimalUI::mouseDrag(e);
	}
}

void DashboardCommentItemUI::mouseDoubleClick(const MouseEvent& e)
{
	ItemMinimalUI::mouseDoubleClick(e);
	textUI.setColour(textUI.backgroundColourId, Colours::black);
	textUI.setInterceptsMouseClicks(true, true);
	textUI.setReadOnly(false);
	textUI.setHighlightedRegion({ 0,0 });
}

void DashboardCommentItemUI::textEditorTextChanged(TextEditor&)
{
	comment->text->setValue(textUI.getText());
}

void DashboardCommentItemUI::textEditorEscapeKeyPressed(TextEditor&)
{
}

void DashboardCommentItemUI::textEditorReturnKeyPressed(TextEditor&)
{
	disableTextEditor();
}

void DashboardCommentItemUI::textEditorFocusLost(TextEditor&)
{
	disableTextEditor();
}


void DashboardCommentItemUI::focusLost(FocusChangeType type)
{
	disableTextEditor();
	ItemMinimalUI::focusLost(type);
}

bool DashboardCommentItemUI::canStartDrag(const MouseEvent& e)
{
	return e.eventComponent == this || e.eventComponent == &textUI;
}

void DashboardCommentItemUI::inspectableSelectionChanged(Inspectable* i)
{
	if (!comment->isSelected)
	{
		disableTextEditor();
	}

	ItemMinimalUI::inspectableSelectionChanged(i);
}

void DashboardCommentItemUI::disableTextEditor()
{
	textUI.setColour(textUI.backgroundColourId, Colours::transparentWhite);
	textUI.setInterceptsMouseClicks(false, false);
	textUI.setReadOnly(true);
	textUI.setHighlightedRegion({ 0,0 });
}

void DashboardCommentItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	ItemMinimalUI::controllableFeedbackUpdateInternal(c);

	if (c == comment->size || c == comment->text)
	{
		if (c == comment->text) textUI.setText(comment->text->stringValue(), dontSendNotification);
		textUI.applyFontToAllText(FontOptions(comment->size->floatValue()), true);
		item->viewUISize->setPoint(textUI.getTextWidth(), textUI.getTextHeight() + 4);
	}
	else if (c == comment->itemColor || c == comment->bgAlpha)
	{
		bgColor = comment->itemColor->getColor().darker().withAlpha(comment->bgAlpha->floatValue());
		textUI.applyColourToAllText(comment->itemColor->getColor(), true);
		repaint();
	}
}
