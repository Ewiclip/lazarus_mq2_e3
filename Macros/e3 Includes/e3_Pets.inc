|--------------------------------------------------------------------------|
|- e3_Macro_Pets.inc v3.0 ( originally designed to work with e3.mac )	    -|
|- Written by Killians of PEQ											                        -|
|--------------------------------------------------------------------------|
|- Checks your pets status. Automatically, resummons, buffs, and shrinks.	-|
|--------------------------------------------------------------------------|
SUB check_Pets
/if (${Debug} || ${Debug_Pets}) /echo |- check_Pets ==>
  /if (${Debug} || ${Debug_Pets}) /echo Pets buffcheck ${BuffCheck} petid ${Me.Pet.ID} action !${ActionTaken} active !${activeTimer} pb ${Defined[petBuffs2D]} med !${medBreak} idle !${idle} pbc ${petBuffCombat} combat !${Me.Combat}
	/if (!${ActionTaken} && !${suspend_Pets} && !${activeTimer} && !${rebuffTimer} && !${Me.Invis}) {
    /if (${Me.Pet.ID} && ${Defined[petHeal2D]}) /call check_petHeal
    /if (!${Me.Pet.ID} && !${ActionTaken} && ${Defined[petSpell2D]} && ${SpawnCount[pc ${Me}]} && (${petSummonCombat} || ${Me.CombatState.NotEqual[COMBAT]})) /call summonPet
    /if (${BuffCheck} && ${Me.Pet.ID} && ${Defined[petBuffs2D]} && !${medBreak} && (${petBuffCombat} || ${Me.CombatState.NotEqual[COMBAT]}))	/call buffPet
    /if (${BuffCheck} && ${Me.Pet.ID} && ${petAutoShrink} && !${combatTimer}) /call check_AutoShrink
	}
/if (${Debug} || ${Debug_Pets}) /echo <== check_Pets -|
/RETURN

|------------------------------------------------------------|
|- Summons a new pet.										                     |
SUB summonPet
/if (${Debug} || ${Debug_Pets}) /echo |- summonPet ==>
  |handle zone delay to prevent summoning pet if one exists
  /if (!${Bool[${Zone.ID}]} || ${currentZone} != ${Zone.ID} || ${SpawnCount[pc = ${Me}]}==0) {
    /delay 3s
    /call check_Zone
    /return
  }
  /call check_Ready "petSpell2D" 1
  /if (${c_Ready}) {
    /if (${check_Mana["petSpell2D",1]}) {
	    /call e3_Cast ${Me.ID} "petSpell2D" 1
    }
  }
  /if (${Me.Pet.ID} && ${petTaunt}) {
    /pet taunt on
  } else {
    /pet taunt off
  }
  /if (${Me.Pet.ID}) {
    /squelch /pet ghold on
  }
  /if (${Me.Pet.ID} && ${petAutoEquip}) {
    /call equipMyPet ${petAutoLoadout}
  }
	/if (${Debug} || ${Debug_Pets}) /echo <== summonPet -|
/RETURN

|------------------------------------------|
|- Automatically shrinks pets to min size	-|
SUB check_AutoShrink
/if (${Debug} || ${Debug_Pets}) /echo |- check_AutoShrink ==>
  /if (!${Defined[petShrink2D]}) /return
	/if (${Me.Pet.ID} && ${Me.Pet.Height}>1 && ${Me.Pet.Distance}) {
    /declare s int local
    /for s 1 to ${petShrink2D.Size[1]}
      /call check_Ready "petShrink2D" ${s}
      /if (${c_Ready}) {
        /if (${check_Mana["petShrink2D",${s}]} && ${Me.Pet.Distance} < ${petShrink2D[${s},${iMyRange}]}) {
          /call e3_Cast ${Me.Pet.ID} "petShrink2D" ${s}
        }
      }
    /if (!${ActionTaken}) /next s
  }
/if (${Debug} || ${Debug_Pets}) /echo <== check_AutoShrink -|
/RETURN

|-------------------------------------------------|
|- Manually shrinks pets once.	 /bc Shrink Pets -|
#event shrinkPets "<#1#> Shrink Pets"
SUB event_shrinkPets(line, ChatSender)
/if (${Debug} || ${Debug_Pets}) /echo |- event_shrinkPets ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
  /call check_AutoShrink
/if (${Debug} || ${Debug_Pets}) /echo <== event_shrinkPets -|	
/RETURN

|------------------------------------------------------------|
|- Checks pet health, and attempts to cast PetHeal spells.	-|
SUB check_petHeal
/if (${Debug} || ${Debug_Pets}) /echo |- PetHeal ==>
	/if (${Me.Pet.ID}) {
    /declare petMended bool local FALSE
		| Check Mend Companion AA.
		/if (${Defined[petMendPct]}) {
      /if (${Me.Pet.PctHPs} < ${petMendPct}) {
        /if (${Me.AltAbilityReady[Replenish Companion]}) {
          /call castSimpleSpell "Replenish Companion" ${Me.Pet.ID}
          /varset petMended TRUE
        } else /if (${Me.AltAbilityReady[Mend Companion]}) {
          /call castSimpleSpell "Mend Companion" ${Me.Pet.ID}
          /varset petMended TRUE
        }
      }
    }
    /if (!${Defined[petHeal2D]}) /return
		/if (!${petMended}) {
      /declare s int local
      /for s 1 to ${petHeal2D.Size[1]}
        /if (${Me.Pet.PctHPs} <= ${petHeal2D[${s},${iHealPct}]}) {
          /call check_Ready "petHeal2D" ${s}
          /if (${c_Ready}) {
            /if (${Me.Pet.Distance} < ${petHeal2D[${s},${iMyRange}]}) {
              /if (${check_Mana["petHeal2D",${s}]}) {
                /call e3_Cast ${Me.Pet.ID} "petHeal2D" ${s}
              }
            }
          }
        }
      /next s
    }
	}
