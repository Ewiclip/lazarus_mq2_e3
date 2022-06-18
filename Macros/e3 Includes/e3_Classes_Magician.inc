|----------------------------------------------------------------------------|
| Magician Functions
|----------------------------------------------------------------------------|

#event armPets "<#1#> Pet Weapon#*#"
#event armPets "<#1#> Arm Pets"
#event armPets "<#1#> Arm Pets #2#"
#event armPets "#1# tells you, 'Arm Pet'"
#event armPets "#1# tells you, 'Arm Pets'"
#event armPets "#1# tells you, 'Pet Weapon#*#'"
#event armPets "#1# tells you, 'Pet Armor*#'"
#event armPets "#1# tells you, 'Arm Pet #2#'"
#event armPets "#1# tells you, 'Arm Pets #2#'"
sub event_armPets(line, ChatSender)
/if (${Debug} || ${Debug_MAG})  /echo event_armPets ==>

  | For some reason events dont like optional parameters, this is how to get the arguments after 'arm pets' in the tell
  /declare characterToWorkOn string local
  /declare loadType string local
  /if (${line.Find[tells you]}) {
    /if (${line.Token[6, ].Length}>0) /varset loadType ${line.Token[6, ]}
    /if (${line.Token[7, ].Length}>0) /varset characterToWorkOn ${line.Token[7, ].Left[-1]}
  } else {
    /if (${line.Token[4, ].Length}>0) /varset loadType ${line.Token[4, ]}
    /if (${line.Token[5, ].Length}>0) /varset characterToWorkOn ${line.Token[5, ].Left[-1]}
  }
  
  /if (${Debug} || ${Debug_MAG}) /echo [${Time}] LoadType: ${loadType}, CharacterToWorkOn: ${characterToWorkOn}
  /echo [${Time}] LoadType: ${loadType}, CharacterToWorkOn: ${characterToWorkOn}

  /call checkForOpenBagSlot
  /if (!${Macro.Return}) {
    /echo There was an error getting a bag slot, unable to arm pets.
    /return 
  }

  /tell ${ChatSender} Starting to arm pets. Stand close.

  | When finished, will try to move mage back to their original location
  /declare meY float local ${Me.Y}
  /declare meX float local ${Me.X}

  /if (${Cursor.ID}) {
    /echo [${Time}]: Arm Pets: Something is on your cursor, exiting.
    | delays are needed as back to back tells fail to send
    /delay 3s 
    /tell ${ChatSender}  Arm Pets: Error, check mage for detail.
    /goto :armpetend
  }

  | Determine if player is in my group
  /if (${Group.Leader.ID}>0) {
    /declare g int local
    /declare memberOfMyGroup bool local
    /for g 0 to ${Group.Members}
      /if (${ChatSender.Equal[${Spawn[${Group.Member[${g}].Name}]}]}) /varset memberOfMyGroup true
    /next g
  }

  |/echo Character to work on is: ${characterToWorkOn}
  /declare groupCounter int local
  /if (${Bool[${line.Find[pet weapon]}]} || ${Bool[${line.Find[pet armor]}]} || !${loadType.Length}>0) { 
    | For compatibility, if player calls pet weapon it will only give armor/heirloom
    /echo [${Time}]: Pet Weapon was called, only handing armor/heirloom to pet
    /if (${Spawn[${ChatSender}].Pet.Level}<45) {
        /call armLowbiePet ${ChatSender}
    } else {
      /delay 3s
      |/tell ${ChatSender}  Either Pet Weapon was called or you did not specify a loadout, only givng Armor and Heirlooms to your pet.
      /tell ${ChatSender}  I can give your pet Weapons too! Next time try 'Arm pet 2fire' or 'Arm pet slow,dispel' (options: fire, ice, slow, mala, dispel, ward, shield) Either comma separate or put 2 in front.  For more info search pet weapons on the wiki.
      /call armOnePet ${ChatSender} noweapon
    }

  } else /if (${characterToWorkOn.Length} > 0) {

    | User passed in a comma separated list of toons to work on
    /if (${characterToWorkOn.Find[,]}) {

      /declare currentChar string local

      /for groupCounter 1 to ${Math.Calc[${characterToWorkOn.Count[,]}+1]}

      /varset currentChar ${characterToWorkOn.Arg[${groupCounter},,]}
      |/echo [${Time}]: Working on ${Spawn[${characterToWorkOn.Arg[${groupCounter},,]}].Name}
      /if (${Spawn[${currentChar}].ID}>0) {
        /call armOnePet ${currentChar} ${loadType}
        /if (${Cursor.ID}) {
          /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
          /goto :armpetend
        } 
      } else {
        /delay 3s
        /tell ${ChatSender} Could not locate ${currentChar}, skipping.
      }
      /next groupCounter

    } else {
      | Character name was provided, arm only their pet
      |/echo Have one character to work on. Char: ${characterToWorkOn} Spawn ID: ${Spawn[${characterToWorkOn}].ID}
      /if (${Spawn[${characterToWorkOn}].ID}>0) {
        /echo [${Time}]: found Character to work on.

        /call armOnePet ${characterToWorkOn} ${loadType} 
      } else {
        /delay 3s
        |/echo [${Time}]: Didnt find character ${characterToWorkOn}
        /tell ${ChatSender}  Could not locate character ${characterToWorkOn}
      }
    }

  } else /if (${memberOfMyGroup}) {
    | Character is a member of my group, arm all pets in group
    /for groupCounter 0 to ${Group.Members}
    /call armOnePet ${Spawn[${Group.Member[${groupCounter}].Name}]} ${loadType} 
    /if (${Cursor.ID}) {
      /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
      /goto :armpetend
    }    
    /next groupCounter

  } else { 
    | When the chat sender is not a member, arm only their pet
    |/echo [${Time}]: Arming ${ChatSender} 's Pet
    /call armOnePet ${ChatSender} ${loadType} 
  }

  /call findAndDestroyOldSatchels
  /if (${FindItemCount[${petPack_SpectralArm}]} > 0) {
    /itemnotify ${petPack_SpectralArm} leftmouseup
  } else {
    /call castSimpleSpell ${petSpell_SpectralArm} ${Me.ID}
  }
  /delay 3s ${Bool[${Cursor.ID}]}
  /call ClearCursor
  
  /if (${Cursor.ID}) {
    /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
    /goto :armpetend
  }

  /delay 3s
  /tell ${ChatSender} Finished arming pets.
  :armpetend
  /moveto loc ${meY} ${meX}

