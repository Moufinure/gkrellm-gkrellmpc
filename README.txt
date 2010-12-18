$Header: /cvsroot/gkrellmpc/README.txt,v 1.7 2005/03/05 22:27:27 mina Exp $

README.txt for GKrellMPC
------------------------------------------------------------



About:
------

GKrellMPC is a GKrellm plugin that allows you to control your MPD music daemon from within gkrellm.

Plugin written by Mina Naguib <webmaster@topfx.com>
http://www.topfx.com



Requirements:
-------------

You will need gkrellm with it's full headers (some distros call this the "dev" package)
You will also need GTK with it's headers
You will need libcurl with it's headers, including the "curl-config" binary



Compiling:
----------

Simply run "make" and you should end up with a gkrellmpc.so

To compile with NLS (multi-language) support, run "make enable_nls=1" instead



Installation:
-------------

1. Run "make install" (or "make enable_nls=1 install" to enable NLS support) - this will try to install gkrellmpc.so in the most appropriate location found, or instruct you if it cannot for any reason - it will also attempt to install the NLS .mo files
2. Restart gkrellm
3. Right-click on GKrellm's title - go to the "Plugins" tab and make sure "GKrellMPC" is checked



Configuration:
--------------

1. Right-click anywhere inside the GKrellMPC area
2. Specify the hostname and port of your MPD server



Use:
----

The top-right light tells you whether the plugin is connected to an MPD server or not.

The scrolling line tells you the currently-playing song.

The volume krell shows you the current volume. You can drag it to adjust the volume.

The buttons are standard audio hi-fi functions (prev/play/pause/stop/next/eject - playlist).



Mouse shortcuts:
----------------

MOUSE WHEEL UP or DOWN adjusts the volume.

RIGHT MOUSE BUTTON opens the configuration dialogue.

MIDDLE MOUSE BUTTON plays/pauses.



Copyright:
----------

This software is copyright 2004-2005 Mina Naguib <webmaster@topfx.com>
http://www.topfx.com
Released under the GNU GPL LICENSE version 2.0

