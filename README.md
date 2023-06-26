# EdiZon-Overlay [![Github All Releases](https://img.shields.io/github/downloads/proferabg/EdiZon-Overlay/total.svg)]()

Originally by [@WerWolv](https://www.github.com/WerWolv)

Continued Support by proferabg
<br />
<br />

# Latest Changelog - v1.0.7

    Squashed some more crashing issues!
    Fixed mismatched cheats when hiding and showing the overlay.
    Added a way to disable submenu logic for those who requested it.
<br />

# How To Use Submenus

In your cheat text document you can add submenus by using these 2 tags

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

