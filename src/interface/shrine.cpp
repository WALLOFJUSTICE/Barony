/*-------------------------------------------------------------------------------

	BARONY
	File: interface.cpp
	Desc: contains code for game interface

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../main.hpp"
#include "../files.hpp"
#include "../game.hpp"
#include "interface.hpp"
#include "../ui/GameUI.hpp"
#include "../ui/MainMenu.hpp"
#include "../json.hpp"
#include "../mod_tools.hpp"
#include "../ui/Field.hpp"
#include "../ui/Image.hpp"
#include "../ui/Button.hpp"
#include "../ui/Slider.hpp"
#include "../collision.hpp"

static void genericgui_deselect_fn(Widget& widget) {
	if ( widget.isSelected() )
	{
		if ( !inputs.getVirtualMouse(widget.getOwner())->draw_cursor )
		{
			widget.deselect();
		}
	}
}

enum InstrumentOrder {
	FLUTE,
	LYRE,
	DRUM,
	LUTE,
	HORN
};

void shrineApplyMusic(Entity* shrine)
{
	if ( !shrine ) { return; }

	int duration = shrine->eternalShrineOrchestrionTimer & 0x00FFFF;
	if ( duration <= 0 ) { return; }
	std::vector<std::pair<int, int>> instrumentsPlaying = {
		{EFF_ENSEMBLE_FLUTE, 0},
		{EFF_ENSEMBLE_LYRE, 0},
		{EFF_ENSEMBLE_DRUM, 0},
		{EFF_ENSEMBLE_LUTE, 0},
		{EFF_ENSEMBLE_HORN, 0}
	};

	if ( ((shrine->eternalShrineOrchestrionInstruments >> 0) & 0xF) )
	{
		instrumentsPlaying[FLUTE].second = ((shrine->eternalShrineOrchestrionInstruments >> 0) & 0xF);
	}
	if ( ((shrine->eternalShrineOrchestrionInstruments >> 4) & 0xF) )
	{
		instrumentsPlaying[LYRE].second = ((shrine->eternalShrineOrchestrionInstruments >> 4) & 0xF);
	}
	if ( ((shrine->eternalShrineOrchestrionInstruments >> 8) & 0xF) )
	{
		instrumentsPlaying[DRUM].second = ((shrine->eternalShrineOrchestrionInstruments >> 8) & 0xF);
	}
	if ( ((shrine->eternalShrineOrchestrionInstruments >> 12) & 0xF) )
	{
		instrumentsPlaying[LUTE].second = ((shrine->eternalShrineOrchestrionInstruments >> 12) & 0xF);
	}
	if ( ((shrine->eternalShrineOrchestrionInstruments >> 16) & 0xF) )
	{
		instrumentsPlaying[HORN].second = ((shrine->eternalShrineOrchestrionInstruments >> 16) & 0xF);
	}

	int player = achievementObserver.checkUidIsFromPlayer(shrine->eternalShrineTarget);

	std::vector<Entity*> targets;
	for ( node_t* node = map.creatures->first; node; node = node->next )
	{
		if ( Entity* entity = (Entity*)(node->element) )
		{
			if ( entity->behavior != &actPlayer && entity->behavior != &actMonster )
			{
				continue;
			}
			if ( Stat* entitystats = entity->getStats() )
			{
				if ( entity->behavior == &actPlayer )
				{
					targets.push_back(entity);
				}
				else if ( player >= 0 && players[player]->entity && players[player]->entity->checkFriend(entity) )
				{
					targets.push_back(entity);
				}
			}
		}
	}

	for ( auto eff : instrumentsPlaying )
	{
		Uint8 tier = eff.second;
		if ( tier ) // tier
		{
			for ( auto target : targets )
			{
				if ( target )
				{
					if ( Stat* targetStats = target->getStats() )
					{
						int effectID = eff.first;
						int dur = targetStats->getEffectActive(effectID) ? targetStats->EFFECTS_TIMERS[effectID] : 0;
						Uint8 tierStrength = Stat::kEnsembleBreakPointTier1 + 1;
						if ( tier >= 4 )
						{
							tierStrength = Stat::kEnsembleBreakPointTier4 + 1;
						}
						else if ( tier == 3 )
						{
							tierStrength = Stat::kEnsembleBreakPointTier3 + 1;
						}
						else if ( tier == 2 )
						{
							tierStrength = Stat::kEnsembleBreakPointTier2 + 1;
						}
						Uint8 effectStrength = std::max(tierStrength, targetStats->getEffectActive(effectID));
						if ( target->setEffect(effectID, effectStrength, std::max(dur, duration), false) )
						{
							if ( dur == 0 )
							{
								if ( target->behavior == &actPlayer )
								{
									playSoundEntity(target, 168, 64);
								}
								createEnsembleTargetParticleCircling(target);
								serverSpawnMiscParticles(target, PARTICLE_EFFECT_ENSEMBLE_OTHER_CAST, 0);
							}
						}
					}
				}
			}
		}
	}
}

void clientUpdateShrineSkill(Uint32 uid, int skill, Sint32 value, bool norespond)
{
	strcpy((char*)net_packet->data, "ESHU");
	net_packet->data[4] = clientnum;
	SDLNet_Write32(uid, &net_packet->data[5]);
	net_packet->data[9] = skill;
	SDLNet_Write32(value, &net_packet->data[10]);
	net_packet->data[14] = norespond ? 1 : 0;
	net_packet->address.host = net_server.host;
	net_packet->address.port = net_server.port;
	net_packet->len = 15;
	sendPacketSafe(net_sock, -1, net_packet, 0);
}

#define LIMB_COFFER 2537
#define LIMB_CRANK 2538
#define LIMB_DOOR_LEFT 2539
#define LIMB_DOOR_RIGHT 2540
#define LIMB_REELS 2541
#define LIMB_HAMMER 2542

void actGlow(Entity* my)
{
	if ( my->skill[0] == 0 )
	{
		// init
		my->skill[0] = 1;
		my->fskill[1] = my->x;
		my->fskill[2] = my->y;
		if ( my->skill[1] == 1 )
		{
			my->scalex = 0.5;
			my->scaley = 0.5;
			my->scalez = 0.5;
		}
	}

	if ( my->skill[0] == 1 )
	{
		// grow to size
		my->scalex += std::min(0.5, 0.025);
		my->scaley += std::min(0.5, 0.025);
		my->scalez += std::min(0.5, 0.025);
		if ( my->scalex >= 0.5 )
		{
			my->skill[0] = 2;
		}
	}



	auto& leftright = my->fskill[3];
	if ( my->skill[1] == 1 )
	{
		leftright += 0.05;
		if ( my->fskill[0] < 1.0 )
		{
			my->fskill[0] += (1.0 - my->fskill[0]) / 50.0;
		}
	}
	else
	{
		leftright += 0.05;
		if ( my->fskill[0] < 1.0 )
		{
			my->fskill[0] += (1.0 - my->fskill[0]) / 50.0;
		}
	}

	my->x = my->fskill[1] + 2.0 * cos(my->yaw) * sin(leftright) * my->fskill[0];
	my->y = my->fskill[2] + 2.0 * sin(my->yaw) * sin(leftright) * my->fskill[0];

	if ( my->skill[1] == 1 )
	{
		if ( my->skill[0] == 2 )
		{
			my->scalex -= 0.01;
			my->scaley -= 0.01;
			my->scalez -= 0.01;
		}

		// float to center
		my->x += (my->fskill[4] - my->x) * my->fskill[0];
		my->y += (my->fskill[5] - my->y) * my->fskill[0];
		my->z += my->vel_z * my->fskill[0];
	}
	else
	{
		if ( my->skill[0] == 2 )
		{
			my->scalex -= 0.005;
			my->scaley -= 0.005;
			my->scalez -= 0.005;
		}

		my->z += my->vel_z * my->fskill[0] * 0.25;
	}

	if ( my->scalex <= 0.0 )
	{
		list_RemoveNode(my->mynode);
		return;
	}
}

static ConsoleVariable<float> cvar_eternal_shrine_ascension_delay("/eternal_shrine_ascension_delay", 50.0);
static ConsoleVariable<float> cvar_eternal_shrine_skip_prompt_speed("/eternal_shrine_skip_prompt_speed", 0.05);

void actEternalShrineOffering(Entity* my)
{
	my->removeLightField();
	if ( my->parent == 0 )
	{
		list_RemoveNode(my->mynode);
		return;
	}

	Entity* parent = uidToEntity(my->parent);
	if ( !parent )
	{
		list_RemoveNode(my->mynode);
		return;
	}

	auto& lastPlayer = my->skill[3];
	auto& lastView = my->skill[4];
	auto& isSprite = my->skill[5];
	if ( !my->eternalShrineInit )
	{
		my->eternalShrineInit = 1;
		lastPlayer = MAXPLAYERS;
	}

	if ( my->skill[0] > 0 )
	{
		--my->skill[0];
	}

	bool invisible = true;
	Entity* interacting = uidToEntity(parent->eternalShrineInteracting);
	int submittedState = GenericGUIMenu::EternalShrineGUI_t::SUBMIT_NONE;
	int previousView = lastView;
	if ( interacting )
	{
		if ( interacting->behavior == &actPlayer )
		{
			lastPlayer = interacting->skill[2];
		}
		int v = (int)(parent->eternalShrineOfferingItemTypeModel & 0xF);
		if ( v == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_ACTION
			&& lastView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_OFFERING
			&& my->skill[0] > 0 )
		{
			// when transitioning from offering -> action, 
			// if the transition is too quick then it will miss the particle fx when item vanishes
			// linger for a few ticks before transitioning states
			my->skill[0] = std::min(my->skill[0], 5);
		}
		else
		{
			if ( v != GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
			{
				// waiting state is buggy, leave it out since this is skipped in singleplayer fine
				lastView = v;
			}
		}
	}
	else
	{
		my->skill[0] = std::min(my->skill[0], TICKS_PER_SECOND / 2); // dont wait for 3 second timer, make it quick if owner has left
	}

	if ( lastPlayer >= 0 && lastPlayer < MAXPLAYERS )
	{
		submittedState = (parent->eternalShrinePlayerStates >> (lastPlayer * 2)) & 0b11;
	}

	int spriteItem = ((parent->eternalShrineOfferingItemTypeModel & 0xFFFF0000) >> 16);
	int spriteType = ((parent->eternalShrineOfferingItemTypeModel & 0xFFF0) >> 4);
	int shrineCurrentView = lastView;
	if ( shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
	{
		// unused path probably
		if ( spriteItem > 0 )
		{
			my->sprite = spriteItem;
			isSprite = (parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION && spriteType == SPELL_ITEM) ? 1 : 0;
			invisible = false;
		}
		/*else if ( parent->eternalShrineOfferingItemVisible >= GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
		{
			if ( my->sprite > 0 && parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION && my->flags[SPRITE] )
			{
				isSprite = true;
				invisible = false;
			}
		}*/
	}
	else if ( shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_ACTION )
	{
		my->skill[0] = 0;
		if ( parent->eternalShrineState > 0 )
		{
			// busy doing an action
			if ( spriteItem == 0 && !my->flags[INVISIBLE] )
			{
				invisible = false;
			}
			else if ( spriteItem > 0 )
			{
				my->sprite = spriteItem;
				isSprite = (parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION && spriteType == SPELL_ITEM) ? 1 : 0;
				invisible = false;
			}
		}
		else
		{
			if ( spriteItem > 0 )
			{
				my->sprite = spriteItem;
				isSprite = (parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION && spriteType == SPELL_ITEM) ? 1 : 0;
				invisible = false;
			}
		}
	}
	else if ( shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_OFFERING )
	{
		if ( interacting )
		{
			if ( submittedState >= GenericGUIMenu::EternalShrineGUI_t::SUBMIT_CONFIRMED )
			{

			}
			else
			{
				my->skill[0] = TICKS_PER_SECOND * 3;
			}
		}

		if ( parent && spriteItem == 0 && !my->flags[INVISIBLE] )
		{
			if ( submittedState > GenericGUIMenu::EternalShrineGUI_t::SUBMIT_NONE )
			{
				if ( my->skill[0] > 0 )
				{
					invisible = false;
				}
			}
		}
		else if ( parent && spriteItem > 0 )
		{
			bool completedOffering = true;
			if ( my->skill[0] > 0 )
			{
				completedOffering = false; // delay before transition
			}

			if ( !completedOffering )
			{
				my->sprite = spriteItem;
				isSprite = (parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION && spriteType == SPELL_ITEM) ? 1 : 0;
				if ( my->sprite > 0 )
				{
					invisible = false;
				}
			}
		}
	}

	if ( my->sprite > 0 )
	{
		real_t groundheight = 0.0;
		if ( isSprite )
		{
			my->roll = 0.0;
			groundheight = 5.5 + 0.35 * sin((my->ticks % 400 / 200.0) * 2 * PI);
		}
		else
		{
			if ( my->sprite == items[CRYSTAL_SHURIKEN].index )
			{
				groundheight = 8.5 - models[my->sprite]->sizey * .25;
				my->roll = PI;
			}
			else if ( my->sprite == items[TOOL_BOMB].index || my->sprite == items[TOOL_FREEZE_BOMB].index
				|| my->sprite == items[TOOL_SLEEP_BOMB].index || my->sprite == items[TOOL_TELEPORT_BOMB].index
				|| my->sprite == items[TOOL_DETONATOR_CHARGE].index )
			{
				groundheight = 7.5 - models[my->sprite]->sizey * .25;
				my->roll = 3 * PI / 2;
			}
			else if ( my->sprite == items[STEEL_CHAKRAM].index )
			{
				groundheight = 8.75 - models[my->sprite]->sizey * .25;
				my->roll = PI;
			}
			else if ( my->sprite == items[BOOMERANG].index )
			{
				groundheight = 9.0 - models[my->sprite]->sizey * .25;
				my->roll = PI;
			}
			else
			{
				groundheight = 7.5 - models[my->sprite]->sizey * .25;
				my->roll = PI / 2.0;
			}
		}

		my->x = parent->x;
		my->y = parent->y;
		my->yaw = parent->yaw;
		my->pitch = 0;
		if ( parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
		{
			my->x += 1.0 * cos(parent->yaw) - 1.0 * cos(parent->yaw + PI / 2);
			my->y += 1.0 * sin(parent->yaw) - 1.0 * sin(parent->yaw + PI / 2);
			groundheight -= 8.0;
		}
		else if ( parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
		{
			groundheight -= 6.0;
		}
		else if ( parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
		{
			if ( isSprite )
			{
			}
			else
			{
				my->x += 0.25 * cos(parent->yaw) - 0.5 * cos(parent->yaw + PI / 2);
				my->y += 0.25 * sin(parent->yaw) - 0.5 * sin(parent->yaw + PI / 2);
			}
			groundheight -= 6.5;
		}
		my->z = groundheight;
	}

	if ( my->flags[INVISIBLE] != invisible && !my->flags[INVISIBLE] 
		&& submittedState >= GenericGUIMenu::EternalShrineGUI_t::SUBMIT_WAITING
		&& previousView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_OFFERING
		&& (shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_ACTION || !interacting) )
	{
		for ( int i = 0; i < 4; ++i )
		{
			Entity* fx = spawnMagicParticle(my);
			fx->vel_x = 0.5 * cos(my->yaw + i * PI / 2);
			fx->vel_y = 0.5 * sin(my->yaw + i * PI / 2);
			fx->sprite = 2544;
		}

		Entity* fx = createParticleAestheticOrbit(my, 2544, TICKS_PER_SECOND / 4, PARTICLE_EFFECT_NULL_PARTICLE_NOSOUND);
		fx->x = my->x;
		fx->y = my->y;
		fx->z = my->z;
		fx->yaw = my->yaw;
		fx->actmagicOrbitDist = 0;
		fx->actmagicNoLight = 1;
		playSoundEntityLocal(parent, 882, 128);
	}

	my->flags[SPRITE] = isSprite ? 1 : 0;
	if ( my->flags[SPRITE] )
	{
		real_t scale = std::min(25.0, std::max(0.0, parent->fskill[0] - (real_t)*cvar_eternal_shrine_ascension_delay))
			/ (25.0); // ascensionTimer
		real_t prevScale = my->scalex;
		my->scalex = 0.2 * (1.0 - scale);
		/*if ( prevScale >= 0.19 && my->scalex < 0.19 )
		{
			playSoundEntityLocal(parent, 252, 128);
		}*/
		if ( prevScale > 0.01 && my->scalex <= 0.01 )
		{
			for ( int i = 0; i < 4; ++i )
			{
				Entity* fx = spawnMagicParticle(my);
				fx->vel_x = 0.5 * cos(my->yaw + i * PI / 2);
				fx->vel_y = 0.5 * sin(my->yaw + i * PI / 2);
				fx->sprite = 2544;
			}

			Entity* fx = createParticleAestheticOrbit(my, 2544, TICKS_PER_SECOND / 4, PARTICLE_EFFECT_NULL_PARTICLE_NOSOUND);
			fx->x = my->x;
			fx->y = my->y;
			fx->z = my->z;
			fx->yaw = my->yaw;
			fx->actmagicOrbitDist = 0;
			fx->actmagicNoLight = 1;
			playSoundEntityLocal(parent, 883, 92);
		}
		my->scaley = 0.2;// (1.0 - scale);
		my->scalez = 0.2;

		real_t floatScale = std::min(25.0 + (real_t)*cvar_eternal_shrine_ascension_delay, parent->fskill[0])
			/ (25.0 + (real_t)*cvar_eternal_shrine_ascension_delay); // ascensionTimer
		my->z -= floatScale * 3.0;
		if ( floatScale > 0.05 && my->ticks % 5 == 0 )
		{
			if ( !my->flags[INVISIBLE] && my->sprite > 0 )
			{
				int viewingMode = parent->eternalShrineAscensionItemColor > 0 ? parent->eternalShrineAscensionItemColor : parent->eternalShrineViewingMode;
				int sprite = 16;
				if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::SORCERY_SPELL )
				{
					sprite = 225;
				}
				else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::MYSTICISM_SPELL )
				{
					sprite = 261;
				}
				else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::THAUMATURGY_SPELL )
				{
					sprite = 160;
				}
				if ( Entity* fx = spawnMagicParticleCustom(my, sprite, 0.1, 1.0) )
				{
					fx->flags[SPRITE] = true;
					fx->x = my->x;
					fx->y = my->y;
					fx->z = my->z;
					fx->ditheringDisabled = true;
					fx->yaw = my->yaw + ((my->ticks % (5 * 8)) / 8) * PI / 2;
					fx->vel_z = 0.25;
					fx->fskill[4] = my->x;
					fx->fskill[5] = my->y;
					fx->behavior = &actGlow;
					fx->skill[1] = 1;
				}
			}
		}
	}
	else
	{
		my->scalex = 1.0;
		my->scaley = 1.0;
		my->scalez = 1.0;
	}
	if ( my->flags[INVISIBLE] && !invisible )
	{
		my->bNeedsRenderPositionInit = true;
	}
	my->flags[INVISIBLE] = invisible;
	if ( my->flags[INVISIBLE] )
	{
		my->sprite = 0;
	}
	parent->eternalShrineOfferingItemVisible = my->flags[INVISIBLE] ? 0 : std::max(parent->eternalShrineOfferingItemVisible, shrineCurrentView + 1);
	if ( my->flags[SPRITE] && !my->flags[INVISIBLE] )
	{
		parent->eternalShrineAscensionItemColor = 0;
		if ( my->sprite > 0 && my->sprite < NUM_SPELLS )
		{
			if ( auto spell = getSpellFromID(my->sprite) )
			{
				if ( spell->skillID == PRO_SORCERY ) { parent->eternalShrineAscensionItemColor = GenericGUIMenu::EternalShrineGUI_t::SORCERY_SPELL; }
				else if ( spell->skillID == PRO_MYSTICISM ) { parent->eternalShrineAscensionItemColor = GenericGUIMenu::EternalShrineGUI_t::MYSTICISM_SPELL; }
				else if ( spell->skillID == PRO_THAUMATURGY ) { parent->eternalShrineAscensionItemColor = GenericGUIMenu::EternalShrineGUI_t::THAUMATURGY_SPELL; }
			}
		}
		else if ( my->sprite == NUM_SPELLS ) { parent->eternalShrineAscensionItemColor = GenericGUIMenu::EternalShrineGUI_t::SORCERY_SPELL; }
		else if ( my->sprite == NUM_SPELLS + 1 ) { parent->eternalShrineAscensionItemColor = GenericGUIMenu::EternalShrineGUI_t::MYSTICISM_SPELL; }
		else if ( my->sprite == NUM_SPELLS + 2 ) { parent->eternalShrineAscensionItemColor = GenericGUIMenu::EternalShrineGUI_t::THAUMATURGY_SPELL; }
	}
	else
	{
		parent->eternalShrineAscensionItemColor = 0;
	}
}

