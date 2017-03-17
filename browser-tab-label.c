#include "browser-tab-label.h"

struct _BrowserTabLabel
{
	GtkBox parent;
};

G_DEFINE_TYPE(BrowserTabLabel, browser_tab_label, GTK_TYPE_BOX);

static void
browser_tab_label_init(BrowserTabLabel *tab_label)
{
	gtk_widget_init_template(GTK_WIDGET(tab_label));
}

static void
browser_tab_label_class_init(BrowserTabLabelClass *class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	file = g_mapped_file_new("browser-tab-label.ui", FALSE, NULL);
	if (!file) {
		return;
	}
	bytes = g_mapped_file_get_bytes(file);
	gtk_widget_class_set_template(widget_class, bytes);
	g_mapped_file_unref(file);

//	gtk_widget_class_bind_template_child(widget_class, BrowserTabLabel, web_view);
}

GtkWidget *
browser_tab_label_new(void)
{
	return g_object_new(BROWSER_TYPE_TAB_LABEL, NULL);
}
