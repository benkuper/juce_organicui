#include "AssetManager.h"

juce_ImplementSingleton(AssetManager);


AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}

ImageButton * AssetManager::getRemoveBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::cancel_png, OrganicUIBinaryData::cancel_pngSize)); }
ImageButton * AssetManager::getConfigBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::settings_png, OrganicUIBinaryData::settings_pngSize)); }
ImageButton * AssetManager::getTargetBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::target_png, OrganicUIBinaryData::target_pngSize)); }
ImageButton * AssetManager::getPowerBT() { return getToggleBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::power_png, OrganicUIBinaryData::power_pngSize)); }
ImageButton * AssetManager::getAddBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::add_png, OrganicUIBinaryData::add_pngSize)); }
ImageButton * AssetManager::getFileBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::file_png, OrganicUIBinaryData::file_pngSize)); }
ImageButton * AssetManager::getEditBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::edit_png, OrganicUIBinaryData::edit_pngSize)); }
ImageButton * AssetManager::getRightArrowBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::arrowright_png, OrganicUIBinaryData::arrowright_pngSize)); }
ImageButton * AssetManager::getDownArrowImageBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::arrowdown_png, OrganicUIBinaryData::arrowdown_pngSize)); }
ImageButton * AssetManager::getUpBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::up_png, OrganicUIBinaryData::up_pngSize)); }
ImageButton * AssetManager::getDownBT() { return getSetupBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::down_png, OrganicUIBinaryData::down_pngSize)); }


Image AssetManager::getInImage() { return ImageCache::getFromMemory(OrganicUIBinaryData::in_png, OrganicUIBinaryData::in_pngSize); }
Image AssetManager::getOutImage() { return ImageCache::getFromMemory(OrganicUIBinaryData::out_png, OrganicUIBinaryData::out_pngSize); }
Image AssetManager::getReloadImage() { return ImageCache::getFromMemory(OrganicUIBinaryData::reload_png, OrganicUIBinaryData::reload_pngSize); }
Image AssetManager::getTriggerImage() { return ImageCache::getFromMemory(OrganicUIBinaryData::trigger_png, OrganicUIBinaryData::trigger_pngSize); }
Image AssetManager::getRelativeImage() { return ImageCache::getFromMemory(OrganicUIBinaryData::relative_png, OrganicUIBinaryData::relative_pngSize); }


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
