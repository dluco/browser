browser: main.c browser-app.c browser-app-window.c
	gcc -Wall -g `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` $^ -o $@
