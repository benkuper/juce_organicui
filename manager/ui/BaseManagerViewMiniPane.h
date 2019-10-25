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
	juce::Point<float> vs(mr.getWidth() / managerUI->viewZoom, mr.getHeight() / managerUI->viewZoom);
	focusRect.setSize(vs.x, vs.y);


	juce::Rectangle<float> itemViewBounds(focusRect);
	for (auto& ui : managerUI->itemsUI)
	{
		itemViewBounds.setLeft(jmin<float>(ui->item->viewUIPosition->x, itemViewBounds.getX()));
		itemViewBounds.setTop(jmin<float>(ui->item->viewUIPosition->y, itemViewBounds.getY()));
		itemViewBounds.setRight(jmax<float>(ui->item->viewUIPosition->x + ui->item->viewUISize->x, itemViewBounds.getRight()));
		itemViewBounds.setBottom(jmax<float>(ui->item->viewUIPosition->y + ui->item->viewUISize->y, itemViewBounds.getBottom()));
	}


	juce::Rectangle<int> r = getLocalBounds();

	float itemsRatio = itemViewBounds.getWidth()*1.0f / itemViewBounds.getHeight();
	float ratio = r.getWidth()*1.0f / r.getHeight();

	if (itemsRatio > ratio)
	{
		float targetHeight = itemViewBounds.getWidth() / ratio;
		itemViewBounds.expand(0, (targetHeight - itemViewBounds.getHeight()) / 2);
	}
	else
	{
		float targetWidth = itemViewBounds.getHeight() * ratio;
		itemViewBounds.expand((targetWidth - itemViewBounds.getWidth()) / 2, 0);
	}

	juce::Rectangle<float> itemRealBounds = managerUI->getBoundsInView(itemViewBounds);

	for (auto& ui : managerUI->itemsUI)
	{
		juce::Rectangle<int> b = ui->getBoundsInParent();
		
		float bx = jmap<float>(b.getX(), itemRealBounds.getX(), itemRealBounds.getRight(), r.getX(), r.getRight());
		float by = jmap<float>(b.getY(), itemRealBounds.getY(), itemRealBounds.getBottom(), r.getY(), r.getBottom());
		float bw = b.getWidth() * r.getWidth() / itemRealBounds.getWidth();
		float bh = b.getHeight() * r.getHeight() / itemRealBounds.getHeight();

		juce::Rectangle<int> uiPaneBounds(bx, by, bw, bh);
		
		g.setColour(Colours::white.withAlpha(.3f));
		g.fillRect(uiPaneBounds);
	}

	

	float fx = jmap<float>(focusRect.getX(), itemViewBounds.getX(), itemViewBounds.getRight(), r.getX(), r.getRight());
	float fy = jmap<float>(focusRect.getY(), itemViewBounds.getY(), itemViewBounds.getBottom(), r.getY(), r.getBottom());
	float fw = focusRect.getWidth() * r.getWidth() / itemViewBounds.getWidth();
	float fh = focusRect.getHeight() * r.getHeight() / itemViewBounds.getHeight();
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
