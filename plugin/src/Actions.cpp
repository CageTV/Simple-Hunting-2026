/*
 * Simple Hunting Overhaul 2026
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Actions.h"

namespace Actions
{
	namespace
	{
		constexpr const char* kIH = "ImmersiveHunting.esp";
		constexpr const char* kSHO = "Simple Hunting Overhaul.esp";
		constexpr unsigned    kIHQuest = 0x000800;   // ImmersiveHuntingQuest, carries ImmersiveHuntingFunctions
		constexpr unsigned    kSHOQuest = 0x000E7F;  // SimpleHuntingOverhaul, carries SHO_PickUpCarCass
		constexpr const char* kIHScript = "ImmersiveHuntingFunctions";
		constexpr const char* kSHOScript = "SHO_PickUpCarCass";

		// Calls a method of a script on a quest, on the game thread.
		template <class... Args>
		void CallQuest(const char* a_plugin, unsigned a_quest, const char* a_script, const char* a_method, Args... a_args)
		{
			SKSE::GetTaskInterface()->AddTask([=]() mutable {
				auto* dh = RE::TESDataHandler::GetSingleton();
				auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				auto* quest = dh ? dh->LookupForm<RE::TESQuest>(a_quest, a_plugin) : nullptr;
				if (!vm || !quest) {
					SKSE::log::warn("Cannot call {}.{}: {} is not loaded", a_script, a_method, a_plugin);
					return;
				}
				const auto handle = vm->GetObjectHandlePolicy()->GetHandleForObject(RE::TESQuest::FORMTYPE, quest);
				RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
				const bool ok = vm->DispatchMethodCall2(handle, a_script, a_method, RE::MakeFunctionArguments(std::move(a_args)...), callback);
				SKSE::log::info("{}.{}() -> {}", a_script, a_method, ok ? "dispatched" : "FAILED (is the script on the quest?)");
			});
		}

		// Gives or takes away the perk that switches Immersive Hunting Animations on (IH_PK_ImmersiveHunting), without its script. StartMod does the
		// same through Papyrus, but when the animation mod's main script does not load (its types do not link) the call fails and the perk would
		// stay on the player, its carcass handling running whatever the setting says.
		void SetIHPerk(bool a_on)
		{
			SKSE::GetTaskInterface()->AddTask([a_on]() {
				auto* player = RE::PlayerCharacter::GetSingleton();
				auto* dh = RE::TESDataHandler::GetSingleton();
				auto* perk = dh ? dh->LookupForm<RE::BGSPerk>(0x000D42, kIH) : nullptr;
				if (!player || !perk) {
					return;
				}
				const bool has = player->HasPerk(perk);
				if (a_on && !has) {
					player->AddPerk(perk);
					SKSE::log::info("Immersive Hunting Animations: perk given directly");
				} else if (!a_on && has) {
					player->RemovePerk(perk);
					SKSE::log::info("Immersive Hunting Animations: perk taken away directly");
				}
			});
		}

		float GlobalValue(const char* a_id, float a_default)
		{
			const auto* d = Settings::Find(a_id);
			return d ? Settings::Get(*d) : a_default;
		}

		void Notify(const std::string& a_text)
		{
			SKSE::GetTaskInterface()->AddTask([a_text]() {
#if __has_include(<RE/S/SendHUDMessage.h>)
				RE::SendHUDMessage::ShowHUDMessage(a_text.c_str());
#else
				RE::DebugNotification(a_text.c_str());
#endif
			});
		}

		constexpr const char* kLooting[] = { "No time passes, no fade", "Time passes and the screen fades", "Only the screen fades", "Only time passes" };

		void CycleLooting()
		{
			if (const auto* d = Settings::Find("looting_mode"); d && Settings::Available(*d)) {
				const int next = (static_cast<int>(Settings::Get(*d)) + 1) % 4;
				Settings::Set(*d, static_cast<float>(next));
				Notify(std::string("When looting a kill: ") + kLooting[next]);
			}
		}

		void ToggleSkinnedLook()
		{
			if (const auto* d = Settings::Find("skinned_look"); d && Settings::Available(*d)) {
				const bool on = Settings::Get(*d) < 0.5f;
				Settings::Set(*d, on ? 1.0f : 0.0f);
				Notify(on ? "Skinned animal look: on" : "Skinned animal look: off");
			}
		}

		bool InputBlocked()
		{
			auto* ui = RE::UI::GetSingleton();
			if (!ui || ui->GameIsPaused() || ui->IsMenuOpen(RE::Console::MENU_NAME)) {
				return true;
			}
			auto* controls = RE::ControlMap::GetSingleton();
			return controls && controls->textEntryCount > 0;
		}

		class Sink : public RE::BSTEventSink<RE::InputEvent*>
		{
		public:
			RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override
			{
				if (!a_event) {
					return RE::BSEventNotifyControl::kContinue;
				}
				for (auto* e = *a_event; e; e = e->next) {
					const auto* button = e->AsButtonEvent();
					if (!button || !button->IsDown() || button->GetDevice() != RE::INPUT_DEVICE::kKeyboard) {
						continue;
					}
					const int code = static_cast<int>(button->GetIDCode());
					if (code <= 0 || InputBlocked()) {
						continue;
					}
					for (const auto& page : Settings::kPages) {
						for (int i = 0; i < page.count; ++i) {
							const auto& d = page.defs[i];
							if (d.kind != Settings::Kind::Key || d.plugin != nullptr || Settings::KeyOf(d) != code) {
								continue;
							}
							switch (d.slot) {
							case kSlotPickUp:
								CallQuest(kSHO, kSHOQuest, kSHOScript, "PickUpCarcass");
								break;
							case kSlotCycleLooting:
								CycleLooting();
								break;
							case kSlotSkinnedLook:
								ToggleSkinnedLook();
								break;
							case kSlotShowXp:
								Notify(XpSummary());
								break;
							default:
								break;
							}
						}
					}
				}
				return RE::BSEventNotifyControl::kContinue;
			}
		};

		Sink sink;
	}

	void Run(Settings::After a_after, float a_value)
	{
		const bool on = a_value > 0.5f;
		switch (a_after) {
		case Settings::After::IHStartMod:
			CallQuest(kIH, kIHQuest, kIHScript, "StartMod", on);
			SetIHPerk(on);
			break;
		case Settings::After::IHPickup:
			CallQuest(kIH, kIHQuest, kIHScript, "EnablePickup", on);
			break;
		case Settings::After::IHQuickLoot:
			CallQuest(kIH, kIHQuest, kIHScript, "EnableQuickLootSupport", on);
			break;
		case Settings::After::IHUseLastDagger:
			CallQuest(kIH, kIHQuest, kIHScript, "UpdateDagger", on);
			break;
		case Settings::After::IHDisplays:
			CallQuest(kIH, kIHQuest, kIHScript, "ToggleDisplays", on);
			break;
		case Settings::After::IHDaggerPos:
			// the animation mod re-evaluates the player's displays through Immersive Equipment Displays
			SKSE::GetTaskInterface()->AddTask([]() {
				auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				auto* player = RE::PlayerCharacter::GetSingleton();
				if (vm && player) {
					RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
					vm->DispatchStaticCall("IED", "Evaluate", RE::MakeFunctionArguments(static_cast<RE::Actor*>(player)), callback);
				}
			});
			break;
		case Settings::After::None:
		default:
			break;
		}
	}

	void RegisterInput()
	{
		if (auto* mgr = RE::BSInputDeviceManager::GetSingleton()) {
			mgr->AddEventSink(&sink);
			SKSE::log::info("Hunting hotkeys are listening");
		}
	}

	float Multiplier()
	{
		const float xp = std::clamp(GlobalValue("xp", 1.0f), 0.0f, 100.0f);
		const float floorMult = GlobalValue("xp_floor", 0.2f);
		return std::max((100.0f - xp) / 100.0f, floorMult);
	}

	float HoursFor(float a_baseHours)
	{
		float hours = a_baseHours * Multiplier();
		hours = std::max(hours, GlobalValue("time_min", 0.15f));
		const float maxHours = GlobalValue("time_max", 4.0f);
		if (maxHours > 0.0f) {
			hours = std::min(hours, maxHours);
		}
		return hours;
	}

	std::string XpSummary()
	{
		const float xp = GlobalValue("xp", 1.0f);
		return std::format("Hunting experience {:.0f} / 100: a skinning takes {:.0f}% of its base time.", xp, Multiplier() * 100.0f);
	}
}