/if (${Debug} || ${Debug_Pets}) /echo <== PetHeal -|
/RETURN

|------------------------------------------------------------------------|
|- Rebuff pet.															                            -|
SUB buffPet
/if (${Debug} || ${Debug_Pets}) /echo |- buffPet ==>
  /if (!${Defined[petBuffs2D]}) /return
	/declare s int local
	/for s 1 to ${petBuffs2D.Size[1]}
	|/echo ${petBuffs2D[${s},${iCastName}]} stack ${petBuffs2D[${s},${iSpellName}]} ${Spell[${petBuffs2D[${s},${iSpellName}]}].StacksPet} has !${Bool[${Me.PetBuff[${petBuffs2D[${s},${iSpellName}]}]}]}
	|removing ${Spell[${petBuffs2D[${s},${iSpellName}]}].StacksPet} for now as balikor's mark reports non-stacking
  /if (!${Bool[${Me.PetBuff[${petBuffs2D[${s},${iSpellName}]}]}]} && !${Bool[${Me.PetBuff[${petBuffs2D[${s},${iCheckFor}]}]}]} && !${Defined[nht${petBuffs2D[${s},${iCastID}]}]}) {
    /call check_Ready "petBuffs2D" ${s}
    /if (${c_Ready}) {
      /if (${Me.Pet.Distance} < ${petBuffs2D[${s},${iMyRange}]}) {
        /if (${check_Mana["petBuffs2D",${s}]}) {
          /if (${petBuffs2D[${s},${iSpellName}].NotEqual[Growl of the Panther]} || !${Bool[${Me.Song[Growl of the Panther]}]}) {
            /call e3_Cast ${Me.Pet.ID} "petBuffs2D" ${s}
            /delay 5 ${Target.BuffsPopulated}
            /if (${castReturn.Equal[CAST_SUCCESS]} && !${Bool[${Me.PetBuff[${petBuffs2D[${s},${iSpellName}]}]}]}) {
              /call CreateTimer "nht${petBuffs2D[${s},${iCastID}]}" "${noHoldDelay}"
              /if (${Debug} || ${Debug_Pets}) /bc [+y+] ${Time} ${petBuffs2D[${s},${iSpellName}]} not on Pet, creating NHT
            }
          }
        }
      }
    }
  }
  :skipBuff
  /if (!${ActionTaken}) /next s
/if (${Debug} || ${Debug_Pets}) /echo <== buffPet -|
/RETURN

|------------------------------------------------------------|
|- Suspend a pet. /bc Suspend Pets
#event suspendMinion "<#1#> Suspend Pets#*#"
SUB event_suspendMinion(live, ChatSender)
  /if (${Debug} || ${Debug_Pets}) /echo |- EVENT_suspendMinion ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
  /if (${Me.Pet.ID}) {
    /docommand ${ChatToggle} Suspending my pet...
    /if (${Me.AltAbilityReady[Suspended Minion]}) {
      /if (${Me.Casting.ID}) /call interrupt
      /call castSimpleSpell "Suspended Minion" 0
      /varset currently_Suspended TRUE
    }
    /if (${Me.Pet.ID}) /pet get lost
    /if (${Defined[suspend_Pets]}) /varset suspend_Pets TRUE
  }
  /if (${Debug} || ${Debug_Pets}) /echo <== EVENT_suspendMinion -|
/RETURN

|------------------------------------------------------------|
|- Return suspended pet. /bc return Pets
#event returnMinion "<#1#> return Pets#*#"
SUB event_returnMinion(line, ChatSender)
  /if (${Debug} || ${Debug_Pets}) /echo |- EVENT_returnMinion ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
  /if (!${c_eventArg}) /return
    /if (${suspend_Pets}) {
      /docommand ${ChatToggle} Returning my pet...
      /if (${currently_Suspended}) {
        /call castSimpleSpell "Suspended Minion" 0
        /varset currently_Suspended FALSE
      }
      /varset suspend_Pets FALSE
    }
  /if (${Debug} || ${Debug_Pets}) /echo <== EVENT_returnMinion -|
/RETURN

