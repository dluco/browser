#include "browser-tab.h"
#include "browser-web-view.h"
#include <glib-object.h>
#include <glib.h>
#include <glib/gstdio.h>

struct _BrowserTab
{
	GtkBox parent;

	BrowserWebView *web_view;
};

enum
{
	PROP_0,
	PROP_STATE,
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

enum
{
	URI_CHANGED,
	TITLE_CHANGED,
	BACK_FORWARD_CHANGED,
	LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE(BrowserTab, browser_tab, GTK_TYPE_BOX)

static void
browser_tab_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
//	BrowserTab *tab = BROWSER_TAB(object);

	switch (prop_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
browser_tab_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
//	BrowserTab *tab = BROWSER_TAB(object);

	switch (prop_id) {
		case PROP_STATE:
			/* TODO: Set state prop. */
			break;
		default:
			break;
	}
}

static void
on_tab_grab_focus(BrowserTab *tab, gpointer unused)
{
	/* Tab cannot focus directly, so forward the grab to the web-view. */
	gtk_widget_grab_focus(GTK_WIDGET(tab->web_view));
}

static void
on_web_view_uri_changed(BrowserWebView *web_view, GParamSpec *pspec, BrowserTab *tab)
{
	g_print("URI changed\n");

	g_signal_emit(tab, signals[URI_CHANGED], 0, NULL);
}

static void
on_web_view_title_changed(BrowserWebView *web_view, GParamSpec *pspec, BrowserTab *tab)
{
	g_signal_emit(tab, signals[TITLE_CHANGED], 0, NULL);
}

static void
on_web_view_load_changed(BrowserWebView *web_view, WebKitLoadEvent load_event, BrowserTab *tab)
{
	switch (load_event) {
		case WEBKIT_LOAD_STARTED:
			break;
		case WEBKIT_LOAD_REDIRECTED:
			break;
		case WEBKIT_LOAD_COMMITTED:
			break;
		case WEBKIT_LOAD_FINISHED:
			break;
		default:
			break;
	}

	/* Notify the change in the tab state. */
	g_object_notify_by_pspec(G_OBJECT(tab), properties[PROP_STATE]);
}

static void
on_back_forward_list_changed(WebKitBackForwardList *back_forward_list, WebKitBackForwardListItem *item_added, gpointer items_removed, BrowserTab *tab)
{
	gboolean can_go_back = webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(tab->web_view));
	gboolean can_go_forward = webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(tab->web_view));

	g_signal_emit(tab, signals[BACK_FORWARD_CHANGED], 0, can_go_back, can_go_forward);
}

static void
browser_tab_uri_changed(BrowserTab *tab)
{
}

static void
browser_tab_title_changed(BrowserTab *tab)
{
}

static void
browser_tab_back_forward_changed(BrowserTab *tab, gboolean can_go_back, gboolean can_go_forward)
{
}

static void
browser_tab_class_init(BrowserTabClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	object_class->set_property = browser_tab_set_property;
	object_class->get_property = browser_tab_get_property;

	properties[PROP_STATE] = g_param_spec_string("state", "State", "The tab's state", NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY);

	signals[URI_CHANGED] = g_signal_new_class_handler("uri-changed",
			G_TYPE_FROM_CLASS(class),
			G_SIGNAL_RUN_LAST,
			G_CALLBACK(browser_tab_uri_changed),
			NULL, NULL, NULL,
			G_TYPE_NONE, 0);

	signals[TITLE_CHANGED] = g_signal_new_class_handler("title-changed",
			G_TYPE_FROM_CLASS(class),
			G_SIGNAL_RUN_LAST,
			G_CALLBACK(browser_tab_title_changed),
			NULL, NULL, NULL,
			G_TYPE_NONE, 0);

	signals[BACK_FORWARD_CHANGED] = g_signal_new_class_handler("back-forward-changed",
			G_TYPE_FROM_CLASS(class),
			G_SIGNAL_RUN_LAST,
			G_CALLBACK(browser_tab_back_forward_changed),
			NULL, NULL, NULL,
			G_TYPE_NONE,
			2,
			G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

	file = g_mapped_file_new("browser-tab.ui", FALSE, NULL);
	if (!file) {
		return;
	}
	bytes = g_mapped_file_get_bytes(file);
	gtk_widget_class_set_template(widget_class, bytes);
	g_mapped_file_unref(file);

	gtk_widget_class_bind_template_child(widget_class, BrowserTab, web_view);
}

static void
browser_tab_init(BrowserTab *tab)
{
	WebKitBackForwardList *back_forward_list;

	g_type_ensure(BROWSER_TYPE_WEB_VIEW);

	gtk_widget_init_template(GTK_WIDGET(tab));

	g_signal_connect(tab, "grab-focus", G_CALLBACK(on_tab_grab_focus), NULL);
	g_signal_connect(tab->web_view, "notify::uri", G_CALLBACK(on_web_view_uri_changed), tab);
	g_signal_connect(tab->web_view, "notify::title", G_CALLBACK(on_web_view_title_changed), tab);
	g_signal_connect(tab->web_view, "load-changed", G_CALLBACK(on_web_view_load_changed), tab);

	back_forward_list = webkit_web_view_get_back_forward_list(WEBKIT_WEB_VIEW(tab->web_view));
	g_signal_connect(back_forward_list, "changed", G_CALLBACK(on_back_forward_list_changed), tab);
}

BrowserWebView *
browser_tab_get_web_view(BrowserTab *tab)
{
	g_return_val_if_fail(BROWSER_IS_TAB(tab), NULL);

	return tab->web_view;
}

void
browser_tab_load_uri(BrowserTab *tab, const gchar *uri)
{
	g_return_if_fail(BROWSER_IS_TAB(tab));
	g_return_if_fail(uri != NULL);

	/* TODO: Check URI here, or in the window? */

	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(tab->web_view), uri);
}

gchar *
browser_tab_get_uri(BrowserTab *tab)
{
	const gchar *uri;

	g_return_val_if_fail(BROWSER_IS_TAB(tab), NULL);

	uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));

	return g_strdup(uri);
}

gchar *
browser_tab_get_title(BrowserTab *tab)
{
	const gchar *title;

	g_return_val_if_fail(BROWSER_IS_TAB(tab), NULL);

	title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(tab->web_view));

	/* TODO: Perform operations on the title here, or in the BrowserWebView class. */
	if (!title) {
		title = "Untitled";
	}

	return g_strdup(title);
}

GtkWidget *
browser_tab_new(void)
{
	return g_object_new(BROWSER_TYPE_TAB, NULL);
}
