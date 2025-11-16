# EdiZon-Overlay [![Github All Releases](https://img.shields.io/github/downloads/proferabg/EdiZon-Overlay/total.svg)]()

Originally by [@WerWolv](https://www.github.com/WerWolv)

Continued Support by proferabg & ppkantorski
<br />
<br />

# Latest Changelog - v1.0.10
    Updated libultrahand (fixes #33)
<br />

# How To Use Submenus

In your cheat text document, you can add submenus by using these 2 tags

    [--SectionStart:<Section Name>--]
    [--SectionEnd:<Section Name>--]

Example:

    [--SectionStart:Item Codes--]
    00000000 00000000 00000000    

    [Items x999]
    040A0000 01DB2A08 52807CE0

    [--SectionEnd:Item Codes--]
    00000000 00000000 00000000

This will create a submenu called ***Item Codes*** with ***Items x999*** being a cheat in that submenu.

You can also disable the submenu feature (skips submenu items in cheat file).
To do so, insert the following at the very top of the file.

    [--DisableSubmenus--]
    00000000 00000000 00000000

Warning: Having too many cheats at once can lead to stability issues and can crash.
<br />

# How To Enable Cheats by Default on Overlay Open

In your cheat text document, you can have cheats turn on by default on opening of the overlay.
To do this, simply find the cheat you want to enable by default and add ***:ENABLED*** onto the end of the name.

Example:

    [60 FPS Mod:ENABLED]
    00000000 00000000 00000000

Warning: This will turn the cheat on every time the overlay is relaunched. Also, turning the cheat off in the menu and relaunching the overlay will turn the cheat back on.
