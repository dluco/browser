#include "browser-app.h"
#include "browser-window.h"
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

struct _BrowserApp
{
	GtkApplication parent;
};

G_DEFINE_TYPE(BrowserApp, browser_app, GTK_TYPE_APPLICATION)

static void
browser_app_startup(GApplication *g_application)
{
	WebKitWebContext *web_context;
	WebKitCookieManager *cookie_manager;
	gchar *path;

	G_APPLICATION_CLASS(browser_app_parent_class)->startup(g_application);

	web_context = webkit_web_context_get_default();

	/* Set and enable favicon database. */
	path = g_build_filename(g_get_user_data_dir(), "browser", "icondatabase", NULL);
	webkit_web_context_set_favicon_database_directory(web_context, path);
	g_free(path);

	cookie_manager = webkit_web_context_get_cookie_manager(web_context);
	webkit_cookie_manager_set_persistent_storage(cookie_manager, "cookies.sqlite", WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);
}

static void
browser_app_activate(GApplication *g_application)
{
	BrowserApp *app = BROWSER_APP(g_application);
	BrowserWindow *window;

	window = browser_window_new(app);

	browser_window_open(window, "http://www.google.ca");

	gtk_window_present(GTK_WINDOW(window));
}

static void
browser_app_open(GApplication *g_application, GFile **files, gint n_files, const gchar *hint)
{
	BrowserApp *app = BROWSER_APP(g_application);
	GList *windows;
	BrowserWindow *window;
//	int i;

	windows = gtk_application_get_windows(GTK_APPLICATION(g_application));
	if (windows)
		window = BROWSER_WINDOW(windows->data);
	else
		window = browser_window_new(app);

//	for (i = 0; i < n_files; i++)
//		browser_window_open(window, files[i]);

	gtk_window_present(GTK_WINDOW(window));
}

static void
browser_app_class_init(BrowserAppClass *class)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS(class);

	app_class->startup = browser_app_startup;
	app_class->activate = browser_app_activate;
	app_class->open = browser_app_open;
}

static void
browser_app_init(BrowserApp *app)
{
}

BrowserApp *
browser_app_new(void)
{
	return g_object_new(BROWSER_TYPE_APP,
			"application-id", "com.example.browserapp",
			"flags", G_APPLICATION_HANDLES_OPEN,
			NULL);
}
