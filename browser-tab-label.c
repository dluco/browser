#include "browser-tab-label.h"
#include "browser-tab.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <glib.h>

struct _BrowserTabLabel
{
	GtkBox parent;

	GtkWidget *icon;
	GtkWidget *spinner;
	GtkWidget *label;
	GtkWidget *close_button;

	BrowserTab *tab;
};

enum
{
	PROP_0,
	PROP_TAB,
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

enum
{
	CLOSE_CLICKED,
	LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE(BrowserTabLabel, browser_tab_label, GTK_TYPE_BOX)

static void
browser_tab_label_set_property(GObject *object, uint prop_id, const GValue *value, GParamSpec *pspec)
{
	BrowserTabLabel *tab_label = BROWSER_TAB_LABEL(object);

	switch (prop_id) {
		case PROP_TAB:
			g_return_if_fail(tab_label->tab == NULL);
			tab_label->tab = BROWSER_TAB(g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
browser_tab_label_get_property(GObject *object, uint prop_id, GValue *value, GParamSpec *pspec)
{
	BrowserTabLabel *tab_label = BROWSER_TAB_LABEL(object);

	switch (prop_id) {
		case PROP_TAB:
			g_value_set_object(value, tab_label->tab);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
update_tooltip(BrowserTabLabel *tab_label)
{
	gchar *str;
	gchar *escaped;

	g_return_if_fail(tab_label->tab != NULL);

	str = browser_tab_get_title(tab_label->tab);
	g_return_if_fail(str != NULL);

	escaped = g_markup_escape_text(str, -1);

	gtk_widget_set_tooltip_markup(GTK_WIDGET(tab_label), escaped);

	g_free(str);
	g_free(escaped);
}

static void
on_close_button_clicked(GtkWidget *widget, BrowserTabLabel *tab_label)
{
	g_signal_emit(tab_label, signals[CLOSE_CLICKED], 0, NULL);
}

static void
on_tab_state_changed(BrowserTab *tab, GParamSpec *pspec, BrowserTabLabel *tab_label)
{
	BrowserTabState state;
	gboolean show_spinner = FALSE;

	g_return_if_fail(tab == tab_label->tab);

	state = browser_tab_get_state(tab);

	switch (state) {
		case BROWSER_TAB_STATE_LOADING:
			show_spinner = TRUE;
			break;
		default:
			break;
	}

	g_print("Tab-label: show_spinner = %s\n", (show_spinner) ? "true" : "false");

	if (show_spinner)
		gtk_spinner_start(GTK_SPINNER(tab_label->spinner));
	else
		gtk_spinner_stop(GTK_SPINNER(tab_label->spinner));

	gtk_widget_set_visible(tab_label->icon, !show_spinner);
	gtk_widget_set_visible(tab_label->spinner, show_spinner);
}

static void
on_tab_title_changed(BrowserTab *tab, BrowserTabLabel *tab_label)
{
	gchar *str;

	g_return_if_fail(tab == tab_label->tab);

	str = browser_tab_get_title(tab);
	g_return_if_fail(str != NULL);

	gtk_label_set_text(GTK_LABEL(tab_label->label), str);
	g_free(str);

	update_tooltip(tab_label);
}

/* TODO: Move to utils file. */
GdkPixbuf *
get_scaled_pixbuf_from_surface(cairo_surface_t *surface, int width, int height)
{
	GdkPixbuf *pixbuf;
	int surface_width;
	int surface_height;

	if (!surface)
		return NULL;

	surface_width = cairo_image_surface_get_width(surface);
	surface_height = cairo_image_surface_get_height(surface);
	pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, surface_width, surface_height);

	if (width && height && (surface_width != width || surface_height != height)) {
		GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
		g_object_unref(pixbuf);
		pixbuf = scaled_pixbuf;
	}

	return pixbuf;
}

static void
on_web_view_favicon_changed(BrowserWebView *web_view, GParamSpec *pspec, BrowserTabLabel *tab_label)
{
	cairo_surface_t *favicon_surface;
	GdkPixbuf *pixbuf;

	g_print("Tab-label: favicon changed\n");

	/* TODO: Maintain ref (as property) to pixbuf and unref when favicon changes. */

	/* TODO: Move to BrowserWebView (getting scaled pixbuf)? */

	favicon_surface = webkit_web_view_get_favicon(WEBKIT_WEB_VIEW(web_view));
	if (favicon_surface) {
		pixbuf = get_scaled_pixbuf_from_surface(favicon_surface, 16, 16);
		gtk_image_set_from_pixbuf(GTK_IMAGE(tab_label->icon), pixbuf);
		g_object_unref(pixbuf);
	} else {
		gtk_image_clear(GTK_IMAGE(tab_label->icon));
	}
}

static void
browser_tab_label_close_clicked(BrowserTabLabel *tab_label)
{
}

static void
browser_tab_label_constructed(GObject *object)
{
	BrowserTabLabel *tab_label = BROWSER_TAB_LABEL(object);
	BrowserTab *tab = tab_label->tab;
	BrowserWebView *web_view;

	if (!tab) {
		g_critical("The tab label is missing a corresponding tab");
		return;
	}

	web_view = browser_tab_get_web_view(tab);

	/* TODO: Do not use signal callbacks imperatively. */
	on_tab_title_changed(tab, tab_label);
	on_tab_state_changed(tab, NULL, tab_label);

	g_signal_connect_object(tab, "notify::state", G_CALLBACK(on_tab_state_changed), tab_label, 0);
	g_signal_connect(tab, "title-changed", G_CALLBACK(on_tab_title_changed), tab_label);
	g_signal_connect(web_view, "notify::favicon", G_CALLBACK(on_web_view_favicon_changed), tab_label);

	G_OBJECT_CLASS(browser_tab_label_parent_class)->constructed(object);
}

static void
browser_tab_label_class_init(BrowserTabLabelClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	object_class->set_property = browser_tab_label_set_property;
	object_class->get_property = browser_tab_label_get_property;
	object_class->constructed = browser_tab_label_constructed;

	properties[PROP_TAB] = g_param_spec_object("tab", "Tab", "The BrowserTab", BROWSER_TYPE_TAB, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties(object_class, LAST_PROP, properties);

	signals[CLOSE_CLICKED] = g_signal_new_class_handler("close-clicked",
			G_TYPE_FROM_CLASS(class),
			G_SIGNAL_RUN_LAST,
			G_CALLBACK(browser_tab_label_close_clicked),
			NULL, NULL, NULL,
			G_TYPE_NONE, 0);

	/* Bind class to template. */
	file = g_mapped_file_new("browser-tab-label.ui", FALSE, NULL);
	if (!file) {
		return;
	}
	bytes = g_mapped_file_get_bytes(file);
	gtk_widget_class_set_template(widget_class, bytes);
	g_mapped_file_unref(file);

	gtk_widget_class_bind_template_child(widget_class, BrowserTabLabel, icon);
	gtk_widget_class_bind_template_child(widget_class, BrowserTabLabel, spinner);
	gtk_widget_class_bind_template_child(widget_class, BrowserTabLabel, label);
	gtk_widget_class_bind_template_child(widget_class, BrowserTabLabel, close_button);
}

static void
browser_tab_label_init(BrowserTabLabel *tab_label)
{
	gtk_widget_init_template(GTK_WIDGET(tab_label));

	g_signal_connect(tab_label->close_button, "clicked", G_CALLBACK(on_close_button_clicked), tab_label);
}

BrowserTab *
browser_tab_label_get_tab(BrowserTabLabel *tab_label)
{
	g_return_val_if_fail(BROWSER_IS_TAB_LABEL(tab_label), NULL);

	return tab_label->tab;
}

GtkWidget *
browser_tab_label_new(BrowserTab *tab)
{
	return g_object_new(BROWSER_TYPE_TAB_LABEL, "tab", tab, NULL);
}
