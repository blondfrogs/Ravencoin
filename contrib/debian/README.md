
Debian
====================
This directory contains files used to package blastd/blast-qt
for Debian-based Linux systems. If you compile blastd/blast-qt yourself, there are some useful files here.

## blast: URI support ##


blast-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install blast-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your blast-qt binary to `/usr/bin`
and the `../../share/pixmaps/blast128.png` to `/usr/share/pixmaps`

blast-qt.protocol (KDE)

