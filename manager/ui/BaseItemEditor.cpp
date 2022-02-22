BaseItemEditor::BaseItemEditor(BaseItem * bi, bool isRoot) :
	EnablingControllableContainerEditor(bi, isRoot),
	item(bi)
{
	if (!isRoot)
	{
		if (item->userCanRemove)
		{
			removeBT.reset(AssetManager::getInstance()->getRemoveBT());
			removeBT->setWantsKeyboardFocus(false);
			addAndMakeVisible(removeBT.get());
			removeBT->addListener(this);
		}

		if(item->userCanDuplicate)
		{
			duplicateBT.reset(AssetManager::getInstance()->getDuplicateBT());
			duplicateBT->setWantsKeyboardFocus(false);
			addAndMakeVisible(duplicateBT.get());
			duplicateBT->addListener(this);
		}

		if (item->canBeReorderedInEditor)
		{
			upBT.reset(AssetManager::getInstance()->getUpBT());
			downBT.reset(AssetManager::getInstance()->getDownBT());
			upBT->setWantsKeyboardFocus(false);
			downBT->setWantsKeyboardFocus(false);

			addAndMakeVisible(upBT.get());
			addAndMakeVisible(downBT.get());
			upBT->addListener(this);
			downBT->addListener(this);
		}
	}
	
}

BaseItemEditor::~BaseItemEditor()
{
	if (!inspectable.wasObjectDeleted()) item->removeAsyncContainerListener(this);
}

void BaseItemEditor::setIsFirst(bool value)
{
	isFirst = value;
	if (upBT != nullptr) upBT->setVisible(!isFirst);
}

void BaseItemEditor::setIsLast(bool value)
{
	isLast = value;
	if (downBT != nullptr) downBT->setVisible(!isLast);
}

void BaseItemEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
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

	if (upBT != nullptr)
	{
		upBT->setBounds(r.removeFromRight(targetHeight).reduced(4));
	}

	if (downBT != nullptr)
	{
		downBT->setBounds(r.removeFromRight(targetHeight).reduced(4));
	}

	EnablingControllableContainerEditor::resizedInternalHeader(r);
	
	resizedInternalHeaderItemInternal(r);
}

void BaseItemEditor::buttonClicked(Button * b)
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
	}else if(b == duplicateBT.get())
	{
		item->duplicate();
	} else if (b == upBT.get())
	{
		item->moveBefore();
	} else if (b == downBT.get())
	{
		item->moveAfter();
	}
}

