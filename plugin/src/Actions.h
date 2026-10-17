/*
 * Simple Hunting Overhaul 2026
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

#include "Settings.h"

// What the plugin does besides keeping settings: the calls into the animation mod's scripts that some settings need, the hunting hotkeys,
// and the skinning-time arithmetic (the same as the main mod's pelt script, so the pages can show what a skinning takes right now).
namespace Actions
{
	// Hotkey slots (Settings::Def::slot)
	inline constexpr int kSlotDak = 0;
	inline constexpr int kSlotPickUp = 1;
	inline constexpr int kSlotCycleLooting = 2;
	inline constexpr int kSlotSkinnedLook = 3;
	inline constexpr int kSlotShowXp = 4;

	// Runs the side effect of a changed setting (calls into the animation mod's scripts) on the game thread.
	void Run(Settings::After a_after, float a_value);

	// Listens for the hunting hotkeys. Call once the game data is loaded.
	void RegisterInput();

	// The share of the base time a skinning takes with the current experience (1 = no experience, down to the "fastest you get" floor).
	float Multiplier();
	// Hours a skinning takes right now for an animal with that base time (an animal of normal size).
	float HoursFor(float a_baseHours);
	// "Hunting experience 37 / 100 ..." for the Overview page and the hotkey.
	std::string XpSummary();
}
