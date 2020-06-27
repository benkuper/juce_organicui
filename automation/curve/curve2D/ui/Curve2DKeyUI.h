/*
  ==============================================================================

    Curve2DKeyUI.h
    Created: 21 Mar 2020 4:06:36pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Curve2DKeyHandle :
    public InspectableContentComponent
{
public:
    Curve2DKeyHandle(Curve2DKey* key);
    ~Curve2DKeyHandle();

    Curve2DKey* key;
    
    void paint(Graphics& g) override;

    void inspectableSelectionChanged(Inspectable* i) override;
};

class Curve2DKeyUI :
    public BaseItemMinimalUI<Curve2DKey>
{
public:
    Curve2DKeyUI(Curve2DKey* key);
    ~Curve2DKeyUI();

    Curve2DKeyHandle handle;

    Rectangle<float> valueBounds;

    std::unique_ptr<Easing2DUI> easingUI;

    void resized() override;
    void paint(Graphics& g) override;

    void setShowEasingHandles(bool showFirst, bool showLast);

    void updateEasingUI();

    void mouseDown(const MouseEvent& e) override;
    void mouseDoubleClick(const MouseEvent& e) override;
    
    bool canStartDrag(const MouseEvent& e) override;
    Point<int> getDragOffset() override;

    bool hitTest(int x, int y) override;

    void setValueBounds(const Rectangle<float> valueBounds);
    Point<int> getUIPosForValuePos(const Point<float>& valuePos) const;

    void controllableFeedbackUpdateInternal(Controllable* c) override;


    class KeyUIListener
    {
    public:
        virtual ~KeyUIListener() {}
        virtual void keyEasingHandleMoved(Curve2DKeyUI * key, bool syncOtherHandle, bool isFirstHandle) {}
    };

    ListenerList<KeyUIListener> keyUIListeners;
    void addKeyUIListener(KeyUIListener* newListener) { keyUIListeners.add(newListener); }
    void removeKeyUIListener(KeyUIListener* listener) { keyUIListeners.remove(listener); }

};