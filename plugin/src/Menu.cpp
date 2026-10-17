/*
 * Simple Hunting Overhaul 2026
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Actions.h"
#include "Hotkeys.h"
#include "Menu.h"
#include "Settings.h"

#include "SKSEMenuFramework.h"

namespace Menu
{
	namespace
	{
		using namespace ImGuiMCP;
		using Settings::Def;
		using Settings::Kind;

		bool registered = false;

		void Tip(const Def& a_d)
		{
			if (a_d.tip && a_d.tip[0] && IsItemHovered()) {
				BeginTooltip();
				PushTextWrapPos(420.0f);
				TextUnformatted(a_d.tip);
				PopTextWrapPos();
				EndTooltip();
			}
		}

		void DrawDef(const Def& a_d, int a_id)
		{
			if (Settings::IsHeader(a_d)) {
				Spacing();
				SeparatorText(a_d.label);
				return;
			}
			if (Settings::IsColumn(a_d)) {
				Spacing();
				Separator();
				return;
			}
			PushID(a_id);
			const bool available = Settings::Available(a_d);
			if (!available) {
				BeginDisabled();
			}
			switch (a_d.kind) {
			case Kind::Toggle:
				{
					bool on = Settings::Get(a_d) > 0.5f;
					if (Checkbox(a_d.label, &on)) {
						Settings::Set(a_d, on ? 1.0f : 0.0f);
					}
					Tip(a_d);
				}
				break;
			case Kind::Slider:
				{
					const float shown = a_d.shown > 0.0f ? a_d.shown : 1.0f;
					float       v = Settings::Get(a_d) * shown;
					if (SliderFloat(a_d.label, &v, a_d.min * shown, a_d.max * shown, a_d.fmt)) {
						Settings::Set(a_d, v / shown);
					}
					Tip(a_d);
				}
				break;
			case Kind::Combo:
				{
					const float value = Settings::Get(a_d);
					int         index = 0;
					for (int i = 0; i < a_d.optionCount; ++i) {
						const float stored = a_d.optionValues ? a_d.optionValues[i] : static_cast<float>(i);
						if (std::abs(stored - value) < 0.5f) {
							index = i;
						}
					}
					if (Combo(a_d.label, &index, a_d.options, a_d.optionCount)) {
						Settings::Set(a_d, a_d.optionValues ? a_d.optionValues[index] : static_cast<float>(index));
					}
					Tip(a_d);
				}
				break;
			case Kind::Key:
				{
					int picked = 0;
					if (Hotkeys::CapturingSlot() == a_d.slot && Hotkeys::TakeCaptured(a_d.slot, picked)) {
						Settings::SetKey(a_d, picked);
					}
					if (Hotkeys::CapturingSlot() == a_d.slot) {
						if (Button("Press a key...  (Esc cancels, Delete clears)")) {
							Hotkeys::CancelCapture();
						}
					} else {
						const std::string text = std::string(Hotkeys::KeyName(Settings::KeyOf(a_d))) + "##key";
						if (Button(text.c_str(), ImVec2(180.0f, 0.0f))) {
							Hotkeys::BeginCapture(a_d.slot);
						}
					}
					SameLine();
					Text("%s", a_d.label);
					Tip(a_d);
				}
				break;
			}
			if (!available) {
				EndDisabled();
				if (a_d.plugin && IsItemHovered()) {
					BeginTooltip();
					Text("%s is not installed.", a_d.after == Settings::After::IHStartMod || std::string_view(a_d.plugin) == "ImmersiveHunting.esp" ? "Immersive Hunting Animations" : a_d.plugin);
					EndTooltip();
				}
			}
			PopID();
		}

		void DrawPage(int a_page)
		{
			if (!Settings::Loaded("Simple Hunting Overhaul.esp")) {
				TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "Simple Hunting Overhaul.esp is not loaded.");
				return;
			}
			const auto& page = Settings::kPages[a_page];
			for (int i = 0; i < page.count; ++i) {
				DrawDef(page.defs[i], i);
			}
			if (std::string_view(page.title) == "Controls") {
				Spacing();
				TextWrapped("Click a key button, then press the key. Escape cancels, Delete clears it. The four hunting hotkeys work in the open game, not in menus.");
			}
			if (std::string_view(page.title) == "Animations" && !Settings::Loaded("ImmersiveHunting.esp")) {
				Spacing();
				TextWrapped("Immersive Hunting Animations is not installed, so these options do nothing. It is the animation mod made for Simple Hunting Overhaul.");
			}
		}

		void Status(const char* a_label, const char* a_plugin, bool a_required)
		{
			const bool ok = Settings::Loaded(a_plugin);
			if (ok) {
				TextColored(ImVec4(0.65f, 0.9f, 0.5f, 1.0f), "%s: installed", a_label);
			} else if (a_required) {
				TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "%s: NOT installed (required)", a_label);
			} else {
				TextDisabled("%s: not installed (optional)", a_label);
			}
		}

		void __stdcall RenderOverview()
		{
			if (!Settings::Loaded("Simple Hunting Overhaul.esp")) {
				TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "Simple Hunting Overhaul.esp is not loaded.");
				return;
			}
			TextColored(ImVec4(0.65f, 0.9f, 0.5f, 1.0f), "Simple Hunting Overhaul 2026 is active.");
			Spacing();
			TextWrapped("All of Simple Hunting Overhaul's settings are on the pages of this section, together with the options of Immersive Hunting Animations and "
						"the hunting hotkeys. It replaces the \"Simple Hunting Overhaul - MCM\" and \"Simple Hunting Overhaul MCM Helper\" menus: turn those two off.");
			Spacing();
			SeparatorText("Installed");
			Status("Simple Hunting Overhaul", "Simple Hunting Overhaul.esp", true);
			Status("Immersive Hunting Animations", "ImmersiveHunting.esp", false);
			Status("Dynamic Activation Key", "Dynamic Activation Key.esp", true);
			Status("Skills of the Wild", "SkillsOfTheWild.esp", false);

			Spacing();
			SeparatorText("Experience");
			TextWrapped("%s", Actions::XpSummary().c_str());
			const auto* xp = Settings::Find("xp");
			if (xp) {
				ProgressBar(Settings::Get(*xp) / 100.0f, ImVec2(-1.0f, 0.0f));
			}

			Spacing();
			SeparatorText("How long a skinning takes now");
			if (BeginTable("times", 3)) {
				TableSetupColumn("Animal");
				TableSetupColumn("Base time");
				TableSetupColumn("Now");
				TableNextColumn();
				TextDisabled("Animal");
				TableNextColumn();
				TextDisabled("Base time");
				TableNextColumn();
				TextDisabled("Now");
				for (const auto* page = &Settings::kPages[2]; page; page = nullptr) {
					for (int i = 0; i < page->count; ++i) {
						const Def& d = page->defs[i];
						if (!d.id || std::string_view(d.id).rfind("time_", 0) != 0 || std::string_view(d.id) == "time_min" || std::string_view(d.id) == "time_max") {
							continue;
						}
						const float base = Settings::Get(d);
						TableNextColumn();
						Text("%s", d.label);
						TableNextColumn();
						Text("%.2f h", base);
						TableNextColumn();
						Text("%.2f h", Actions::HoursFor(base));
					}
				}
				EndTable();
			}
			Spacing();
			TextDisabled("Simple Hunting Overhaul by JaySerpa, Immersive Hunting Animations by Elsawirr and GiraPomba, Dynamic Activation Key by its author. Settings pages by CageTV.");
		}

		void __stdcall RenderGeneral() { DrawPage(0); }
		void __stdcall RenderRewards() { DrawPage(1); }
		void __stdcall RenderTimes() { DrawPage(2); }
		void __stdcall RenderAnimations() { DrawPage(3); }
		void __stdcall RenderControls() { DrawPage(4); }

		void __stdcall RenderProfile()
		{
			static int   confirmReset = 0;
			static float confirmTimer = 0.0f;
			confirmTimer -= GetIO()->DeltaTime;
			if (confirmTimer <= 0.0f) {
				confirmReset = 0;
			}
			SeparatorText("Settings file");
			bool apply = Settings::ApplyOnLoad();
			if (Checkbox("Apply the saved settings every time a game loads", &apply)) {
				Settings::SetApplyOnLoad(apply);
			}
			if (IsItemHovered()) {
				BeginTooltip();
				PushTextWrapPos(420.0f);
				TextUnformatted("On: your settings follow you to new characters and old saves. Off: each save keeps the values it has, and the file is only written.");
				PopTextWrapPos();
				EndTooltip();
			}
			TextDisabled("%s", Settings::FilePath());
			Spacing();
			if (Button("Save the current values now")) {
				Settings::SaveNow();
			}
			SameLine();
			if (Button("Load the file now")) {
				Settings::LoadNow();
			}
			Spacing();
			SeparatorText("Defaults");
			if (confirmReset == 1) {
				TextWrapped("Reset every setting on the pages to the value the mods ship with? Your hunting experience and the Dynamic Activation Key are left alone.");
				if (Button("Yes, reset")) {
					Settings::ResetDefaults();
					confirmReset = 0;
				}
				SameLine();
				if (Button("Cancel")) {
					confirmReset = 0;
				}
			} else if (Button("Reset all settings to default")) {
				confirmReset = 1;
				confirmTimer = 10.0f;
			}
		}

		// Drawn every frame whether or not the menu is open: writes the settings file a moment after the last change.
		void __stdcall Frame()
		{
			Settings::Tick(GetIO()->DeltaTime);
		}
	}

	bool Registered() { return registered; }

	void Register()
	{
		if (!SKSEMenuFramework::IsInstalled()) {
			SKSE::log::warn("SKSE Menu Framework is not installed: the Simple Hunting Overhaul settings pages are not available");
			return;
		}
		SKSEMenuFramework::SetSection("Simple Hunting Overhaul");
		SKSEMenuFramework::AddSectionItem("Overview", RenderOverview);
		SKSEMenuFramework::AddSectionItem("General", RenderGeneral);
		SKSEMenuFramework::AddSectionItem("Rewards", RenderRewards);
		SKSEMenuFramework::AddSectionItem("Skinning Time", RenderTimes);
		SKSEMenuFramework::AddSectionItem("Animations", RenderAnimations);
		SKSEMenuFramework::AddSectionItem("Controls", RenderControls);
		SKSEMenuFramework::AddSectionItem("Profile", RenderProfile);
		SKSEMenuFramework::AddHudElement(Frame);
		registered = true;
		SKSE::log::info("Registered with SKSE Menu Framework {}", SKSEMenuFramework::GetMenuFrameworkVersion());
	}
}
