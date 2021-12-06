/*
  ==============================================================================

	SharedTextureDashboardItem.cpp
	Created: 5 Dec 2021 3:53:20pm
	Author:  bkupe

  ==============================================================================
*/

SharedTextureDashboardItem::SharedTextureDashboardItem(var params) :
	DashboardItem(this, getTypeString()),
	receiver(nullptr),
	stNotifier(5)
{
	textureName = addStringParameter("Texture Name", "The Spout / Syphon name of the texture", "");
	exposeOnWeb = addBoolParameter("Expose on Web", "If checked, this will convert the GPU texture to a CPU rescaled one, and feed it to the web clients. The downside is that is will slow the UI on larger images.", false);
	setupReceiver();

	SharedTextureManager::getInstance()->addListener(this);
}

SharedTextureDashboardItem::~SharedTextureDashboardItem()
{
	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr)
	{
		if (receiver != nullptr)
		{
			receiver->removeListener(this);
			SharedTextureManager::getInstance()->removeReceiver(receiver);
			receiver = nullptr;
		}

		SharedTextureManager::getInstance()->removeListener(this);
	}

}

void SharedTextureDashboardItem::setupReceiver()
{
	if (receiver != nullptr)
	{
		receiver->removeListener(this);
		SharedTextureManager::getInstance()->removeReceiver(receiver);
	}

	receiver = nullptr;

	receiver = SharedTextureManager::getInstance()->addReceiver(textureName->stringValue());

	if (receiver != nullptr)
	{
		receiver->addListener(this);
		//receiver->setUseCPUImage(true);
		//receiver->createReceiver();
	}

}

Image SharedTextureDashboardItem::getImage()
{
	if (receiver == nullptr || isClearing) return Image();
	else return receiver->getImage();
}

void SharedTextureDashboardItem::textureUpdated(SharedTextureReceiver*)
{
	//Spatializer::getInstance()->computeSpat(getImage());
	//updateColorsFromImage();


	/*
	Rectangle<float> imgBounds = Rectangle<float>(0, 0, viewUISize->x, viewUISize->y);

	RectanglePlacement p;
	p.getTransformToFit(img.getBounds().toFloat(), imgBounds);

	Graphics g(outImage);
	g.drawImage(img, imgBounds, p);
	*/
	if (exposeOnWeb->boolValue())
	{
		Image img = getImage();
		MemoryOutputStream os;
		JPEGImageFormat fmt;
		fmt.writeImageToStream(img.rescaled(viewUISize->x, viewUISize->y), os);

		String s = ScriptUtil::base64_encode((const unsigned char*)os.getData(), os.getDataSize());

		var data(new DynamicObject());
		data.getDynamicObject()->setProperty("controlAddress", this->getControlAddress(DashboardManager::getInstance()));
		data.getDynamicObject()->setProperty("value", s);
		notifyDataFeedback(data);
	}

	stNotifier.addMessage(new STEvent(STEvent::TEXTURE_UPDATED, this));
}

void SharedTextureDashboardItem::connectionChanged(SharedTextureReceiver*)
{
	//inputIsLive->setValue(receiver->isConnected);
	stNotifier.addMessage(new STEvent(STEvent::TEXTURE_UPDATED, this));
}

void SharedTextureDashboardItem::receiverRemoved(SharedTextureReceiver* r)
{
	if (r == receiver) receiver = nullptr;
	stNotifier.addMessage(new STEvent(STEvent::TEXTURE_UPDATED, this));
}

void SharedTextureDashboardItem::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == textureName) setupReceiver();
}

DashboardItemUI* SharedTextureDashboardItem::createUI()
{
	return new SharedTextureDashboardItemUI(this);
}