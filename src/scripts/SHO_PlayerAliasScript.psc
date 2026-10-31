Scriptname SHO_PlayerAliasScript extends ReferenceAlias  

FormList Property _Skins Auto
FormList Property _Traits Auto
FormList Property _SkinTime Auto
FormList Property _RestrictPelt Auto
MiscObject property _FListFiller Auto

Actor Property PlayerRef Auto
Formlist property _VanillaPelts Auto
Formlist property _BrumaPelts Auto
Formlist property _CustomPelts Auto

Perk Property _PickUpPerkForPickingUp Auto
Sound Property _SkinSound Auto

GlobalVariable Property _HuntingXP Auto
GlobalVariable Property GameHour Auto
GlobalVariable Property _FadeTimePass Auto
GlobalVariable Property _RemovePelts Auto

GlobalVariable Property _MaxSkinningTime Auto
GlobalVariable Property _MinSkinningTime Auto
GlobalVariable Property _MinExperienceMult Auto

GlobalVariable Property SHO_EnableDebugMessages Auto

GlobalVariable Property SHO_EnablePlayerHorseSkinning Auto
Faction Property PlayerHorseFaction Auto;"Player Horse Faction" [FACT:00068D78]
VoiceType Property CrGiantVoice Auto

Formlist property SHO_Blacklist Auto

; Added by Simple Hunterborn (Monops), which makes Hunterborn's loot follow SHO's hunting experience: the harvest level and the four tier globals its
; leveled lists read. Empty (None) without Simple Hunterborn, and then not used.
GlobalVariable Property _DS_Hunterborn_HarvestLevel Auto
GlobalVariable Property SHB_HarvestingTier1_MNPS Auto
GlobalVariable Property SHB_HarvestingTier2_MNPS Auto
GlobalVariable Property SHB_HarvestingTier3_MNPS Auto
GlobalVariable Property SHB_HarvestingTier4_MNPS Auto
GlobalVariable Property SHB_SkinningsRequired_Tier2_MNPS Auto
GlobalVariable Property SHB_SkinningsRequired_Tier3_MNPS Auto
GlobalVariable Property SHB_SkinningsRequired_Tier4_MNPS Auto

; What each option means:
; _FadeTimePass == 0 means that no time passes and there's no fadeout of the screen
; _FadeTimePass == 1 means both time and fadeout of the screen happen. This is the default.
; _FadeTimePass == 2 means only fadeout of the screen happens
; _FadeTimePass == 3 means only time passes

MiscObject Property _MeatTracker Auto
Formlist property _IngredientsAndMeats Auto

bool debugMessages = false

; Simple Hunting Overhaul 2026: with Hunterborn installed but not Simple Hunterborn, Hunterborn does the hunting (field dressing, skinning, harvesting,
; butchering, time passing), so SHO's own reaction to a pelt or meat taken from a dead animal (the skinned look, time passing, experience) stays off. It would
; otherwise skin the animal a second time, behind Hunterborn's prompt. With Simple Hunterborn SHO runs the hunt and Hunterborn supplies the loot.
bool hunterbornChecked = false
bool hunterbornPresent = false

bool Function HunterbornPresent()
	If !hunterbornChecked
		hunterbornPresent = Game.IsPluginInstalled("Hunterborn.esp") && !Game.IsPluginInstalled("SimpleHunterborn.esp")
		hunterbornChecked = true
	EndIf
	return hunterbornPresent
EndFunction

Event OnInIt()
	AddInventoryEventFilter(_VanillaPelts)
	AddInventoryEventFilter(_Brumapelts)
	AddInventoryEventFilter(_CustomPelts)
	AddInventoryEventFilter(_IngredientsAndMeats)
	PlayerRef.AddPerk(_PickUpPerkForPickingUp)
EndEvent

