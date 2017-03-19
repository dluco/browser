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

struct _BrowserWindow
{
	GtkApplicationWindow parent;

	GtkWidget *box;
	GtkWidget *toolbar;
	GtkWidget *notebook;
};

G_DEFINE_TYPE(BrowserWindow, browser_window, GTK_TYPE_APPLICATION_WINDOW)

static void
update_title(BrowserWindow *window)
{
	BrowserTab *tab;
	gchar *title;

	tab = browser_window_get_active_tab(window);

	if (tab) {
		title = browser_tab_get_title(tab);
	} else {
		title = g_strdup("Browser");
	}

	gtk_window_set_title(GTK_WINDOW(window), title);

	g_free(title);
}

static void
on_entry_activated(BrowserToolbar *toolbar, BrowserWindow *window)
{
	gchar *entry_text;

	entry_text = browser_toolbar_get_entry_text(toolbar);
	if (entry_text) {
		g_print("Entered text: %s\n", entry_text);
	}

	g_free(entry_text);
}

static void
on_tab_uri_changed(BrowserTab *tab, BrowserWindow *window)
{
	g_print("Window: tab URI changed\n");

	if (tab == browser_window_get_active_tab(window)) {

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

	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_uri_changed), window);
	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_title_changed), window);
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
process_create_tab(BrowserWindow *window, BrowserTab *tab)
{
	g_return_val_if_fail(BROWSER_IS_WINDOW(window), NULL);
	g_return_val_if_fail(BROWSER_IS_TAB(tab), NULL);

	gtk_widget_show(GTK_WIDGET(tab));
	browser_notebook_add_tab(BROWSER_NOTEBOOK(window->notebook), tab, -1, TRUE); // TODO: Set jump_to from settings.

	/* Present window is necessary. */
	if (!gtk_widget_get_visible(GTK_WIDGET(window))) {
		gtk_window_present(GTK_WINDOW(window));
	}

	return tab;
}

BrowserTab *
browser_window_create_tab_from_uri(BrowserWindow *window, const gchar *uri)
{
	GtkWidget *tab;

	g_return_val_if_fail(BROWSER_IS_WINDOW(window), NULL);
	g_return_val_if_fail(uri != NULL, NULL);

	tab = browser_tab_new();

	browser_tab_load_uri(BROWSER_TAB(tab), uri);

	return process_create_tab(window, BROWSER_TAB(tab));
}

void
browser_window_open(BrowserWindow *window, const gchar *uri)
{
	g_return_if_fail(BROWSER_IS_WINDOW(window));
	g_return_if_fail(uri != NULL);

	/* TODO: Check whether the uri is already open in a tab, etc. */

	browser_window_create_tab_from_uri(window, uri);
}

BrowserWindow *
browser_window_new(BrowserApp *app)
{
	return g_object_new(BROWSER_TYPE_WINDOW, "application", app, NULL);
}
