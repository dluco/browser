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

enum
{
	ENTRY_ACTIVATED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE(BrowserToolbar, browser_toolbar, GTK_TYPE_TOOLBAR)

static void
on_entry_activated(GtkEntry *entry, BrowserToolbar *toolbar)
{
	g_signal_emit(toolbar, signals[ENTRY_ACTIVATED], 0, NULL);
}

static void
browser_toolbar_entry_activated(BrowserToolbar *toolbar)
{
}

static void
browser_toolbar_class_init(BrowserToolbarClass *class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	signals[ENTRY_ACTIVATED] = g_signal_new_class_handler("entry-activated",
			G_TYPE_FROM_CLASS(class),
			G_SIGNAL_RUN_LAST,
			G_CALLBACK(browser_toolbar_entry_activated),
			NULL, NULL, NULL,
			G_TYPE_NONE, 0);

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

static void
browser_toolbar_init(BrowserToolbar *toolbar)
{
	gtk_widget_init_template(GTK_WIDGET(toolbar));

	g_signal_connect(toolbar->entry, "activate", G_CALLBACK(on_entry_activated), toolbar);
}

gchar *
browser_toolbar_get_entry_text(BrowserToolbar *toolbar)
{
	const gchar *text;

	g_return_val_if_fail(BROWSER_IS_TOOLBAR(toolbar), NULL);

	text = gtk_entry_get_text(GTK_ENTRY(toolbar->entry));

	return g_strdup(text);
}

GtkWidget *
browser_toolbar_new(void)
{
	return g_object_new(BROWSER_TYPE_TOOLBAR, NULL);
}
