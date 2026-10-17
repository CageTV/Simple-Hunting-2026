Scriptname SHO_PickUpCarCass extends Quest  

Keyword Property _DamagedCarcass Auto
Actor Property PlayerRef Auto

VoiceType Property CRWolfVoice Auto
VoiceType Property CRDeerVoice Auto
VoiceType Property CRFoxVoice Auto
VoiceType Property CRGoatVoice Auto
VoiceType Property CRHareVoice Auto
VoiceType Property CRMudcrabVoice Auto
VoiceType Property CRDogVoice Auto
VoiceType Property CRChickenVoice Auto
VoiceType Property CRSkeeverVoice Auto
VoiceType Property CRSlaughterfishVoice Auto

Formlist property _VanillaPelts Auto
Formlist property _Brumapelts Auto

MiscObject Property _Carcass_Chicken Auto
MiscObject Property _Carcass_Deer Auto
MiscObject Property _Carcass_DeerVale Auto
MiscObject Property _Carcass_Dog Auto
MiscObject Property _Carcass_ElkFemale Auto
MiscObject Property _Carcass_ElkMale Auto
MiscObject Property _Carcass_Fox Auto
MiscObject Property _Carcass_FoxSnow Auto
MiscObject Property _Carcass_Goat Auto
MiscObject Property _Carcass_Hare Auto
MiscObject Property _Carcass_Mudcrab01 Auto
MiscObject Property _Carcass_Mudcrab02 Auto
MiscObject Property _Carcass_Mudcrab03 Auto
MiscObject Property _Carcass_Skeever Auto
MiscObject Property _Carcass_Slaughterfish Auto
MiscObject Property _Carcass_Wolf Auto
MiscObject Property _Carcass_WolfIce Auto
MiscObject Property _MeatTracker Auto

spell property AbWolfIce auto
spell property AbAnimalResistFrost auto
Race property deerrace auto
Race Property DLC1DeerGlowRace Auto
armor property skinmudcrablarge auto
armor property skinmudcrabmedium auto
Idle property boundstandingcut auto

keyword property ArmorMaterialLeather Auto


; Simple Hunting Overhaul 2026: the voice type of the animal is read once instead of up to a dozen times, a missing crosshair target no longer
; raises errors, and the animation mod's trigger global is looked up once instead of on every pick-up.
GlobalVariable IHAnimTrigger

GlobalVariable Function GetIHAnimTrigger()
	If !IHAnimTrigger && Game.IsPluginInstalled("ImmersiveHunting.esp")
		IHAnimTrigger = Game.GetFormFromFile(0x0000E88, "ImmersiveHunting.esp") as GlobalVariable
	EndIf
	return IHAnimTrigger
EndFunction

