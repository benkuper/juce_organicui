#include "JuceHeader.h"

BaseItemEditor::BaseItemEditor(Array<BaseItem*> bi, bool isRoot) :
	EnablingControllableContainerEditor(Inspectable::getArrayAs<BaseItem, EnablingControllableContainer>(bi), isRoot),
	items(bi)
{
	jassert(items.size() > 0);
	item = items[0];



	if (item->userCanRemove)
	{
		removeBT.reset(AssetManager::getInstance()->getRemoveBT());
		removeBT->setWantsKeyboardFocus(false);
		addAndMakeVisible(removeBT.get());
		removeBT->addListener(this);
	}

	if (!isRoot)
	{
		if (item->userCanDuplicate)
		{
			duplicateBT.reset(AssetManager::getInstance()->getDuplicateBT());
			duplicateBT->setWantsKeyboardFocus(false);
			addAndMakeVisible(duplicateBT.get());
			duplicateBT->addListener(this);
		}

		//if (item->canBeReorderedInEditor)
		//{
			//	upBT.reset(AssetManager::getInstance()->getUpBT());
			//	downBT.reset(AssetManager::getInstance()->getDownBT());
			//	upBT->setWantsKeyboardFocus(false);
			//	downBT->setWantsKeyboardFocus(false);

			//	addAndMakeVisible(upBT.get());
			//	addAndMakeVisible(downBT.get());
			//	upBT->addListener(this);
			//	downBT->addListener(this);
		//}
	}
	else
	{
		if (item->itemColor != nullptr)
		{
			itemColorUI.reset(item->itemColor->createColorParamUI());
			addAndMakeVisible(itemColorUI.get());
		}

	}

	if (item->canBeReorderedInEditor && !isRoot)
	{
		setDragAndDropEnabled(true);
	}

}

BaseItemEditor::~BaseItemEditor()
{
	if (!inspectable.wasObjectDeleted()) item->removeAsyncContainerListener(this);
}

//void BaseItemEditor::setIsFirst(bool value)
//{
//	isFirst = value;
//	if (upBT != nullptr) upBT->setVisible(!isFirst);
//}
//
//void BaseItemEditor::setIsLast(bool value)
//{
//	isLast = value;
//	if (downBT != nullptr) downBT->setVisible(!isLast);
//}

void BaseItemEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (r.getWidth() == 0) return;

	int targetHeight = jmin<int>(r.getHeight(), 20);

	if (item->userCanRemove && removeBT != nullptr)
	{
		removeBT->setBounds(r.removeFromRight(targetHeight).reduced(2));
		r.removeFromRight(2);

	}

	if (item->userCanDuplicate && duplicateBT != nullptr)
	{
		duplicateBT->setBounds(r.removeFromRight(targetHeight).reduced(2));
		r.removeFromRight(2);
	}

	if (itemColorUI != nullptr)
	{
		itemColorUI->setBounds(r.removeFromRight(targetHeight).reduced(2));
	}


	EnablingControllableContainerEditor::resizedInternalHeader(r);

	resizedInternalHeaderItemInternal(r);
}

void BaseItemEditor::buttonClicked(Button* b)
{
	EnablingControllableContainerEditor::buttonClicked(b);

	if (b == removeBT.get())
	{
		if (this->item->askConfirmationBeforeRemove && GlobalSettings::getInstance()->askBeforeRemovingItems->boolValue())
		{
			AlertWindow::showAsync(
				MessageBoxOptions().withIconType(AlertWindow::QuestionIcon)
				.withTitle("Delete " + this->item->niceName)
				.withMessage("Are you sure you want to delete this ?")
				.withButton("Delete")
				.withButton("Cancel"),
				[this](int result)
				{
					if (result != 0) this->item->remove();
				}
			);
		}
		else this->item->remove();
		return;
	}
	else if (b == duplicateBT.get())
	{
		item->duplicate();
	}
	//else if (b == upBT.get())
	//{
	//	item->moveBefore();
	//} else if (b == downBT.get())
	//{
	//	item->moveAfter();
	//}
}

void BaseItemEditor::setDragDetails(var& details)
{
	details.getDynamicObject()->setProperty("type", item->getTypeString());
	details.getDynamicObject()->setProperty("dataType", item->itemDataType);
}