#include "browser-app.h"
#include <gtk/gtk.h>

int
main(int argc, char *argv[])
{
	return (int) g_application_run(G_APPLICATION(browser_app_new()), argc, argv);
}
