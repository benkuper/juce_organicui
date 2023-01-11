/*
  ==============================================================================

	CommentUI.cpp
	Created: 4 Apr 2019 9:39:30am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

CommentUI::CommentUI(CommentItem* comment) :
	BaseItemMinimalUI(comment),
	resizer(this, nullptr)
{
	bgColor = item->itemColor->getColor().darker().withAlpha(item->bgAlpha->floatValue());

	//textUI.setEditable(false, true, false);
	textUI.setColour(textUI.textColourId, item->itemColor->getColor());
	textUI.setColour(textUI.backgroundColourId, Colours::transparentBlack);
	textUI.setColour(TextEditor::outlineColourId, Colours::transparentBlack); // non-transparent red
	textUI.setColour(TextEditor::shadowColourId, Colours::transparentBlack); // non-transparent red
	textUI.setColour(CaretComponent::caretColourId, Colours::orange);

	//textUI.setColour(textUI.backgroundWhenEditingColourId, Colours::darkgrey.withAlpha(.2f));

	textUI.setFont(item->size->floatValue());
	textUI.setMultiLine(true, true);
	textUI.setText(item->text->stringValue(), dontSendNotification);
	textUI.setReturnKeyStartsNewLine(false);
	textUI.setShiftReturnKeyStartsNewLine(true);
	textUI.setScrollbarsShown(false);

	textUI.addListener(this);

	disableTextEditor();

	textUI.setFont(item->size->floatValue());
	addAndMakeVisible(&textUI);

	addAndMakeVisible(&resizer);

	//autoHideWhenDragging = false;
	//drawEmptyDragIcon = true;

	setSize(item->viewUISize->x, item->viewUISize->y);
	//setSize(textUI.getTextWidth(), textUI.getTextHeight()+4);
}

CommentUI::~CommentUI()
{
}

void CommentUI::paint(Graphics& g)
{
	BaseItemMinimalUI::paint(g);


	//if (inspectable.wasObjectDeleted()) return;
	//
	//Colour c = item->color->getColor();
	//float bgAlpha = item->bgAlpha->floatValue();
	//
	//g.setColour(c.darker().withAlpha(bgAlpha));
	//g.fillRoundedRectangle(getLocalBounds().toFloat(), 2);
	//g.setColour(c.withAlpha(bgAlpha));
	//g.drawRoundedRectangle(getLocalBounds().toFloat(), 2, 2);
	// 
	//g.setColour(c);
	//g.setFont(item->size->floatValue());
	//g.drawText(item->text->stringValue(), getLocalBounds().toFloat(), Justification::centred);
}

void CommentUI::resized()
{
	resizer.setBounds(getLocalBounds());
	textUI.setBounds(getLocalBounds());
	item->viewUISize->setPoint(getWidth(), getHeight());
}

void CommentUI::mouseDown(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseDown(e);

	if (e.eventComponent == &resizer)
	{
		this->baseItem->setSizeReference(true);
	}
}

void CommentUI::mouseDrag(const MouseEvent& e)
{
	if (e.eventComponent == &textUI)
	{
		//avoid dragging the item if selecting text inside editor
	}
	else
	{
		BaseItemMinimalUI::mouseDrag(e);

		if (e.eventComponent == &resizer)
		{
			itemMinimalUIListeners.call(&BaseItemMinimalUI<CommentItem>::ItemMinimalUIListener::itemUIResizeDrag, this, e.getOffsetFromDragStart());
		}
	}
}

void CommentUI::mouseUp(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseDrag(e);

	if (e.eventComponent == &resizer)
	{
		itemMinimalUIListeners.call(&BaseItemMinimalUI<CommentItem>::ItemMinimalUIListener::itemUIResizeEnd, this);
	}
}

void CommentUI::mouseDoubleClick(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseDoubleClick(e);
	textUI.setColour(textUI.backgroundColourId, Colours::black);
	textUI.setInterceptsMouseClicks(true, true);
	textUI.setReadOnly(false);
	textUI.setHighlightedRegion({ 0,0 });
}

void CommentUI::textEditorTextChanged(TextEditor&)
{
	item->text->setValue(textUI.getText());
}

void CommentUI::textEditorEscapeKeyPressed(TextEditor&)
{
}

void CommentUI::textEditorReturnKeyPressed(TextEditor&)
{
	disableTextEditor();
}

void CommentUI::textEditorFocusLost(TextEditor&)
{
	disableTextEditor();
}


void CommentUI::focusLost(FocusChangeType type)
{
	disableTextEditor();
	BaseItemMinimalUI::focusLost(type);
}

bool CommentUI::canStartDrag(const MouseEvent& e)
{
	return e.eventComponent == this || e.eventComponent == &textUI;
}

void CommentUI::inspectableSelectionChanged(Inspectable* i)
{
	if (!item->isSelected)
	{
		disableTextEditor();
	}

	BaseItemMinimalUI::inspectableSelectionChanged(i);
}

void CommentUI::disableTextEditor()
{
	textUI.setColour(textUI.backgroundColourId, Colours::transparentWhite);
	textUI.setInterceptsMouseClicks(false, false);
	textUI.setReadOnly(true);
	textUI.setHighlightedRegion({ 0,0 });
}

void CommentUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	BaseItemMinimalUI::controllableFeedbackUpdateInternal(c);

	if (c == item->size || c == item->text)
	{
		if (c == item->text) textUI.setText(item->text->stringValue(), dontSendNotification);
		textUI.applyFontToAllText(item->size->floatValue(), true);
		//setSize(textUI.getTextWidth(), textUI.getTextHeight()+4);
	}
	else if (c == item->bgAlpha || c == item->itemColor)
	{
		bgColor = item->itemColor->getColor().darker().withAlpha(item->bgAlpha->floatValue());
		textUI.applyColourToAllText(item->itemColor->getColor(), true);
		repaint();
	}
}
