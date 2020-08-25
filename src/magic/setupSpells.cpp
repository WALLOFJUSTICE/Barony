/*-------------------------------------------------------------------------------

	BARONY
	File: spell.cpp
	Desc: contains spell definitions

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../main.hpp"
#include "../game.hpp"
#include "../stat.hpp"
#include "magic.hpp"

std::vector<spell_t*> allGameSpells;

void setupSpells()   ///TODO: Verify this function.
{
	node_t* node = NULL;
	spellElement_t* element = NULL;

	spellElementConstructor(&spellElement_unintelligible);
	spellElement_unintelligible.mana = 0;
	spellElement_unintelligible.base_mana = 0;
	spellElement_unintelligible.overload_multiplier = 0; //NOTE: Might crash due to divide by zero?
	spellElement_unintelligible.damage = 0;
	spellElement_unintelligible.duration = 0;
	spellElement_unintelligible.can_be_learned = false;
	strcpy(spellElement_unintelligible.name, language[413]);

	spellElementConstructor(&spellElement_missile);
	spellElement_missile.mana = 1;
	spellElement_missile.base_mana = 1;
	spellElement_missile.overload_multiplier = 1;
	spellElement_missile.damage = 0;
	spellElement_missile.duration = 75; //1.25 seconds.
	//spellElement_missile.name = "Missile";
	strcpy(spellElement_missile.name, language[414]);

	spellElementConstructor(&spellElement_force);
	spellElement_force.mana = 4;
	spellElement_force.base_mana = 4;
	spellElement_force.overload_multiplier = 1;
	spellElement_force.damage = 15;
	spellElement_force.duration = 0;
	strcpy(spellElement_force.name, language[415]);

	spellElementConstructor(&spellElement_fire);
	spellElement_fire.mana = 6;
	spellElement_fire.base_mana = 6;
	spellElement_fire.overload_multiplier = 1;
	spellElement_fire.damage = 25;
	spellElement_fire.duration = 0;
	strcpy(spellElement_fire.name, language[416]);

	spellElementConstructor(&spellElement_lightning);
	spellElement_lightning.mana = 5;
	spellElement_lightning.base_mana = 5;
	spellElement_lightning.overload_multiplier = 1;
	spellElement_lightning.damage = 25;
	spellElement_lightning.duration = 75;
	strcpy(spellElement_lightning.name, language[417]);

	spellElementConstructor(&spellElement_light);
	spellElement_light.mana = 1;
	spellElement_light.base_mana = 1;
	spellElement_light.overload_multiplier = 1;
	spellElement_light.damage = 0;
	spellElement_light.duration = 750; //500 a better value? //NOTE: 750 is original value.
	strcpy(spellElement_light.name, language[418]);

	spellElementConstructor(&spellElement_dig);
	spellElement_dig.mana = 20;
	spellElement_dig.base_mana = 20;
	spellElement_dig.overload_multiplier = 1;
	spellElement_dig.damage = 0;
	spellElement_dig.duration = 0;
	strcpy(spellElement_dig.name, language[419]);

	spellElementConstructor(&spellElement_invisible);
	spellElement_invisible.mana = 2;
	spellElement_invisible.base_mana = 2;
	spellElement_invisible.overload_multiplier = 1;
	spellElement_invisible.damage = 0;
	spellElement_invisible.duration = 100;
	strcpy(spellElement_invisible.name, language[420]);

	spellElementConstructor(&spellElement_identify);
	spellElement_identify.mana = 10;
	spellElement_identify.base_mana = 10;
	spellElement_identify.overload_multiplier = 0; //NOTE: Might segfault due to divide by zero?
	spellElement_identify.damage = 0;
	spellElement_identify.duration = 0;
	strcpy(spellElement_identify.name, language[421]);

	spellElementConstructor(&spellElement_magicmapping);
	spellElement_magicmapping.mana = 40;
	spellElement_magicmapping.base_mana = 40;
	spellElement_magicmapping.overload_multiplier = 0; //NOTE: Might segfault due to divide by zero?
	spellElement_magicmapping.damage = 0;
	spellElement_magicmapping.duration = 0;
	strcpy(spellElement_magicmapping.name, language[422]);

	spellElementConstructor(&spellElement_heal);
	spellElement_heal.mana = 1;
	spellElement_heal.base_mana = 1;
	spellElement_heal.overload_multiplier = 1;
	spellElement_heal.damage = 1;
	spellElement_heal.duration = 0;
	strcpy(spellElement_heal.name, language[423]);

	spellElementConstructor(&spellElement_confuse);
	spellElement_confuse.mana = 4;
	spellElement_confuse.base_mana = 4;
	spellElement_confuse.overload_multiplier = 1;
	spellElement_confuse.damage = 0;
	spellElement_confuse.duration = TICKS_PER_SECOND * SPELLELEMENT_CONFUSE_BASE_DURATION; //TODO: Decide on something.
	strcpy(spellElement_confuse.name, language[424]);

	spellElementConstructor(&spellElement_cure_ailment);
	spellElement_cure_ailment.mana = 10;
	spellElement_cure_ailment.base_mana = 10;
	spellElement_cure_ailment.overload_multiplier = 0;
	spellElement_cure_ailment.damage = 0;
	spellElement_cure_ailment.duration = 0;
	strcpy(spellElement_cure_ailment.name, language[425]);

	spellElementConstructor(&spellElement_locking);
	spellElement_locking.mana = 10;
	spellElement_locking.base_mana = 10;
	spellElement_locking.overload_multiplier = 0;
	spellElement_locking.damage = 0;
	spellElement_locking.duration = 0;
	strcpy(spellElement_locking.name, language[426]);

	spellElementConstructor(&spellElement_opening);
	spellElement_opening.mana = 5;
	spellElement_opening.base_mana = 5;
	spellElement_opening.overload_multiplier = 0;
	spellElement_opening.damage = 0;
	spellElement_opening.duration = 0;
	strcpy(spellElement_opening.name, language[427]);

	spellElementConstructor(&spellElement_sleep);
	spellElement_sleep.mana = 3;
	spellElement_sleep.base_mana = 3;
	spellElement_sleep.overload_multiplier = 0;
	spellElement_sleep.damage = 0;
	spellElement_sleep.duration = 0;
	strcpy(spellElement_sleep.name, language[428]);

	spellElementConstructor(&spellElement_cold);
	spellElement_cold.mana = 5;
	spellElement_cold.base_mana = 5;
	spellElement_cold.overload_multiplier = 1;
	spellElement_cold.damage = 20;
	spellElement_cold.duration = 180;
	strcpy(spellElement_cold.name, language[429]);

	spellElementConstructor(&spellElement_slow);
	spellElement_slow.mana = 3;
	spellElement_slow.base_mana = 3;
	spellElement_slow.overload_multiplier = 1;
	spellElement_slow.damage = 0;
	spellElement_slow.duration = 180;
	strcpy(spellElement_slow.name, language[430]);

	spellElementConstructor(&spellElement_levitation);
	spellElement_levitation.mana = 1;
	spellElement_levitation.base_mana = 1;
	spellElement_levitation.overload_multiplier = 1;
	spellElement_levitation.damage = 0;
	spellElement_levitation.duration = 30;
	strcpy(spellElement_levitation.name, language[431]);

	spellElementConstructor(&spellElement_teleportation);
	spellElement_teleportation.mana = 20;
	spellElement_teleportation.base_mana = 20;
	spellElement_teleportation.overload_multiplier = 0;
	spellElement_teleportation.damage = 0;
	spellElement_teleportation.duration = 0;
	strcpy(spellElement_teleportation.name, language[432]);

	spellElementConstructor(&spellElement_selfPolymorph);
	spellElement_selfPolymorph.mana = 40;
	spellElement_selfPolymorph.base_mana = 40;
	spellElement_selfPolymorph.overload_multiplier = 0;
	spellElement_selfPolymorph.damage = 0;
	spellElement_selfPolymorph.duration = 0;
	strcpy(spellElement_selfPolymorph.name, language[3885]);

	spellElementConstructor(&spellElement_magicmissile);
	spellElement_magicmissile.mana = 6;
	spellElement_magicmissile.base_mana = 6;
	spellElement_magicmissile.overload_multiplier = 1;
	spellElement_magicmissile.damage = 30;
	spellElement_magicmissile.duration = 0;
	strcpy(spellElement_magicmissile.name, language[433]);

	spellElementConstructor(&spellElement_removecurse);
	spellElement_removecurse.mana = 20;
	spellElement_removecurse.base_mana = 20;
	spellElement_removecurse.overload_multiplier = 0;
	spellElement_removecurse.damage = 0;
	spellElement_removecurse.duration = 0;
	strcpy(spellElement_removecurse.name, language[434]);

	spellElementConstructor(&spellElement_summon);
	spellElement_summon.mana = 17;
	spellElement_summon.base_mana = 17;
	spellElement_summon.overload_multiplier = 1;
	spellElement_summon.damage = 0;
	spellElement_summon.duration = 0;
	strcpy(spellElement_summon.name, language[2390]);

	spellElementConstructor(&spellElement_stoneblood);
	spellElement_stoneblood.mana = 20;
	spellElement_stoneblood.base_mana = 20;
	spellElement_stoneblood.overload_multiplier = 1;
	spellElement_stoneblood.damage = 0;
	spellElement_stoneblood.duration = TICKS_PER_SECOND * SPELLELEMENT_STONEBLOOD_BASE_DURATION;
	strcpy(spellElement_stoneblood.name, language[2391]);

	spellElementConstructor(&spellElement_bleed);
	spellElement_bleed.mana = 10;
	spellElement_bleed.base_mana = 10;
	spellElement_bleed.overload_multiplier = 1;
	spellElement_bleed.damage = 30;
	spellElement_bleed.duration = TICKS_PER_SECOND * SPELLELEMENT_BLEED_BASE_DURATION; //TODO: Decide on something.;
	strcpy(spellElement_bleed.name, language[2392]);

	spellElementConstructor(&spellElement_missile_trio);
	spellElement_missile_trio.mana = 1;
	spellElement_missile_trio.base_mana = 1;
	spellElement_missile_trio.overload_multiplier = 1;
	spellElement_missile_trio.damage = 0;
	spellElement_missile_trio.duration = 25; //1 second.
	strcpy(spellElement_missile_trio.name, language[2426]);

	spellElementConstructor(&spellElement_dominate);
	spellElement_dominate.mana = 20;
	spellElement_dominate.base_mana = 20;
	spellElement_dominate.overload_multiplier = 1;
	spellElement_dominate.damage = 0;
	spellElement_dominate.duration = 0;
	strcpy(spellElement_dominate.name, language[2393]);

	spellElementConstructor(&spellElement_reflectMagic);
	spellElement_reflectMagic.mana = 10;
	spellElement_reflectMagic.base_mana = 10;
	spellElement_reflectMagic.overload_multiplier = 1;
	spellElement_reflectMagic.damage = 0;
	spellElement_reflectMagic.duration = 3000;
	strcpy(spellElement_reflectMagic.name, language[2394]);

	spellElementConstructor(&spellElement_acidSpray);
	spellElement_acidSpray.mana = 10;
	spellElement_acidSpray.base_mana = 10;
	spellElement_acidSpray.overload_multiplier = 1;
	spellElement_acidSpray.damage = 10;
	spellElement_acidSpray.duration = TICKS_PER_SECOND * SPELLELEMENT_ACIDSPRAY_BASE_DURATION; //TODO: Decide on something.;
	strcpy(spellElement_acidSpray.name, language[2395]);

	spellElementConstructor(&spellElement_stealWeapon);
	spellElement_stealWeapon.mana = 50;
	spellElement_stealWeapon.base_mana = 50;
	spellElement_stealWeapon.overload_multiplier = 1;
	spellElement_stealWeapon.damage = 0;
	spellElement_stealWeapon.duration = 0;
	strcpy(spellElement_stealWeapon.name, language[2396]);

	spellElementConstructor(&spellElement_drainSoul);
	spellElement_drainSoul.mana = 17;
	spellElement_drainSoul.base_mana = 17;
	spellElement_drainSoul.overload_multiplier = 1;
	spellElement_drainSoul.damage = 18;
	spellElement_drainSoul.duration = 0;
	strcpy(spellElement_drainSoul.name, language[2397]);

	spellElementConstructor(&spellElement_vampiricAura);
	spellElement_vampiricAura.mana = 5;
	spellElement_vampiricAura.base_mana = 5;
	spellElement_vampiricAura.overload_multiplier = 1;
	spellElement_vampiricAura.damage = 0;
	spellElement_vampiricAura.duration = 85; //TODO: Decide on something.
	strcpy(spellElement_vampiricAura.name, language[2398]);

	spellElementConstructor(&spellElement_amplifyMagic);
	spellElement_amplifyMagic.mana = 7;
	spellElement_amplifyMagic.base_mana = 7;
	spellElement_amplifyMagic.overload_multiplier = 1;
	spellElement_amplifyMagic.damage = 0;
	spellElement_amplifyMagic.duration = 85; //TODO: Decide on something.
	strcpy(spellElement_amplifyMagic.name, language[3440]);

	spellElementConstructor(&spellElement_charmMonster);
	spellElement_charmMonster.mana = 49;
	spellElement_charmMonster.base_mana = 49;
	spellElement_charmMonster.overload_multiplier = 1;
	spellElement_charmMonster.damage = 0;
	spellElement_charmMonster.duration = 300;
	strcpy(spellElement_charmMonster.name, language[2399]);

	spellElementConstructor(&spellElement_shapeshift);
	spellElement_shapeshift.mana = 1;
	spellElement_shapeshift.base_mana = 1;
	spellElement_shapeshift.overload_multiplier = 1;
	spellElement_shapeshift.damage = 0;
	spellElement_shapeshift.duration = 0;
	strcpy(spellElement_shapeshift.name, language[3407]);

	spellElementConstructor(&spellElement_sprayWeb);
	spellElement_sprayWeb.mana = 7;
	spellElement_sprayWeb.base_mana = 7;
	spellElement_sprayWeb.overload_multiplier = 1;
	spellElement_sprayWeb.damage = 0;
	spellElement_sprayWeb.duration = 0;
	strcpy(spellElement_sprayWeb.name, language[3412]);

	spellElementConstructor(&spellElement_poison);
	spellElement_poison.mana = 4;
	spellElement_poison.base_mana = 4;
	spellElement_poison.overload_multiplier = 1;
	spellElement_poison.damage = 10;
	spellElement_poison.duration = 0;
	strcpy(spellElement_poison.name, language[3413]);

	spellElementConstructor(&spellElement_speed);
	spellElement_speed.mana = 11;
	spellElement_speed.base_mana = 11;
	spellElement_speed.overload_multiplier = 1;
	spellElement_speed.damage = 0;
	spellElement_speed.duration = 30 * TICKS_PER_SECOND;
	strcpy(spellElement_speed.name, language[3414]);

	spellElementConstructor(&spellElement_fear);
	spellElement_fear.mana = 28;
	spellElement_fear.base_mana = 28;
	spellElement_fear.overload_multiplier = 1;
	spellElement_fear.damage = 0;
	spellElement_fear.duration = 0;
	strcpy(spellElement_fear.name, language[3415]);

	spellElementConstructor(&spellElement_strike);
	spellElement_strike.mana = 23;
	spellElement_strike.base_mana = 23;
	spellElement_strike.overload_multiplier = 1;
	spellElement_strike.damage = 1;
	spellElement_strike.duration = 0;
	strcpy(spellElement_strike.name, language[3416]);

	spellElementConstructor(&spellElement_weakness);
	spellElement_weakness.mana = 1;
	spellElement_weakness.base_mana = 1;
	spellElement_weakness.overload_multiplier = 1;
	spellElement_weakness.damage = 0;
	spellElement_weakness.duration = 0;
	strcpy(spellElement_weakness.name, language[3422]);

	spellElementConstructor(&spellElement_detectFood);
	spellElement_detectFood.mana = 14;
	spellElement_detectFood.base_mana = 14;
	spellElement_detectFood.overload_multiplier = 0; //NOTE: Might segfault due to divide by zero?
	spellElement_detectFood.damage = 0;
	spellElement_detectFood.duration = 0;
	strcpy(spellElement_detectFood.name, language[3421]);

	spellElementConstructor(&spellElement_trollsBlood);
	spellElement_trollsBlood.mana = 25;
	spellElement_trollsBlood.base_mana = 25;
	spellElement_trollsBlood.overload_multiplier = 0; //NOTE: Might segfault due to divide by zero?
	spellElement_trollsBlood.damage = 0;
	spellElement_trollsBlood.duration = 80 * TICKS_PER_SECOND;
	strcpy(spellElement_trollsBlood.name, language[3489]);

	spellElementConstructor(&spellElement_flutter);
	spellElement_flutter.mana = 10;
	spellElement_flutter.base_mana = 10;
	spellElement_flutter.overload_multiplier = 1;
	spellElement_flutter.damage = 0;
	spellElement_flutter.duration = 6 * TICKS_PER_SECOND;
	strcpy(spellElement_flutter.name, language[3764]);

	spellElementConstructor(&spellElement_dash);
	spellElement_dash.mana = 5;
	spellElement_dash.base_mana = 5;
	spellElement_dash.overload_multiplier = 1;
	spellElement_dash.damage = 0;
	spellElement_dash.duration = 1 * TICKS_PER_SECOND;
	strcpy(spellElement_dash.name, language[3765]);

	spellElementConstructor(&spellElement_salvageItem);
	spellElement_salvageItem.mana = 6;
	spellElement_salvageItem.base_mana = 6;
	spellElement_salvageItem.overload_multiplier = 0; //NOTE: Might segfault due to divide by zero?
	spellElement_salvageItem.damage = 0;
	spellElement_salvageItem.duration = 0;
	strcpy(spellElement_salvageItem.name, language[3711]);

	spellElementConstructor(&spellElement_shadowTag);
	spellElement_shadowTag.mana = 4;
	spellElement_shadowTag.base_mana = 4;
	spellElement_shadowTag.overload_multiplier = 1;
	spellElement_shadowTag.damage = 0;
	spellElement_shadowTag.duration = 0;
	strcpy(spellElement_shadowTag.name, language[3447]);

	spellElementConstructor(&spellElement_telePull);
	spellElement_telePull.mana = 19;
	spellElement_telePull.base_mana = 19;
	spellElement_telePull.overload_multiplier = 1;
	spellElement_telePull.damage = 0;
	spellElement_telePull.duration = 0;
	strcpy(spellElement_telePull.name, language[3448]);

	spellElementConstructor(&spellElement_demonIllusion);
	spellElement_demonIllusion.mana = 24;
	spellElement_demonIllusion.base_mana = 24;
	spellElement_demonIllusion.overload_multiplier = 1;
	spellElement_demonIllusion.damage = 0;
	spellElement_demonIllusion.duration = 0;
	strcpy(spellElement_demonIllusion.name, language[3449]);

	spellConstructor(&spell_forcebolt);
	strcpy(spell_forcebolt.name, language[415]);
	spell_forcebolt.ID = SPELL_FORCEBOLT;
	spell_forcebolt.difficulty = 0;
	node = list_AddNodeLast(&spell_forcebolt.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_force);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_magicmissile);
	strcpy(spell_magicmissile.name, language[433]);
	spell_magicmissile.ID = SPELL_MAGICMISSILE;
	spell_magicmissile.difficulty = 60;
	node = list_AddNodeLast(&spell_magicmissile.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_magicmissile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_cold);
	strcpy(spell_cold.name, language[429]);
	spell_cold.ID = SPELL_COLD;
	spell_cold.difficulty = 40;
	node = list_AddNodeLast(&spell_cold.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_cold);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_fireball);
	strcpy(spell_fireball.name, language[416]);
	spell_fireball.ID = SPELL_FIREBALL;
	spell_fireball.difficulty = 20;
	spell_fireball.elements.first = NULL;
	spell_fireball.elements.last = NULL;
	node = list_AddNodeLast(&spell_fireball.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_fire);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_lightning);
	strcpy(spell_lightning.name, language[417]);
	spell_lightning.ID = SPELL_LIGHTNING;
	spell_lightning.difficulty = 60;
	spell_lightning.elements.first = NULL;
	spell_lightning.elements.last = NULL;
	node = list_AddNodeLast(&spell_lightning.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_lightning);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_removecurse);
	strcpy(spell_removecurse.name, language[434]);
	spell_removecurse.ID = SPELL_REMOVECURSE;
	spell_removecurse.difficulty = 60;
	spell_removecurse.elements.first = NULL;
	spell_removecurse.elements.last = NULL;
	node = list_AddNodeLast(&spell_removecurse.elements);
	node->element = copySpellElement(&spellElement_removecurse);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.

	spellConstructor(&spell_light);
	strcpy(spell_light.name, language[418]);
	spell_light.ID = SPELL_LIGHT;
	spell_light.difficulty = 0;
	spell_light.elements.first = NULL;
	spell_light.elements.last = NULL;
	node = list_AddNodeLast(&spell_light.elements);
	node->element = copySpellElement(&spellElement_light);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	element->channeled = true;

	spellConstructor(&spell_identify);
	strcpy(spell_identify.name, language[421]);
	spell_identify.ID = SPELL_IDENTIFY;
	spell_identify.difficulty = 60;
	spell_identify.elements.first = NULL;
	spell_identify.elements.last = NULL;
	node = list_AddNodeLast(&spell_identify.elements);
	node->element = copySpellElement(&spellElement_identify);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.

	spellConstructor(&spell_magicmapping);
	strcpy(spell_magicmapping.name, language[435]);
	spell_magicmapping.ID = SPELL_MAGICMAPPING;
	spell_magicmapping.difficulty = 60;
	spell_magicmapping.elements.first = NULL;
	spell_magicmapping.elements.last = NULL;
	node = list_AddNodeLast(&spell_magicmapping.elements);
	node->element = copySpellElement(&spellElement_magicmapping);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_sleep);
	strcpy(spell_sleep.name, language[428]);
	spell_sleep.ID = SPELL_SLEEP;
	spell_sleep.difficulty = 20;
	node = list_AddNodeLast(&spell_sleep.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_sleep);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_confuse);
	strcpy(spell_confuse.name, language[424]);
	spell_confuse.ID = SPELL_CONFUSE;
	spell_confuse.difficulty = 20;
	node = list_AddNodeLast(&spell_confuse.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_confuse);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;
	element->mana = 15; //Set the spell's mana to 15 so that it lasts ~30 seconds.

	spellConstructor(&spell_slow);
	strcpy(spell_slow.name, language[430]);
	spell_slow.ID = SPELL_SLOW;
	spell_slow.difficulty = 20;
	node = list_AddNodeLast(&spell_slow.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_slow);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_opening);
	strcpy(spell_opening.name, language[427]);
	spell_opening.ID = SPELL_OPENING;
	spell_opening.difficulty = 20;
	node = list_AddNodeLast(&spell_opening.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_opening);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_locking);
	strcpy(spell_locking.name, language[426]);
	spell_locking.ID = SPELL_LOCKING;
	spell_locking.difficulty = 20;
	node = list_AddNodeLast(&spell_locking.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_locking);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_levitation);
	strcpy(spell_levitation.name, language[431]);
	spell_levitation.ID = SPELL_LEVITATION;
	spell_levitation.difficulty = 80;
	spell_levitation.elements.first = NULL;
	spell_levitation.elements.last = NULL;
	node = list_AddNodeLast(&spell_levitation.elements);
	node->element = copySpellElement(&spellElement_levitation);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	element->channeled = true;

	spellConstructor(&spell_invisibility);
	strcpy(spell_invisibility.name, language[420]);
	spell_invisibility.ID = SPELL_INVISIBILITY;
	spell_invisibility.difficulty = 80;
	spell_invisibility.elements.first = NULL;
	spell_invisibility.elements.last = NULL;
	node = list_AddNodeLast(&spell_invisibility.elements);
	node->element = copySpellElement(&spellElement_invisible);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	element->channeled = true;

	spellConstructor(&spell_teleportation);
	strcpy(spell_teleportation.name, language[432]);
	spell_teleportation.ID = SPELL_TELEPORTATION;
	spell_teleportation.difficulty = 80;
	spell_teleportation.elements.first = NULL;
	spell_teleportation.elements.last = NULL;
	node = list_AddNodeLast(&spell_teleportation.elements);
	node->element = copySpellElement(&spellElement_teleportation);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.

	spellConstructor(&spell_polymorph);
	strcpy(spell_polymorph.name, language[3885]);
	spell_polymorph.ID = SPELL_SELF_POLYMORPH;
	spell_polymorph.difficulty = 60;
	spell_polymorph.elements.first = NULL;
	spell_polymorph.elements.last = NULL;
	node = list_AddNodeLast(&spell_polymorph.elements);
	node->element = copySpellElement(&spellElement_selfPolymorph);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.

	spellConstructor(&spell_healing);
	strcpy(spell_healing.name, language[423]);
	spell_healing.ID = SPELL_HEALING;
	spell_healing.difficulty = 20;
	spell_healing.elements.first = NULL;
	spell_healing.elements.last = NULL;
	node = list_AddNodeLast(&spell_healing.elements);
	node->element = copySpellElement(&spellElement_heal);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;
	element->mana = 10;

	spellConstructor(&spell_extrahealing);
	strcpy(spell_extrahealing.name, language[438]);
	spell_extrahealing.ID = SPELL_EXTRAHEALING;
	spell_extrahealing.difficulty = 60;
	spell_extrahealing.elements.first = NULL;
	spell_extrahealing.elements.last = NULL;
	node = list_AddNodeLast(&spell_extrahealing.elements);
	node->element = copySpellElement(&spellElement_heal);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;
	element->mana = 40;

	/*spellConstructor(&spell_restoreability);
	//spell_restoreability.mana = 1;
	strcpy(spell_restoreability.name, "Restore Ability");
	spell_restoreability.ID = SPELL_RESTOREABILITY;
	spell_restoreability.difficulty = 100; //Basically unlearnable (since unimplemented).*/

	spellConstructor(&spell_cureailment);
	strcpy(spell_cureailment.name, language[425]);
	spell_cureailment.ID = SPELL_CUREAILMENT;
	spell_cureailment.difficulty = 20;
	spell_cureailment.elements.first = NULL;
	spell_cureailment.elements.last = NULL;
	node = list_AddNodeLast(&spell_cureailment.elements);
	node->element = copySpellElement(&spellElement_cure_ailment);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_dig);
	strcpy(spell_dig.name, "Dig");
	spell_dig.ID = SPELL_DIG;
	spell_dig.difficulty = 40;
	spell_dig.elements.first = NULL;
	spell_dig.elements.last = NULL;
	node = list_AddNodeLast(&spell_dig.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_dig);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_stoneblood);
	strcpy(spell_stoneblood.name, language[2391]);
	spell_stoneblood.ID = SPELL_STONEBLOOD;
	spell_stoneblood.difficulty = 80;
	node = list_AddNodeLast(&spell_stoneblood.elements);
	node->element = copySpellElement(&spellElement_missile_trio);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_stoneblood);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_bleed);
	strcpy(spell_bleed.name, language[2392]);
	spell_bleed.ID = SPELL_BLEED;
	spell_bleed.difficulty = 80;
	node = list_AddNodeLast(&spell_bleed.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_bleed);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_summon);
	strcpy(spell_summon.name, language[2390]);
	spell_summon.ID = SPELL_SUMMON;
	spell_summon.difficulty = 40;
	spell_summon.elements.first = NULL;
	spell_summon.elements.last = NULL;
	node = list_AddNodeLast(&spell_summon.elements);
	node->element = copySpellElement(&spellElement_summon);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_dominate);
	strcpy(spell_dominate.name, language[2393]);
	spell_dominate.ID = SPELL_DOMINATE;
	spell_dominate.difficulty = 100;
	node = list_AddNodeLast(&spell_dominate.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_dominate);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node;

	spellConstructor(&spell_reflectMagic);
	strcpy(spell_reflectMagic.name, language[2394]);
	spell_reflectMagic.ID = SPELL_REFLECT_MAGIC;
	spell_reflectMagic.difficulty = 80;
	spell_reflectMagic.elements.first = nullptr;
	spell_reflectMagic.elements.last = nullptr;
	node = list_AddNodeLast(&spell_reflectMagic.elements);
	node->element = copySpellElement(&spellElement_reflectMagic);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*) node->element;
	element->node = node; //Tell the element what list it resides in.
	element->channeled = true;

	spellConstructor(&spell_acidSpray);
	strcpy(spell_acidSpray.name, language[2395]);
	spell_acidSpray.ID = SPELL_ACID_SPRAY;
	spell_acidSpray.difficulty = 80;
	node = list_AddNodeLast(&spell_acidSpray.elements);
	node->element = copySpellElement(&spellElement_missile_trio);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_acidSpray);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_stealWeapon);
	strcpy(spell_stealWeapon.name, language[2396]);
	spell_stealWeapon.ID = SPELL_STEAL_WEAPON;
	spell_stealWeapon.difficulty = 100;
	node = list_AddNodeLast(&spell_stealWeapon.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_stealWeapon);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_drainSoul);
	strcpy(spell_drainSoul.name, language[2397]);
	spell_drainSoul.ID = SPELL_DRAIN_SOUL;
	spell_drainSoul.difficulty = 80;
	node = list_AddNodeLast(&spell_drainSoul.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
	//Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_drainSoul);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_vampiricAura);
	strcpy(spell_vampiricAura.name, language[2398]);
	spell_vampiricAura.ID = SPELL_VAMPIRIC_AURA;
	spell_vampiricAura.difficulty = 80;
	spell_vampiricAura.elements.first = nullptr;
	spell_vampiricAura.elements.last = nullptr;
	node = list_AddNodeLast(&spell_vampiricAura.elements);
	node->element = copySpellElement(&spellElement_vampiricAura);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
	element->channeled = true;

	spellConstructor(&spell_amplifyMagic);
	strcpy(spell_amplifyMagic.name, language[3440]);
	spell_amplifyMagic.ID = SPELL_AMPLIFY_MAGIC;
	spell_amplifyMagic.difficulty = 80;
	spell_amplifyMagic.elements.first = nullptr;
	spell_amplifyMagic.elements.last = nullptr;
	node = list_AddNodeLast(&spell_amplifyMagic.elements);
	node->element = copySpellElement(&spellElement_amplifyMagic);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
	element->channeled = true;

	spellConstructor(&spell_charmMonster);
	strcpy(spell_charmMonster.name, language[2399]);
	spell_charmMonster.ID = SPELL_CHARM_MONSTER;
	spell_charmMonster.difficulty = 80;
	node = list_AddNodeLast(&spell_charmMonster.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_charmMonster);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_revertForm);
	strcpy(spell_revertForm.name, language[3407]);
	spell_revertForm.ID = SPELL_REVERT_FORM;
	spell_revertForm.difficulty = 0;
	spell_revertForm.elements.first = NULL;
	spell_revertForm.elements.last = NULL;
	node = list_AddNodeLast(&spell_revertForm.elements);
	node->element = copySpellElement(&spellElement_shapeshift);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;
	element->mana = 5;

	spellConstructor(&spell_ratForm);
	strcpy(spell_ratForm.name, language[3408]);
	spell_ratForm.ID = SPELL_RAT_FORM;
	spell_ratForm.difficulty = 0;
	spell_ratForm.elements.first = NULL;
	spell_ratForm.elements.last = NULL;
	node = list_AddNodeLast(&spell_ratForm.elements);
	node->element = copySpellElement(&spellElement_shapeshift);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;
	element->mana = 8;

	spellConstructor(&spell_spiderForm);
	strcpy(spell_spiderForm.name, language[3409]);
	spell_spiderForm.ID = SPELL_SPIDER_FORM;
	spell_spiderForm.difficulty = 40;
	spell_spiderForm.elements.first = NULL;
	spell_spiderForm.elements.last = NULL;
	node = list_AddNodeLast(&spell_spiderForm.elements);
	node->element = copySpellElement(&spellElement_shapeshift);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;
	element->mana = 16;

	spellConstructor(&spell_trollForm);
	strcpy(spell_trollForm.name, language[3410]);
	spell_trollForm.ID = SPELL_TROLL_FORM;
	spell_trollForm.difficulty = 60;
	spell_trollForm.elements.first = NULL;
	spell_trollForm.elements.last = NULL;
	node = list_AddNodeLast(&spell_trollForm.elements);
	node->element = copySpellElement(&spellElement_shapeshift);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;
	element->mana = 24;

	spellConstructor(&spell_impForm);
	strcpy(spell_impForm.name, language[3411]);
	spell_impForm.ID = SPELL_IMP_FORM;
	spell_impForm.difficulty = 80;
	spell_impForm.elements.first = NULL;
	spell_impForm.elements.last = NULL;
	node = list_AddNodeLast(&spell_impForm.elements);
	node->element = copySpellElement(&spellElement_shapeshift);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;
	element->mana = 32;

	spellConstructor(&spell_sprayWeb);
	strcpy(spell_sprayWeb.name, language[3412]);
	spell_sprayWeb.ID = SPELL_SPRAY_WEB;
	spell_sprayWeb.difficulty = 20;
	node = list_AddNodeLast(&spell_sprayWeb.elements);
	node->element = copySpellElement(&spellElement_missile_trio);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_sprayWeb);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_poison);
	strcpy(spell_poison.name, language[3413]);
	spell_poison.ID = SPELL_POISON;
	spell_poison.difficulty = 40;
	node = list_AddNodeLast(&spell_poison.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_poison);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_speed);
	strcpy(spell_speed.name, language[3414]);
	spell_speed.ID = SPELL_SPEED;
	spell_speed.difficulty = 40;
	spell_speed.elements.first = NULL;
	spell_speed.elements.last = NULL;
	node = list_AddNodeLast(&spell_speed.elements);
	node->element = copySpellElement(&spellElement_speed);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_fear);
	strcpy(spell_fear.name, language[3415]);
	spell_fear.ID = SPELL_FEAR;
	spell_fear.difficulty = 80;
	spell_fear.elements.first = NULL;
	spell_fear.elements.last = NULL;
	node = list_AddNodeLast(&spell_fear.elements);
	node->element = copySpellElement(&spellElement_fear);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_weakness);
	strcpy(spell_weakness.name, language[3422]);
	spell_weakness.ID = SPELL_WEAKNESS;
	spell_weakness.difficulty = 100;
	node = list_AddNodeLast(&spell_weakness.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_weakness);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_strike);
	strcpy(spell_strike.name, language[3416]);
	spell_strike.ID = SPELL_STRIKE;
	spell_strike.difficulty = 80;
	spell_strike.elements.first = NULL;
	spell_strike.elements.last = NULL;
	node = list_AddNodeLast(&spell_strike.elements);
	node->element = copySpellElement(&spellElement_strike);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_detectFood);
	strcpy(spell_detectFood.name, language[3421]);
	spell_detectFood.ID = SPELL_DETECT_FOOD;
	spell_detectFood.difficulty = 40;
	spell_detectFood.elements.first = NULL;
	spell_detectFood.elements.last = NULL;
	node = list_AddNodeLast(&spell_detectFood.elements);
	node->element = copySpellElement(&spellElement_detectFood);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_trollsBlood);
	strcpy(spell_trollsBlood.name, language[3489]);
	spell_trollsBlood.ID = SPELL_TROLLS_BLOOD;
	spell_trollsBlood.difficulty = 60;
	spell_trollsBlood.elements.first = NULL;
	spell_trollsBlood.elements.last = NULL;
	node = list_AddNodeLast(&spell_trollsBlood.elements);
	node->element = copySpellElement(&spellElement_trollsBlood);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_flutter);
	strcpy(spell_flutter.name, language[3764]);
	spell_flutter.ID = SPELL_FLUTTER;
	spell_flutter.difficulty = 60;
	spell_flutter.elements.first = NULL;
	spell_flutter.elements.last = NULL;
	node = list_AddNodeLast(&spell_flutter.elements);
	node->element = copySpellElement(&spellElement_flutter);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_dash);
	strcpy(spell_dash.name, language[3765]);
	spell_dash.ID = SPELL_DASH;
	spell_dash.difficulty = 40;
	spell_dash.elements.first = NULL;
	spell_dash.elements.last = NULL;
	node = list_AddNodeLast(&spell_dash.elements);
	node->element = copySpellElement(&spellElement_dash);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_shadowTag);
	strcpy(spell_shadowTag.name, language[3447]);
	spell_shadowTag.ID = SPELL_SHADOW_TAG;
	spell_shadowTag.difficulty = 20;
	node = list_AddNodeLast(&spell_shadowTag.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_shadowTag);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_telePull);
	strcpy(spell_telePull.name, language[3448]);
	spell_telePull.ID = SPELL_TELEPULL;
	spell_telePull.difficulty = 60;
	node = list_AddNodeLast(&spell_telePull.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_telePull);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_demonIllusion);
	strcpy(spell_demonIllusion.name, language[3449]);
	spell_demonIllusion.ID = SPELL_DEMON_ILLUSION;
	spell_demonIllusion.difficulty = 80;
	node = list_AddNodeLast(&spell_demonIllusion.elements);
	node->element = copySpellElement(&spellElement_missile);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node; //Tell the element what list it resides in.
						  //Now for the second element.
	element->elements.first = NULL;
	element->elements.last = NULL;
	node = list_AddNodeLast(&element->elements);
	node->element = copySpellElement(&spellElement_demonIllusion);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;

	spellConstructor(&spell_salvageItem);
	strcpy(spell_salvageItem.name, language[3711]);
	spell_salvageItem.ID = SPELL_SALVAGE;
	spell_salvageItem.difficulty = 20;
	spell_salvageItem.elements.first = NULL;
	spell_salvageItem.elements.last = NULL;
	node = list_AddNodeLast(&spell_salvageItem.elements);
	node->element = copySpellElement(&spellElement_salvageItem);
	node->size = sizeof(spellElement_t);
	node->deconstructor = &spellElementDeconstructor;
	element = (spellElement_t*)node->element;
	element->node = node;
}
