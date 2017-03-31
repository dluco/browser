#include "browser-toolbar.h"
#include <glib-object.h>
#include <glib.h>
#include <glib/gstdio.h>

struct _BrowserToolbar
{
	GtkToolbar parent;

	GtkWidget *back_button;
	GtkWidget *forward_button;
	GtkWidget *stop_reload_button;
	GtkWidget *entry;
	GtkWidget *home_button;

	GtkWidget *stop_image;
	GtkWidget *reload_image;

	gboolean entry_modified;
};

enum
{
	ENTRY_ACTIVATED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE(BrowserToolbar, browser_toolbar, GTK_TYPE_TOOLBAR)

static void
on_entry_insert_text(GtkEntryBuffer *buffer, guint position, gchar *chars, guint n_chars, BrowserToolbar *toolbar)
{
	toolbar->entry_modified = TRUE;

	g_print("Entry modified\n");
}

static void
on_entry_delete_text(GtkEntryBuffer *buffer, guint position, guint n_chars, BrowserToolbar *toolbar)
{
	toolbar->entry_modified = TRUE;

	g_print("Entry modified\n");
}

static void
on_entry_activated(GtkEntry *entry, BrowserToolbar *toolbar)
{
	/* Allow the entry to be set externally. */
	toolbar->entry_modified = FALSE;

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

	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, back_button);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, forward_button);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, stop_reload_button);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, entry);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, home_button);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, stop_image);
	gtk_widget_class_bind_template_child(widget_class, BrowserToolbar, reload_image);
}

static void
browser_toolbar_init(BrowserToolbar *toolbar)
{
	GtkEntryBuffer *buffer;

	toolbar->entry_modified = FALSE;

	gtk_widget_init_template(GTK_WIDGET(toolbar));

	buffer = gtk_entry_get_buffer(GTK_ENTRY(toolbar->entry));

	g_signal_connect(buffer, "inserted-text", G_CALLBACK(on_entry_insert_text), toolbar);
	g_signal_connect(buffer, "deleted-text", G_CALLBACK(on_entry_delete_text), toolbar);
	g_signal_connect(toolbar->entry, "activate", G_CALLBACK(on_entry_activated), toolbar);
}

void
browser_toolbar_update_buttons(BrowserToolbar *toolbar, gboolean can_go_back, gboolean can_go_forward)
{
	g_return_if_fail(BROWSER_IS_TOOLBAR(toolbar));

	gtk_widget_set_sensitive(toolbar->back_button, can_go_back);
	gtk_widget_set_sensitive(toolbar->forward_button, can_go_forward);
}

void
browser_toolbar_set_entry_uri(BrowserToolbar *toolbar, const gchar *uri, gboolean user)
{
	g_return_if_fail(BROWSER_IS_TOOLBAR(toolbar));

	gtk_entry_set_text(GTK_ENTRY(toolbar->entry), uri);

	toolbar->entry_modified = user;
}

gchar *
browser_toolbar_get_entry_text(BrowserToolbar *toolbar)
{
	const gchar *text;

	g_return_val_if_fail(BROWSER_IS_TOOLBAR(toolbar), NULL);

	text = gtk_entry_get_text(GTK_ENTRY(toolbar->entry));

	return g_strdup(text);
}

gboolean
browser_toolbar_is_entry_modified(BrowserToolbar *toolbar)
{
	g_return_val_if_fail(BROWSER_IS_TOOLBAR(toolbar), FALSE);

	return toolbar->entry_modified;
}

GtkWidget *
browser_toolbar_new(void)
{
	return g_object_new(BROWSER_TYPE_TOOLBAR, NULL);
}
