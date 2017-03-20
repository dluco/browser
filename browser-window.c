#include "browser-window.h"
#include "browser-app.h"
#include "browser-notebook.h"
#include "browser-tab.h"
#include "browser-toolbar.h"
#include "browser-web-view.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <webkit2/webkit2.h>

#define SAVED_USER_INPUT "saved-user-input"

struct _BrowserWindow
{
	GtkApplicationWindow parent;

	GtkWidget *box;
	GtkWidget *toolbar;
	GtkWidget *notebook;
};

G_DEFINE_TYPE(BrowserWindow, browser_window, GTK_TYPE_APPLICATION_WINDOW)

static void
update_uri_from_tab(BrowserWindow *window, BrowserTab *tab, gboolean overwrite)
{
	gchar *uri;

	if (tab) {
		uri = browser_tab_get_uri(tab);
	} else {
		uri = g_strdup("");
	}

	if (!browser_toolbar_is_entry_modified(BROWSER_TOOLBAR(window->toolbar)) || overwrite) {
		browser_toolbar_set_entry_uri(BROWSER_TOOLBAR(window->toolbar), uri, FALSE);
	}

	g_free(uri);
}

static void
update_title_from_tab(BrowserWindow *window, BrowserTab *tab)
{
	gchar *title;

	if (tab) {
		title = browser_tab_get_title(tab);
	} else {
		title = g_strdup("Browser");
	}

	gtk_window_set_title(GTK_WINDOW(window), title);

	g_free(title);
}

static void
update_uri(BrowserWindow *window, gboolean overwrite)
{
	BrowserTab *tab = browser_window_get_active_tab(window);
	update_uri_from_tab(window, tab, overwrite);
}

static void
update_title(BrowserWindow *window)
{
	BrowserTab *tab = browser_window_get_active_tab(window);
	update_title_from_tab(window, tab);
}

static void
on_entry_activated(BrowserToolbar *toolbar, BrowserWindow *window)
{
	BrowserTab *tab;
	gchar *entry_text;

	tab = browser_window_get_active_tab(window);
	entry_text = browser_toolbar_get_entry_text(toolbar);

	/* TODO: Perform some validation on the URI before loading. */
	if (tab && entry_text) {
		browser_tab_load_uri(tab, entry_text);

		gtk_widget_grab_focus(GTK_WIDGET(tab));
	}

	g_free(entry_text);
}

static void
on_tab_uri_changed(BrowserTab *tab, BrowserWindow *window)
{
	g_print("Window: tab URI changed\n");

	if (tab == browser_window_get_active_tab(window)) {
		update_uri(window, FALSE);
	}
}

static void
on_tab_title_changed(BrowserTab *tab, BrowserWindow *window)
{
	g_print("Window: tab title changed\n");

	if (tab == browser_window_get_active_tab(window)) {
		update_title(window);
	}
}

static void
on_tab_added(GtkNotebook *notebook, GtkWidget *child, guint page_num, BrowserWindow *window)
{
	BrowserTab *tab = BROWSER_TAB(child);

	g_print("Window: tab added\n");

	/* IMPORTANT: Signal handlers connected here must be disconnected
	 * when the tab is removed from the notebook.
	 */
	g_signal_connect(tab, "uri-changed", G_CALLBACK(on_tab_uri_changed), window);
	g_signal_connect(tab, "title-changed", G_CALLBACK(on_tab_title_changed), window);
}

static void
on_tab_removed(GtkNotebook *notebook, GtkWidget *child, guint page_num, BrowserWindow *window)
{
	BrowserTab *tab = BROWSER_TAB(child);

	g_print("Window: tab removed\n");

	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_uri_changed), window);
	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_title_changed), window);
}