|------------------------------------------------------------|
| functions for supplying equipment and weapons to pets
#event equipMyPet "<#1#> Equip Pet"
#event equipMyPet "<#1#> Equip Pet #2#"
#event equipMyPet "[#1#(msg)] Equip Pet"
#event equipMyPet "[#1#(msg)] Equip Pet #2#"
#event equipMyPet "#1# tells you, 'Equip Pet'"
#event equipMyPet "#1# tells you, 'Equip Pet #2#'"
sub event_equipMyPet(line, ChatSender)
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: equipMyPet ==>

  /if (!${NetBots[${ChatSender}].ID}>0) /return 

  /declare loadType string local
  /if (${line.Find[raid]}) {
    /varset loadType ${petRaidLoadout}
  } else /if (${line.Find[dps]}) {
    /varset loadType ${petDpsLoadout}
  } else /if (${line.Find[hc]}) {
    /varset loadType ${petHcLoadout}
  } else {
      /if (${line.Find[tells you]}) {
        /if (${line.Token[6, ].Length}>0) /varset loadType ${line.Token[6, ].Left[-1]}
    } else {
        /if (${line.Token[4, ].Length}>0) /varset loadType ${line.Token[4, ].Left[-1]}
    }
  }
  /if (${Debug} || ${Debug_Pets}) /echo [${Time}]: equipMyPet: loadType is ${loadType} and line is ${line}

  /call equipMyPet ${loadType}

/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== equipMyPet
/return 

|------------------------------------------------------------|
| Summons a new pet and equips specified weapon loadout
#event resummonAndGearPet "<#1#> New Pet #2#"
#event resummonAndGearPet "[#1#(msg)] New Pet #2#"
#event resummonAndGearPet "#1# tells you, 'New Pet #2#'"
sub event_resummonAndGearPet(line, ChatSender)
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: resummonAndGearPet ==>
  | Prevents other players from killing your pet
  /if (!${NetBots[${ChatSender}].ID}>0) /return 
  /declare newLoadOut string local
  /if (${line.Find[raid]}) {
    /varset newLoadOut ${petRaidLoadout}
  } else /if (${line.Find[dps]}) {
    /varset newLoadOut ${petDpsLoadout}
  } else /if (${line.Find[hc]}) {
    /varset newLoadOut ${petHcLoadout}
  } else {
      /if (${line.Find[tells you]}) {
        /if (${line.Token[6, ].Length}>0) /varset newLoadOut ${line.Token[6, ].Left[-1]}
    } else {
        /if (${line.Token[4, ].Length}>0) /varset newLoadOut ${line.Token[4, ].Left[-1]}
    }
  }
  
  /if (!${newLoadOut.Length}>0) {
    /bc [${Time}]: No loadout was specified, exiting.
    /return
  }
  /if (!(${FindItemCount[${petPack_EnibiksHeirlooms}]} > 0)) {
    /bc [${Time}]: I do not have a ${petPack_EnibiksHeirlooms} to give to pet. Cancelling.
    /return
  }
  /if (!(${FindItemCount[${petPack_SpectralPlate}]} > 0)) {
    /bc [${Time}]: I do not have a ${petPack_SpectralPlate} to give to pet. Cancelling.
    /return
  }
  /if (!(${FindItemCount[${petPack_SpectralArm}]} > 0)) {
    /bc [${Time}]: I do not have a ${petPack_SpectralArm} to give to pet. Cancelling.
    /return
  }

  /if (${Debug} || ${Debug_Pets}) /echo [${Time}]: resummonAndGearPet: new load out is ${newLoadOut} and line is ${line}

  /declare autoEquipEnabled bool local ${petAutoEquip}
  /if (${petAutoEquip}) /varset petAutoEquip false
  /pet leave
  /call summonPet 
  /call equipMyPet ${newLoadOut}
  /if (${autoEquipEnabled}) /varset petAutoEquip true

/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== resummonAndGearPet
/return

sub equipMyPet(loadoutName)
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: equipMyPet ==>
  /call checkForOpenBagSlot
  /if (!${Macro.Return}) {
    /bc [${Time}]: There was an error getting a bag slot, unable to arm pets.
    /return 
  }

  /bc [${Time}]: Attempting to equip my pet with ${loadoutName} load out.
  /call equipOnePet ${Me.Name} ${loadoutName}
  /call destroyOldSummonedPacks

  /if (!(${FindItemCount[${petPack_EnibiksHeirlooms}]} > 0) || !(${FindItemCount[${petPack_SpectralPlate}]} > 0) || !(${FindItemCount[${petPack_SpectralArm}]} > 0)) {
    /bc [${Time}]: I am out of one or more Mage folded packs and will need more to equip my next pet. 
  } 

  | Try to put a folded pack in the bag slot to placehold it.
  /if ((${FindItemCount[${petPack_EnibiksHeirlooms}]} > 0)) {
    /itemnotify ${petPack_EnibiksHeirlooms} leftmouseup
  } else /if ((${FindItemCount[${petPack_SpectralPlate}]} > 0)) {
    /itemnotify ${petPack_SpectralPlate} leftmouseup
  } else /if ((${FindItemCount[${petPack_SpectralArm}]} > 0)) {
    /itemnotify ${petPack_SpectralArm} leftmouseup
  }
  /call ClearCursor
  
  /bc [${Time}]: Finished Equiping my pet. Remaining packs - Heirloom: ${FindItemCount[${petPack_EnibiksHeirlooms}]}, Plate: ${FindItemCount[${petPack_SpectralPlate}]}, Arm: ${FindItemCount[${petPack_SpectralArm}]}
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== equipMyPet 
/return 