Event OnItemAdded(Form akBaseItem, Int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)

	If HunterbornPresent()
		Return
	EndIf

	Actor TheActor = akSourceContainer as actor 
	
	debugMessages = SHO_EnableDebugMessages.GetValue()>0
	Form Filler = _FListFiller
 
	If TheActor && TheActor.IsDead()

		ActorBase theBase = TheActor.GetActorBase()
		if SHO_Blacklist.HasForm(theBase)
			DebugMessage( "Target is in the blacklist")
			return
		endif 
		if SHO_EnablePlayerHorseSkinning.GetValue() == 0 && TheActor.GetFactionRank(PlayerHorseFaction)>0
			DebugMessage( "Target is a player horse")
			return
		endif
		if TheActor.GetVoiceType() == crgiantvoice
			DebugMessage( "Target is giant carrying pelts")
			return
		endif
	
	If _IngredientsAndMeats.HasForm(akBaseItem)
		akSourceContainer.AddItem(_MeatTracker, 1)
		;This adds a token invisible item that will let us know the animal has been poached already, which means you cannot pick it up anymore
		;this is also a good way for anyone to add animations for getting meat here. 

		elseif _VanillaPelts.HasForm(akBaseItem) || _BrumaPelts.HasForm(akBaseItem) || _CustomPelts.HasForm(akBaseItem)

			int i =  _Traits.Find(theBase);
			if (i <0) 
				i =  _Traits.Find(TheActor.GetRace());
			endif
			if (i <0) 
				i =  _Traits.Find(TheActor.GetVoiceType());
			endif
			if (i>=0)
				DebugMessage("Trait at index " + i + " " + _Traits.getat(i).GetName())
				
				Form pelt = _RestrictPelt.getat(i)
				
				if (pelt && pelt  != Filler )
					Formlist RestrictPelts = pelt as Formlist
					
					if RestrictPelts
						if !RestrictPelts.HasForm(akBaseItem) 
							return
						endif
					elseif (pelt != akBaseItem )
						return
					endif 
				endif
				
				GlobalCheck(i , TheActor.GetScale())
				If _RemovePelts.GetValue() == 1 
					Armor _skin = (_Skins.getat(i) as Armor)
					if (_skin && _skin  != Filler )
						
						DebugMessage("_Skins at index " + i + " " + _skin.GetName())
						TheActor.UnequipAll() ; Removes all armors equipped in the animal before equipping the skinned armor
						TheActor.EquipItem(_skin, true)
					else
						DebugMessage("_Skins at index " + i + " is None")
					endif
				endif
			endif
		endif
	endif
endEvent

function DebugMessage(string text)
	if debugMessages
		Debug.notification(text)
		Debug.trace(text)
	endif
endfunction

Function GlobalCheck(int creatureIndex, float scale)
	XP()
	
	float fadeMode = _FadeTimePass.GetValue()
	If fadeMode == 0
		;nothing happens
	else
		GlobalVariable cTime = _SkinTime.GetAt(creatureIndex) as GlobalVariable
		
		float timeToPass = 2*scale
		if cTime
			DebugMessage("_SkinTime at index " + creatureIndex + " " + cTime.GetName())
			timeToPass = cTime.GetValue() * scale 
		endif
		
		if fadeMode == 3 
			PassTime( timeToPass) 
		else
			FadeOut(timeToPass)
		endif
	endif

	UpdateHarvestingGlobals()
endfunction

; Simple Hunterborn: the experience tiers (skinnings required for tier 2, 3 and 4) switch Hunterborn's four loot tiers; a tier global at 0 is the open one.
Function UpdateHarvestingGlobals()
	If !SHB_HarvestingTier1_MNPS
		Return
	EndIf
	float currentXP = _HuntingXP.GetValue()
	float tier2 = SHB_SkinningsRequired_Tier2_MNPS.GetValue()
	float tier3 = SHB_SkinningsRequired_Tier3_MNPS.GetValue()
	float tier4 = SHB_SkinningsRequired_Tier4_MNPS.GetValue()
	SHB_HarvestingTier1_MNPS.SetValue(100)
	SHB_HarvestingTier2_MNPS.SetValue(100)
	SHB_HarvestingTier3_MNPS.SetValue(100)
	SHB_HarvestingTier4_MNPS.SetValue(100)
	If currentXP >= tier4 + 1
		SHB_HarvestingTier4_MNPS.SetValue(0)
	ElseIf currentXP >= tier3
		SHB_HarvestingTier3_MNPS.SetValue(0)
	ElseIf currentXP >= tier2
		SHB_HarvestingTier2_MNPS.SetValue(0)
	Else
		SHB_HarvestingTier1_MNPS.SetValue(0)
	EndIf
