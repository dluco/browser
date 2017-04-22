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

struct _BrowserWindow {
	GtkApplicationWindow parent;

	GtkWidget *box;
	BrowserToolbar *toolbar;
	GtkWidget *notebook;

	GList *closed_tabs;
};

G_DEFINE_TYPE(BrowserWindow, browser_window, GTK_TYPE_APPLICATION_WINDOW)

typedef struct _TabInfo {
	WebKitWebViewSessionState *state;
	gint index;
} TabInfo;

TabInfo *
tab_info_new(void)
{
	return g_slice_new0(TabInfo);
}

void
tab_info_free(TabInfo *tab_info)
{
	g_return_if_fail(tab_info != NULL);

	if (tab_info->state)
		webkit_web_view_session_state_unref(tab_info->state);

	g_slice_free(TabInfo, tab_info);
}

static void
win_activate_print(GSimpleAction *action,
				   GVariant      *parameter,
				   gpointer       user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);
	BrowserTab *tab;
	BrowserWebView *web_view;
	WebKitPrintOperation *print_operation;

	g_print("Window: print action\n");

	tab = browser_window_get_active_tab(window);
	if (tab) {
		web_view = browser_tab_get_web_view(tab);

		print_operation = webkit_print_operation_new(WEBKIT_WEB_VIEW(web_view));
		webkit_print_operation_run_dialog(print_operation, GTK_WINDOW(window));

		g_object_unref(print_operation);
	}
}

static void
win_toggle_fullscreen(GSimpleAction *action,
					  GVariant      *state,
					  gpointer       user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);

	if (g_variant_get_boolean(state))
		browser_window_fullscreen(window);
	else
		browser_window_unfullscreen(window);
}

//static void
//win_toggle_show_menubar(GSimpleAction *action,
//					    GVariant      *state,
//					    gpointer       user_data)
//{
//	BrowserWindow *window = BROWSER_WINDOW(user_data);
//
//	if (g_variant_get_boolean(state))
//		browser_window_fullscreen(window);
//	else
//		browser_window_unfullscreen(window);
//}

static void
win_activate_back(GSimpleAction *action,
				  GVariant      *parameter,
				  gpointer       user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);
	BrowserTab *tab;
	BrowserWebView *web_view;

	g_print("Window: back action\n");

	tab = browser_window_get_active_tab(window);
	if (tab) {
		web_view = browser_tab_get_web_view(tab);
		webkit_web_view_go_back(WEBKIT_WEB_VIEW(web_view));
	}
}

static void
win_activate_foward(GSimpleAction *action,
					GVariant      *parameter,
					gpointer       user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);
	BrowserTab *tab;
	BrowserWebView *web_view;

	g_print("Window: forward action\n");

	tab = browser_window_get_active_tab(window);
	if (tab) {
		web_view = browser_tab_get_web_view(tab);
		webkit_web_view_go_forward(WEBKIT_WEB_VIEW(web_view));
	}
}

static void
win_activate_home(GSimpleAction *action,
					GVariant      *parameter,
					gpointer       user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);
	BrowserTab *tab;

	g_print("Window: home action\n");

	tab = browser_window_get_active_tab(window);
	if (tab) {

	}
}

static void
win_activate_new_tab(GSimpleAction *action,
					GVariant      *parameter,
					gpointer       user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);

	g_print("Window: new tab action\n");

	// TODO: Set jump_to from settings.
	browser_window_create_tab_from_uri(window, "about:blank", -1, TRUE);
}

static void
win_activate_close_tab(GSimpleAction *action,
					   GVariant      *parameter,
					   gpointer       user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);
	BrowserTab *tab;

	g_print("Window: close tab action\n");

	tab = browser_window_get_active_tab(window);
	if (tab) {
		browser_window_close_tab(window, tab);
	}
}

static void
win_activate_undo_close_tab(GSimpleAction *action,
							GVariant      *parameter,
							gpointer       user_data)
{
	BrowserWindow *window = BROWSER_WINDOW(user_data);
	BrowserTab *tab;
	BrowserWebView *web_view;
	TabInfo *tab_info = NULL;
	GList *l;

	g_print("Window: undo close tab action\n");

	l = g_list_first(window->closed_tabs);
	if (l) {
		tab_info = l->data;
	}

	/* Restore tab using tab_info. */
	if (tab_info) {
		tab = browser_window_create_tab(window, tab_info->index, TRUE);
		if (tab) {
			web_view = browser_tab_get_web_view(tab);
			webkit_web_view_restore_session_state(WEBKIT_WEB_VIEW(web_view),
					tab_info->state);
		}
		tab_info_free(tab_info);
	}

	/* Remove (and free) the list item. */
	window->closed_tabs = g_list_delete_link(window->closed_tabs, l);
}