|------------------------------------------------------------|
| Gives armor, heirlooms and weapons to pet. Also used by Mages for Arm Pet.
sub equipOnePet(playerName, loadoutType)
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: equipOnePet ==>
    /if (!${Spawn[${playerName}].Pet.ID} || ${Spawn[${playerName}].Pet.Name.Find[familiar]} || !${Spawn[${playerName}].Pet.Level}>1) /return

    /if (${Spawn[${playerName}].Pet.Distance}>50) {
      /echo [${Time}]: equipOnePet: Pet is too far away.
      /return
    }

    /if (!(${FindItemCount[${petPack_EnibiksHeirlooms}]} > 0)) {
      /bc [${Time}]: equipOnePet: I do not have a ${petPack_EnibiksHeirlooms} to give to pet.
      /return
    }
    /if (!(${FindItemCount[${petPack_SpectralPlate}]} > 0)) {
      /bc [${Time}]: equipOnePet: I do not have a ${petPack_SpectralPlate} to give to pet.
      /return
    }
    /if (!(${FindItemCount[${petPack_SpectralArm}]} > 0)) {
      /bc [${Time}]: equipOnePet: I do not have a ${petPack_SpectralArm} to give to pet.
      /return
    }

    | In case they decide to leave while casting.
    /if (!${SpawnCount[pc ${playerName}]} || ${Spawn[pc ${playerName}].Distance} > 100) {
      /tell ${playerName} It appears you left before I was finished.
      /return
    }

    /declare bodytype string local
    /declare char_class string local
    /declare primaryWeapon string local
    /declare secondaryWeapon string local 

    /if (${FindItemCount[${petPack_EnibiksHeirlooms}]} > 0) {
      /if (${Debug} || ${Debug_Pets}) /echo [${Time}]: equipOnePet: Creating Heirlooms
      /call unpackAndGive ${Spawn[${playerName}].Pet.ID} ${petPack_EnibiksHeirlooms}
    } 
    /if (${Cursor.ID}) {
      /call ClearCursor
      /delay 2s !${Bool[${Cursor.ID}]}
      /bc [${Time}]: equipOnePet: Something went wrong and something is on your cursor, exiting.
      /return
    }
    /if (!${SpawnCount[pc ${playerName}]} || ${Spawn[pc ${playerName}].Distance} > 100) {
      /tell ${playerName} It appears you left before I was finished.
      /return
    }

    /if (${FindItemCount[${petPack_SpectralPlate}]} > 0) {
      /if (${Debug} || ${Debug_Pets}) /echo [${Time}]: equipOnePet: Creating Spectral Plate
      /call unpackAndGive ${Spawn[${playerName}].Pet.ID} ${petPack_SpectralPlate}
    } 
    /if (${Cursor.ID}) {
      /call ClearCursor
      /delay 2s !${Bool[${Cursor.ID}]}
      /bc [${Time}]: equipOnePet: Something went wrong and something is on your cursor, exiting.
      /return
    }
    /if (!${SpawnCount[pc ${playerName}]} || ${Spawn[pc ${playerName}].Distance} > 100) {
      /tell ${playerName} It appears you left before I was finished.
      /return
    }
        
    |/echo [${Time}]: the line to parse is:${loadoutType}
    /if ((${FindItemCount[${petPack_SpectralArm}]} > 0) && !${loadoutType.Find[none]} && !${loadoutType.Find[noweapon]}) {
      /varset char_class ${Spawn[${playerName}].Class} 
      /if (${Debug} || ${Debug_Pets}) /echo [${Time}]: loadoutType: ${loadoutType}

      /if (${loadoutType.Find[,]} || ${loadoutType.Find[2]}) {
        /if (${Debug} || ${Debug_Pets}) /echo [${Time}]: Loadout Token1 ${loadoutType.Token[1,,]} Token2 ${loadoutType.Token[2,,]} 
        /declare firstItem string local
        /declare secondItem string local

        /if (${loadoutType.Find[2]}) {
          /if (${Debug} || ${Debug_Pets}) /echo [${Time}]: Setting loadout type for same primary/secondary: ${loadoutType},${loadoutType}
          /varset loadoutType ${loadoutType},${loadoutType}
        }
        
        /if (${loadoutType.Token[1,,].Length}>0) {
          /varset firstItem ${loadoutType.Token[1,,]}

          /if (${firstItem.Find[fire]}) {
            /varset primaryWeapon ${pet_FireWeapon}

          } else /if (${firstItem.Find[ice]}) {
            /varset primaryWeapon ${pet_IceWeapon}

          } else /if (${firstItem.Find[slow]}) {
            /varset primaryWeapon ${pet_SlowWeapon}

          } else /if (${firstItem.Find[mala]}) {
            /varset primaryWeapon ${pet_MalaWeapon}

          } else /if (${firstItem.Find[dispel]}) {
            /varset primaryWeapon ${pet_DispelWeapon}

          } else /if (${firstItem.Find[ward]}) {
            /varset primaryWeapon ${pet_WardWeapon}

          } else {
            /bc [${Time}]: Invalid primary weapon specified, skipping weapon step.
            /return
          }
        }

        /if (${loadoutType.Token[2,,].Length}>0) {
          /varset secondItem ${loadoutType.Token[2,,]}

          /if (${secondItem.Find[fire]}) {
            /varset secondaryWeapon ${pet_FireWeapon}

          } else /if (${secondItem.Find[ice]}) {
            /varset secondaryWeapon ${pet_IceWeapon} 

          } else /if (${secondItem.Find[mala]}) {
            /varset secondaryWeapon ${pet_MalaWeapon}

          } else /if (${secondItem.Find[dispel]}) {
            /varset secondaryWeapon ${pet_DispelWeapon}

          } else /if (${secondItem.Find[slow]}) {
            /varset secondaryWeapon ${pet_SlowWeapon}

          } else /if (${secondItem.Find[shield]}) {
            /varset secondaryWeapon ${pet_DrainingBuckler}

          } else {
            /bc [${Time}]: Invalid secondary weapon specified, skipping weapon step.
            /return
          }
        }

      } else /if (${loadoutType.Find[default]} || ${loadoutType.Find[class]}) {

          /if (${char_class.Find[Magician]} || ${char_class.Find[Necro]} || ${char_class.Find[Beastlord]}) {
            |/echo [${Time}]: Arm Pet: Doing ${char_class} loadout
            /if (${loadoutType.Find[ice]}) {
              /varset primaryWeapon ${pet_IceWeapon}
              /varset secondaryWeapon ${pet_IceWeapon}
              
            } else {
              /varset primaryWeapon ${pet_FireWeapon}
              /varset secondaryWeapon ${pet_FireWeapon}
            }
          } else {
            |When its any other class with pet
            |/echo [${Time}]: loadoutType: ${loadoutType}
            /echo [${Time}]: equipOnePet: Doing Default ${char_class} loadout
            
            /varset primaryWeapon ${pet_DispelWeapon}
            /varset secondaryWeapon ${pet_MalaWeapon}
          }
      } else {
        /bc [${Time}]: Invalid loadout specified, skipping weapon step.
        /return
      }

      /echo [${Time}]: Equip Pet: Selected Weapons are ${primaryWeapon} and ${secondaryWeapon}

      /call unpackAndSelect ${Spawn[${playerName}].Pet.ID} ${petPack_SpectralArm} ${primaryWeapon} ${secondaryWeapon}
    } 
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== equipOnePet
/return

