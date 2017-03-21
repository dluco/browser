#include "browser-notebook.h"
#include "browser-tab-label.h"
#include "browser-tab.h"
#include <glib-object.h>
#include <glib.h>
#include <glib/gstdio.h>

struct _BrowserNotebook
{
	GtkNotebook parent;

	GtkWidget *new_tab_button;
};

enum
{
	NEW_TAB,
	CLOSE_TAB,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE(BrowserNotebook, browser_notebook, GTK_TYPE_NOTEBOOK)

static void
on_new_tab_button_clicked(GtkButton *button, BrowserNotebook *notebook)
{
	g_print("Notebook: new tab clicked\n");

	g_signal_emit(notebook, signals[NEW_TAB], 0, NULL);
}

static void
on_close_tab_button_clicked(BrowserTabLabel *tab_label, BrowserNotebook *notebook)
{
	BrowserTab *tab;

	g_print("Notebook: close tab clicked\n");

	tab = browser_tab_label_get_tab(tab_label);
	g_signal_emit(notebook, signals[CLOSE_TAB], 0, tab);
}

static void
browser_notebook_new_tab(BrowserNotebook *notebook)
{
}

static void
browser_notebook_close_tab(BrowserNotebook *notebook, BrowserTab *tab)
{
}

static void
browser_notebook_page_added(GtkNotebook *notebook, GtkWidget *page, guint page_num)
{
	GtkWidget *tab_label;

	g_return_if_fail(BROWSER_IS_TAB(page));

	tab_label = gtk_notebook_get_tab_label(notebook, page);
	g_return_if_fail(BROWSER_IS_TAB_LABEL(tab_label));

	g_signal_connect(tab_label, "close-clicked", G_CALLBACK(on_close_tab_button_clicked), notebook);
}

static void
browser_notebook_remove(GtkContainer *container, GtkWidget *widget)
{
	GtkNotebook *notebook = GTK_NOTEBOOK(container);
	GtkWidget *tab_label;

	g_return_if_fail(BROWSER_IS_TAB(widget));

	tab_label = gtk_notebook_get_tab_label(notebook, widget);
	g_return_if_fail(BROWSER_IS_TAB_LABEL(tab_label));

	g_signal_handlers_disconnect_by_func(tab_label, G_CALLBACK(on_close_tab_button_clicked), notebook);

	if (GTK_CONTAINER_CLASS(browser_notebook_parent_class)->remove != NULL) {
		GTK_CONTAINER_CLASS(browser_notebook_parent_class)->remove(container, widget);
	}
}

static void
browser_notebook_class_init(BrowserNotebookClass *class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS(class);
	GtkNotebookClass *notebook_class = GTK_NOTEBOOK_CLASS(class);
	GMappedFile *file;
	GBytes *bytes;

	/* Cannot use notebook_class->page_removed since at the point that it
	 * is called, the page is already removed and the tab_label cannot be
	 * retrieved.
	 */
	container_class->remove = browser_notebook_remove;

	notebook_class->page_added = browser_notebook_page_added;

	signals[NEW_TAB] = g_signal_new_class_handler("new-tab",
			G_TYPE_FROM_CLASS(class),
			G_SIGNAL_RUN_LAST,
			G_CALLBACK(browser_notebook_new_tab),
			NULL, NULL, NULL,
			G_TYPE_NONE, 0);

	signals[CLOSE_TAB] = g_signal_new_class_handler("close-tab",
			G_TYPE_FROM_CLASS(class),
			G_SIGNAL_RUN_LAST,
			G_CALLBACK(browser_notebook_close_tab),
			NULL, NULL, NULL,
			G_TYPE_NONE,
			1,
			BROWSER_TYPE_TAB);

	file = g_mapped_file_new("browser-notebook.ui", FALSE, NULL);
	if (!file) {
		return;
	}
	bytes = g_mapped_file_get_bytes(file);
	gtk_widget_class_set_template(widget_class, bytes);
	g_mapped_file_unref(file);

	gtk_widget_class_bind_template_child(widget_class, BrowserNotebook, new_tab_button);
}

static void
browser_notebook_init(BrowserNotebook *notebook)
{
	gtk_widget_init_template(GTK_WIDGET(notebook));

	g_signal_connect(notebook->new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), notebook);
}

void
browser_notebook_add_tab(BrowserNotebook *notebook, BrowserTab *tab, gint position, gboolean jump_to)
{
	GtkWidget *tab_label;

	g_return_if_fail(BROWSER_IS_NOTEBOOK(notebook));
	g_return_if_fail(BROWSER_IS_TAB(tab));

	tab_label = browser_tab_label_new(BROWSER_TAB(tab));

	gtk_notebook_insert_page(GTK_NOTEBOOK(notebook), GTK_WIDGET(tab), tab_label, position);

	g_print("Notebook: Setting new tab props.\n");

	/* Set tab properties. */
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook), GTK_WIDGET(tab), TRUE);
	gtk_notebook_set_tab_detachable(GTK_NOTEBOOK(notebook), GTK_WIDGET(tab), TRUE);

	/* Position may have changed after insertion due to signal handlers. */
	position = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), GTK_WIDGET(tab));

	if (jump_to) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), position);
		gtk_widget_grab_focus(GTK_WIDGET(tab));
	}
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
