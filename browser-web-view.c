#include "browser-web-view.h"

struct _BrowserWebView
{
	WebKitWebView parent;
};

G_DEFINE_TYPE(BrowserWebView, browser_web_view, WEBKIT_TYPE_WEB_VIEW);

static void
browser_web_view_init(BrowserWebView *web_view)
{
}

static void
browser_web_view_class_init(BrowserWebViewClass *class)
{
}

GtkWidget *
browser_web_view_new(BrowserApp *app)
{
	return g_object_new(BROWSER_WEB_VIEW_TYPE, NULL);
}