sub unpackAndSelect(targetID, ItemName, primaryItem, secondaryItem)
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: unpackAndSelect ==>

    /if (${FindItemCount[${ItemName}]} > 0) {
        |/echo [${Time}]: unfolding the bag to cursor
        /ItemNotify "${ItemName}" rightmouseup
        /delay 3s ${Bool[${Cursor.ID}]}
        /call ClearCursor
        /delay 3s !${Bool[${Cursor.ID}]}
        /delay 1s
    } else {
      /bc [${Time}]: I do not have a ${ItemName} to equip pet
      /return
    }

    /echo [${Time}]: Giving Primary Weapon to pet: ${primaryItem}
    /itemnotify "${primaryItem}" leftmouseup
    /delay 5s ${Bool[${Cursor.ID}]}
    /if (!${Cursor.ID}) {
      /echo [${Time}]: unpackAndSelect: Could not load primary item to cursor, exiting
      /return
    }
    |/echo giving primary item to pet
    /call targetMoveAndGive ${targetID}

    /if (${Cursor.ID}) {
      /echo [${Time}]: unpackAndSelect: Something went wrong and something is on your cursor, exiting.
      /return
    }

    |check to see if we have a 2ndary item
    /if (${secondaryItem.Length}>0) {
      /if (!${Bool[${FindItem[${secondaryItem}]}]}) {
      
      |we don't have the 2ndary item in this pack, summon the pack again and try again
      |now delete the object
      /echo need new pack for secondary
        /itemnotify ${pet_QuelliousPouch} leftmouseup
        /delay 5s ${Bool[${Cursor.ID}]}
        /if (${Cursor.Name.Find[${pet_QuelliousPouch}]}) {
          /destroy
        } else {
          /echo [${Time}]: unpackAndSelect: Could not destroy bag, exiting
          /return
        }
        /if (!${FindItemCount[${ItemName}]} > 0) {
          /bc [${Time}]: I do not have a ${ItemName} to give pet secondary ${secondaryWeapon}
          /return 
        }
        /if (${FindItemCount[${ItemName}]} > 0) {
          |/echo unfolding the bag to cursor
          /ItemNotify "${ItemName}" rightmouseup
          /delay 3s ${Bool[${Cursor.ID}]}
          /call ClearCursor
          /delay 3s !${Bool[${Cursor.ID}]}
          /call TrueTarget ${targetID}
        }
      }
        
      |2ndary item
      /echo [${Time}]: Giving Secondary weapon to pet: ${secondaryItem}
      /itemnotify "${secondaryItem}" leftmouseup
      /delay 5s ${Bool[${Cursor.ID}]}
      /if (!${Cursor.ID}) {
        /echo [${Time}]: unpackAndSelect: Could not load secondary item to cursor.
        /return
      }
      
      |/echo giving secondary item to pet
      /call targetMoveAndGive ${targetID}
    }
    /if (${Cursor.ID}) {
      /echo [${Time}]: unpackAndSelect: Something went wrong and something is on your cursor.
      /return
    }
  
    |now delete the object
    /itemnotify ${pet_QuelliousPouch} leftmouseup
    /delay 5s ${Bool[${Cursor.ID}]}
    /if (${Cursor.Name.Find[${pet_QuelliousPouch}]}) {
        /destroy
    } else {
      /echo [${Time}]: unpackAndSelect: Could not destroy bag.
      /return
    }
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== unpackAndSelect
/return


