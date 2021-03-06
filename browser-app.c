#include "browser-app.h"

#include "browser-download-manager.h"
#include "browser-window.h"

#include <webkit2/webkit2.h>

struct _BrowserApp {
	GtkApplication parent;
};

G_DEFINE_TYPE(BrowserApp, browser_app, GTK_TYPE_APPLICATION)

typedef struct _AccelEntry {
	const gchar *detailed_action_name;
	const gchar *accel;
} AccelEntry;

static void
app_activate_quit(GSimpleAction *action,
				  GVariant      *parameter,
				  gpointer       user_data)
{
	BrowserApp *app = BROWSER_APP(user_data);

	g_application_quit(G_APPLICATION(app));
}

static const GActionEntry app_action_entries[] = {
	{ "quit", app_activate_quit },
};

static const AccelEntry app_accel_entries[] = {
	/* App actions. */
	{ "app.quit",			"<Primary>Q" },
	/* Window actions. */
	{ "win.print",			"<Primary>P" },
	{ "win.back",			"<Alt>Left" },
	{ "win.forward",		"<Alt>Right" },
	{ "win.new-tab",		"<Primary>T" },
	{ "win.close-tab",		"<Primary>W" },
	{ "win.undo-close-tab",	"<Primary><Shift>T" },
	{ "win.fullscreen",		"F11" },
};

static void
on_download_started(WebKitWebContext *web_context,
					WebKitDownload *download,
					BrowserApp *app)
{
	g_print("App: download started\n");

	browser_download_manager_add_download(browser_download_manager_get_default(), download);
}

static void
browser_app_startup(GApplication *g_application)
{
	BrowserApp *app = BROWSER_APP(g_application);
	GtkBuilder *builder;
	GMenuModel *menubar;
	WebKitWebContext *web_context;
	WebKitCookieManager *cookie_manager;
	gchar *path;

	G_APPLICATION_CLASS(browser_app_parent_class)->startup(g_application);

	builder = gtk_builder_new_from_file("menus.ui");
	menubar = G_MENU_MODEL(gtk_builder_get_object(builder, "menubar"));
	gtk_application_set_menubar(GTK_APPLICATION(g_application), menubar);
	g_object_unref(builder);

	g_action_map_add_action_entries(G_ACTION_MAP(app),
									app_action_entries,
									G_N_ELEMENTS(app_action_entries),
									app);

	/* Associate accelerators with actions. */
	for (int i = 0; i < G_N_ELEMENTS(app_accel_entries); i++) {
		const gchar *accels[] = { app_accel_entries[i].accel, NULL };
		gtk_application_set_accels_for_action(GTK_APPLICATION(app),
											  app_accel_entries[i].detailed_action_name,
											  accels);
	}

	web_context = webkit_web_context_get_default();

	g_signal_connect(web_context, "download-started", G_CALLBACK(on_download_started), app);

	/* Set web context settings. */
	/* TODO: Move to a util so the same settings are used for ephemeral contexts. */
	webkit_web_context_set_process_model(web_context, WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);

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
browser_app_open(GApplication *g_application,
				 GFile       **files,
				 gint          n_files,
				 const gchar  *hint)
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
