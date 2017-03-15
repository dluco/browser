#include "browser-notebook.h"
#include "browser-web-view.h"
#include <glib-object.h>
#include <glib.h>
#include <glib/gstdio.h>

struct _BrowserNotebook
{
	GtkNotebook parent;

	BrowserWebView *web_view;
};

G_DEFINE_TYPE(BrowserNotebook, browser_notebook, GTK_TYPE_NOTEBOOK);

static void
browser_notebook_init(BrowserNotebook *notebook)
{
	char *uri = "https://www.google.ca";

	g_type_ensure(BROWSER_TYPE_WEB_VIEW);

	gtk_widget_init_template(GTK_WIDGET(notebook));

	printf("Uri: %s\n", uri);

	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(notebook->web_view), uri);
}

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

	gtk_widget_class_bind_template_child(widget_class, BrowserNotebook, web_view);
}

GtkWidget *
browser_notebook_new(void)
{
	return g_object_new(BROWSER_TYPE_NOTEBOOK, NULL);
}
