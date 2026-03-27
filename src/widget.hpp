#pragma once

#include "image_slice.hpp"

namespace browservice {

static constexpr int HandCursor = 0;
static constexpr int NormalCursor = 1;
static constexpr int TextCursor = 2;
static constexpr int CursorTypeCount = 3;

enum class GlobalHotkey {
    Address,
    Find,
    FindNext,
    Refresh,
    ZoomReset,
    ZoomIn,
    ZoomOut
};

class Widget;

class WidgetParent {
public:
    // Event handlers called directly for performance - avoid re-entrancy
    virtual void onWidgetViewDirty() = 0;
    virtual void onWidgetCursorChanged() = 0;
    virtual void onWidgetTakeFocus(Widget* child) {}
    virtual void onGlobalHotkeyPressed(GlobalHotkey key) = 0;
};

class Widget : public WidgetParent {
public:
    Widget(weak_ptr<WidgetParent> parent);

    void setViewport(ImageSlice viewport);
    ImageSlice getViewport();

    void render();

    int cursor();

    // Make this widget focused widget in widget tree
    void takeFocus();

    // Send global mouse event to widget or descendants
    void sendMouseDownEvent(int x, int y, int button);
    void sendMouseUpEvent(int x, int y, int button);
    void sendMouseDoubleClickEvent(int x, int y);
    void sendMouseWheelEvent(int x, int y, int delta);
    void sendMouseMoveEvent(int x, int y);
    void sendMouseEnterEvent(int x, int y);
    void sendMouseLeaveEvent(int x, int y);
    void sendKeyDownEvent(int key);
    void sendKeyUpEvent(int key);
    void sendGainFocusEvent(int x, int y);
    void sendLoseFocusEvent();

    // WidgetParent - (forward events from possible children)
    virtual void onWidgetViewDirty() override;
    virtual void onWidgetCursorChanged() override;
    virtual void onWidgetTakeFocus(Widget* child) override;
    virtual void onGlobalHotkeyPressed(GlobalHotkey key) override;

protected:
    // widget should call this when its view has updated and changes
    // should be rendered
    void signalViewDirty_();

    // widget should call this to update its own cursor - effects might
    // not be immediately visible if mouse is not over this widget
    void setCursor_(int newCursor);

    // Functions to query widget status so that it does not always need its own
    // bookkeeping
    bool isMouseOver_();
    bool isFocused_();
    pair<int, int> getLastMousePos_();

    // Functions to be implemented by widget - 

    // Called after viewport update to update child viewports 
    virtual void widgetViewportUpdated_() {}

    // Render widget to viewport
    virtual void widgetRender_() {}

    // List child widgets for event routing
    virtual vector<shared_ptr<Widget>> widgetListChildren_() {
        return {};
    }

    // Input event handlers - mouse coords local to viewport
    virtual void widgetMouseDownEvent_(int x, int y, int button) {}
    virtual void widgetMouseUpEvent_(int x, int y, int button) {}
    virtual void widgetMouseDoubleClickEvent_(int x, int y) {}
    virtual void widgetMouseWheelEvent_(int x, int y, int delta) {}
    virtual void widgetMouseMoveEvent_(int x, int y) {}
    virtual void widgetMouseEnterEvent_(int x, int y) {}
    virtual void widgetMouseLeaveEvent_(int x, int y) {}
    virtual void widgetKeyDownEvent_(int key) {}
    virtual void widgetKeyUpEvent_(int key) {}
    virtual void widgetGainFocusEvent_(int x, int y) {}
    virtual void widgetLoseFocusEvent_() {}

private:
    void updateFocus_(int x, int y);
    void updateMouseOver_(int x, int y);

    void clearEventState_(int x, int y);

    shared_ptr<Widget> childByPoint_(int x, int y);

    void updateCursor_();

    void forwardMouseDownEvent_(int x, int y, int button);
    void forwardMouseUpEvent_(int x, int y, int button);
    void forwardMouseDoubleClickEvent_(int x, int y);
    void forwardMouseWheelEvent_(int x, int y, int delta);
    void forwardMouseMoveEvent_(int x, int y);
    void forwardMouseEnterEvent_(int x, int y);
    void forwardMouseLeaveEvent_(int x, int y);
    void forwardKeyDownEvent_(int key);
    void forwardKeyUpEvent_(int key);
    void forwardGainFocusEvent_(int x, int y);
    void forwardLoseFocusEvent_();

    weak_ptr<WidgetParent> parent_;
    ImageSlice viewport_;
    bool viewDirty_;

    shared_ptr<Widget> focusChild_;
    shared_ptr<Widget> mouseOverChild_;

    bool mouseOver_;
    bool focused_;

    int lastMouseX_;
    int lastMouseY_;

    set<int> mouseButtonsDown_;
    set<int> keysDown_;

    int cursor_;
    int myCursor_;
};

}
