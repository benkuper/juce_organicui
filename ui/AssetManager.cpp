
juce_ImplementSingleton(AssetManager);


AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{


}

ImageButton * AssetManager::getRemoveBT()
{
	Image removeImage = ImageCache::getFromMemory(OrganicUIBinaryData::cancel_png, OrganicUIBinaryData::cancel_pngSize);
	return getSetupBTImage(removeImage);
}

ImageButton * AssetManager::getConfigBT()
{
	Image removeImage = ImageCache::getFromMemory(OrganicUIBinaryData::settings_png, OrganicUIBinaryData::settings_pngSize);
	return getSetupBTImage(removeImage);
}

ImageButton * AssetManager::getTargetBT()
{
	Image removeImage = ImageCache::getFromMemory(OrganicUIBinaryData::target_png, OrganicUIBinaryData::target_pngSize);
	return getSetupBTImage(removeImage);
}

ImageButton * AssetManager::getPowerBT()
{
	Image removeImage = ImageCache::getFromMemory(OrganicUIBinaryData::power_png, OrganicUIBinaryData::power_pngSize);
	return getToggleBTImage(removeImage);
}

ImageButton * AssetManager::getAddBT()
{
	Image removeImage = ImageCache::getFromMemory(OrganicUIBinaryData::add_png, OrganicUIBinaryData::add_pngSize);
	return getSetupBTImage(removeImage);
}

ImageButton * AssetManager::getFileBT()
{
	Image fileImage = ImageCache::getFromMemory(OrganicUIBinaryData::file_png, OrganicUIBinaryData::file_pngSize);
	return getSetupBTImage(fileImage);
}


ImageButton * AssetManager::getEditBT()
{
	Image editImage = ImageCache::getFromMemory(OrganicUIBinaryData::edit_png, OrganicUIBinaryData::edit_pngSize);
	return getSetupBTImage(editImage);
}



Image AssetManager::getInImage()
{
	return ImageCache::getFromMemory(OrganicUIBinaryData::in_png, OrganicUIBinaryData::in_pngSize);
}

Image AssetManager::getOutImage()
{
	return ImageCache::getFromMemory(OrganicUIBinaryData::out_png, OrganicUIBinaryData::out_pngSize);
}

Image AssetManager::getReloadImage()
{
	return ImageCache::getFromMemory(OrganicUIBinaryData::reload_png, OrganicUIBinaryData::reload_pngSize);
}

Image AssetManager::getPlayImage()
{
	return ImageCache::getFromMemory(OrganicUIBinaryData::playpause_png, OrganicUIBinaryData::playpause_pngSize);
}

Image AssetManager::getStopImage()
{
	return ImageCache::getFromMemory(OrganicUIBinaryData::stop_png, OrganicUIBinaryData::stop_pngSize);
}

Image AssetManager::getPrevCueImage()
{
	return ImageCache::getFromMemory(OrganicUIBinaryData::prevcue_png, OrganicUIBinaryData::prevcue_pngSize);
}

Image AssetManager::getNextCueImage()
{
	return ImageCache::getFromMemory(OrganicUIBinaryData::nextcue_png, OrganicUIBinaryData::nextcue_pngSize);
}

ImageButton * AssetManager::getSetupBTImage(const Image & image)
{
	ImageButton * bt = new ImageButton();
	bt->setImages(false, true, true, 
		image, 0.7f, Colours::transparentBlack,
		image, 1.0f, Colours::transparentBlack,
		image, 1.0f, Colours::white.withAlpha(.7f),0);
	return bt;
}

ImageButton * AssetManager::getToggleBTImage(const Image & image)
{
	ImageButton * bt = new ImageButton();
	Image offImage = image.createCopy();
	offImage.desaturate();

	bt->setImages(false, true, true, 
		offImage, 0.5f, Colours::transparentBlack,
		offImage, 1.0f, Colours::white.withAlpha(.2f),
		image, 1.0f, Colours::transparentBlack, 0);
	return bt; 
}
