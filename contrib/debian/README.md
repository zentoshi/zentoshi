
Debian
====================
This directory contains files used to package zenxd/zenx-qt
for Debian-based Linux systems. If you compile zenxd/zenx-qt yourself, there are some useful files here.

## zenx: URI support ##


zenx-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install zenx-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your zenx-qt binary to `/usr/bin`
and the `../../share/pixmaps/zenx128.png` to `/usr/share/pixmaps`

zenx-qt.protocol (KDE)

