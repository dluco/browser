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
	BrowserToolbar *toolbar;
	GtkWidget *notebook;
};

G_DEFINE_TYPE(BrowserWindow, browser_window, GTK_TYPE_APPLICATION_WINDOW)

static void
win_toggle_fullscreen(GSimpleAction *action,
		GVariant *state,
		gpointer user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);

	if (g_variant_get_boolean(state))
		browser_window_fullscreen(window);
	else
		browser_window_unfullscreen(window);

	g_simple_action_set_state(action, state);
}

static const GActionEntry win_action_entries[] = {
	{ "fullscreen", NULL, NULL, "false",  win_toggle_fullscreen },
};

static GdkWindowState
get_state(BrowserWindow *window)
{
	GdkWindow *gdk_window;
	GdkWindowState state = 0;

	g_return_val_if_fail(BROWSER_IS_WINDOW(window), 0);

	/* GdkWindow can be NULL if window is not realized. */
	gdk_window = gtk_widget_get_window(GTK_WIDGET(window));
	if (gdk_window)
		state = gdk_window_get_state(gdk_window);

	return state;
}

static void
update_uri_from_tab(BrowserWindow *window, BrowserTab *tab, gboolean overwrite)
{
	gchar *uri;

	if (tab) {
		uri = browser_tab_get_uri(tab);
	} else {
		uri = g_strdup("");
	}

	if (!browser_toolbar_is_entry_modified(window->toolbar) || overwrite) {
		browser_toolbar_set_entry_uri(window->toolbar, uri, FALSE);
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
on_back_clicked(BrowserToolbar *toolbar, BrowserWindow *window)
{
	BrowserTab *tab;
	BrowserWebView *web_view;

	g_print("Window: back clicked\n");

	tab = browser_window_get_active_tab(window);
	if (tab) {
		web_view = browser_tab_get_web_view(tab);
		webkit_web_view_go_back(WEBKIT_WEB_VIEW(web_view));
	}
}

static void
on_forward_clicked(BrowserToolbar *toolbar, BrowserWindow *window)
{
	BrowserTab *tab;
	BrowserWebView *web_view;

	g_print("Window: forward clicked\n");

	tab = browser_window_get_active_tab(window);
	if (tab) {
		web_view = browser_tab_get_web_view(tab);
		webkit_web_view_go_forward(WEBKIT_WEB_VIEW(web_view));
	}
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
on_tab_state_changed(BrowserTab *tab, GParamSpec *pspec, BrowserWindow *window)
{
	g_print("Window: tab state changed\n");
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
on_tab_back_forward_changed(BrowserTab *tab, gboolean can_go_back, gboolean can_go_forward, BrowserWindow *window)
{
	g_print("Window: tab back-forward changed\n");

	if (tab == browser_window_get_active_tab(window)) {
		browser_toolbar_update_buttons(window->toolbar, can_go_back, can_go_forward);
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
	g_signal_connect(tab, "notify::state", G_CALLBACK(on_tab_state_changed), window);
	g_signal_connect(tab, "uri-changed", G_CALLBACK(on_tab_uri_changed), window);
	g_signal_connect(tab, "title-changed", G_CALLBACK(on_tab_title_changed), window);
	g_signal_connect(tab, "back-forward-changed", G_CALLBACK(on_tab_back_forward_changed), window);
}

static void
on_tab_removed(GtkNotebook *notebook, GtkWidget *child, guint page_num, BrowserWindow *window)
{
	BrowserTab *tab = BROWSER_TAB(child);
	char *user_input;

	g_print("Window: tab removed\n");

	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_uri_changed), window);
	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_title_changed), window);
	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_back_forward_changed), window);

	/* TODO: Save the user inputed text in the toolbar entry? This is only
	 * applicable if tabs will be restorable in the future.
	 */

	/* TODO: Don't clear the SAVED_USER_INPUT data in the removed tab,
	 * since we might add an undo-close-tab feature, in which case the
	 * tab and its state needs to be kept alive.
	 */
	user_input = g_object_get_data(G_OBJECT(child), SAVED_USER_INPUT);
	if (user_input) {
		g_print("Window: clearing saved user input from removed tab\n");
		g_object_set_data(G_OBJECT(child), SAVED_USER_INPUT, NULL);
		g_free(user_input);
	}
}

