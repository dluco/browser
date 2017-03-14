browser: main.c browser-app.c browser-window.c browser-web-view.c browser-notebook.c
	gcc -Wall -g `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` $^ -o $@
