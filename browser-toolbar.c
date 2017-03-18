#include "browser-toolbar.h"
#include <glib-object.h>
#include <glib.h>
#include <glib/gstdio.h>

struct _BrowserToolbar
{
	GtkToolbar parent;

	GtkWidget *back;
	GtkWidget *forward;
	GtkWidget *entry;
	GtkWidget *home;
};

G_DEFINE_TYPE(BrowserToolbar, browser_toolbar, GTK_TYPE_TOOLBAR);

static void
browser_toolbar_init(BrowserToolbar *toolbar)
{
	gtk_widget_init_template(GTK_WIDGET(toolbar));
}

static void
browser_toolbar_class_init(BrowserToolbarClass *class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	file = g_mapped_file_new("browser-toolbar.ui", FALSE, NULL);
	if (!file) {
		return;
	}
	bytes = g_mapped_file_get_bytes(file);
	gtk_widget_class_set_template(widget_class, bytes);
	g_mapped_file_unref(file);

	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, back);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, forward);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, entry);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, home);
}

GtkWidget *
browser_toolbar_new(void)
{
	return g_object_new(BROWSER_TYPE_TOOLBAR, NULL);
}