sub checkForOpenBagSlot
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: checkForOpenBagSlot ==>
  |first check that we have an empty slot
  /declare BackSlotThatIsOpen int 0
  /declare itemInSlotID int local 0
  /declare itemInSlot string local NULL
  /declare i int local

  /for i 1 to 10
  /if (!${Bool[${InvSlot[pack${i}].Item.ID}]}) {

    /varset BackSlotThatIsOpen ${i}
    /goto :bagslotcheckdone

  } else /if (!${Bool[${InvSlot[pack${i}].Item.Container}]}) {

    /varset BackSlotThatIsOpen ${i}
    /varset itemInSlotID ${i}
    /varset itemInSlot ${InvSlot[pack${i}].Item.Name}
    /goto :bagslotcheckdone

  } else /if (${InvSlot[pack${i}].Item.Name.Equal[${pet_PhantomSatchel}]} || ${InvSlot[pack${i}].Item.Name.Equal[${pet_QuelliousPouch}]}) {

    /call destroyOldSummonedPacks
    /if (!${Bool[${InvSlot[pack${i}].Item.ID}]}) {
      |/echo Pack slot ${i}
      /varset BackSlotThatIsOpen ${i}
      /goto :bagslotcheckdone
    } else {
      /if (${Bool[${InvSlot[pack${itemInSlotID}].Item.ID}]}) {
        /bc [${Time}]: checkForOpenBagSlot: Bag slot not cleared. Exiting
        /return false
      }
    }
  }
  /next i
  :bagslotcheckdone

  | If an item was found in a pack slot, move the item to the first available slot inside a bag
  /if (${itemInSlotID} > 0) {
    /declare m int local 1
    /declare n int local 1
    /for m 1 to 10
    /for n 1 to 10 
    /if (!${InvSlot[pack${m}].Item.Item[${n}].ID}) {
      /shift /itemnotify pack${itemInSlotID} leftmouseup
      /itemnotify in pack${m} ${n} leftmouseup
      /goto :bagslotcleared
    }
    /next n
    /next m
    :bagslotcleared
    /if (${Cursor.ID}) {
      |/bc Equip: Unable to clear bagslot. Exiting
      /goto :armpetend
    }
    /if (${Bool[${InvSlot[pack${itemInSlotID}].Item.ID}]}) {
      /bc [${Time}]: checkForOpenBagSlot: Bag slot not cleared. Exiting
      /return false
    }
  }

  /if (${BackSlotThatIsOpen}==0) {
    |no empty bag slot available
    /bc [${Time}]: checkForOpenBagSlot: No empty bag slot on toon. Exiting.
    /return false
  } 
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== checkForOpenBagSlot
/return true


sub destroyOldSummonedPacks
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: destroyOldSummonedPacks ==>
    /echo [${Time}]: Finding and destroying opened summoned packs
    | Just in case mage has been looting and items ended up in an old phantom pack
    /call ClearCursor
    /if (${Cursor.ID}) {
      /bc [${Time}]: destroyOldSummonedPacks: Unable to clear cursor.
      /return
    }
    /declare m int local
    /declare n int local
    /for m 1 to 10
    /if (${InvSlot[pack${m}].Item.Name.Equal[${pet_PhantomSatchel}]} || ${InvSlot[pack${m}].Item.Name.Equal[${pet_QuelliousPouch}]}) {
      /for n 1 to 10
      |/echo Item ${InvSlot[pack${m}].Item.Item[${n}].Name}
      /if (!${InvSlot[pack${m}].Item.Item[${n}].NoRent} && ${InvSlot[pack${m}].Item.Item[${n}].ID} > 0) {
        /bc [${Time}]: destroyOldSummonedPacks: Non-Temporary Item ${InvSlot[pack${m}].Item.Item[${n}]} found in ${InvSlot[pack${m}].Item.Name}.
        /return
      }      
      /if (${InvSlot[pack${m}].Item.Item[${n}].NoRent} && (!${InvSlot[pack${m}].Item.Item[${n}].Name.Find[Enibik]} && !${InvSlot[pack${m}].Item.Item[${n}].Name.Find[Spectral]} && !${InvSlot[pack${m}].Item.Item[${n}].Name.Find[Summoned]})) {
        /bc [${Time}]: destroyOldSummonedPacks: Unknown temporary item ${InvSlot[pack${m}].Item.Item[${n}]} found in ${InvSlot[pack${m}].Item.Name}.
        /return 
      }

      /next n
      /itemnotify pack${m} leftmouseup
      /if (${Cursor.Name.Find[${pet_PhantomSatchel}]} || ${Cursor.Name.Find[${pet_QuelliousPouch}]}) {
        /destroy
        /delay 1s
        /if (${Cursor.ID}) {
          /bc [${Time}]: destroyOldSummonedPacks: Unable to clear bagslot.
          /return
        }
      }
    }
    /next m
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== destroyOldSummonedPacks
/return