void actEternalShrineLimb(Entity* my)
{
	my->removeLightField();
	if ( my->parent == 0 )
	{
		list_RemoveNode(my->mynode);
		return;
	}

	Entity* parent = uidToEntity(my->parent);
	if ( !parent )
	{
		list_RemoveNode(my->mynode);
		return;
	}

	if ( !my->eternalShrineInit )
	{
		my->eternalShrineInit = 1;
		my->flags[INVISIBLE] = false;
	}

	if ( my->sprite >= LIMB_COFFER && my->sprite <= LIMB_REELS )
	{
		real_t& animRise = my->fskill[0];
		real_t& animStrike = my->fskill[1];
		real_t& animBounce = my->fskill[2];
		real_t& animPlay = my->fskill[3];
		Entity* interacting = uidToEntity(parent->eternalShrineInteracting);

		int timeLeft = parent->eternalShrineOrchestrionTimer & 0x00FFFF;

		int shrineCurrentView = (int)(parent->eternalShrineOfferingItemTypeModel & 0xF);
		if ( (interacting
			&& (shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_ACTION
				|| shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING))
			|| timeLeft > 0
			|| parent->eternalShrineOfferingItemVisible >= GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
		{
			real_t prev = animRise;
			animRise += std::max((1.0 - animRise), 0.01) / 20.0;
			animRise = std::min(1.0, animRise);
			if ( my->sprite == LIMB_DOOR_LEFT )
			{
				if ( prev < 0.1 && animRise >= 0.1 )
				{
					playSoundEntityLocal(parent, 21, 64);
				}
			}
		}
		else
		{
			real_t prev = animRise;
			animRise -= std::max(animRise, 0.05) / 10.0;
			animRise = std::max(0.0, animRise);
			if ( my->sprite == LIMB_DOOR_LEFT )
			{
				if ( prev >= 0.1 && animRise < 0.1 )
				{
					playSoundEntityLocal(parent, 22, 64);
				}
			}
		}

		if ( my->sprite == LIMB_DOOR_LEFT )
		{
			if ( animRise >= 0.05 )
			{
				int windupTime = (parent->eternalShrineOrchestrionTimer & 0xFF0000) >> 16;
				if ( timeLeft > 0 && windupTime >= 3 * TICKS_PER_SECOND )
				{
					my->light = addLight(my->x / 16, my->y / 16, "chorale_shrine_playing");
				}
				else
				{
					my->light = addLight(my->x / 16, my->y / 16, "chorale_shrine_open");
				}
			}
			else
			{
				my->light = addLight(my->x / 16, my->y / 16, "chorale_shrine");
			}
		}

		if ( timeLeft > 0 )
		{
			int windupTime = (parent->eternalShrineOrchestrionTimer & 0xFF0000) >> 16;
			if ( windupTime < 3 * TICKS_PER_SECOND )
			{
				animPlay += 0.1;
			}
			else
			{
				real_t scale = ((4 * TICKS_PER_SECOND) - std::min(windupTime, 4 * TICKS_PER_SECOND)) / (real_t)TICKS_PER_SECOND;
				scale = std::max(0.0, scale);
				scale = std::min(1.0, scale);
				animPlay += 0.05 + 0.05 * (scale);
			}
		}
		else
		{
			if ( animPlay > 0.0 )
			{
				animPlay = fmod(animPlay, 2.0);
				animPlay -= 4.0;
			}

			if ( animPlay < 0.0 )
			{
				animPlay += std::max(abs(animPlay) / 2.0, 0.1) / 30.0;
				animPlay = std::min(0.0, animPlay);
			}
		}

		if ( my->sprite == LIMB_COFFER )
		{
			if ( interacting && shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_OFFERING )
			{
				if ( animBounce > 1.0 )
				{
					real_t prev = animBounce;
					animBounce += 0.125;
					if ( (prev < 2.0 && animBounce >= 2.0)
						|| (prev < 4.0 && animBounce >= 4.0)
						|| (prev < 6.0 && animBounce >= 6.0) )
					{
						animBounce = 0.0;
					}
				}
				else
				{
					animBounce += 0.05;
					animBounce = std::min(1.0, animBounce);
				}
			}
			else
			{
				if ( animBounce >= 0.005 )
				{
					animBounce += 0.125;
					if ( animBounce >= 6.0 )
					{
						animBounce = 0.0;
					}
				}
			}

			int interval = 25;
			int sprite = 16;
			if ( my->ticks % 80 == 0 )
			{
				interval = 80;
			}
			if ( interacting && my->ticks % interval == 0 && animBounce >= 0.005 )
			{
				if ( Entity* fx = spawnMagicParticleCustom(my, sprite, 0.1, 1.0) )
				{
					fx->flags[SPRITE] = true;
					fx->x = my->x + cos(my->yaw);
					fx->y = my->y + sin(my->yaw);
					fx->z = my->z + 1.0;
					fx->ditheringDisabled = true;
					fx->yaw = my->yaw + ((my->ticks % (interval * 8)) / 8) * PI / 4;
					fx->vel_z = -0.25;
					fx->fskill[4] = fx->x;
					fx->fskill[5] = fx->y;
					fx->behavior = &actGlow;
					fx->skill[1] = 0;
				}
			}
		}
	}

	if ( my->sprite == LIMB_COFFER )
	{
		real_t& animBounce = my->fskill[2];
		my->x = parent->x + 1.25 * cos(parent->yaw);
		my->y = parent->y + 1.25 * sin(parent->yaw);
		my->z = parent->z - 3.5;
		my->focalx = 1;
		my->focalz = 1.25;
		/*if ( my->ticks % 100 <= 100 )
		{
			my->pitch = -(PI * 0.5) * sin(PI * (my->ticks % 100) / 100.0);
		}*/
		real_t dampen = 1.0;
		if ( animBounce >= 4.0 )
		{
			dampen = 3.0;
		}
		else if ( animBounce >= 2.0 )
		{
			dampen = 2.0;
		}
		my->pitch = -(PI * 0.5) * abs(sin(PI * animBounce / 2.0)) / dampen;
	}
	else if ( my->sprite == LIMB_CRANK )
	{
		my->x = parent->x + -0.75 * cos(parent->yaw);
		my->y = parent->y + -0.75 * sin(parent->yaw);
		my->z = parent->z - 3.5;
		my->focalx = 0;
		my->focalz = 0.5;
		
		real_t& animRise = my->fskill[0];
		real_t& animPlay = my->fskill[3];
		my->pitch = 0.0;
		my->pitch += animRise * 2 * PI;
		my->pitch -= animPlay * 2 * PI;
	}
	else if ( my->sprite == LIMB_DOOR_LEFT )
	{
		my->yaw = parent->yaw;
		my->x = parent->x + 0 * cos(parent->yaw) + 4.25 * cos(parent->yaw + PI / 2);
		my->y = parent->y + 0 * sin(parent->yaw) + 4.25 * sin(parent->yaw + PI / 2);
		my->z = parent->z - 9;
		my->focalx = 0.5;
		my->focaly = -2;
		my->focalz = 0;
		my->pitch = 0;
		/*if ( my->ticks % 100 <= 50 )
		{
			my->yaw -= -(PI * 0.5) * sin(PI * (my->ticks % 100) / 50.0);
		}*/
		real_t& animRise = my->fskill[0];
		my->yaw -= -(PI * 0.5) * sin(PI * animRise / 2.0);
	}
	else if ( my->sprite == LIMB_DOOR_RIGHT )
	{
		my->yaw = parent->yaw;
		my->x = parent->x + 0 * cos(parent->yaw) + -4.25 * cos(parent->yaw + PI / 2);
		my->y = parent->y + 0 * sin(parent->yaw) + -4.25 * sin(parent->yaw + PI / 2);
		my->z = parent->z - 9;
		my->focalx = 0.5;
		my->focaly = 2;
		my->focalz = 0;
		my->pitch = 0;
		/*if ( my->ticks % 100 <= 50 )
		{
			my->yaw += -(PI * 0.5) * sin(PI * (my->ticks % 100) / 50.0);
		}*/
		real_t& animRise = my->fskill[0];
		my->yaw += -(PI * 0.5) * sin(PI * animRise / 2.0);
	}
	else if ( my->sprite == LIMB_REELS )
	{
		my->x = parent->x + 2.25 * cos(parent->yaw);
		my->y = parent->y + 2.25 * sin(parent->yaw);
		my->z = parent->z - 2.5;
		my->focalx = 0;
		my->focalz = 0.5;
		//my->pitch += 0.1;

		real_t& animRise = my->fskill[0];
		real_t& animPlay = my->fskill[3];
		my->pitch = 0.0;
		my->pitch -= animRise * 2 * PI;
		my->pitch += animPlay * 2 * PI;

		int timeLeft = parent->eternalShrineOrchestrionTimer & 0x00FFFF;
		int windupTime = (parent->eternalShrineOrchestrionTimer & 0xFF0000) >> 16;
		if ( timeLeft > 0 )
		{
			int interval = 100;
			int sprite = 16;
			if ( my->ticks % 65 == 0 )
			{
				interval = 80;
			}
			if ( windupTime < 3 * TICKS_PER_SECOND )
			{
				interval = 10;
			}
			if ( my->ticks % interval == 0 )
			{
				int i = my->ticks % (interval * 2) == 0;
				{
					if ( Entity* fx = spawnMagicParticleCustom(my, sprite, 0.1, 1.0) )
					{
						fx->flags[SPRITE] = true;
						fx->x = my->x + (i == 0 ? -1 : 1) * 3.0 * cos(my->yaw + PI / 2);
						fx->y = my->y + (i == 0 ? -1 : 1) * 3.0 * sin(my->yaw + PI / 2);
						fx->z = my->z - 2.0;
						fx->ditheringDisabled = true;
						fx->yaw = my->yaw + ((my->ticks % (interval * 8)) / 8) * PI / 4;
						fx->vel_z = -0.25;
						fx->fskill[4] = fx->x;
						fx->fskill[5] = fx->y;
						fx->behavior = &actGlow;
						if ( windupTime < 3 * TICKS_PER_SECOND )
						{
							fx->skill[1] = 1;
						}
						else
						{
							fx->skill[1] = 0;
						}
					}
				}
			}
		}
	}
	else if ( my->sprite == LIMB_HAMMER )
	{
		my->x = parent->x + 3.75 * cos(parent->yaw) + -3 * cos(parent->yaw + PI / 2);
		my->y = parent->y + 3.75 * sin(parent->yaw) + -3 * sin(parent->yaw + PI / 2);
		my->z = parent->z - 5.75;
		//my->focalx = 0;
		//my->focalz = 0.5;
		my->pitch = 0;
		my->yaw = parent->yaw;
		my->roll = 0;
		my->focalz = 2;

		real_t& animRise = my->fskill[0];
		real_t& animStrike = my->fskill[1];
		/*if ( keystatus[SDLK_b] )
		{
			keystatus[SDLK_b] = 0;
			animStrike = 4.0;
		}*/
		if ( my->eternalShrineState != parent->eternalShrineState )
		{
			my->eternalShrineState = parent->eternalShrineState;
			if ( my->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
			{
				animStrike = 4.0;
				playSoundEntityLocal(parent, 806, 128);
			}
		}

		real_t prevStrike = animStrike;
		animStrike -= 0.05;
		animStrike = std::max(0.0, animStrike);
		if ( prevStrike >= 1.25 && animStrike < 1.25 )
		{
			playSoundEntityLocal(parent, 885, 92);
			if ( multiplayer != CLIENT )
			{
				Uint32 lifetime = 50;
				Entity* spellTimer = createParticleTimer(parent, lifetime + TICKS_PER_SECOND, -1);
				spellTimer->particleTimerCountdownAction = PARTICLE_TIMER_ACTION_LIGHTNING_INSTANT;
				spellTimer->particleTimerCountdownSprite = 1757;
				spellTimer->yaw = parent->yaw;
				spellTimer->x = parent->x;
				spellTimer->y = parent->y;
				spellTimer->flags[NOUPDATE] = false; // spawn for client
				spellTimer->flags[UPDATENEEDED] = true;
				Sint32 val = (1 << 31);
				val |= (Uint8)(19);
				val |= (((Uint16)(spellTimer->particleTimerDuration) & 0xFFF) << 8);
				val |= (Uint8)(spellTimer->particleTimerCountdownAction & 0xFF) << 20;
				spellTimer->skill[2] = val;
				spellTimer->actmagicOrbitHitTargetUID1 = parent->getUID();
				spellTimer->particleTimerEffectLifetime = lifetime;

				Item* item = newItem(
					(ItemType)parent->eternalShrineItemType,
					(Status)parent->eternalShrineItemStatus,
					parent->eternalShrineItemBeatitude,
					parent->eternalShrineItemCount,
					(Uint32)parent->eternalShrineItemAppearance,
					parent->eternalShrineItemIdentified, nullptr);
				if ( Entity* dropped = dropItemMonster(item, parent, nullptr, item->count) )
				{
					dropped->x = parent->x;
					dropped->y = parent->y;
					dropped->z = -4.0;
					dropped->vel_z *= 0.5;
					dropped->x += 1.0 * cos(parent->yaw) - 1.0 * cos(parent->yaw + PI / 2);
					dropped->y += 1.0 * sin(parent->yaw) - 1.0 * sin(parent->yaw + PI / 2);
					dropped->itemEternalShrineResult = parent->eternalShrineType;
				}
				else
				{
					if ( item->node )
					{
						list_RemoveNode(item->node);
					}
					else
					{
						free(item);
					}
				}

				parent->eternalShrineItemType = 0;
				parent->eternalShrineItemStatus = 0;
				parent->eternalShrineItemBeatitude = 0;
				parent->eternalShrineItemCount = 0;
				parent->eternalShrineItemAppearance = 0;
				parent->eternalShrineItemIdentified = 0;

				parent->eternalShrineOfferingItemTypeModel = 0;
				serverUpdateEntitySkill(parent, 16);

				parent->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_END;
			}
		}

		if ( animStrike >= 2.0 )
		{
			my->roll -= (2 * PI + 0.5 * PI) * sin(2 * PI * (2.0 - (animStrike - 2.0)) / 8.0);
			my->z -= 1.5 * (2.0 - (animStrike - 2.0));
		}
		else if ( animStrike >= 1.25 )
		{
			my->roll -= (0.5 * PI) -(1 * PI) * (1.0 - (animStrike - 1.25) / 0.75);
			my->z -= 3.0 - 3.0 * (1.0 - (animStrike - 1.25) / 0.75);
		}
		else if ( animStrike >= 1.25 )
		{
			my->roll -= -0.5 * PI;
		}
		else
		{
			my->roll -= -0.5 * PI * sin(2 * PI * ((animStrike - 0.0) / 1.25) / 4.0);
		}

		if ( animStrike < 1.25 )
		{
			my->z -= 2.0 * sin(PI * (1.25 - animStrike) / 1.25);
		}

		Entity* interacting = uidToEntity(parent->eternalShrineInteracting);
		int shrineCurrentView = (int)(parent->eternalShrineOfferingItemTypeModel & 0xF);
		if ( (interacting 
			&& (shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_ACTION
				|| shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING)) 
			|| animStrike > 0.01
			|| parent->eternalShrineOfferingItemVisible >= GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
		{
			animRise += std::max((1.0 - animRise), 0.01) / 10.0;
			animRise = std::min(1.0, animRise);
		}
		else
		{
			if ( animStrike <= 0.0 )
			{
				animRise -= std::max(animRise, 0.01) / 10.0;
				animRise = std::max(0.0, animRise);
			}
		}
		my->roll += (PI + PI * 1 / 8) * animRise;
		my->yaw += (PI / 32) * animRise * sin((my->ticks % 100 / 50.0) * PI);
		my->pitch += (PI / 16) * animRise * sin((my->ticks % 100 / 50.0) * PI + PI / 2);
		my->z += -4 * animRise;

		real_t& animFloat = my->fskill[3];
		if ( animStrike >= 1.0 )
		{
			animFloat -= std::max(animFloat, 0.01) / 20.0;
			animFloat = std::max(0.0, animFloat);
		}
		else
		{
			animFloat += std::max((1.0 - animFloat), 0.01) / 20.0;
			animFloat = std::min(1.0, animFloat);
		}
		my->x -= (1.0 - animFloat) * 3.0 * cos(parent->yaw);
		my->y -= (1.0 - animFloat) * 3.0 * sin(parent->yaw);
		my->z += 0.5 * animRise * sin(((my->ticks % 200 / 100.0) * PI + PI / 2)) * animFloat;

		int interval = 25;
		int sprite = 16;
		if ( my->ticks % 80 == 0 )
		{
			interval = 80;
		}
		if ( animStrike >= 1.25 )
		{
			interval = 4;
			my->light = addLight(my->x / 16, my->y / 16, "orb_blue");
		}
		else
		{
			if ( animRise >= 0.05 )
			{
				my->light = addLight(my->x / 16, my->y / 16, "anvil_shrine_active");
			}
			else
			{
				my->light = addLight(my->x / 16, my->y / 16, "anvil_shrine");
			}
		}

		if ( interacting && my->ticks % interval == 0 )
		{
			if ( Entity* fx = spawnMagicParticleCustom(my, sprite, 0.1, 1.0) )
			{
				fx->flags[SPRITE] = true;
				fx->x = my->x + (1.5 - (my->focalz) * sin(my->roll)) * cos(my->yaw + PI / 2);
				fx->y = my->y + (1.5 - (my->focalz) * sin(my->roll)) * sin(my->yaw + PI / 2);
				fx->z = my->z - 0.5 + (my->focalz) * cos(my->roll);
				fx->ditheringDisabled = true;
				fx->yaw = my->yaw + ((my->ticks % (interval * 8)) / 8) * PI / 4;
				fx->vel_z = 0.25;
				fx->fskill[4] = fx->x;
				fx->fskill[5] = fx->y;
				fx->behavior = &actGlow;
				fx->skill[1] = animStrike >= 1.25 ? 1 : 0;
			}
		}
	}
}

void actEternalShrine(Entity* my)
{
	my->removeLightField();

	if ( !my->eternalShrineInit )
	{
		my->eternalShrineInit = 1;
		my->createWorldUITooltip();

		if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC )
		{
			for ( int i = 0; i < 5; ++i )
			{
				Entity* entity = newEntity(LIMB_COFFER + i, 1, map.entities, nullptr); //Sprite entity.
				entity->x = my->x;
				entity->y = my->y;
				entity->z = my->z;
				entity->behavior = &actEternalShrineLimb;
				entity->flags[PASSABLE] = true;
				entity->flags[NOUPDATE] = true;
				entity->flags[UPDATENEEDED] = false;
				entity->flags[UNCLICKABLE] = true;
				entity->flags[INVISIBLE] = true;
				entity->yaw = my->yaw;
				entity->parent = my->getUID();
				if ( multiplayer != CLIENT )
				{
					entity_uids--;
				}
				entity->setUID(-3);
			}
		}
		else if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
		{
			Entity* entity = newEntity(LIMB_HAMMER, 1, map.entities, nullptr); //Sprite entity.
			entity->x = my->x;
			entity->y = my->y;
			entity->z = my->z;
			entity->behavior = &actEternalShrineLimb;
			entity->flags[PASSABLE] = true;
			entity->flags[NOUPDATE] = true;
			entity->flags[UPDATENEEDED] = false;
			entity->flags[UNCLICKABLE] = true;
			entity->flags[INVISIBLE] = true;
			entity->yaw = my->yaw;
			entity->parent = my->getUID();
			if ( multiplayer != CLIENT )
			{
				entity_uids--;
			}
			entity->setUID(-3);
		}

		Entity* entity = newEntity(-1, 1, map.entities, nullptr); //Sprite entity.
		entity->x = my->x;
		entity->y = my->y;
		entity->z = my->z;
		entity->behavior = &actEternalShrineOffering;
		entity->flags[PASSABLE] = true;
		entity->flags[NOUPDATE] = true;
		entity->flags[UPDATENEEDED] = false;
		entity->flags[UNCLICKABLE] = true;
		entity->flags[INVISIBLE] = true;
		entity->yaw = my->yaw;
		entity->parent = my->getUID();
		if ( multiplayer != CLIENT )
		{
			entity_uids--;
		}
		entity->setUID(-3);
	}

	if ( my->eternalShrineLighting > 0 )
	{
		--my->eternalShrineLighting;
	}

#ifdef USE_FMOD
	if ( my->eternalShrineAmbience == 0 )
	{
		my->eternalShrineAmbience--;
		my->stopEntitySound();
		my->entity_sound = playSoundEntityLocal(my, 149, 16);
	}
	if ( my->entity_sound )
	{
		bool playing = false;
		my->entity_sound->isPlaying(&playing);
		if ( !playing )
		{
			my->entity_sound = nullptr;
		}
	}
#else
	my->eternalShrineAmbience--;
	if ( my->eternalShrineAmbience <= 0 )
	{
		my->eternalShrineAmbience = TICKS_PER_SECOND * 30;
		playSoundEntityLocal(my, 149, 16);
	}
#endif

	auto& ascensionTimer = my->fskill[0];
	auto& supplicationFlareType = my->fskill[1];
	if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION && my->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
	{
		real_t prev = ascensionTimer;
		ascensionTimer += 1.0;
		if ( prev <= 5.5 && ascensionTimer > 5.5 )
		{
			playSoundEntityLocal(my, 252, 128);
		}
	}
	else if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION && my->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
	{
		real_t prev = ascensionTimer;
		ascensionTimer += 1.0;
		if ( (int)supplicationFlareType < 3 )
		{
			supplicationFlareType = 3.0;
		}

		if ( prev <= 5.5 && ascensionTimer > 5.5 )
		{
			playSoundEntityLocal(my, 252, 128);
		}
	}
	else
	{
		if ( ascensionTimer > 0.01 )
		{
			ascensionTimer += 1;
		}
		if ( ascensionTimer >= 50.0 + *cvar_eternal_shrine_ascension_delay )
		{
			ascensionTimer = 0.0;
			supplicationFlareType = 0.0;
		}
	}

	if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC )
	{
		if ( my->eternalShrineOrchestrionTimer == 0 )
		{
			my->eternalShrineOrchestrionInstruments = 0;
		}
		if ( my->eternalShrineOrchestrionTimer != 0 )
		{
			Sint32 timeLeft = my->eternalShrineOrchestrionTimer & 0x00FFFF;
			timeLeft = std::max(0, timeLeft - 1);

			if ( timeLeft == 0 )
			{
				my->eternalShrineOrchestrionTimer = 0;
				my->eternalShrineOrchestrionInstruments = 0;
				serverUpdateEntitySkill(my, 20);
			}

			if ( my->eternalShrineOrchestrionTimer != 0 )
			{
				Uint32 mask = (0x00FFFF);
				my->eternalShrineOrchestrionTimer &= ~(mask); // zero out the timer
				my->eternalShrineOrchestrionTimer |= timeLeft & 0x00FFFF; // reapply timer

				Sint32 windupTime = (my->eternalShrineOrchestrionTimer & 0xFF0000) >> 16;
				if ( windupTime == 0 && timeLeft > 0 )
				{
					// start playing
					playSoundEntityLocal(my, 916, 128);
				}

				windupTime = std::min(4 * TICKS_PER_SECOND, windupTime + 1);
				if ( windupTime >= 4 * TICKS_PER_SECOND || windupTime == (3 * TICKS_PER_SECOND)
					|| windupTime == (3 * TICKS_PER_SECOND + 1) )
				{
					if ( windupTime == (3 * TICKS_PER_SECOND)
						|| windupTime == (3 * TICKS_PER_SECOND + 1) )
					{
						if ( Entity* fx = spawnDamageGib(my, combatmusicplaying ? 192 : 198, DamageGib::DMG_STRONGEST, DamageGibDisplayType::DMG_GIB_SPRITE) )
						{
							fx->z -= 10.0;
						}
					}
					else if ( ticks % 80 == 0 || ticks % 125 == 0 )
					{
						if ( Entity* fx = spawnDamageGib(my, combatmusicplaying ? 192 : 198, DamageGib::DMG_STRONGER, DamageGibDisplayType::DMG_GIB_SPRITE) )
						{
							fx->z -= 10.0;
						}
					}
				}

				mask = (0xFF0000);
				my->eternalShrineOrchestrionTimer &= ~(mask); // zero out the timer
				my->eternalShrineOrchestrionTimer |= (windupTime << 16) & 0xFF0000; // reapply timer
			}
		}
	}

	if ( multiplayer != CLIENT )
	{
		if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC && my->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
		{
			Sint32 windupTime = (my->eternalShrineOrchestrionTimer & 0xFF0000) >> 16;
			if ( windupTime == 3 * TICKS_PER_SECOND )
			{
				if ( my->eternalShrineItemCount > 0 )
				{
					Item* item = newItem(
						(ItemType)my->eternalShrineItemType,
						(Status)my->eternalShrineItemStatus,
						my->eternalShrineItemBeatitude,
						my->eternalShrineItemCount,
						(Uint32)my->eternalShrineItemAppearance,
						my->eternalShrineItemIdentified, nullptr);

					if ( item )
					{
						if ( Entity* dropped = dropItemMonster(item, my, nullptr, item->count) )
						{
							dropped->x = my->x;
							dropped->y = my->y;
							dropped->z = -4.0;
							dropped->yaw = my->yaw + PI;
							dropped->x += 4.0 * cos(my->yaw) - 0.0 * cos(my->yaw + PI / 2);
							dropped->y += 4.0 * sin(my->yaw) - 0.0 * sin(my->yaw + PI / 2);
							dropped->vel_x = 0.0;
							dropped->vel_y = 0.0;
							dropped->itemEternalShrineResult = my->eternalShrineType;

							dropped->itemNotMoving = 0;
							dropped->itemNotMovingClient = 0;
							//dropped->vel_z = -0.25;
							dropped->itemLevitate = 1.0;
							dropped->itemLevitateStartZ = dropped->z;

							playSoundEntity(my, 909, 128);
						}
						else
						{
							if ( item->node )
							{
								list_RemoveNode(item->node);
							}
							else
							{
								free(item);
							}
						}
					}
				}

				if ( my->eternalShrineOrchestrionInstruments != 0 )
				{
					if ( my->eternalShrineOrchestrionInstruments & (1 << 31) )
					{
						// reapply music
						my->eternalShrineOrchestrionInstruments &= ~(1 << 31); // unset the bit
						shrineApplyMusic(my);
					}
				}

				my->eternalShrineItemType = 0;
				my->eternalShrineItemStatus = 0;
				my->eternalShrineItemBeatitude = 0;
				my->eternalShrineItemCount = 0;
				my->eternalShrineItemAppearance = 0;
				my->eternalShrineItemIdentified = 0;

				my->eternalShrineOfferingItemTypeModel = 0;
				serverUpdateEntitySkill(my, 16);

				my->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_END;
			}
		}
		else if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION && my->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
		{
			if ( ascensionTimer >= 25.0 + *cvar_eternal_shrine_ascension_delay )
			{
				my->eternalShrineItemType = 0;
				my->eternalShrineItemStatus = 0;
				my->eternalShrineItemBeatitude = 0;
				my->eternalShrineItemCount = 0;
				my->eternalShrineItemAppearance = 0;
				my->eternalShrineItemIdentified = 0;

				my->eternalShrineOfferingItemTypeModel = 0;
				serverUpdateEntitySkill(my, 16);

				my->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_END;
			}
		}
		else if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION && my->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
		{
			if ( ascensionTimer >= 25.0 + *cvar_eternal_shrine_ascension_delay )
			{
				Item* item = newItem(
					(ItemType)my->eternalShrineItemType,
					(Status)my->eternalShrineItemStatus,
					my->eternalShrineItemBeatitude,
					my->eternalShrineItemCount,
					(Uint32)my->eternalShrineItemAppearance,
					my->eternalShrineItemIdentified, nullptr);

				if ( item )
				{
					if ( item->type == TOME_SORCERY )
					{
						my->eternalShrineViewingMode = GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::SORCERY_SPELL;
					}
					else if ( item->type == TOME_MYSTICISM )
					{
						my->eternalShrineViewingMode = GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::MYSTICISM_SPELL;
					}
					else if ( item->type == TOME_THAUMATURGY )
					{
						my->eternalShrineViewingMode = GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::THAUMATURGY_SPELL;
					}
					serverUpdateEntitySkill(my, 7);
					if ( Entity* dropped = dropItemMonster(item, my, nullptr, item->count) )
					{
						dropped->x = my->x;
						dropped->y = my->y;
						dropped->z = -4.0;
						dropped->yaw = my->yaw + PI;
						dropped->x += 0.25 * cos(my->yaw) - 0.0 * cos(my->yaw + PI / 2);
						dropped->y += 0.25 * sin(my->yaw) - 0.0 * sin(my->yaw + PI / 2);
						dropped->itemEternalShrineResult = my->eternalShrineType;

						dropped->itemNotMoving = 0;
						dropped->itemNotMovingClient = 0;
						//dropped->vel_z = -0.25;
						dropped->itemLevitate = 1.0;
						dropped->itemLevitateStartZ = dropped->z;
					}
					else
					{
						if ( item->node )
						{
							list_RemoveNode(item->node);
						}
						else
						{
							free(item);
						}
					}
				}

				my->eternalShrineItemType = 0;
				my->eternalShrineItemStatus = 0;
				my->eternalShrineItemBeatitude = 0;
				my->eternalShrineItemCount = 0;
				my->eternalShrineItemAppearance = 0;
				my->eternalShrineItemIdentified = 0;

				my->eternalShrineOfferingItemTypeModel = 0;
				serverUpdateEntitySkill(my, 16);

				my->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_END;
			}
		}

		if ( my->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_END )
		{
			my->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_NONE;
			my->eternalShrineTarget = 0;
			serverUpdateEntitySkill(my, 4); // eternalShrineState
		}
	}

	Entity* interacting = uidToEntity(my->eternalShrineInteracting);
	if ( !interacting )
	{
		if ( multiplayer != CLIENT )
		{
			if ( my->eternalShrineOfferingItemTypeModel != 0 )
			{
				my->eternalShrineOfferingItemTypeModel = 0;
				serverUpdateEntitySkill(my, 16);
			}
		}
	}

	if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
	{
		// left
		int interval = 8;
		if ( !interacting )
		{
			interval = 25;
			if ( my->ticks % 80 == 0 )
			{
				interval = 80;
			}
		}

		if ( my->ticks % interval == 0 )
		{
			if ( Entity* fx = spawnMagicParticleCustom(my, 16, 0.1, 1.0) )
			{
				fx->flags[SPRITE] = true;
				fx->x = my->x - 2.25 * cos(my->yaw) + 2.5 * cos(my->yaw + PI / 2);
				fx->y = my->y - 2.25 * sin(my->yaw) + 2.5 * sin(my->yaw + PI / 2);
				fx->z = my->z + -5;
				fx->ditheringDisabled = true;
				fx->yaw = my->yaw + ((my->ticks % (interval * 8)) / 8) * PI / 2;
				fx->vel_z = -0.5;
				fx->fskill[4] = my->x + 2 * cos(my->yaw);
				fx->fskill[5] = my->y + 2 * sin(my->yaw);
				fx->behavior = &actGlow;
				fx->skill[1] = interacting ? 1 : 0;
			}
		}
	}
	else if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
	{
		int interval = 8;
		if ( !interacting )
		{
			interval = 25;
			if ( my->ticks % 80 == 0 )
			{
				interval = 80;
			}
		}

		int viewingMode = my->eternalShrineAscensionItemColor > 0 ? my->eternalShrineAscensionItemColor : my->eternalShrineViewingMode;

		if ( my->ticks % interval == 0 )
		{
			if ( !interacting )
			{
			}

			bool particle = true;
			if ( !interacting )
			{
				particle = (my->ticks % (interval * 3) / interval) == 0;
			}

			if ( particle )
			{
				int sprite = 160;
				if ( !interacting 
					|| (interacting && viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::ASCENSION_SPELL) )
				{
					if ( (my->ticks % (interval * 4) / interval) % 4 != 0 )
					{
						sprite = 16;
					}
				}
				if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::SORCERY_SPELL )
				{
					sprite = 225;
				}
				else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::MYSTICISM_SPELL )
				{
					sprite = 261;
				}
				else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::THAUMATURGY_SPELL )
				{
					sprite = 160;
				}

				//back center
				if ( Entity* fx = spawnMagicParticleCustom(my, sprite, 0.1, 1.0) )
				{
					fx->flags[SPRITE] = true;
					fx->x = my->x + -2.25 * cos(my->yaw) + 0 * cos(my->yaw + PI / 2);
					fx->y = my->y + -2.25 * sin(my->yaw) + 0 * sin(my->yaw + PI / 2);
					fx->z = my->z + -11.25;
					fx->ditheringDisabled = true;
					fx->yaw = my->yaw + ((my->ticks % (interval * 8)) / 8) * PI / 2;
					fx->vel_z = 0.25;
					fx->fskill[4] = my->x;
					fx->fskill[5] = my->y;
					fx->behavior = &actGlow;
					fx->skill[1] = interacting ? 1 : 0;
				}
			}

			particle = true;
			if ( !interacting )
			{
				particle = (my->ticks % (interval * 3) / interval) == 1;
			}

			if ( particle )
			{
				int sprite = 225;
				if ( !interacting
					|| (interacting && viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::ASCENSION_SPELL) )
				{
					if ( (my->ticks % (interval * 4) / interval) % 4 != 0 )
					{
						sprite = 16;
					}
				}
				if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::SORCERY_SPELL )
				{
					sprite = 225;
				}
				else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::MYSTICISM_SPELL )
				{
					sprite = 261;
				}
				else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::THAUMATURGY_SPELL )
				{
					sprite = 160;
				}

				// left
				if ( Entity* fx = spawnMagicParticleCustom(my, sprite, 0.1, 1.0) )
				{
					fx->flags[SPRITE] = true;
					fx->x = my->x + 1.75 * cos(my->yaw) + 2.5 * cos(my->yaw + PI / 2);
					fx->y = my->y + 1.75 * sin(my->yaw) + 2.5 * sin(my->yaw + PI / 2);
					fx->z = my->z + -14.25;
					fx->ditheringDisabled = true;
					fx->yaw = my->yaw + ((my->ticks % (interval * 8)) / 8) * PI / 2;
					fx->vel_z = 0.35;
					fx->fskill[4] = my->x;
					fx->fskill[5] = my->y;
					fx->behavior = &actGlow;
					fx->skill[1] = interacting ? 1 : 0;
				}
			}

			particle = true;
			if ( !interacting )
			{
				particle = (my->ticks % (interval * 3) / interval) == 2;
			}

			if ( particle )
			{
				int sprite = 261;
				if ( !interacting
					|| (interacting && viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::ASCENSION_SPELL) )
				{
					if ( (my->ticks % (interval * 4) / interval) % 4 != 0 )
					{
						sprite = 16;
					}
				}
				if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::SORCERY_SPELL )
				{
					sprite = 225;
				}
				else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::MYSTICISM_SPELL )
				{
					sprite = 261;
				}
				else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::THAUMATURGY_SPELL )
				{
					sprite = 160;
				}

				// right
				if ( Entity* fx = spawnMagicParticleCustom(my, sprite, 0.1, 1.0) )
				{
					fx->flags[SPRITE] = true;
					fx->x = my->x + 1.75 * cos(my->yaw) + -2.5 * cos(my->yaw + PI / 2);
					fx->y = my->y + 1.75 * sin(my->yaw) + -2.5 * sin(my->yaw + PI / 2);
					fx->z = my->z + -12.75;
					fx->ditheringDisabled = true;
					fx->yaw = my->yaw + ((my->ticks % (interval * 8)) / 8) * PI / 2;
					fx->vel_z = 0.25;
					fx->fskill[4] = my->x;
					fx->fskill[5] = my->y;
					fx->behavior = &actGlow;
					fx->skill[1] = interacting ? 1 : 0;
				}
			}
		}
		if ( ascensionTimer >= 1.0 )
		{
			my->eternalShrineLighting = TICKS_PER_SECOND;
		}
		if ( ascensionTimer >= 20.0 + *cvar_eternal_shrine_ascension_delay && ascensionTimer <= 25.0 + *cvar_eternal_shrine_ascension_delay )
		{
			if ( (int)floor(ascensionTimer) == (int)(20 + *cvar_eternal_shrine_ascension_delay) )
			{
				if ( Entity* fx = createParticleAOEIndicator(my, my->x, my->y, 0.0, TICKS_PER_SECOND, 16.0) )
				{
					fx->scalex = 0.8;
					fx->scaley = 0.8;
					if ( auto indicator = AOEIndicators_t::getIndicator(fx->skill[10]) )
					{
						//indicator->arc = PI / 2;
						Uint32 color = makeColorRGB(255, 255, 255);
						if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::SORCERY_SPELL )
						{
							color = makeColorRGB(255, 128, 0);
						}
						else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::MYSTICISM_SPELL )
						{
							color = makeColorRGB(132, 47, 241);
						}
						indicator->indicatorColor = color;
						indicator->loop = false;
						indicator->gradient = 2;
						indicator->framesPerTick = 2;
						indicator->ticksPerUpdate = 1;
						indicator->delayTicks = 0;
						indicator->expireAlphaRate = 0.95;
						indicator->cacheType = AOEIndicators_t::CACHE_ETERNAL_SHRINE;
					}
				}
			}

			int sprite = 1866;
			if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::SORCERY_SPELL )
			{
				sprite = 2543;
			}
			else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::MYSTICISM_SPELL )
			{
				sprite = 2374;
			}
			for ( int i = 0; i < 4; ++i )
			{
				if ( Entity* fx = createParticleAestheticOrbit(my, sprite, 2 * TICKS_PER_SECOND, PARTICLE_EFFECT_FOCI_LIGHT) )
				{
					fx->yaw = i * PI / 2 + 3 * PI / 4;
					fx->fskill[2] = 7.75; // start z
					fx->fskill[3] = -16.25; // end z
					fx->fskill[6] = 0.3; // yaw speed
					fx->fskill[7] = 0.1; // yaw scale?
					fx->z = 7.75;
					fx->flags[INVISIBLE] = true;
					fx->scaley = 1.0;
				}
			}
		}

		if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::ASCENSION_SPELL )
		{
			my->light = addLight(my->x / 16, my->y / 16, my->eternalShrineLighting > 0 ? "ascension_shrine_flicker" : "ascension_shrine");
		}
		else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::SORCERY_SPELL )
		{
			my->light = addLight(my->x / 16, my->y / 16, my->eternalShrineLighting > 0 ? "ascension_shrine_sorcery_flicker" : "ascension_shrine_sorcery");
		}
		else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::MYSTICISM_SPELL )
		{
			my->light = addLight(my->x / 16, my->y / 16, my->eternalShrineLighting > 0 ? "ascension_shrine_mysticism_flicker" : "ascension_shrine_mysticism");
		}
		else if ( viewingMode == GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::THAUMATURGY_SPELL )
		{
			my->light = addLight(my->x / 16, my->y / 16, my->eternalShrineLighting > 0 ? "ascension_shrine_thaumaturgy_flicker" : "ascension_shrine_thaumaturgy");
		}
	}
	else if ( my->eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
	{
		// spew flame particles
		if ( my->ticks % 30 == 0 )
		{
			if ( Entity* fx = createParticleAestheticOrbit(my, supplicationFlareType <= 2.0 ? 233 : 288, 10, PARTICLE_EFFECT_FLAMES_BURNING) )
			{
				fx->flags[SPRITE] = true;
				fx->flags[INVISIBLE] = true;
				fx->x = my->x;
				fx->y = my->y;
				fx->z = my->z;
				fx->scalex = 1.0;
				fx->scaley = fx->scalex;
				fx->scalez = fx->scalex;
				fx->fskill[0] = fx->x;
				fx->fskill[1] = fx->y;
				fx->vel_z = -0.25;
				fx->actmagicOrbitDist = 0;
				fx->fskill[2] = my->yaw + (local_rng.rand() % 8) * PI / 4.0;
				fx->yaw = fx->fskill[2];
				fx->actmagicNoLight = 1;
			}
		}

		int shrineCurrentView = (int)(my->eternalShrineOfferingItemTypeModel & 0xF);
		if ( (interacting
			&& (shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_ACTION
				|| shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING))
			|| my->eternalShrineOfferingItemVisible >= GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
		{
			if ( (int)supplicationFlareType == 0 )
			{
				supplicationFlareType = 1.0;
			}

			if ( my->ticks % 25 == 0 )
			{
				if ( Entity* fx = createParticleAestheticOrbit(my, supplicationFlareType <= 2.0 ? 233 : 288, 25, PARTICLE_EFFECT_IGNITE_ORBIT) )
				{
					fx->x = my->x;
					fx->y = my->y;
					fx->fskill[0] = fx->x;
					fx->fskill[1] = fx->y;
					fx->flags[SPRITE] = true;
					int dir = ((my->ticks % (25 * 4)) % 4) * 2;
					fx->fskill[2] = (dir / 8.0) * 2 * PI + PI / 8;
					fx->fskill[3] += (local_rng.rand() % 10) * PI / 10.0;
					fx->z = 2.0;
					fx->vel_z = -0.1 + (local_rng.rand() % 10) * -.025;
					fx->actmagicOrbitDist = 0;
					fx->actmagicNoLight = 1;
				}
			}
		}
		else
		{
			if ( (int)supplicationFlareType <= 2 )
			{
				supplicationFlareType = 0.0;
			}
		}

		bool flareUp = (int)supplicationFlareType == 1;
		if ( ascensionTimer >= 20.0 + *cvar_eternal_shrine_ascension_delay && ascensionTimer <= 25.0 + *cvar_eternal_shrine_ascension_delay )
		{
			if ( (int)floor(ascensionTimer) == (int)(20 + *cvar_eternal_shrine_ascension_delay) )
			{
				flareUp = true;
			}
		}
		if ( flareUp )
		{
			if ( (int)supplicationFlareType == 1 )
			{
				supplicationFlareType = 2.0;
			}
			if ( (int)supplicationFlareType == 3 )
			{
				supplicationFlareType = 4.0;
			}

			if ( supplicationFlareType <= 2.0 )
			{
				playSoundEntityLocal(my, 827, 128);
			}
			else
			{
				playSoundEntityLocal(my, 914, 128);
			}
		}


		if ( flareUp )
		{
			my->eternalShrineLighting = TICKS_PER_SECOND;
			for ( int i = 0; i < 4; ++i )
			{
				if ( Entity* fx = createParticleAestheticOrbit(my, supplicationFlareType <= 2.0 ? 233 : 288, 25, PARTICLE_EFFECT_IGNITE_ORBIT) )
				{
					fx->x = my->x;
					fx->y = my->y;
					fx->fskill[0] = fx->x;
					fx->fskill[1] = fx->y;
					fx->flags[SPRITE] = true;
					fx->fskill[2] = (i / 4.0) * 2 * PI + PI / 4;
					fx->fskill[3] += (local_rng.rand() % 10) * PI / 10.0;
					fx->z = 0.0;
					fx->vel_z = -0.2 + (local_rng.rand() % 10) * -.025;
					fx->actmagicOrbitDist = 4;
					fx->actmagicNoLight = 1;
				}
			}
		}

		int interval = interacting ? 8 : 25;
		if ( my->ticks % 80 == 0 )
		{
			interval = 80;
		}

		if ( my->ticks % interval == 0 )
		{
			int i = (my->ticks % (interval * 4) / interval);
			//for ( int i = 0; i < 4; ++i )
			{
				if ( Entity* fx = spawnMagicParticleCustom(my, 16, 0.1, 1.0) )
				{
					fx->flags[SPRITE] = true;
					real_t dir = i * PI / 2 + PI / 4;
					fx->x = my->x + -4 * cos(dir);
					fx->y = my->y + -4 * sin(dir);
					fx->z = 0.0;
					fx->ditheringDisabled = true;
					fx->yaw = my->yaw + ((my->ticks % (interval * 8)) / 8) * PI / 2;
					fx->vel_z = -0.25;
					fx->fskill[4] = my->x;
					fx->fskill[5] = my->y;
					fx->behavior = &actGlow;
					fx->skill[1] = interacting ? 1 : 0;
				}
			}
		}

		// light environment
		if ( my->eternalShrineLighting > 0 )
		{
			if ( supplicationFlareType >= 3.0 )
			{
				my->light = addLight(my->x / 16, my->y / 16, "magic_blue");
			}
			else
			{
				my->light = addLight(my->x / 16, my->y / 16, "supplication_shrine_flicker");
			}
		}
		else if ( supplicationFlareType >= 3.0 )
		{
			if ( supplicationFlareType >= 3.0 )
			{
				my->light = addLight(my->x / 16, my->y / 16, "magic_blue_flicker");
			}
			else
			{
				my->light = addLight(my->x / 16, my->y / 16, "supplication_shrine_flicker");
			}
		}
		else if ( interacting )
		{
			my->light = addLight(my->x / 16, my->y / 16, "supplication_shrine_flash");
		}
		else
		{
			my->light = addLight(my->x / 16, my->y / 16, "supplication_shrine");
		}
	}

	if ( multiplayer == CLIENT )
	{
		return;
	}

	if ( my->eternalShrineInteracting > 0 )
	{
		if ( !interacting || (entityDist(interacting, my) > TOUCHRANGE && my->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_NONE) )
		{
			int playernum = -1;
			if ( !interacting )
			{
				for ( int i = 0; i < MAXPLAYERS; ++i )
				{
					if ( achievementObserver.playerUids[i] == my->eternalShrineInteracting )
					{
						playernum = i;
						break;
					}
				}
			}
			else if ( interacting->behavior == &actPlayer )
			{
				playernum = interacting->skill[2];
			}
			my->eternalShrineInteracting = 0;
			serverUpdateEntitySkill(my, 6);
			if ( multiplayer == SERVER && playernum > 0 )
			{
				strcpy((char*)net_packet->data, "ESHC");
				net_packet->data[4] = playernum;
				SDLNet_Write32(my->getUID(), &net_packet->data[5]);
				net_packet->address.host = net_clients[playernum - 1].host;
				net_packet->address.port = net_clients[playernum - 1].port;
				net_packet->len = 9;
				sendPacketSafe(net_sock, -1, net_packet, playernum - 1);
			}
			else if ( multiplayer == SINGLE || playernum == 0 )
			{
				if ( playernum >= 0 && playernum < MAXPLAYERS )
				{
					GenericGUI[playernum].eternalShrineGUI.closeEternalShrine();
				}
			}
		}
	}

	// using
	for ( int i = 0; i < MAXPLAYERS; i++ )
	{
		if ( selectedEntity[i] == my || client_selected[i] == my )
		{
			if ( inrange[i] && players[i]->entity )
			{
				if ( my->eternalShrineInteracting != 0 || my->eternalShrineState >= GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
				{
					if ( Entity* interacting = uidToEntity(my->eternalShrineInteracting) )
					{
						if ( interacting != players[i]->entity )
						{
							messagePlayer(i, MESSAGE_INTERACTION, Language::get(7088));
						}
					}
				}
				else
				{
					my->eternalShrineInteracting = players[i]->entity->getUID();
					if ( multiplayer == SERVER )
					{
						serverUpdateEntitySkill(my, 6);
					}
					if ( players[i]->isLocalPlayer() )
					{
						if ( my->eternalShrineType >= GUI_TYPE_ETERNALSHRINE_ANVIL && my->eternalShrineType <= GUI_TYPE_ETERNALSHRINE_ASCENSION )
						{
							GenericGUI[i].openGUI(my->eternalShrineType, my);
							if ( my->eternalShrineViewingMode != GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::ASCENSION_SPELL )
							{
								my->eternalShrineViewingMode = GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::ASCENSION_SPELL;
								serverUpdateEntitySkill(my, 7);
							}
						}
					}
					else if ( multiplayer == SERVER && i > 0 )
					{
						if ( !client_disconnected[i] )
						{
							strcpy((char*)net_packet->data, "ESHO");
							SDLNet_Write32(my->getUID(), &net_packet->data[4]);
							SDLNet_Write16(players[i]->mechanics.divine_favor, &net_packet->data[8]);
							SDLNet_Write32(my->eternalShrinePlayerStates, &net_packet->data[10]);
							net_packet->address.host = net_clients[i - 1].host;
							net_packet->address.port = net_clients[i - 1].port;
							net_packet->len = 14;
							sendPacketSafe(net_sock, -1, net_packet, i - 1);

							if ( my->eternalShrineViewingMode != GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::ASCENSION_SPELL )
							{
								my->eternalShrineViewingMode = GenericGUIMenu::EternalShrineGUI_t::EternalShrineAscensionType::ASCENSION_SPELL;
								serverUpdateEntitySkill(my, 7);
							}
						}

						for ( int c = 1; c < MAXPLAYERS; c++ )
						{
							if ( client_disconnected[c] || players[c]->isLocalPlayer() )
							{
								continue;
							}
							if ( c == i )
							{
								continue; // dont respond to original sender
							}
							strcpy((char*)net_packet->data, "ENTS");
							SDLNet_Write32(my->getUID(), &net_packet->data[4]);
							net_packet->data[8] = 17; // update eternalShrinePlayerStates
							SDLNet_Write32(my->eternalShrinePlayerStates, &net_packet->data[9]);
							net_packet->address.host = net_clients[c - 1].host;
							net_packet->address.port = net_clients[c - 1].port;
							net_packet->len = 13;
							sendPacketSafe(net_sock, -1, net_packet, c - 1);
						}
					}
				}
				break;
			}
		}
	}
}

bool eternalShrineProcessOfferingItem(const int player, Uint32 shrineUid, int shrineType, Item* item)
{
	if ( player < 0 || player >= MAXPLAYERS ) { return false; }
	if ( multiplayer == CLIENT ) { return false; }

	Entity* shrine = shrineUid ? uidToEntity(shrineUid) : nullptr;

	players[player]->mechanics.divine_favor += 1 + local_rng.rand() % 3;
	players[player]->mechanics.divine_favor = std::min(players[player]->mechanics.divine_favor, Player::DIVINE_FAVOR_MAX);

	if ( shrine )
	{
		int playerProgress = (shrine->eternalShrinePlayerStates >> (player * 2)) & 0b11;
		if ( !players[player]->isLocalPlayer() )
		{
			playerProgress = GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_CONFIRMED;
		}
		else if ( playerProgress == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_WAITING
			|| playerProgress == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE )
		{
			playerProgress = GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_CONFIRMED;
		}
		Uint32 newvalue = playerProgress << (player * 2);
		Uint32 mask = (0b11) << (player * 2);
		shrine->eternalShrinePlayerStates &= ~(mask); // zero out the player slot
		shrine->eternalShrinePlayerStates |= newvalue; // apply new value

		if ( multiplayer == SERVER )
		{
			for ( int c = 1; c < MAXPLAYERS; c++ )
			{
				if ( client_disconnected[c] || players[c]->isLocalPlayer() )
				{
					continue;
				}
				if ( c == player )
				{
					continue; // dont respond to original sender
				}
				strcpy((char*)net_packet->data, "ENTS");
				SDLNet_Write32(shrineUid, &net_packet->data[4]);
				net_packet->data[8] = 17; // update eternalShrinePlayerStates
				SDLNet_Write32(shrine->eternalShrinePlayerStates, &net_packet->data[9]);
				net_packet->address.host = net_clients[c - 1].host;
				net_packet->address.port = net_clients[c - 1].port;
				net_packet->len = 13;
				sendPacketSafe(net_sock, -1, net_packet, c - 1);
			}
		}
	}

	if ( multiplayer == SERVER && !players[player]->isLocalPlayer() && player > 0 )
	{
		if ( !client_disconnected[player] )
		{
			strcpy((char*)net_packet->data, "ESHF");
			net_packet->data[4] = player;
			SDLNet_Write32(shrineUid, &net_packet->data[5]);
			SDLNet_Write16(players[player]->mechanics.divine_favor, &net_packet->data[9]);
			SDLNet_Write32(shrine ? shrine->eternalShrinePlayerStates : 0, &net_packet->data[11]);

			net_packet->address.host = net_clients[player - 1].host;
			net_packet->address.port = net_clients[player - 1].port;
			net_packet->len = 15;
			sendPacketSafe(net_sock, -1, net_packet, player - 1);
		}
	}

	return true;
}

bool eternalShrineProcessSupplication(const int player, Uint32 shrineUid, int shrineType)
{
	bool error = false;
	if ( player < 0 || player >= MAXPLAYERS ) { error = true; }
	if ( multiplayer == CLIENT ) { error = true; }
	if ( shrineType != GUI_TYPE_ETERNALSHRINE_SUPPLICATION ) { error = true; }

	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { error = true; }
	if ( shrine && (shrine->eternalShrineType != shrineType) ) { error = true; }

	if ( error )
	{
		if ( shrine )
		{
			shrine->eternalShrineState = 0;
			serverUpdateEntitySkill(shrine, 4); // eternalShrineState
			shrine->eternalShrineOfferingItemTypeModel = 0;
			serverUpdateEntitySkill(shrine, 16);
		}
		return false;
	}

	shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE;
	shrine->eternalShrineTarget = achievementObserver.playerUids[player];
	serverUpdateEntitySkill(shrine, 4); // eternalShrineState

	//shrine->eternalShrineItemType = item->type;
	//shrine->eternalShrineItemStatus = item->status;
	//shrine->eternalShrineItemBeatitude = item->beatitude;
	//shrine->eternalShrineItemCount = item->count;
	//shrine->eternalShrineItemAppearance = item->appearance;
	//shrine->eternalShrineItemIdentified = item->identified;

	Sint32 offeringTypeModel = 0;
	//int sprite = itemModel(item);
	//Sint32 offeringTypeModel = (sprite & 0xFFFF) << 16;
	//offeringTypeModel |= (int)(item->type & 0x0FFF) << 4;

	offeringTypeModel |= ((GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING) & 0xF);
	shrine->eternalShrineOfferingItemTypeModel = offeringTypeModel;
	serverUpdateEntitySkill(shrine, 16);

	return true;
}

bool eternalShrineProcessMusic(const int player, Uint32 shrineUid, int shrineType)
{
	bool error = false;
	if ( player < 0 || player >= MAXPLAYERS ) { error = true; }
	if ( multiplayer == CLIENT ) { error = true; }
	if ( shrineType != GUI_TYPE_ETERNALSHRINE_MUSIC ) { error = true; }

	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { error = true; }
	if ( shrine && (shrine->eternalShrineType != shrineType) ) { error = true; }

	if ( error )
	{
		if ( shrine )
		{
			shrine->eternalShrineState = 0;
			serverUpdateEntitySkill(shrine, 4); // eternalShrineState
			shrine->eternalShrineOfferingItemTypeModel = 0;
			serverUpdateEntitySkill(shrine, 16);
		}
		return false;
	}

	shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE;
	shrine->eternalShrineTarget = achievementObserver.playerUids[player];
	serverUpdateEntitySkill(shrine, 4); // eternalShrineState

	shrine->eternalShrineItemType = 0;// item->type;
	shrine->eternalShrineItemStatus = 0;// item->status;
	shrine->eternalShrineItemBeatitude = 0;// item->beatitude;
	shrine->eternalShrineItemCount = 0;// item->count;
	shrine->eternalShrineItemAppearance = 0;// item->appearance;
	shrine->eternalShrineItemIdentified = 0;// item->identified;

	std::vector<std::pair<int, int>> instrumentsPlaying = {
		{EFF_ENSEMBLE_FLUTE, 0},
		{EFF_ENSEMBLE_LYRE, 0},
		{EFF_ENSEMBLE_DRUM, 0},
		{EFF_ENSEMBLE_LUTE, 0},
		{EFF_ENSEMBLE_HORN, 0}
	};
	std::vector<unsigned int> chances {1, 1, 1, 1, 1};
	Sint32 timeLeft = shrine->eternalShrineOrchestrionTimer & 0x00FFFF;
	if ( timeLeft )
	{
		if ( ((shrine->eternalShrineOrchestrionInstruments >> 0) & 0xF) )
		{
			instrumentsPlaying[FLUTE].second = ((shrine->eternalShrineOrchestrionInstruments >> 0) & 0xF);
			chances[FLUTE] = 0;
		}
		if ( ((shrine->eternalShrineOrchestrionInstruments >> 4) & 0xF) )
		{
			instrumentsPlaying[LYRE].second = ((shrine->eternalShrineOrchestrionInstruments >> 4) & 0xF);
			chances[LYRE] = 0;
		}
		if ( ((shrine->eternalShrineOrchestrionInstruments >> 8) & 0xF) )
		{
			instrumentsPlaying[DRUM].second = ((shrine->eternalShrineOrchestrionInstruments >> 8) & 0xF);
			chances[DRUM] = 0;
		}
		if ( ((shrine->eternalShrineOrchestrionInstruments >> 12) & 0xF) )
		{
			instrumentsPlaying[LUTE].second = ((shrine->eternalShrineOrchestrionInstruments >> 12) & 0xF);
			chances[LUTE] = 0;
		}
		if ( ((shrine->eternalShrineOrchestrionInstruments >> 16) & 0xF) )
		{
			instrumentsPlaying[HORN].second = ((shrine->eternalShrineOrchestrionInstruments >> 16) & 0xF);
			chances[HORN] = 0;
		}
	}

	bool anychances = false;
	for ( auto chance : chances )
	{
		if ( chance )
		{
			anychances = true;
		}
	}

	shrine->eternalShrineOrchestrionInstruments = 0;

	auto& rng = shrine->entity_rng ? *shrine->entity_rng : local_rng;

	bool receiveItem = local_rng.rand() % 4 == 0;

	if ( anychances )
	{
		int pick = rng.discrete(chances.data(), chances.size());
		int tier = 1 + local_rng.rand() % 4; // 1-4
		instrumentsPlaying[pick].second = std::max(tier, instrumentsPlaying[pick].second);
	}
	else
	{
		receiveItem = true;
	}


	int index = -1;
	for ( auto eff : instrumentsPlaying )
	{
		++index;
		Uint8 tier = eff.second;
		if ( tier ) // tier
		{
			shrine->eternalShrineOrchestrionInstruments |= (tier & 0xF) << (index * 4);
		}
	}

	if ( !receiveItem )
	{
		if ( shrine->eternalShrineOrchestrionInstruments != 0 )
		{
			shrine->eternalShrineOrchestrionInstruments |= (1 << 31); // signal to reapply music
		}
		int duration = 20 * TICKS_PER_SECOND;
		shrine->eternalShrineOrchestrionTimer = std::max(timeLeft, duration);
	}
	else
	{
		if ( timeLeft )
		{
			Uint32 mask = 0xFFFFFF;
			shrine->eternalShrineOrchestrionTimer &= ~(mask); // replay the windup + timeleft
			shrine->eternalShrineOrchestrionTimer = std::max(timeLeft, 4 * TICKS_PER_SECOND);
		}
		else
		{
			shrine->eternalShrineOrchestrionTimer = 4 * TICKS_PER_SECOND;
		}

		shrine->eternalShrineItemType = INSTRUMENT_FLUTE + rng.rand() % 5;
		shrine->eternalShrineItemStatus = EXCELLENT;
		shrine->eternalShrineItemBeatitude = 0;
		shrine->eternalShrineItemCount = 1;
		shrine->eternalShrineItemAppearance = rng.rand();
		shrine->eternalShrineItemIdentified = 0;
	}

	serverUpdateEntitySkill(shrine, 20); // eternalShrineOrchestrionTimer

	Sint32 offeringTypeModel = 0;
	//int sprite = itemModel(item);
	//Sint32 offeringTypeModel = (sprite & 0xFFFF) << 16;
	//offeringTypeModel |= (int)(item->type & 0x0FFF) << 4;

	offeringTypeModel |= ((GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING) & 0xF);
	shrine->eternalShrineOfferingItemTypeModel = offeringTypeModel;
	serverUpdateEntitySkill(shrine, 16);

	return true;
}

bool eternalShrineProcessAnvilItem(const int player, Uint32 shrineUid, int shrineType, Item* item)
{
	bool error = false;
	if ( player < 0 || player >= MAXPLAYERS ) { error = true; }
	if ( multiplayer == CLIENT ) { error = true; }
	if ( !item ) { error = true; }
	if ( shrineType != GUI_TYPE_ETERNALSHRINE_ANVIL ) { error = true; }

	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { error = true; }
	if ( shrine && (shrine->eternalShrineType != shrineType) ) { error = true; }

	if ( error )
	{
		if ( shrine )
		{
			shrine->eternalShrineState = 0;
			serverUpdateEntitySkill(shrine, 4); // eternalShrineState
			shrine->eternalShrineOfferingItemTypeModel = 0;
			serverUpdateEntitySkill(shrine, 16);
		}
		return false;
	}

	shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE;
	shrine->eternalShrineTarget = achievementObserver.playerUids[player];
	serverUpdateEntitySkill(shrine, 4); // eternalShrineState

	shrine->eternalShrineItemType = item->type;
	shrine->eternalShrineItemStatus = item->status;
	shrine->eternalShrineItemBeatitude = item->beatitude;
	shrine->eternalShrineItemCount = item->count;
	shrine->eternalShrineItemAppearance = item->appearance;
	shrine->eternalShrineItemIdentified = item->identified;

	int sprite = itemModel(item);
	Sint32 offeringTypeModel = (sprite & 0xFFFF) << 16;
	offeringTypeModel |= (int)(item->type & 0x0FFF) << 4;
	offeringTypeModel |= ((GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING) & 0xF);
	shrine->eternalShrineOfferingItemTypeModel = offeringTypeModel;
	serverUpdateEntitySkill(shrine, 16);

	return true;
}

bool eternalShrineProcessAscensionItem(const int player, Uint32 shrineUid, int shrineType, Item* item, std::set<int>& learnedSpells)
{
	bool error = false;
	if ( player < 0 || player >= MAXPLAYERS ) { error = true; }
	if ( multiplayer == CLIENT ) { error = true; }
	if ( !item ) { error = true; }
	if ( item && item->type != SPELL_ITEM ) { error = true; }
	if ( shrineType != GUI_TYPE_ETERNALSHRINE_ASCENSION ) { error = true; }

	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { error = true; }
	if ( shrine && (shrine->eternalShrineType != shrineType) ) { error = true; }

	if ( item )
	{
		if ( item->appearance >= 1000 )
		{
			item->appearance -= 1000;
		}
		if ( item->appearance == 0 || item->appearance > NUM_SPELLS + 2 )
		{
			error = true;
		}
	}

	if ( error )
	{
		if ( shrine )
		{
			shrine->eternalShrineState = 0;
			serverUpdateEntitySkill(shrine, 4); // eternalShrineState
			shrine->eternalShrineOfferingItemTypeModel = 0;
			serverUpdateEntitySkill(shrine, 16);
		}
		return false;
	}

	shrine->eternalShrineItemType = 0;
	shrine->eternalShrineItemAppearance = 0;

	int skillID = 0;
	if ( item->appearance == NUM_SPELLS )
	{
		skillID = PRO_SORCERY;
	}
	else if ( item->appearance == NUM_SPELLS + 1 )
	{
		skillID = PRO_MYSTICISM;
	}
	else if ( item->appearance == NUM_SPELLS + 2 )
	{
		skillID = PRO_THAUMATURGY;
	}
	else
	{
		// ascension spells todo
		if ( auto spell = getSpellFromID(item->appearance) )
		{
			if ( spell->skillID == PRO_SORCERY )
			{
				shrine->eternalShrineItemType = TOME_SORCERY;
				shrine->eternalShrineItemAppearance = spellTomeIDToAppearance[spell->ID];
			}
			else if ( spell->skillID == PRO_MYSTICISM )
			{
				shrine->eternalShrineItemType = TOME_MYSTICISM;
				shrine->eternalShrineItemAppearance = spellTomeIDToAppearance[spell->ID];
			}
			else if ( spell->skillID == PRO_THAUMATURGY )
			{
				shrine->eternalShrineItemType = TOME_THAUMATURGY;
				shrine->eternalShrineItemAppearance = spellTomeIDToAppearance[spell->ID];
			}
		}
	}

	if ( skillID > 0 )
	{
		int itemType = 0;
		std::vector<std::pair<int, int>> chances;
		int maxDifficulty = std::max(0, (((players[player]->mechanics.divine_favor + 1) / 2) - 1) * 20);
		if ( players[player]->mechanics.divine_favor >= Player::DIVINE_FAVOR_MAX )
		{
			maxDifficulty = 100;
		}
		int minDifficulty = 0;
		for ( auto& def : allGameSpells )
		{
			if ( auto spell = def.second )
			{
				if ( spell->ID != SPELL_NONE 
					&& !spell->hide_from_ui 
					&& spell->drop_table >= 0
					&& learnedSpells.find(spell->ID) == learnedSpells.end() )
				{
					if ( spell->difficulty <= maxDifficulty
						&& (spell->difficulty >= minDifficulty)
						&& spell->skillID == skillID )
					{
						chances.push_back(std::make_pair(spell->skillID, spell->ID));
					}
				}
			}
		}

		Uint32 appearance = 0;
		if ( chances.size() )
		{
			int pick = local_rng.rand() % chances.size();
			{
				itemType = TOME_SORCERY;
				appearance = spellTomeIDToAppearance[chances[pick].second];
				if ( chances[pick].first == PRO_MYSTICISM )
				{
					itemType = TOME_MYSTICISM;
				}
				else if ( chances[pick].first == PRO_THAUMATURGY )
				{
					itemType = TOME_THAUMATURGY;
				}
			}
		}

		if ( itemType > 0 )
		{
			shrine->eternalShrineItemType = itemType;
			shrine->eternalShrineItemAppearance = appearance;
		}
	}

	if ( shrine->eternalShrineItemType == 0 )
	{
		shrine->eternalShrineState = 0;
		serverUpdateEntitySkill(shrine, 4); // eternalShrineState
		shrine->eternalShrineOfferingItemTypeModel = 0;
		serverUpdateEntitySkill(shrine, 16);
		return false;
	}

	shrine->eternalShrineItemStatus = DECREPIT;
	shrine->eternalShrineItemBeatitude = 0;
	shrine->eternalShrineItemCount = 1;
	shrine->eternalShrineItemIdentified = false;

	shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE;
	shrine->eternalShrineTarget = achievementObserver.playerUids[player];
	serverUpdateEntitySkill(shrine, 4); // eternalShrineState

	int sprite = item->appearance;
	int itemType = item->type;
	if ( item->type != SPELL_ITEM )
	{
		ItemType tmpType = item->type;
		Uint32 tmpAppearance = item->appearance;
		item->type = (ItemType)shrine->eternalShrineItemType;
		item->appearance = shrine->eternalShrineItemAppearance;
		sprite = itemModel(item);
		item->type = tmpType;
		item->appearance = tmpAppearance;

		itemType = shrine->eternalShrineItemType;
	}
	Sint32 offeringTypeModel = (sprite & 0xFFFF) << 16;
	offeringTypeModel |= (int)(itemType & 0x0FFF) << 4;
	offeringTypeModel |= ((GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING) & 0xF);
	shrine->eternalShrineOfferingItemTypeModel = offeringTypeModel;
	serverUpdateEntitySkill(shrine, 16);

	return true;
}

bool eternalShrineOnAscendItem(const int player, Uint32 shrineUid, Item* item)
{
	if ( player < 0 || player >= MAXPLAYERS ) { return false; }
	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { return false; }
	bool result = true;
	if ( multiplayer == CLIENT )
	{
		// submit to server
		strcpy((char*)net_packet->data, "ESHA");
		net_packet->data[4] = clientnum;
		net_packet->data[5] = shrine->eternalShrineType;
		SDLNet_Write32(shrineUid, &net_packet->data[6]);

		SDLNet_Write16((Sint16)item->type, &net_packet->data[10]);
		net_packet->data[12] = (int)item->status;
		SDLNet_Write16((Sint16)item->beatitude, &net_packet->data[13]);
		SDLNet_Write16((Uint16)1, &net_packet->data[15]);
		SDLNet_Write32((Uint32)item->appearance, &net_packet->data[17]);
		net_packet->data[21] = item->identified ? 1 : 0;

		std::set<int> learnedSpells;
		for ( auto node = stats[player]->inventory.first; node; node = node->next )
		{
			if ( Item* item = (Item*)node->element )
			{
				if ( item->type == SPELL_ITEM )
				{
					if ( item->appearance < 1000 )
					{
						if ( item->appearance > 0 && item->appearance < NUM_SPELLS )
						{
							if ( auto spell = getSpellFromID(item->appearance) )
							{
								if ( spell->drop_table >= 0 && !spell->hide_from_ui )
								{
									learnedSpells.insert(item->appearance);
								}
							}
						}
					}
				}
			}
		}

		int numSpells = std::min(220, (int)learnedSpells.size());
		net_packet->data[22] = numSpells;
		int i = -1;
		for ( auto& id : learnedSpells )
		{
			++i;
			if ( i >= numSpells ) { break; }
			SDLNet_Write16(id, &net_packet->data[23 + i * 2]);
		}

		net_packet->address.host = net_server.host;
		net_packet->address.port = net_server.port;
		net_packet->len = 23 + numSpells * 2;
		sendPacketSafe(net_sock, -1, net_packet, 0);

		shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_CLIENT_WAITING_RESULT;
	}
	else
	{
		std::set<int> learnedSpells;
		for ( auto node = stats[player]->inventory.first; node; node = node->next )
		{
			if ( Item* item = (Item*)node->element )
			{
				if ( item->type == SPELL_ITEM )
				{
					if ( item->appearance < 1000 )
					{
						if ( item->appearance > 0 && item->appearance < NUM_SPELLS )
						{
							if ( auto spell = getSpellFromID(item->appearance) )
							{
								if ( spell->drop_table >= 0 && !spell->hide_from_ui )
								{
									learnedSpells.insert(item->appearance);
								}
							}
						}
					}
				}
			}
		}

		result = eternalShrineProcessAscensionItem(player, shrineUid, shrine->eternalShrineType, item, learnedSpells);
	}

	//playSoundEntityLocal(shrine, 883, 128);
	return result;
}

bool eternalShrineOnSupplication(const int player, Uint32 shrineUid)
{
	if ( player < 0 || player >= MAXPLAYERS ) { return false; }
	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { return false; }
	bool result = true;
	if ( multiplayer == CLIENT )
	{
		// submit to server
		strcpy((char*)net_packet->data, "ESHT");
		net_packet->data[4] = clientnum;
		net_packet->data[5] = shrine->eternalShrineType;
		SDLNet_Write32(shrineUid, &net_packet->data[6]);

		net_packet->address.host = net_server.host;
		net_packet->address.port = net_server.port;
		net_packet->len = 10;
		sendPacketSafe(net_sock, -1, net_packet, 0);

		shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_CLIENT_WAITING_RESULT;
	}
	else
	{
		result = eternalShrineProcessSupplication(player, shrineUid, shrine->eternalShrineType);
	}

	return result;
}

bool eternalShrineOnMusic(const int player, Uint32 shrineUid)
{
	if ( player < 0 || player >= MAXPLAYERS ) { return false; }
	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { return false; }
	bool result = true;
	if ( multiplayer == CLIENT )
	{
		// submit to server
		strcpy((char*)net_packet->data, "ESHT");
		net_packet->data[4] = clientnum;
		net_packet->data[5] = shrine->eternalShrineType;
		SDLNet_Write32(shrineUid, &net_packet->data[6]);

		net_packet->address.host = net_server.host;
		net_packet->address.port = net_server.port;
		net_packet->len = 10;
		sendPacketSafe(net_sock, -1, net_packet, 0);

		shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_CLIENT_WAITING_RESULT;
	}
	else
	{
		result = eternalShrineProcessMusic(player, shrineUid, shrine->eternalShrineType);
	}

	return result;
}

bool eternalShrineOnSmithItem(const int player, Uint32 shrineUid, Item* item, int qty)
{
	if ( player < 0 || player >= MAXPLAYERS ) { return false; }
	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { return false; }
	bool result = true;
	if ( multiplayer == CLIENT )
	{
		// submit to server
		strcpy((char*)net_packet->data, "ESHA");
		net_packet->data[4] = clientnum;
		net_packet->data[5] = shrine->eternalShrineType;
		SDLNet_Write32(shrineUid, &net_packet->data[6]);

		SDLNet_Write16((Sint16)item->type, &net_packet->data[10]);
		net_packet->data[12] = (int)item->status;
		SDLNet_Write16((Sint16)item->beatitude, &net_packet->data[13]);
		SDLNet_Write16((Uint16)qty, &net_packet->data[15]);
		SDLNet_Write32((Uint32)item->appearance, &net_packet->data[17]);
		net_packet->data[21] = item->identified ? 1 : 0;
		net_packet->address.host = net_server.host;
		net_packet->address.port = net_server.port;
		net_packet->len = 22;
		sendPacketSafe(net_sock, -1, net_packet, 0);

		shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_CLIENT_WAITING_RESULT;
	}
	else
	{
		int tmpQty = item->count;
		item->count = std::min(qty, (int)item->count);
		result = eternalShrineProcessAnvilItem(player, shrineUid, shrine->eternalShrineType, item);
		item->count = tmpQty;
	}

	//playSoundEntityLocal(shrine, 883, 128);
	return result;
}

bool eternalShrineOnOfferItem(const int player, Uint32 shrineUid, Item* item, int qty)
{
	if ( player < 0 || player >= MAXPLAYERS ) { return false; }
	Entity* shrine = uidToEntity(shrineUid);
	if ( !shrine ) { return false; }

	int playerProgress = (shrine->eternalShrinePlayerStates >> (player * 2)) & 0b11;
	if ( playerProgress != GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE )
	{
		// unexpected
		return false;
	}
	else
	{
		playerProgress = std::min(3, std::max(0, (int)GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_WAITING));
		Uint32 newvalue = playerProgress << (player * 2);
		Uint32 mask = (0b11) << (player * 2);
		shrine->eternalShrinePlayerStates &= ~(mask); // zero out the player slot
		shrine->eternalShrinePlayerStates |= newvalue; // apply new value

		if ( multiplayer == CLIENT )
		{
			auto& gui = GenericGUI[player].eternalShrineGUI;
			gui.clientOfferingShrineUid = shrineUid;
			gui.clientOfferingShrineType = shrine->eternalShrineType;

			gui.clientOfferingItemToSend.type = item->type;
			gui.clientOfferingItemToSend.status = item->status;
			gui.clientOfferingItemToSend.beatitude = item->beatitude;
			gui.clientOfferingItemToSend.appearance = item->appearance;
			gui.clientOfferingItemToSend.count = qty;
			gui.clientOfferingItemToSend.identified = item->identified;


			// (delay this to updateEternalShrine) submit to server
			//strcpy((char*)net_packet->data, "ESHI");
			//net_packet->data[4] = clientnum;
			//net_packet->data[5] = shrine->eternalShrineType;
			//SDLNet_Write32(shrineUid, &net_packet->data[6]);
			//
			//SDLNet_Write16((Sint16)item->type, &net_packet->data[10]);
			//net_packet->data[12] = (int)item->status;
			//SDLNet_Write16((Sint16)item->beatitude, &net_packet->data[13]);
			//SDLNet_Write16((Uint16)qty, &net_packet->data[15]);
			//SDLNet_Write32((Uint32)item->appearance, &net_packet->data[17]);
			//net_packet->data[21] = item->identified ? 1 : 0;
			//net_packet->address.host = net_server.host;
			//net_packet->address.port = net_server.port;
			//net_packet->len = 22;
			//sendPacketSafe(net_sock, -1, net_packet, 0);
		}
		else
		{
			int tmpQty = item->count;
			item->count = std::min(qty, (int)item->count);
			eternalShrineProcessOfferingItem(player, shrineUid, shrine->eternalShrineType, item);
			item->count = tmpQty;
		}

		//playSoundEntityLocal(shrine, 883, 128);
		return true;
	}
}

void GenericGUIMenu::EternalShrineGUI_t::closeEternalShrine()
{
	const int playernum = parentGUI.getPlayer();
	auto& player = *players[playernum];

	if ( eternalShrineFrame )
	{
		eternalShrineFrame->setDisabled(true);
	}
	animx = 0.0;
	animTooltip = 0.0;
	animFilter = 0.0;
	submittedItem = EternalShrineSubmitStatus::SUBMIT_NONE;
	submitTick = 0;
	animSubmit = 0.0;
	animAction = 0.0;

	ascensionType = ASCENSION_SPELL;
	//currentView = EternalShrineView_t::ASSIST_SHRINE_VIEW_OFFERING;

	pipsTarget = 0;
	pipsAdd = 0;
	pipsTick = 0;
	pipsFlashTick = 0;
	pipsAnimThisTick = 0;
	pipsAddSpeed = 0;

	animSendItem1 = 0.0;
	sendItem1Uid = 0;
	//mailReceiveItem.type = POTION_EMPTY;
	//recvItemUid = 0;
	//animRecvItem = 0.0;

	isInteractable = false;
	bool wasOpen = bOpen;
	bOpen = false;
	bFirstTimeSnapCursor = false;
	bSkipOfferingPrompt = false;
	if ( wasOpen )
	{
		if ( inputs.getUIInteraction(playernum)->selectedItem )
		{
			inputs.getUIInteraction(playernum)->selectedItem = nullptr;
			inputs.getUIInteraction(playernum)->toggleclick = false;
		}
		inputs.getUIInteraction(playernum)->selectedItemFromChest = 0;
	}
	if ( players[playernum]->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE
		&& !players[playernum]->shootmode )
	{
		// reset to inventory mode if still hanging in alchemy GUI
		players[playernum]->hud.compactLayoutMode = Player::HUD_t::COMPACT_LAYOUT_INVENTORY;
		players[playernum]->GUI.activateModule(Player::GUI_t::MODULE_INVENTORY);
		if ( !inputs.getVirtualMouse(playernum)->draw_cursor )
		{
			players[playernum]->GUI.warpControllerToModule(false);
		}
	}
	clearItemDisplayed();
	//itemRequiresTitleReflow = true;
	if ( eternalShrineFrame )
	{
		for ( auto f : eternalShrineFrame->getFrames() )
		{
			f->removeSelf();
		}
		eternalShrineSlotFrames.clear();
	}
	//notifications.clear();

	if ( multiplayer != CLIENT )
	{
		if ( Entity* eternalShrine = uidToEntity(parentGUI.eternalShrineEntityUid) )
		{
			if ( currentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
			{
				// no close
			}
			else
			{
				eternalShrine->eternalShrineInteracting = 0;
				serverUpdateEntitySkill(eternalShrine, 6);	
			}
			serverUpdateEntitySkill(eternalShrine, 17); // update player states
		}
	}

	if ( currentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
	{
		// no close
	}
	else
	{
		if ( multiplayer == CLIENT && parentGUI.eternalShrineEntityUid > 0 )
		{
			strcpy((char*)net_packet->data, "ESHC");
			net_packet->data[4] = clientnum;
			SDLNet_Write32(parentGUI.eternalShrineEntityUid, &net_packet->data[5]);
			net_packet->address.host = net_server.host;
			net_packet->address.port = net_server.port;
			net_packet->len = 9;
			sendPacketSafe(net_sock, -1, net_packet, 0);
		}
	}
	parentGUI.eternalShrineEntityUid = 0;
}

int GenericGUIMenu::EternalShrineGUI_t::heightOffsetWhenNotCompact = 150;
const int eternalShrineBaseWidth = 206;

void getInventoryItemEternalShrineAnimSlotPos(Frame* slotFrame, Player* player, int itemx, int itemy, int& outPosX, int& outPosY, int yOffset)
{
	outPosX = slotFrame->getSize().x + slotFrame->getParent()->getSize().x;
	outPosY = slotFrame->getSize().y + (player->inventoryUI.bCompactView ? 8 : 0) + yOffset;
	if ( player && player->inventory_mode == INVENTORY_MODE_ITEM )
	{
		if ( itemy >= player->inventoryUI.DEFAULT_INVENTORY_SIZEY )
		{
			// backpack slots, add another offset.
			if ( auto invSlotsFrame = player->inventoryUI.frame->findFrame("inventory slots") )
			{
				outPosY += invSlotsFrame->getSize().h;
			}
		}
	}
	else if ( player && player->inventory_mode == INVENTORY_MODE_SPELL )
	{
		outPosY -= slotFrame->getParent()->getActualSize().y;
	}
}

bool GenericGUIMenu::EternalShrineGUI_t::eternalShrineGUIHasBeenCreated() const
{
	if ( eternalShrineFrame )
	{
		if ( !eternalShrineFrame->getFrames().empty() )
		{
			for ( auto f : eternalShrineFrame->getFrames() )
			{
				if ( !f->isToBeDeleted() )
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			return false;
		}
	}
	return false;
}

void buttonEternalShrineUpdateSelectorOnHighlight(const int player, Button* button)
{
	if ( button->isHighlighted() )
	{
		players[player]->GUI.setHoveringOverModuleButton(Player::GUI_t::MODULE_ETERNALSHRINE);
		if ( players[player]->GUI.activeModule != Player::GUI_t::MODULE_ETERNALSHRINE )
		{
			players[player]->GUI.activateModule(Player::GUI_t::MODULE_ETERNALSHRINE);
		}
		SDL_Rect pos = button->getAbsoluteSize();
		// make sure to adjust absolute size to camera viewport
		pos.x -= players[player]->camera_virtualx1();
		pos.y -= players[player]->camera_virtualy1();
		players[player]->hud.setCursorDisabled(false);
		players[player]->hud.updateCursorAnimation(pos.x - 1, pos.y - 1, pos.w, pos.h, inputs.getVirtualMouse(player)->draw_cursor);
	}
}

void GenericGUIMenu::EternalShrineGUI_t::openEternalShrine(Entity* shrineEntity)
{
	const int playernum = parentGUI.getPlayer();
	auto player = players[playernum];

	if ( eternalShrineFrame )
	{
		bool wasDisabled = eternalShrineFrame->isDisabled();
		eternalShrineFrame->setDisabled(false);
		if ( wasDisabled )
		{
			//notifications.clear();
			animx = 0.0;
			animFilter = 0.0;
			animTooltip = 0.0;
			submittedItem = EternalShrineSubmitStatus::SUBMIT_NONE;
			submitTick = 0;
			animSubmit = 0.0;
			animAction = 0.0;

			ascensionType = ASCENSION_SPELL;
			currentView = EternalShrineView_t::ASSIST_SHRINE_VIEW_OFFERING;

			pipsTarget = 0;
			pipsAdd = 0;
			pipsTick = 0;
			pipsFlashTick = 0;
			pipsAnimThisTick = 0;
			pipsAddSpeed = 0;

			isInteractable = false;
			bFirstTimeSnapCursor = false;
			bSkipOfferingPrompt = false;
		}
		selectEternalShrineSlot(ETERNALSHRINE_SLOT_SEND, 0);
		player->hud.compactLayoutMode = Player::HUD_t::COMPACT_LAYOUT_INVENTORY;
		player->inventory_mode = INVENTORY_MODE_ITEM;
		bOpen = true;
	}
	parentGUI.eternalShrineEntityUid = 0;
	if ( shrineEntity )
	{
		parentGUI.eternalShrineEntityUid = shrineEntity->getUID();
		submittedItem = (EternalShrineSubmitStatus)((shrineEntity->eternalShrinePlayerStates >> (playernum * 2)) & 0b11);
		if ( submittedItem != SUBMIT_NONE )
		{
			bSkipOfferingPrompt = true;
		}
	}
	pipsTotal = players[playernum]->mechanics.divine_favor;

	if ( inputs.getUIInteraction(playernum)->selectedItem )
	{
		inputs.getUIInteraction(playernum)->selectedItem = nullptr;
		inputs.getUIInteraction(playernum)->toggleclick = false;
	}
	inputs.getUIInteraction(playernum)->selectedItemFromChest = 0;
	clearItemDisplayed();
}

void GenericGUIMenu::EternalShrineGUI_t::updateEternalShrine()
{
	if ( multiplayer == CLIENT )
	{
		if ( clientOfferingShrineType > 0 )
		{
			if ( ticks - submitTick >= TICKS_PER_SECOND )
			{
				if ( net_packet && clientOfferingItemToSend.count > 0 )
				{
					// submit to server
					strcpy((char*)net_packet->data, "ESHI");
					net_packet->data[4] = clientnum;
					net_packet->data[5] = clientOfferingShrineType;
					SDLNet_Write32(clientOfferingShrineUid, &net_packet->data[6]);

					SDLNet_Write16((Sint16)clientOfferingItemToSend.type, &net_packet->data[10]);
					net_packet->data[12] = (int)clientOfferingItemToSend.status;
					SDLNet_Write16((Sint16)clientOfferingItemToSend.beatitude, &net_packet->data[13]);
					SDLNet_Write16((Uint16)clientOfferingItemToSend.count, &net_packet->data[15]);
					SDLNet_Write32((Uint32)clientOfferingItemToSend.appearance, &net_packet->data[17]);
					net_packet->data[21] = clientOfferingItemToSend.identified ? 1 : 0;
					net_packet->address.host = net_server.host;
					net_packet->address.port = net_server.port;
					net_packet->len = 22;
					sendPacketSafe(net_sock, -1, net_packet, 0);
				}
				clientOfferingItemToSend.count = 0;
				clientOfferingShrineType = 0;
			}
		}
	}
	else
	{
		clientOfferingItemToSend.count = 0;
		clientOfferingShrineType = 0;
	}

	const int playernum = parentGUI.getPlayer();
	auto player = players[playernum];

	if ( !player->isLocalPlayer() )
	{
		closeEternalShrine();
		return;
	}

	Entity* eternalShrineStation = nullptr;
	if ( bOpen )
	{
		if ( parentGUI.eternalShrineEntityUid != 0 )
		{
			eternalShrineStation = uidToEntity(parentGUI.eternalShrineEntityUid);
			if ( !eternalShrineStation )
			{
				parentGUI.closeGUI();
				return;
			}
		}

		if ( eternalShrineStation )
		{
			if ( eternalShrineStation->eternalShrineState >= GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
			{
				currentView = ASSIST_SHRINE_VIEW_OFFERING;
				//parentGUI.closeGUI();
				player->closeAllGUIs(CLOSEGUI_ENABLE_SHOOTMODE, CLOSEGUI_CLOSE_ALL);
				return;
			}

			if ( player->entity && eternalShrineStation->eternalShrineState == GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_NONE
				&& (entityDist(player->entity, eternalShrineStation) > TOUCHRANGE) )
			{
				parentGUI.closeGUI();
				return;
			}
		}
	}

	if ( !eternalShrineFrame )
	{
		return;
	}

	eternalShrineFrame->setSize(SDL_Rect{ players[playernum]->camera_virtualx1(),
		players[playernum]->camera_virtualy1(),
		eternalShrineBaseWidth,
		players[playernum]->camera_virtualHeight() });

	if ( !eternalShrineFrame->isDisabled() && bOpen )
	{
		if ( !eternalShrineGUIHasBeenCreated() )
		{
			createEternalShrine();
		}

		const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
		real_t setpointDiffX = fpsScale * std::max(.01, (1.0 - animx)) / 2.0;
		animx += setpointDiffX;
		animx = std::min(1.0, animx);
		if ( animx >= .9999 )
		{
			if ( !bFirstTimeSnapCursor )
			{
				bFirstTimeSnapCursor = true;
				if ( !inputs.getUIInteraction(playernum)->selectedItem
					&& player->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE )
				{
					//warpMouseToSelectedAlchemyItem(nullptr, (Inputs::SET_CONTROLLER));
				}
			}
			isInteractable = true;
		}
	}
	else
	{
		animx = 0.0;
		animTooltip = 0.0;
		isInteractable = false;
	}

	auto eternalShrineFramePos = eternalShrineFrame->getSize();
	if ( player->inventoryUI.inventoryPanelJustify == Player::PANEL_JUSTIFY_LEFT )
	{
		if ( !player->inventoryUI.bCompactView )
		{
			const int fullWidth = eternalShrineFramePos.w + 210; // inventory width 210
			eternalShrineFramePos.x = -eternalShrineFramePos.w + animx * fullWidth;
			if ( player->bUseCompactGUIWidth() )
			{
				if ( player->inventoryUI.slideOutPercent >= .0001 )
				{
					isInteractable = false;
				}
				eternalShrineFramePos.x -= player->inventoryUI.slideOutWidth * player->inventoryUI.slideOutPercent;
			}
		}
		else
		{
			if ( player->bAlignGUINextToInventoryCompact() )
			{
				const int fullWidth = eternalShrineFramePos.w + 210; // inventory width 210
				eternalShrineFramePos.x = -eternalShrineFramePos.w + animx * fullWidth;
			}
			else
			{
				eternalShrineFramePos.x = player->camera_virtualWidth() - animx * eternalShrineFramePos.w;
			}
			if ( player->bUseCompactGUIWidth() )
			{
				if ( player->inventoryUI.slideOutPercent >= .0001 )
				{
					isInteractable = false;
				}
				eternalShrineFramePos.x -= -player->inventoryUI.slideOutWidth * player->inventoryUI.slideOutPercent;
			}
		}
	}
	else if ( player->inventoryUI.inventoryPanelJustify == Player::PANEL_JUSTIFY_RIGHT )
	{
		if ( !player->inventoryUI.bCompactView )
		{
			const int fullWidth = eternalShrineFramePos.w + 210; // inventory width 210
			eternalShrineFramePos.x = player->camera_virtualWidth() - animx * fullWidth;
			if ( player->bUseCompactGUIWidth() )
			{
				if ( player->inventoryUI.slideOutPercent >= .0001 )
				{
					isInteractable = false;
				}
				eternalShrineFramePos.x -= -player->inventoryUI.slideOutWidth * player->inventoryUI.slideOutPercent;
			}
		}
		else
		{
			if ( player->bAlignGUINextToInventoryCompact() )
			{
				const int fullWidth = eternalShrineFramePos.w + 210; // inventory width 210
				eternalShrineFramePos.x = player->camera_virtualWidth() - animx * fullWidth;
			}
			else
			{
				eternalShrineFramePos.x = -eternalShrineFramePos.w + animx * eternalShrineFramePos.w;
			}
			if ( player->bUseCompactGUIWidth() )
			{
				if ( player->inventoryUI.slideOutPercent >= .0001 )
				{
					isInteractable = false;
				}
				eternalShrineFramePos.x -= player->inventoryUI.slideOutWidth * player->inventoryUI.slideOutPercent;
				eternalShrineFramePos.w = player->camera_virtualWidth();
			}
		}
	}

	bool viewActionReady = false;
	if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
	{
		if ( submittedItem != EternalShrineSubmitStatus::SUBMIT_NONE )
		{
			isInteractable = false;
			const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
			real_t setpointDiffX = fpsScale * std::max(.01, (1.0 - animSubmit)) / 10.0;
			animSubmit += setpointDiffX;
			animSubmit = std::min(1.0, animSubmit);

			if ( submittedItem == EternalShrineSubmitStatus::SUBMIT_WAITING
				|| submittedItem == EternalShrineSubmitStatus::SUBMIT_CONFIRMED )
			{
				if ( multiplayer != CLIENT )
				{
					if ( submitTick == 0 || (ticks - submitTick >= 1 * TICKS_PER_SECOND) )
					{
						submittedItem = EternalShrineSubmitStatus::SUBMIT_CONFIRMED;
					}
				}
			}
		}

		animAction = 0.0;
	}
	else 
	{
		if ( currentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
		{
			isInteractable = false;
		}
		else if ( submittedItem != EternalShrineSubmitStatus::SUBMIT_DONE )
		{
			isInteractable = false;
		}
		if ( animFilter > 0.01 )
		{
			isInteractable = false;
		}

		if ( player->mechanics.divine_favor > pipsTotal )
		{
			int diff = player->mechanics.divine_favor - pipsTotal;
			pipsTotal = player->mechanics.divine_favor;
			pipsAddSpeed = 20;
		}

		/*if ( pipsQueued > 0 )
		{
			pipsTotal += pipsQueued;
			pipsQueued = 0;
			pipsAddSpeed = 20;
			pipsTotal = std::min(10, pipsTotal);
		}*/

		if ( animx >= .9999 && isInteractable )
		{
			viewActionReady = true;
			if ( sendItem1Uid == 0 )
			{
				animSendItem1 = 0.0;
			}

			const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz

			real_t filterMinSpeed = bSkipOfferingPrompt ? *cvar_eternal_shrine_skip_prompt_speed : 0.01;
			real_t setpointDiffX = fpsScale * std::max(filterMinSpeed, (1.0 - animAction)) / 10.0;
			animAction += setpointDiffX;
			animAction = std::min(1.0, animAction);
		}
		else
		{
			animAction = 0.0;
		}
	}

	int eternalShrineItemAnimOffsetY = 0; // all animations tested at heightOffsetWhenNotCompact = 200, so needs offset
	if ( !player->bUseCompactGUIHeight() && !player->bUseCompactGUIWidth() )
	{
		eternalShrineFramePos.y = heightOffsetWhenNotCompact;
		eternalShrineItemAnimOffsetY = 200 - heightOffsetWhenNotCompact;
	}
	else
	{
		eternalShrineFramePos.y = 0;
	}

	if ( !eternalShrineGUIHasBeenCreated() )
	{
		return;
	}

	auto baseFrame = eternalShrineFrame->findFrame("eternal base");
	baseFrame->setDisabled(false);
	if ( auto baseFrameImg = baseFrame->findImage("eternal base img") )
	{
		if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ANVIL )
		{
			baseFrameImg->path = "*#images/ui/Shrines/divine_anvil/Anvil_Base_01.png";
		}
		else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
		{
			baseFrameImg->path = "*#images/ui/Shrines/altar_supplication/Supplication_Base_01B.png";
		}
		else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_MUSIC )
		{
			baseFrameImg->path = "*#images/ui/Shrines/chorale_shrine/Chorale_Base_01.png";
		}
		else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
		{
			baseFrameImg->path = "*#images/ui/Shrines/altar_inspiration/Inspiration_Base_01B.png";
		}
	}

	eternalShrineFrame->setSize(eternalShrineFramePos);

	SDL_Rect baseFramePos = baseFrame->getSize();
	baseFramePos.x = 0;
	baseFramePos.w = eternalShrineBaseWidth;
	baseFrame->setSize(baseFramePos);

	eternalShrineFramePos.h = baseFramePos.y + baseFramePos.h;
	if ( animx >= .9999 )
	{
		baseFramePos.x += eternalShrineFramePos.x;
		eternalShrineFramePos.w += eternalShrineFramePos.x;
		eternalShrineFramePos.w = std::min(player->camera_virtualWidth(), eternalShrineFramePos.w);
		eternalShrineFramePos.x = 0;
		baseFrame->setSize(baseFramePos);
	}
	eternalShrineFrame->setSize(eternalShrineFramePos);

	if ( pipsTotal == 0 )
	{
		pipsTarget = 0;
		pipsAdd = 0;
	}
	else
	{
		while ( pipsAdd + pipsTarget > pipsTotal )
		{
			if ( pipsAdd > 0 )
			{
				--pipsAdd;
			}
			else if ( pipsTarget > 0 )
			{
				--pipsTarget;
			}

			if ( pipsAdd < 0 )
			{
				pipsAdd = 0;
			}
			if ( pipsTarget < 0 )
			{
				pipsTarget = 0;
			}
		}
		while ( pipsAdd + pipsTarget < pipsTotal )
		{
			++pipsAdd;
		}
	}

	Frame* shroudFrame = baseFrame->findFrame("eternal shroud frame");
	Frame* shroudTopFrame = shroudFrame->findFrame("eternal shroud top frame");
	shroudTopFrame->setOpacity(0.0);
	auto offeringPrompt = eternalShrineFrame->findField("offering prompt");
	offeringPrompt->setDisabled(true);
	auto shroudBadge = shroudTopFrame->findImage("eternal shroud badge");
	auto shroundItemBg = shroudTopFrame->findImage("eternal shroud item bg");
	if ( bSkipOfferingPrompt || currentView == ASSIST_SHRINE_VIEW_WAITING )
	{
		shroundItemBg->disabled = true;
	}
	if ( animx >= .9999 )
	{
		const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
		if ( keystatus[SDLK_g] )
		{
			keystatus[SDLK_g] = 0;
			if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
			{
				currentView = ASSIST_SHRINE_VIEW_ACTION;
				submittedItem = EternalShrineSubmitStatus::SUBMIT_DONE;
			}
			else if ( currentView == ASSIST_SHRINE_VIEW_ACTION )
			{
				currentView = ASSIST_SHRINE_VIEW_WAITING;
				submittedItem = EternalShrineSubmitStatus::SUBMIT_NONE;
			}
			else
			{
				currentView = ASSIST_SHRINE_VIEW_OFFERING;
				submittedItem = EternalShrineSubmitStatus::SUBMIT_NONE;
			}

			Uint32 newvalue = submittedItem << (playernum * 2);
			Uint32 mask = (0b11) << (playernum * 2);
			if ( eternalShrineStation )
			{
				eternalShrineStation->eternalShrinePlayerStates &= ~(mask); // zero out the player slot
				eternalShrineStation->eternalShrinePlayerStates |= newvalue; // apply new value
				if ( multiplayer == SERVER )
				{
					serverUpdateEntitySkill(eternalShrineStation, 17);
				}
			}
		}

		if ( keystatus[SDLK_h] )
		{
			keystatus[SDLK_h] = 0;
			if ( keystatus[SDLK_LSHIFT] )
			{
				//pipsTotal -= 1;
				//pipsTotal = std::max(0, pipsTotal);
				player->mechanics.divine_favor -= 1;
				player->mechanics.divine_favor = std::max(0, player->mechanics.divine_favor);
			}
			else
			{
				player->mechanics.divine_favor += 1;
				if ( player->mechanics.divine_favor > 10 )
				{
					player->mechanics.divine_favor = 0;
				}
				/*pipsTotal += 1;
				if ( pipsTotal > 10 )
				{
					pipsTotal = 0;
				}*/
			}
		}
		if ( keystatus[SDLK_j] )
		{
			keystatus[SDLK_j] = 0;
			if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ANVIL )
			{
				parentGUI.guiType = GUICurrentType::GUI_TYPE_ETERNALSHRINE_SUPPLICATION;
			}
			else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
			{
				parentGUI.guiType = GUICurrentType::GUI_TYPE_ETERNALSHRINE_MUSIC;
			}
			else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_MUSIC )
			{
				parentGUI.guiType = GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION;
			}
			else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
			{
				parentGUI.guiType = GUICurrentType::GUI_TYPE_ETERNALSHRINE_ANVIL;
			}
		}

		real_t filterMinSpeed = bSkipOfferingPrompt ? *cvar_eternal_shrine_skip_prompt_speed : 0.01;
		if ( currentView == ASSIST_SHRINE_VIEW_WAITING )
		{
			real_t setpointDiffX = fpsScale * std::max(0.01, (1.0 - animFilter)) / 10.0;
			animFilter += setpointDiffX;
			animFilter = std::min(1.0, animFilter);
		}
		else if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
		{
			real_t setpointDiffX = fpsScale * std::max(filterMinSpeed, (1.0 - animFilter)) / 10.0;
			animFilter += setpointDiffX;
			animFilter = std::min(1.0, animFilter);

			if ( animFilter >= .9999 && submittedItem == SUBMIT_CONFIRMED )
			{
				submittedItem = SUBMIT_DONE;
				currentView = ASSIST_SHRINE_VIEW_ACTION;
			}
		}
		else
		{
			real_t setpointDiffX = fpsScale * std::max(filterMinSpeed, animFilter) / 10.0;
			animFilter -= setpointDiffX;
			animFilter = std::max(0.0, animFilter);
		}

		shroudTopFrame->setOpacity(animFilter * 100.0);

		shroudBadge->pos.y = shroudTopFrame->getSize().h - shroudBadge->pos.h;
		shroudBadge->pos.y += animSubmit * 16;
		//shroudBadge->pos.y += (animSubmit) * 16 * sin(PI * (ticks % 200 / 100.0));

		if ( currentView == ASSIST_SHRINE_VIEW_OFFERING || currentView == ASSIST_SHRINE_VIEW_WAITING )
		{
			shroudBadge->pos.y -= animFilter * 100;
		}
		else
		{
			shroudBadge->pos.y -= 100 + (1.0 - animFilter) * 100;
		}
		shroundItemBg->pos.y = shroudBadge->pos.y + 24;
		/*Uint8 r, g, b, a;
		getColor(shroudBadge->color, &r, &g, &b, &a);*/
		Uint8 a = animFilter * std::max(0, std::min(255, (int)(255 * (0.5 + (1.0 + sin(PI * (ticks % 200 / 100.0))) / 4.0))));
		shroudBadge->color = makeColor(255, 255, 255, a);

		SDL_Rect pos = offeringPrompt->getSize();
		pos.x = baseFrame->getSize().x;
		pos.y = shroudTopFrame->getSize().h - 224;
		pos.h = 24;
		pos.w = baseFrame->getSize().w;
		offeringPrompt->setText(Language::get(6999));
		if ( bSkipOfferingPrompt || currentView == ASSIST_SHRINE_VIEW_WAITING )
		{
		}
		else
		{
			offeringPrompt->setDisabled(false);
		}
		if ( auto textGet = offeringPrompt->getTextObject() )
		{
			pos.x += pos.w / 2;
			pos.x -= textGet->getWidth() / 2;
		}
		SDL_Color color;
		getColor(offeringPrompt->getColor(), &color.r, &color.g, &color.b, &color.a);
		color.a = (Uint8)(255 * animFilter * (1.0 - animSubmit));
		offeringPrompt->setColor(makeColor(color.r, color.g, color.b, color.a));
		offeringPrompt->setSize(pos);
	}

	{
		auto pipsFrame = baseFrame->findFrame("eternal pips");
		auto pipsLinks = pipsFrame->findImage("pips links");
		std::vector<Frame::image_t*> pipImgs;
		for ( int i = 0; i < 10; ++i )
		{
			std::string imgName = "pip " + std::to_string(i);
			if ( auto img = pipsFrame->findImage(imgName.c_str()) )
			{
				pipImgs.push_back(img);
			}
		}

		int tickUpdateSpeed = 4;
		if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
		{
			tickUpdateSpeed = 8;
		}

		bool updatePip = false;
		if ( pipsAnimThisTick != ticks )
		{
			updatePip = true;
			pipsAnimThisTick = ticks;
			if ( pipsTick > 0 )
			{
				--pipsTick;
			}
			++pipsFlashTick;
		}
		if ( pipsAdd > 0 )
		{
			if ( pipsTick == 0 )
			{
				if ( pipsAddSpeed > 0 )
				{
					playSound(555, 32);
				}
				pipsTarget += 1;
				pipsAdd--;
				pipsTick = tickUpdateSpeed;
				if ( pipsAddSpeed > 0 )
				{
					pipsTick = pipsAddSpeed;
				}
				pipsFlashTick = 1;
			}
		}

		std::map<int, std::map<int, std::string>> imgPaths; 
		if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ANVIL )
		{
			pipsLinks->path = "#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_Connectors_00.png";
			pipsLinks->pos.x = 46;
			pipsLinks->pos.y = 88;
			pipsLinks->pos.h = 2;
			imgPaths[0] =
			{
				{1, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_1DarkRed_00.png"},
				{2, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_1DarkRed_00.png"},
				{3, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_2DarkOrange_00.png"},
				{4, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_2DarkOrange_00.png"},
				{5, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_3Orange_00.png"},
				{6, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_3Orange_00.png"},
				{7, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_4DarkYellow_00.png"},
				{8, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_4DarkYellow_00.png"},
				{9, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_5Yellow_00.png"},
				{10, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_5Yellow_00.png"},
				{11, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_6LightYellow_00.png"},
				{12, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_6LightYellow_00.png"},
				{13, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_6LightYellow_00.png"}
			};
			for ( int i = 0; i < 10; ++i )
			{
				if ( i < pipImgs.size() )
				{
					pipImgs[i]->pos = SDL_Rect{ pipsLinks->pos.x - 6 + i * 12, pipsLinks->pos.y - 2, 6, 6 };
				}
			}
		}
		else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
		{
			pipsLinks->path = "#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_Connectors_00.png";
			pipsLinks->pos.x = 46;
			pipsLinks->pos.y = 140;
			pipsLinks->pos.h = 28;
			imgPaths[0] =
			{
				{1, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_1DarkRed_00.png"},
				{2, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_1DarkRed_00.png"},
				{3, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_2DarkOrange_00.png"},
				{4, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_2DarkOrange_00.png"},
				{5, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_3Orange_00.png"},
				{6, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_3Orange_00.png"},
				{7, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_4DarkYellow_00.png"},
				{8, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_4DarkYellow_00.png"},
				{9, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_5Yellow_00.png"},
				{10, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_5Yellow_00.png"},
				{11, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_6LightYellow_00.png"},
				{12, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_6LightYellow_00.png"},
				{13, "*#images/ui/Shrines/altar_supplication/Supplication_Bar_Gem_6LightYellow_00.png"}
			};

			for ( int i = 0; i < 10; ++i )
			{
				if ( i < pipImgs.size() )
				{
					pipImgs[i]->pos = SDL_Rect{ pipsLinks->pos.x - 6 + i * 12, pipsLinks->pos.y - 2, 6, 6 };
					if ( i < 5 )
					{
						pipImgs[i]->pos.y += i * 6;
					}
					else
					{
						pipImgs[i]->pos.x += 12;
						pipImgs[i]->pos.y += 24 - (i - 5) * 6;
					}
				}
			}
		}
		else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_MUSIC )
		{
			pipsLinks->path = "#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_Connectors_00.png";
			pipsLinks->pos.x = 32;
			pipsLinks->pos.y = 58;
			pipsLinks->pos.h = 48;
			imgPaths[0] =
			{
				{1, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_1DarkRed_00.png"},
				{2, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_1DarkRed_00.png"},
				{3, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_2DarkOrange_00.png"},
				{4, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_2DarkOrange_00.png"},
				{5, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_3Orange_00.png"},
				{6, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_3Orange_00.png"},
				{7, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_4DarkYellow_00.png"},
				{8, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_4DarkYellow_00.png"},
				{9, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_5Yellow_00.png"},
				{10, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_5Yellow_00.png"},
				{11, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_6LightYellow_00.png"},
				{12, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_6LightYellow_00.png"},
				{13, "*#images/ui/Shrines/chorale_shrine/Chorale_Bar_Gem_6LightYellow_00.png"}
			};

			const std::vector<std::pair<int, int>> coords =
			{
				{4, 24},
				{18, 32},
				{32, 20},
				{46, 16},
				{60, 12},
				{74, 16},
				{88, 20},
				{102, 12},
				{116, 16},
				{130, 20}
			};

			for ( int i = 0; i < 10; ++i )
			{
				if ( i < pipImgs.size() )
				{
					pipImgs[i]->pos = SDL_Rect{ pipsLinks->pos.x, pipsLinks->pos.y, 6, 6 };
					pipImgs[i]->pos.x += coords[i].first;
					pipImgs[i]->pos.y += coords[i].second;
				}
			}
		}
		else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
		{
			pipsLinks->path = "#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Gem_Connectors_00.png";
			pipsLinks->pos.x = 28;
			pipsLinks->pos.y = 92;
			pipsLinks->pos.h = 22;

			imgPaths[0] =
			{
				{1, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_00.png"},
				{2, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_00.png"},
				{3, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_01.png"},
				{4, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_01.png"},
				{5, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_02.png"},
				{6, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_02.png"},
				{7, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_03.png"},
				{8, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_03.png"},
				{9, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_04.png"},
				{10, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_04.png"},
				{11, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_05.png"},
				{12, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_05.png"},
				{13, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier1_05.png"}
			};
			imgPaths[1] =
			{
				{1, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_00.png"},
				{2, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_00.png"},
				{3, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_01.png"},
				{4, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_01.png"},
				{5, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_02.png"},
				{6, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_02.png"},
				{7, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_03.png"},
				{8, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_03.png"},
				{9, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_04.png"},
				{10, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_04.png"},
				{11, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_05.png"},
				{12, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_05.png"},
				{13, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier2_05.png"}
			};
			imgPaths[2] =
			{
				{1, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_00.png"},
				{2, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_00.png"},
				{3, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_01.png"},
				{4, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_01.png"},
				{5, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_02.png"},
				{6, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_02.png"},
				{7, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_03.png"},
				{8, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_03.png"},
				{9, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_04.png"},
				{10, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_04.png"},
				{11, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_05.png"},
				{12, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_05.png"},
				{13, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier3_05.png"}
			};
			imgPaths[3] =
			{
				{1, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_00.png"},
				{2, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_00.png"},
				{3, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_01.png"},
				{4, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_01.png"},
				{5, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_02.png"},
				{6, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_02.png"},
				{7, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_03.png"},
				{8, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_03.png"},
				{9, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_04.png"},
				{10, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_04.png"},
				{11, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_05.png"},
				{12, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_05.png"},
				{13, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier4_05.png"}
			};
			imgPaths[4] =
			{
				{1, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_00.png"},
				{2, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_00.png"},
				{3, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_01.png"},
				{4, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_01.png"},
				{5, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_02.png"},
				{6, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_02.png"},
				{7, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_03.png"},
				{8, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_03.png"},
				{9, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_04.png"},
				{10, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_04.png"},
				{11, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_05.png"},
				{12, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_05.png"},
				{13, "*#images/ui/Shrines/altar_inspiration/Inspiration_Bar_Tier5_05.png"}
			};

			for ( int i = 0; i < 10; ++i )
			{
				if ( i < pipImgs.size() )
				{
					pipImgs[i]->pos = SDL_Rect{ pipsLinks->pos.x + 0 + (i / 2) * 32, pipsLinks->pos.y + 2, 20, 18 };
				}
			}
		}

		if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_MUSIC )
		{
			if ( pipsTarget <= 0 )
			{
				pipsLinks->disabled = true;
				pipsLinks->pos.w = 2;
			}
			else
			{
				int width = pipsTarget * 14;
				if ( updatePip )
				{
					if ( pipsLinks->pos.w < width )
					{
						pipsLinks->pos.w += 4;
						pipsLinks->pos.w = std::min(pipsLinks->pos.w, width);
					}
					else
					{
						pipsLinks->pos.w = width;
					}
				}
				pipsLinks->disabled = false;
			}
		}
		else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
		{
			if ( pipsTarget <= 0 )
			{
				pipsLinks->disabled = true;
				pipsLinks->pos.w = 2;
			}
			else
			{
				int width = 0;
				width += (((pipsTarget % 2) == 1) ? 12 : 22);
				width += ((pipsTarget - 1) / 2) * 32;
				if ( updatePip )
				{
					if ( pipsLinks->pos.w < width )
					{
						pipsLinks->pos.w += 2;
						pipsLinks->pos.w = std::min(pipsLinks->pos.w, width);
					}
					else
					{
						pipsLinks->pos.w = width;
					}
				}
				pipsLinks->disabled = false;
			}
		}
		else
		{
			if ( pipsTarget <= 1 )
			{
				pipsLinks->disabled = true;
				pipsLinks->pos.w = 2;
			}
			else
			{
				int width = (pipsTarget - 1) * 12;
				if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
				{
					if ( pipsTarget >= 6 )
					{
						width += 6;
					}
				}
				if ( updatePip )
				{
					if ( pipsLinks->pos.w < width )
					{
						pipsLinks->pos.w += 4;
						pipsLinks->pos.w = std::min(pipsLinks->pos.w, width);
					}
					else
					{
						pipsLinks->pos.w = width;
					}
				}
				pipsLinks->disabled = false;
			}
		}

		for ( int i = 0; i < 10; ++i )
		{
			if ( i >= pipImgs.size() )
			{
				continue;
			}

			int imgPathsIndex = 0;
			if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
			{
				if ( i <= 1 ) { imgPathsIndex = 0; }
				else if ( i <= 3 ) { imgPathsIndex = 1; }
				else if ( i <= 5 ) { imgPathsIndex = 2; }
				else if ( i <= 7 ) { imgPathsIndex = 3; }
				else if ( i <= 9 ) { imgPathsIndex = 4; }

				if ( i % 2 == 1 )
				{
					pipImgs[i]->disabled = true;
					pipImgs[i]->path = "";
					continue;
				}
			}
			if ( i < pipsTarget )
			{
				if ( pipImgs[i]->disabled )
				{
					pipImgs[i]->path = imgPaths[imgPathsIndex][imgPaths[imgPathsIndex].size() - 1];
				}
				else
				{
					if ( pipsFlashTick % tickUpdateSpeed == 0 && updatePip )
					{
						int currentIndex = imgPaths[imgPathsIndex].size() - 1;
						for ( auto& pair : imgPaths[imgPathsIndex] )
						{
							if ( pipImgs[i]->path == pair.second )
							{
								currentIndex = pair.first;
								break;
							}
						}

						int targetIndex = i + 1;
						if ( currentIndex > targetIndex )
						{
							--currentIndex;
						}
						pipImgs[i]->path = imgPaths[imgPathsIndex][currentIndex];
					}
				}
				pipImgs[i]->disabled = false;
			}
			else
			{
				pipImgs[i]->disabled = true;
				pipImgs[i]->path = "";
			}
		}
	}

	static ConsoleVariable<int> cvar_eternal_shrine_send_item_destx1("/eternal_shrine_send_item_destx1", 36);
	static ConsoleVariable<int> cvar_eternal_shrine_send_item_desty1("/eternal_shrine_send_item_desty1", 128);
	animSendItem1DestX = baseFrame->getSize().x + shroudTopFrame->getSize().x + shroundItemBg->pos.x;// + *cvar_eternal_shrine_send_item_destx1;
	animSendItem1DestY = baseFrame->getSize().y + shroudTopFrame->getSize().y + shroundItemBg->pos.y;// + *cvar_eternal_shrine_send_item_desty1;
	if ( viewActionReady )
	{
		animSendItem1DestX = baseFrame->getSize().x + shroudTopFrame->getSize().x + 80;
		animSendItem1DestY = baseFrame->getSize().y + shroudTopFrame->getSize().y + 174;

		if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
		{
			animSendItem1DestX += 2;
			animSendItem1DestY += 10;
		}
	}

	auto animSendItem1Frame = getEternalShrineSlotFrame(ETERNALSHRINE_SLOT_SEND, 0);
	bool updateSlotFrame = false;
	if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
	{
		if ( submittedItem == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE )
		{
			animSendItem1Frame->setDisabled(true);
			updateSlotFrame = true;
		}
	}
	else
	{
		if ( isInteractable )
		{
			animSendItem1Frame->setDisabled(true);
			updateSlotFrame = true;
		}
	}
	Item* eternalShrineSendItem1 = nullptr;

	{
		const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
		real_t setpointDiffX = fpsScale * std::max(.05, (animSendItem1)) / 3.0;
		animSendItem1 -= setpointDiffX;
		animSendItem1 = std::max(0.0, animSendItem1);
	}

	ItemType sendItemType = WOODEN_SHIELD;
	int sendItemModel = 0;

	if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ASCENSION 
		&& (ascensionType == SORCERY_SPELL || ascensionType == MYSTICISM_SPELL || ascensionType == THAUMATURGY_SPELL) )
	{
		sendItemType = SPELL_ITEM;
		if ( ascensionType == SORCERY_SPELL )
		{
			sendItemModel = NUM_SPELLS;
		}
		else if ( ascensionType == MYSTICISM_SPELL )
		{
			sendItemModel = NUM_SPELLS + 1;
		}
		else if ( ascensionType == THAUMATURGY_SPELL )
		{
			sendItemModel = NUM_SPELLS + 2;
		}
	}
	else if ( sendItem1Uid != 0 )
	{
		if ( eternalShrineSendItem1 = uidToItem(sendItem1Uid) )
		{
			sendItemType = eternalShrineSendItem1->type;
			if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ASCENSION && ascensionType == ASCENSION_SPELL
				&& eternalShrineSendItem1->type == SPELL_ITEM )
			{
				if ( auto spell = getSpellFromItem(playernum, eternalShrineSendItem1, false) )
				{
					sendItemModel = spell->ID;
				}
			}
			else
			{
				sendItemModel = itemModel(eternalShrineSendItem1);
			}

			if ( /*!eternalShrineSendItem1->identified || */itemIsEquipped(eternalShrineSendItem1, playernum) )
			{
				eternalShrineSendItem1 = nullptr; // if this got unidentified somehow, remove it
				sendItem1Uid = 0;
				animSendItem1 = 0.0;
			}
			else if ( updateSlotFrame )
			{
				animSendItem1Frame->setDisabled(false);
				if ( animSendItem1 < 0.001 )
				{
					animSendItem1Frame->setUserData(nullptr);
				}
				else
				{
					animSendItem1Frame->setUserData(&GAMEUI_FRAMEDATA_ANIMATING_ITEM);
				}
				int oldCount = eternalShrineSendItem1->count;
				if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
				{
					eternalShrineSendItem1->count = 1;
				}
				updateSlotFrameFromItem(animSendItem1Frame, eternalShrineSendItem1);
				eternalShrineSendItem1->count = oldCount;
			}
		}
	}

	if ( eternalShrineStation )
	{
		Entity* interacting = uidToEntity(eternalShrineStation->eternalShrineInteracting);
		if ( interacting && interacting->behavior == &actPlayer && players[interacting->skill[2]]->isLocalPlayer() )
		{
			if ( currentView == ASSIST_SHRINE_VIEW_WAITING )
			{

			}
			else
			{
				int submittedState = (eternalShrineStation->eternalShrinePlayerStates >> (playernum * 2)) & 0b11;
				if ( submittedState == EternalShrineSubmitStatus::SUBMIT_NONE 
					|| (currentView == ASSIST_SHRINE_VIEW_ACTION /*&& parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ANVIL*/) )
				{
					Sint32 offeringTypeModel = (sendItemModel & 0xFFFF) << 16;
					offeringTypeModel |= (int)(sendItemType & 0x0FFF) << 4;
					offeringTypeModel |= ((currentView) & 0xF);
					if ( multiplayer != CLIENT )
					{
						if ( eternalShrineStation->eternalShrineOfferingItemTypeModel != offeringTypeModel )
						{
							eternalShrineStation->eternalShrineOfferingItemTypeModel = offeringTypeModel;
							serverUpdateEntitySkill(eternalShrineStation, 16);
						}
					}
					else
					{
						if ( eternalShrineStation->eternalShrineOfferingItemTypeModel != offeringTypeModel )
						{
							eternalShrineStation->eternalShrineOfferingItemTypeModel = offeringTypeModel;
							clientUpdateShrineSkill(eternalShrineStation->getUID(), 16, eternalShrineStation->eternalShrineOfferingItemTypeModel, true);
						}
					}
				}
			}
		}
	}

	auto animSendItem1Pos = animSendItem1Frame->getSize();
	animSendItem1Pos.x = animSendItem1StartX + (1.0 - animSendItem1) * (animSendItem1DestX - animSendItem1StartX);
	animSendItem1Pos.y = animSendItem1StartY + (1.0 - animSendItem1) * (animSendItem1DestY - animSendItem1StartY);
	animSendItem1Frame->setSize(animSendItem1Pos);

	bSendItemAllowed = true;
	if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_SUPPLICATION
		|| parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_MUSIC )
	{
		if ( viewActionReady )
		{
			bSendItemAllowed = false;
		}
	}

	if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
	{
		if ( viewActionReady )
		{
			if ( player->inventory_mode == INVENTORY_MODE_ITEM )
			{
				player->inventoryUI.cycleInventoryTab();
			}
			if ( ascensionType != ASCENSION_SPELL )
			{
				bSendItemAllowed = false;
			}
		}
	}

	if ( viewActionReady )
	{
		if ( bSendItemAllowed )
		{
			animSendItem1Frame->setOpacity(100.0);
		}
		else
		{
			animSendItem1Frame->setOpacity(0.0);
		}
	}
	else
	{
		animSendItem1Frame->setOpacity(100.0 * animFilter);
	}

	if ( !bOpen )
	{
		return;
	}

	if ( !parentGUI.isGUIOpen()
		|| (parentGUI.guiType != GUICurrentType::GUI_TYPE_ETERNALSHRINE_ANVIL
			&& parentGUI.guiType != GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION
			&& parentGUI.guiType != GUICurrentType::GUI_TYPE_ETERNALSHRINE_MUSIC
			&& parentGUI.guiType != GUICurrentType::GUI_TYPE_ETERNALSHRINE_SUPPLICATION)
		|| !stats[playernum]
		|| stats[playernum]->HP <= 0
		|| !player->entity
		|| player->shootmode )
	{
		closeEternalShrine();
		return;
	}

	if ( player->entity && player->entity->isBlind() )
	{
		messagePlayer(playernum, MESSAGE_MISC, Language::get(4159));
		parentGUI.closeGUI();
		return; // I can't see!
	}

	// alembic status
	{
		auto title = baseFrame->findField("eternal title");
		if ( eternalShrineStation )
		{
			title->setText(Language::get(6983));
		}
		else
		{
			title->setText("");
		}

		SDL_Rect textPos{ 0, 21, baseFrame->getSize().w, 24 };
		textPos.y -= 14;
		title->setSize(textPos);
	}

	bool usingGamepad = inputs.hasController(playernum) && !inputs.getVirtualMouse(playernum)->draw_cursor;

	{
		// close btn
		auto closeBtn = baseFrame->findButton("close eternal button");
		auto closeGlyph = baseFrame->findImage("close eternal glyph");
		closeBtn->setDisabled(true);
		closeGlyph->disabled = true;
		if ( inputs.getVirtualMouse(playernum)->draw_cursor )
		{
			closeBtn->setDisabled(!isInteractable);
			if ( isInteractable )
			{
				buttonEternalShrineUpdateSelectorOnHighlight(playernum, closeBtn);
			}
		}
		else if ( closeBtn->isSelected() )
		{
			closeBtn->deselect();
		}
		if ( closeBtn->isDisabled() && usingGamepad && sendItem1Uid == 0 )
		{
			closeGlyph->path = Input::inputs[playernum].getGlyphPathForBinding("MenuCancel");
			if ( auto imgGet = Image::get(closeGlyph->path.c_str()) )
			{
				closeGlyph->pos.w = imgGet->getWidth();
				closeGlyph->pos.h = imgGet->getHeight();
				closeGlyph->disabled = false;
			}

			closeGlyph->pos.x = closeBtn->getSize().x - closeGlyph->pos.w;
			closeGlyph->pos.y = closeBtn->getSize().y + closeBtn->getSize().h / 2 - closeGlyph->pos.h / 2;
			if ( closeGlyph->pos.y % 2 == 1 )
			{
				++closeGlyph->pos.y;
			}
		}
	}

	Uint32 negativeColor = hudColors.characterSheetRed;
	Uint32 neutralColor = hudColors.characterSheetLightNeutral;
	Uint32 positiveColor = hudColors.characterSheetGreen;
	Uint32 secondaryPositiveColor = hudColors.characterSheetHighlightText;
	Uint32 defaultPromptColor = makeColor(255, 255, 255, 255);

	auto itemDisplayTooltip = baseFrame->findFrame("eternal display tooltip");
	itemDisplayTooltip->setDisabled(false);
	auto displayItemName = itemDisplayTooltip->findField("item display name");
	auto displayItemTextImg = itemDisplayTooltip->findImage("item text img");
	const int displayItemTextImgBaseX = 0;
	displayItemTextImg->pos.x = displayItemTextImgBaseX;
	displayItemTextImg->pos.y = 0;
	SDL_Rect displayItemNamePos{ displayItemTextImg->pos.x + 8, displayItemTextImg->pos.y - 4, 170, displayItemName->getSize().h };
	displayItemName->setSize(displayItemNamePos);

	SDL_Rect tooltipPos = itemDisplayTooltip->getSize();
	tooltipPos.w = 186;
	tooltipPos.h = baseFrame->getSize().h - 100;
	tooltipPos.y = currentView == ASSIST_SHRINE_VIEW_OFFERING ? 150 : 184;
	tooltipPos.x = 10;// 18 - (tooltipPos.w + 18) * (1.0 - animTooltip);
	itemDisplayTooltip->setSize(tooltipPos);

	bool modifierPressed = false;
	if ( usingGamepad && Input::inputs[playernum].binary("MenuPageLeftAlt") )
	{
		modifierPressed = true;
	}
	else if ( inputs.bPlayerUsingKeyboardControl(playernum)
		&& (keystatus[SDLK_LSHIFT] || keystatus[SDLK_RSHIFT]) )
	{
		modifierPressed = true;
	}

	bool inventoryControlActive = player->bControlEnabled
		&& !gamePaused
		&& !player->usingCommand()
		&& !player->GUI.isDropdownActive();

	if ( isInteractable && !inputs.getUIInteraction(playernum)->selectedItem
		&& inventoryControlActive
		&& player->GUI.bModuleAccessibleWithMouse(Player::GUI_t::MODULE_ETERNALSHRINE)
		&& player->GUI.bActiveModuleUsesInventory()
		&& bSendItemAllowed
		&& player->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE )
	{
		if ( getSelectedEternalShrineX() >= ETERNALSHRINE_SLOT_SEND && getSelectedEternalShrineX() < 0
			&& getSelectedEternalShrineY() == 0 )
		{
			if ( auto slotFrame = getEternalShrineSlotFrame(getSelectedEternalShrineX(), getSelectedEternalShrineY()) )
			{
				if ( !slotFrame->isDisabled() && slotFrame->capturesMouse() )
				{
					setItemDisplayNameAndPrice(eternalShrineSendItem1, false);
				}
			}
		}
	}

	auto offeringBtn = baseFrame->findButton("offering button");
	auto offeringGlyph = baseFrame->findImage("offering glyph");
	offeringBtn->setDisabled(true);
	offeringBtn->setText(Language::get(7000));
	offeringGlyph->disabled = true;

	auto actionBtn = baseFrame->findButton("action button");
	actionBtn->setDisabled(true);
	if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
	{
		if ( ascensionType == ASCENSION_SPELL )
		{
			actionBtn->setText(Language::get(7086));
		}
		else
		{
			actionBtn->setText(Language::get(7085));
		}
	}
	else if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ANVIL )
	{
		actionBtn->setText(Language::get(7001));
	}
	else if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
	{
		actionBtn->setText(Language::get(7083));
	}
	else if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_MUSIC )
	{
		actionBtn->setText(Language::get(7084));
	}

	auto arrowLeftBtn = baseFrame->findButton("arrow left button");
	auto arrowRightBtn = baseFrame->findButton("arrow right button");
	arrowLeftBtn->setDisabled(true);
	arrowRightBtn->setDisabled(true);
	arrowLeftBtn->setInvisible(true);
	arrowRightBtn->setInvisible(true);
	auto ascensionImg = baseFrame->findImage("ascension img");
	ascensionImg->disabled = true;

	if ( currentView == ASSIST_SHRINE_VIEW_ACTION )
	{
		offeringBtn->setDisabled(true);
		offeringBtn->setInvisible(true);
		if ( offeringBtn->isSelected() )
		{
			offeringBtn->deselect();
		}

		if ( viewActionReady )
		{
			real_t alphaRatio = animAction;
			if ( actionBtn->isInvisible() )
			{
				actionBtn->setInvisible(false);
			}
			if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
			{
				if ( isInteractable && sendItem1Uid == 0 )
				{
					arrowLeftBtn->setInvisible(false);
					arrowRightBtn->setInvisible(false);
					ascensionImg->disabled = false;
				}
			}

			auto pos = actionBtn->getSize();
			pos.x = baseFramePos.w / 2 - pos.w / 2;
			pos.y = 260;
			actionBtn->setSize(pos);

			SDL_Color color;
			getColor(actionBtn->getColor(), &color.r, &color.g, &color.b, &color.a);

			color.a = (Uint8)(255 * alphaRatio);
			actionBtn->setColor(makeColor(color.r, color.g, color.b, color.a));
			actionBtn->setHighlightColor(makeColor(255, 255, 255, color.a));
			actionBtn->setTextColor(actionBtn->getColor());
			actionBtn->setTextHighlightColor(makeColor(201, 162, 100, 255 * alphaRatio));

			if ( !arrowLeftBtn->isInvisible() )
			{
				arrowLeftBtn->setColor(makeColor(255, 255, 255, color.a));
				arrowLeftBtn->setHighlightColor(makeColor(255, 255, 255, color.a));
				SDL_Rect pos = arrowLeftBtn->getSize();
				pos.x = 46;
				pos.y = 194;
				arrowLeftBtn->setSize(pos);
			}
			if ( !arrowRightBtn->isInvisible() )
			{
				arrowRightBtn->setColor(makeColor(255, 255, 255, color.a));
				arrowRightBtn->setHighlightColor(makeColor(255, 255, 255, color.a));
				SDL_Rect pos = arrowRightBtn->getSize();
				pos.x = 140;
				pos.y = 194;
				arrowRightBtn->setSize(pos);

				ascensionImg->color = arrowLeftBtn->getColor();
				if ( ascensionType == ASCENSION_SPELL )
				{
					ascensionImg->disabled = true;
					ascensionImg->path = "images/system/white.png";
					ascensionImg->pos.w = 32;
					ascensionImg->pos.h = 32;
				}
				else
				{
					for ( auto& skill : Player::SkillSheet_t::skillSheetData.skillEntries )
					{
						if ( (skill.skillId == PRO_SORCERY && ascensionType == SORCERY_SPELL)
							|| (skill.skillId == PRO_MYSTICISM && ascensionType == MYSTICISM_SPELL)
							|| (skill.skillId == PRO_THAUMATURGY && ascensionType == THAUMATURGY_SPELL) )
						{
							ascensionImg->path = skill.skillIconPath32px;
							if ( auto imgGet = Image::get(ascensionImg->path.c_str()) )
							{
								ascensionImg->pos.w = imgGet->getWidth();
								ascensionImg->pos.h = imgGet->getHeight();
							}
							break;
						}
					}
				}

				ascensionImg->pos.x = 87;
				ascensionImg->pos.y = 189;
			}

			if ( inputs.getVirtualMouse(playernum)->draw_cursor )
			{
				if ( (sendItem1Uid != 0 || !bSendItemAllowed) && isInteractable )
				{
					actionBtn->setDisabled(false);
					actionBtn->setTextColor(makeColor(255, 255, 255, 255 * alphaRatio));
					buttonEternalShrineUpdateSelectorOnHighlight(playernum, actionBtn);
				}
				else
				{
					SDL_Color color;
					getColor(hudColors.characterSheetFaintText, &color.r, &color.g, &color.b, &color.a);
					color.a = (Uint8)(255 * alphaRatio);
					actionBtn->setTextColor(makeColor(color.r, color.g, color.b, color.a));
				}

				if ( !arrowLeftBtn->isInvisible() )
				{
					arrowLeftBtn->setDisabled(false);
					buttonEternalShrineUpdateSelectorOnHighlight(playernum, arrowLeftBtn);
				}
				if ( !arrowRightBtn->isInvisible() )
				{
					arrowRightBtn->setDisabled(false);
					buttonEternalShrineUpdateSelectorOnHighlight(playernum, arrowRightBtn);
				}
			}
			else
			{
				if ( actionBtn->isSelected() )
				{
					actionBtn->deselect();
				}
				if ( arrowLeftBtn->isSelected() )
				{
					arrowLeftBtn->deselect();
				}
				if ( arrowRightBtn->isSelected() )
				{
					arrowRightBtn->deselect();
				}
			}

			if ( usingGamepad && !inputs.getVirtualMouse(playernum)->draw_cursor )
			{
				if ( !isInteractable )
				{
					SDL_Color color;
					getColor(hudColors.characterSheetFaintText, &color.r, &color.g, &color.b, &color.a);
					color.a = (Uint8)(255 * alphaRatio);
					actionBtn->setTextColor(makeColor(color.r, color.g, color.b, color.a));
				}
				else
				{
					actionBtn->setTextColor(makeColor(255, 255, 255, 255 * alphaRatio));
				}
				actionBtn->setDisabled(true);
				if ( isInteractable )
				{
					offeringGlyph->path = Input::inputs[playernum].getGlyphPathForBinding("MenuAlt2");
					if ( auto imgGet = Image::get(offeringGlyph->path.c_str()) )
					{
						offeringGlyph->pos.w = imgGet->getWidth();
						offeringGlyph->pos.h = imgGet->getHeight();
						offeringGlyph->disabled = false;
					}
					offeringGlyph->pos.x = actionBtn->getSize().x + actionBtn->getSize().w - 16;
					if ( offeringGlyph->pos.x % 2 == 1 )
					{
						++offeringGlyph->pos.x;
					}
					offeringGlyph->pos.y = actionBtn->getSize().y + actionBtn->getSize().h - 16;
				}
			}
		}

		if ( offeringBtn->isSelected() )
		{
			offeringBtn->deselect();
		}
	}
	else 
	{
		animAction = 0.0;
		actionBtn->setInvisible(true);

		if ( bSkipOfferingPrompt || currentView == ASSIST_SHRINE_VIEW_WAITING )
		{
			offeringBtn->setInvisible(true);
		}
		else if ( !bSkipOfferingPrompt )
		{
			offeringBtn->setInvisible(false);

			auto pos = offeringBtn->getSize();
			pos.x = baseFramePos.w / 2 - pos.w / 2;
			pos.y = 250;
			offeringBtn->setSize(pos);

			SDL_Color color;
			getColor(offeringBtn->getColor(), &color.r, &color.g, &color.b, &color.a);

			real_t alphaRatio = (1.0 - animSubmit);
			color.a = (Uint8)(255 * alphaRatio);
			offeringBtn->setColor(makeColor(color.r, color.g, color.b, color.a));
			offeringBtn->setHighlightColor(makeColor(255, 255, 255, color.a));
			offeringBtn->setTextColor(offeringBtn->getColor());
			offeringBtn->setTextHighlightColor(makeColor(201, 162, 100, 255 * alphaRatio));

			if ( inputs.getVirtualMouse(playernum)->draw_cursor )
			{
				if ( (sendItem1Uid != 0) && isInteractable )
				{
					offeringBtn->setDisabled(false);
					offeringBtn->setTextColor(makeColor(255, 255, 255, 255 * alphaRatio));
					buttonEternalShrineUpdateSelectorOnHighlight(playernum, offeringBtn);
				}
				else
				{
					SDL_Color color;
					getColor(hudColors.characterSheetFaintText, &color.r, &color.g, &color.b, &color.a);
					color.a = (Uint8)(255 * alphaRatio);
					offeringBtn->setTextColor(makeColor(color.r, color.g, color.b, color.a));
				}
			}
			else if ( offeringBtn->isSelected() )
			{
				offeringBtn->deselect();
			}
			if ( usingGamepad && !inputs.getVirtualMouse(playernum)->draw_cursor )
			{
				if ( !isInteractable/*potionResultFrame->isDisabled() || alchemyMissingIngredientQty(nullptr)*/ )
				{
					SDL_Color color;
					getColor(hudColors.characterSheetFaintText, &color.r, &color.g, &color.b, &color.a);
					color.a = (Uint8)(255 * alphaRatio);
					offeringBtn->setTextColor(makeColor(color.r, color.g, color.b, color.a));
				}
				else
				{
					offeringBtn->setTextColor(makeColor(255, 255, 255, 255 * alphaRatio));
				}
				offeringBtn->setDisabled(true);
				if ( isInteractable/*!potionResultFrame->isDisabled() && !alchemyMissingIngredientQty(nullptr)*/ )
				{
					offeringGlyph->path = Input::inputs[playernum].getGlyphPathForBinding("MenuAlt2");
					if ( auto imgGet = Image::get(offeringGlyph->path.c_str()) )
					{
						offeringGlyph->pos.w = imgGet->getWidth();
						offeringGlyph->pos.h = imgGet->getHeight();
						offeringGlyph->disabled = false;
					}
					offeringGlyph->pos.x = offeringBtn->getSize().x + offeringBtn->getSize().w - 16;
					if ( offeringGlyph->pos.x % 2 == 1 )
					{
						++offeringGlyph->pos.x;
					}
					offeringGlyph->pos.y = offeringBtn->getSize().y + offeringBtn->getSize().h - 16;
				}
			}
		}
	}

	if ( !arrowLeftBtn->isInvisible() )
	{
		if ( arrowLeftBtn->isSelected() )
		{
			arrowLeftBtn->deselect();
		}
	}
	if ( !arrowRightBtn->isInvisible() )
	{
		if ( arrowRightBtn->isSelected() )
		{
			arrowRightBtn->deselect();
		}
	}

	auto activateSelectionGlyph = eternalShrineFrame->findImage("activate glyph");
	auto activateSelectionPrompt = eternalShrineFrame->findField("activate prompt");
	if ( !strcmp(activateSelectionPrompt->getText(), "") )
	{
		activateSelectionGlyph->disabled = true;
		activateSelectionPrompt->setDisabled(true);
	}
	itemDisplayTooltip->setDisabled(true);

	if ( currentView == ASSIST_SHRINE_VIEW_WAITING )
	{
		animTooltip = 0.0;
	}
	else if ( itemType != -1 && itemDesc.size() > 1 
		&& (submittedItem == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE
			|| currentView == ASSIST_SHRINE_VIEW_ACTION) )
	{
		if ( isInteractable )
		{
			//const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
			//real_t setpointDiffX = fpsScale * std::max(.01, (1.0 - animTooltip)) / 2.0;
			//animTooltip += setpointDiffX;
			//animTooltip = std::min(1.0, animTooltip);
			animTooltip = 1.0;
			animTooltipTicks = ticks;
		}

		{
			// item name + text bg
			displayItemName->setVJustify(Field::justify_t::CENTER);
			displayItemName->setHJustify(Field::justify_t::CENTER);
		}

		if ( itemActionType == ETERNAL_ITEM_OK && strcmp(activateSelectionPrompt->getText(), "") )
		{
			if ( usingGamepad )
			{
				activateSelectionGlyph->path = Input::inputs[playernum].getGlyphPathForBinding("MenuConfirm");
			}
			else
			{
				activateSelectionGlyph->path = Input::inputs[playernum].getGlyphPathForBinding("MenuRightClick");
			}
			if ( auto imgGet = Image::get(activateSelectionGlyph->path.c_str()) )
			{
				activateSelectionGlyph->pos.w = imgGet->getWidth();
				activateSelectionGlyph->pos.h = imgGet->getHeight();
			}

			SDL_Rect pos{ 0, 0, activateSelectionGlyph->pos.w, activateSelectionGlyph->pos.h };
			pos.x += baseFrame->getSize().x + itemDisplayTooltip->getSize().x;
			pos.y += baseFrame->getSize().y + itemDisplayTooltip->getSize().y;
			pos.x += displayItemTextImg->pos.x + displayItemTextImg->pos.w / 2;
			pos.y += displayItemTextImg->pos.y + displayItemTextImg->pos.h;
			pos.y += 4;

			auto activateSelectionPromptPos = SDL_Rect{ pos.x, pos.y + 1, baseFrame->getSize().w, 24 };
			if ( auto textGet = activateSelectionPrompt->getTextObject() )
			{
				activateSelectionPromptPos.x -= textGet->getWidth() / 2;
				activateSelectionPromptPos.x += (8 + pos.w) / 2;
				pos.x = activateSelectionPromptPos.x - 8 - pos.w;
				pos.y += activateSelectionPrompt->getSize().h / 2;
				pos.y -= pos.h / 2;
			}
			activateSelectionPrompt->setSize(activateSelectionPromptPos);
			if ( pos.x % 2 == 1 )
			{
				++pos.x;
			}
			if ( pos.y % 2 == 1 )
			{
				--pos.y;
			}
			activateSelectionGlyph->pos = pos;
			activateSelectionGlyph->disabled = false;
			activateSelectionPrompt->setDisabled(false);
		}
	}
	else
	{
		if ( (!usingGamepad && (ticks - animTooltipTicks > TICKS_PER_SECOND / 3))
			|| (usingGamepad)
			|| animTooltip < 0.9999 )
		{
			const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
			real_t setpointDiffX = fpsScale * std::max(.01, (animTooltip)) / 2.0;
			animTooltip -= setpointDiffX;
			animTooltip = std::max(0.0, animTooltip);
		}
	}

	{
		SDL_Color color;
		getColor(displayItemTextImg->color, &color.r, &color.g, &color.b, &color.a);
		color.a = (Uint8)(192 * animTooltip);
		displayItemTextImg->color = (makeColor(color.r, color.g, color.b, color.a));
	}

	{
		SDL_Color color;
		getColor(displayItemName->getColor(), &color.r, &color.g, &color.b, &color.a);
		color.a = (Uint8)(255 * animTooltip);
		displayItemName->setColor(makeColor(color.r, color.g, color.b, color.a));
	}

	{
		SDL_Color color;
		getColor(activateSelectionPrompt->getColor(), &color.r, &color.g, &color.b, &color.a);
		color.a = (Uint8)(255 * animTooltip);
		activateSelectionPrompt->setColor(makeColor(color.r, color.g, color.b, color.a));
	}

	{
		SDL_Color color;
		getColor(activateSelectionGlyph->color, &color.r, &color.g, &color.b, &color.a);
		color.a = (Uint8)(255 * animTooltip);
		activateSelectionGlyph->color = (makeColor(color.r, color.g, color.b, color.a));
	}

	bool tryBrew = false;
	bool activateSelection = false;
	if ( isInteractable )
	{
		if ( !inputs.getUIInteraction(playernum)->selectedItem
			&& !player->GUI.isDropdownActive()
			&& (player->GUI.bModuleAccessibleWithMouse(Player::GUI_t::MODULE_ETERNALSHRINE)
				|| player->GUI.bModuleAccessibleWithMouse(Player::GUI_t::MODULE_INVENTORY))
			&& player->bControlEnabled && !gamePaused
			&& !player->usingCommand() )
		{

			if ( Input::inputs[playernum].binaryToggle("MenuCancel") )
			{
				Input::inputs[playernum].consumeBinaryToggle("MenuCancel");
				if ( sendItem1Uid == 0 )
				{
					parentGUI.closeGUI();
					Player::soundCancel();
					return;
				}
				else if ( true /*animRecvItem < 0.001*/ )
				{
					if ( sendItem1Uid != 0 )
					{
						sendItem1Uid = 0;
						animSendItem1 = 0.0;
						//animPotion1Frame->setDisabled(true);
					}
					Player::soundCancel();
				}
			}
			else
			{
				if ( usingGamepad && Input::inputs[playernum].binaryToggle("MenuConfirm") )
				{
					activateSelection = true;
					Input::inputs[playernum].consumeBinaryToggle("MenuConfirm");
				}
				else if ( !usingGamepad && Input::inputs[playernum].binaryToggle("MenuRightClick") )
				{
					activateSelection = true;
					Input::inputs[playernum].consumeBinaryToggle("MenuRightClick");
				}
			}
		}
	}

	if ( activateSelection && players[playernum] && players[playernum]->entity
		/*&& animRecvItem < 0.001*/ )
	{
		if ( itemActionType != ETERNAL_ITEM_OK && itemActionType != ETERNAL_ITEM_NONE )
		{
			playSound(90, 64);
		}
		if ( (player->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE || (tryBrew && player->GUI.activeModule == Player::GUI_t::MODULE_INVENTORY))
			&& (itemActionType == ETERNAL_ITEM_OK || tryBrew)
			)
		{
			ItemType oldPotion1Type = WOODEN_SHIELD;
			ItemType oldPotion2Type = WOODEN_SHIELD;
			if ( (getSelectedEternalShrineX() >= ETERNALSHRINE_SLOT_SEND && getSelectedEternalShrineX() < 0
				&& getSelectedEternalShrineY() == 0) || tryBrew )
			{
				if ( !tryBrew && getSelectedEternalShrineX() == ETERNALSHRINE_SLOT_SEND )
				{
					sendItem1Uid = 0;
					animSendItem1 = 0.0;
					animSendItem1Frame->setDisabled(true);
					Player::soundCancel();
				}
			}
		}
		else if ( player->GUI.activeModule == Player::GUI_t::MODULE_INVENTORY
			&& itemActionType == ETERNAL_ITEM_OK )
		{
			if ( auto slotFrame = player->inventoryUI.getInventorySlotFrame(player->inventoryUI.getSelectedSlotX(),
				player->inventoryUI.getSelectedSlotY()) )
			{
				for ( node_t* node = stats[playernum]->inventory.first; node != NULL; node = node->next )
				{
					Item* item = (Item*)node->element;
					if ( !item )
					{
						continue;
					}
					if ( itemCategory(item) == SPELL_CAT )
					{
						continue;
					}

					if ( item->x == player->inventoryUI.getSelectedSlotX()
						&& item->y == player->inventoryUI.getSelectedSlotY()
						&& item->x >= 0 && item->x < player->inventoryUI.getPlayerItemInventoryX()
						&& item->y >= 0 && item->y < player->inventoryUI.getPlayerItemInventoryY() )
					{
						if ( sendItem1Uid == item->uid )
						{
							sendItem1Uid = 0;
							animSendItem1 = 0.0;
							animSendItem1Frame->setDisabled(true);
							Player::soundCancel();
						}
						else
						{
							if ( sendItem1Uid == 0 || true )
							{
								if ( !parentGUI.isItemEternalShrineUsable(item) )
								{
									continue;
								}
								animSendItem1 = 1.0;
								getInventoryItemEternalShrineAnimSlotPos(slotFrame, player, item->x, item->y, animSendItem1StartX, animSendItem1StartY, eternalShrineItemAnimOffsetY);
								sendItem1Uid = item->uid;
								//alchemyResultPotion.type = POTION_EMPTY;
								playSound(139, 64); // click sound
							}
						}
						break;
					}
				}
			}
		}
		else if ( player->GUI.activeModule == Player::GUI_t::MODULE_SPELLS && parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION
			&& itemActionType == ETERNAL_ITEM_OK )
		{
			if ( auto slotFrame = player->inventoryUI.getSpellSlotFrame(player->inventoryUI.getSelectedSpellX(),
				player->inventoryUI.getSelectedSpellY()) )
			{
				for ( node_t* node = stats[playernum]->inventory.first; node != NULL; node = node->next )
				{
					Item* item = (Item*)node->element;
					if ( !item )
					{
						continue;
					}
					if ( itemCategory(item) != SPELL_CAT )
					{
						continue;
					}

					if ( item->x == player->inventoryUI.getSelectedSpellX()
						&& item->y == player->inventoryUI.getSelectedSpellY()
						&& item->x >= 0 && item->x < player->inventoryUI.MAX_SPELLS_X
						&& item->y >= 0 && item->y < player->inventoryUI.MAX_SPELLS_Y )
					{
						if ( sendItem1Uid == item->uid )
						{
							sendItem1Uid = 0;
							animSendItem1 = 0.0;
							animSendItem1Frame->setDisabled(true);
							Player::soundCancel();
						}
						else
						{
							if ( sendItem1Uid == 0 || true )
							{
								if ( !parentGUI.isItemEternalShrineUsable(item) )
								{
									continue;
								}
								animSendItem1 = 1.0;
								getInventoryItemEternalShrineAnimSlotPos(slotFrame, player, item->x, item->y, animSendItem1StartX, animSendItem1StartY, eternalShrineItemAnimOffsetY);
								sendItem1Uid = item->uid;
								//alchemyResultPotion.type = POTION_EMPTY;
								playSound(139, 64); // click sound
							}
						}
						break;
					}
				}
			}
		}
	}
}

//void GenericGUIMenu::mailboxClaimItem()
//{
//	//auto& item = mailboxGUI.mailReceiveItem;
//
//	//Item* claimedItem = newItem(item.type, item.status, item.beatitude, item.count, item.appearance, item.identified, nullptr);
//	//Item* pickedUp = itemPickup(gui_player, claimedItem);
//	//if ( pickedUp )
//	//{
//	//	mailboxGUI.recvItemUid = pickedUp->uid;
//	//	messagePlayer(gui_player, MESSAGE_MISC, Language::get(504), claimedItem->description());
//	//	//mailboxGUI.animPotionResultCount = alchemyGUI.alchemyResultPotion.count;
//	//	playSoundEntity(players[gui_player]->entity, 35 + local_rng.rand() % 3, 64);
//	//}
//
//	//free(claimedItem);
//	//claimedItem = nullptr;
//
//	//mailboxGUI.mailReceiveItem.type = POTION_EMPTY;
//}

void GenericGUIMenu::EternalShrineGUI_t::createEternalShrine()
{
	const int player = parentGUI.getPlayer();
	if ( !gui || !eternalShrineFrame || !players[player]->inventoryUI.frame )
	{
		return;
	}
	if ( eternalShrineGUIHasBeenCreated() )
	{
		return;
	}

	SDL_Rect basePos{ 0, 0, eternalShrineBaseWidth, 292 };
	eternalShrineSlotFrames.clear();

	const int inventorySlotSize = players[player]->inventoryUI.getSlotSize();

	/*{
		auto notificationFrame = alchFrame->addFrame("notification");
		notificationFrame->setHollow(false);
		notificationFrame->setBorder(0);
		notificationFrame->setInheritParentFrameOpacity(false);
		notificationFrame->setDisabled(true);
		notificationFrame->setSize(SDL_Rect{ 0, 0, 180, 56 });

		auto notifBg = notificationFrame->addImage(SDL_Rect{ 0, 0, 180, 56 }, 0xFFFFFFFF,
			"*#images/ui/Alchemy/Alchemy_Notification_00.png", "notif bg");

		auto notifIcon = notificationFrame->addImage(SDL_Rect{ 8, 56 / 2 - players[player]->inventoryUI.getItemSpriteSize() / 2,
			players[player]->inventoryUI.getItemSpriteSize(),
			players[player]->inventoryUI.getItemSpriteSize() }, 0xFFFFFFFF,
			"", "notif icon");

		auto title = notificationFrame->addField("notif title", 128);
		title->setFont("fonts/pixel_maz_multiline.ttf#16#2");
		title->setText("New Title Unlocked!");
		title->setHJustify(Field::justify_t::LEFT);
		title->setVJustify(Field::justify_t::TOP);
		title->setSize(SDL_Rect{ notifIcon->pos.x + notifIcon->pos.w, 8, notificationFrame->getSize().w, 24 });
		title->setColor(makeColor(255, 255, 0, 255));

		auto body = notificationFrame->addField("notif body", 128);
		body->setFont("fonts/pixel_maz_multiline.ttf#16#2");
		body->setText("Blah Blah Blah!");
		body->setHJustify(Field::justify_t::LEFT);
		body->setVJustify(Field::justify_t::TOP);
		body->setSize(SDL_Rect{ notifIcon->pos.x + notifIcon->pos.w, 8 + 18, notificationFrame->getSize().w, 24 });
		body->setColor(makeColor(255, 255, 255, 255));
	}*/

	{
		auto bgFrame = eternalShrineFrame->addFrame("eternal base");
		bgFrame->setSize(basePos);
		bgFrame->setHollow(false);
		bgFrame->setDisabled(true);
		auto bg = bgFrame->addImage(SDL_Rect{ 0, 0, basePos.w, basePos.h },
			makeColor(255, 255, 255, 255),
			"*#images/ui/Shrines/divine_anvil/Anvil_Base_01.png", "eternal base img");

		{
			auto pipsFrame = bgFrame->addFrame("eternal pips");
			pipsFrame->setHollow(true);
			pipsFrame->setSize(SDL_Rect{ 0, 0, basePos.w, basePos.h });

			auto pipsLinks = pipsFrame->addImage(SDL_Rect{46, 88, 2, 2}, 
				0xFFFFFFFF, "#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_Connectors_00.png", "pips links");
			pipsLinks->tiled = true;
			pipsLinks->disabled = true;

			for ( int i = 0; i < 10; ++i )
			{
				std::string pipsName = "pip " + std::to_string(i);
				auto pipImg = pipsFrame->addImage(SDL_Rect{ pipsLinks->pos.x - 6 + i * 12, pipsLinks->pos.y - 2, 6, 6},
				0xFFFFFFFF, "*#images/ui/Shrines/divine_anvil/Anvil_Bar_Gem_1DarkRed_00.png", pipsName.c_str());
				pipImg->disabled = true;
			}
		}

		auto headerFont = "fonts/pixel_maz_multiline.ttf#16#2";
		auto title = bgFrame->addField("eternal title", 128);
		title->setFont(headerFont);
		title->setText("");
		title->setHJustify(Field::justify_t::CENTER);
		title->setVJustify(Field::justify_t::TOP);
		title->setSize(SDL_Rect{ 0, 0, 0, 0 });
		title->setTextColor(hudColors.characterSheetLightNeutral);
		title->setOutlineColor(makeColor(29, 16, 11, 255));
		/*auto alembicStatus = bgFrame->addField("alchemy alembic status", 128);
		alembicStatus->setFont(headerFont);
		alembicStatus->setText("");
		alembicStatus->setHJustify(Field::justify_t::CENTER);
		alembicStatus->setVJustify(Field::justify_t::TOP);
		alembicStatus->setSize(SDL_Rect{ 0, 0, 0, 0 });
		alembicStatus->setTextColor(hudColors.characterSheetLightNeutral);
		alembicStatus->setOutlineColor(makeColor(29, 16, 11, 255));*/

		auto itemFont = "fonts/pixel_maz_multiline.ttf#16#2";
		{
			auto itemDisplayTooltip = bgFrame->addFrame("eternal display tooltip");
			itemDisplayTooltip->setSize(SDL_Rect{ 0, 0, 186, 108 });
			itemDisplayTooltip->setHollow(true);
			itemDisplayTooltip->setInheritParentFrameOpacity(false);
			{
				auto itemNameText = itemDisplayTooltip->addField("item display name", 1024);
				itemNameText->setFont(itemFont);
				itemNameText->setText("");
				itemNameText->setHJustify(Field::justify_t::LEFT);
				itemNameText->setVJustify(Field::justify_t::TOP);
				itemNameText->setSize(SDL_Rect{ 0, 0, 0, 0 });
				itemNameText->setTextColor(hudColors.characterSheetLightNeutral);

				auto itemDisplayTextBg = itemDisplayTooltip->addImage(SDL_Rect{ 0, 0, 186, 42 },
					0xFFFFFFFF, "*#images/ui/Alchemy/Alchemy_LabelName_2Row_00.png", "item text img");
			}
		}

		{
			auto closeBtn = bgFrame->addButton("close eternal button");
			SDL_Rect closeBtnPos{ basePos.w - 0 - 26, 0, 26, 26 };
			closeBtn->setSize(closeBtnPos);
			closeBtn->setColor(makeColor(255, 255, 255, 255));
			closeBtn->setHighlightColor(makeColor(255, 255, 255, 255));
			closeBtn->setText("X");
			closeBtn->setFont(itemFont);
			closeBtn->setHideGlyphs(true);
			closeBtn->setHideKeyboardGlyphs(true);
			closeBtn->setHideSelectors(true);
			closeBtn->setMenuConfirmControlType(0);
			closeBtn->setBackground("*#images/ui/Alchemy/Button_X_00.png");
			closeBtn->setBackgroundHighlighted("*#images/ui/Alchemy/Button_XHigh_00.png");
			closeBtn->setBackgroundActivated("*#images/ui/Alchemy/Button_XPress_00.png");
			closeBtn->setTextHighlightColor(makeColor(201, 162, 100, 255));
			closeBtn->setCallback([](Button& button) {
				GenericGUI[button.getOwner()].closeGUI();
				Player::soundCancel();
				});
			closeBtn->setTickCallback(genericgui_deselect_fn);

			auto closeGlyph = bgFrame->addImage(SDL_Rect{ 0, 0, 0, 0 },
				0xFFFFFFFF, "", "close eternal glyph");
			closeGlyph->disabled = true;
			closeGlyph->ontop = true;
		}

		{
			Frame* shroudFrame = bgFrame->addFrame("eternal shroud frame");
			shroudFrame->setSize(basePos);
			shroudFrame->setInheritParentFrameOpacity(false);
			shroudFrame->setHollow(true);

			Frame* shroudTopFrame = shroudFrame->addFrame("eternal shroud top frame");
			shroudTopFrame->setSize(SDL_Rect{ 0, 0, basePos.w, basePos.h });
			shroudTopFrame->setInheritParentFrameOpacity(false);
			shroudTopFrame->setHollow(true);

			shroudTopFrame->addImage(SDL_Rect{ 0, 46, basePos.w, 246 }, 0xFFFFFFFF,
				"*#images/ui/Shrines/divine_anvil/Anvil_Shroud_01.png", "eternal shroud bg");

			auto badge = shroudTopFrame->addImage(SDL_Rect{ 56, 0, 94, 94 }, 0xFFFFFFFF,
				"*#images/ui/Shrines/divine_anvil/Anvil_Seal00.png", "eternal shroud badge");
			shroudTopFrame->addImage(SDL_Rect{ badge->pos.x + 26, badge->pos.y + 24, 42, 42 }, 0xFFFFFFFF,
				"*#images/ui/Shrines/divine_anvil/Anvil_Item00.png", "eternal shroud item bg");

			Frame* slotFrame = eternalShrineFrame->addFrame("eternal send frame");
			SDL_Rect slotPos{ 0, 0, players[player]->inventoryUI.getSlotSize(), players[player]->inventoryUI.getSlotSize() };
			slotFrame->setSize(slotPos);
			slotFrame->setDisabled(true);
			slotFrame->setInheritParentFrameOpacity(false);
			createPlayerInventorySlotFrameElements(slotFrame);
			eternalShrineSlotFrames[ETERNALSHRINE_SLOT_SEND + 0 * 100] = slotFrame;

			/*slotFrame = eternalShrineFrame->addFrame("eternal recv frame");
			slotFrame->setSize(slotPos);
			slotFrame->setDisabled(true);
			slotFrame->setInheritParentFrameOpacity(false);
			createPlayerInventorySlotFrameElements(slotFrame);
			mailSlotFrames[MAIL_SLOT_RECV + 0 * 100] = slotFrame;*/
		}

		auto activateSelectionGlyph = eternalShrineFrame->addImage(SDL_Rect{ 0, 0, 0, 0 },
			0xFFFFFFFF, "", "activate glyph");
		activateSelectionGlyph->disabled = true;
		activateSelectionGlyph->ontop = true;
		auto activateSelectionPrompt = eternalShrineFrame->addField("activate prompt", 64);
		activateSelectionPrompt->setFont("fonts/pixel_maz_multiline.ttf#16#2");
		activateSelectionPrompt->setText("");
		activateSelectionPrompt->setHJustify(Field::justify_t::LEFT);
		activateSelectionPrompt->setVJustify(Field::justify_t::TOP);
		activateSelectionPrompt->setSize(SDL_Rect{ 0, 0, 0, 0 });
		activateSelectionPrompt->setColor(makeColor(255, 255, 255, 255));
		activateSelectionPrompt->setDisabled(true);
		activateSelectionPrompt->setOntop(true);

		auto offeringPrompt = eternalShrineFrame->addField("offering prompt", 64);
		offeringPrompt->setFont("fonts/pixel_maz_multiline.ttf#16#2");
		offeringPrompt->setText("");
		offeringPrompt->setHJustify(Field::justify_t::LEFT);
		offeringPrompt->setVJustify(Field::justify_t::TOP);
		offeringPrompt->setSize(SDL_Rect{ 0, 0, 0, 0 });
		offeringPrompt->setColor(makeColor(255, 255, 255, 255));
		offeringPrompt->setDisabled(true);
		offeringPrompt->setOntop(true);

		auto offeringButton = bgFrame->addButton("offering button");
		SDL_Rect offeringBtnPos{ 0, 0, 188, 26 };
		offeringButton->setSize(offeringBtnPos);
		offeringButton->setColor(makeColor(255, 255, 255, 255));
		offeringButton->setHighlightColor(makeColor(255, 255, 255, 255));
		offeringButton->setText(Language::get(7000));
		offeringButton->setFont(itemFont);
		offeringButton->setHideGlyphs(true);
		offeringButton->setHideKeyboardGlyphs(true);
		offeringButton->setHideSelectors(true);
		offeringButton->setMenuConfirmControlType(0);
		offeringButton->setBackground("*#images/ui/Shrines/Button_Request_00.png");
		offeringButton->setBackgroundHighlighted("*#images/ui/Shrines/Button_RequestHigh_00.png");
		offeringButton->setBackgroundActivated("*#images/ui/Shrines/Button_RequestPress_00.png");
		offeringButton->setTextHighlightColor(makeColor(201, 162, 100, 255));
		offeringButton->setCallback([](Button& button) {
			int player = button.getOwner();
			auto& gui = GenericGUI[player].eternalShrineGUI;
			Player::soundActivate();
			if ( gui.submittedItem == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE
				&& !(gui.clientOfferingShrineType > 0 && multiplayer == CLIENT) )
			{
				if ( gui.sendItem1Uid != 0 )
				{
					if ( Item* item = uidToItem(gui.sendItem1Uid) )
					{
						if ( Entity* shrine = uidToEntity(GenericGUI[player].eternalShrineEntityUid) )
						{
							int qty = 1;
							if ( eternalShrineOnOfferItem(player, shrine->getUID(), item, 1) )
							{
								consumeItem(item, player);
								gui.sendItem1Uid = 0;
								gui.submittedItem = GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_WAITING;
								gui.submitTick = ticks;
								//gui.pipsQueued += 2;
							}
						}
					}
				}
			}
			});
		offeringButton->setTickCallback(genericgui_deselect_fn);
		offeringButton->setOntop(true);

		auto actionButton = bgFrame->addButton("action button");
		SDL_Rect actionBtnPos{ 0, 0, 82, 26 };
		actionButton->setSize(actionBtnPos);
		actionButton->setColor(makeColor(255, 255, 255, 0));
		actionButton->setHighlightColor(makeColor(255, 255, 255, 0));
		actionButton->setText(Language::get(7000));
		actionButton->setFont(itemFont);
		actionButton->setHideGlyphs(true);
		actionButton->setHideKeyboardGlyphs(true);
		actionButton->setHideSelectors(true);
		actionButton->setMenuConfirmControlType(0);
		actionButton->setBackground("*#images/ui/Shrines/Button_TakeAll_00.png");
		actionButton->setBackgroundHighlighted("*#images/ui/Shrines/Button_TakeAllHigh_00.png");
		actionButton->setBackgroundActivated("*#images/ui/Shrines/Button_TakeAllPress_00.png");
		actionButton->setTextHighlightColor(makeColor(201, 162, 100, 255));
		actionButton->setCallback([](Button& button) {
			int player = button.getOwner();
			auto& gui = GenericGUI[player].eternalShrineGUI;
			Entity* shrine = uidToEntity(GenericGUI[player].eternalShrineEntityUid);
			if ( !shrine || shrine->eternalShrineState != 0 )
			{
				return;
			}
			Player::soundActivate();
			if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
			{
				if ( eternalShrineOnSupplication(player, shrine->getUID()) )
				{
					gui.currentView = ASSIST_SHRINE_VIEW_WAITING;
				}
			}
			else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC )
			{
				if ( eternalShrineOnMusic(player, shrine->getUID()) )
				{
					gui.currentView = ASSIST_SHRINE_VIEW_WAITING;
				}
			}
			else if ( gui.sendItem1Uid != 0 )
			{
				if ( Item* item = uidToItem(gui.sendItem1Uid) )
				{
					if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
					{
						if ( eternalShrineOnSmithItem(player, shrine->getUID(), item, item->count) )
						{
							gui.currentView = ASSIST_SHRINE_VIEW_WAITING;
							if ( item->node )
							{
								list_RemoveNode(item->node);
							}
							else
							{
								free(item);
							}
						}
					}
					else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
					{
						if ( eternalShrineOnAscendItem(player, shrine->getUID(), item) )
						{
							gui.currentView = ASSIST_SHRINE_VIEW_WAITING;
						}
					}
				}
			}
			else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
			{
				if ( gui.ascensionType == SORCERY_SPELL
					|| gui.ascensionType == MYSTICISM_SPELL
					|| gui.ascensionType == THAUMATURGY_SPELL )
				{
					if ( Item* item = newItem(SPELL_ITEM, EXCELLENT, 0, 1, 0, true, nullptr) )
					{
						if ( gui.ascensionType == SORCERY_SPELL )
						{
							item->appearance = NUM_SPELLS;
						}
						else if ( gui.ascensionType == MYSTICISM_SPELL )
						{
							item->appearance = NUM_SPELLS + 1;
						}
						else if ( gui.ascensionType == THAUMATURGY_SPELL )
						{
							item->appearance = NUM_SPELLS + 2;
						}
						if ( eternalShrineOnAscendItem(player, shrine->getUID(), item) )
						{
							gui.currentView = ASSIST_SHRINE_VIEW_WAITING;
						}
						if ( item->node )
						{
							list_RemoveNode(item->node);
						}
						else
						{
							free(item);
						}
					}
				}
			}
			});
		actionButton->setTickCallback(genericgui_deselect_fn);
		actionButton->setOntop(true);
		actionButton->setDisabled(true);
		actionButton->setInvisible(true);

		auto offeringGlyph = bgFrame->addImage(SDL_Rect{ 0, 0, 0, 0 },
			0xFFFFFFFF, "", "offering glyph");
		offeringGlyph->disabled = true;
		offeringGlyph->ontop = true;

		{
			auto arrowButton = bgFrame->addButton("arrow left button");
			SDL_Rect btnPos{ 0, 0, 20, 32 };
			arrowButton->setSize(btnPos);
			arrowButton->setHideGlyphs(true);
			arrowButton->setHideKeyboardGlyphs(true);
			arrowButton->setHideSelectors(true);
			arrowButton->setMenuConfirmControlType(0);
			arrowButton->setBackground("*#images/ui/Shrines/altar_inspiration/ArrowLeft.png");
			arrowButton->setBackgroundHighlighted("*#images/ui/Shrines/altar_inspiration/ArrowLeftHigh.png");
			arrowButton->setBackgroundActivated("*#images/ui/Shrines/altar_inspiration/ArrowLeftPress.png");
			arrowButton->setCallback([](Button& button) {
				int player = button.getOwner();
				auto& gui = GenericGUI[player].eternalShrineGUI;
				if ( GenericGUI[player].getGuiType() == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
				{
					Player::soundModuleNavigation();
					int type = gui.ascensionType;
					--type;
					if ( type < EternalShrineAscensionType::ASCENSION_SPELL )
					{
						type = EternalShrineAscensionType::THAUMATURGY_SPELL;
					}
					if ( multiplayer == CLIENT )
					{
						Entity* shrine = uidToEntity(GenericGUI[player].eternalShrineEntityUid);
						if ( shrine )
						{
							shrine->eternalShrineViewingMode = type;

							clientUpdateShrineSkill(GenericGUI[player].eternalShrineEntityUid, 7, type, true);
						}
					}
					else
					{
						Entity* shrine = uidToEntity(GenericGUI[player].eternalShrineEntityUid);
						if ( shrine )
						{
							shrine->eternalShrineViewingMode = type;
							serverUpdateEntitySkill(shrine, 7);
						}
					}
					gui.ascensionType = (EternalShrineAscensionType)type;
				}
				});
			arrowButton->setTickCallback(genericgui_deselect_fn);
			arrowButton->setOntop(true);
			arrowButton->setDisabled(true);
			arrowButton->setInvisible(true);

			arrowButton = bgFrame->addButton("arrow right button");
			arrowButton->setSize(btnPos);
			arrowButton->setHideGlyphs(true);
			arrowButton->setHideKeyboardGlyphs(true);
			arrowButton->setHideSelectors(true);
			arrowButton->setMenuConfirmControlType(0);
			arrowButton->setBackground("*#images/ui/Shrines/altar_inspiration/ArrowRight.png");
			arrowButton->setBackgroundHighlighted("*#images/ui/Shrines/altar_inspiration/ArrowRightHigh.png");
			arrowButton->setBackgroundActivated("*#images/ui/Shrines/altar_inspiration/ArrowRightPress.png");
			arrowButton->setCallback([](Button& button) {
				int player = button.getOwner();
				auto& gui = GenericGUI[player].eternalShrineGUI;
				if ( GenericGUI[player].getGuiType() == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
				{
					Player::soundModuleNavigation();
					int type = gui.ascensionType;
					++type;
					if ( type > EternalShrineAscensionType::THAUMATURGY_SPELL )
					{
						type = 0;
					}
					if ( multiplayer == CLIENT )
					{
						Entity* shrine = uidToEntity(GenericGUI[player].eternalShrineEntityUid);
						if ( shrine )
						{
							shrine->eternalShrineViewingMode = type;

							clientUpdateShrineSkill(GenericGUI[player].eternalShrineEntityUid, 7, type, true);
						}
					}
					else
					{
						Entity* shrine = uidToEntity(GenericGUI[player].eternalShrineEntityUid);
						if ( shrine )
						{
							shrine->eternalShrineViewingMode = type;
							serverUpdateEntitySkill(shrine, 7);
						}
					}
					gui.ascensionType = (EternalShrineAscensionType)type;
				}
				});
			arrowButton->setTickCallback(genericgui_deselect_fn);
			arrowButton->setOntop(true);
			arrowButton->setDisabled(true);
			arrowButton->setInvisible(true);

			auto img = bgFrame->addImage(SDL_Rect{ 0, 0, 0, 0 }, 0xFFFFFFFF,
				"", "ascension img");
			img->ontop = true;
		}
	}
}

void GenericGUIMenu::EternalShrineGUI_t::selectEternalShrineSlot(const int x, const int y)
{
	selectedEternalShrineSlotX = x;
	selectedEternalShrineSlotY = y;
}

Frame* GenericGUIMenu::EternalShrineGUI_t::getEternalShrineSlotFrame(int x, int y) const
{
	if ( eternalShrineFrame )
	{
		int key = x + y * 100;
		if ( eternalShrineSlotFrames.find(key) != eternalShrineSlotFrames.end() )
		{
			return eternalShrineSlotFrames.at(key);
		}
	}
	return nullptr;
}

bool GenericGUIMenu::EternalShrineGUI_t::inventoryItemAllowedInGUI(Item* item)
{
	if ( !item ) { return false; }
	/*if ( item->status == BROKEN )
	{
		return false;
	}*/

	if ( item->type == READABLE_BOOK || itemCategory(item) == SCROLL || true )
	{
		return true;
	}

	if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
	{
		if ( itemCategory(item) == SPELL_CAT )
		{
			return true;
		}
	}
	return false;
}

GenericGUIMenu::EternalShrineGUI_t::EternalItemActions_t GenericGUIMenu::EternalShrineGUI_t::setItemDisplayNameAndPrice(Item* item, bool checkResultOnly)
{
	GenericGUIMenu::EternalShrineGUI_t::EternalItemActions_t resultAction = ETERNAL_ITEM_NONE;
	if ( !item || (item->type == SPELL_ITEM && parentGUI.guiType != GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION) )
	{
		if ( !checkResultOnly )
		{
			clearItemDisplayed();
		}
	}
	if ( !bSendItemAllowed )
	{
		if ( !checkResultOnly )
		{
			clearItemDisplayed();
		}
		return resultAction;
	}
	if ( !item )
	{
		return resultAction;
	}

	char buf[1024] = "";
	if ( !item->identified && !checkResultOnly )
	{
		/*if ( isTooltipForRecvItem )
		{
			snprintf(buf, sizeof(buf), "%s (?)", Language::get(4161));
		}
		else*/
		{
			snprintf(buf, sizeof(buf), "%s %s (?)", ItemTooltips.getItemStatusAdjective(item->type, item->status).c_str(), item->getName());
		}
	}
	else
	{
		snprintf(buf, sizeof(buf), "%s %s (%+d)", ItemTooltips.getItemStatusAdjective(item->type, item->status).c_str(), item->getName(), item->beatitude);
	}

	auto activateSelectionPrompt = eternalShrineFrame->findField("activate prompt");
	activateSelectionPrompt->setText("");

	int player = parentGUI.getPlayer();
	/*if ( isTooltipForRecvItem )
	{
		if ( item->type != POTION_EMPTY )
		{
			resultAction = MAIL_ACTION_OK;
		}
	}
	else */
	if ( item->type == SPELL_ITEM )
	{
		if ( currentView == ASSIST_SHRINE_VIEW_ACTION
			&& parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
		{
			if ( ascensionType == ASCENSION_SPELL )
			{
				resultAction = ETERNAL_ITEM_OK;
			}
			else
			{
				resultAction = ETERNAL_ITEM_NONE;
			}
		}
		else
		{
			resultAction = ETERNAL_ITEM_INVALID;
		}
	}
	else if ( itemCategory(item) == SCROLL || item->type == READABLE_BOOK || true )
	{
		bool isEquipped = itemIsEquipped(item, player);
		if ( (!item->identified || isEquipped) )
		{
			resultAction = ETERNAL_ITEM_UNIDENTIFIED;
		}
		else
		{
			resultAction = ETERNAL_ITEM_OK;
		}
		if ( isEquipped && !checkResultOnly )
		{
			strcat(buf, "\n");
			strcat(buf, Language::get(4165));
		}
	}
	else
	{
		resultAction = ETERNAL_ITEM_INVALID;
	}

	if ( !checkResultOnly )
	{
		if ( itemDesc != buf && !checkResultOnly )
		{
			itemRequiresTitleReflow = true;
		}
		itemDesc = buf;
		itemType = item->type;
		if ( resultAction == ETERNAL_ITEM_OK )
		{
			if ( item->uid == sendItem1Uid )
			{
				activateSelectionPrompt->setText(Language::get(4173));
			}
			else
			{
				activateSelectionPrompt->setText(Language::get(4172));
			}
			/*else if ( isTooltipForRecvItem )
			{
				bool usingGamepad = inputs.hasController(player) && !inputs.getVirtualMouse(player)->draw_cursor;
				if ( !usingGamepad )
				{
					activateSelectionPrompt->setText(Language::get(6988));
				}
			}*/
		}

		itemActionType = resultAction;
	}

	return resultAction;
}

bool GenericGUIMenu::EternalShrineGUI_t::warpMouseToSelectedEternalShrineItem(Item* snapToItem, Uint32 flags)
{
	if ( eternalShrineGUIHasBeenCreated() )
	{
		int x = getSelectedEternalShrineX();
		int y = getSelectedEternalShrineY();
		if ( snapToItem )
		{
			x = snapToItem->x;
			y = snapToItem->y;
		}

		if ( auto slot = getEternalShrineSlotFrame(x, y) )
		{
			int playernum = parentGUI.getPlayer();
			auto player = players[playernum];
			if ( !isInteractable )
			{
				//messagePlayer(0, "[Debug]: select item queued");
				player->inventoryUI.cursor.queuedModule = Player::GUI_t::MODULE_ETERNALSHRINE;
				player->inventoryUI.cursor.queuedFrameToWarpTo = slot;
				return false;
			}
			else
			{
				//messagePlayer(0, "[Debug]: select item warped");
				player->inventoryUI.cursor.queuedModule = Player::GUI_t::MODULE_NONE;
				player->inventoryUI.cursor.queuedFrameToWarpTo = nullptr;
				slot->warpMouseToFrame(playernum, flags);
			}
			return true;
		}
	}
	return false;
}

void GenericGUIMenu::EternalShrineGUI_t::clearItemDisplayed()
{
	itemType = -1;
	itemActionType = ETERNAL_ITEM_NONE;
}

bool GenericGUIMenu::isItemEternalShrineUsable(const Item* item)
{
	if ( !item )
	{
		return false;
	}

	if ( !eternalShrineGUI.inventoryItemAllowedInGUI(const_cast<Item*>(item)) )
	{
		return false;
	}

	if ( itemIsEquipped(item, gui_player) )
	{
		return false; // don't want to deal with client/server desync problems here.
	}

	return true;
}