/if (${Debug} || ${Debug_MAG}) /echo <== event_armPets -|
/return


#event petGear "#1# tells you, 'Pet Gear'"
#event petGear "#1# tells you, 'Pet Gear #2#'"
#event petGear "<#1#(msg)> Pet Gear"
#event petGear "<#1#(msg)> Pet Gear #2#"
sub event_petGear(line, ChatSender)
/if (${Debug} || ${Debug_MAG}) /echo event_PetGear ==>
  /declare quantity int local 1
  
  /if (${Debug} || ${Debug_MAG}) /echo Line tokens 1 ${line.Token[1, ]}  2: ${line.Token[2, ]} 3: ${line.Token[3, ]} 4: ${line.Token[4, ]} 5:  ${line.Token[5, ]} 6: ${line.Token[6, ]} 7: ${line.Token[7, ]}
  /if (${line.Find[tells you]}) {
    /if (${line.Token[6, ].Length}>0) /varset quantity ${line.Token[6, ]}
  } else /if (${line.Find[msg]}) {
    /if (${line.Token[4, ].Length}>0) /varset quantity ${line.Token[4, ]
  }
  /if (${Debug} || ${Debug_MAG}) /echo quantity is ${quantity}

  /call memPetGearSpells
  /declare requestorId int local ${Spawn[${ChatSender}].ID}

  /call TrueTarget ${requestorId}
  /if (${Target.ID} && ${Target.Distance}>100) {
    /tell ${ChatSender} You are too far away.
    /return
  }
  /tell ${ChatSender} Casting pet gear to you, stand close and prepare yourself for cursor spam.

  /declare i int local
  /for i 1 to ${quantity}
  /call castSimpleSpell ${petSpell_SpectralPlate} ${requestorId}
  /delay 2s
  /call castSimpleSpell ${petSpell_EnibiksHeirlooms} ${requestorId}
  /delay 2s
  /call castSimpleSpell ${petSpell_SpectralArm} ${requestorId}
  /delay 2s
  /next i
  /tell ${ChatSender} Finished casting gear, come again soon.

/return


#event makePetBags "#1# tells you, 'Make Pet Bag#'"
#event makePetBags "#1# tells you, 'Make Pet Bags #2#'"
#event makePetBags "<#1#(msg)> Make Pet Bag#"
#event makePetBags "<#1#(msg)> Make Pet Bags #2#"
sub event_makePetBags(line, ChatSender)
/if (${Debug} || ${Debug_MAG})  /echo event_makePetBags ==>
  /bc Making Pet Bags...
  /declare requestedPlates int local 1
  /declare requestedHeirlooms int local 1
  /declare requestedArms int local 1
  
  /if (${line.Find[tells you]}) {
    /if (${line.Token[6, ].Length}>0) /varset requestedPlates ${line.Token[6, ]}
    /if (${line.Token[7, ].Length}>0) /varset requestedHeirlooms ${line.Token[7, ]}
    /if (${line.Token[8, ].Length}>0) /varset requestedArms ${line.Token[8, ].Left[-1]}
  } else /if (${line.Find[msg]}) {
    /if (${line.Token[4, ].Length}>0) /varset requestedPlates ${line.Token[4, ]}
    /if (${line.Token[5, ].Length}>0) /varset requestedHeirlooms ${line.Token[5, ]}
    /if (${line.Token[6, ].Length}>0) /varset requestedArms ${line.Token[6, ].Left[-1]}
  }
  /bc plate: ${requestedPlates} looms: ${requestedHeirlooms} arms: ${requestedArms}
  
  /call makePetBags_Base ${requestedPlates} ${requestedHeirlooms} ${requestedArms}
  /echo finished making bags.
/if (${Debug} || ${Debug_MAG})  /echo <== event_makePetBags
/return


sub makePetBags_Base(armorBags, heirloomBags, weaponBags)
/if (${Debug} || ${Debug_MAG}) /echo makePetBags_Base: starting

  /echo armorBags: ${armorBags}, heirlooms: ${heirloomBags}, weapons: ${weaponBags}
  /call memPetGearSpells

  /declare i int local
  /for i 1 to 5
  /if (${i} <= ${armorBags}) {
    /call castSimpleSpell ${petSpell_SpectralPlate} ${Me.ID}
    /delay 3s ${Bool[${Cursor.ID}]}
    /call ClearCursor
  }

  /if (${i} <= ${heirloomBags}) {
    /call castSimpleSpell ${petSpell_EnibiksHeirlooms} ${Me.ID}
    /delay 3s ${Bool[${Cursor.ID}]}
    /call ClearCursor
  }

  /if (${i} <= ${weaponBags}) {
    /call castSimpleSpell ${petSpell_SpectralArm} ${Me.ID}
    /delay 3s ${Bool[${Cursor.ID}]}
    /call ClearCursor
  }
  /next i

/if (${Debug} || ${Debug_MAG}) /echo makePetBags_Base: finished.
/return


| This will mem the pet gear spells to slots assigned by the player in the mage ini. 
| It does not remem the original spells so they can keep casting pet gear.
| TODO - should make it optional to remem original spells
sub memPetGearSpells
/if (${Debug} || ${Debug_MAG}) /echo memPetGearSpells: starting

  /if (!${Defined[PetGearSpells]}) /call IniToArrayV "${Character_Ini},Magician,Pet Gear Spell#" PetGearSpells
  /echo ${PetGearSpells}
  /declare i int local
  /if (!${Defined[gearSpell]}) /declare gearSpell string local NULL
  /if (!${Defined[gearSpellGem]}) /declare gearSpellGem string local NULL
  
  /for i 1 to ${PetGearSpells.Size}
  
  /varset gearSpell ${PetGearSpells[${i}].Arg[1,|]}
  /varset gearSpellGem ${PetGearSpells[${i}].Arg[2,|]}
  /call memorize_spell "${gearSpell}" ${gearSpellGem}
  /if (${Debug} || ${Debug_MAG})  /echo memPetGearSpells: memorizing ${gearSpell} to gem ${gearSpellGem}
  /next i

/if (${Debug} || ${Debug_MAG})  /echo memPetGearSpells: finished
/return


sub armOnePet(playersName, loadoutSelected)

  /bc armOnePet: Working on ${playersName} 's pet with loadout ${loadoutSelected}
  /declare neededArms int local 0
  /declare neededPlates int local 0
  /declare neededHeirlooms int local 0

  /if (!(${FindItemCount[${petPack_SpectralPlate}]} > 0)) {
    /varset neededPlates 1
  }
  /if (!(${FindItemCount[${petPack_EnibiksHeirlooms}]} > 0)) {
    /varset neededHeirlooms 1
  }
  /if ((${FindItemCount[${petPack_SpectralArm}]} < 2)) {
    /varset neededArms 1
    /if ((${FindItemCount[${petPack_SpectralArm}]} == 0)) {
      /varset neededArms 2
    }
  }

  /call makePetBags_Base ${neededPlates} ${neededHeirlooms} ${neededArms} ${Me.Name}

  /call equipOnePet ${playersName} ${loadoutSelected}
  
/return 

| A simplified version of equipPet (the old arm pet function) which is only called when we detect pet below lvl 45.
| Just trying to provide an easy way for new folks to arm their pets.
sub armLowbiePet(chatSender)

    /if (!${Spawn[${chatSender}].Pet.ID} || ${Spawn[${chatSender}].Pet.Name.Find[familiar]} || !${Spawn[${chatSender}].Pet.Level}>1) /return
    /echo arming lower level pet
    /declare primaryWeapon string local
    /declare secondaryWeapon string local 

    /declare neededArms int local 0
    /declare neededPlates int local 0
    /declare neededHeirlooms int local 0
    /declare petId int local ${Spawn[${chatSender}].Pet.ID}

    /if (!(${FindItemCount[${petPack_SpectralPlate}]} > 0)) {
      /varset neededPlates 1
    }
    /if (!(${FindItemCount[${petPack_EnibiksHeirlooms}]} > 0)) {
      /varset neededHeirlooms 1
    }
    /if (!(${FindItemCount[${petPack_SpectralArm}]} > 0)) {
        /varset neededArms 1
    }

    /call makePetBags_Base ${neededPlates} ${neededHeirlooms} ${neededArms} ${Me.Name}

    /if (${FindItemCount[${petPack_EnibiksHeirlooms}]} > 0) {
      /bc Arm Lowbie Pet: Creating Heirlooms
      /call unpackAndGive ${petId} ${petPack_EnibiksHeirlooms}
    } 
    /if (${Cursor.ID}) {
      /bc Arm Lowbie Pet: Something went wrong and something is on your cursor, exiting.
      /return
    }
    /if (${FindItemCount[${petPack_SpectralPlate}]} > 0) {
      /bc Arm Lowbie Pet: Creating Spectral Plate
      /call unpackAndGive ${petId} ${petPack_SpectralPlate}
    }
    /if (${Cursor.ID}) {
      /bc Arm Lowbie Pet: Something went wrong and something is on your cursor, exiting.
      /return
    }

    /if (${FindItemCount[${pet_ClericHammer}]}>0) {
      /ItemNotify ${pet_ClericHammer} leftmouseup
      /delay 3s !${Bool[${Cursor.ID}]}
      /call targetMoveAndGive ${petId}
    } else {
      /delay 3s
      /tell ${chatSender} Pet ${Spawn[${chatSender}].Pet.CleanName} is below level 45, you should ask a cleric for a Pet Hammer when I am finished.
    }
 
    | Umfold pack for secondary
    /ItemNotify ${petPack_SpectralArm} rightmouseup
    /delay 3s ${Bool[${Cursor.ID}]}
    /call ClearCursor

    /if (${FindItemCount[${pet_DrainingBuckler}]}>0) {
      | for some reason itemnotify cant get the buckler by name or invslot, have to look for its pack slot
      /echo [${Time}]: Giving buckler to pet: ${pet_DrainingBuckler}
      /declare i int local
      /for i 1 to 10
      /if (${Bool[${InvSlot[pack${i}].Item.Name.Equal[${pet_QuelliousPouch}]}]}) {
        /itemnotify in pack${i} 5 leftmouseup
        /goto :pouchFound
      }
      /next i
      :pouchFound
      /delay 3s !${Bool[${Cursor.ID}]}
      |/echo giving secondary item to pet
      /call targetMoveAndGive ${petId}
    }

    |now delete the object
    /Itemnotify ${pet_QuelliousPouch} leftmouseup
    /delay 5s ${Bool[${Cursor.ID}]}
    /if (${Cursor.Name.Find[${pet_QuelliousPouch}]}) {
        /destroy
    } else {
      /echo [${Time}]: armLowbiePet: Could not destroy bag.
      /return
    }

/return


|----------------------------------------------------------------------------|

|#event petWeapons "#1# tells you, 'Pet Weapon#*#'"
#event petWeapons "#1# tells you, 'Distribute Pet Weapon#*#'"
#event petWeapons "<#1#> Pet Weapon#*#"
#event petWeapons "<#1#> Distribute Pet Weapon#*#"
sub event_petWeapons(line, ChatSender)
/if (${Debug} || ${Debug_MAG}) /echo |- event_petWeapons ==>
  /declare wasFollowing bool local ${Following}
  /varset Following FALSE
  /varset Debug_MAG FALSE
  /declare singlePet bool local TRUE
  /if (${Defined[petIDArray]}) /deletevar petIDArray
  /if (${Bool[${line.Find[Distribute]}]}) /varset singlePet FALSE

  /if (!${checkEventArgs[${ChatSender},${line},ZR]}) /return
  /if (!${Defined[petItems2D]}) {
    /if (${Debug} || ${Debug_MAG}) /echo I do not have any Pet Items defined...
    /if (${singlePet} && ${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} I do not have any Pet Items defined...
  } else /if (!${SpawnCount[${c_eventArgChatSender}]}) {
    /echo ${c_eventArgChatSender} is not in the zone
    /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} We are not in the same zone...
  } else /if (!${singlePet} && !${SpawnCount[pcpet radius 50]}) {
      /echo There are no PC Pets withing a radius of 50
      /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} No pets a radius of 50...
  } else /if (${singlePet} && ${Spawn[${c_eventArgChatSender}].Pet.Distance}>100) {
      /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Your pet is not in a radius of 100...
  } else {
    /if (!${singlePet} && ${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Handing out pet weapons...
    /declare originalLoc string local ${Me.Loc}
    /declare p int local
    |if distribute, give to everyone, else just give to the single chatsender
    /for p 1 to ${SpawnCount[pcpet radius 50]}
      /if (!${NearestSpawn[${p},pcpet].Name.Find[familiar]}) {
        |/echo ${p} ${NearestSpawn[${p},pcpet]} ${NearestSpawn[${p},pcpet].Name} ${NearestSpawn[${p},pcpet].Master} ${c_eventArgChatSender}
        |/echo los ${NearestSpawn[${p},pcpet].LineOfSight} d ${Bool[${line.Find[Distribute]}]}
        /if (${NearestSpawn[${p},pcpet].LineOfSight}) {
          /if (!${singlePet}) {
            /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
            |/echo all added ${NearestSpawn[${p},pcpet].Name}-${NearestSpawn[${p},pcpet].Master} ${petIDArray.Size}
          } else /if (${NearestSpawn[${p},pcpet].Master.CleanName.Equal[${c_eventArgChatSender}]}) {
            /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
            |/echo you added ${NearestSpawn[${p},pcpet].Name}-${NearestSpawn[${p},pcpet].Master} ${petIDArray.Size}
          }
        }
      }
    /next p
    /if (${Defined[petIDArray]}) {
      /if (${Stick.Active}) /squelch /stick off
      /if (${NetAdvPath.Status}==1) /squelch /netfollow off
      /call give_PetsWeapons FALSE
    }

    /if (${Math.Distance[${originalLoc}]}>20) /call MoveToLoc ${originalLoc} 50 20
    /if (${c_eventArgChatSender.NotEqual[${Me}]}) {
		/delay 1s
		/tell ${c_eventArgChatSender} Finished arming pets. If anything is missing, ask for it directly. Ex: "/tell ${Me.CleanName} Grant Spectral Plate".
	}
    /varset Following ${wasFollowing}
  }
  |/varset Debug_MAG FALSE
/if (${Debug} || ${Debug_MAG}) /echo <== event_petWeapons -|
/return


SUB give_PetsWeapons(autoWeap)
  /declare p int local
  /declare w int local
  /declare allWeapSuccess bool local
  /declare retryWeapTimer timer local 10s
  /declare DebugPetWeapons bool local false

  /for p 1 to ${petIDArray.Size}
    |/echo ${p}-${petIDArray[${p}]}-${Spawn[id ${petIDArray[${p}]}].Name}-${Spawn[id ${petIDArray[${p}]}].Master}
    /if (${SpawnCount[id ${petIDArray[${p}]}]}) {
      /varset allWeapSuccess TRUE
      /if (${Spawn[id ${petIDArray[${p}]}].Master.CleanName.NotEqual[${Me}]}) /tell ${Spawn[id ${petIDArray[${p}]}].Master} Incoming pet weapons, hold still!
      /call TrueTarget ${petIDArray[${p}]}
      /if (${Target.ID} && ${Target.Distance}>19) /call MoveToLoc ${Target.Y} ${Target.X} 50 15
        /for w 1 to ${petItems2D.Size[1]}
          :SummonWeap_Loop
          /if (${petItems2D[${w},${iCastName}].Equal["Blazing Stone of Demise"]} && ${FindItemCount[=Summoned: Burning Shank]}) {
            /call SwapItem "Summoned: Burning Shank" "Cursor"
          } else /if (${Bool[${FindItem[=${petItems2D[${w},${iCastName}]}]}]} && ${Bool[${FindItem[=${petItems2D[${w},${iCastName}]}].NoRent}]}) {
            /ItemNotify "${petItems2D[${w},${iCastName}]}" leftmouseup
          } else /if (${Bool[${FindItem[=Summoned: ${petItems2D[${w},${iCastName}]}]}]} && ${Bool[${FindItem[=Summoned: ${petItems2D[${w},${iCastName}]}].NoRent}]}) {
            /ItemNotify "Summoned: ${petItems2D[${w},${iCastName}]}" leftmouseup
          } else {
            /call check_Ready "petItems2D" ${w}
            /if (${c_Ready}) {
              /if (${check_Mana["petItems2D",${w}]}) {
				/if (${DebugPetWeapons}) /echo Item = ${petItems2D[${w},${iCastName}]}
				/if (${Select[${petItems2D[${w},${iCastName}]},Grant Spectral Plate]}) {
					/if (${Bool[${FindItem[=Folded Pack of Spectral Plate]}]}) {
						/ItemNotify "Folded Pack of Spectral Plate" rightmouseup
					} else {
						/if (${DebugPetWeapons}) /echo Grant Spectral Plate
						/tar ${Me}
						/delay 5s ${Select[${Target},${Me}]}
						/call e3_Cast ${Me.ID} "petItems2D" ${w}
						/delay 5s ${Cursor.ID}
						/autoinventory
						/if (${Bool[${FindItem[=Folded Pack of Spectral Plate]}]}) {
							/ItemNotify "Folded Pack of Spectral Plate" rightmouseup
						}
					}
				} else {
					/call e3_Cast ${Me.ID} "petItems2D" ${w}
				}
                /delay 5s ${Cursor.ID}
              }
            }
          }
          /if (!${Cursor.ID}) {
            /if (${retryWeapTimer}) {
              /goto :SummonWeap_Loop
            } else {
              /varset allWeapSuccess FALSE
            }
          } else /if (${Cursor.ID}) {
            /call TrueTarget ${petIDArray[${p}]}
            :OpenTrade_Loop
            /click left target
            /delay 1s ${Window[GiveWnd].Open}
            /if (!${Window[GiveWnd].Open}) {
              /if (${retryWeapTimer} && ${Cursor.ID}) {
                /goto :OpenTrade_Loop
              } else {
                /echo Failed to open trade with ${Target.CleanName}.
              }
            } else {
              :WaitAccept_Loop
			  /if (${c_eventArgChatSender.NotEqual[${Me}]}) {
				/tell ${Spawn[id ${petIDArray[${p}]}].Master} ---Giving "${petItems2D[${w},${iCastName}]}" to ${Spawn[id ${petIDArray[${p}]}].CleanName}.
			  }
              /notify GiveWnd GVW_Give_Button LeftMouseUp
              /delay 1s !${Window[GiveWnd].Open}
              /if (${Window[GiveWnd].Open}) {
                /if (${retryWeapTimer}) {
                  /goto :WaitAccept_Loop
                } else {
                  /echo Failed to open trade with ${Target.CleanName}.
                }
              }
            }
          }
        /next w
      /if (${autoWeap} && ${allWeapSuccess}) {
        /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Auto-Pet Weapons-${Spawn[id ${petIDArray[${p}]}].Master}" "${Spawn[id ${petIDArray[${p}]}].CleanName}" 1
      }
    }
  /next p
/return
|----------------------------------------------------------------------------|
SUB check_SummonItems
/if (${Debug} || ${Debug_MAG}) /echo |- check_SummonItems ==>
/if (!${Me.CombatState.Equal[COMBAT]}) {
  /if (${summonMoltenOrb}) {
    /if (!${Me.Moving} && !${combatTimer} && !${activeTimer} && !${rebuffTimer}) {
        /if (!${FindItemCount[=Molten Orb]}) {
          /call check_Ready "moltenOrb2D" 1
          /if (${c_Ready}) {
            /if (${check_Mana["moltenOrb2D",1]}) {
              /call e3_Cast ${Me.ID} "moltenOrb2D" 1
              /delay 2s ${Cursor.ID}
              /if (${Cursor.ID}) /call ClearCursor
            }
          }
        }
    }
  }

	/if (${autoWeapons} && ${Macro.RunTime} > 10) {
		/if (!${Me.Moving} && !${combatTimer} && !${activeTimer} && !${rebuffTimer}) {
			/declare i int local
      /declare p int local
      /declare masterName string local
      /declare petName string local
      /declare petRace string local
      /declare petID int local
      /if (${Defined[petIDArray]}) /deletevar petIDArray
      /for p 1 to ${SpawnCount[pcpet radius 100]}
        /varset masterName ${NearestSpawn[${p},pcpet].Master}
        /varset petName ${NearestSpawn[${p},pcpet].CleanName}
        /varset petID ${NearestSpawn[${p},pcpet].ID}
        /varset petRace ${NearestSpawn[${p},pcpet].Race}
        /if (!${Bool[${petName}]} || !${Bool[${masterName}]} || ${petName.Find[familiar]} || ${petRace.Equal[Gnomework]}) /return
        /if (${NetBots[${masterName}].ID}) {
          /if (${Spawn[id ${petID}].LineOfSight}) {
            /if (${petName.Equal[${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Auto-Pet Weapons-${masterName}]}]}) {
						  /if (${Debug} || ${Debug_MAG}) /echo check_SummonItems: I have already given weapons to ${petName}
					  } else {
              /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
					  }
				  }
		    }
		  /next p
		  /if (${Defined[petIDArray]}) {
        /call give_PetsWeapons TRUE
      }
	  }
	}
}
/if (${Debug} || ${Debug_MAG}) /echo <== check_SummonItems -|
/RETURN
|----------------------------------------------------------------------------|
#event auto_coh "#1# tells you, 'autocoh #2#'"
#event auto_coh "<#1#> autocoh #2#"
SUB event_auto_coh(line, ChatSender, toggleCoH)
  /declare i int local
  /if (${toggleCoH.Equal[on]}) {
    /if (${Bool[${Group.Member[${ChatSender}]}]}) {
      /if (${Defined[cohTargets]}) {
        /for i 1 to ${cohTargets.Size}
          /if (${cohTargets[${i}].Equal[${ChatSender}]}) {
            /docommand ${ChatToggle} ${ChatSender} is already in the AutoCoH array
            /return
          }
        /next i
      }
      /call BuildArray cohTargets "${ChatSender}"
      /docommand ${ChatToggle} Auto CoH Enabled on ${ChatSender}
    } else {
      /docommand ${ChatToggle} Auto CoH will not be enabled until you are in a group with this mage
    }
  } else {
    /if (${Defined[cohTargets]}) /call RemoveArrayElement cohTargets "${ChatSender}"
    /docommand ${ChatToggle} Auto CoH Disabled on ${ChatSender}
  }
/RETURN
|----------------------------------------------------------------------------|
SUB check_CoH
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_checkCoH ==>
  /if (${Defined[cohTargets]}) {
	/if (${Debug_MAG}) /echo cohTargets=${cohTargets.Size}
    /declare t int local
    /declare s int local
    /for t 1 to ${cohTargets.Size}
      /if (${Bool[${Group.Member[${cohTargets[${t}]}]}]}) {
        /for s 1 to ${cohSpells2D.Size[1]}     
		  /if (${Debug_MAG}) /echo Checking ${cohTargets[${t}]}
		  /if (${Debug_MAG}) /echo Distance ${Spawn[pc =${cohTargets[${t}]}].Distance}
		  /if (${Debug_MAG}) /echo check_Mana ${check_Mana["cohSpells2D",${s}]}
            /if (${Spawn[pc =${cohTargets[${t}]}].Distance} >= ${coh_dist}) {
			  /call check_Ready "cohSpells2D" ${s}
			  /if (${c_Ready}) {
                /if (${check_Mana["cohSpells2D",${s}]}) {
                  /docommand ${ChatToggle} << Auto CoH casting on ${cohTargets[${t}]} >>
                  /g << Auto CoH casting on ${cohTargets[${t}]} >>
                  /call e3_cast ${Spawn[pc =${cohTargets[${t}]}].ID} "cohSpells2D" ${s}
                } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH CoH is not ready for ${cohTargets[${t}]}  >>
              } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH CoH is not ready for ${cohTargets[${t}]}  >>
			} else {
				/echo AutoCoh complete on ${cohTargets[${t}]}
				/call RemoveArrayElement cohTargets "${cohTargets[${t}]}"				
			}
        /if (!${ActionTaken})  /next s
      } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH ${cohTargets[${t}]} is not in my group >>
    /if (!${ActionTaken})  /next t
  }
  /if (${Debug} || ${Debug_MAG}) /echo <== MAG_checkCoH -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Setup
/declare Debug_MAG bool outer FALSE
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_Setup ==>
  /declare coh_dist int outer 150
	/call iniToVarV "${Character_Ini},Magician,Auto-Pet Weapons (On/Off)" autoWeapons bool outer
	/call IniToArrayV "${Character_Ini},Magician,Summoned Pet Item#" petItems
  /call iniToVarV "${Character_Ini},Magician,Auto-Summon Orb of Mastery (On/Off)" autoOrbOfMastery bool outer

	/declare mageRangedSwap bool outer FALSE
	/if (${Defined[petItems]}) /call BuildSpellArray "petItems"	"petItems2D"

  /if (${Bool[${Me.Book[Call of the Hero]}]}) {
    /declare cohSpells[1] string outer Call of the Hero
    /call BuildSpellArray "cohSpells" "cohSpells2D"
  }

  /if (${Bool[${Me.Book[Summon: Molten Orb]}]}) {
    /declare moltenOrb[1] string outer Summon: Molten Orb
    /call BuildSpellArray "moltenOrb" "moltenOrb2D"
  }

  | Orb of Mastery must be enabled and the toon must have the spell
  /if (${autoOrbOfMastery} && ${Bool[${Me.Book[Summon Orb]}]}) {
    /varset autoOrbOfMastery TRUE
  } else {
    /varset autoOrbOfMastery FALSE
  }

/if (${Debug} || ${Debug_MAG}) /echo <== MAG_Setup -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_MacroSettings
/if (${Debug}) /echo |- MAG_MacroSettings ==>
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#1" "check_CoH" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#2" "check_Buffs" 1
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#3" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#4" "check_Pets" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#5" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#6" "check_Ae" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#7" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#8" "check_SummonItems" 1
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#9" "check_OrbOfMasteryCharges" 1
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#10" "check_Food" 1
/if (${Debug}) /echo <== MAG_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_CharacterSettings
/if (${Debug}) /echo |- MAG_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},Magician,Auto-Pet Weapons (On/Off)"
  /call WriteToIni "${Character_Ini},Magician,Auto-Summon Orb of Mastery (On/Off)"
	/call WriteToIni "${Character_Ini},Magician,Summoned Pet Item"
  /call WriteToIni "${Character_Ini},Magician,Pet Gear Spell"