static void
on_tab_changed(GtkNotebook *notebook, GtkWidget *page, guint page_num, BrowserWindow *window)
{
	BrowserTab *incoming = BROWSER_TAB(page);
	BrowserTab *outgoing;
	gchar *user_input = NULL;

	g_print("Window: tab changed\n");

	outgoing = browser_window_get_active_tab(window);

	/* Save user inputed text in the toolbar entry. */
	if (outgoing && browser_toolbar_is_entry_modified(BROWSER_TOOLBAR(window->toolbar))) {
		g_print("Window: saving user input in outgoing tab\n");

		user_input = browser_toolbar_get_entry_text(BROWSER_TOOLBAR(window->toolbar));
		g_object_set_data(G_OBJECT(outgoing), SAVED_USER_INPUT, user_input);
	}

	/* Restore saved user inputed text in the toolbar entry. */
	user_input = g_object_get_data(G_OBJECT(incoming), SAVED_USER_INPUT);
	if (user_input) {
		g_print("Window: restoring user input from incoming tab\n");

		browser_toolbar_set_entry_uri(BROWSER_TOOLBAR(window->toolbar), user_input, TRUE);
		/* Clear the saved text in the object. */
		g_object_set_data(G_OBJECT(incoming), SAVED_USER_INPUT, NULL);
		g_free(user_input);
	} else {
		update_uri_from_tab(window, incoming, TRUE);
	}

	update_title_from_tab(window, incoming);
}

static void
on_new_tab_clicked(BrowserNotebook *notebook, BrowserWindow *window)
{
	g_print("Window: new tab clicked\n");

	browser_window_create_tab_from_uri(window, "about:blank", -1, TRUE); // TODO: Set jump_to from settings.
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

static void
browser_window_init(BrowserWindow *window)
{
	g_type_ensure(BROWSER_TYPE_TOOLBAR);
	g_type_ensure(BROWSER_TYPE_NOTEBOOK);

	gtk_widget_init_template(GTK_WIDGET(window));

	g_signal_connect(window->toolbar, "entry-activated", G_CALLBACK(on_entry_activated), window);

	g_signal_connect(window->notebook, "page-added", G_CALLBACK(on_tab_added), window);
	g_signal_connect(window->notebook, "page-removed", G_CALLBACK(on_tab_removed), window);
	g_signal_connect(window->notebook, "switch-page", G_CALLBACK(on_tab_changed), window);
	g_signal_connect(window->notebook, "new-tab", G_CALLBACK(on_new_tab_clicked), window);

	update_title(window);
}

BrowserTab *
browser_window_get_active_tab(BrowserWindow *window)
{
	g_return_val_if_fail(BROWSER_IS_WINDOW(window), NULL);
	g_return_val_if_fail(window->notebook != NULL, NULL);

	return browser_notebook_get_active_tab(BROWSER_NOTEBOOK(window->notebook));
}

static BrowserTab *
process_create_tab(BrowserWindow *window, BrowserTab *tab, gint position, gboolean jump_to)
{
	g_return_val_if_fail(BROWSER_IS_WINDOW(window), NULL);
	g_return_val_if_fail(BROWSER_IS_TAB(tab), NULL);

	gtk_widget_show(GTK_WIDGET(tab));
	browser_notebook_add_tab(BROWSER_NOTEBOOK(window->notebook), tab, position, jump_to);

	/* Present window is necessary. */
	if (!gtk_widget_get_visible(GTK_WIDGET(window))) {
		gtk_window_present(GTK_WINDOW(window));
	}

	return tab;
}

BrowserTab *
browser_window_create_tab_from_uri(BrowserWindow *window, const gchar *uri, gint position, gboolean jump_to)
{
	GtkWidget *tab;

	g_return_val_if_fail(BROWSER_IS_WINDOW(window), NULL);
	g_return_val_if_fail(uri != NULL, NULL);

	tab = browser_tab_new();

	browser_tab_load_uri(BROWSER_TAB(tab), uri);

	return process_create_tab(window, BROWSER_TAB(tab), position, jump_to);
}

void
browser_window_open(BrowserWindow *window, const gchar *uri)
{
	g_return_if_fail(BROWSER_IS_WINDOW(window));
	g_return_if_fail(uri != NULL);

	/* TODO: Check whether the uri is already open in a tab, etc. */

	browser_window_create_tab_from_uri(window, uri, -1, TRUE); // TODO: Set jump_to from settings.
}

BrowserWindow *
browser_window_new(BrowserApp *app)
{
	return g_object_new(BROWSER_TYPE_WINDOW, "application", app, NULL);
}
