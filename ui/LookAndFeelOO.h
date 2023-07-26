/*
  ==============================================================================

    LookAndFeelOO.h
    Created: 3 Mar 2016 10:21:45am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#pragma warning( disable : 4505 )

//keep
namespace LookAndFeelHelpers {


    // initialise the standard set of colours..
    static const juce::uint32 textButtonColour = NORMAL_COLOR.getARGB();
    static const juce::uint32 textHighlightColour = TEXT_COLOR.getARGB();
    static const juce::uint32 standardOutlineColour = 0xb2808080;

	static const juce::uint32 standardColours[] =
	{
        juce::TextButton::buttonColourId,                 textButtonColour,
        juce::TextButton::buttonOnColourId,               HIGHLIGHT_COLOR.darker().getARGB(),
        juce::TextButton::textColourOnId,                 textHighlightColour,
        juce::TextButton::textColourOffId,                TEXT_COLOR.getARGB(),

        juce::ToggleButton::textColourId,                 TEXT_COLOR.getARGB(),

        juce::TextEditor::backgroundColourId,             0xffffffff,
        juce::TextEditor::textColourId,                   0xff000000,
        juce::TextEditor::highlightColourId,              textHighlightColour,
        juce::TextEditor::highlightedTextColourId,        0xff000000,
        juce::TextEditor::outlineColourId,                0x00000000,
        juce::TextEditor::focusedOutlineColourId,         textButtonColour,
        juce::TextEditor::shadowColourId,                 0x38000000,

        juce::CaretComponent::caretColourId,              0xff000000,

        juce::Label::backgroundColourId,                  0x00000000,
        juce::Label::textColourId,                        TEXT_COLOR.getARGB(),
        juce::Label::outlineColourId,                     0x00000000,

        juce::ScrollBar::backgroundColourId,              BG_COLOR.brighter(.2f).getARGB(),
        juce::ScrollBar::thumbColourId,                   BG_COLOR.brighter(.6f).getARGB(),
        juce::ScrollBar::trackColourId,                   juce::Colours::red.getARGB(),

        juce::TreeView::linesColourId,                    0x4c000000,
        juce::TreeView::backgroundColourId,               0x00000000,
        juce::TreeView::dragAndDropIndicatorColourId,     0x80ff0000,
        juce::TreeView::selectedItemBackgroundColourId,   0x00000000,
        juce::TreeView::oddItemsColourId,					0,
        juce::TreeView::evenItemsColourId,				0,

        juce::PopupMenu::backgroundColourId,              BG_COLOR.getARGB(),
        juce::PopupMenu::textColourId,                    TEXT_COLOR.getARGB(),
        juce::PopupMenu::headerTextColourId,              TEXT_COLOR.darker().getARGB(),
        juce::PopupMenu::highlightedTextColourId,         BG_COLOR.getARGB(),
        juce::PopupMenu::highlightedBackgroundColourId,   HIGHLIGHT_COLOR.getARGB(),

        juce::ComboBox::buttonColourId,                   0xffCCCCCC,
        juce::ComboBox::outlineColourId,                  standardOutlineColour,
        juce::ComboBox::textColourId,                     0xff666666,
        juce::ComboBox::backgroundColourId,               0xffCCCCCC,
        juce::ComboBox::arrowColourId,                    0x99000000,

        juce::PropertyComponent::backgroundColourId,      0x66ffffff,
        juce::PropertyComponent::labelTextColourId,       0xff000000,

        juce::TextPropertyComponent::backgroundColourId,  0xffffffff,
        juce::TextPropertyComponent::textColourId,        0xff000000,
        juce::TextPropertyComponent::outlineColourId,     standardOutlineColour,

        juce::BooleanPropertyComponent::backgroundColourId, 0xffffffff,
        juce::BooleanPropertyComponent::outlineColourId,  standardOutlineColour,

        juce::ListBox::backgroundColourId,                0xffffffff,
        juce::ListBox::outlineColourId,                   standardOutlineColour,
        juce::ListBox::textColourId,                      0xff000000,

		juce::Slider::backgroundColourId,                 0x00000000,
		juce::Slider::thumbColourId,                      textButtonColour,
		juce::Slider::trackColourId,                      0x7fffffff,
		juce::Slider::rotarySliderFillColourId,           0x7f0000ff,
		juce::Slider::rotarySliderOutlineColourId,        0x66000000,
		juce::Slider::textBoxTextColourId,                TEXT_COLOR.getARGB(),
		juce::Slider::textBoxBackgroundColourId,          BG_COLOR.darker(.1f).getARGB(),
		juce::Slider::textBoxHighlightColourId,           textHighlightColour,
		juce::Slider::textBoxOutlineColourId,             BG_COLOR.brighter(.1f).getARGB() ,

        juce::ResizableWindow::backgroundColourId,        BG_COLOR.brighter(.1f).getARGB(),
        //DocumentWindow::textColourId,               0xff000000, // (this is deliberately not set)

        juce::AlertWindow::backgroundColourId,            BG_COLOR.getARGB(),
        juce::AlertWindow::textColourId,                  TEXT_COLOR.getARGB(),
        juce::AlertWindow::outlineColourId,               BG_COLOR.brighter(.2f).getARGB(),

        juce::ProgressBar::backgroundColourId,            0xffeeeeee,
        juce::ProgressBar::foregroundColourId,            0xffaaaaee,

        juce::TooltipWindow::backgroundColourId,          0xffeeeebb,
        juce::TooltipWindow::textColourId,                0xff000000,
        juce::TooltipWindow::outlineColourId,             0x4c000000,

        juce::TabbedComponent::backgroundColourId,        0x00000000,
        juce::TabbedComponent::outlineColourId,           0xff777777,
        juce::TabbedButtonBar::tabOutlineColourId,        0x80000000,
        juce::TabbedButtonBar::frontOutlineColourId,      0x90000000,

        juce::Toolbar::backgroundColourId,                0xfff6f8f9,
        juce::Toolbar::separatorColourId,                 0x4c000000,
        juce::Toolbar::buttonMouseOverBackgroundColourId, 0x4c0000ff,
        juce::Toolbar::buttonMouseDownBackgroundColourId, 0x800000ff,
        juce::Toolbar::labelTextColourId,                 0xff000000,
        juce::Toolbar::editingModeOutlineColourId,        0xffff0000,

        juce::DrawableButton::textColourId,               0xff000000,
        juce::DrawableButton::textColourOnId,             0xff000000,
        juce::DrawableButton::backgroundColourId,         0x00000000,
        juce::DrawableButton::backgroundOnColourId,       0xaabbbbff,

        juce::HyperlinkButton::textColourId,              0xcc1111ee,

        juce::GroupComponent::outlineColourId,            0x66000000,
        juce::GroupComponent::textColourId,               0xff000000,

        juce::BubbleComponent::backgroundColourId,        0xeeeeeebb,
        juce::BubbleComponent::outlineColourId,           0x77000000,

        juce::DirectoryContentsDisplayComponent::highlightColourId,   textHighlightColour,
        juce::DirectoryContentsDisplayComponent::textColourId,        0xff000000,

        0x1000440, /*LassoComponent::lassoFillColourId*/        0x66dddddd,
        0x1000441, /*LassoComponent::lassoOutlineColourId*/     0x99111111,

        0x1005000, /*MidiKeyboardComponent::whiteNoteColourId*/               0xffffffff,
        0x1005001, /*MidiKeyboardComponent::blackNoteColourId*/               0xff000000,
        0x1005002, /*MidiKeyboardComponent::keySeparatorLineColourId*/        0x66000000,
        0x1005003, /*MidiKeyboardComponent::mouseOverKeyOverlayColourId*/     0x80ffff00,
        0x1005004, /*MidiKeyboardComponent::keyDownOverlayColourId*/          0xffb6b600,
        0x1005005, /*MidiKeyboardComponent::textLabelColourId*/               0xff000000,
        0x1005006, /*MidiKeyboardComponent::upDownButtonBackgroundColourId*/  0xffd3d3d3,
        0x1005007, /*MidiKeyboardComponent::upDownButtonArrowColourId*/       0xff000000,
        0x1005008, /*MidiKeyboardComponent::shadowColourId*/                  0x4c000000,

        0x1004500, /*CodeEditorComponent::backgroundColourId*/                0xffffffff,
        0x1004502, /*CodeEditorComponent::highlightColourId*/                 textHighlightColour,
        0x1004503, /*CodeEditorComponent::defaultTextColourId*/               0xff000000,
        0x1004504, /*CodeEditorComponent::lineNumberBackgroundId*/            0x44999999,
        0x1004505, /*CodeEditorComponent::lineNumberTextId*/                  0x44000000,

        0x1007000, /*ColourSelector::backgroundColourId*/                     0xffe5e5e5,
        0x1007001, /*ColourSelector::labelTextColourId*/                      0xff000000,

        0x100ad00, /*KeyMappingEditorComponent::backgroundColourId*/          0x00000000,
        0x100ad01, /*KeyMappingEditorComponent::textColourId*/                0xff000000,

        juce::FileSearchPathListComponent::backgroundColourId,        0xffffffff,

        juce::FileChooserDialogBox::titleTextColourId,                0xff000000,
    };



	
    static inline juce::Colour createBaseColour(juce::Colour buttonColour,
        bool hasKeyboardFocus,
        bool isMouseOverButton,
        bool isButtonDown) noexcept
    {
        const float sat = hasKeyboardFocus ? 1.3f : 0.9f;
        const juce::Colour baseColour(buttonColour.withMultipliedSaturation(sat));

        if (isButtonDown)      return baseColour.contrasting(0.2f);
        if (isMouseOverButton) return baseColour.contrasting(0.1f);

        return baseColour;
    }

    static inline juce::TextLayout layoutTooltipText(const juce::String& text, juce::Colour colour) noexcept
    {
        const float tooltipFontSize = 13.0f;
        const int maxToolTipWidth = 400;

        juce::AttributedString s;
        s.setJustification(juce::Justification::centred);
        s.append(text, juce::Font(tooltipFontSize, juce::Font::bold), colour);

        juce::TextLayout tl;
        tl.createLayoutWithBalancedLineLengths(s, (float)maxToolTipWidth);
        return tl;
    }
    

};


