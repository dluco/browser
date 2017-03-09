#ifndef _BROWSER_APP_H_
#define _BROWSER_APP_H_

#include <gtk/gtk.h>

#define BROWSER_APP_TYPE (browser_app_get_type())
G_DECLARE_FINAL_TYPE(BrowserApp, browser_app, BROWSER, APP, GtkApplication)

BrowserApp *browser_app_new(void);

#endif /* _BROWSER_APP_H_ */