static const GActionEntry win_action_entries[] = {
	{ "print", win_activate_print },
	{ "fullscreen", NULL, NULL, "false", win_toggle_fullscreen },
//	{ "show-menubar", NULL, NULL, "true", win_toggle_show_menubar },
	{ "back", win_activate_back, NULL, "false" },
	{ "forward", win_activate_foward, NULL, "false" },
	{ "home", win_activate_home },
	{ "new-tab", win_activate_new_tab },
	{ "close-tab", win_activate_close_tab },
	{ "undo-close-tab", win_activate_undo_close_tab },
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
update_fullscreen_state(BrowserWindow *window,
						gboolean       is_fullscreen)
{
	GAction *action;

	g_return_if_fail(BROWSER_IS_WINDOW(window));

	/* Update "fullscreen" action state. */
	action = g_action_map_lookup_action(G_ACTION_MAP(window),
										"fullscreen");
	if (action) {
		g_simple_action_set_state(G_SIMPLE_ACTION(action),
								  g_variant_new_boolean(is_fullscreen));
	}

	/* Update window widgets' visibility. */
	gtk_widget_set_visible(GTK_WIDGET(window->toolbar), !is_fullscreen);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(window->notebook), !is_fullscreen);

	g_object_set(G_OBJECT(window), "show-menubar", !is_fullscreen, NULL);
}

static void
update_uri_from_tab(BrowserWindow *window,
					BrowserTab    *tab,
					gboolean       overwrite)
{
	gchar *uri;

	if (tab)
		uri = browser_tab_get_uri(tab);
	else
		uri = g_strdup("");

	if (!browser_toolbar_is_entry_modified(window->toolbar) || overwrite)
		browser_toolbar_set_entry_uri(window->toolbar, uri, FALSE);

	g_free(uri);
}

static void
update_title_from_tab(BrowserWindow *window,
					  BrowserTab    *tab)
{
	gchar *title;

	if (tab)
		title = browser_tab_get_title(tab);
	else
		title = g_strdup("Browser");

	gtk_window_set_title(GTK_WINDOW(window), title);

	g_free(title);
}

static void
update_uri(BrowserWindow *window,
		   gboolean       overwrite)
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

static gboolean
key_release_event(GtkWidget *widget,
				  GdkEventKey *event)
{
	BrowserWindow *window = BROWSER_WINDOW(widget);
	gboolean handled = FALSE;

	g_print("Window: key release event\n");

	/* Check if Alt released as the only key. */
	if (event->state == GDK_MOD1_MASK && (event->keyval == GDK_KEY_Alt_L || event->keyval == GDK_KEY_Alt_R)) {
		/* Toggle the "show-menubar" action (& property) state. */
		g_action_group_activate_action(G_ACTION_GROUP(window), "show-menubar", NULL);
		handled = TRUE;
	}

	if (!handled)
		handled = GTK_WIDGET_CLASS(browser_window_parent_class)->key_release_event(widget, event);

	return handled;
}

static gboolean
window_state_event(GtkWidget           *widget,
				   GdkEventWindowState *event)
{
	BrowserWindow *window = BROWSER_WINDOW(widget);

	g_print("Window: window state event\n");

	/* Update win.fullscreen action if window fullscreen state changed. */
	if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
		update_fullscreen_state(window, event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);

	return GTK_WIDGET_CLASS(browser_window_parent_class)->window_state_event(widget, event);
}

static void
on_entry_activated(BrowserToolbar *toolbar,
				   BrowserWindow  *window)
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
on_tab_state_changed(BrowserTab    *tab,
					 GParamSpec    *pspec,
					 BrowserWindow *window)
{
	g_print("Window: tab state changed\n");
}

static void
on_tab_uri_changed(BrowserTab    *tab,
				   BrowserWindow *window)
{
	g_print("Window: tab URI changed\n");

	if (tab == browser_window_get_active_tab(window))
		update_uri(window, FALSE);
}

static void
on_tab_title_changed(BrowserTab    *tab,
					 BrowserWindow *window)
{
	g_print("Window: tab title changed\n");

	if (tab == browser_window_get_active_tab(window))
		update_title(window);
}

