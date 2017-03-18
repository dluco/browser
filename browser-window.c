#include "browser-window.h"
#include "browser-app.h"
#include "browser-notebook.h"
#include "browser-toolbar.h"
#include "browser-web-view.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <webkit2/webkit2.h>

struct _BrowserWindow
{
	GtkApplicationWindow parent;

	GtkWidget *box;
	GtkWidget *toolbar;
	GtkWidget *notebook;
};

G_DEFINE_TYPE(BrowserWindow, browser_window, GTK_TYPE_APPLICATION_WINDOW);

static void
browser_window_init(BrowserWindow *window)
{
	g_type_ensure(BROWSER_TYPE_TOOLBAR);
	g_type_ensure(BROWSER_TYPE_NOTEBOOK);

	gtk_widget_init_template(GTK_WIDGET(window));
}

static void
browser_window_class_init(BrowserWindowClass *class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	file = g_mapped_file_new("browser-window.ui", FALSE, NULL);
	if (!file) {
		return;
	}
	bytes = g_mapped_file_get_bytes(file);

	gtk_widget_class_set_template(widget_class, bytes);

	g_mapped_file_unref(file);

	gtk_widget_class_bind_template_child(widget_class, BrowserWindow, box);
	gtk_widget_class_bind_template_child(widget_class, BrowserWindow, toolbar);
	gtk_widget_class_bind_template_child(widget_class, BrowserWindow, notebook);
}

BrowserWindow *
browser_window_new(BrowserApp *app)
{
	return g_object_new(BROWSER_TYPE_WINDOW, "application", app, NULL);
}

void
browser_window_open(BrowserWindow *window, GFile *file)
{
}