sub unpackAndGive(targetID, ItemName)
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: unpackAndGive ==>
    /if (${FindItemCount[${ItemName}]} > 0) {
      |/echo ItemFound: ${ItemName}
      /itemnotify "${ItemName}" leftmouseup
    } 

    /delay 1s
    |/echo done casting, put in inventory
    /autoinventory
    /delay 3s !${Bool[${Cursor.ID}]}
    /delay 1s
    |/echo unfolding the bag to cursor
    /ItemNotify "${ItemName}" rightmouseup
    /delay 3s ${Bool[${Cursor.ID}]}
    |/echo getting the target of the pet
    /call targetMoveAndGive ${targetID}

    /if (${Cursor.ID}) {
      /echo [${Time}]: unpackAndGive: Something went wrong and something is on your cursor, exiting.
      /return
    }
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== unpackAndGive
/return


sub targetMoveAndGive(petId)
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: targetMoveAndGive ==>
    /call TrueTarget ${petId}
    /if (${Target.ID} && ${Target.Distance}<50) {
      /call MoveToLoc ${Target.Y} ${Target.X} 50 15
      /call giveItemOnCursorToTarget
    } else {
      /echo [${Time}]: Pet is too far away.
      /return
    }
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== targetMoveAndGive
/return 


sub giveItemOnCursorToTarget()
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: giveItemOnCursorToTarget ==>
    /if (${Defined[retryWeapTimer]}) /deletevar retryWeapTimer
    /declare retryWeapTimer timer local 10s

    |/echo trying to trade heirloom pack to pet
    :OpenTrade_Loop

    /click left target
    /delay 3s ${Window[GiveWnd].Open}
    /if (!${Window[GiveWnd].Open}) {
        /if (${retryWeapTimer} && ${Cursor.ID}) {
          /goto :OpenTrade_Loop
        } else {
          /echo [${Time}]: Failed to open trade with ${Target.CleanName}.
        }
    } else {
      :WaitAccept_Loop
      /notify GiveWnd GVW_Give_Button LeftMouseUp
      /delay 1s !${Window[GiveWnd].Open}
      /if (${Window[GiveWnd].Open}) {
        /if (${retryWeapTimer}) {
          /goto :WaitAccept_Loop
        } else {
          /echo [${Time}]: Failed to open trade with ${Target.CleanName}.
        }
      }
    }
/if (${Debug} || ${Debug_Pets}) /echo [${Time}]: <== giveItemOnCursorToTarget
/return


