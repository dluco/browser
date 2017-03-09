#include "browser-app-window.h"
#include "browser-app.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <webkit2/webkit2.h>

struct _BrowserAppWindow
{
	GtkApplicationWindow parent;

	GtkWidget *box;
	GtkWidget *entry;
};

G_DEFINE_TYPE(BrowserAppWindow, browser_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
browser_app_window_init(BrowserAppWindow *window)
{
	GtkWidget *web_view;
	char *uri = "https://www.google.ca";

	gtk_widget_init_template(GTK_WIDGET(window));

	web_view = webkit_web_view_new();

	printf("Uri: %s\n", uri);

	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), uri);

	gtk_box_pack_end(GTK_BOX(window->box), web_view, TRUE, TRUE, 0);

	gtk_widget_show(web_view);
}

static void
browser_app_window_class_init(BrowserAppWindowClass *class)
{
	GtkWidgetClass *widget_class;
	GMappedFile *file;
	GBytes *bytes;

	widget_class = GTK_WIDGET_CLASS(class);

	file = g_mapped_file_new("browser-window.ui", FALSE, NULL);
	if (!file) {
		return;
	}
	bytes = g_mapped_file_get_bytes(file);

	gtk_widget_class_set_template(widget_class, bytes);

	g_mapped_file_unref(file);

	gtk_widget_class_bind_template_child(widget_class, BrowserAppWindow, box);
	gtk_widget_class_bind_template_child(widget_class, BrowserAppWindow, entry);
}

BrowserAppWindow *
browser_app_window_new(BrowserApp *app)
{
	return g_object_new(BROWSER_APP_WINDOW_TYPE, "application", app, NULL);
}

void
browser_app_window_open(BrowserAppWindow *window, GFile *file)
{
}