static void
on_tab_back_forward_changed(BrowserTab    *tab,
							gboolean       can_go_back,
							gboolean       can_go_forward,
							BrowserWindow *window)
{
	g_print("Window: tab back-forward changed\n");

	if (tab == browser_window_get_active_tab(window))
		browser_toolbar_update_buttons(window->toolbar, can_go_back, can_go_forward);
}

static gboolean
handle_navigation_action_decision(WebKitWebView *web_view,
								  WebKitPolicyDecision *decision,
								  BrowserWindow *window)
{
	WebKitNavigationPolicyDecision *navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);

	return FALSE; // Unhandled
}

static gboolean
handle_new_window_decision(WebKitWebView *web_view,
						   WebKitPolicyDecision *decision,
						   BrowserWindow *window)
{
	WebKitNavigationPolicyDecision *navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);

	return FALSE; // Unhandled
}

static gboolean
handle_response_decision(WebKitWebView *web_view,
						 WebKitPolicyDecision *decision,
						 BrowserWindow *window)
{
	WebKitResponsePolicyDecision *response_decision = WEBKIT_RESPONSE_POLICY_DECISION(decision);
	/* Decide whether to load or download a resource. */
	if (!webkit_response_policy_decision_is_mime_type_supported(response_decision)) {
		webkit_policy_decision_download(decision);
		return TRUE;
	}

	return FALSE; // Unhandled
}

static gboolean
on_web_view_decide_policy(WebKitWebView *web_view,
						  WebKitPolicyDecision *decision,
						  WebKitPolicyDecisionType type,
						  BrowserWindow *window)
{
	switch (type) {
		case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
			return handle_navigation_action_decision(web_view, decision, window);
		case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
			return handle_new_window_decision(web_view, decision, window);
		case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
			return handle_response_decision(web_view, decision, window);
		default:
			break;
	}
	return FALSE;
}

static void
on_tab_added(GtkNotebook   *notebook,
			 GtkWidget     *child,
			 guint          page_num,
			 BrowserWindow *window)
{
	BrowserTab *tab = BROWSER_TAB(child);
	BrowserWebView *web_view;

	g_print("Window: tab added\n");

	web_view = browser_tab_get_web_view(tab);

	/* IMPORTANT: Signal handlers connected here must be disconnected
	 * when the tab is removed from the notebook.
	 */
	g_signal_connect(tab, "notify::state", G_CALLBACK(on_tab_state_changed), window);
	g_signal_connect(tab, "uri-changed", G_CALLBACK(on_tab_uri_changed), window);
	g_signal_connect(tab, "title-changed", G_CALLBACK(on_tab_title_changed), window);
	g_signal_connect(tab, "back-forward-changed", G_CALLBACK(on_tab_back_forward_changed), window);

	if (web_view) {
		g_signal_connect(web_view, "decide-policy", G_CALLBACK(on_web_view_decide_policy), window);
	}
}

static void
on_tab_removed(GtkNotebook   *notebook,
			   GtkWidget     *child,
			   guint          page_num,
			   BrowserWindow *window)
{
	BrowserTab *tab = BROWSER_TAB(child);
	BrowserWebView *web_view;
	TabInfo *tab_info;
	gchar *user_input;
	gint n_pages;

	g_print("Window: tab removed\n");

	web_view = browser_tab_get_web_view(tab);

	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_state_changed), window);
	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_uri_changed), window);
	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_title_changed), window);
	g_signal_handlers_disconnect_by_func(tab, G_CALLBACK(on_tab_back_forward_changed), window);

	if (web_view) {
		g_signal_handlers_disconnect_by_func(web_view, G_CALLBACK(on_web_view_decide_policy), window);
	}


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

	/* Save tab state so that it can be restored later. */
	tab_info = tab_info_new();
	tab_info->index = page_num;

	/* Save tab's web-view state. */
	if (web_view) {
		tab_info->state = webkit_web_view_get_session_state(WEBKIT_WEB_VIEW(web_view));
	}
	window->closed_tabs = g_list_prepend(window->closed_tabs, tab_info);

	n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(window->notebook));
	if (n_pages < 1) {
		/* All tabs have been closed - close this window. */
		gtk_window_close(GTK_WINDOW(window));
	}
}

