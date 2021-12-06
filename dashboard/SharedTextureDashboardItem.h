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
	public SharedTextureManager::Listener
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

	DashboardItemUI* createUI() override;

	DECLARE_ASYNC_EVENT(SharedTextureDashboardItem, ST, st, { TEXTURE_UPDATED });

	String getTypeString() const override { return "Shared Texture"; }
};