EndFunction


Function XP()
	If _HuntingXP.Getvalue() < 100
		_HuntingXP.SetValue(_HuntingXP.GetValue() + 1)
	endif
EndFunction


Function PassTime(float timeToPass = 2.0)
	WaitForAnim()
	
	float MinSkinningTime = _MinSkinningTime.GetValue()
	float MaxSkinningTime = _MaxSkinningTime.GetValue()
	float MinExperienceMult = _MinExperienceMult.GetValue()

	float howlong
	int harvestLevel = 0
	float currentXP = _HuntingXP.GetValue()

	float xpMult =  (100.0 - currentXP)/100.0
	
	if xpMult < MinExperienceMult
		xpMult = MinExperienceMult
	endif
	
	howlong = timeToPass * xpMult

	if howlong < MinSkinningTime
		howlong = MinSkinningTime
	endif
	if MaxSkinningTime >0 && howlong > MaxSkinningTime
		howlong = MaxSkinningTime
	endif
	
	If  howlong > 2.5
		Debug.notification("A couple of hours pass while you retrieve the pelt...")
	elseif howlong >= 1.75
		Debug.notification("About Two hours pass while you retrieve the pelt...")
		harvestLevel = 1
	elseif howlong >= 1.25
		Debug.notification("About an hour and half pass while you retrieve the pelt...")
		harvestLevel = 2
	elseif  howlong >= 1
		Debug.notification("Around one hour passes...")
		harvestLevel = 3
	elseif howlong >= 0.8
		Debug.notification("Less than one hour passes...")
		harvestLevel = 4
	elseif   howlong >= 0.4
		Debug.notification("About half an hour passes...")
		harvestLevel = 5
	elseif    howlong >= 0.2
		Debug.notification("About a quarter of an hour passes...")
		harvestLevel = 7
	else
		Debug.notification("A few minutes pass...")
		harvestLevel = 9
	endif
	If _DS_Hunterborn_HarvestLevel && harvestLevel > 0  ; Simple Hunterborn: the quicker the skinning (experience), the better Hunterborn's harvest
		_DS_Hunterborn_HarvestLevel.SetValue(harvestLevel)
	EndIf

	DebugMessage("Passing time = "+ howlong)
	float time = GameHour.getValue()
	GameHour.setValue(time + howlong)
EndFunction

Function FadeOut(float timeToPass = 2.0)
	; Reworked the function a bit to close the menu then fade to black, to better sync with the animation from ImmersiveHunting
	If UI.IsMenuOpen("ContainerMenu")
		UI.Invoke("ContainerMenu", "_root.Menu_mc.onExitMenuRectClick")
	endif
	
	if _FadeTimePass.GetValue() == 1
		PassTime(timeToPass)
	else
		WaitForAnim()
	EndIf
	
	Game.FadeOutGame(false, true, 0.7, 1.6)
	_SkinSound.Play(PlayerRef)
EndFunction

; Simple Hunting Overhaul 2026: the animation mod's globals and perk are looked up once (they used to be looked up, with a plugin check, on
; every skinning), and the loot mode is read once per call.
GlobalVariable IHAnimTrigger
GlobalVariable IHLootMode
Perk IHPerk

Function WaitForAnim()
	If !IHAnimTrigger
		If !Game.IsPluginInstalled("ImmersiveHunting.esp")
			return
		EndIf
		IHAnimTrigger = Game.GetFormFromFile(0x000E88, "ImmersiveHunting.esp") as GlobalVariable
		IHLootMode = Game.GetFormFromFile(0x000EA0, "ImmersiveHunting.esp") as GlobalVariable
		IHPerk = Game.GetFormFromFile(0x000D42, "ImmersiveHunting.esp") as Perk
	EndIf
	If !IHAnimTrigger || !IHLootMode
		return
	EndIf

	; Waits for the skinning animation to end before triggering fade to black or pass time
	If IHLootMode.GetValue() == 0
		If IHPerk && PlayerRef.HasPerk(IHPerk) && IHAnimTrigger.GetValue() >= 10
			Utility.Wait(1.0)
		EndIf

		While IHAnimTrigger.GetValue() > 11
			Utility.Wait(0.1)
		EndWhile
	EndIf
EndFunction