|------------------------------------------------------------------------|
|- Setup															                                  -|
Sub pet_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Pets (On/Off)" Debug_Pets bool outer
/if (${Debug} || ${Debug_Pets}) /echo |- pet_Setup ==>
	/if (${Ini[${Character_Ini},Pets].Length}) {
		/if (!${Defined[suspend_Pets]})        /declare suspend_Pets bool global FALSE
		/if (!${Defined[currently_Suspended]}) /declare currently_Suspended bool global FALSE
    /if (${Ini[${Character_Ini},Pets,Pet Mend (Pct)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Mend (Pct)" petMendPct int outer
    /if (${Ini[${Character_Ini},Pets,Pet Taunt (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Taunt (On/Off)" petTaunt bool outer
		/if (${Ini[${Character_Ini},Pets,Pet Auto-Shrink (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Auto-Shrink (On/Off)" petAutoShrink bool outer
    /if (${Ini[${Character_Ini},Pets,Pet Summon Combat (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Summon Combat (On/Off)" petSummonCombat bool outer
    /if (${Ini[${Character_Ini},Pets,Pet Buff Combat (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Buff Combat (On/Off)" petBuffCombat bool outer
    /if (${Ini[${Character_Ini},Pets,Pet Auto Equip (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Auto Equip (On/Off)" petAutoEquip bool outer false
    /if (${Ini[${Character_Ini},Pets,Pet Auto Loadout].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Auto Loadout" petAutoLoadout string outer
    /if (${Ini[${Character_Ini},Pets,Pet Raid Loadout].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Raid Loadout" petRaidLoadout string outer
    /if (${Ini[${Character_Ini},Pets,Pet DPS Loadout].Length}) /call iniToVarV "${Character_Ini},Pets,Pet DPS Loadout" petDpsLoadout string outer
		/if (${Ini[${Character_Ini},Pets,Pet HC Loadout].Length}) /call iniToVarV "${Character_Ini},Pets,Pet HC Loadout" petHcLoadout string outer    
	}

	/call pet_spellArrays
  /call equipPet_Globals
/if (${Debug} || ${Debug_Pets}) /echo <== pet_Setup -|
/return

|------------------------------------------------------------------------|
|- spell setup
SUB pet_spellArrays
  /call IniToArrayV "${Character_Ini},Pets,Pet Spell#" petSpell
  /call IniToArrayV "${Character_Ini},Pets,Pet Buff#" petBuffs
  /call IniToArrayV "${Character_Ini},Pets,Pet Heal#" petHeal

  /if (${Defined[petSpell]}) /call BuildSpellArray "petSpell"	"petSpell2D"

  /if (${Defined[petBuffs]}) {
    /varset BuffCheck TRUE
    /call BuildSpellArray "petBuffs" "petBuffs2D"
  }
  /if (${Defined[petHeal]})   /call BuildSpellArray "petHeal"	"petHeal2D"

  /if (${FindItemCount[=Gemstone of Dark Flame]}) {
    /call BuildArray petShrink "Gemstone of Dark Flame"
  } else /if (${FindItemCount[=Symbol of Ancient Summoning]}) {
    /call BuildArray petShrink "Symbol of Ancient Summoning"
  } else /if (${Me.Book[Tiny Companion]}) {
    /call BuildArray petShrink "Tiny Companion"
  }
  /if (${Defined[petShrink]}) /call BuildSpellArray "petShrink"	"petShrink2D"

/RETURN

|------------------------------------------------------------------------|
SUB pet_MacroSettings
	/call WriteToIni "${advSettings_Ini},Debug,Debug Pets (On/Off)" Off
/RETURN

|------------------------------------------------------------------------|
SUB pet_CharacterSettings
/if (${Debug}) /echo |- pet_CharacterSettings ==>
	/if (${Select[${Me.Class.ShortName},BST,DRU,ENC,MAG,NEC,SHD,SHM]}) {	
		/call WriteToIni "${Character_Ini},Pets,Pet Spell"
    /call WriteToIni "${Character_Ini},Pets,Pet Heal"
		/call WriteToIni "${Character_Ini},Pets,Pet Buff"
    /if (${Select[${Me.Class.ShortName},BST,MAG,NEC]}) /call WriteToIni "${Character_Ini},Pets,Pet Mend (Pct)"
    /call WriteToIni "${Character_Ini},Pets,Pet Taunt (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Auto-Shrink (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Summon Combat (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Buff Combat (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Auto Equip (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Auto Loadout"
    /call WriteToIni "${Character_Ini},Pets,Pet Raid Loadout"
 		/call WriteToIni "${Character_Ini},Pets,Pet DPS Loadout"
    /call WriteToIni "${Character_Ini},Pets,Pet HC Loadout"
	}
/if (${Debug}) /echo <== pet_CharacterSettings -|
/RETURN


sub equipPet_Globals

  /if (!${Defined[pet_FireWeapon]}) /declare pet_FireWeapon string outer "Summoned: Fist of Flame"
  /if (!${Defined[pet_IceWeapon]}) /declare pet_IceWeapon string outer "Summoned: Orb of Chilling Water"
  /if (!${Defined[pet_MalaWeapon]}) /declare pet_MalaWeapon string outer "Summoned: Spear of Maliciousness"
  /if (!${Defined[pet_SlowWeapon]}) /declare pet_SlowWeapon string outer "Summoned: Mace of Temporal Distortion"
  /if (!${Defined[pet_DispelWeapon]}) /declare pet_DispelWeapon string outer "Summoned: Wand of Dismissal"
  /if (!${Defined[pet_WardWeapon]}) /declare pet_WardWeapon string outer "Summoned: Short Sword of Warding"
  /if (!${Defined[pet_DrainingBuckler]}) /declare pet_DrainingBuckler string outer "Summoned: Buckler of Draining Defense"
  /if (!${Defined[pet_ClericHammer]}) /declare pet_ClericHammer string outer "Hammer of Damnation"
  
  /if (!${Defined[pet_PhantomSatchel]}) /declare pet_PhantomSatchel string outer "Phantom Satchel"
  /if (!${Defined[pet_QuelliousPouch]}) /declare pet_QuelliousPouch string outer "Pouch of Quellious"
  /if (!${Defined[hugeDisenchantedPack]}) /declare hugeDisenchantedPack string outer "Huge Disenchanted Backpack"  

  /if (!${Defined[petSpell_SpectralArm]}) /declare petSpell_SpectralArm string outer "Grant Spectral Armaments"
  /if (!${Defined[petSpell_SpectralPlate]}) /declare petSpell_SpectralPlate string outer "Grant Spectral Plate"
  /if (!${Defined[petSpell_EnibiksHeirlooms]}) /declare petSpell_EnibiksHeirlooms string outer "Grant Enibik's Heirlooms"

  /if (!${Defined[petPack_SpectralArm]}) /declare petPack_SpectralArm string outer "Folded Pack of Spectral Armaments"
  /if (!${Defined[petPack_SpectralPlate]}) /declare petPack_SpectralPlate string outer "Folded Pack of Spectral Plate"
  /if (!${Defined[petPack_EnibiksHeirlooms]}) /declare petPack_EnibiksHeirlooms string outer "Folded Pack of Enibik's Heirlooms"

/return 

|------------------------------------------------------------------------|
Sub pet_Aliases
	/squelch /alias /SuspendPets /bc Suspend Pets
	/squelch /alias /ReturnPets /bc Return Pets
	/squelch /alias /ShrinkPets /bc Shrink Pets
/return

|------------------------------------------------------------------------|
Sub pet_Background_Events
  /doevents equipMyPet
  /doevents resummonAndGearPet
  /doevents suspendMinion
  /doevents returnMinion
  /doevents shrinkPets
/return