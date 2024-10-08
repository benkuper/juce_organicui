/*
  ==============================================================================

    SharedTextureDashboardItem.h
    Created: 5 Dec 2021 3:53:20pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class SharedTextureDashboardItem :
	public DashboardItem,
	public SharedTextureReceiver::Listener,
	public SharedTextureManager::Listener,
	public EngineListener
{
public:
    SharedTextureDashboardItem(juce::var params = juce::var());
    ~SharedTextureDashboardItem();

	StringParameter* textureName;
    StringParameter* appName;
	SharedTextureReceiver* receiver;
	BoolParameter* exposeOnWeb;

	juce::Image outImage;

	void setupReceiver();
	juce::Image getImage();

	void onContainerParameterChangedInternal(Parameter* p) override;

	// Inherited via Listener
	virtual void textureUpdated(SharedTextureReceiver*) override;
	virtual void connectionChanged(SharedTextureReceiver*) override;
	virtual void receiverRemoved(SharedTextureReceiver* r) override;

	virtual void afterLoadJSONDataInternal() override;
	virtual void fileLoaded() override;

	static std::string base64_encode(unsigned char const* src, unsigned int len);

	DashboardItemUI* createUI() override;
	virtual juce::var getItemParameterFeedback(Parameter* p) override;

	DECLARE_ASYNC_EVENT(SharedTextureDashboardItem, ST, st, ENUM_LIST(TEXTURE_UPDATED), EVENT_ITEM_CHECK);

	juce::String getTypeString() const override { return "Shared Texture"; }
};
