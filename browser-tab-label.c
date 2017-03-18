#include "browser-tab-label.h"

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
on_close_button_clicked(GtkWidget *widget, BrowserTabLabel *tab_label)
{
	g_signal_emit(tab_label, signals[CLOSE_CLICKED], 0, NULL);
}

static void
on_tab_title_changed(BrowserTab *tab, GParamSpec *pspec, BrowserTabLabel *tab_label)
{
	gchar *str;

	g_return_if_fail(tab == tab_label->tab);

	str = browser_tab_get_title(tab);
	g_return_if_fail(str != NULL);

	gtk_label_set_text(GTK_LABEL(tab_label->label), str);
	g_free(str);

	/* TODO: Sync tooltip. */
}

static void
on_tab_state_changed(BrowserTab *tab, GParamSpec *pspec, BrowserTabLabel *tab_label)
{
	g_return_if_fail(tab == tab_label->tab);

	/* TODO: Sync state. */
}

static void
browser_tab_label_close_clicked(BrowserTabLabel *tab_label)
{
}

static void
browser_tab_label_constructed(GObject *object)
{
	BrowserTabLabel *tab_label = BROWSER_TAB_LABEL(object);

	if (!tab_label->tab) {
		g_critical("The tab label is missing a corresponding tab");
		return;
	}

	on_tab_title_changed(tab_label->tab, NULL, tab_label);
	on_tab_state_changed(tab_label->tab, NULL, tab_label);

	g_signal_connect_object(tab_label->tab, "notify::title", G_CALLBACK(on_tab_title_changed), tab_label, 0);
	g_signal_connect_object(tab_label->tab, "notify::state", G_CALLBACK(on_tab_state_changed), tab_label, 0);

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
