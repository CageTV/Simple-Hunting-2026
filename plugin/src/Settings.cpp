/*
 * Simple Hunting Overhaul 2026
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Settings.h"
#include "Actions.h"

namespace Settings
{
	namespace
	{
		constexpr const char* kSHO = "Simple Hunting Overhaul.esp";
		constexpr const char* kIH = "ImmersiveHunting.esp";
		constexpr const char* kDAK = "Dynamic Activation Key.esp";
		constexpr const char* kFile = "Data/SKSE/Plugins/SimpleHunting/settings.ini";
		constexpr float       kWriteDelay = 0.6f;

		constexpr unsigned kIHPerk = 0x000D42;  // IH_PK_ImmersiveHunting: the perk StartMod adds to the player

		// ---- table helpers -------------------------------------------------------------------------------------------------------------
#define HEADER(l) { nullptr, l, "", Kind::Slider, nullptr, 0, 0, 0, 0, 0, "", 1, nullptr, 0, nullptr, After::None, false, 0 }
#define COLUMN { nullptr, "", "", Kind::Slider, nullptr, 0, 0, 0, 0, 0, "", 1, nullptr, 0, nullptr, After::None, false, 0 }
#define TOG(id, l, tip, pl, loc, d, prof, aft) { id, l, tip, Kind::Toggle, pl, loc, d, 0, 1, 1, "", 1, nullptr, 0, nullptr, aft, prof, 0 }
#define SLD(id, l, tip, pl, loc, d, mn, mx, st, f, sh, prof) { id, l, tip, Kind::Slider, pl, loc, d, mn, mx, st, f, sh, nullptr, 0, nullptr, After::None, prof, 0 }
#define CMB(id, l, tip, pl, loc, d, opts, vals, aft, prof) { id, l, tip, Kind::Combo, pl, loc, d, 0, 0, 1, "", 1, opts, static_cast<int>(std::size(opts)), vals, aft, prof, 0 }
#define KEY(id, l, tip, pl, loc, sl, prof) { id, l, tip, Kind::Key, pl, loc, 0, 0, 0, 0, "", 1, nullptr, 0, nullptr, After::None, prof, sl }

		constexpr const char* kLooting[] = { "No time passes, no fade", "Time passes and the screen fades", "Only the screen fades", "Only time passes" };
		constexpr const char* kLootMode[] = { "Vanilla (loot the carcass normally)", "Prompt (ask what to do)", "Process (do it all automatically)" };
		constexpr const char* kDagger[] = { "Ankle", "Back hip" };
		constexpr float       kDaggerValues[] = { 0.0f, 10.0f };

		// ---- the pages -----------------------------------------------------------------------------------------------------------------
		constexpr Def kGeneral[] = {
			HEADER("Skinning"),
			CMB("looting_mode", "When Looting a Kill", "What happens while you take a pelt: time passes, the screen fades, both, or neither. How long it takes comes from the Skinning Time page.", kSHO, 0x821, 1, kLooting, nullptr, After::None, true),
			TOG("skinned_look", "Skinned Animal Look", "A skinned animal is shown without its fur or hide.", kSHO, 0x8E4, 1, true, After::None),
			TOG("horse_skinning", "Skin Your Own Horse", "Allows skinning a horse that is yours.", kSHO, 0xCFB, 0, true, After::None),
			TOG("hunting_permits", "Hunting Permits", "Guards ask for a hunting permit when you carry game inside their hold.", kSHO, 0x83D, 1, true, After::None),
			COLUMN,
			HEADER("Experience"),
			SLD("xp", "Hunting Experience", "Your hunting experience, 0 to 100. Every pelt you take adds 1 and shortens the time skinning takes. Changing it here is a cheat; it is not part of the saved settings.", kSHO, 0x822, 1, 0, 100, 1, "%.0f / 100", 1, false),
			HEADER("Debug"),
			TOG("debug_messages", "Debug Messages", "Shows what the skinning script is doing as notifications and in the Papyrus log.", kSHO, 0xC05, 0, true, After::None),
		};

		constexpr Def kRewards[] = {
			HEADER("Carcass Rewards (gold)"),
			SLD("reward_small", "Small Carcass", "Skeever, dog.", kSHO, 0x841, 25, 0, 1000, 5, "%.0f Gold", 1, true),
			SLD("reward_medium", "Medium Carcass", "Wolf, ice wolf, fox, snow fox, chicken, hare, slaughterfish, mudcrab.", kSHO, 0x842, 50, 0, 1000, 5, "%.0f Gold", 1, true),
			SLD("reward_large", "Large Carcass", "Goat, large and giant mudcrab.", kSHO, 0x843, 75, 0, 1000, 5, "%.0f Gold", 1, true),
			SLD("reward_xl", "XL Carcass", "Deer, male and female elk.", kSHO, 0x844, 100, 0, 1000, 5, "%.0f Gold", 1, true),
			SLD("reward_xxl", "XXL Carcass", "Vale deer.", kSHO, 0x845, 300, 0, 1000, 5, "%.0f Gold", 1, true),
		};

		constexpr Def kTimes[] = {
			HEADER("How long skinning takes"),
			SLD("xp_floor", "Fastest You Get", "At full experience, skinning takes this share of the base time of the animal.", kSHO, 0xC06, 0.2f, 0.01f, 0.99f, 0.01f, "%.0f %%", 100, true),
			SLD("time_min", "Shortest Skinning", "Skinning never takes less than this, in hours.", kSHO, 0xBF7, 0.15f, 0.0f, 10.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_max", "Longest Skinning", "Skinning never takes longer than this, in hours. 0 means no limit.", kSHO, 0xBF6, 4.0f, 0.0f, 24.0f, 0.01f, "%.2f h", 1, true),
			COLUMN,
			HEADER("Base time per animal (hours, before experience)"),
			SLD("time_mammoth", "Mammoth / Dragon", "", kSHO, 0xBF8, 4.0f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_bear", "Bear", "", kSHO, 0xBF9, 2.66f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_troll", "Troll / Werewolf", "", kSHO, 0xBFA, 2.5f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_sabrecat", "Sabre Cat", "", kSHO, 0xBFB, 2.33f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_horsecow", "Horse / Cow", "", kSHO, 0xBFC, 2.0f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_elkdeer", "Elk / Deer", "", kSHO, 0xBFD, 1.33f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_wolfboar", "Wolf / Boar", "", kSHO, 0xBFE, 1.0f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_skeeverdog", "Skeever / Dog", "", kSHO, 0xBFF, 0.75f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_goat", "Goat", "", kSHO, 0xC00, 0.66f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_fox", "Fox", "", kSHO, 0xC01, 0.5f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
			SLD("time_rabbit", "Rabbit", "", kSHO, 0xC02, 0.15f, 0.01f, 24.0f, 0.01f, "%.2f h", 1, true),
		};

		constexpr Def kAnimations[] = {
			HEADER("Immersive Hunting Animations"),
			TOG("ih_enabled", "Animations Enabled", "Starts or stops the whole animation mod (it gives or removes its perk).", kIH, kIHPerk, 1, true, After::IHStartMod),
			CMB("ih_loot_mode", "Loot Mode", "Vanilla leaves carcasses alone. Prompt asks what to do with a carcass. Process does every step by itself.", kIH, 0xEA0, 0, kLootMode, nullptr, After::None, true),
			SLD("ih_variants", "Animation Variants", "How many different animations it chooses from (1 to 5).", kIH, 0xD7A, 5, 1, 5, 1, "%.0f", 1, true),
			TOG("ih_process", "Process Animation", "Play the animation when a carcass is processed.", kIH, 0xE9F, 1, true, After::None),
			TOG("ih_arrow", "Remove Arrows", "Play the animation for pulling arrows out.", kIH, 0xD75, 1, true, After::None),
			TOG("ih_skin", "Skin", "Play the animation for skinning.", kIH, 0xD76, 1, true, After::None),
			TOG("ih_harvest", "Harvest", "Play the animation for harvesting parts.", kIH, 0xD77, 1, true, After::None),
			TOG("ih_butcher", "Butcher", "Play the animation for butchering meat.", kIH, 0xD78, 1, true, After::None),
			TOG("ih_extract", "Extract", "Play the animation for extracting blood and venom.", kIH, 0xD79, 1, true, After::None),
			COLUMN,
			HEADER("Behaviour"),
			TOG("ih_third", "Force Third Person", "Switches to third person for the animations.", kIH, 0xD7F, 1, true, After::None),
			TOG("ih_sheathe", "Sheathe Weapons", "Puts your weapons away before an animation.", kIH, 0xD80, 0, true, After::None),
			TOG("ih_dispose", "Dispose of Carcasses", "Lets you dispose of a carcass after processing it.", kIH, 0xEC2, 1, true, After::None),
			TOG("ih_shovel", "Bury With a Shovel", "Disposing of a carcass uses a shovel and buries it.", kIH, 0xEA3, 0, true, After::None),
			TOG("ih_campfire", "Campfire Cooking", "Cook at a campfire with the animations.", kIH, 0xE92, 1, true, After::None),
			TOG("ih_bottle", "Require Bottles", "Extracting blood and venom needs an empty bottle.", kIH, 0xEF9, 0, true, After::None),
			TOG("ih_bow", "Bow Animation", "Use the bow animation for a kill with a bow.", kIH, 0xEF2, 0, true, After::None),
			TOG("ih_realistic", "Realistic Skinning", "Uses the animation mod's own realistic skinning.", kIH, 0xEF0, 0, true, After::None),
			COLUMN,
			HEADER("Dagger and Displays (needs Immersive Equipment Displays)"),
			TOG("ih_displays", "Carcass and Gear Displays", "Shows carried carcasses and the dagger on your body.", kIH, 0xD7B, 1, true, After::IHDisplays),
			TOG("ih_lastdagger", "Use Last Dagger", "Uses the dagger you last had equipped for the animations.", kIH, 0xD7C, 1, true, After::IHUseLastDagger),
			CMB("ih_dagger_pos", "Dagger Position", "Where the dagger is worn when it is shown.", kIH, 0xEAE, 0, kDagger, kDaggerValues, After::IHDaggerPos, true),
			TOG("ih_require_dagger", "Require a Dagger", "You need a dagger in your inventory to skin.", kIH, 0xEAF, 0, true, After::None),
			TOG("ih_pickup", "Pick Up Carcasses", "Carry a carcass on your body after picking it up.", kIH, 0xD7D, 0, true, After::IHPickup),
			TOG("ih_torch", "Place Torch", "Puts a held torch down for the animations.", kIH, 0xD81, 0, true, After::None),
			HEADER("Support"),
			TOG("ih_souls", "Souls", "Collect souls with the animations.", kIH, 0xD82, 0, true, After::None),
			TOG("ih_quickloot", "QuickLoot", "Use the animations with QuickLoot.", kIH, 0xD83, 0, true, After::IHQuickLoot),
		};

		constexpr Def kControls[] = {
			HEADER("Dynamic Activation Key"),
			KEY("dak_key", "Alternative Activation Key", "The key Dynamic Activation Key uses (hold it while you look at an animal to use the hunting actions). This is DAK's own setting, shared with Skills of the Wild and other mods. It applies the next time you load a game.", kDAK, 0x804, 0, false),
			COLUMN,
			HEADER("Hunting Hotkeys"),
			KEY("key_pickup", "Pick Up Carcass", "Picks up the carcass you are looking at, as the pick-up perk does.", nullptr, 0, 1, true),
			KEY("key_cycle_looting", "Next Looting Mode", "Cycles When Looting a Kill through its four modes.", nullptr, 0, 2, true),
			KEY("key_skinned_look", "Toggle Skinned Look", "Switches the skinned animal look on or off.", nullptr, 0, 3, true),
			KEY("key_xp", "Show Hunting Experience", "Shows your experience and how long a skinning takes right now.", nullptr, 0, 4, true),
		};
	}

	const Page kPages[kPageTotal] = {
		{ "General", kGeneral, static_cast<int>(std::size(kGeneral)) },
		{ "Rewards", kRewards, static_cast<int>(std::size(kRewards)) },
		{ "Skinning Time", kTimes, static_cast<int>(std::size(kTimes)) },
		{ "Animations", kAnimations, static_cast<int>(std::size(kAnimations)) },
		{ "Controls", kControls, static_cast<int>(std::size(kControls)) },
	};


	bool IsHeader(const Def& a_d) { return a_d.id == nullptr && a_d.label && a_d.label[0]; }
	bool IsColumn(const Def& a_d) { return a_d.id == nullptr && !(a_d.label && a_d.label[0]); }

	namespace
	{
		std::map<std::string, float> file;  // settings.ini as read, plus what has been written since
		std::mutex                   mu;
		std::atomic<bool>            applyOnLoad{ true };
		bool                         dirty = false;
		float                        timer = -1.0f;
		bool                         fileRead = false;

		RE::TESGlobal* Glob(const Def& a_d)
		{
			if (!a_d.plugin) {
				return nullptr;
			}
			auto* dh = RE::TESDataHandler::GetSingleton();
			return dh ? dh->LookupForm<RE::TESGlobal>(a_d.local, a_d.plugin) : nullptr;
		}

		// Callers hold mu.
		void ReadFile()
		{
			if (fileRead) {
				return;
			}
			fileRead = true;
			std::ifstream in(kFile);
			std::string   line;
			while (std::getline(in, line)) {
				if (line.empty() || line[0] == '#' || line[0] == ';') {
					continue;
				}
				const auto eq = line.find('=');
				if (eq == std::string::npos) {
					continue;
				}
				std::string key = line.substr(0, eq);
				std::string val = line.substr(eq + 1);
				while (!val.empty() && (val.back() == '\r' || val.back() == ' ')) {
					val.pop_back();
				}
				if (key == "apply_on_load") {
					applyOnLoad = val != "0";
				} else {
					file[key] = std::strtof(val.c_str(), nullptr);
				}
			}
		}

		void WriteFile()
		{
			std::error_code ec;
			std::filesystem::create_directories(std::filesystem::path(kFile).parent_path(), ec);
			std::ofstream out(kFile, std::ios::trunc);
			if (!out) {
				SKSE::log::warn("Cannot write {}", kFile);
				return;
			}
			out << "# Simple Hunting Overhaul 2026 settings. Edited by the SKSE Menu Framework pages; applied to the game on every load\n";
			out << "# while apply_on_load=1. Delete a line to let the game's current value take its place on the next load.\n";
			out << "apply_on_load=" << (applyOnLoad ? 1 : 0) << "\n";
			for (const auto& [k, v] : file) {
				char buf[48];
				std::snprintf(buf, sizeof(buf), "%g", static_cast<double>(v));
				out << k << "=" << buf << "\n";
			}
			SKSE::log::info("Wrote {} ({} settings)", kFile, file.size());
		}

		void Touch()
		{
			dirty = true;
			timer = kWriteDelay;
		}

		float Clamp(const Def& a_d, float a_v)
		{
			if (a_d.kind == Kind::Slider) {
				if (a_d.step > 0.0f) {
					a_v = a_d.min + std::round((a_v - a_d.min) / a_d.step) * a_d.step;
				}
				return std::clamp(a_v, a_d.min, a_d.max);
			}
			return a_v;
		}

		bool StartModOn()
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			auto* dh = RE::TESDataHandler::GetSingleton();
			auto* perk = dh ? dh->LookupForm<RE::BGSPerk>(kIHPerk, kIH) : nullptr;
			return player && perk && player->HasPerk(perk);
		}

		// The current game value of a setting (no locking: used while mu is held).
		float Current(const Def& a_d)
		{
			if (a_d.after == After::IHStartMod) {
				return StartModOn() ? 1.0f : 0.0f;
			}
			if (!a_d.plugin) {
				const auto it = file.find(a_d.id);
				return it != file.end() ? it->second : a_d.def;
			}
			auto* g = Glob(a_d);
			return g ? g->value : a_d.def;
		}

		// Writes one setting to the game (not to the file). Returns true when the value changed.
		bool Apply(const Def& a_d, float a_v)
		{
			const float before = Current(a_d);
			if (a_d.after == After::IHStartMod) {
				if ((before > 0.5f) != (a_v > 0.5f)) {
					Actions::Run(a_d.after, a_v);
					return true;
				}
				return false;
			}
			if (auto* g = Glob(a_d)) {
				g->value = a_v;
			}
			if (std::abs(before - a_v) > 1e-6f) {
				Actions::Run(a_d.after, a_v);
				return true;
			}
			return false;
		}
	}

	bool Loaded(const char* a_plugin)
	{
		auto* dh = RE::TESDataHandler::GetSingleton();
		return a_plugin && dh && dh->LookupModByName(a_plugin);
	}

	const Def* Find(std::string_view a_id)
	{
		for (const auto& page : kPages) {
			for (int i = 0; i < page.count; ++i) {
				if (page.defs[i].id && a_id == page.defs[i].id) {
					return &page.defs[i];
				}
			}
		}
		return nullptr;
	}

	bool Available(const Def& a_d)
	{
		if (!a_d.plugin) {
			return true;  // a hotkey kept in the file
		}
		if (a_d.after == After::IHStartMod) {
			return Loaded(kIH);
		}
		return Glob(a_d) != nullptr;
	}

	float Get(const Def& a_d)
	{
		std::lock_guard lock(mu);
		ReadFile();
		return Current(a_d);
	}

	void Set(const Def& a_d, float a_value)
	{
		a_value = Clamp(a_d, a_value);
		std::lock_guard lock(mu);
		ReadFile();
		if (a_d.plugin) {
			Apply(a_d, a_value);
		}
		if (a_d.profile && a_d.id) {
			file[a_d.id] = a_value;
			Touch();
		}
	}

	int  KeyOf(const Def& a_d) { return static_cast<int>(Get(a_d)); }
	void SetKey(const Def& a_d, int a_key) { Set(a_d, static_cast<float>(a_key)); }

	bool ApplyOnLoad() { return applyOnLoad.load(); }

	void SetApplyOnLoad(bool a_on)
	{
		std::lock_guard lock(mu);
		ReadFile();
		applyOnLoad = a_on;
		Touch();
	}

	namespace
	{
		// Immersive Hunting Animations' own menu script puts its saved values on the globals about 3 seconds after every load. Applying the file
		// once more a little later makes this plugin's values the ones that stay, whichever menu of that mod is installed.
		void ApplyLate()
		{
			std::lock_guard lock(mu);
			if (!applyOnLoad) {
				return;
			}
			int applied = 0;
			for (const auto& page : kPages) {
				for (int i = 0; i < page.count; ++i) {
					const Def& d = page.defs[i];
					if (!d.id || !d.profile || !d.plugin || !Available(d)) {
						continue;
					}
					const auto it = file.find(d.id);
					if (it != file.end() && Apply(d, Clamp(d, it->second))) {
						++applied;
					}
				}
			}
			SKSE::log::info("Late apply: {} setting(s) put back after other mods' load scripts", applied);
		}

		std::atomic<int> lateGeneration{ 0 };

		void ScheduleLateApply()
		{
			const int mine = ++lateGeneration;
			std::thread([mine]() {
				std::this_thread::sleep_for(std::chrono::seconds(8));
				if (lateGeneration.load() == mine) {
					SKSE::GetTaskInterface()->AddTask([]() { ApplyLate(); });
				}
			}).detach();
		}
	}

	void OnGameLoaded()
	{
		std::lock_guard lock(mu);
		ReadFile();
		int applied = 0, adopted = 0;
		for (const auto& page : kPages) {
			for (int i = 0; i < page.count; ++i) {
				const Def& d = page.defs[i];
				if (!d.id || !d.profile || !Available(d)) {
					continue;
				}
				const auto it = file.find(d.id);
				if (it == file.end()) {
					file[d.id] = Current(d);  // first time: keep what the game has, so nothing resets
					++adopted;
					dirty = true;
				} else if (applyOnLoad && d.plugin) {
					if (Apply(d, Clamp(d, it->second))) {
						++applied;
					}
				}
			}
		}
		if (dirty) {
			timer = kWriteDelay;
		}
		SKSE::log::info("Game loaded: {} setting(s) applied from settings.ini, {} adopted from the game", applied, adopted);
		if (applyOnLoad) {
			ScheduleLateApply();
		}
	}

	void SaveNow()
	{
		std::lock_guard lock(mu);
		ReadFile();
		for (const auto& page : kPages) {
			for (int i = 0; i < page.count; ++i) {
				const Def& d = page.defs[i];
				if (d.id && d.profile && d.plugin && Available(d)) {
					file[d.id] = Current(d);
				}
			}
		}
		WriteFile();
		dirty = false;
		timer = -1.0f;
	}

	void LoadNow()
	{
		std::lock_guard lock(mu);
		fileRead = false;
		file.clear();
		ReadFile();
		for (const auto& page : kPages) {
			for (int i = 0; i < page.count; ++i) {
				const Def& d = page.defs[i];
				if (d.id && d.profile && d.plugin && Available(d)) {
					const auto it = file.find(d.id);
					if (it != file.end()) {
						Apply(d, Clamp(d, it->second));
					}
				}
			}
		}
	}

	void ResetDefaults()
	{
		std::lock_guard lock(mu);
		ReadFile();
		for (const auto& page : kPages) {
			for (int i = 0; i < page.count; ++i) {
				const Def& d = page.defs[i];
				if (d.id && d.profile && Available(d)) {
					if (d.plugin) {
						Apply(d, d.def);
					}
					file[d.id] = d.def;
				}
			}
		}
		Touch();
	}

	void Tick(float a_dt)
	{
		if (timer < 0.0f) {
			return;
		}
		timer -= a_dt;
		if (timer <= 0.0f) {
			std::lock_guard lock(mu);
			timer = -1.0f;
			if (dirty) {
				WriteFile();
				dirty = false;
			}
		}
	}

	const char* FilePath() { return kFile; }
}
