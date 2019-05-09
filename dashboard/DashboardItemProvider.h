class DashboardItem;
class DashboardItemUI;

class DashboardItemProvider
{
public:
	DashboardItemProvider(StringRef name) : providerName(name) {}
	virtual ~DashboardItemProvider() {}

private:
	String providerName;

public:
	String getProviderName() { return providerName; }
	virtual PopupMenu getDashboardCreateMenu(int idOffset) = 0;
	virtual DashboardItem * getDashboardItemFromMenuResult(int result) = 0;
};

class DashboardItemTarget
{
public:
	DashboardItemTarget() {}
	~DashboardItemTarget() {}

	virtual DashboardItem * createDashboardItem() = 0;
};