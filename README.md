# SLumpEd

**Note that this project is discontinued as it has been completely superseded by [SLADE](https://github.com/sirjuddington/SLADE). I am just putting the source up here for archival purposes.**

Overview
--------
SLumpEd is a wad lump management tool. Sure there are many of these out already,
but this one I designed just to be simple, and to be able to import any kind of
file as a lump, without the manager complaining and/or converting it to an
undesirable format. The layout and controls should really be self-explanatory.


Features
--------
 - All the basic features you'd expect of a wad lump manager
   (create/open/save wads, manipulate lump ordering, import/export lumps, etc)
 - Full zip/pk3 support including directory manipulation
 - Open multiple wadfiles at once in tabs
 - Cut/copy/paste lumps either within a wad or across multiple wads
 - Simple, easy-to-use, nice looking interface
 - Advanced text editor with syntax hilighting and autocomplete for various
   text lump types (ACS scripts, DECORATE, etc)
 - Full PNG image support, including offset manipilation and high-colour support
 - Compile ACS scripts
 - Convert between PNG/BMP/Doom Gfx/Doom Flat image formats
 - Ability to edit TEXTUREx lumps


Planned Features
----------------
 - Wad merging
 - Convert between doom wad and zip formats
 - View more lump types (map data, palette, etc)
 - Open lumps in external programs for editing
 - Much more...


Installation
------------
Just extract the program somewhere (preferrably to it's own directory).


Shortcut Keys
-------------

**Lump List:**  
 - `Delete` - Delete
 - `Ctrl+U` - Move up
 - `Ctrl+D` - Move down
 - `Ctrl+X` - Cut
 - `Ctrl+C` - Copy
 - `Ctrl+V` - Paste
 - `Ctrl+I` - Import
 - `Ctrl+E` - Export
 - `Ctrl+W` - Export To Wad
 - `Ctrl+R` or `F2` - Rename
 - `Ctrl+Alt+R` - Reload

**TEXTUREx Editor:**  
 - `Delete` - Remove selected patch
 - `(Arrow Keys)` - Move selected patch
 - `Ctrl+B` - Push patch back
 - `Ctrl+F` - Bring patch forward
 - `Ctrl+Left` - Next patch
 - `Ctrl+Right` - Previous patch
