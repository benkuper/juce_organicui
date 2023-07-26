/*
  ==============================================================================

    AssetManager.h
    Created: 23 Oct 2016 2:22:04pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

using namespace juce;

class AssetManager
{
public:
	juce_DeclareSingleton(AssetManager, true);
	AssetManager();
	virtual ~AssetManager();

	juce::Image warningImage;
	juce::Image eyeImage;
	juce::Image powerOn;
	juce::Image powerOff;
	juce::Image drag;
	juce::Image cancel;

	juce::Image inImage;
	juce::Image outImage;
	juce::Image reloadImage;
	juce::Image triggerImage;
	juce::Image relativeImage;
	juce::Image minusImage;
	juce::Image explorerImage;


	juce::ImageButton * getRemoveBT();
	juce::ImageButton * getConfigBT();
	juce::ImageButton * getTargetBT();
	juce::ImageButton * getPowerBT();  
	juce::ImageButton * getAddBT();
	juce::ImageButton * getDuplicateBT();
	juce::ImageButton * getFileBT();
	juce::ImageButton * getEditBT();
	juce::ImageButton * getRightArrowBT();
	juce::ImageButton * getDownArrowImageBT();
	juce::ImageButton * getUpBT();


	juce::ImageButton * getSetupBTImage(const juce::Image & image);
	juce::ImageButton * getToggleBTImage(const juce::Image &image);
	juce::ImageButton* getBTDoubleImage(const juce::Image& onImage, const juce::Image& offImage);

};