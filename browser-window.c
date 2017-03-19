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
on_tab_title_changed(BrowserTab *tab, GParamSpec *pspec, BrowserWindow *window)
{
	if (tab == browser_window_get_active_tab(window)) {
		update_title(window);
	}
}

static void
on_tab_added(GtkNotebook *notebook, GtkWidget *child, guint page_num, BrowserWindow *window)
{
	BrowserTab *tab = BROWSER_TAB(child);

	/* IMPORTANT: Signal handlers connected here must be disconnected
	 * when the tab is removed from the notebook.
	 */
	g_signal_connect(tab, "notify::title", G_CALLBACK(on_tab_title_changed), window);
}

static void
on_tab_removed(GtkNotebook *notebook, GtkWidget *child, guint page_num, BrowserWindow *window)
{
	BrowserTab *tab = BROWSER_TAB(child);

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

BrowserWindow *
browser_window_new(BrowserApp *app)
{
	return g_object_new(BROWSER_TYPE_WINDOW, "application", app, NULL);
}

void
browser_window_open(BrowserWindow *window, GFile *file)
{
}
