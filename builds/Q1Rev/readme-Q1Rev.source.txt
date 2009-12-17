... and this, is the complete source code for Q1Rev.

It's based on Q1DS, a previous attempt to port the Quake engine to the Nintendo DS. This, in turn, is based on winquake, one of the projects that can be obtained at ftp://ftp.idsoftware.com/idstuff/source/q1source.zip .

As such, the engine implements the original software renderer in use (presumably) by the original DOS version, save of course for the assembly routines. Also, it automatically adjusts its resolution to match the one suggested by libogc.







The source in this package compiles under devkitPPC r19, libogc 1.8.1. For that, several important modifications were applied.

Almost all modifications were tagged with the following text:

// >>> FIX: For Nintendo Wii using devkitPPC / libogc

// {Description of the modification/bug}:
// {Previous code 1}
// {Previous code 2}
// {...}
// {Previous code n}
{New source 1}
{New source 2}
{...}
{New source n}
// <<< FIX

This way, any visual source code comparer tool can be used to infer the changes to the original sources.

The few modifications not handled this way correspond to either:

- cd_revol.c, in_revol.c, net_revol.c, snd_revol.c, sys_revol.c, vid_revol.c, and osk_revol.c (the "drivers" for the Wii), and

- Not needed .c files renamed to .c.donotinclude

This way, the /WinQuake/ folder can be compiled to a .dol application using a (slightly modified) standard template makefile under devkitPPC.

A .vcproj file was also included to be able to work on the sources using Visual C++ 2010 Express (or newer).








The modifications fall under several categories:

- Almost all big local variables in functions were removed from the stack, and into a new "big stack" implemented in sys_revol.c . Applications compiled under devkitPPC have a really tiny stack (128KB), and *many* functions in the sources have local variables in the range of 4kb - 256kb.

- Enums were converted to int variables, since they cannot be guaranteed to be 4-bytes in length, as required by the original source when loading data from disk. Note that qboolean, being itself a enum, was also converted to an int.

- A few rendering routines were modified so they don't do extra operations unless absolutely needed; in one of them, it was possible to eliminate floating-point variables in a performance-critical area. NOTE: This was inherited from the original Q1DS, done since the DS has no hardware FPU.




With all of these modifications, plus all the additions required for libogc, the Wii Remote Look Mode control schema, the new on-screen keyboard, the software sound mixing engine already present on the engine, and the network code, the engine was made fully playable at 30fps with no hiccups or any similar problems. 



Comments & suggestions can be sent to heribertod@gmail.com.





Original source code (C) 1996-1997 Id Software, Inc.
Modifications (c) 2009 Heriberto Delgado.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

All trademarks or registered trademarks used are properties of their respective owners.
