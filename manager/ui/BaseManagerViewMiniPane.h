template<class M, class T, class U>
class BaseManagerViewUI;
	
template<class M, class T, class U>
class BaseManagerViewMiniPane :
		public Component,
		public Timer
{
public:
	BaseManagerViewMiniPane(BaseManagerViewUI<M,T,U> * managerUI);
	virtual ~BaseManagerViewMiniPane() {}

	BaseManagerViewUI<M, T, U> * managerUI;
	ComponentAnimator ca;

	float showTime;

	virtual void paint(juce::Graphics& g) override;
	virtual void paintInternal(juce::Graphics& g, const juce::Rectangle<float>& itemViewBounds, const juce::Rectangle<int>& itemRealBounds) {}
	virtual void resized() override;

	virtual void updateContent();

	void timerCallback() override;

};

template<class M, class T, class U>
BaseManagerViewMiniPane<M, T, U>::BaseManagerViewMiniPane(BaseManagerViewUI<M, T, U> * managerUI) :
	managerUI(managerUI),
	showTime(1.5f)
{
	setAlpha(0);
	setOpaque(false);
	setInterceptsMouseClicks(true, true);
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::paint(juce::Graphics& g)
{
	g.fillAll(Colours::white.withAlpha(.05f));
	g.setColour(Colours::white.withAlpha(.2f));
	g.drawRect(getLocalBounds());

	if (managerUI->itemsUI.size() == 0) return;


	juce::Rectangle<int> mr = managerUI->getLocalBounds();
	juce::Rectangle<float> focusRect = managerUI->getViewBounds(mr);

	//AffineTransform af = managerUI->getUITransform(ui);

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

	juce::Rectangle<int> itemRealBounds = managerUI->getBoundsInView(itemViewBounds);


	paintInternal(g, itemViewBounds, itemRealBounds);

	for (auto& ui : managerUI->itemsUI)
	{
		juce::Rectangle<float> b = ui->getBoundsInParent().toFloat();

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
	if ((!isVisible() || getAlpha() < 1) && !ca.isAnimating())
	{
		ca.fadeIn(this, 200);
	}

	stopTimer();
	startTimer(showTime*1000);

	repaint();
}
template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::timerCallback()
{
	ca.fadeOut(this, 200);
	stopTimer();
}
