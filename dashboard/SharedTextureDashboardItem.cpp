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
		Image img = getImage().rescaled(viewUISize->x, viewUISize->y);
		MemoryOutputStream os;
		JPEGImageFormat fmt;
		fmt.writeImageToStream(img, os);
		String s = base64_encode((const unsigned char*)os.getData(), os.getDataSize());

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


inline std::string SharedTextureDashboardItem::base64_encode(unsigned char const* src, unsigned int len) {
	const unsigned char base64_table[65] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	unsigned char* out, * pos;
	const unsigned char* end, * in;

	size_t olen;

	olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

	if (olen < len)
		return std::string(); /* integer overflow */

	std::string outStr;
	outStr.resize(olen);
	out = (unsigned char*)&outStr[0];

	end = src + len;
	in = src;
	pos = out;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		}
		else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
				(in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
	}

	return outStr;

}

DashboardItemUI* SharedTextureDashboardItem::createUI()
{
	return new SharedTextureDashboardItemUI(this);
}