class LookAndFeelOO:
	public juce::LookAndFeel_V4
{
public:
//  juce_DeclareSingleton(LookAndFeelOO, true);
    LookAndFeelOO();
    ~LookAndFeelOO();
    //==============================================================================
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&, bool isMouseOverButton, bool isButtonDown) override;
    int getTextButtonWidthToFitText (juce::TextButton&, int buttonHeight) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool isMouseOverButton, bool isButtonDown) override;

    void changeToggleButtonWidthToFitText (juce::ToggleButton&) override;

    void drawTickBox (juce::Graphics&, juce::Component&,
                      float x, float y, float w, float h,
                      bool ticked, bool isEnabled, bool isMouseOverButton, bool isButtonDown) override;

    void drawDrawableButton (juce::Graphics&, juce::DrawableButton&, bool isMouseOverButton, bool isButtonDown) override;

    //==============================================================================
    juce::AlertWindow* createAlertWindow (const juce::String& title, const juce::String& message,
                                    const juce::String& button1,
                                    const juce::String& button2,
                                    const juce::String& button3,
                                    juce::AlertWindow::AlertIconType iconType,
                                    int numButtons, juce::Component* associatedComponent) override;

    void drawAlertBox (juce::Graphics&, juce::AlertWindow&, const juce::Rectangle<int>& textArea, juce::TextLayout&) override;
    int getAlertBoxWindowFlags() override;


    juce::Array<int> getWidthsForTextButtons(juce::AlertWindow &, const juce::Array<juce::TextButton *> & buttons) override
	{
        juce::Array<int> w;
		
		for (int i = 0; i<buttons.size(); ++i)
		{
			w.add(juce::jmax<int>(buttons.getUnchecked(i)->getWidth(),60));
		}

		return w;
	}

    int getAlertWindowButtonHeight() override;

    /** Override this function to supply a custom font for the alert window title.
     This default implementation will use a boldened and slightly larger version
     of the alert window message font.

     @see getAlertWindowMessageFont.
     */
    juce::Font getAlertWindowTitleFont() override;

    /** Override this function to supply a custom font for the alert window message.
     This default implementation will use the default font with height set to 15.0f.

     @see getAlertWindowTitleFont
     */
    juce::Font getAlertWindowMessageFont() override;

    juce::Font getAlertWindowFont() override;

    //==============================================================================
    void drawProgressBar (juce::Graphics&, juce::ProgressBar&, int width, int height, double progress, const juce::String& textToShow) override;
    void drawSpinningWaitAnimation (juce::Graphics&, const juce::Colour& colour, int x, int y, int w, int h) override;

    //==============================================================================
    bool areScrollbarButtonsVisible() override;
    void drawScrollbarButton (juce::Graphics&, juce::ScrollBar&, int width, int height, int buttonDirection,
                              bool isScrollbarVertical, bool isMouseOverButton, bool isButtonDown) override;

    void drawScrollbar (juce::Graphics&, juce::ScrollBar&, int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                        bool isMouseOver, bool isMouseDown) override;

    juce::ImageEffectFilter* getScrollbarEffect() override;
    int getMinimumScrollbarThumbSize (juce::ScrollBar&) override;
    int getDefaultScrollbarWidth() override;
    int getScrollbarButtonSize (juce::ScrollBar&) override;

    //==============================================================================
    juce::Path getTickShape (float height) override;
    juce::Path getCrossShape (float height) override;

    //==============================================================================
    void drawTreeviewPlusMinusBox (juce::Graphics&, const juce::Rectangle<float>& area,
                                   juce::Colour backgroundColour, bool isOpen, bool isMouseOver) override;
    bool areLinesDrawnForTreeView (juce::TreeView&) override;
    int getTreeViewIndentSize (juce::TreeView&) override;

    //==============================================================================
    void fillTextEditorBackground (juce::Graphics&, int width, int height, juce::TextEditor&) override;
    void drawTextEditorOutline (juce::Graphics&, int width, int height, juce::TextEditor&) override;
    juce::CaretComponent* createCaretComponent (juce::Component* keyFocusOwner) override;


    juce::AttributedString createFileChooserHeaderText (const juce::String& title, const juce::String& instructions) override;

	// Inherited via LookAndFeel
	virtual void drawFileBrowserRow(juce::Graphics &, int width, int height, const juce::File & file, const juce::String & filename, juce::Image * optionalIcon, const juce::String & fileSizeDescription, const juce::String & fileTimeDescription, bool isDirectory, bool isItemSelected, int itemIndex, juce::DirectoryContentsDisplayComponent &) override;

    juce::Button* createFileBrowserGoUpButton() override;

    void layoutFileBrowserComponent (juce::FileBrowserComponent&,
                                     juce::DirectoryContentsDisplayComponent*,
                                     juce::FilePreviewComponent*,
                                     juce::ComboBox* currentPathBox,
                                     juce::TextEditor* filenameBox,
                                     juce::Button* goUpButton) override;

    //==============================================================================
    void drawBubble (juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>& tip, const juce::Rectangle<float>& body) override;

    void drawLasso (juce::Graphics&, juce::Component&) override;

    //==============================================================================

    void preparePopupMenuWindow (juce::Component& newWindow) override ;
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;

    void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                            const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override;

    void drawPopupMenuSectionHeader (juce::Graphics&, const juce::Rectangle<int>& area,
                                     const juce::String& sectionName) override;

    juce::Font getPopupMenuFont() override;

    void drawPopupMenuUpDownArrow (juce::Graphics&, int width, int height, bool isScrollUpArrow) override;

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override;
    int getMenuWindowFlags() override;

    void drawMenuBarBackground (juce::Graphics&, int width, int height, bool isMouseOverBar, juce::MenuBarComponent&) override;
    int getMenuBarItemWidth (juce::MenuBarComponent&, int itemIndex, const juce::String& itemText) override;
    juce::Font getMenuBarFont (juce::MenuBarComponent&, int itemIndex, const juce::String& itemText) override;
    int getDefaultMenuBarHeight() override;

    void drawMenuBarItem (juce::Graphics&, int width, int height,
                          int itemIndex, const juce::String& itemText,
                          bool isMouseOverItem, bool isMenuOpen, bool isMouseOverBar,
                          juce::MenuBarComponent&) override;

    juce::Component* getParentComponentForMenuOptions (const juce::PopupMenu::Options& options) override;
    //==============================================================================
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Label* createComboBoxTextBox (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    //==============================================================================
    void drawLabel (juce::Graphics&, juce::Label&) override;
    juce::Font getLabelFont (juce::Label&) override;

    //==============================================================================
    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle, juce::Slider&) override;

    void drawLinearSliderBackground (juce::Graphics&, int x, int y, int width, int height,
                                     float sliderPos, float minSliderPos, float maxSliderPos,
                                     const juce::Slider::SliderStyle, juce::Slider&) override;

    void drawLinearSliderThumb (juce::Graphics&, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const juce::Slider::SliderStyle, juce::Slider&) override;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    int getSliderThumbRadius (juce::Slider&) override;
    juce::Button* createSliderButton(juce::Slider&, bool isIncrement) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;
    juce::ImageEffectFilter* getSliderEffect(juce::Slider&) override;
    juce::Font getSliderPopupFont(juce::Slider&) override;
    int getSliderPopupPlacement(juce::Slider&) override;
    juce::Slider::SliderLayout getSliderLayout(juce::Slider&) override;

    //==============================================================================
    juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;

    //==============================================================================
    juce::Button* createFilenameComponentBrowseButton (const juce::String& text) override;
    void layoutFilenameComponent (juce::FilenameComponent&, juce::ComboBox* filenameBox, juce::Button* browseButton) override;

    //==============================================================================
    void drawConcertinaPanelHeader (juce::Graphics&, const juce::Rectangle<int>& area,
                                    bool isMouseOver, bool isMouseDown,
                                    juce::ConcertinaPanel&, juce::Component& panel) override;

    //==============================================================================
    void drawCornerResizer (juce::Graphics&, int w, int h, bool isMouseOver, bool isMouseDragging) override;
    void drawResizableFrame (juce::Graphics&, int w, int h, const juce::BorderSize<int>&) override;

    //==============================================================================
    void fillResizableWindowBackground (juce::Graphics&, int w, int h, const juce::BorderSize<int>&, juce::ResizableWindow&) override;
    void drawResizableWindowBorder(juce::Graphics&, int w, int h, const juce::BorderSize<int>& border, juce::ResizableWindow&) override;

    //==============================================================================
    void drawDocumentWindowTitleBar (juce::DocumentWindow&, juce::Graphics&, int w, int h,
                                     int titleSpaceX, int titleSpaceW,
                                     const juce::Image* icon, bool drawTitleTextOnLeft) override;

    juce::Button* createDocumentWindowButton (int buttonType) override;

    void positionDocumentWindowButtons (juce::DocumentWindow&,
                                        int titleBarX, int titleBarY, int titleBarW, int titleBarH,
                                        juce::Button* minimiseButton,
                                        juce::Button* maximiseButton,
                                        juce::Button* closeButton,
                                        bool positionTitleBarButtonsOnLeft) override;

    //==============================================================================

    //==============================================================================
    void drawStretchableLayoutResizerBar (juce::Graphics&, int w, int h, bool isVerticalBar,
                                          bool isMouseOver, bool isMouseDragging) override;

    //==============================================================================
    void drawGroupComponentOutline (juce::Graphics&, int w, int h, const juce::String& text,
                                    const juce::Justification&, juce::GroupComponent&) override;

    //==============================================================================
    int getTabButtonSpaceAroundImage() override;
    int getTabButtonOverlap (int tabDepth) override;
    int getTabButtonBestWidth (juce::TabBarButton&, int tabDepth) override;
    juce::Rectangle<int> getTabButtonExtraComponentBounds (const juce::TabBarButton&, juce::Rectangle<int>& textArea, juce::Component& extraComp) override;

    void drawTabButton (juce::TabBarButton&, juce::Graphics&, bool isMouseOver, bool isMouseDown) override;
    void drawTabButtonText (juce::TabBarButton&, juce::Graphics&, bool isMouseOver, bool isMouseDown) override;
    void drawTabbedButtonBarBackground (juce::TabbedButtonBar&, juce::Graphics&) override;
    void drawTabAreaBehindFrontButton (juce::TabbedButtonBar&, juce::Graphics&, int w, int h) override;

    void createTabButtonShape (juce::TabBarButton&, juce::Path&,  bool isMouseOver, bool isMouseDown) override;
    void fillTabButtonShape (juce::TabBarButton&, juce::Graphics&, const juce::Path&, bool isMouseOver, bool isMouseDown) override;

    juce::Button* createTabBarExtrasButton() override;

    //==============================================================================
    void drawImageButton (juce::Graphics&, juce::Image*,
                          int imageX, int imageY, int imageW, int imageH,
                          const juce::Colour& overlayColour, float imageOpacity, juce::ImageButton&) override;

    //==============================================================================
    void drawTableHeaderBackground (juce::Graphics&, juce::TableHeaderComponent&) override;


	void drawTableHeaderColumn(juce::Graphics &, juce::TableHeaderComponent &, const juce::String & columnName, int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags) override;

    //==============================================================================
    void paintToolbarBackground (juce::Graphics&, int width, int height, juce::Toolbar&) override;

    juce::Button* createToolbarMissingItemsButton (juce::Toolbar&) override;

    void paintToolbarButtonBackground (juce::Graphics&, int width, int height,
                                       bool isMouseOver, bool isMouseDown,
                                       juce::ToolbarItemComponent&) override;

    void paintToolbarButtonLabel (juce::Graphics&, int x, int y, int width, int height,
                                  const juce::String& text, juce::ToolbarItemComponent&) override;

    //==============================================================================
    void drawPropertyPanelSectionHeader (juce::Graphics&, const juce::String& name, bool isOpen, int width, int height) override;
    void drawPropertyComponentBackground (juce::Graphics&, int width, int height, juce::PropertyComponent&) override;
    void drawPropertyComponentLabel (juce::Graphics&, int width, int height, juce::PropertyComponent&) override;
    juce::Rectangle<int> getPropertyComponentContentPosition (juce::PropertyComponent&) override;

    //==============================================================================
    void drawCallOutBoxBackground (juce::CallOutBox&, juce::Graphics&, const juce::Path& path, juce::Image& cachedImage) override;
    int getCallOutBoxBorderSize (const juce::CallOutBox&) override;

    //==============================================================================
    void drawLevelMeter (juce::Graphics&, int width, int height, float level) override;

    void drawKeymapChangeButton (juce::Graphics&, int width, int height, juce::Button&, const juce::String& keyDescription) override;

    //==============================================================================
    /** Draws a 3D raised (or indented) bevel using two colours.

     The bevel is drawn inside the given rectangle, and greater bevel thicknesses
     extend inwards.

     The top-left colour is used for the top- and left-hand edges of the
     bevel; the bottom-right colour is used for the bottom- and right-hand
     edges.

     If useGradient is true, then the bevel fades out to make it look more curved
     and less angular. If sharpEdgeOnOutside is true, the outside of the bevel is
     sharp, and it fades towards the centre; if sharpEdgeOnOutside is false, then
     the centre edges are sharp and it fades towards the outside.
     */
    static void drawBevel (juce::Graphics&,
                           int x, int y, int width, int height,
                           int bevelThickness,
                           const juce::Colour& topLeftColour = juce::Colours::white,
                           const juce::Colour& bottomRightColour = juce::Colours::black,
                           bool useGradient = true,
                           bool sharpEdgeOnOutside = true);

    /** Utility function to draw a shiny, glassy circle (for round LED-type buttons). */
    static void drawGlassSphere (juce::Graphics&, float x, float y, float diameter,
                                 const juce::Colour&, float outlineThickness) noexcept;

    static void drawGlassPointer (juce::Graphics&, float x, float y, float diameter,
                                  const juce::Colour&, float outlineThickness, int direction) noexcept;

    /** Utility function to draw a shiny, glassy oblong (for text buttons). */
    static void drawGlassLozenge (juce::Graphics&,
                                  float x, float y, float width, float height,
                                  const juce::Colour&, float outlineThickness, float cornerSize,
                                  bool flatOnLeft, bool flatOnRight, bool flatOnTop, bool flatOnBottom) noexcept;

private:
    //==============================================================================
    std::unique_ptr<juce::Drawable> folderImage, documentImage;

    void drawShinyButtonShape (juce::Graphics&,
                               float x, float y, float w, float h, float maxCornerSize,
                               const juce::Colour&, float strokeWidth,
                               bool flatOnLeft, bool flatOnRight, bool flatOnTop, bool flatOnBottom) noexcept;

    class GlassWindowButton;
    class SliderLabelComp;

	// Inherited via LookAndFeel
#if JUCE_MAJOR_VERSION >= 5
	virtual bool isProgressBarOpaque(juce::ProgressBar &) override;
#endif
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeelOO)


		


		
	virtual bool shouldPopupMenuScaleWithTargetComponent(const juce::PopupMenu::Options & options) override;


	// Inherited via LookAndFeel
	virtual int getPopupMenuBorderSize() override;

	virtual juce::Font getTabButtonFont(juce::TabBarButton &, float height) override;

};
