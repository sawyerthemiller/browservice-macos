#pragma once

#include "rect.hpp"
#include "widget.hpp"

class CefBrowser;
class CefRenderHandler;

namespace browservice {

class BrowserAreaEventHandler {
public:
    virtual void onBrowserAreaViewDirty() = 0;
};

class TextLayout;

// BrowserArea is special widget in sense that it renders continuously
// (outside render() calls) and does not notify of updates using
// WidgetParent -  - onWidgetViewDirty - instead it calls
// BrowserAreaEventHandler -  - onBrowserAreaViewDirty This is to avoid redrawing
// rest of UI every time browser area updates
class BrowserArea :
    public Widget,
    public enable_shared_from_this<BrowserArea>
{
SHARED_ONLY_CLASS(BrowserArea);
public:
    BrowserArea(CKey,
        weak_ptr<WidgetParent> widgetParent,
        weak_ptr<BrowserAreaEventHandler> eventHandler
    );
    ~BrowserArea();

    // Creates new CefRenderHandler than retains pointer to this BrowserArea
    // and paints browser contents to viewport
    CefRefPtr<CefRenderHandler> createCefRenderHandler();

    // Sets browser that will be kept up to date about size changes of
    // this widget browser can be unset by passing null pointer
    void setBrowser(CefRefPtr<CefBrowser> browser);

    // Inform browser again about focus and mouseover status Should be
    // called when loading new page
    void refreshStatusEvents();

    // After calling showError and before clearError browser area switches
    // to special mode in which it only shows given error message
    void showError(string message);
    void clearError();

    // Notify browser area that browser has changed cursor type
    void setCursor(int cursor);

private:
    class RenderHandler;

    virtual void widgetViewportUpdated_() override;

    virtual void widgetMouseDownEvent_(int x, int y, int button) override;
    virtual void widgetMouseUpEvent_(int x, int y, int button) override;
    virtual void widgetMouseDoubleClickEvent_(int x, int y) override;
    virtual void widgetMouseWheelEvent_(int x, int y, int delta) override;
    virtual void widgetMouseMoveEvent_(int x, int y) override;
    virtual void widgetMouseEnterEvent_(int x, int y) override;
    virtual void widgetMouseLeaveEvent_(int x, int y) override;
    virtual void widgetKeyDownEvent_(int key) override;
    virtual void widgetKeyUpEvent_(int key) override;
    virtual void widgetGainFocusEvent_(int x, int y) override;
    virtual void widgetLoseFocusEvent_() override;

    weak_ptr<BrowserAreaEventHandler> eventHandler_;
    CefRefPtr<CefBrowser> browser_;

    bool popupOpen_;
    Rect popupRect_;

    uint32_t eventModifiers_;

    bool errorActive_;
    shared_ptr<TextLayout> errorLayout_;
};

}
