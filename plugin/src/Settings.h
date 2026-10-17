/*
 * Simple Hunting Overhaul 2026
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

// Every setting of Simple Hunting Overhaul 2026's pages. A setting is one global variable of the original mods (the main mod's, Immersive
// Hunting Animations' or Dynamic Activation Key's): the pages read and write that global directly, and the value is also kept in
// Data/SKSE/Plugins/SimpleHunting/settings.ini so it survives a new character and is applied again on every game load.
namespace Settings
{
	enum class Kind { Toggle, Slider, Combo, Key };

	// What else has to happen when the value changes (the animation mod keeps some state in its scripts, not in the global).
	enum class After { None, IHStartMod, IHPickup, IHQuickLoot, IHUseLastDagger, IHDisplays, IHDaggerPos };

	struct Def
	{
		const char*        id;       // key in settings.ini
		const char*        label;
		const char*        tip;
		Kind               kind;
		const char*        plugin;   // plugin that holds the global; nullptr = a hotkey kept only in settings.ini
		unsigned int       local;    // local FormID of the global (light plugins: the low 12 bits)
		float              def, min, max, step;
		const char*        fmt;      // slider display format (printf, one float)
		float              shown;    // slider display factor (100 shows 0.2 as 20)
		const char* const* options;  // combo entries
		int                optionCount;
		const float*       optionValues;  // value stored for each combo entry; nullptr = the entry's index
		After              after;
		bool               profile;  // kept in settings.ini and applied on load (false: progress or another mod's own setting)
		int                slot;     // hotkeys: slot number (see Actions.h)
	};

	struct Page
	{
		const char* title;
		const Def*  defs;
		int         count;  // headers and column breaks are defs with kind Slider and a null id: see IsHeader
	};

	inline constexpr int kPageTotal = 5;
	extern const Page kPages[kPageTotal];

	bool IsHeader(const Def& a_d);   // a section title (label only)
	bool IsColumn(const Def& a_d);   // a column break

	const Def* Find(std::string_view a_id);
	bool       Available(const Def& a_d);  // the plugin and the global are there
	float      Get(const Def& a_d);
	void       Set(const Def& a_d, float a_value);  // writes the global, the file, and runs the side effect
	int        KeyOf(const Def& a_d);               // a hotkey's scan code (0 = none)
	void       SetKey(const Def& a_d, int a_key);

	bool Loaded(const char* a_plugin);  // is that plugin in the load order

	// Game load / new game: applies settings.ini to the globals (or writes the file from the globals the first time).
	void OnGameLoaded();
	bool ApplyOnLoad();
	void SetApplyOnLoad(bool a_on);
	void SaveNow();       // write settings.ini from the current globals
	void LoadNow();       // apply settings.ini to the globals now
	void ResetDefaults();
	void Tick(float a_dt);  // flushes the file a moment after the last change
	const char* FilePath();
}
