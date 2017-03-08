#include "browser-app.h"
#include "browser-app-window.h"
#include <gtk/gtk.h>

struct _BrowserApp
{
	GtkApplication parent;
};

G_DEFINE_TYPE(BrowserApp, browser_app, GTK_TYPE_APPLICATION);

static void
browser_app_init(BrowserApp *app)
{

}

static void
browser_app_activate(GApplication *g_app)
{
	BrowserAppWindow *window;

	window = browser_app_window_new(BROWSER_APP(g_app));
	gtk_window_present(GTK_WINDOW(window));
}

static void
browser_app_open(GApplication *g_app, GFile **files, gint n_files, const gchar *hint)
{
	GList *windows;
	BrowserAppWindow *window;
	int i;

	windows = gtk_application_get_windows(GTK_APPLICATION(g_app));
	if (windows)
		window = BROWSER_APP_WINDOW(windows->data);
	else
		window = browser_app_window_new(BROWSER_APP(g_app));

	for (i = 0; i < n_files; i++)
		browser_app_window_open(window, files[i]);

	gtk_window_present(GTK_WINDOW(window));
}

static void
browser_app_class_init(BrowserAppClass *class)
{
	G_APPLICATION_CLASS(class)->activate = browser_app_activate;
	G_APPLICATION_CLASS(class)->open = browser_app_open;
}

BrowserApp *
browser_app_new(void)
{
	return g_object_new(BROWSER_APP_TYPE,
			"application-id", "com.example.browserapp",
			"flags", G_APPLICATION_HANDLES_OPEN,
			NULL);
}
