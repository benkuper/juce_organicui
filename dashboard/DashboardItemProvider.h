class DashboardItem;

class DashboardItemProvider
{
public:
	DashboardItemProvider(juce::StringRef name) : providerName(name) {}
	virtual ~DashboardItemProvider() {}

private:
	juce::String providerName;

public:
	juce::String getProviderName() { return providerName; }
	virtual juce::PopupMenu getDashboardCreateMenu(int idOffset) = 0;
	virtual DashboardItem * getDashboardItemFromMenuResult(int result) = 0;
};

class DashboardItemTarget
{
public:
	DashboardItemTarget() {}
	virtual ~DashboardItemTarget() {}

	virtual DashboardItem * createDashboardItem() = 0;
};
