#include "browser-download-manager.h"

#include <gio/gio.h>

struct _BrowserDownloadManager
{
	GObject parent;

	GList *downloads;
};

G_DEFINE_TYPE(BrowserDownloadManager, browser_download_manager, G_TYPE_OBJECT)

static gboolean
on_download_decide_destination(WebKitDownload *download,
								gchar *suggested_filename,
								BrowserDownloadManager *manager)
{
	const gchar *user_download_dir;
	gchar *path;
	gchar *destination;

	g_print("Download Manager: downloading \"%s\"\n", suggested_filename);

	/* TODO: Use a setting to control the download directory. */
	user_download_dir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
	path = g_build_filename(user_download_dir, suggested_filename, NULL);
	destination = g_filename_to_uri(path, NULL, NULL);

	g_print("Destination: \"%s\"\n", destination);

	webkit_download_set_destination(download, destination);

	g_free(path);
	g_free(destination);

	/* Handled. */
	return TRUE;
}

static void
on_download_created_destination(WebKitDownload *download,
								gchar *destination,
								BrowserDownloadManager *manager)
{
	g_print("Download Manager: created download destination: %s\n", destination);
}

static void
browser_download_manager_dispose(GObject *object)
{
	BrowserDownloadManager *manager = BROWSER_DOWNLOAD_MANAGER(object);

	g_list_free_full(manager->downloads, g_object_unref);

	G_OBJECT_CLASS(browser_download_manager_parent_class)->dispose(object);
}

static void
browser_download_manager_class_init(BrowserDownloadManagerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	object_class->dispose = browser_download_manager_dispose;
}

static void
browser_download_manager_init(BrowserDownloadManager *manager)
{
}

void
browser_download_manager_add_download(BrowserDownloadManager *manager,
		WebKitDownload *download)
{
	g_return_if_fail(BROWSER_IS_DOWNLOAD_MANAGER(manager));
	g_return_if_fail(WEBKIT_IS_DOWNLOAD(download));

	/* Skip adding download if already added. */
	if (g_list_find(manager->downloads, download))
		return;

	manager->downloads = g_list_prepend(manager->downloads, g_object_ref(download));

	g_signal_connect(download, "decide-destination", G_CALLBACK(on_download_decide_destination), manager);
	g_signal_connect(download, "created-destination", G_CALLBACK(on_download_created_destination), manager);
}

static gpointer
create_default_download_manager(gpointer data)
{
	return browser_download_manager_new();
}

BrowserDownloadManager *
browser_download_manager_get_default(void)
{
	static GOnce once_init = G_ONCE_INIT;
	return BROWSER_DOWNLOAD_MANAGER(g_once(&once_init, create_default_download_manager, NULL));
}

BrowserDownloadManager *
browser_download_manager_new(void)
{
	return BROWSER_DOWNLOAD_MANAGER(g_object_new(BROWSER_TYPE_DOWNLOAD_MANAGER, NULL));
}