/if (${Debug}) /echo <== MAG_CharacterSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Aliases
/if (${Debug}) /echo |- MAG_Aliases ==>
	/squelch /alias /dpw /bc Distribute Pet Weapons
	/squelch /alias /PetWeapons /bc Distribute Pet Weapons
/if (${Debug}) /echo <== MAG_Aliases -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Background_Events
|**
	/if (${Bool[${Me.AltAbility[Companion of Necessity]}]} && ${Me.AltAbilityReady[Companion of Necessity]}) {
		/if (${Assisting} && ${Me.CombatState.Equal[Combat]} && ${Target.Named} && ${Target.Level} >= ${Math.Calc[${Me.Level}+3]} && !${Select[${Me.TargetOfTarget.Class},WAR,PAL,SHD]} && ${Spawn[${Target.ID}].PlayerState}==4) {
			/if (${Me.Casting.ID}) /call interrupt
			/delay 1s !${Me.Casting.ID}
			/gu ${Target.Name} has aggro on ${Me.TargetOfTarget.Name}, casting Companion of Necessity
			/casting "Companion of Necessity"
		}
	}
**|
  /doevents armPets
  /doevents auto_coh
  /doevents petGear
  /doevents makePetBags
  /doevents petWeapons
/RETURN

|----------------------------------------------------------------------------|
| Destroy Orb of Mastery when it's out of charges and immediately summon a new one
| To enable:
| Add to MAG's Advanced Settings.ini
| [Magician]
| Auto-Summon Orb of Mastery (On/Off)=On
|----------------------------------------------------------------------------|
SUB check_OrbOfMasteryCharges 
  /if (!${autoOrbOfMastery}) /return 
  | Nothing on the cursor
  /if (!${Assisting} && !${Me.CombatState.Equal[Combat]} && !${Bool[${Cursor.ID}]}) {
    | if orb of mastery has 0 charges
    /if (${Bool[${FindItem[=Orb of Mastery]}]} && ${FindItem[=Orb of Mastery].Charges} == 0 ) {
      /docommand ${ChatToggle} Orb of Mastery is out of charges, destroying it
      /call SwapItem "Orb of Mastery" "Cursor"
      | double check that the item on the cursor is the Orb of Mastery
      /if (${Cursor.Name.Equal[Orb of Mastery]}) {
        /destroy
      }
    }
    | if toon has no orb of mastery
    /if (!${Bool[${FindItem[=Orb of Mastery]}]}) {
      /docommand ${ChatToggle} No Orb of Mastery in inventory, summoning an Orb of Mastery
      /call castSimpleSpell "Summon Orb" 0
      /delay 12s ${Cursor.ID}
      /while (${Cursor.ID}) {
        /autoinv
        /delay 1s
      }
    }    
  }  
/RETURN
