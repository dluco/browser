#ifndef _BROWSER_TOOLBAR_H_
#define _BROWSER_TOOLBAR_H_

#include <gtk/gtk.h>

#define BROWSER_TYPE_TOOLBAR (browser_toolbar_get_type())
G_DECLARE_FINAL_TYPE(BrowserToolbar, browser_toolbar, BROWSER, TOOLBAR, GtkToolbar)

gchar *browser_toolbar_get_entry_text(BrowserToolbar *toolbar);
gboolean browser_toolbar_is_entry_modified(BrowserToolbar *toolbar);
GtkWidget *browser_toolbar_new(void);

#endif /* _BROWSER_TOOLBAR_H_ */
