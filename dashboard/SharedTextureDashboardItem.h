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
    SharedTextureDashboardItem(var params = var());
    ~SharedTextureDashboardItem();

	StringParameter* textureName;
	SharedTextureReceiver* receiver;
	BoolParameter* exposeOnWeb;

	Image outImage;

	void setupReceiver();
	Image getImage();

	void onContainerParameterChangedInternal(Parameter* p) override;

	// Inherited via Listener
	virtual void textureUpdated(SharedTextureReceiver*) override;
	virtual void connectionChanged(SharedTextureReceiver*) override;
	virtual void receiverRemoved(SharedTextureReceiver* r) override;

	virtual void afterLoadJSONDataInternal() override;
	virtual void endLoadFile() override;

	static std::string base64_encode(unsigned char const* src, unsigned int len);

	DashboardItemUI* createUI() override;
	virtual var getItemParameterFeedback(Parameter* p) override;

	DECLARE_ASYNC_EVENT(SharedTextureDashboardItem, ST, st, { TEXTURE_UPDATED }, EVENT_ITEM_CHECK);

	String getTypeString() const override { return "Shared Texture"; }
};
