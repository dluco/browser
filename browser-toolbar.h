#ifndef _BROWSER_TOOLBAR_H_
#define _BROWSER_TOOLBAR_H_

#include <gtk/gtk.h>

#define BROWSER_TYPE_TOOLBAR (browser_toolbar_get_type())
G_DECLARE_FINAL_TYPE(BrowserToolbar, browser_toolbar, BROWSER, TOOLBAR, GtkToolbar)

void browser_toolbar_update_buttons(BrowserToolbar *toolbar, gboolean can_go_back, gboolean can_go_forward);
void browser_toolbar_set_entry_uri(BrowserToolbar *toolbar, const gchar *uri, gboolean user);
gchar *browser_toolbar_get_entry_text(BrowserToolbar *toolbar);
gboolean browser_toolbar_is_entry_modified(BrowserToolbar *toolbar);
GtkWidget *browser_toolbar_new(void);

#endif /* _BROWSER_TOOLBAR_H_ */