Function PickUpCarcass()
	Actor TheActor = Game.GetCurrentCrosshairRef() as Actor
	If !TheActor
		return
	EndIf
	VoiceType vt = TheActor.GetVoiceType()

	If TheActor.WornHasKeyword(_DamagedCarcass)  || TheActor.GetItemCount(_MeatTracker) > 0 || TheActor.WornHasKeyword(ArmorMaterialLeather) == true
		Debug.notification("This carcass is too damaged...")
	elseif PlayerRef.IsOnMount()
		Debug.notification("You can't reach the carcass from your horse...")
	Elseif TheActor.GetItemCount(_VanillaPelts) > 0 || TheActor.GetItemCount(_BrumaPelts) > 0 
		TheActor.AddItem(_MeatTRacker, 1)

		if vt == CrWolfVoice 
			PlayTheAnimation(902)
			TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)
			
			If TheActor.HasSpell(AbWolfIce) == 1
				PlayerRef.AddItem(_Carcass_WolfIce, 1)
			else
              	PlayerRef.AddItem(_Carcass_Wolf, 1)
			endif
		elseif vt == CRDeerVoice 
			PlayTheAnimation(902)
			TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)

			If TheActor.GetRace() == DeerRace
				PlayerRef.AddItem(_Carcass_Deer, 1)
			elseif TheActor.GetRace() == DLC1DeerGlowRace
				PlayerRef.AddItem(_Carcass_DeerVale, 1)
			elseif TheActor.GetLeveledActorBase().GetSex() == 0
				PlayerRef.AddItem(_Carcass_ElkMale, 1)
			else
				PlayerRef.AddItem(_Carcass_ElkFemale, 1)
			endif
		elseif vt == CRFoxVoice 
			PlayTheAnimation(902)
			TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)

			If TheActor.HasSpell(AbAnimalResistFrost) == 1
				PlayerRef.AddItem(_Carcass_FoxSnow, 1)
			else
				PlayerRef.AddItem(_Carcass_Fox, 1)
			endif
		elseif vt == CRGoatVoice 
			PlayTheAnimation(905)
			TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)
            PlayerRef.AddItem(_Carcass_Goat, 1)
		elseif vt == CRSkeeverVoice
			PlayTheAnimation(902)
			TheActor.AddItem(_MeatTRacker, 1)
			TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)
			PlayerRef.AddItem(_Carcass_Skeever, 1)
		elseif vt == CRHareVoice 
			PlayTheAnimation(901)
			TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)
            PlayerRef.AddItem(_Carcass_Hare, 1)
		else
			Debug.notification("You can't carry this carcass...")
		endIf
	elseif vt == CRDogVoice 
		PlayTheAnimation(902)
		TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)
		TheActor.AddItem(_MeatTRacker, 1)
        PlayerRef.AddItem(_Carcass_Dog, 1)
	elseif vt == CRMudcrabVoice
		TheActor.AddItem(_MeatTRacker, 1)
		PlayTheAnimation(906)
		TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)

		If TheActor.IsEquipped(skinmudcrablarge)
			PlayerRef.AddItem(_Carcass_Mudcrab03, 1)
		elseif TheActor.IsEquipped(skinmudcrabmedium)
			PlayerRef.AddItem(_Carcass_Mudcrab02, 1)
		else
			PlayerRef.AddItem(_Carcass_Mudcrab01, 1)
		endif
	elseif vt == CRChickenVoice
		PlayTheAnimation(901)
		TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)
		TheActor.AddItem(_MeatTRacker, 1)
		PlayerRef.AddItem(_Carcass_Chicken, 1)
	elseif vt == CRSlaughterfishVoice
		PlayTheAnimation(908)
		TheActor.SetCriticalStage(TheActor.CritStage_DisintegrateEnd)
		TheActor.AddItem(_MeatTRacker, 1)
		PlayerRef.AddItem(_Carcass_Slaughterfish, 1)
	else
		Debug.notification("You can't carry this carcass...")
	endif
endfunction


Function PlayTheAnimation(Int iTypeofAnim = 0)

	If PlayerRef.IsWeaponDrawn() 
		PlayerRef.SheatheWeapon()
		Utility.Wait(1.5)
	EndIf

	If PlayerRef.IsSwimming()
		;no animation

	elseif Game.GetCameraState() == 0 
		;Player Is In First Person
		playerref.playidle(boundstandingcut)
		Utility.Wait(1.3)
	else
		GlobalVariable IH_AnimTrigger = GetIHAnimTrigger()
		If IH_AnimTrigger
			; Plays the pick up animations from ImmersiveHunting
			IH_AnimTrigger.SetValue(iTypeofAnim)
			Debug.SendAnimationEvent(PlayerRef, "IdlePickUp_Ground")
			IH_AnimTrigger.SetValue(0)
		Else
			Debug.SendanimationEvent(PlayerRef, "idleactivatepickuplow")
			Utility.Wait(1.5)
			Debug.SendanimationEvent(PlayerRef, "offsetstop")
		endif			
	endif
	
endfunction