static void
on_tab_changed(GtkNotebook   *notebook,
			   GtkWidget     *page,
			   guint          page_num,
			   BrowserWindow *window)
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
on_close_tab_clicked(BrowserNotebook *notebook,
					 BrowserTab      *tab,
					 BrowserWindow   *window)
{
	g_print("Window: close tab clicked\n");

	browser_window_close_tab(window, tab);
}

static void
on_show_menubar_changed(BrowserWindow *window, GParamSpec *pspec, gpointer unused)
{
	g_print("Window: show-menubar changed\n");
}

static void
browser_window_class_init(BrowserWindowClass *class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	widget_class->key_release_event = key_release_event;
	widget_class->window_state_event = window_state_event;

	file = g_mapped_file_new("browser-window.ui", FALSE, NULL);
	if (!file)
		return;
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
	GPropertyAction *action;

	window->closed_tabs = NULL;

	g_type_ensure(BROWSER_TYPE_TOOLBAR);
	g_type_ensure(BROWSER_TYPE_NOTEBOOK);

	gtk_widget_init_template(GTK_WIDGET(window));

	g_action_map_add_action_entries(G_ACTION_MAP(window),
									win_action_entries,
									G_N_ELEMENTS(win_action_entries),
									window);

	action = g_property_action_new("show-menubar",
			window,
			"show-menubar");
	g_action_map_add_action(G_ACTION_MAP(window), G_ACTION(action));

	g_signal_connect(window->toolbar, "entry-activated", G_CALLBACK(on_entry_activated), window);

	g_signal_connect(window->notebook, "page-added", G_CALLBACK(on_tab_added), window);
	g_signal_connect(window->notebook, "page-removed", G_CALLBACK(on_tab_removed), window);
	g_signal_connect(window->notebook, "switch-page", G_CALLBACK(on_tab_changed), window);
	g_signal_connect(window->notebook, "close-tab", G_CALLBACK(on_close_tab_clicked), window);

	g_object_set(G_OBJECT(window),
			"show-menubar", FALSE,
			NULL);

	g_signal_connect(window, "notify::show-menubar", G_CALLBACK(on_show_menubar_changed), NULL);

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
process_create_tab(BrowserWindow *window,
				   BrowserTab    *tab,
				   gint           position,
				   gboolean       jump_to)
{
	g_return_val_if_fail(BROWSER_IS_WINDOW(window), NULL);
	g_return_val_if_fail(BROWSER_IS_TAB(tab), NULL);

	gtk_widget_show(GTK_WIDGET(tab));
	browser_notebook_add_tab(BROWSER_NOTEBOOK(window->notebook), tab, position, jump_to);

	/* Present window is necessary. */
	if (!gtk_widget_get_visible(GTK_WIDGET(window)))
		gtk_window_present(GTK_WINDOW(window));

	return tab;
}

BrowserTab *
browser_window_create_tab_from_uri(BrowserWindow *window,
								   const gchar   *uri,
								   gint           position,
								   gboolean       jump_to)
{
	GtkWidget *tab;

	g_return_val_if_fail(BROWSER_IS_WINDOW(window), NULL);

	tab = browser_tab_new();

	/* URI is permitted to be NULL. */
	if (uri) {
		browser_tab_load_uri(BROWSER_TAB(tab), uri);
	}

	return process_create_tab(window, BROWSER_TAB(tab), position, jump_to);
}

BrowserTab *
browser_window_create_tab(BrowserWindow *window,
						  gint position,
						  gboolean jump_to)
{
	return browser_window_create_tab_from_uri(window, NULL, position, jump_to);
}

void
browser_window_open(BrowserWindow *window,
					const gchar   *uri)
{
	g_return_if_fail(BROWSER_IS_WINDOW(window));
	g_return_if_fail(uri != NULL);

	/* TODO: Check whether the uri is already open in a tab, etc. */
	// TODO: Set jump_to from settings.
	browser_window_create_tab_from_uri(window, uri, -1, TRUE);
}

void
browser_window_close_tab(BrowserWindow *window,
						 BrowserTab    *tab)
{
	gint page_num;

	g_return_if_fail(BROWSER_IS_WINDOW(window));
	g_return_if_fail(BROWSER_IS_TAB(tab));

	/* TODO: Check tab state before closing. */

	page_num = gtk_notebook_page_num(GTK_NOTEBOOK(window->notebook), GTK_WIDGET(tab));
	if (page_num > -1)
		gtk_notebook_remove_page(GTK_NOTEBOOK(window->notebook), page_num);
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
