# EdiZon-Overlay [![Github All Releases](https://img.shields.io/github/downloads/Arch9SK7/EdiZon-Overlay/total.svg)]()

Originally by [@WerWolv](https://www.github.com/WerWolv)

Continued Support by proferabg & ppkantorski & Arch9SK7
<br />
<br />

# Latest Changelog - v1.0.15
    Update libultrahand to v2.2.5
	Added new Cheat notes functionality
<br />

# How To Use Cheat Notes

In your atmosphere game directory ex. /atmosphere/contents/[TID]/cheats/
Place a new text file called notes.txt in that directory with all your cheats for the game.

In your notes text document, you can add notes using this tag

	[<Cheat Name>]
	
Example: 

	[hp doesnt decrease]
	"Hold ZL to enable infinite HP"
	[Restore Code (Use after unchecking any cheats below)]
	"Use this to disable any asm codes used"
	
This will help keep your cheat files clean and allow users/cheat devs to opt in or out and use notes on their codes.
Only if the name itself matches one of the cheat files actual cheat names will it validate the note. 
If the note is valid you will see the "Y" button be offered next to your cheat. Press "Y" to open the note and same to close.
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
