template<class M, class T, class U>
class BaseManagerViewUI;
	
template<class M, class T, class U>
class BaseManagerViewMiniPane :
		public Component
{
public:
	BaseManagerViewMiniPane(BaseManagerViewUI<M,T,U> * managerUI);
	~BaseManagerViewMiniPane() {}

	BaseManagerViewUI<M, T, U> * managerUI;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void updateContent();
	void updateHandle();

	class PanHandle :
		public Component
	{
	public:
		PanHandle() {}
		~PanHandle() {}

		void paint(Graphics& g) override;
	};
};

template<class M, class T, class U>
BaseManagerViewMiniPane<M, T, U>::BaseManagerViewMiniPane(BaseManagerViewUI<M, T, U> * managerUI) :
	managerUI(managerUI)
{

}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::paint(juce::Graphics& g)
{
	g.fillAll(Colours::white.withAlpha(.05f));
	g.setColour(Colours::white.withAlpha(.2f));
	g.drawRect(getLocalBounds());

	if (managerUI->itemsUI.size() == 0) return;

	juce::Rectangle<int> mr = managerUI->getLocalBounds();
	juce::Rectangle<float> focusRect;
	focusRect.setPosition(managerUI->getViewPos(mr.getPosition()).toFloat());
	juce::Point<float> vs(mr.getWidth() * managerUI->viewZoom, mr.getHeight() * managerUI->viewZoom);
	focusRect.setSize(vs.x, vs.y);


	juce::Rectangle<float> itemsRealBounds(focusRect);
	for (auto& ui : managerUI->itemsUI)
	{
		itemsRealBounds.setLeft(jmin<float>(ui->item->viewUIPosition->x, itemsRealBounds.getX()));
		itemsRealBounds.setTop(jmin<float>(ui->item->viewUIPosition->y, itemsRealBounds.getY()));
		itemsRealBounds.setRight(jmax<float>(ui->item->viewUIPosition->x + ui->item->viewUISize->x, itemsRealBounds.getRight()));
		itemsRealBounds.setBottom(jmax<float>(ui->item->viewUIPosition->y + ui->item->viewUISize->y, itemsRealBounds.getBottom()));
	}


	DBG("Real bounds : " << itemsRealBounds.toString());
	juce::Rectangle<int> r = getLocalBounds();

	float itemsRatio = itemsRealBounds.getWidth()*1.0f / itemsRealBounds.getHeight();
	float ratio = r.getWidth()*1.0f / r.getHeight();

	if (itemsRatio > ratio)
	{
		float targetHeight = itemsRealBounds.getWidth() / ratio;
		itemsRealBounds.expand(0, (targetHeight - itemsRealBounds.getHeight()) / 2);
	}
	else
	{
		float targetWidth = itemsRealBounds.getHeight() * ratio;
		itemsRealBounds.expand((targetWidth - itemsRealBounds.getWidth()) / 2, 0);
	}

	for (auto& ui : managerUI->itemsUI)
	{
		juce::Rectangle<float> b(ui->item->viewUIPosition->x - itemsRealBounds.getX(), ui->item->viewUIPosition->y - itemsRealBounds.getY(), ui->item->viewUISize->x, ui->item->viewUISize->y);
		
		juce::Rectangle<int> uiPaneBounds(	b.getX() * r.getWidth() / itemsRealBounds.getWidth(), 
											b.getY() * r.getHeight() / itemsRealBounds.getHeight(),
											b.getWidth() * r.getWidth() / itemsRealBounds.getWidth(),
											b.getHeight() * r.getHeight() / itemsRealBounds.getHeight());
		
		g.setColour(Colours::white.withAlpha(.3f));
		g.fillRect(uiPaneBounds);
	}

	

	float fx = jmap<float>(focusRect.getX(), itemsRealBounds.getX(), itemsRealBounds.getRight(), r.getX(), r.getRight());
	float fy = jmap<float>(focusRect.getY(), itemsRealBounds.getY(), itemsRealBounds.getBottom(), r.getY(), r.getBottom());
	float fw = focusRect.getWidth() * r.getWidth() / itemsRealBounds.getWidth();
	float fh = focusRect.getHeight() * r.getHeight() / itemsRealBounds.getHeight();
	juce::Rectangle<int> focusPaneRect(fx, fy, fw, fh);

	g.setColour(BLUE_COLOR.withAlpha(.2f));
	g.fillRect(focusPaneRect);
	g.setColour(BLUE_COLOR.withAlpha(.5f));
	g.drawRect(focusPaneRect);
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::resized()
{
	updateContent();
}


template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::updateContent()
{
	updateHandle();
	repaint();
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::updateHandle()
{
	
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::PanHandle::paint(Graphics& g)
{

}
