#include "browser-app-window.h"
#include "browser-app.h"
#include <gtk/gtk.h>

struct _BrowserAppWindow
{
	GtkApplicationWindow parent;
};

G_DEFINE_TYPE(BrowserAppWindow, browser_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
browser_app_window_init(BrowserAppWindow *window)
{
}

static void
browser_app_window_class_init(BrowserAppWindowClass *class)
{
}

BrowserAppWindow *
browser_app_window_new(BrowserApp *app)
{
	return g_object_new(BROWSER_APP_WINDOW_TYPE, "application", app, NULL);
}

void
browser_app_open(BrowserAppWindow *window, GFile *file)
{
}
