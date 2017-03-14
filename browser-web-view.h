#ifndef _BROWSER_WEB_VIEW_H_
#define _BROWSER_WEB_VIEW_H_

#include <webkit2/webkit2.h>

#define BROWSER_WEB_VIEW_TYPE (browser_web_view_get_type())
G_DECLARE_FINAL_TYPE(BrowserWebView, browser_web_view, BROWSER, WEB_VIEW, WebKitWebView)

GtkWidget *browser_web_view_new(void);

#endif /* _BROWSER_WEB_VIEW_H_ */
