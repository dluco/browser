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
	PROP_TITLE,
	PROP_STATE,
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

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
	BrowserTab *tab = BROWSER_TAB(object);

	switch (prop_id) {
		case PROP_TITLE:
			g_value_take_string(value, browser_tab_get_title(tab));
			break;
		case PROP_STATE:
			/* TODO: Set state prop. */
			break;
		default:
			break;
	}
}

static void
on_web_view_title_changed(BrowserWebView *web_view, GParamSpec *pspec, BrowserTab *tab)
{
	/* Notify the change in the tab title. */
	g_object_notify_by_pspec(G_OBJECT(tab), properties[PROP_TITLE]);
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
browser_tab_class_init(BrowserTabClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	object_class->set_property = browser_tab_set_property;
	object_class->get_property = browser_tab_get_property;

	properties[PROP_TITLE] = g_param_spec_string("title", "Title", "The tab's title", NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY);
	properties[PROP_STATE] = g_param_spec_string("state", "State", "The tab's state", NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY);

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
	char *uri = "https://www.google.ca";

	g_type_ensure(BROWSER_TYPE_WEB_VIEW);

	gtk_widget_init_template(GTK_WIDGET(tab));

	g_signal_connect(tab->web_view, "notify::title", G_CALLBACK(on_web_view_title_changed), tab);
	g_signal_connect(tab->web_view, "load-changed", G_CALLBACK(on_web_view_load_changed), tab);

	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(tab->web_view), uri);
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
