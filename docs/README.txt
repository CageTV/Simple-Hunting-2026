Simple Hunting Overhaul 2026
Based on Simple Hunting Overhaul (Nexus 95943) by JaySerpa. Needs the original mod and Dynamic Activation Key. Part of the 2026 series with
Campfire 2026, Frostfall 2026 and Last Seed 2026.

WHAT IT DOES
------------
All the settings of Simple Hunting Overhaul move into SKSE Menu Framework, in one "Simple Hunting Overhaul" section. It replaces both menu add-ons
("Simple Hunting Overhaul - MCM" and "Simple Hunting Overhaul MCM Helper"): turn those two off, they cannot work together. It needs neither SkyUI
nor MCM Helper.

  Overview        what is installed, your hunting experience, and how long a skinning takes right now for every animal
  General         looting mode (time / fade), skinned animal look, skinning your own horse, hunting permits, experience, debug messages
  Rewards         the gold for the five carcass sizes
  Skinning Time   the experience floor, the shortest and longest skinning, and the base time of every animal
  Animations      the options of Immersive Hunting Animations (loot mode, which animations play, dagger, displays, QuickLoot, ...)
  Controls        Dynamic Activation Key's key, and four hunting hotkeys: pick up carcass, next looting mode, toggle skinned look, show experience
  Profile         the settings file, applying it on every load, saving, loading and resetting

Every option and default is the original mods' own. The settings are the original mods' global variables, which the pages read and write directly.
A copy is kept in Data/SKSE/Plugins/SimpleHunting/settings.ini and applied on every game load, so your settings follow you to a new character. The
first time you load a game, whatever the game already has is taken into that file, so nothing resets.

THE OPTIMISED SCRIPTS (optional, GitHub only)
--------------------------------------------
Two of the original mod's scripts are rebuilt with the same behaviour and fewer calls: SHO_PickUpCarCass reads the animal's voice type once (it did
up to twelve native calls) and no longer errors when you press the pick-up key with nothing in the crosshair; SHO_PlayerAliasScript looks the animation
mod's globals up once instead of on every skinning, and reads the actor and the looting mode once. Nothing about how skinning plays changes.
They contain modified copies of JaySerpa's scripts, so they are not uploaded to Nexus: download "Simple Hunting Overhaul 2026 - Optimised Scripts" from
https://github.com/CageTV/Simple-Hunting-2026/releases and install it after the original mod. Everything else here works without them.

REQUIREMENTS
------------
- Simple Hunting Overhaul (Nexus 95943) and Dynamic Activation Key, as the original requires
- SKSE Menu Framework 3
- Optional: Immersive Hunting Animations (the Animations page), Immersive Equipment Displays (dagger and carcass displays)
- Turn off "Simple Hunting Overhaul - MCM", "Simple Hunting Overhaul MCM Helper", and the menu part of Immersive Hunting Animations (its MCM Helper or
  SkyUI menu), or they will put their own values back on every load.

LOAD ORDER
----------
Install after (below) the original Simple Hunting Overhaul in MO2's left pane. The plugin has no .esp.

BUILDS
------
The installer asks which SimpleHunting.dll: "new" for Skyrim 1.6.1170 and newer, "older" for Skyrim VR and 1.6.1130 and older.

CREDITS AND LICENSE
-------------------
Simple Hunting Overhaul by JaySerpa. Immersive Hunting Animations by Elsawirr and GiraPomba. Dynamic Activation Key by its author. The two menus this
replaces were made by thmsn and MaskedRPGFan. The settings pages and SimpleHunting.dll are by CageTV, MIT licensed (see LICENSE.txt); they contain
none of the original mods' code, meshes, textures or sounds. The optional scripts (GitHub only) are JaySerpa's own scripts, rebuilt with fewer calls and shared with credit; they are not covered by that license and
they come down on the author's request.
