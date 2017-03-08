#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
	GtkBuilder *builder;
	GtkWidget *window, *container;
	GtkWidget *web_view;
	char *uri;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new_from_file("browser.ui");

	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	container = GTK_WIDGET(gtk_builder_get_object(builder, "container"));
//	web_view = GTK_WIDGET(gtk_builder_get_object(builder, "web_view"));

//	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	web_view = webkit_web_view_new();

	uri = (argc > 1) ?
		argv[1] :
		"https://www.google.ca";

	printf("Uri: %s\n", uri);

	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), uri);

	gtk_box_pack_end(GTK_BOX(container), web_view, TRUE, TRUE, 0);

//	gtk_container_add(GTK_CONTAINER(window), web_view);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
