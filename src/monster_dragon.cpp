/*-------------------------------------------------------------------------------

	BARONY
	File: monster_skeleton.cpp
	Desc: implements all of the skeleton monster's code

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "entity.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "engine/audio/sound.hpp"
#include "net.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "magic/magic.hpp"
#include "prng.hpp"
#include "scores.hpp"
#include "mod_tools.hpp"
#include "paths.hpp"

void initDragon(Entity* my, Stat* myStats)
{
	node_t* node;

	my->z = limbs[DRAGON][0][2];
	my->flags[BURNABLE] = false;
	my->initMonster(2509);
	my->flags[INVISIBLE] = true; // hide the "AI" bodypart
	if ( multiplayer != CLIENT )
	{
		MONSTER_SPOTSND = -1;
		MONSTER_SPOTVAR = 1;
		MONSTER_IDLESND = -1;
		MONSTER_IDLEVAR = 1;
	}

	if ( multiplayer != CLIENT && !MONSTER_INIT )
	{
		auto& rng = my->entity_rng ? *my->entity_rng : local_rng;

		if ( myStats != nullptr )
		{
			if ( !myStats->leader_uid )
			{
				myStats->leader_uid = 0;
			}

			// apply random stat increases if set in stat_shared.cpp or editor
			setRandomMonsterStats(myStats, rng);

			// generate 6 items max, less if there are any forced items from boss variants
			int customItemsToGenerate = ITEM_CUSTOM_SLOT_LIMIT;

			// generates equipment and weapons if available from editor
			createMonsterEquipment(myStats, rng);

			// create any custom inventory items from editor if available
			createCustomInventory(myStats, customItemsToGenerate, rng);

			// count if any custom inventory items from editor
			int customItems = countCustomItems(myStats); //max limit of 6 custom items per entity.

			// count any inventory items set to default in edtior
			int defaultItems = countDefaultItems(myStats);

			my->setHardcoreStats(*myStats);

			myStats->setEffectActive(EFF_LEVITATING, 1);
			myStats->EFFECTS_TIMERS[EFF_LEVITATING] = 0;
		}
	}

	std::vector<int> limbs_list{
		2513, // torso
		2512, // neck
		2510, // head
		2511, // jaw
		2514, // left arm
		2515, // left claw
		2516, // right arm
		2517, // right claw
		2518, // left leg
		2519, // left foot
		2520, // right leg
		2521, // right foot
		2522, // tail
		2523, // wing left
		2525  // wing right
	};
	
	int index = -1;
	for ( auto limb : limbs_list )
	{
		++index;
		Entity* entity = newEntity(limb, 1, map.entities, nullptr); //Limb entity.
		entity->sizex = 2;
		entity->sizey = 2;
		entity->skill[2] = my->getUID();
		entity->flags[PASSABLE] = true;
		entity->flags[NOUPDATE] = true;
		entity->yaw = my->yaw;
		entity->flags[USERFLAG2] = my->flags[USERFLAG2];
		entity->focalx = limbs[DRAGON][1 + index * 2][0];
		entity->focaly = limbs[DRAGON][1 + index * 2][1];
		entity->focalz = limbs[DRAGON][1 + index * 2][2];
		entity->behavior = &actDragonLimb;
		entity->parent = my->getUID();
		node = list_AddNodeLast(&my->children);
		node->element = entity;
		node->deconstructor = &emptyDeconstructor;
		node->size = sizeof(Entity*);
		my->bodyparts.push_back(entity);
	}
}

void actDragonLimb(Entity* my)
{
	my->actMonsterLimb(false);
}

void dragonDie(Entity* my)
{
	int index = -1;
	for ( auto bodypart : my->bodyparts )
	{
		++index;
		Entity* entity = spawnGib(my, bodypart->sprite);
		entity->z = bodypart->z;
		entity->skill[5] = 1; // poof
		serverSpawnGibForClient(entity);
	}

	my->removeMonsterDeathNodes();
	list_RemoveNode(my->mynode);
	return;
}

#define DRAGON_BODY 2
#define DRAGON_NECK 3
#define DRAGON_HEAD 4
#define DRAGON_JAW 5
#define DRAGON_LEFTARM 6
#define DRAGON_LEFTCLAW 7
#define DRAGON_RIGHTARM 8
#define DRAGON_RIGHTCLAW 9
#define DRAGON_LEFTLEG 10
#define DRAGON_LEFTFOOT 11
#define DRAGON_RIGHTLEG 12
#define DRAGON_RIGHTFOOT 13
#define DRAGON_TAIL 14
#define DRAGON_WINGLEFT 15
#define DRAGON_WINGRIGHT 16
#define DRAGON_FLOATX body->fskill[0]
#define DRAGON_FLOATY body->fskill[1]
#define DRAGON_FLOATZ body->fskill[2]
#define DRAGON_WALK_ANIM body->fskill[6]
#define DRAGON_WALK_MULT body->fskill[7]
#define DRAGON_IDLE_ANIM body->fskill[8]
#define DRAGON_IDLE_MULT body->fskill[9]
#define DRAGON_IDLE_WAIT_ANIM body->fskill[10]
#define DRAGON_IDLE_WAIT_MULT body->fskill[11]
#define DRAGON_RUN_ANIM body->fskill[12]
#define DRAGON_RUN_MULT body->fskill[13]
#define DRAGON_STAND_ANIM body->fskill[14]
#define DRAGON_STAND_MULT body->fskill[15]
#define DRAGON_STAND_ATK_MULT body->fskill[16]
#define DRAGON_STAND_ATK_ANIM body->fskill[17]
#define DRAGON_ATK_MULT body->fskill[18]
#define DRAGON_ATK_ANIM body->fskill[19]
#define DRAGON_FLY_ANIM body->fskill[20]
#define DRAGON_FLY_MULT body->fskill[21]
#define DRAGON_STATE body->skill[0]
#define DRAGON_FLY_ATK_ANIM neck->fskill[7]
#define DRAGON_FLY_ATK_MULT neck->fskill[8]
#define DRAGON_DESCEND_ANIM neck->fskill[9]
#define DRAGON_DESCEND_MULT neck->fskill[10]
#define DRAGON_FLY_Z neck->fskill[11]
#define DRAGON_GLIDE_MULT neck->fskill[12]
#define DRAGON_WALK_DIST neck->fskill[13]

#define DRAGON_LIMB_POOF entity->skill[1]
#define DRAGON_LIMB_PITCH entity->fskill[3]
#define DRAGON_WING_ROLL entity->fskill[4]
#define DRAGON_WING_YAW entity->fskill[5]
#define DRAGON_ARM_Z leftarm->fskill[4]
#define DRAGON_ARM_FORWARD leftarm->fskill[5]
#define DRAGON_ARM_SIDE leftarm->fskill[6]
#define DRAGON_LEG_FORWARD leftleg->fskill[5]
#define DRAGON_LEG_SIDE leftleg->fskill[6]
#define DRAGON_NECK_Z neck->fskill[4]
#define DRAGON_NECK_FORWARD neck->fskill[5]
#define DRAGON_LEG_Z leftleg->fskill[4]

void createWindAOE(Entity* my, Entity* limb)
{
	if ( !my ) { return; }

	for ( int i = 0; i < 8; ++i )
	{
		if ( Entity* fx = createParticleAOEIndicator(my, limb->z, limb->z, limb->z, TICKS_PER_SECOND * 5, 16 + (i / 2) * 2) )
		{
			fx->yaw = my->yaw + PI / 2 - (i / 2) * PI / 2;
			fx->pitch += PI / 32;
			if ( i % 2 == 1 )
			{
				fx->pitch += PI;
			}
			fx->z = limb->z;
			fx->z -= (i / 2) * 0.5;
			fx->vel_z -= 0.25;
			fx->fskill[0] = 0.3; // rotate
			fx->scalex = 1.0;
			fx->scaley = 1.0;
			fx->flags[ENTITY_SKIP_CULLING] = false;
			if ( auto indicator = AOEIndicators_t::getIndicator(fx->skill[10]) )
			{
				indicator->expireAlphaRate = 0.9;
				indicator->cacheType = AOEIndicators_t::CACHE_VORTEX_AESTHETIC;
				indicator->arc = PI / 4;
				indicator->indicatorColor = 0xFFFFFFFF;
				indicator->loop = false;
				indicator->framesPerTick = 1;
				indicator->ticksPerUpdate = 1;
				indicator->delayTicks = 0;
			}
		}
	}
}

void dragonAnimate(Entity* my, Stat* myStats, double dist)
{
	node_t* node;
	Entity* entity = nullptr;
	int bodypart;

	my->flags[INVISIBLE] = true; // hide the "AI" bodypart

	my->sizex = 4;
	my->sizey = 4;

	//myStats->setEffectActive(EFF_STUNNED, true);

	if ( keystatus[SDLK_g] && enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
	{
		keystatus[SDLK_g] = 0;
		my->monsterLookDir = PI / 4;
		my->monsterRotate();
		if ( !myStats->getEffectActive(EFF_PARALYZED) )
		{
			myStats->setEffectActive(EFF_PARALYZED, true);
		}
		else
		{
			myStats->clearEffect(EFF_PARALYZED);
		}
	}
	if ( keystatus[SDLK_h] && enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
	{
		keystatus[SDLK_h] = 0;
		//my->monsterLookDir += 0.05;
		//my->monsterRotate();
		CastSpellProps_t props;
		props.caster_x = my->x + 0.0 * cos(my->yaw);
		props.caster_y = my->y + 0.0 * sin(my->yaw);
		props.target_x = my->x + 96.0 * cos(my->yaw);
		props.target_y = my->y + 96.0 * sin(my->yaw);
		castSpell(my->getUID(), getSpellFromID(SPELL_METEOR), true, false, false, &props);
	}

	if ( multiplayer != CLIENT )
	{
		my->z = limbs[DRAGON][0][2];
		my->creatureHandleLiftZ();

		myStats->setEffectActive(EFF_LEVITATING, 1);
		myStats->EFFECTS_TIMERS[EFF_LEVITATING] = 0;
	}

	enum SpecialState
	{
		SPECIAL_IDLE,
		SPECIAL_STAND,
		SPECIAL_RUN,
		SPECIAL_FLY,
		SPECIAL_ENUM_END
	};
	const std::vector<std::pair<int, int>> coords = {
		{9, 9},
		{28, 9},
		{28, 27},
		{9, 27},
		{18, 16}
	};

	static ConsoleVariable<int> cvar_dragon_pose("/dragon_pose", 0);
	static ConsoleVariable<int> cvar_dragon_auto("/dragon_auto", 0);
	if ( my->monsterSpecialTimer == 0 )
	{
		my->monsterSpecialTimer = 6 * TICKS_PER_SECOND;
		
		if ( my->monsterState == MONSTER_STATE_WAIT )
		{
			auto& pair = coords[local_rng.rand() % coords.size()];
			if ( my->monsterSetPathToLocation(pair.first, pair.second, 2, GeneratePathTypes::GENERATE_PATH_BOSS_TRACKING_IDLE, true, false) )
			{
				my->monsterSpecialState = local_rng.rand() % SPECIAL_ENUM_END;
				my->monsterState = MONSTER_STATE_HUNT;
			}
		}
	}
	else
	{
		if ( my->monsterState == MONSTER_STATE_WAIT )
		{
			--my->monsterSpecialTimer;
		}
	}

	Entity* body = nullptr;
	Entity* leftarm = nullptr;
	Entity* rightarm = nullptr;
	Entity* leftleg = nullptr;
	Entity* rightleg = nullptr;
	Entity* neck = nullptr;
	Entity* head = nullptr;
	Entity* wingleft = nullptr;

	static ConsoleVariable<int> cvar_dragon_wing("/dragon_wing", 0);

	for ( bodypart = 0, node = my->children.first; node != nullptr; node = node->next, ++bodypart )
	{
		if ( bodypart < 2 )
		{
			continue;
		}
		if ( bodypart == DRAGON_BODY )
		{
			body = (Entity*)node->element;
		}
		else if ( bodypart == DRAGON_NECK )
		{
			neck = (Entity*)node->element;
		}
		else if ( bodypart == DRAGON_LEFTLEG )
		{
			leftleg = (Entity*)node->element;
		}
		else if ( bodypart == DRAGON_RIGHTLEG )
		{
			rightleg = (Entity*)node->element;
		}
	}

	if ( dist > 0.01 )
	{
		DRAGON_WALK_DIST = std::min(1.0, DRAGON_WALK_DIST + 0.05);
	}
	else
	{
		DRAGON_WALK_DIST = std::max(0.0, DRAGON_WALK_DIST - 0.05);
	}

	if ( *cvar_dragon_auto == 1 )
	{
		if ( my->monsterSpecialState == SPECIAL_RUN )
		{
			myStats->DEX = 25;
		}
		else
		{
			myStats->DEX = 15;
		}

		if ( DRAGON_WALK_DIST > 0.25 )
		{
			if ( my->monsterSpecialState != SPECIAL_FLY
				&& ((*cvar_dragon_pose == 8
					|| *cvar_dragon_pose == 9
					|| *cvar_dragon_pose == 10) || DRAGON_FLY_ANIM > 0.05) )
			{
				if ( (*cvar_dragon_pose == 8
					|| *cvar_dragon_pose == 9
					|| *cvar_dragon_pose == 10) )
				{
					my->setEffect(EFF_STUNNED, true, 50, false);
				}
				*cvar_dragon_pose = 5;
			}
			else if ( my->monsterSpecialState == SPECIAL_IDLE )
			{
				*cvar_dragon_pose = 1;
			}
			else if ( my->monsterSpecialState == SPECIAL_STAND )
			{
				*cvar_dragon_pose = 1;
			}
			else if ( my->monsterSpecialState == SPECIAL_RUN )
			{
				*cvar_dragon_pose = 4;
			}
			else if ( my->monsterSpecialState == SPECIAL_FLY )
			{
				if ( DRAGON_FLY_ANIM < 0.995 )
				{
					*cvar_dragon_pose = 8;
					my->setEffect(EFF_STUNNED, true, 50, false);
				}
				else
				{
					*cvar_dragon_pose = 10;
				}
			}
		}
		else
		{
			if ( my->monsterSpecialState != SPECIAL_FLY 
				&& ((*cvar_dragon_pose == 8
					|| *cvar_dragon_pose == 9
					|| *cvar_dragon_pose == 10) || DRAGON_FLY_ANIM > 0.05) )
			{
				my->setEffect(EFF_STUNNED, true, 50, false);
				*cvar_dragon_pose = 5;
			}
			else if ( my->monsterSpecialState == SPECIAL_IDLE )
			{
				*cvar_dragon_pose = 3;
			}
			else if ( my->monsterSpecialState == SPECIAL_RUN )
			{
				*cvar_dragon_pose = 2;
			}
			else if ( my->monsterSpecialState == SPECIAL_STAND )
			{
				*cvar_dragon_pose = 5;
			}
			else if ( my->monsterSpecialState == SPECIAL_FLY )
			{
				*cvar_dragon_pose = 8;
				if ( DRAGON_FLY_ANIM < 0.995 )
				{
					my->setEffect(EFF_STUNNED, true, 50, false);
				}
			}
		}
	}

	const real_t animPoseFadein = 0.05;
	const real_t animPoseFadeout = 0.05;

	for ( bodypart = 0, node = my->children.first; node != nullptr; node = node->next, ++bodypart )
	{
		if ( bodypart < 2 )
		{
			continue;
		}

		entity = (Entity*)node->element;

		entity->roll = 0.0;
		entity->pitch = 0.0;

		if ( bodypart == DRAGON_BODY )
		{
			body = entity;
			if ( enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
			{
				if ( keystatus[SDLK_KP_4] )
				{
					DRAGON_LIMB_PITCH -= 0.05;
				}
				if ( keystatus[SDLK_KP_6] )
				{
					DRAGON_LIMB_PITCH += 0.05;
				}
				if ( keystatus[SDLK_KP_5] )
				{
					DRAGON_LIMB_PITCH = 0;
					DRAGON_IDLE_MULT = 0.0;
					DRAGON_WALK_MULT = 0.0;
				}
			}

			//if ( DRAGON_STATE == 2 )
			//{
			//	//DRAGON_WALK_ANIM += 0.225;
			//	DRAGON_WALK_ANIM += 0.01;
			//}
			//else
			//{
				//DRAGON_WALK_ANIM += 0.075;
			//}
			DRAGON_IDLE_ANIM += 0.1;
			DRAGON_STAND_ANIM += 0.1;
			DRAGON_IDLE_WAIT_ANIM += 0.07;
			DRAGON_IDLE_WAIT_ANIM = fmod(DRAGON_IDLE_WAIT_ANIM, 6 * PI);
			{
				DRAGON_WALK_ANIM += 0.1;
				real_t walkAng = fmod(DRAGON_WALK_ANIM, 2 * PI);
				if ( walkAng < PI / 2 || (walkAng >= PI && walkAng < 1.5 * PI) )
				{
				}
				else
				{
					DRAGON_WALK_ANIM += 0.1;
				}
			}
			{
				DRAGON_RUN_ANIM += 0.06;
				real_t walkAng = fmod(DRAGON_RUN_ANIM, 2 * PI);
				if ( walkAng < PI / 2 )
				{
					DRAGON_RUN_ANIM += 0.1;
				}
				else if ( walkAng < 1.5 * PI )
				{
					DRAGON_RUN_ANIM += 0.1;
				}
			}

			if ( enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
			{
				if ( keystatus[SDLK_KP_MINUS] )
				{
					*cvar_dragon_pose = 5;
				}
				if ( keystatus[SDLK_KP_PLUS] )
				{
					*cvar_dragon_pose = 8;
				}
			}

			if ( *cvar_dragon_pose == 1 )
			{
				DRAGON_WALK_MULT = std::min(1.0, DRAGON_WALK_MULT + animPoseFadein);

				DRAGON_FLY_MULT = std::max(0.0, DRAGON_FLY_MULT - animPoseFadeout);
				DRAGON_STAND_MULT = std::max(0.0, DRAGON_STAND_MULT - animPoseFadeout);
				DRAGON_STAND_ATK_MULT = std::max(0.0, DRAGON_STAND_ATK_MULT - animPoseFadeout);
				DRAGON_ATK_MULT = std::max(0.0, DRAGON_ATK_MULT - animPoseFadeout);
				DRAGON_IDLE_MULT = std::max(0.0, DRAGON_IDLE_MULT - animPoseFadeout);
				DRAGON_IDLE_WAIT_MULT = std::max(0.0, DRAGON_IDLE_WAIT_MULT - animPoseFadeout);
				DRAGON_RUN_MULT = std::max(0.0, DRAGON_RUN_MULT - animPoseFadeout);
				DRAGON_FLY_ATK_MULT = std::max(0.0, DRAGON_FLY_ATK_MULT - animPoseFadeout);
			}
			else if ( *cvar_dragon_pose == 2 || *cvar_dragon_pose == 7 )
			{
				if ( *cvar_dragon_pose == 7 )
				{
					DRAGON_ATK_MULT = std::min(1.0, DRAGON_ATK_MULT + 3 * animPoseFadein);
					DRAGON_ATK_ANIM += 0.125;
					if ( fmod(DRAGON_ATK_ANIM, 4 * PI) >= 2 * PI )
					{
						DRAGON_ATK_ANIM += 0.05;
					}
				}
				else
				{
					DRAGON_ATK_MULT = std::max(0.0, DRAGON_ATK_MULT - animPoseFadeout);
					DRAGON_ATK_ANIM = 0.0;
				}

				DRAGON_IDLE_MULT = std::min(1.0, DRAGON_IDLE_MULT + animPoseFadein);

				DRAGON_STAND_MULT = std::max(0.0, DRAGON_STAND_MULT - animPoseFadeout);
				DRAGON_WALK_MULT = std::max(0.0, DRAGON_WALK_MULT - 2 * animPoseFadeout);
				DRAGON_IDLE_WAIT_MULT = std::max(0.0, DRAGON_IDLE_WAIT_MULT - animPoseFadeout);
				DRAGON_RUN_MULT = std::max(0.0, DRAGON_RUN_MULT - animPoseFadeout);
				DRAGON_STAND_ATK_MULT = std::max(0.0, DRAGON_STAND_ATK_MULT - animPoseFadeout);
				DRAGON_FLY_MULT = std::max(0.0, DRAGON_FLY_MULT - animPoseFadeout);
				DRAGON_FLY_ATK_MULT = std::max(0.0, DRAGON_FLY_ATK_MULT - animPoseFadeout);
			}
			else if ( *cvar_dragon_pose == 3 )
			{
				DRAGON_IDLE_WAIT_MULT = std::min(1.0, DRAGON_IDLE_WAIT_MULT + animPoseFadein);

				DRAGON_STAND_MULT = std::max(0.0, DRAGON_STAND_MULT - animPoseFadeout);
				DRAGON_IDLE_MULT = std::max(0.0, DRAGON_IDLE_MULT - animPoseFadeout);
				DRAGON_WALK_MULT = std::max(0.0, DRAGON_WALK_MULT - 2 * animPoseFadeout);
				DRAGON_RUN_MULT = std::max(0.0, DRAGON_RUN_MULT - animPoseFadeout);
				DRAGON_STAND_ATK_MULT = std::max(0.0, DRAGON_STAND_ATK_MULT - animPoseFadeout);
				DRAGON_ATK_MULT = std::max(0.0, DRAGON_ATK_MULT - animPoseFadeout);
				DRAGON_FLY_MULT = std::max(0.0, DRAGON_FLY_MULT - animPoseFadeout);
				DRAGON_FLY_ATK_MULT = std::max(0.0, DRAGON_FLY_ATK_MULT - animPoseFadeout);
			}
			else if ( *cvar_dragon_pose == 4 )
			{
				DRAGON_RUN_MULT = std::min(1.0, DRAGON_RUN_MULT + animPoseFadein);

				DRAGON_STAND_MULT = std::max(0.0, DRAGON_STAND_MULT - animPoseFadeout);
				DRAGON_WALK_MULT = std::max(0.0, DRAGON_WALK_MULT - 2 * animPoseFadeout);
				DRAGON_IDLE_MULT = std::max(0.0, DRAGON_IDLE_MULT - animPoseFadeout);
				DRAGON_IDLE_WAIT_MULT = std::max(0.0, DRAGON_IDLE_WAIT_MULT - animPoseFadeout);
				DRAGON_STAND_ATK_MULT = std::max(0.0, DRAGON_STAND_ATK_MULT - animPoseFadeout);
				DRAGON_ATK_MULT = std::max(0.0, DRAGON_ATK_MULT - animPoseFadeout);
				DRAGON_FLY_MULT = std::max(0.0, DRAGON_FLY_MULT - animPoseFadeout);
				DRAGON_FLY_ATK_MULT = std::max(0.0, DRAGON_FLY_ATK_MULT - animPoseFadeout);
			}
			else if ( *cvar_dragon_pose == 5 || *cvar_dragon_pose == 6 
				|| *cvar_dragon_pose == 8
				|| *cvar_dragon_pose == 9
				|| *cvar_dragon_pose == 10 )
			{
				if ( *cvar_dragon_pose == 6 )
				{
					DRAGON_STAND_ATK_MULT = std::min(1.0, DRAGON_STAND_ATK_MULT + animPoseFadein);

					DRAGON_STAND_ATK_ANIM += 0.1;
				}
				else
				{
					DRAGON_STAND_ATK_MULT = std::max(0.0, DRAGON_STAND_ATK_MULT - animPoseFadeout);
					DRAGON_STAND_ATK_ANIM = 0.0;
				}

				if ( *cvar_dragon_pose == 8 || *cvar_dragon_pose == 9 || *cvar_dragon_pose == 10 )
				{
					DRAGON_FLY_MULT = std::min(1.0, DRAGON_FLY_MULT + animPoseFadein);

					DRAGON_FLY_ANIM += 0.1 * (1.0 - (DRAGON_FLY_ANIM >= PI ? 0.9 * DRAGON_GLIDE_MULT : 0.0));
					if ( fmod(DRAGON_FLY_ANIM, 2 * PI) >= PI )
					{
						DRAGON_FLY_ANIM += 0.1 * (1.0 - (DRAGON_FLY_ANIM >= PI ? 0.9 * DRAGON_GLIDE_MULT : 0.0));
					}

					if ( DRAGON_FLY_ATK_ANIM >= 2.0 * PI )
					{
						*cvar_dragon_pose = 8;
					}
					else if ( fmod(DRAGON_FLY_ATK_ANIM, 2 * PI) >= PI )
					{
						DRAGON_FLY_ATK_ANIM += 0.05;
					}
					else
					{
						DRAGON_FLY_ATK_ANIM += 0.15;
					}
					if ( *cvar_dragon_pose == 9 )
					{
						DRAGON_FLY_ATK_MULT = std::min(1.0, DRAGON_FLY_ATK_MULT + animPoseFadein);
					}
					else
					{
						DRAGON_FLY_ATK_ANIM = 0.0;
						DRAGON_FLY_ATK_MULT = std::max(0.0, DRAGON_FLY_ATK_MULT - animPoseFadeout);
					}
				}
				else
				{
					DRAGON_FLY_MULT = std::max(0.0, DRAGON_FLY_MULT - animPoseFadeout);
					DRAGON_FLY_ATK_ANIM = 0.0;
					DRAGON_FLY_ATK_MULT = std::max(0.0, DRAGON_FLY_ATK_MULT - animPoseFadeout);
				}

				DRAGON_STAND_MULT = std::min(1.0, DRAGON_STAND_MULT + animPoseFadein);

				DRAGON_RUN_MULT = std::max(0.0, DRAGON_RUN_MULT - animPoseFadeout);
				DRAGON_WALK_MULT = std::max(0.0, DRAGON_WALK_MULT - 2 * animPoseFadeout);
				DRAGON_IDLE_MULT = std::max(0.0, DRAGON_IDLE_MULT - animPoseFadeout);
				DRAGON_IDLE_WAIT_MULT = std::max(0.0, DRAGON_IDLE_WAIT_MULT - animPoseFadeout);
				DRAGON_ATK_MULT = std::max(0.0, DRAGON_ATK_MULT - animPoseFadeout);
			}
			else
			{
				DRAGON_STAND_MULT = std::max(0.0, DRAGON_STAND_MULT - animPoseFadeout);
				DRAGON_RUN_MULT = std::max(0.0, DRAGON_RUN_MULT - animPoseFadeout);
				DRAGON_WALK_MULT = std::max(0.0, DRAGON_WALK_MULT - 2 * animPoseFadeout);
				DRAGON_IDLE_MULT = std::max(0.0, DRAGON_IDLE_MULT - animPoseFadeout);
				DRAGON_IDLE_WAIT_MULT = std::max(0.0, DRAGON_IDLE_WAIT_MULT - animPoseFadeout);
				DRAGON_STAND_ATK_MULT = std::max(0.0, DRAGON_STAND_ATK_MULT - animPoseFadeout);
				DRAGON_ATK_MULT = std::max(0.0, DRAGON_ATK_MULT - animPoseFadeout);
				DRAGON_FLY_MULT = std::max(0.0, DRAGON_FLY_MULT - animPoseFadeout);
				DRAGON_FLY_ATK_MULT = std::max(0.0, DRAGON_FLY_ATK_MULT - animPoseFadeout);
			}

			if ( *cvar_dragon_pose == 10 )
			{
				DRAGON_GLIDE_MULT = std::min(1.0, DRAGON_GLIDE_MULT + std::max(0.01, (1.0 - DRAGON_GLIDE_MULT) / 20.0));
			}
			else
			{
				DRAGON_GLIDE_MULT = std::max(0.0, DRAGON_GLIDE_MULT - std::max(0.01, (DRAGON_GLIDE_MULT) / 20.0));
			}

			if ( *cvar_dragon_pose == 8
				|| *cvar_dragon_pose == 9
				|| *cvar_dragon_pose == 10 )
			{
				DRAGON_DESCEND_ANIM = 1.0 * PI;
				DRAGON_DESCEND_MULT = std::min(1.0, DRAGON_DESCEND_MULT + animPoseFadein);
			}
			else
			{
				if ( *cvar_dragon_pose == 5 || *cvar_dragon_pose == 6 )
				{
					if ( DRAGON_FLY_MULT <= 0.001 )
					{
						DRAGON_FLY_ANIM = 0.0;
					}
					else
					{
						DRAGON_FLY_ANIM += 0.1;
					}
				}
				else
				{
					DRAGON_FLY_ANIM = 0.0;
				}
				DRAGON_DESCEND_ANIM = std::min(2.0 * PI, DRAGON_DESCEND_ANIM + animPoseFadein);
				if ( DRAGON_DESCEND_MULT >= 0.25 )
				{
					DRAGON_DESCEND_MULT = std::max(0.0, DRAGON_DESCEND_MULT - animPoseFadeout);
				}
				else
				{
					DRAGON_DESCEND_MULT = std::max(0.0, DRAGON_DESCEND_MULT - 0.0125);
				}
			}

			entity->pitch += DRAGON_LIMB_PITCH;
		}
		else if ( bodypart == DRAGON_NECK )
		{
			neck = entity;
			if ( enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
			{
				if ( keystatus[SDLK_KP_7] )
				{
					DRAGON_LIMB_PITCH -= 0.05;
				}
				if ( keystatus[SDLK_KP_9] )
				{
					DRAGON_LIMB_PITCH += 0.05;
				}
				if ( keystatus[SDLK_KP_8] )
				{
					DRAGON_LIMB_PITCH = 0;
				}
			}
			entity->pitch += DRAGON_LIMB_PITCH;
		}
		else if ( bodypart == DRAGON_HEAD )
		{
			head = entity;
		}
		else if ( bodypart == DRAGON_LEFTARM )
		{
			leftarm = entity;
		}
		else if ( bodypart == DRAGON_RIGHTARM )
		{
			rightarm = entity;
		}
		else if ( bodypart == DRAGON_LEFTLEG )
		{
			leftleg = entity;
		}
		else if ( bodypart == DRAGON_RIGHTLEG )
		{
			rightleg = entity;
		}
		else if ( bodypart == DRAGON_WINGLEFT )
		{
			wingleft = entity;
			if ( enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
			{
				if ( keystatus[SDLK_KP_1] )
				{
					DRAGON_LIMB_PITCH -= 0.05;
				}
				if ( keystatus[SDLK_KP_3] )
				{
					DRAGON_LIMB_PITCH += 0.05;
				}
				if ( keystatus[SDLK_KP_2] )
				{
					DRAGON_LIMB_PITCH = 0;
				}
				if ( keystatus[SDLK_1] )
				{
					DRAGON_WING_ROLL -= 0.05;
				}
				if ( keystatus[SDLK_3] )
				{
					DRAGON_WING_ROLL += 0.05;
				}
				if ( keystatus[SDLK_2] )
				{
					DRAGON_WING_ROLL = 0;
				}
				if ( keystatus[SDLK_4] )
				{
					DRAGON_WING_YAW -= 0.05;
				}
				if ( keystatus[SDLK_5] )
				{
					DRAGON_WING_YAW += 0.05;
				}
				if ( keystatus[SDLK_6] )
				{
					DRAGON_WING_YAW = 0;
				}
			}
		}

		while ( DRAGON_LIMB_PITCH > PI )
		{
			DRAGON_LIMB_PITCH -= 2 * PI;
		}
		while ( DRAGON_LIMB_PITCH <= -PI )
		{
			DRAGON_LIMB_PITCH += 2 * PI;
		}

		entity->x = my->x;
		entity->y = my->y;
		entity->z = my->z;
		entity->yaw = my->yaw;

		const real_t walkAngle = PI / 4;
		const real_t walkFootAngle = PI / 2;
		const real_t walkFootAngleFront = PI / 4;
		const real_t walkMult = std::min(1.0, DRAGON_WALK_MULT);
		const real_t runMult = std::min(1.0, DRAGON_RUN_MULT);
		const real_t idleMult = std::min(1.0, DRAGON_IDLE_MULT);
		const real_t idleWaitMult = std::min(1.0, DRAGON_IDLE_WAIT_MULT);
		const real_t standMult = std::min(1.0, DRAGON_STAND_MULT);
		const real_t standAtkMult = std::min(1.0, DRAGON_STAND_ATK_MULT);
		const real_t flyMult = std::min(1.0, DRAGON_FLY_MULT);
		const real_t flyAtkMult = std::min(1.0, DRAGON_FLY_ATK_MULT);
		const real_t glideMult = std::min(1.0, DRAGON_GLIDE_MULT);
		const real_t descendMult = std::min(1.0, DRAGON_DESCEND_MULT);
		const real_t flyAtkAnim = fmod(DRAGON_FLY_ATK_ANIM, 4 * PI);
		const real_t standAtkAnim = fmod(DRAGON_STAND_ATK_ANIM, 4 * PI);
		const real_t atkAnim = fmod(DRAGON_ATK_ANIM, 4 * PI);
		const real_t idleAtkMult = std::min(1.0, DRAGON_ATK_MULT);
		const real_t idleWaitZ = 1.0;
		real_t idleWaitTurn = 0.0;
		if ( DRAGON_IDLE_WAIT_ANIM >= 0.5 * PI && DRAGON_IDLE_WAIT_ANIM < 2.5 * PI )
		{
			idleWaitTurn = std::min(0.5 * PI, (DRAGON_IDLE_WAIT_ANIM) - 0.5 * PI);
		}
		else if ( DRAGON_IDLE_WAIT_ANIM >= 2.5 * PI && DRAGON_IDLE_WAIT_ANIM <= 3.5 * PI )
		{
			idleWaitTurn = ((3.5 * PI - DRAGON_IDLE_WAIT_ANIM)) / 2.0;
		}
		else if ( DRAGON_IDLE_WAIT_ANIM >= 3.5 * PI && DRAGON_IDLE_WAIT_ANIM < 5.0 * PI )
		{
			idleWaitTurn = -std::min(0.5 * PI, (DRAGON_IDLE_WAIT_ANIM) - 3.5 * PI);
		}
		else if ( DRAGON_IDLE_WAIT_ANIM >= 5.0 * PI && DRAGON_IDLE_WAIT_ANIM <= 6.0 * PI )
		{
			idleWaitTurn = std::min(0.0, -(6.0 * PI - DRAGON_IDLE_WAIT_ANIM) * 0.5);
		}

		entity->yaw += glideMult * (PI / 128) * sin(DRAGON_FLY_ANIM * 4);

		switch ( bodypart )
		{
			case DRAGON_BODY:
			{
				entity->x += limbs[DRAGON][2][0] * cos(my->yaw) + limbs[DRAGON][2][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][2][0] * sin(my->yaw) + limbs[DRAGON][2][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][2][2];
				entity->focalx = limbs[DRAGON][1][0];
				entity->focaly = limbs[DRAGON][1][1];
				entity->focalz = limbs[DRAGON][1][2];

				DRAGON_FLOATX = 0.0;
				DRAGON_FLOATY = 0.0;
				DRAGON_FLOATZ = 0.0;

				//DRAGON_FLOATZ += 24 * sin(body->pitch);
				//if ( DRAGON_STATE == 2 )
				//{
				//	DRAGON_FLOATZ += -2 * walkMult * sin(((DRAGON_WALK_ANIM * 1.25) / 2 - PI / 2));
				//	entity->pitch += (PI / 16) * walkMult * sin(((DRAGON_WALK_ANIM * 1.25) / 2) + PI / 2);
				//}
				//else
				//{
				//	entity->pitch += (PI / 64) * walkMult * sin((DRAGON_WALK_ANIM / 2) + PI);
				//	//DRAGON_FLOATZ += -0.5 + 0.5 * walkMult * sin((DRAGON_WALK_ANIM / 2) + PI / 2);
				//}
				
				// walk
				{
					entity->pitch += walkMult * (PI / 32 + (0 * PI / 64) * sin((DRAGON_WALK_ANIM / 2) + PI));
					DRAGON_FLOATZ += walkMult * (-0.5 + 0.5 * sin((DRAGON_WALK_ANIM * 2) + PI / 2));
					entity->roll += walkMult * ((PI / 32) * sin((DRAGON_WALK_ANIM)));
				}

				// stand
				{
					entity->pitch -= standMult * ((PI * 0.3) + (PI / 128) * sin((DRAGON_STAND_ANIM / 2)));

					real_t atkMult = 0.0;
					if ( standAtkAnim <= 0.5 * PI )
					{
						atkMult = -0.5 * sin(std::min(PI / 2, standAtkAnim));
					}
					else if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 2.5 * PI )
					{
						atkMult = -0.5 + 1.5 * sin(std::min(PI / 2, (standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim >= 2.5 * PI && standAtkAnim < 3.5 * PI )
					{
						atkMult = 1.0 - 1.0 * sin((standAtkAnim - 2.5 * PI) / 2);
					}
					entity->pitch += standAtkMult * PI / 12 * atkMult;

					real_t atkMult2 = 0.0;
					if ( standAtkAnim <= 0.5 * PI )
					{
						atkMult2 = -0.5 * sin(std::min(PI / 2, standAtkAnim));
					}
					else if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 2.0 * PI )
					{
						atkMult2 = -0.5 + 1.5 * sin(std::min(PI / 2, (standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim >= 2.0 * PI && standAtkAnim < 3.5 * PI )
					{
						atkMult2 = 1.0 - 1.0 * sin((standAtkAnim - 2.0 * PI) / 2);
					}
					DRAGON_FLOATX += standAtkMult * 4.0 * atkMult2 * (cos(my->yaw));
					DRAGON_FLOATY += standAtkMult * 4.0 * atkMult2 * (sin(my->yaw));
				}

				// stand fly
				{
					entity->pitch -= (1.0 - 0.5 * flyAtkMult) * flyMult * ((PI * 0.1) + (1.0 - glideMult) * (PI / 16) * sin((DRAGON_FLY_ANIM)));
					entity->pitch += glideMult * flyMult * PI * 0.45;
					if ( DRAGON_FLY_ANIM < 2 * PI )
					{
						entity->pitch -= flyMult * (PI / 8) * sin(std::min(PI, DRAGON_FLY_ANIM / 2));
						//DRAGON_FLOATX -= sin(std::min(PI, DRAGON_FLY_ANIM / 2)) * flyMult * 12.0 * (cos(my->yaw));
						//DRAGON_FLOATY -= sin(std::min(PI, DRAGON_FLY_ANIM / 2)) * flyMult * 12.0 * (sin(my->yaw));
					}

					real_t prevFlyZ = DRAGON_FLY_Z;

					if ( *cvar_dragon_pose == 8
						|| *cvar_dragon_pose == 9
						|| *cvar_dragon_pose == 10 )
					{
						// *sin(std::max(0.0, std::min(PI / 2, (DRAGON_FLY_ANIM) * 0.25)));
						if ( DRAGON_FLY_Z < 0.25 )
						{
							real_t inc = std::max(0.01, (flyMult - DRAGON_FLY_Z) / 20);
							DRAGON_FLY_Z = std::min(1.0, DRAGON_FLY_Z + inc);
						}
						else
						{
							real_t inc = std::max(0.01, (flyMult - DRAGON_FLY_Z) / 5);
							DRAGON_FLY_Z = std::min(1.0, DRAGON_FLY_Z + inc);
						}
					}
					else
					{
						if ( DRAGON_FLY_Z >= 0.25 )
						{
							real_t inc = std::max((DRAGON_FLY_Z) / 15.0, 0.01);
							DRAGON_FLY_Z = std::max(0.0, DRAGON_FLY_Z - inc);
						}
						else if ( DRAGON_FLY_Z >= 0.2 )
						{
							DRAGON_FLY_Z -= 0.01;
						}
						else if ( DRAGON_FLY_Z >= 0.0 )
						{
							real_t inc = std::max(DRAGON_FLY_Z / 10.0, 0.01);
							DRAGON_FLY_Z = std::max(0.0, DRAGON_FLY_Z - inc);
						}
					}

					if ( prevFlyZ < 0.25 && DRAGON_FLY_Z > 0.25 )
					{
						leftleg->skill[1] = 2; // poof
						rightleg->skill[1] = 2; // poof
					}
					else if ( prevFlyZ > 0.05 && DRAGON_FLY_Z < 0.05 )
					{
						leftleg->skill[1] = 2; // poof
						rightleg->skill[1] = 2; // poof
					}

					DRAGON_FLOATZ -= (48.0 + 48.0 * glideMult) * DRAGON_FLY_Z;

					DRAGON_FLOATX -= flyMult * sin(DRAGON_FLY_ANIM) * 2.0 * (cos(my->yaw));
					DRAGON_FLOATY -= flyMult * sin(DRAGON_FLY_ANIM) * 2.0 * (sin(my->yaw));
					DRAGON_FLOATZ += 4.0 * flyMult * sin(DRAGON_FLY_ANIM + PI / 4);

					// fly atk
					{
						entity->pitch += flyAtkMult * (PI / 5);
					}
				}

				{
					// idle atk
					real_t atkMult = 0.0;
					if ( atkAnim >= 0.0 * PI && atkAnim < 0.5 * PI )
					{
						atkMult = -sin(atkAnim * 2);
						DRAGON_FLOATZ += 2.0 * atkMult * idleAtkMult;

						entity->pitch -= idleAtkMult * atkMult * PI / 12;
					}
					else if ( atkAnim >= 0.5 * PI && atkAnim < 2.0 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
						DRAGON_FLOATZ += -3.0 * idleAtkMult * sin(std::min(PI, 1.25 * (atkAnim - 0.5 * PI)));

						if ( atkAnim < 1.5 * PI )
						{
							real_t pitch = 1.0 * sin(std::min(PI, (atkAnim - 0.5 * PI)));
							entity->pitch -= idleAtkMult * pitch * PI / 12;
						}
					}
					else if ( atkAnim >= 2.0 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 2.0 * PI) / 2));
					}
					atkMult *= idleAtkMult;

					real_t atkMult2 = 0.0;
					if ( atkAnim >= 0.5 * PI && atkAnim < 2.0 * PI )
					{
						atkMult2 = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 2.0 * PI )
					{
						atkMult2 = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 2.0 * PI) / 2));
					}
					DRAGON_FLOATX += idleAtkMult * atkMult2 * 8.0 * (cos(my->yaw));
					DRAGON_FLOATY += idleAtkMult * atkMult2 * 8.0 * (sin(my->yaw));

					// idle combat
					entity->pitch += (1.0 - atkMult) * idleMult * ((PI / 16) + (PI / 32) * sin(DRAGON_IDLE_ANIM));
					real_t angOffset = 2 * PI * 0.26;
					DRAGON_FLOATZ -= -15.5 * sin(body->pitch - PI / 2 + angOffset);
				}
				// idle wait
				{
					DRAGON_FLOATZ -= idleWaitMult * (-2.0 + idleWaitZ * sin(DRAGON_IDLE_WAIT_ANIM));
					entity->pitch += idleWaitMult * ((PI / 128) * sin(DRAGON_IDLE_WAIT_ANIM / 2));
				}

				// run
				{
					DRAGON_FLOATZ += runMult * (0.5) * sin(DRAGON_RUN_ANIM);
					if ( fmod(DRAGON_RUN_ANIM, 2 * PI) >= PI )
					{
						DRAGON_FLOATZ += runMult * (12) * sin(DRAGON_RUN_ANIM);
					}
					entity->pitch += runMult * ((PI / 4) * sin((DRAGON_RUN_ANIM + PI / 4)));
				}

				// descend
				{
					entity->pitch += descendMult * ((0.5 * PI) * sin(DRAGON_DESCEND_ANIM));
					DRAGON_FLOATX += descendMult * 16.0 * sin(DRAGON_DESCEND_ANIM) * (cos(my->yaw));
					DRAGON_FLOATY += descendMult * 16.0 * sin(DRAGON_DESCEND_ANIM) * (sin(my->yaw));
					DRAGON_FLOATZ += descendMult * 4.0 * sin(DRAGON_DESCEND_ANIM);
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;

				if ( ticks % 5 == 0 && DRAGON_FLY_Z > 0.05 && DRAGON_FLY_Z < 0.95 )
				{
					createWindAOE(my, entity);
				}
				break;
			}
			case DRAGON_NECK:
			{
				entity->scalex = 1.0025;
				entity->scaley = 1.0025;
				entity->scalez = 1.0025;

				entity->x += limbs[DRAGON][4][0] * cos(my->yaw) + limbs[DRAGON][4][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][4][0] * sin(my->yaw) + limbs[DRAGON][4][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][4][2];
				entity->focalx = limbs[DRAGON][3][0];
				entity->focaly = limbs[DRAGON][3][1];
				entity->focalz = limbs[DRAGON][3][2];

				DRAGON_NECK_Z = 0.0;

				// walk
				{
					entity->pitch += (PI / 32) * walkMult * sin((DRAGON_WALK_ANIM) + PI);
				}

				// run
				{
					entity->pitch += runMult * (PI / 2 - PI / 16 + ((PI / 8) * sin((DRAGON_RUN_ANIM + PI / 2))));
					DRAGON_NECK_Z += runMult * (4.0 - 2.0 * sin(DRAGON_RUN_ANIM));
					DRAGON_NECK_FORWARD = runMult * (2.0 + 2.0 * sin(DRAGON_RUN_ANIM + PI / 2));
					entity->x += DRAGON_NECK_FORWARD * cos(my->yaw);
					entity->y += DRAGON_NECK_FORWARD * sin(my->yaw);
				}

				// stand
				{
					entity->pitch += standMult * (PI / 3 + ((PI / 128) * sin((DRAGON_STAND_ANIM + PI / 2))));
					DRAGON_NECK_Z += (1.0 - flyMult) * standMult * (6.0);

					real_t atkMult = 0.0;
					if ( standAtkAnim <= 0.5 * PI )
					{
						atkMult = -0.5 * sin(std::min(PI / 2, standAtkAnim));
					}
					else if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 2.5 * PI )
					{
						atkMult = -0.5 + 1.5 * sin(std::min(PI / 2, (standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim >= 2.5 * PI && standAtkAnim < 3.5 * PI )
					{
						atkMult = 1.0 - 1.0 * sin((standAtkAnim - 2.5 * PI) / 2);
					}
					entity->pitch += standAtkMult * PI / 8 * atkMult;
					DRAGON_NECK_Z += standAtkMult * atkMult * 2.0;
				}

				// fly
				{
					entity->pitch -= (1.0 - 0.9 * glideMult) * flyMult * (PI / 3);
					if ( DRAGON_FLY_ANIM < 2 * PI )
					{
						entity->pitch -= flyMult * (PI / 8) * sin(std::min(PI, DRAGON_FLY_ANIM / 2));
					}

					entity->pitch += flyAtkMult * (PI / 8 + (PI / 8) * sin(flyAtkAnim / 2));
					DRAGON_NECK_Z += flyAtkMult * (1.0 + 1.0 * sin(flyAtkAnim / 2));
				}

				// descend
				{
					entity->pitch += descendMult * (PI / 4) * sin(DRAGON_DESCEND_ANIM * 2);
				}

				entity->pitch += idleMult * (PI / 2 - PI / 16 + (PI / 32) * sin(DRAGON_IDLE_ANIM + PI));
				DRAGON_NECK_Z += idleMult * (6.0 - 0.5 * sin(DRAGON_IDLE_ANIM));

				entity->pitch += walkMult * (PI / 3);// +(PI / 64) * sin(DRAGON_WALK_ANIM + PI));
				DRAGON_NECK_Z += walkMult * (4.0 - 0.5 * sin(DRAGON_WALK_ANIM));

				entity->z -= DRAGON_NECK_Z;
				//entity->x += (DRAGON_NECK_Z / 4) * cos(my->yaw);
				//entity->y += (DRAGON_NECK_Z / 4) * sin(my->yaw);

				{
					real_t angleLook = idleWaitMult * (PI / 4) * sin(idleWaitTurn);
					entity->yaw += angleLook;
					entity->x += idleWaitMult * (5.0 + -3.5 * cos(angleLook)) * cos(my->yaw);
					entity->y += idleWaitMult * (5.0 + -3.5 * cos(angleLook)) * sin(my->yaw);
					entity->x += idleWaitMult * -3.5 * (sin(angleLook)) * cos(my->yaw + PI / 2);
					entity->y += idleWaitMult * -3.5 * (sin(angleLook)) * sin(my->yaw + PI / 2);
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_HEAD:
			{
				entity->scalex = 1.005;
				entity->scaley = 1.005;
				entity->scalez = 1.005;
				entity->x += limbs[DRAGON][6][0] * cos(my->yaw) + limbs[DRAGON][6][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][6][0] * sin(my->yaw) + limbs[DRAGON][6][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][6][2];
				entity->focalx = limbs[DRAGON][5][0];
				entity->focaly = limbs[DRAGON][5][1];
				entity->focalz = limbs[DRAGON][5][2];

				real_t angOffset = 2 * PI * 0.13;
				entity->x += 11 * cos(body->yaw) * sin(neck->pitch + angOffset);
				entity->y += 11 * sin(body->yaw) * sin(neck->pitch + angOffset);
				entity->z += 11 * sin(neck->pitch - PI / 2 + angOffset);

				entity->z -= DRAGON_NECK_Z * 0.6;
				entity->x += (DRAGON_NECK_Z / 4) * cos(my->yaw);
				entity->y += (DRAGON_NECK_Z / 4) * sin(my->yaw);

				entity->pitch += idleMult * (PI / 12 + ((PI / 64) * sin(DRAGON_IDLE_ANIM)));

				DRAGON_LIMB_PITCH = 0.0;

				{
					real_t angleLook = idleWaitMult * (PI * 0.4) * sin(std::max(-PI * 0.5, std::min(PI * 0.5, idleWaitTurn * 1.25)));
					entity->yaw += angleLook;
					entity->pitch += idleWaitMult * (PI / 64 + (PI / 64) * sin(DRAGON_IDLE_WAIT_ANIM));
				}

				// run
				{
					entity->pitch += runMult * ((PI / 32) + ((PI / 32) * sin((DRAGON_RUN_ANIM + PI / 2))));
				}

				// stand
				{
					entity->pitch += standMult * (PI / 4);
					entity->z -= standMult * (3);

					if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 1.5 * PI )
					{
						entity->pitch -= standAtkMult * PI / 8 * sin(((standAtkAnim - 0.5 * PI) / 2));
					}
					else if ( standAtkAnim >= 1.5 * PI && standAtkAnim < 3.0 * PI )
					{
						entity->pitch -= standAtkMult * (PI / 8);
					}
					else if ( standAtkAnim >= 3.0 * PI )
					{
						entity->pitch -= standAtkMult * (PI / 8 - PI / 8 * sin((standAtkAnim - 3.0 * PI) * 0.5));
					}
				}

				// fly
				{
					entity->pitch -= flyMult * (PI / 16);
					entity->pitch -= glideMult * flyMult * (PI * 0.2);
					if ( DRAGON_FLY_ANIM < 2 * PI )
					{
						entity->pitch -= flyMult * (PI / 2) * sin(std::min(PI, DRAGON_FLY_ANIM / 2));
					}
					else
					{
						entity->pitch += (1.0 - 0.9 * glideMult) * flyMult * (PI / 8 - (PI / 16) * sin(DRAGON_FLY_ANIM));
					}

					entity->z += 3.0 * glideMult;
				}

				// idle atk
				{
					real_t atkMult = 0.0;
					if ( atkAnim >= 0.0 * PI && atkAnim < 1.5 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.0 * PI)));
					}
					else if ( atkAnim >= 1.5 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 1.5 * PI)));
					}
					entity->pitch -= idleAtkMult * atkMult * (PI / 8);
					entity->roll += idleAtkMult * atkMult * (PI * 0.25);
				}

				entity->x += idleWaitMult * 3.5 * cos(entity->yaw);
				entity->y += idleWaitMult * 3.5 * sin(entity->yaw);

				entity->yaw += walkMult * (PI / 64) * sin(DRAGON_WALK_ANIM / 2);
				entity->x += walkMult * (-1) * sin(DRAGON_WALK_ANIM / 2 + PI / 4) * cos(my->yaw + PI / 2);
				entity->y += walkMult * (-1) * sin(DRAGON_WALK_ANIM / 2 + PI / 4) * sin(my->yaw + PI / 2);

				entity->x += DRAGON_NECK_FORWARD * cos(my->yaw);
				entity->y += DRAGON_NECK_FORWARD * sin(my->yaw);

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_JAW:
			{
				entity->x += limbs[DRAGON][8][0] * cos(my->yaw) + limbs[DRAGON][8][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][8][0] * sin(my->yaw) + limbs[DRAGON][8][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][8][2];
				entity->focalx = limbs[DRAGON][7][0];
				entity->focaly = limbs[DRAGON][7][1];
				entity->focalz = limbs[DRAGON][7][2];

				entity->pitch = head->pitch;

				{
					real_t angleLook = idleWaitMult * (PI * 0.4) * sin(std::max(-PI * 0.5, std::min(PI * 0.5, idleWaitTurn * 1.25)));
					entity->yaw += angleLook;
					entity->pitch += idleWaitMult * (PI / 64 + (PI / 64) * sin(DRAGON_IDLE_WAIT_ANIM));
				}
				entity->x += idleWaitMult * 3.5 * cos(entity->yaw);
				entity->y += idleWaitMult * 3.5 * sin(entity->yaw);

				real_t angOffset2 = 2 * PI * 0.13;
				entity->x += 11 * cos(body->yaw) * sin(neck->pitch + angOffset2);
				entity->y += 11 * sin(body->yaw) * sin(neck->pitch + angOffset2);
				entity->z += 11 * sin(neck->pitch - PI / 2 + angOffset2);

				real_t angOffset = 2 * PI * 0.265;
				entity->x += 5 * cos(head->yaw) * sin(head->pitch + angOffset);
				entity->y += 5 * sin(head->yaw) * sin(head->pitch + angOffset);
				entity->z += 5 * sin(head->pitch - PI / 2 + angOffset);

				//entity->pitch += idleMult * (PI / 16 + (PI / 32) * sin(DRAGON_IDLE_ANIM + PI));
				entity->pitch += walkMult * (PI / 64 + (PI / 64) * sin(DRAGON_WALK_ANIM + PI));
				entity->pitch += runMult * (PI / 32 + (PI / 32) * sin(DRAGON_RUN_ANIM + PI));
				entity->pitch += (1.0 - flyMult) * standMult * (PI / 8 + (PI / 64) * sin(DRAGON_RUN_ANIM + PI));

				entity->z -= DRAGON_NECK_Z * 0.6;
				entity->x += (DRAGON_NECK_Z / 4) * cos(my->yaw);
				entity->y += (DRAGON_NECK_Z / 4) * sin(my->yaw);

				entity->yaw += walkMult * (PI / 64) * sin(DRAGON_WALK_ANIM / 2);
				entity->x += walkMult * (-1) * sin(DRAGON_WALK_ANIM / 2 + PI / 4) * cos(my->yaw + PI / 2);
				entity->y += walkMult * (-1) * sin(DRAGON_WALK_ANIM / 2 + PI / 4) * sin(my->yaw + PI / 2);

				{
					entity->z -= standMult * (3);
					real_t atkMult = 0.0;
					if ( standAtkAnim <= 0.5 * PI )
					{
						atkMult = -0.5 * sin(std::min(PI / 2, standAtkAnim));
					}
					else if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 2.5 * PI )
					{
						atkMult = -0.5 + 1.5 * sin(std::min(PI / 2, (standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim >= 2.5 * PI && standAtkAnim < 3.0 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (standAtkAnim - 2.5 * PI)));
					}
					entity->pitch += standMult * atkMult * (PI / 4);
				}

				// idle atk
				{
					real_t atkMult = 0.0;
					if ( atkAnim >= 0.0 * PI && atkAnim < 1.5 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.0 * PI)));
					}
					else if ( atkAnim >= 1.5 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 1.5 * PI)));
					}
					entity->roll += idleAtkMult * atkMult * (PI * 0.25);

					real_t atkMult2 = 0.0;
					if ( atkAnim >= 0.5 * PI && atkAnim < 1.0 * PI )
					{
						atkMult2 = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 1.0 * PI )
					{
						atkMult2 = 1.0 - 1.0 * sin(std::min(PI / 2, 1.5 * (atkAnim - 1.0 * PI)));
					}
					real_t pitch = idleAtkMult * atkMult2 * (PI / 3);
					entity->pitch += pitch;
					entity->roll += pitch * 0.5;
					entity->yaw += pitch * -1.0;
				}

				// fly atk
				{
					entity->pitch += flyAtkMult * (PI / 6 + (PI / 6) * sin(flyAtkAnim / 2));

					entity->z += 3.0 * glideMult;
				}

				entity->x += DRAGON_NECK_FORWARD * cos(my->yaw);
				entity->y += DRAGON_NECK_FORWARD * sin(my->yaw);

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_LEFTARM:
			{
				entity->x += limbs[DRAGON][10][0] * cos(my->yaw) + limbs[DRAGON][10][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][10][0] * sin(my->yaw) + limbs[DRAGON][10][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][10][2];
				entity->focalx = limbs[DRAGON][9][0];
				entity->focaly = limbs[DRAGON][9][1];
				entity->focalz = limbs[DRAGON][9][2];

				entity->pitch = body->pitch;

				// walk
				{
					real_t walkAnim = DRAGON_WALK_ANIM - PI / 4;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = walkMult * (walkAngle)*sin(walkAng);

					walkPitch -= walkMult * PI / 8;
					entity->pitch += walkPitch;
				}

				// run
				{
					real_t walkAnim = DRAGON_RUN_ANIM - PI / 4;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = runMult * (walkAngle) * sin(walkAng);

					walkPitch -= runMult * PI / 8;
					entity->pitch += walkPitch;

					if ( DRAGON_LIMB_POOF )
					{
						if ( walkAng > PI )
						{
							DRAGON_LIMB_POOF = 0;
						}
					}
					else if ( runMult > 0.5
						&& (walkAng >= 1.9 * PI - 0.15)
						&& (walkAng <= 1.9 * PI + 0.15) )
					{
						spawnPoof(entity->x, entity->y, 7.5, 1.0);
						DRAGON_LIMB_POOF = 1;
					}
				}
				//else if ( DRAGON_STATE == 2 )
				//{
				//	real_t walkAnim = DRAGON_WALK_ANIM * 1.25 - PI + PI / 2 + (*cvar_dragon_leap_front * PI / 4);
				//	real_t walkAng = fmodf(walkAnim, 4 * PI);

				//	real_t walkPitch = walkMult * ((-PI / 16) + (PI / 4) * sin(walkAng / 2));
				//	entity->pitch += walkPitch;

				//	entity->x += walkMult * sin(-walkAng / 2) * cos(body->yaw);
				//	entity->y += walkMult * sin(-walkAng / 2) * sin(body->yaw);
				//	entity->z += walkMult * cos(walkAng / 2);
				//}

				DRAGON_ARM_Z = 0.0;

				// idle combat
				{
					entity->pitch += (PI / 32) * idleMult * sin(DRAGON_IDLE_ANIM);
					DRAGON_ARM_Z += idleMult * (-1.5 + -0.5 * sin(DRAGON_IDLE_ANIM));
					DRAGON_ARM_FORWARD = idleMult * ((1.5 + 1.5 * sin(DRAGON_IDLE_ANIM)) - 2.0);
					entity->x += DRAGON_ARM_FORWARD * cos(body->yaw);
					entity->y += DRAGON_ARM_FORWARD * sin(body->yaw);
					DRAGON_ARM_SIDE = idleMult * -2.0;
					entity->roll += idleMult * (PI / 64);
					entity->yaw += idleMult * (PI / 16);
				}

				// idle wait
				{
					DRAGON_ARM_Z += idleWaitMult * idleWaitZ * sin(DRAGON_IDLE_WAIT_ANIM);
					entity->pitch += (PI / 8) * idleWaitMult;
				}

				// stand
				{
					entity->pitch += (PI / 4 - (PI / 64) * sin(DRAGON_STAND_ANIM)) * standMult;
					entity->roll -= (PI / 32) * standMult;
					//DRAGON_ARM_SIDE = standMult * -2.0;
				}

				// fly
				{
					entity->roll += (PI / 8) * flyMult;
					DRAGON_ARM_Z += flyMult * (1.0 + (1.0 - 0.9 * glideMult) * (1.0) * sin(DRAGON_FLY_ANIM));
					entity->pitch += flyMult * (PI / 16 + ((1.0 - 0.9 * glideMult) * (1.0 - flyAtkMult) * (-PI / 16) * sin(DRAGON_FLY_ANIM)));
					entity->pitch -= flyAtkMult * (PI / 12 + (1.0 - 0.9 * glideMult) * (PI / 12) * sin(flyAtkAnim / 2));
				}

				// idle atk
				{
					real_t atkMult = 0.0;
					if ( atkAnim >= 0.5 * PI && atkAnim < 2.0 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 2.0 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 2.0 * PI) / 2));
					}
					entity->pitch -= idleAtkMult * atkMult * PI / 4;
					DRAGON_ARM_Z += idleAtkMult * atkMult * 3.5;

					real_t atkMult2 = 0.0;
					if ( atkAnim >= 0.0 * PI && atkAnim < 0.5 * PI )
					{
						atkMult2 = 0.5 * sin(std::min(PI / 2, (atkAnim)));
					}
					else if ( atkAnim >= 0.5 * PI && atkAnim < 1.0 * PI )
					{
						atkMult2 = 0.5 - 1.5 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 1.0 * PI )
					{
						atkMult2 = -1.0 + 1.0 * sin(std::min(PI / 2, (atkAnim - 1.0 * PI)));
					}
					DRAGON_ARM_SIDE += idleAtkMult * atkMult2 * 1.0;
					entity->roll -= idleAtkMult * atkMult2 * (PI / 12);
				}

				entity->x += DRAGON_ARM_SIDE * cos(body->yaw + PI / 2);
				entity->y += DRAGON_ARM_SIDE * sin(body->yaw + PI / 2);
				entity->z += DRAGON_ARM_Z;

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_LEFTCLAW:
			{
				entity->x += limbs[DRAGON][12][0] * cos(my->yaw) + limbs[DRAGON][12][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][12][0] * sin(my->yaw) + limbs[DRAGON][12][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][12][2];
				entity->focalx = limbs[DRAGON][11][0];
				entity->focaly = limbs[DRAGON][11][1];
				entity->focalz = limbs[DRAGON][11][2];

				entity->pitch = body->pitch;

				real_t angOffset = 2 * PI * 0.0725;
				entity->x += -10 * cos(body->yaw) * sin(leftarm->pitch + angOffset);
				entity->y += -10 * sin(body->yaw) * sin(leftarm->pitch + angOffset);
				real_t offsetZ = -10 * sin(leftarm->pitch - PI / 2 + angOffset);

				// walk
				{
					real_t walkAnim = DRAGON_WALK_ANIM - PI / 4 - PI / 2;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;

					walkPitch = walkMult * (walkAngle * 1.25) * sin(std::min(PI, walkAng * 0.75));
					entity->pitch -= walkPitch;

					if ( walkAng >= 1 * PI && walkAng < 2.0 * PI )
					{
						offsetZ *= 1.0 - walkMult * 0.25 * (sin(walkAng - PI));
					}

					entity->roll -= walkMult * ((PI / 16) * std::max(0.0, sin(walkAng)));
					entity->x -= (walkMult * 0.5) * cos(body->yaw + PI / 2) * std::max(0.0, sin(walkAng));
					entity->y -= (walkMult * 0.5) * sin(body->yaw + PI / 2) * std::max(0.0, sin(walkAng));

					//entity->x += walkMult * ((0.5 + 0.5 * sin(walkAng - PI / 4 - PI / 8))) * cos(body->yaw);
					//entity->y += walkMult * ((0.5 + 0.5 * sin(walkAng - PI / 4 - PI / 8))) * sin(body->yaw);
					//entity->z -= walkMult * 1 * sin((walkAng / (2 * PI)) * PI);

				}

				// run
				{
					real_t walkAnim = DRAGON_RUN_ANIM - PI / 4;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = runMult * (walkAngle)*sin(walkAng);
					if ( walkAng >= 1.0 * PI && walkAng < 1.5 * PI )
					{
						walkPitch -= runMult * (PI / 4) * sin((1.5 * PI - walkAng) * 2.0);
					}
					walkPitch -= runMult * PI / 8;
					entity->pitch += walkPitch;
				}

				// stand
				{
					entity->roll -= (PI / 8) * standMult;
					entity->pitch += (PI / 8 + (PI / 64) * sin(DRAGON_STAND_ANIM)) * standMult;

					entity->x += standMult * (1.5) * cos(body->yaw + PI / 2);
					entity->y += standMult * (1.5) * sin(body->yaw + PI / 2);

					if ( standAtkAnim < 0.5 * PI )
					{
						entity->pitch += standAtkMult * (-PI / 8 * sin(standAtkAnim * 2));
					}
					else if ( standAtkAnim < 1.0 * PI )
					{
						entity->pitch += standAtkMult * (PI / 4 * sin((standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim < 3.0 * PI )
					{
						entity->pitch += standAtkMult * PI / 4;
					}
					else if ( standAtkAnim < 3.5 * PI )
					{
						entity->pitch += standAtkMult * (PI / 4 - PI / 4 * sin((standAtkAnim + PI)));
					}
				}

				// fly
				{
					entity->roll -= (PI / 5) * flyMult;
					entity->pitch += (PI / 8) * flyMult;
					entity->x -= flyMult * (3.5) * cos(body->yaw + PI / 2);
					entity->y -= flyMult * (3.5) * sin(body->yaw + PI / 2);
					if ( DRAGON_FLY_ANIM < 2 * PI )
					{
						entity->pitch += flyMult * (PI / 4) * sin(std::min(PI, DRAGON_FLY_ANIM / 2));
					}

					entity->pitch += flyMult * (PI / 8 + ((1.0 - 0.9 * glideMult) * (1.0 - flyAtkMult) * (PI / 8) * sin(DRAGON_FLY_ANIM)));
					entity->pitch -= flyAtkMult * (PI / 8 + (1.0 - 0.9 * glideMult) * (PI / 8) * sin(flyAtkAnim / 2));
				}

				entity->z += offsetZ;

				{
					entity->pitch -= idleMult * ((PI / 16) + (PI / 32) * sin(DRAGON_IDLE_ANIM));
					entity->x += DRAGON_ARM_FORWARD * cos(body->yaw);
					entity->y += DRAGON_ARM_FORWARD * sin(body->yaw);
					entity->z += DRAGON_ARM_Z;

					entity->x += (DRAGON_ARM_SIDE * 1.5) * cos(body->yaw + PI / 2);
					entity->y += (DRAGON_ARM_SIDE * 1.5) * sin(body->yaw + PI / 2);
					entity->x += idleMult * 1.75 * cos(body->yaw);
					entity->y += idleMult * 1.75 * sin(body->yaw);
					entity->yaw -= idleMult * (PI / 32 + (PI / 64) * sin(DRAGON_IDLE_ANIM));
				}

				// idle atk
				{
					real_t atkMult2 = 0.0;
					if ( atkAnim >= 1.0 * PI && atkAnim < 2.0 * PI )
					{
						atkMult2 = sin(std::min(PI / 2, (atkAnim - 1.0 * PI)));
					}
					else if ( atkAnim >= 2.0 * PI && atkAnim < 3.5 * PI )
					{
						atkMult2 = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 2.0 * PI) / 2));
					}
					entity->x += (atkMult2 * idleAtkMult * 3.0) * cos(body->yaw + PI / 2);
					entity->y += (atkMult2 * idleAtkMult * 3.0) * sin(body->yaw + PI / 2);

					real_t atkMult = 0.0;
					if ( atkAnim >= 0.0 * PI && atkAnim < 0.75 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.0 * PI)));
					}
					else if ( atkAnim >= 0.75 * PI && atkAnim < 1.5 * PI )
					{
						atkMult = 1 - 1.0 * sin(std::min(PI / 2, (atkAnim - 0.75 * PI)));
					}

					if ( atkAnim >= 1.5 * PI && atkAnim < 3.5 * PI )
					{
						entity->z -= 1.0 * idleAtkMult * sin(std::max(0.0, std::min(PI, 0.75 * (atkAnim - 1.5 * PI))));
					}
					entity->yaw += atkMult * idleAtkMult * (PI / 8);
					entity->pitch -= atkMult * idleAtkMult * (PI / 4);
				}

				{
					entity->z += idleWaitMult * 0.5;
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_RIGHTARM:
			{
				entity->x += limbs[DRAGON][14][0] * cos(my->yaw) + limbs[DRAGON][14][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][14][0] * sin(my->yaw) + limbs[DRAGON][14][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][14][2];
				entity->focalx = limbs[DRAGON][13][0];
				entity->focaly = limbs[DRAGON][13][1];
				entity->focalz = limbs[DRAGON][13][2];

				entity->pitch = body->pitch;

				// walk
				{
					real_t walkAnim = DRAGON_WALK_ANIM + PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = walkMult * (walkAngle) * sin(walkAng);

					walkPitch -= walkMult * PI / 8;
					entity->pitch += walkPitch;
				}

				{
					real_t walkAnim = DRAGON_RUN_ANIM - PI / 4;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = runMult * (walkAngle)*sin(walkAng);

					walkPitch -= runMult * PI / 8;
					entity->pitch += walkPitch;

					if ( DRAGON_LIMB_POOF )
					{
						if ( walkAng > PI )
						{
							DRAGON_LIMB_POOF = 0;
						}
					}
					else if ( runMult > 0.5
						&& (walkAng >= 1.75 * PI - 0.15)
						&& (walkAng <= 1.75 * PI + 0.15) )
					{
						spawnPoof(entity->x, entity->y, 7.5, 1.0);
						DRAGON_LIMB_POOF = 1;
					}
				}

				// idle atk
				{
					real_t atkMult = 0.0;
					if ( atkAnim >= 0.5 * PI && atkAnim < 2.0 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 2.0 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 2.0 * PI) / 2));
					}

					real_t atkMult2 = 0.0;
					if ( atkAnim >= 0.0 * PI && atkAnim < 0.5 * PI )
					{
						atkMult2 = 0.5 * sin(std::min(PI / 2, (atkAnim)));
					}
					else if ( atkAnim >= 0.5 * PI && atkAnim < 1.0 * PI )
					{
						atkMult2 = 0.5 - 1.5 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 1.0 * PI )
					{
						atkMult2 = -1.0 + 1.0 * sin(std::min(PI / 2, (atkAnim - 1.0 * PI)));
					}
					entity->roll += idleAtkMult * atkMult2 * (PI / 12);
					entity->pitch -= idleAtkMult * atkMult * PI / 4;
				}

				{
					entity->pitch += (PI / 32) * idleMult * sin(DRAGON_IDLE_ANIM);
					entity->x += DRAGON_ARM_FORWARD * cos(body->yaw);
					entity->y += DRAGON_ARM_FORWARD * sin(body->yaw);
					entity->x += -DRAGON_ARM_SIDE * cos(body->yaw + PI / 2);
					entity->y += -DRAGON_ARM_SIDE * sin(body->yaw + PI / 2);
					entity->z += DRAGON_ARM_Z;
					entity->roll += -idleMult * (PI / 64);
					entity->yaw += -idleMult * (PI / 16);
				}

				// stand
				{
					entity->pitch += (PI / 4 - (PI / 64) * sin(DRAGON_STAND_ANIM)) * standMult;
					entity->roll += (PI / 32) * standMult;
				}

				// fly
				{
					entity->roll -= (PI / 8) * flyMult;
					entity->pitch += flyMult * (PI / 16 + ((1.0 - 0.9 * glideMult) * (1.0 - flyAtkMult) * (-PI / 16) * sin(DRAGON_FLY_ANIM)));
					entity->pitch -= flyAtkMult * (PI / 12 + (1.0 - 0.9 * glideMult) * (PI / 12) * sin(flyAtkAnim / 2));
				}

				{
					entity->pitch += (PI / 8) * idleWaitMult;
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_RIGHTCLAW:
			{
				entity->x += limbs[DRAGON][16][0] * cos(my->yaw) + limbs[DRAGON][16][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][16][0] * sin(my->yaw) + limbs[DRAGON][16][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][16][2];
				entity->focalx = limbs[DRAGON][15][0];
				entity->focaly = limbs[DRAGON][15][1];
				entity->focalz = limbs[DRAGON][15][2];

				entity->pitch = body->pitch;

				real_t angOffset = 2 * PI * 0.0725;
				entity->x += -10 * cos(body->yaw) * sin(rightarm->pitch + angOffset);
				entity->y += -10 * sin(body->yaw) * sin(rightarm->pitch + angOffset);

				real_t offsetZ = -10 * sin(rightarm->pitch - PI / 2 + angOffset);

				// walk
				{
					real_t walkAnim = DRAGON_WALK_ANIM + 0.5 * PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;

					walkPitch = walkMult * (walkAngle * 1.25) * sin(std::min(PI, walkAng * 0.75));
					entity->pitch -= walkPitch;

					if ( walkAng >= 1 * PI && walkAng < 2.0 * PI )
					{
						offsetZ *= 1.0 - walkMult * 0.25 * (sin(walkAng - PI));
					}

					entity->roll += walkMult * ((PI / 16) * std::max(0.0, sin(walkAng)));
					entity->x += (walkMult * 0.5) * cos(body->yaw + PI / 2) * std::max(0.0, sin(walkAng));
					entity->y += (walkMult * 0.5) * sin(body->yaw + PI / 2) * std::max(0.0, sin(walkAng));
				}
				
				// run
				{
					real_t walkAnim = DRAGON_RUN_ANIM - PI / 4;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = runMult * (walkAngle)*sin(walkAng);
					if ( walkAng >= 1.0 * PI && walkAng < 1.5 * PI )
					{
						walkPitch -= runMult * (PI / 4) * sin((1.5 * PI - walkAng) * 2.0);
					}
					walkPitch -= runMult * PI / 8;
					entity->pitch += walkPitch;
				}

				// stand
				{
					entity->roll += (PI / 8) * standMult;
					entity->pitch += (PI / 8 + (PI / 64) * sin(DRAGON_STAND_ANIM)) * standMult;

					entity->x -= standMult * (1.5) * cos(body->yaw + PI / 2);
					entity->y -= standMult * (1.5) * sin(body->yaw + PI / 2);

					if ( standAtkAnim < 0.5 * PI )
					{
						entity->pitch += standAtkMult * (-PI / 8 * sin(standAtkAnim * 2));
					}
					else if ( standAtkAnim < 1.0 * PI )
					{
						entity->pitch += standAtkMult * (PI / 4 * sin((standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim < 3.0 * PI )
					{
						entity->pitch += standAtkMult * PI / 4;
					}
					else if ( standAtkAnim < 3.5 * PI )
					{
						entity->pitch += standAtkMult * (PI / 4 - PI / 4 * sin((standAtkAnim + PI)));
					}
				}

				// fly
				{
					entity->roll += (PI / 5) * flyMult;
					entity->pitch += (PI / 8) * flyMult;
					entity->x += flyMult * (3.5) * cos(body->yaw + PI / 2);
					entity->y += flyMult * (3.5) * sin(body->yaw + PI / 2);

					if ( DRAGON_FLY_ANIM < 2 * PI )
					{
						entity->pitch += flyMult * (PI / 4) * sin(std::min(PI, DRAGON_FLY_ANIM / 2));
					}

					entity->pitch += flyMult * (PI / 8 + ((1.0 - 0.9 * glideMult) * (1.0 - flyAtkMult) * (PI / 8) * sin(DRAGON_FLY_ANIM)));
					entity->pitch -= flyAtkMult * (PI / 8 + (1.0 - 0.9 * glideMult) * (PI / 8) * sin(flyAtkAnim / 2));
				}

				entity->z += offsetZ;

				{
					entity->pitch -= idleMult * ((PI / 16) + (PI / 32) * sin(DRAGON_IDLE_ANIM));
					entity->x += DRAGON_ARM_FORWARD * cos(body->yaw);
					entity->y += DRAGON_ARM_FORWARD * sin(body->yaw);
					entity->z += DRAGON_ARM_Z;

					entity->x += (-DRAGON_ARM_SIDE * 1.5) * cos(body->yaw + PI / 2);
					entity->y += (-DRAGON_ARM_SIDE * 1.5) * sin(body->yaw + PI / 2);
					entity->x += idleMult * 1.75 * cos(body->yaw);
					entity->y += idleMult * 1.75 * sin(body->yaw);
					entity->yaw += idleMult * (PI / 32 + (PI / 64) * sin(DRAGON_IDLE_ANIM));
				}

				// idle atk
				{
					real_t atkMult2 = 0.0;
					if ( atkAnim >= 1.0 * PI && atkAnim < 2.0 * PI )
					{
						atkMult2 = sin(std::min(PI / 2, (atkAnim - 1.0 * PI)));
					}
					else if ( atkAnim >= 2.0 * PI && atkAnim < 3.5 * PI )
					{
						atkMult2 = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 2.0 * PI) / 2));
					}
					entity->x -= (atkMult2 * idleAtkMult * 3.0) * cos(body->yaw + PI / 2);
					entity->y -= (atkMult2 * idleAtkMult * 3.0) * sin(body->yaw + PI / 2);

					real_t atkMult = 0.0;
					if ( atkAnim >= 0.0 * PI && atkAnim < 0.75 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (atkAnim - 0.0 * PI)));
					}
					else if ( atkAnim >= 0.75 * PI && atkAnim < 2.5 * PI )
					{
						atkMult = 1 - 1.0 * sin(std::min(PI / 2, (atkAnim - 0.75 * PI)));
					}

					if ( atkAnim >= 1.5 * PI && atkAnim < 3.5 * PI )
					{
						entity->z -= 1.0 * idleAtkMult * sin(std::max(0.0, std::min(PI, 0.75 * (atkAnim - 1.5 * PI))));
					}
					entity->yaw -= atkMult * idleAtkMult * (PI / 8);
					entity->pitch -= atkMult * idleAtkMult * (PI / 4);
				}

				{
					entity->z += idleWaitMult * 0.5;
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_LEFTLEG:
			{
				entity->x += limbs[DRAGON][18][0] * cos(my->yaw) + limbs[DRAGON][18][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][18][0] * sin(my->yaw) + limbs[DRAGON][18][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][18][2];
				entity->focalx = limbs[DRAGON][17][0];
				entity->focaly = limbs[DRAGON][17][1];
				entity->focalz = limbs[DRAGON][17][2];

				real_t angOffset = 2 * PI * 0.26;
				entity->x += -15.5 * cos(body->yaw) * sin(body->pitch + angOffset);
				entity->y += -15.5 * sin(body->yaw) * sin(body->pitch + angOffset);
				entity->z += -15.5 * sin(body->pitch - PI / 2 + angOffset);

				// walk
				{
					real_t walkAnim = DRAGON_WALK_ANIM - PI / 4 + PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = walkMult * (walkAngle / 2) * sin(walkAng);

					walkPitch += walkMult * PI / 8;
					entity->pitch += walkPitch;
				}
				
				// run
				{
					real_t walkAnim = DRAGON_RUN_ANIM - PI / 4 + PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = runMult * (walkAngle)*sin(walkAng);

					walkPitch += runMult * PI / 8;
					entity->pitch += walkPitch;

					if ( DRAGON_LIMB_POOF )
					{
						if ( runMult > 0.5 && walkAng < PI )
						{
							DRAGON_LIMB_POOF = 0;
						}
					}
					else if ( runMult > 0.5
						&& (walkAng >= 1.75 * PI - 0.15)
						&& (walkAng <= 1.75 * PI + 0.15) )
					{
						spawnPoof(entity->x, entity->y, 7.5, 1.0);
						DRAGON_LIMB_POOF = 1;
					}
				}

				// idle
				{
					entity->yaw += -idleMult * (PI / 32 + 0 * (PI / 64) * sin(DRAGON_IDLE_ANIM));
					DRAGON_LEG_SIDE = idleMult * -1.0;
					DRAGON_LEG_FORWARD = idleMult * (-2.0 /*- 1.0 * sin(DRAGON_IDLE_ANIM)*/);
					entity->x += DRAGON_LEG_SIDE * cos(body->yaw + PI / 2);
					entity->y += DRAGON_LEG_SIDE * sin(body->yaw + PI / 2);
					entity->x += DRAGON_LEG_FORWARD * cos(body->yaw);
					entity->y += DRAGON_LEG_FORWARD * sin(body->yaw);
				}

				// idle atk
				{
					real_t atkMult = 0.0;
					if ( atkAnim <= 0.5 * PI )
					{
						atkMult = -0.5 * sin(std::min(PI / 2, atkAnim));
					}
					else if ( atkAnim >= 0.5 * PI && atkAnim < 1.5 * PI )
					{
						atkMult = -0.5 + 1.5 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 1.5 * PI && atkAnim < 3.5 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 1.5 * PI) / 2));
					}
					entity->pitch -= -idleAtkMult * atkMult * PI / 4;
				}

				// idle wait
				{
					DRAGON_LEG_Z = idleWaitMult * idleWaitZ * sin(DRAGON_IDLE_WAIT_ANIM);
					entity->z += DRAGON_LEG_Z;
					entity->pitch += (PI / 8) * idleWaitMult;
				}

				// stand
				{
					entity->yaw += -standMult * (PI / 8 + 0 * (PI / 64) * sin(DRAGON_STAND_ANIM));
					real_t atkMult = 0.0;
					if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 3.0 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim >= 3.0 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (standAtkAnim - 3.0 * PI)));
					}
					entity->yaw += standMult * atkMult * (PI / 24);
				}

				// fly
				{
					entity->pitch -= (1.0 - 0.9 * glideMult) * flyMult * (PI / 16 - ((1.0 - 0.5 * flyAtkMult) * (PI / 32) * sin(DRAGON_FLY_ANIM)));
					entity->pitch += glideMult * (PI * 0.3);

					// fly atk
					{
						entity->pitch -= flyAtkMult * (PI / 8 + (PI / 8) * sin(flyAtkAnim / 2));
					}

					if ( DRAGON_LIMB_POOF == 2 )
					{
						spawnPoof(entity->x, entity->y, 7.5, 1.0);
						DRAGON_LIMB_POOF = 0;
					}
				}

				// descend
				{
					entity->pitch += descendMult * ((0.75 * PI) * sin(DRAGON_DESCEND_ANIM));
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_LEFTFOOT:
			{
				entity->x += limbs[DRAGON][20][0] * cos(my->yaw) + limbs[DRAGON][20][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][20][0] * sin(my->yaw) + limbs[DRAGON][20][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][20][2];
				entity->focalx = limbs[DRAGON][19][0];
				entity->focaly = limbs[DRAGON][19][1];
				entity->focalz = limbs[DRAGON][19][2];

				real_t angOffset2 = 2 * PI * 0.26;
				entity->x += -15.5 * cos(body->yaw) * sin(body->pitch + angOffset2);
				entity->y += -15.5 * sin(body->yaw) * sin(body->pitch + angOffset2);
				entity->z += -15.5 * sin(body->pitch - PI / 2 + angOffset2);

				real_t angOffset = 2 * PI * 0.045;
				entity->x += -12.6 * cos(body->yaw) * sin(leftleg->pitch + angOffset);
				entity->y += -12.6 * sin(body->yaw) * sin(leftleg->pitch + angOffset);
				entity->z += -12.6 * sin(leftleg->pitch - PI / 2 + angOffset);

				// walk
				{
					real_t walkAnim = DRAGON_WALK_ANIM + PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = walkMult * (walkAngle / 2) * sin(walkAng);

					walkPitch += walkMult * PI / 8;
					entity->pitch += walkPitch;

					entity->roll -= walkMult * ((PI / 12) * std::max(0.0, sin(walkAng)));
				}
				
				// run
				{
					real_t walkAnim = DRAGON_RUN_ANIM - PI / 4 + PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = runMult * (walkAngle)*sin(walkAng);
					if ( walkAng >= 0.0 * PI && walkAng < 0.5 * PI )
					{
						walkPitch += runMult * (PI / 4) * sin((0.5 * PI - walkAng) * 2.0);
					}
					walkPitch += runMult * PI / 8;
					entity->pitch += walkPitch;
				}

				{
					entity->yaw += -idleMult * (PI / 32 + 0 * (PI / 64) * sin(DRAGON_IDLE_ANIM));
					entity->x += idleMult * 3.5 * (PI / 32 + 0 * (PI / 64) * sin(DRAGON_IDLE_ANIM)) * cos(body->yaw + PI / 2);
					entity->y += idleMult * 3.5 * (PI / 32 + 0 * (PI / 64) * sin(DRAGON_IDLE_ANIM)) * sin(body->yaw + PI / 2);
					entity->x += DRAGON_LEG_SIDE * cos(body->yaw + PI / 2);
					entity->y += DRAGON_LEG_SIDE * sin(body->yaw + PI / 2);
					entity->x += DRAGON_LEG_FORWARD * cos(body->yaw);
					entity->y += DRAGON_LEG_FORWARD * sin(body->yaw);
				}

				// stand
				{
					entity->yaw += -standMult * (PI / 8 + 0 * (PI / 64) * sin(DRAGON_STAND_ANIM));
					real_t atkMult = 0.0;
					if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 3.0 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim >= 3.0 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (standAtkAnim - 3.0 * PI)));
					}
					entity->yaw += standMult * atkMult * (PI / 12);
				}

				// fly
				{
					entity->pitch += (PI / 8) * flyMult;
					entity->pitch += (PI * 0.3) * glideMult;
					if ( DRAGON_FLY_ANIM < 2 * PI )
					{
						entity->pitch += flyMult * (PI / 4) * sin(std::min(PI, DRAGON_FLY_ANIM / 2));
					}
					entity->pitch += flyMult * (PI / 8 + ((1.0 - 0.9 * glideMult) * (1.0 - 0.5 * flyAtkMult) * (PI / 8) * sin(DRAGON_FLY_ANIM)));

					// fly atk
					{
						entity->pitch -= flyAtkMult * (PI / 3) * (0.5 + 0.5 * sin(flyAtkAnim / 2));
					}
				}

				// descend
				{
					entity->pitch += descendMult * ((1.0 * PI) * sin(DRAGON_DESCEND_ANIM));
				}

				// idle atk
				{
					real_t atkMult = 0.0;
					if ( atkAnim <= 0.5 * PI )
					{
						atkMult = -0.5 * sin(std::min(PI / 2, atkAnim));
					}
					else if ( atkAnim >= 0.5 * PI && atkAnim < 1.5 * PI )
					{
						atkMult = -0.5 + 1.5 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 1.5 * PI && atkAnim < 3.5 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 1.5 * PI) / 2));
					}
					entity->pitch -= -idleAtkMult * atkMult * PI / 2;
				}

				{
					entity->z += DRAGON_LEG_Z;
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_RIGHTLEG:
			{
				entity->x += limbs[DRAGON][22][0] * cos(my->yaw) + limbs[DRAGON][22][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][22][0] * sin(my->yaw) + limbs[DRAGON][22][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][22][2];
				entity->focalx = limbs[DRAGON][21][0];
				entity->focaly = limbs[DRAGON][21][1];
				entity->focalz = limbs[DRAGON][21][2];

				//entity->pitch = leftleg->pitch + PI;

				real_t angOffset = 2 * PI * 0.26;
				entity->x += -15.5 * cos(body->yaw) * sin(body->pitch + angOffset);
				entity->y += -15.5 * sin(body->yaw) * sin(body->pitch + angOffset);
				entity->z += -15.5 * sin(body->pitch - PI / 2 + angOffset);

				// walk
				{
					real_t walkAnim = DRAGON_WALK_ANIM - PI / 4;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = walkMult * (walkAngle / 2) * sin(walkAng);

					walkPitch += walkMult * PI / 8;
					entity->pitch += walkPitch;
				}
				
				// run
				{
					real_t walkAnim = DRAGON_RUN_ANIM - PI / 4 + PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = runMult * (walkAngle)*sin(walkAng);

					walkPitch += runMult * PI / 8;
					entity->pitch += walkPitch;

					if ( DRAGON_LIMB_POOF )
					{
						if ( runMult > 0.5 && walkAng < PI )
						{
							DRAGON_LIMB_POOF = 0;
						}
					}
					else if ( runMult > 0.5
						&& (walkAng >= 1.5 * PI - 0.15)
						&& (walkAng <= 1.5 * PI + 0.15) )
					{
						spawnPoof(entity->x, entity->y, 7.5, 1.0);
						DRAGON_LIMB_POOF = 1;
					}
				}

				{
					entity->yaw += idleMult * (PI / 32 + 0 * (PI / 64) * sin(DRAGON_IDLE_ANIM));
					entity->x += -DRAGON_LEG_SIDE * cos(body->yaw + PI / 2);
					entity->y += -DRAGON_LEG_SIDE * sin(body->yaw + PI / 2);
					entity->x += DRAGON_LEG_FORWARD * cos(body->yaw);
					entity->y += DRAGON_LEG_FORWARD * sin(body->yaw);
				}

				// stand
				{
					entity->yaw += standMult * (PI / 8 + 0 * (PI / 64) * sin(DRAGON_STAND_ANIM));
					real_t atkMult = 0.0;
					if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 3.0 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim >= 3.0 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (standAtkAnim - 3.0 * PI)));
					}
					entity->yaw -= standMult * atkMult * (PI / 24);
				}

				// idle atk
				{
					real_t atkMult = 0.0;
					if ( atkAnim <= 0.5 * PI )
					{
						atkMult = -0.5 * sin(std::min(PI / 2, atkAnim));
					}
					else if ( atkAnim >= 0.5 * PI && atkAnim < 1.5 * PI )
					{
						atkMult = -0.5 + 1.5 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 1.5 * PI && atkAnim < 3.5 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 1.5 * PI) / 2));
					}
					entity->pitch -= -idleAtkMult * atkMult * PI / 4;
				}

				// fly
				{
					entity->pitch -= (1.0 - 0.9 * glideMult) * flyMult * (PI / 16 - ((1.0 - 0.5 * flyAtkMult) * (PI / 32) * sin(DRAGON_FLY_ANIM)));
					entity->pitch += glideMult * (PI * 0.3);

					// fly atk
					{
						entity->pitch -= flyAtkMult * (PI / 8 + (PI / 8) * sin(flyAtkAnim / 2));
					}

					if ( DRAGON_LIMB_POOF == 2 )
					{
						spawnPoof(entity->x, entity->y, 7.5, 1.0);
						DRAGON_LIMB_POOF = 0;
					}
				}

				// descend
				{
					entity->pitch += descendMult * ((0.75 * PI) * sin(DRAGON_DESCEND_ANIM));
				}

				{
					entity->z += DRAGON_LEG_Z;
					entity->pitch += (PI / 8) * idleWaitMult;
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_RIGHTFOOT:
			{
				entity->x += limbs[DRAGON][24][0] * cos(my->yaw) + limbs[DRAGON][24][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][24][0] * sin(my->yaw) + limbs[DRAGON][24][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][24][2];
				entity->focalx = limbs[DRAGON][23][0];
				entity->focaly = limbs[DRAGON][23][1];
				entity->focalz = limbs[DRAGON][23][2];

				real_t angOffset2 = 2 * PI * 0.26;
				entity->x += -15.5 * cos(body->yaw) * sin(body->pitch + angOffset2);
				entity->y += -15.5 * sin(body->yaw) * sin(body->pitch + angOffset2);
				entity->z += -15.5 * sin(body->pitch - PI / 2 + angOffset2);

				real_t angOffset = 2 * PI * 0.045;
				entity->x += -12.6 * cos(body->yaw) * sin(rightleg->pitch + angOffset);
				entity->y += -12.6 * sin(body->yaw) * sin(rightleg->pitch + angOffset);
				entity->z += -12.6 * sin(rightleg->pitch - PI / 2 + angOffset);

				// walk
				{
					real_t walkAnim = DRAGON_WALK_ANIM;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = walkMult * (walkAngle) * sin(walkAng);

					walkPitch += walkMult * PI / 8;
					entity->pitch += walkPitch;

					entity->roll += walkMult * ((PI / 12) * std::max(0.0, sin(walkAng)));
				}
				
				// run
				{
					real_t walkAnim = DRAGON_RUN_ANIM - PI / 4 + PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					real_t walkPitch = 0.0;
					walkPitch = runMult * (walkAngle)*sin(walkAng);
					if ( walkAng >= 0.0 * PI && walkAng < 0.5 * PI )
					{
						walkPitch += runMult * (PI / 4) * sin((0.5 * PI - walkAng) * 2.0);
					}
					walkPitch += runMult * PI / 8;
					entity->pitch += walkPitch;
				}

				{
					entity->yaw += idleMult * (PI / 32 + 0 * (PI / 64) * sin(DRAGON_IDLE_ANIM));
					entity->x += -idleMult * 3.5 * (PI / 32 + 0 * (PI / 64) * sin(DRAGON_IDLE_ANIM)) * cos(body->yaw + PI / 2);
					entity->y += -idleMult * 3.5 * (PI / 32 + 0 * (PI / 64) * sin(DRAGON_IDLE_ANIM)) * sin(body->yaw + PI / 2);
					entity->x += -DRAGON_LEG_SIDE * cos(body->yaw + PI / 2);
					entity->y += -DRAGON_LEG_SIDE * sin(body->yaw + PI / 2);
					entity->x += DRAGON_LEG_FORWARD * cos(body->yaw);
					entity->y += DRAGON_LEG_FORWARD * sin(body->yaw);
				}

				// stand
				{
					entity->yaw += standMult * (PI / 8 + 0 * (PI / 64) * sin(DRAGON_STAND_ANIM));
					real_t atkMult = 0.0;
					if ( standAtkAnim >= 0.5 * PI && standAtkAnim < 3.0 * PI )
					{
						atkMult = 1.0 * sin(std::min(PI / 2, (standAtkAnim - 0.5 * PI)));
					}
					else if ( standAtkAnim >= 3.0 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (standAtkAnim - 3.0 * PI)));
					}
					entity->yaw -= standMult * atkMult * (PI / 12);
				}

				// fly
				{
					entity->pitch += (PI / 8) * flyMult;
					entity->pitch += (PI * 0.3) * glideMult;
					if ( DRAGON_FLY_ANIM < 2 * PI )
					{
						entity->pitch += flyMult * (PI / 4) * sin(std::min(PI, DRAGON_FLY_ANIM / 2));
					}
					entity->pitch += flyMult * (PI / 8 + ((1.0 - 0.9 * glideMult) * (1.0 - 0.5 * flyAtkMult) * (PI / 8) * sin(DRAGON_FLY_ANIM)));

					// fly atk
					{
						entity->pitch -= flyAtkMult * (PI / 3) * (0.5 + 0.5 * sin(flyAtkAnim / 2));
					}
				}

				// descend
				{
					entity->pitch += descendMult * ((1.0 * PI) * sin(DRAGON_DESCEND_ANIM));
				}

				// idle atk
				{
					real_t atkMult = 0.0;
					if ( atkAnim <= 0.5 * PI )
					{
						atkMult = -0.5 * sin(std::min(PI / 2, atkAnim));
					}
					else if ( atkAnim >= 0.5 * PI && atkAnim < 1.5 * PI )
					{
						atkMult = -0.5 + 1.5 * sin(std::min(PI / 2, (atkAnim - 0.5 * PI)));
					}
					else if ( atkAnim >= 1.5 * PI && atkAnim < 3.5 * PI )
					{
						atkMult = 1.0 - 1.0 * sin(std::min(PI / 2, (atkAnim - 1.5 * PI) / 2));
					}
					entity->pitch -= -idleAtkMult * atkMult * PI / 2;
				}

				{
					entity->z += DRAGON_LEG_Z;
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_TAIL:
			{
				entity->x += limbs[DRAGON][26][0] * cos(my->yaw) + limbs[DRAGON][26][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[DRAGON][26][0] * sin(my->yaw) + limbs[DRAGON][26][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[DRAGON][26][2];
				entity->focalx = limbs[DRAGON][25][0];
				entity->focaly = limbs[DRAGON][25][1];
				entity->focalz = limbs[DRAGON][25][2];

				/*if ( DRAGON_STATE == 2 )
				{
					entity->pitch += (PI / 64) * walkMult * sin((DRAGON_WALK_ANIM)+PI / 2);
				}*/
				
				// walk
				{
					entity->pitch += (PI / 32) * walkMult * sin((DRAGON_WALK_ANIM) + PI / 2);
				}
				entity->yaw += (PI / 64) * walkMult * sin(DRAGON_WALK_ANIM / 2 - PI / 2);

				// stand
				{
					entity->pitch += standMult * (-PI / 8 + (PI / 64) * sin((DRAGON_STAND_ANIM)+PI / 2));
				}

				// fly
				{
					entity->pitch -= flyMult * (PI / 8);
					entity->pitch += glideMult * (PI / 4);
					if ( DRAGON_FLY_ANIM >= PI && DRAGON_FLY_ANIM < 2 * PI )
					{
						entity->pitch -= flyMult * (PI / 16) * sin(std::min(PI, DRAGON_FLY_ANIM));
					}
				}

				// descend
				{
					entity->pitch -= descendMult * (PI / 2) * sin(DRAGON_DESCEND_ANIM);
				}

				real_t angOffset = 2 * PI * 0.2545;
				entity->x += -20 * cos(body->yaw) * sin(body->pitch + angOffset);
				entity->y += -20 * sin(body->yaw) * sin(body->pitch + angOffset);
				entity->z += -20 * sin(body->pitch - PI / 2 + angOffset);

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_WINGLEFT:
			{
				if ( *cvar_dragon_wing )
				{
					if ( entity->sprite != 2524 )
					{
						entity->bNeedsRenderPositionInit = true;
					}
					entity->sprite = 2524;
				}
				else
				{
					if ( entity->sprite != 2523 )
					{
						entity->bNeedsRenderPositionInit = true;
					}
					entity->sprite = 2523;
				}

				if ( entity->sprite == 2523 )
				{
					entity->x += limbs[DRAGON][28][0] * cos(my->yaw) + limbs[DRAGON][28][1] * cos(my->yaw + PI / 2);
					entity->y += limbs[DRAGON][28][0] * sin(my->yaw) + limbs[DRAGON][28][1] * sin(my->yaw + PI / 2);
					entity->z += limbs[DRAGON][28][2];
					entity->focalx = limbs[DRAGON][27][0];
					entity->focaly = limbs[DRAGON][27][1];
					entity->focalz = limbs[DRAGON][27][2];

					real_t angOffset = 2 * PI * -0.0225;
					entity->x += 7.05 * cos(body->yaw) * sin(body->pitch + angOffset);
					entity->y += 7.05 * sin(body->yaw) * sin(body->pitch + angOffset);
					entity->z += 7.05 * sin(body->pitch - PI / 2 + angOffset);
				}
				else
				{
					entity->x += limbs[DRAGON][33][0] * cos(my->yaw) + limbs[DRAGON][33][1] * cos(my->yaw + PI / 2);
					entity->y += limbs[DRAGON][33][0] * sin(my->yaw) + limbs[DRAGON][33][1] * sin(my->yaw + PI / 2);
					entity->z += limbs[DRAGON][33][2];
					entity->focalx = limbs[DRAGON][32][0];
					entity->focaly = limbs[DRAGON][32][1];
					entity->focalz = limbs[DRAGON][32][2];

					real_t angOffset = 2 * PI * -0.021;
					entity->x += 7.55 * cos(body->yaw) * sin(body->pitch + angOffset);
					entity->y += 7.55 * sin(body->yaw) * sin(body->pitch + angOffset);
					entity->z += 7.55 * sin(body->pitch - PI / 2 + angOffset);
				}

				//entity->roll += (PI / 32) * DRAGON_WALK_MULT * sin((DRAGON_WALK_ANIM * 2) + PI);
				entity->roll += (PI / 16) * DRAGON_WALK_MULT * sin((DRAGON_WALK_ANIM / 2) + PI);
				entity->roll += (PI / 16) * DRAGON_IDLE_MULT * sin((DRAGON_IDLE_ANIM));
				entity->roll += (PI / 16) * DRAGON_IDLE_WAIT_MULT * sin((DRAGON_IDLE_WAIT_ANIM / 2) + PI);
				entity->roll -= DRAGON_RUN_MULT * (PI / 8 + (PI / 8) * sin((DRAGON_RUN_ANIM)));
				entity->roll -= DRAGON_STAND_MULT * ((PI / 16) * sin((DRAGON_STAND_ANIM)));

				entity->pitch += DRAGON_LIMB_PITCH;
				entity->roll += DRAGON_WING_ROLL;
				entity->yaw += DRAGON_WING_YAW;

				// fly
				{
					real_t flyAng = (PI - 0.2 * PI);
					entity->pitch += flyMult * (0.15 - (0.5 + (1.0 - 0.9 * glideMult) * 0.5 * sin(DRAGON_FLY_ANIM + flyAng)));
					entity->roll += flyMult * (1.15 - (0.85 + (1.0 - 0.9 * glideMult) * 0.85 * sin(DRAGON_FLY_ANIM + flyAng)));
					entity->yaw += flyMult * (0.5 - (0.35 + (1.0 - 0.9 * glideMult) * 0.35 * sin(DRAGON_FLY_ANIM + flyAng)));
					entity->pitch += glideMult * flyMult * 0.35;
					entity->roll += glideMult * flyMult * -0.4;
					entity->yaw += glideMult * flyMult * 0.3;
					entity->pitch -= (1.0 - flyMult) * DRAGON_STAND_MULT * (PI / 4 + (PI / 32) * sin((DRAGON_STAND_ANIM)));
					entity->roll -= flyMult * ((PI / 4) + (0.0 * PI * 0.4) * sin((DRAGON_FLY_ANIM + PI - 0.2 * PI)));
				}

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			case DRAGON_WINGRIGHT:
			{
				if ( *cvar_dragon_wing )
				{
					if ( entity->sprite != 2526 )
					{
						entity->bNeedsRenderPositionInit = true;
					}
					entity->sprite = 2526;
				}
				else
				{
					if ( entity->sprite != 2525 )
					{
						entity->bNeedsRenderPositionInit = true;
					}
					entity->sprite = 2525;
				}

				if ( entity->sprite == 2525 )
				{
					entity->x += limbs[DRAGON][30][0] * cos(my->yaw) + limbs[DRAGON][30][1] * cos(my->yaw + PI / 2);
					entity->y += limbs[DRAGON][30][0] * sin(my->yaw) + limbs[DRAGON][30][1] * sin(my->yaw + PI / 2);
					entity->z += limbs[DRAGON][30][2];
					entity->focalx = limbs[DRAGON][29][0];
					entity->focaly = limbs[DRAGON][29][1];
					entity->focalz = limbs[DRAGON][29][2];

					real_t angOffset = 2 * PI * -0.0225;
					entity->x += 7.05 * cos(body->yaw) * sin(body->pitch + angOffset);
					entity->y += 7.05 * sin(body->yaw) * sin(body->pitch + angOffset);
					entity->z += 7.05 * sin(body->pitch - PI / 2 + angOffset);
				}
				else
				{
					entity->x += limbs[DRAGON][35][0] * cos(my->yaw) + limbs[DRAGON][35][1] * cos(my->yaw + PI / 2);
					entity->y += limbs[DRAGON][35][0] * sin(my->yaw) + limbs[DRAGON][35][1] * sin(my->yaw + PI / 2);
					entity->z += limbs[DRAGON][35][2];
					entity->focalx = limbs[DRAGON][34][0];
					entity->focaly = limbs[DRAGON][34][1];
					entity->focalz = limbs[DRAGON][34][2];

					real_t angOffset = 2 * PI * -0.021;
					entity->x += 7.55 * cos(body->yaw) * sin(body->pitch + angOffset);
					entity->y += 7.55 * sin(body->yaw) * sin(body->pitch + angOffset);
					entity->z += 7.55 * sin(body->pitch - PI / 2 + angOffset);
				}

				//entity->roll -= (PI / 32) * DRAGON_WALK_MULT * sin((DRAGON_WALK_ANIM * 2) + PI);
				entity->roll = -wingleft->roll;
				entity->yaw += my->yaw - wingleft->yaw;
				entity->pitch = wingleft->pitch;

				entity->x += DRAGON_FLOATX;
				entity->y += DRAGON_FLOATY;
				entity->z += DRAGON_FLOATZ;
				break;
			}
			default:
				break;
		}
	}

	if ( MONSTER_ATTACK > 0 )
	{
		MONSTER_ATTACKTIME++;
	}
	else if ( MONSTER_ATTACK == 0 )
	{
		MONSTER_ATTACKTIME = 0;
	}
	else
	{
		// do nothing, don't reset attacktime or increment it.
	}
}