static void
on_tab_changed(GtkNotebook *notebook, GtkWidget *page, guint page_num, BrowserWindow *window)
{
	BrowserTab *incoming = BROWSER_TAB(page);
	BrowserTab *outgoing;
	BrowserWebView *web_view;
	gchar *user_input = NULL;

	g_print("Window: tab changed\n");

	outgoing = browser_window_get_active_tab(window);

	/* Save user inputed text in the toolbar entry. */
	if (outgoing && browser_toolbar_is_entry_modified(window->toolbar)) {
		g_print("Window: saving user input in outgoing tab\n");

		user_input = browser_toolbar_get_entry_text(window->toolbar);
		g_object_set_data(G_OBJECT(outgoing), SAVED_USER_INPUT, user_input);
	}

	/* Restore saved user inputed text in the toolbar entry. */
	user_input = g_object_get_data(G_OBJECT(incoming), SAVED_USER_INPUT);
	if (user_input) {
		g_print("Window: restoring user input from incoming tab\n");

		browser_toolbar_set_entry_uri(window->toolbar, user_input, TRUE);
		/* Clear the saved text in the object. */
		g_object_set_data(G_OBJECT(incoming), SAVED_USER_INPUT, NULL);
		g_free(user_input);
	} else {
		update_uri_from_tab(window, incoming, TRUE);
	}

	update_title_from_tab(window, incoming);

	web_view = browser_tab_get_web_view(incoming);
	browser_toolbar_update_buttons(window->toolbar,
			webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(web_view)),
			webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(web_view)));
}

static void
on_new_tab_clicked(BrowserNotebook *notebook, BrowserWindow *window)
{
	g_print("Window: new tab clicked\n");

	browser_window_create_tab_from_uri(window, "about:blank", -1, TRUE); // TODO: Set jump_to from settings.
}

static void
on_close_tab_clicked(BrowserNotebook *notebook, BrowserTab *tab, BrowserWindow *window)
{
	g_print("Window: close tab clicked\n");

	browser_window_close_tab(window, tab);
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

	g_action_map_add_action_entries(G_ACTION_MAP(window),
			win_action_entries,
			G_N_ELEMENTS(win_action_entries),
			window);

	g_signal_connect(window->toolbar, "back-clicked", G_CALLBACK(on_back_clicked), window);
	g_signal_connect(window->toolbar, "forward-clicked", G_CALLBACK(on_forward_clicked), window);
	g_signal_connect(window->toolbar, "entry-activated", G_CALLBACK(on_entry_activated), window);

	g_signal_connect(window->notebook, "page-added", G_CALLBACK(on_tab_added), window);
	g_signal_connect(window->notebook, "page-removed", G_CALLBACK(on_tab_removed), window);
	g_signal_connect(window->notebook, "switch-page", G_CALLBACK(on_tab_changed), window);
	g_signal_connect(window->notebook, "new-tab", G_CALLBACK(on_new_tab_clicked), window);
	g_signal_connect(window->notebook, "close-tab", G_CALLBACK(on_close_tab_clicked), window);

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

void
browser_window_close_tab(BrowserWindow *window, BrowserTab *tab)
{
	gint page_num;

	g_return_if_fail(BROWSER_IS_WINDOW(window));
	g_return_if_fail(BROWSER_IS_TAB(tab));

	/* TODO: Check tab state before closing. */

	page_num = gtk_notebook_page_num(GTK_NOTEBOOK(window->notebook), GTK_WIDGET(tab));
	if (page_num > -1) {
		gtk_notebook_remove_page(GTK_NOTEBOOK(window->notebook), page_num);
	}
}

void
browser_window_fullscreen(BrowserWindow *window)
{
	g_return_if_fail(BROWSER_IS_WINDOW(window));

	if (browser_window_is_fullscreen(window))
		return;

	gtk_window_fullscreen(GTK_WINDOW(window));
}

void
browser_window_unfullscreen(BrowserWindow *window)
{
	g_return_if_fail(BROWSER_IS_WINDOW(window));

	if (!browser_window_is_fullscreen(window))
		return;

	gtk_window_unfullscreen(GTK_WINDOW(window));
}

gboolean
browser_window_is_fullscreen(BrowserWindow *window)
{
	GdkWindowState state;

	g_return_val_if_fail(BROWSER_IS_WINDOW(window), FALSE);

	state = get_state(window);

	return state & GDK_WINDOW_STATE_FULLSCREEN;
}

BrowserWindow *
browser_window_new(BrowserApp *app)
{
	return g_object_new(BROWSER_TYPE_WINDOW, "application", app, NULL);
}
