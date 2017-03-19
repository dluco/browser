#include "browser-notebook.h"
#include "browser-tab-label.h"
#include "browser-tab.h"
#include <glib-object.h>
#include <glib.h>
#include <glib/gstdio.h>

struct _BrowserNotebook
{
	GtkNotebook parent;

	BrowserTab *tab;
	BrowserTabLabel *tab_label;
};

G_DEFINE_TYPE(BrowserNotebook, browser_notebook, GTK_TYPE_NOTEBOOK)

static void
browser_notebook_class_init(BrowserNotebookClass *class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	file = g_mapped_file_new("browser-notebook.ui", FALSE, NULL);
	if (!file) {
		return;
	}
	bytes = g_mapped_file_get_bytes(file);
	gtk_widget_class_set_template(widget_class, bytes);
	g_mapped_file_unref(file);
}

static void
browser_notebook_init(BrowserNotebook *notebook)
{
	GtkWidget *tab;
	GtkWidget *tab_label;

	gtk_widget_init_template(GTK_WIDGET(notebook));

	tab = browser_tab_new();
	tab_label = browser_tab_label_new(BROWSER_TAB(tab));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab, tab_label);
}

BrowserTab *
browser_notebook_get_active_tab(BrowserNotebook *notebook)
{
	GtkWidget *tab = NULL;
	gint curr_page;

	g_return_val_if_fail(BROWSER_IS_NOTEBOOK(notebook), NULL);

	curr_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	if (curr_page >= 0) {
		tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), curr_page);
	}
	return BROWSER_IS_TAB(tab) ? BROWSER_TAB(tab) : NULL;
}

GtkWidget *
browser_notebook_new(void)
{
	return g_object_new(BROWSER_TYPE_NOTEBOOK, NULL);
}
