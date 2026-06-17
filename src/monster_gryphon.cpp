/*-------------------------------------------------------------------------------

	BARONY
	File: monster_cockatrice.cpp
	Desc: implements all of the cockatrice monster's code

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

void initGryphon(Entity* my, Stat* myStats)
{
	node_t* node;

	my->flags[BURNABLE] = true;
	my->flags[INVISIBLE] = true;
	my->initMonster(2430);
	my->z = 0.0;

	if ( multiplayer != CLIENT )
	{
		MONSTER_SPOTSND = 385;
		MONSTER_SPOTVAR = 3;
		MONSTER_IDLESND = 382;
		MONSTER_IDLEVAR = 2;
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

			// boss variants

			// random effects
			myStats->setEffectActive(EFF_LEVITATING, 1);
			myStats->EFFECTS_TIMERS[EFF_LEVITATING] = 0;

			// generates equipment and weapons if available from editor
			createMonsterEquipment(myStats, rng);

			// create any custom inventory items from editor if available
			createCustomInventory(myStats, customItemsToGenerate, rng);

			// count if any custom inventory items from editor
			int customItems = countCustomItems(myStats); //max limit of 6 custom items per entity.

														 // count any inventory items set to default in edtior
			int defaultItems = countDefaultItems(myStats);

			my->setHardcoreStats(*myStats);
		}
	}

	// torso
	Entity* entity = newEntity(2415, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->focaly = 1;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][1][0];
	entity->focaly = limbs[GRYPHON][1][1];
	entity->focalz = limbs[GRYPHON][1][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// head
	entity = newEntity(2416, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->focaly = 1;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][3][0];
	entity->focaly = limbs[GRYPHON][3][1];
	entity->focalz = limbs[GRYPHON][3][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// left leg
	entity = newEntity(2417, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][5][0];
	entity->focaly = limbs[GRYPHON][5][1];
	entity->focalz = limbs[GRYPHON][5][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// right leg
	entity = newEntity(2419, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][7][0];
	entity->focaly = limbs[GRYPHON][7][1];
	entity->focalz = limbs[GRYPHON][7][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// left hind
	entity = newEntity(2421, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][9][0];
	entity->focaly = limbs[GRYPHON][9][1];
	entity->focalz = limbs[GRYPHON][9][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// right hind
	entity = newEntity(2423, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][11][0];
	entity->focaly = limbs[GRYPHON][11][1];
	entity->focalz = limbs[GRYPHON][11][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// tail
	entity = newEntity(2425, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][13][0];
	entity->focaly = limbs[GRYPHON][13][1];
	entity->focalz = limbs[GRYPHON][13][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// wing left
	entity = newEntity(2426, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][15][0];
	entity->focaly = limbs[GRYPHON][15][1];
	entity->focalz = limbs[GRYPHON][15][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// wing right
	entity = newEntity(2428, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[GRYPHON][17][0];
	entity->focaly = limbs[GRYPHON][17][1];
	entity->focalz = limbs[GRYPHON][17][2];
	entity->behavior = &actGryphonLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);
}

void actGryphonLimb(Entity* my)
{
	my->actMonsterLimb();
}

void gryphonDie(Entity* my)
{
	int c;
	for ( c = 0; c < 12; c++ )
	{
		Entity* gib = spawnGib(my);
	    if (c < 8) {
			switch ( c )
			{
			case 0:
				gib->sprite = my->sprite + 1;
				break;
			case 1:
				gib->sprite = my->sprite + 2;
				break;
			case 2:
				gib->sprite = my->sprite + 3;
				break;
			case 3:
				gib->sprite = my->sprite + 5;
				break;
			case 4:
				gib->sprite = my->sprite + 7;
				break;
			case 5:
				gib->sprite = my->sprite + 9;
				break;
			case 6:
				gib->sprite = my->sprite + 12;
				break;
			case 7:
				gib->sprite = my->sprite + 14;
				break;
			default:
				break;
			}

	        gib->skill[5] = 1; // poof
	    }
		if ( my->bodyparts.size() )
		{
			gib->z = my->bodyparts[0]->z - 4;
		}
		serverSpawnGibForClient(gib);
	}

	my->spawnBlood();

	//playSoundEntity(my, 28, 128);
	playSoundEntity(my, 388 + local_rng.rand() % 2, 128);

	my->removeMonsterDeathNodes();

	list_RemoveNode(my->mynode);
	return;
}

#define COCKATRICEWALKSPEED .01
#define GRYPHON_BODY 2
#define GRYPHON_HEAD 3
#define GRYPHON_LEFTLEG 4
#define GRYPHON_RIGHTLEG 5
#define GRYPHON_LEFTHIND 6
#define GRYPHON_RIGHTHIND 7
#define GRYPHON_TAIL 8
#define GRYPHON_LEFTWING 9
#define GRYPHON_RIGHTWING 10
#define GRYPHON_LIMB_YAW entity->fskill[0]
#define GRYPHON_LIMB_PITCH entity->fskill[1]
#define GRYPHON_LIMB_ROLL entity->fskill[2]
#define GRYPHON_STATE body->skill[0]
#define GRYPHON_STATE_ANIM body->fskill[3]
#define GRYPHON_FLY_ANIM body->fskill[4]
#define GRYPHON_FLOAT_Z body->fskill[5]
#define GRYPHON_WALK_ANIM body->fskill[6]
#define GRYPHON_WALK_MULT body->fskill[7]
#define GRYPHON_ATK_ANIM body->fskill[8]
#define GRYPHON_ATK_X body->fskill[9]
#define GRYPHON_DIVE_ANIM body->fskill[10]
#define GRYPHON_DIVE_PITCH body->fskill[11]

void gryphonCeilingBust(Entity* my)
{
	if ( multiplayer == CLIENT )
	{
		return;
	}
	auto entLists = TileEntityList.getEntitiesWithinRadiusAroundEntity(my, 1);
	for ( std::vector<list_t*>::iterator it = entLists.begin(); it != entLists.end(); ++it )
	{
		list_t* currentList = *it;
		node_t* node;
		node_t* nextnode;
		for ( node = currentList->first; node != nullptr; node = nextnode )
		{
			nextnode = node->next;
			Entity* ent = (Entity*)node->element;
			if ( entityInsideEntity(my, ent) )
			{
				if ( ent->behavior == &actCeilingTile )
				{
					for ( int i = 0; i < 3; ++i )
					{
						Entity* childEntity = nullptr;
						if ( multiplayer == SERVER )
						{
							childEntity = spawnGib(ent);
						}
						else
						{
							childEntity = spawnGibClient(ent->x, ent->y, ent->z, 5);
						}
						if ( childEntity )
						{
							childEntity->x = ((int)(ent->x / 16)) * 16 + local_rng.rand() % 16;
							childEntity->y = ((int)(ent->y / 16)) * 16 + local_rng.rand() % 16;
							childEntity->z = -24;
							childEntity->flags[PASSABLE] = true;
							childEntity->flags[INVISIBLE] = false;
							childEntity->flags[NOUPDATE] = true;
							childEntity->flags[UPDATENEEDED] = false;
							if ( ent->sprite == 1635 || ent->sprite == 1636 )
							{
								childEntity->sprite = 1643;
							}
							else
							{
								childEntity->sprite = items[GEM_ROCK].index;
							}
							childEntity->yaw = local_rng.rand() % 360 * PI / 180;
							childEntity->pitch = local_rng.rand() % 360 * PI / 180;
							childEntity->roll = local_rng.rand() % 360 * PI / 180;
							childEntity->vel_x = (local_rng.rand() % 20 - 10) / 10.0;
							childEntity->vel_y = (local_rng.rand() % 20 - 10) / 10.0;
							childEntity->vel_z = -.25;
							childEntity->fskill[3] = 0.03;
						}
					}
					list_RemoveNode(ent->mynode);
				}
				else if ( ent->behavior == &actDoor )
				{
					ent->doorHealth = 0; // destroy the door
				}
				else if ( ent->isDamageableCollider() )
				{
					ent->colliderCurrentHP = 0;
					ent->colliderKillerUid = 0;
				}
				else if ( ent->behavior == &actFurniture )
				{
					ent->furnitureHealth = 0;
				}
			}
		}
	}
}

void createVortexFx(Entity* my, real_t x, real_t y)
{
	int duration = 100;
	Entity* spellTimer = createParticleTimer(my, duration, -1);
	spellTimer->particleTimerCountdownAction = PARTICLE_TIMER_ACTION_VORTEX_AESTHETIC;
	spellTimer->particleTimerCountdownSprite = -1;
	//spellTimer->particleTimerVariable2 = spell->ID;
	spellTimer->flags[UPDATENEEDED] = true;
	spellTimer->flags[NOUPDATE] = false;
	spellTimer->yaw = my->yaw;
	spellTimer->x = x;
	spellTimer->y = y;
	/*if ( spellBookBonusPercent > 0 )
	{
		spellTimer->actmagicSpellbookBonus = spellBookBonusPercent;
	}
	spellTimer->actmagicFromSpellbook = usingSpellbook ? 1 : 0;*/

	/*if ( caster->behavior == &actMonster )
	{
		spellTimer->x = castSpellProps->caster_x + 8.0 * cos(tangent);
		spellTimer->y = castSpellProps->caster_y + 8.0 * sin(tangent);
		spellTimer->vel_x = 3.0 * cos(spellTimer->yaw);
		spellTimer->vel_y = 3.0 * sin(spellTimer->yaw);
	}*/
	spellTimer->particleTimerDuration = std::min(spellTimer->particleTimerDuration, 0xFFF);
	Sint32 val = (1 << 31);
	val |= (Uint8)(19);
	val |= (((Uint16)(spellTimer->particleTimerDuration) & 0xFFF) << 8);
	val |= (Uint8)(spellTimer->particleTimerCountdownAction & 0xFF) << 20;
	spellTimer->skill[2] = val;
}

void Entity::gryphonChooseWeapon(const Entity* target, double dist)
{
	if ( monsterSpecialTimer != 0 )
	{
		return;
	}

	Stat* myStats = getStats();
	if ( !myStats )
	{
		return;
	}

	if ( monsterSpecialTimer == 0
		&& (ticks % 10 == 0) )
	{
		int timer = 0;
		if ( myStats->getAttribute("gryphon_state_delay") != "" )
		{
			timer = std::stoi(myStats->getAttribute("gryphon_state_delay"));
		}

		int idle_timer = 0;
		if ( myStats->getAttribute("gryphon_idle_delay") != "" )
		{
			idle_timer = std::stoi(myStats->getAttribute("gryphon_idle_delay"));
		}

		int mapx = (static_cast<int>(this->x) >> 4);
		int mapy = (static_cast<int>(this->y) >> 4);
		int index = (mapy)*MAPLAYERS + (mapx)*MAPLAYERS * map.height;
		bool mapTileValid = mapx > 0 && mapx < map.width - 1 && mapy > 0 && mapy < map.height - 1;

		if ( timer > 0 )
		{

		}
		else if ( monsterSpecialState == GRYPHON_SKYBOX )
		{
			if ( target && entityDist(this, const_cast<Entity*>(target)) < 64.0 )
			{
				if ( mapTileValid && !map.tiles[(MAPLAYERS - 1) + index] )
				{
					monsterSpecialState = GRYPHON_FLY;
					myStats->setAttribute("gryphon_state_delay", std::to_string(5 * TICKS_PER_SECOND));
					this->setEffect(EFF_ROOTED, true, 3 * TICKS_PER_SECOND, false);
				}
			}
		}
		else if ( monsterSpecialState == GRYPHON_FLY )
		{
			if ( target )
			{
				monsterSpecialState = GRYPHON_WALK;
				myStats->setAttribute("gryphon_state_delay", std::to_string(10 * TICKS_PER_SECOND));
			}
			else
			{
				if ( idle_timer == 0 && mapTileValid && !map.tiles[(MAPLAYERS - 1) + index] )
				{
					monsterSpecialState = GRYPHON_SKYBOX;
					myStats->setAttribute("gryphon_state_delay", std::to_string(5 * TICKS_PER_SECOND));
					this->setEffect(EFF_ROOTED, true, 3 * TICKS_PER_SECOND, false);
				}
			}
		}
		else if ( monsterSpecialState == GRYPHON_WALK )
		{
			monsterSpecialState = GRYPHON_FLY;
			myStats->setAttribute("gryphon_state_delay", std::to_string(5 * TICKS_PER_SECOND));
		}
	}
}

void gryphonAnimate(Entity* my, Stat* myStats, double dist)
{
	node_t* node;
	int bodypart = 0;
	int limbSpeedMultiplier = 1;
	my->flags[INVISIBLE] = true; // hide the "AI" bodypart

	// set invisibility //TODO: use isInvisible()?
	if ( multiplayer != CLIENT )
	{
		//my->z = limbs[WATER_ELEMENTAL][0][2];
		if ( !myStats->getEffectActive(EFF_LEVITATING) )
		{
			myStats->setEffectActive(EFF_LEVITATING, 1);
			myStats->EFFECTS_TIMERS[EFF_LEVITATING] = 0;
		}
		my->creatureHandleLiftZ();

		int charge = 0;
		if ( myStats->getAttribute("gryphon_charge") != "" )
		{
			charge = std::stoi(myStats->getAttribute("gryphon_charge"));
			--charge;
			charge = std::max(0, charge);
			myStats->setAttribute("gryphon_charge", std::to_string(charge));
		}

		if ( myStats->getAttribute("gryphon_state_delay") != "" )
		{
			int timer = std::stoi(myStats->getAttribute("gryphon_state_delay"));
			--timer;
			timer = std::max(0, timer);
			myStats->setAttribute("gryphon_state_delay", std::to_string(timer));
		}

		Entity* target = uidToEntity(my->monsterTarget);
		if ( target )
		{
			myStats->setAttribute("gryphon_idle_delay", std::to_string(5 * TICKS_PER_SECOND));
		}
		if ( myStats->getAttribute("gryphon_idle_delay") != "" )
		{
			int timer = std::stoi(myStats->getAttribute("gryphon_idle_delay"));
			--timer;
			timer = std::max(0, timer);
			myStats->setAttribute("gryphon_idle_delay", std::to_string(timer));
		}

		my->gryphonChooseWeapon(target, target ? entityDist(target, my) : 256.0);
		if ( target )
		{
			if ( !myStats->getEffectActive(EFF_FAST) 
				&& (my->monsterSpecialState == GRYPHON_WALK || my->monsterSpecialState == GRYPHON_SKYBOX || my->monsterSpecialState == GRYPHON_FLY) )
			{
				if ( charge == 0 )
				{
					my->setEffect(EFF_FAST, true, 3 * TICKS_PER_SECOND, false);
					if ( my->monsterSpecialState == GRYPHON_WALK )
					{
						charge = 5 * TICKS_PER_SECOND;
					}
					else
					{
						charge = 3 * TICKS_PER_SECOND;
					}
					myStats->setAttribute("gryphon_charge", std::to_string(charge));
				}
			}
		}
	}

	if ( enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
	{
		/*if ( keystatus[SDLK_KP_7] )
		{
			if ( keystatus[SDLK_LSHIFT] )
			{
				my->pitch -= 0.05;
			}
			else
			{
				my->pitch += 0.05;
			}
		}*/
		if ( keystatus[SDLK_KP_5] )
		{
			keystatus[SDLK_KP_5] = 0;
			if ( myStats->getEffectActive(EFF_STUNNED) )
			{
				myStats->clearEffect(EFF_STUNNED);
			}
			else
			{
				myStats->setEffectActive(EFF_STUNNED, true);
			}
		}
		if ( keystatus[SDLK_KP_4] )
		{
			keystatus[SDLK_KP_4] = 0;
			my->yaw = 0.0;
			my->monsterLookDir = my->yaw;
			my->pitch = 0.0;
		}
		if ( keystatus[SDLK_KP_6] )
		{
			my->yaw += 0.25;
			my->monsterLookDir = my->yaw;
		}
		if ( keystatus[SDLK_g] )
		{
			keystatus[SDLK_g] = 0;
			my->monsterAttack = MONSTER_POSE_MELEE_WINDUP1;
			my->monsterAttackTime = 0;
		}
	}

	static ConsoleVariable<int> cvar_gryphon_limb_rotate("/gryphon_limb_rotate", 0);
	static ConsoleVariable<int> cvar_gryphon_leg("/gryphon_leg", 0);
	static ConsoleVariable<int> cvar_gryphon_alt("/gryphon_alt", 0);
	if ( !*cvar_gryphon_alt )
	{
		my->sprite = 2430;
	}
	else
	{
		my->sprite = 2414;
	}

	Entity* body = nullptr;

	//Move bodyparts
	for (bodypart = 0, node = my->children.first; node != nullptr; node = node->next, bodypart++)
	{
		if ( bodypart < 2 )
		{
			continue;
		}
		Entity* entity = (Entity*)node->element;
		entity->x = my->x;
		entity->y = my->y;
		entity->z = my->z;
		entity->yaw = my->yaw;
		entity->pitch = my->pitch;
		entity->roll = my->roll;
		if ( bodypart != GRYPHON_BODY )
		{
			entity->yaw = body->yaw;
			if ( bodypart != GRYPHON_LEFTLEG && bodypart != GRYPHON_RIGHTLEG )
			{
				if ( bodypart == GRYPHON_HEAD )
				{
					entity->pitch += GRYPHON_DIVE_PITCH / 2;
				}
				else
				{
					entity->pitch += GRYPHON_DIVE_PITCH;
				}
			}
			else
			{
				entity->pitch -= GRYPHON_DIVE_PITCH / 4;
			}
		}

		if ( enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
		{
			if ( *cvar_gryphon_limb_rotate == bodypart || *cvar_gryphon_limb_rotate + 1 == bodypart )
			{
				entity->pitch = GRYPHON_LIMB_PITCH;
				if ( keystatus[SDLK_KP_1] )
				{
					GRYPHON_LIMB_PITCH = 0.0;
				}
				GRYPHON_LIMB_PITCH += 0.1;
				//GRYPHON_LIMB_ROLL += 0.1;
				//entity->roll = 0.25 * sin(GRYPHON_LIMB_ROLL);
				//
				//if ( keystatus[SDLK_KP_1] )
				//{
				//	GRYPHON_LIMB_PITCH = std::min(GRYPHON_LIMB_PITCH + 0.1, PI / 4);
				//}
				//if ( keystatus[SDLK_KP_3] )
				//{
				//	GRYPHON_LIMB_PITCH = std::max(GRYPHON_LIMB_PITCH - 0.1, -PI / 4);
				//}
				//entity->pitch = GRYPHON_LIMB_PITCH;
			}
		}

		if ( bodypart == GRYPHON_BODY )
		{
			body = entity;

			if ( my->monsterSpecialState == GRYPHON_SKYBOX )
			{
				if ( GRYPHON_STATE == 0 || GRYPHON_STATE == 2 )
				{
					GRYPHON_STATE = 1;
				}
				if ( GRYPHON_STATE == 1 && GRYPHON_DIVE_ANIM <= 0.0 )
				{
					if ( GRYPHON_FLY_ANIM >= 1.0 )
					{
						GRYPHON_STATE = 3;
					}
				}
			}
			else if ( my->monsterSpecialState == GRYPHON_FLY )
			{
				if ( GRYPHON_STATE == 3 )
				{
					if ( GRYPHON_FLY_ANIM >= 1.0 && GRYPHON_DIVE_ANIM >= 2.0 )
					{
						GRYPHON_STATE = 1;
					}
				}
				else
				{
					GRYPHON_STATE = 1;
				}
			}
			else if ( my->monsterSpecialState == GRYPHON_WALK )
			{
				if ( GRYPHON_STATE == 3 )
				{
					if ( GRYPHON_FLY_ANIM >= 1.0 && GRYPHON_DIVE_ANIM >= 2.0 )
					{
						GRYPHON_STATE = 1;
					}
				}
				else if ( GRYPHON_STATE == 1 && GRYPHON_DIVE_ANIM <= 0.0 )
				{
					if ( GRYPHON_FLY_ANIM >= 1.0 )
					{
						GRYPHON_STATE = 0;
					}
				}
			}
			else if ( my->monsterSpecialState == GRYPHON_STATE3 )
			{
				if ( GRYPHON_STATE == 0 || GRYPHON_STATE == 2 )
				{
					GRYPHON_STATE = 1;
				}
			}

			if ( uidToEntity(my->monsterTarget) && !myStats->getEffectActive(EFF_STUNNED) )
			{
				if ( GRYPHON_STATE == 0 )
				{
					if ( myStats->getEffectActive(EFF_FAST) && dist > 0.05 )
					{
						GRYPHON_STATE = 2;
					}
				}
				else if ( GRYPHON_STATE == 2 )
				{
					if ( !myStats->getEffectActive(EFF_FAST) )
					{
						GRYPHON_STATE = 0;
					}
				}
			}

			if ( enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
			{
				if ( keystatus[SDLK_KP_2] )
				{
					keystatus[SDLK_KP_2] = 0;
					myStats->setAttribute("gryphon_state_delay", "250");
					myStats->setAttribute("gryphon_idle_delay", "250");
					if ( my->monsterSpecialState == GRYPHON_SKYBOX )
					{
						my->monsterSpecialState = GRYPHON_FLY;
					}
					else if ( my->monsterSpecialState == GRYPHON_FLY )
					{
						my->monsterSpecialState = GRYPHON_WALK;
					}
					else if ( my->monsterSpecialState == GRYPHON_WALK )
					{
						my->monsterSpecialState = GRYPHON_FLY;
					}
				}
			}
			*cvar_gryphon_leg = (GRYPHON_STATE == 1 || GRYPHON_STATE == 3) ? 1 : 0;
			if ( enableDebugKeys && (svFlags & SV_FLAG_CHEATS) )
			{
				if ( keystatus[SDLK_KP_8] )
				{
					if ( keystatus[SDLK_LSHIFT] )
					{
						GRYPHON_WALK_MULT -= 0.05;
						GRYPHON_WALK_MULT = std::max(0.0, GRYPHON_WALK_MULT);
					}
					else
					{
						GRYPHON_WALK_MULT += 0.05;
						GRYPHON_WALK_MULT = std::min(1.0, GRYPHON_WALK_MULT);
					}
				}

			}
			if ( GRYPHON_STATE == 1 || GRYPHON_STATE == 3 )
			{
				if ( GRYPHON_STATE == 3 )
				{
					real_t prevAnim = GRYPHON_DIVE_ANIM;
					GRYPHON_DIVE_ANIM = std::min(2.0, GRYPHON_DIVE_ANIM + 0.025);
					if ( GRYPHON_DIVE_ANIM >= 0.65 && prevAnim < 0.65 )
					{
						createVortexFx(my, my->x, my->y);
					}

					if ( GRYPHON_DIVE_ANIM >= 1.0 && prevAnim < 1.0 )
					{
						gryphonCeilingBust(my);
					}
				}
				else
				{
					real_t prevAnim = GRYPHON_DIVE_ANIM;
					GRYPHON_DIVE_ANIM = std::max(0.0, GRYPHON_DIVE_ANIM - 0.025);
					if ( GRYPHON_DIVE_ANIM <= 0.65 && prevAnim > 0.65 )
					{
						createVortexFx(my, my->x, my->y);
					}

					if ( GRYPHON_DIVE_ANIM <= 1.0 && prevAnim > 1.0 )
					{
						gryphonCeilingBust(my);
					}
				}
				GRYPHON_STATE_ANIM = std::min(1.0, GRYPHON_STATE_ANIM + 0.05);
				GRYPHON_FLY_ANIM += 0.15;
				GRYPHON_LIMB_PITCH = -GRYPHON_STATE_ANIM * 6 * PI / 16;

				real_t ang = 0.75 * PI;
				if ( GRYPHON_STATE == 3 )
				{
					ang = 0.15 * PI;
				}
				if ( GRYPHON_DIVE_ANIM > 1.0 )
				{
					GRYPHON_DIVE_PITCH = GRYPHON_STATE_ANIM * ang * sin((1.0 -((1.0 - GRYPHON_DIVE_ANIM) / 1.0)) * PI / 2);
				}
				else if ( GRYPHON_DIVE_ANIM >= 0.5 && GRYPHON_DIVE_ANIM <= 1.0 )
				{
					GRYPHON_DIVE_PITCH = GRYPHON_STATE_ANIM * ang * cos(((1.0 - GRYPHON_DIVE_ANIM) / 0.5) * PI / 2);
				}
				else if ( GRYPHON_DIVE_ANIM > 1.0 )
				{
					GRYPHON_DIVE_PITCH = GRYPHON_STATE_ANIM * ang;
				}
				else
				{
					GRYPHON_DIVE_PITCH = 0.0;
				}
			}
			else
			{
				GRYPHON_STATE_ANIM = std::max(0.0, GRYPHON_STATE_ANIM - 0.05);
				GRYPHON_DIVE_ANIM = std::max(0.0, GRYPHON_DIVE_ANIM - 0.05);

				GRYPHON_LIMB_PITCH = -GRYPHON_STATE_ANIM * 6 * PI / 16;
				GRYPHON_DIVE_PITCH = 0.0;
				if ( GRYPHON_STATE == 2 )
				{
					GRYPHON_WALK_ANIM += 0.4;
				}
				else
				{
					GRYPHON_WALK_ANIM += 0.1;
				}
			}

			if ( my->monsterAttack == MONSTER_POSE_MELEE_WINDUP1 )
			{
				if ( my->monsterAttackTime == 0 )
				{
					GRYPHON_ATK_ANIM = 0.0;
					GRYPHON_ATK_X = 0.0;
				}

				GRYPHON_ATK_ANIM += 0.1;
				GRYPHON_ATK_ANIM = std::min(GRYPHON_ATK_ANIM, 1.0);

				GRYPHON_ATK_X = GRYPHON_ATK_ANIM * -2.0;
				if ( GRYPHON_ATK_ANIM >= 1.0 )
				{
					my->monsterAttack = 1;
					my->monsterAttackTime = 0;
				}
			}
			else if ( my->monsterAttack == 1 )
			{
				if ( my->monsterSpecialState == GRYPHON_FLY && multiplayer != CLIENT )
				{
					if ( my->monsterAttackTime == 1 )
					{
						Entity* target = uidToEntity(my->monsterTarget);
						if ( false && target && target->getStats() && !target->getStats()->getEffectActive(EFF_LIFT) )
						{
							CastSpellProps_t props;
							props.caster_x = my->x;
							props.caster_y = my->y;
							props.target_x = my->x + 32 * cos(my->yaw);
							props.target_y = my->y + 32 * sin(my->yaw);
							castSpell(my->getUID(), getSpellFromID(SPELL_SLAM), true, false, false, &props);
						}
						else
						{
							castSpell(my->getUID(), getSpellFromID(SPELL_FOCI_WINDBLAST), true, false, false, nullptr);
						}
					}
					else if ( my->monsterAttackTime % 7 == 1 )
					{
						castSpell(my->getUID(), getSpellFromID(SPELL_FOCI_WINDBLAST), true, false, false, nullptr);
					}
				}

				auto prevAnim = GRYPHON_ATK_ANIM;
				GRYPHON_ATK_ANIM += 0.1;

				if ( my->monsterSpecialState == GRYPHON_WALK && prevAnim < 2.0 && GRYPHON_ATK_ANIM >= 2.0 )
				{
					if ( multiplayer != CLIENT )
					{
						Sint32 tmp = my->monsterAttackTime;
						my->attack(1, 0, nullptr);
						my->monsterAttackTime = tmp;
					}
				}

				GRYPHON_ATK_ANIM = std::min(GRYPHON_ATK_ANIM, 4.0);

				GRYPHON_ATK_X = -2.0 - (1.0 - GRYPHON_ATK_ANIM) * 2.0 / 3;

				if ( GRYPHON_ATK_ANIM >= 4.0 )
				{
					my->monsterAttack = 0;
					my->monsterAttackTime = 0;
					GRYPHON_ATK_ANIM = 0.0;
				}
			}
			else
			{
				GRYPHON_ATK_X = 0.0;
			}

			if ( my->monsterAttack > 0 || (dist < 0.05 && !myStats->getEffectActive(EFF_STUNNED)) )
			{
				GRYPHON_WALK_MULT -= 0.05;
				GRYPHON_WALK_MULT = std::max(0.25, GRYPHON_WALK_MULT);
				if ( GRYPHON_WALK_MULT <= 0.25 )
				{
					if ( GRYPHON_STATE == 2 )
					{
						GRYPHON_STATE = 0;
					}
				}
			}
			else if ( dist > 0.05 && !myStats->getEffectActive(EFF_STUNNED) )
			{
				GRYPHON_WALK_MULT += 0.05;
				GRYPHON_WALK_MULT = std::min(1.0, GRYPHON_WALK_MULT);
			}

			GRYPHON_FLOAT_Z = -8 * sin(GRYPHON_STATE_ANIM * PI / 2);
			GRYPHON_FLOAT_Z += 1.0 * GRYPHON_STATE_ANIM * sin(GRYPHON_FLY_ANIM);
			if ( GRYPHON_STATE == 2 )
			{
				GRYPHON_FLOAT_Z += 1.0 * (1.0 - GRYPHON_STATE_ANIM) * GRYPHON_WALK_MULT * sin((GRYPHON_WALK_ANIM / 2) + PI / 2);
			}
			else if ( GRYPHON_STATE == 0 )
			{
				GRYPHON_FLOAT_Z += 0.25 * (1.0 - GRYPHON_STATE_ANIM) * GRYPHON_WALK_MULT * sin((GRYPHON_WALK_ANIM * 2) + PI / 2);
			}

			//GRYPHON_LIMB_YAW = GRYPHON_DIVE_ANIM * 2 * PI;
			GRYPHON_FLOAT_Z -= sin(GRYPHON_DIVE_ANIM * PI / 4) * 48.0;
			if ( GRYPHON_DIVE_ANIM >= 0.0 && GRYPHON_DIVE_ANIM < 1.0 )
			{
				GRYPHON_FLOAT_Z -= sin(((GRYPHON_DIVE_ANIM - 1.0) / 1.0) * PI) * 16.0;
			}
			else
			{
				GRYPHON_FLOAT_Z -= sin(((GRYPHON_DIVE_ANIM - 1.0) / 1.0) * PI) * 16.0;
			}
		}
		else if ( bodypart == GRYPHON_RIGHTWING || bodypart == GRYPHON_LEFTWING || bodypart == GRYPHON_TAIL )
		{
			entity->pitch = body->pitch;
		}

		int bodypartFocal = (bodypart - 1) + (bodypart - 2);
		int bodypartXYZ = bodypartFocal + 1;
		switch ( bodypart )
		{
			// torso
			case GRYPHON_BODY:
			{
				entity->sprite = my->sprite == 2414 ? 2415 : 2431;
				entity->pitch += GRYPHON_LIMB_PITCH;
				entity->pitch += GRYPHON_DIVE_PITCH;
				entity->yaw += GRYPHON_LIMB_YAW;

				real_t atkPitch = 0.0;
				if ( GRYPHON_ATK_ANIM <= 1.0 )
				{
					atkPitch += (PI / 16) * GRYPHON_ATK_ANIM;
				}
				else if ( GRYPHON_ATK_ANIM <= 1.5 )
				{
					atkPitch += (PI / 16);
				}
				else if ( GRYPHON_ATK_ANIM <= 2.0 )
				{
					atkPitch += (PI / 16) + (PI / 16) * (1.5 - GRYPHON_ATK_ANIM) / 0.5;
				}
				else if ( GRYPHON_ATK_ANIM <= 4.0 )
				{
					atkPitch = (-PI / 8) * sin((GRYPHON_ATK_ANIM - 2.0) * PI / 2);
				}

				if ( GRYPHON_STATE == 1 )
				{
					entity->pitch -= atkPitch;
				}
				else
				{
					entity->pitch += atkPitch;
				}
				GRYPHON_FLOAT_Z += 1.0 * atkPitch / (PI / 16);

				if ( GRYPHON_STATE == 2 )
				{
					entity->pitch += (PI / 32) * (1.0 - GRYPHON_STATE_ANIM) * GRYPHON_WALK_MULT * sin((GRYPHON_WALK_ANIM / 2) + PI);
				}
				else if ( GRYPHON_STATE == 0 )
				{
					entity->pitch += (PI / 64) * (1.0 - GRYPHON_STATE_ANIM) * GRYPHON_WALK_MULT * sin((GRYPHON_WALK_ANIM * 2) + PI);
				}

				entity->x += (GRYPHON_ATK_X * cos(entity->pitch) + limbs[GRYPHON][bodypartXYZ][0]) * cos(entity->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(entity->yaw + PI / 2);
				entity->y += (GRYPHON_ATK_X * cos(entity->pitch) + limbs[GRYPHON][bodypartXYZ][0]) * sin(entity->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(entity->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;

				if ( ticks % 5 == 0 && 
					((GRYPHON_STATE == 3 && GRYPHON_DIVE_ANIM > 0.5 && GRYPHON_DIVE_ANIM < 1.25)
					|| (GRYPHON_STATE == 1 && GRYPHON_DIVE_ANIM >= 0.5 && GRYPHON_DIVE_ANIM < 1.5 )))
				{
					for ( int i = 0; i < 8; ++i )
					{
						if ( Entity* fx = createParticleAOEIndicator(my, entity->z, entity->z, entity->z, TICKS_PER_SECOND * 5, 16 + (i / 2) * 2) )
						{
							fx->yaw = my->yaw + PI / 2 - (i / 2) * PI / 2;
							fx->pitch += PI / 32;
							if ( i % 2 == 1 )
							{
								fx->pitch += PI;
							}
							fx->z = entity->z + cos(entity->pitch) * 16.0;
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

				//Entity* particle = spawnMagicParticle(my);
				//particle->sprite = 576;
				//real_t offset = PI / 4 - 2 * PI / 32;
				//real_t dist = 8.5;
				//particle->x = entity->x - 0 * cos(entity->yaw + PI / 2) + dist * cos(entity->yaw) * sin(body->pitch - PI / 4 - offset);
				//particle->y = entity->y - 0 * sin(entity->yaw + PI / 2) + dist * sin(entity->yaw) * sin(body->pitch - PI / 4 - offset);
				//particle->z = entity->z + dist * sin(body->pitch - 3 * PI / 4 - offset);
				break;
			}
			case GRYPHON_HEAD:
			{
				entity->sprite = my->sprite == 2414 ? 2416 : 2432;
				entity->scalex = 1.05;
				entity->scaley = 1.05;
				entity->scalez = 1.05;

				entity->x += (GRYPHON_ATK_X + limbs[GRYPHON][bodypartXYZ][0]) * cos(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(body->yaw + PI / 2);
				entity->y += (GRYPHON_ATK_X + limbs[GRYPHON][bodypartXYZ][0]) * sin(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(body->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;

				real_t angOffset = GRYPHON_STATE_ANIM * 2 * PI / 8;
				if ( GRYPHON_STATE == 1 || GRYPHON_STATE == 3 )
				{
					angOffset += 3 * PI / 16 * sin(GRYPHON_ATK_ANIM * PI / 4);
				}
				entity->x += 3.25 * cos(body->yaw) * sin(body->pitch + PI / 4 - 1 * PI / 16 + angOffset);
				entity->y += 3.25 * sin(body->yaw) * sin(body->pitch + PI / 4 - 1 * PI / 16 + angOffset);
				entity->z += 3.25 * sin(body->pitch - PI / 4 - 1 * PI / 16 + angOffset);

				entity->pitch += GRYPHON_STATE_ANIM * PI / 16;
				entity->pitch += GRYPHON_STATE_ANIM * (PI / 32 + (PI / 32) * sin(GRYPHON_FLY_ANIM + PI / 2));

				if ( GRYPHON_ATK_ANIM >= 1.0 )
				{
					real_t atkPitch = sin(PI * (1.0 - GRYPHON_ATK_ANIM) / 3.0);
					entity->pitch += atkPitch * (-PI / 8);
					real_t dist = 1.0;
					if ( GRYPHON_STATE == 1 )
					{
						dist = 0.25;
					}
					entity->x += -dist * atkPitch * cos(body->yaw);
					entity->y += -dist * atkPitch * sin(body->yaw);
				}
				break;
			}
			case GRYPHON_LEFTLEG:
				entity->x += limbs[GRYPHON][bodypartXYZ][0] * cos(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(body->yaw + PI / 2);
				entity->y += limbs[GRYPHON][bodypartXYZ][0] * sin(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(body->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;

				entity->pitch += GRYPHON_STATE_ANIM * (PI / 32 + (PI / 32) * cos(GRYPHON_FLY_ANIM + PI / 2));

				entity->sprite = *cvar_gryphon_leg ? 2418 : 2417;
				if ( GRYPHON_STATE == 0 )
				{
					real_t walkAnim = GRYPHON_WALK_ANIM;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					if ( walkAng >= (8 * PI / 16) && walkAng < 20 * PI / 16 && GRYPHON_WALK_MULT > 0.25 )
					{
						entity->sprite = 2418;
					}

					if ( walkAng < PI )
					{
						real_t walkPitch = GRYPHON_WALK_MULT * (PI / 8) * sin(std::min(1.5 * PI, walkAng * 1.5));
						entity->pitch += walkPitch;
					}
					else if ( walkAng < 1.5 * PI )
					{
						real_t walkPitch = GRYPHON_WALK_MULT * -PI / 8;
						entity->pitch += walkPitch;
					}
					else if ( walkAng >= 1.5 * PI )
					{
						real_t walkPitch = GRYPHON_WALK_MULT * (PI / 8) * sin(walkAng);
						entity->pitch += walkPitch;
					}

					entity->x += GRYPHON_WALK_MULT * ((0.5 + 0.5 * sin(walkAng - PI / 4 - PI / 8))) * cos(body->yaw);
					entity->y += GRYPHON_WALK_MULT * ((0.5 + 0.5 * sin(walkAng - PI / 4 - PI / 8))) * sin(body->yaw);
				}
				else if ( GRYPHON_STATE == 2 )
				{
					real_t walkAnim = GRYPHON_WALK_ANIM * 1.25 - PI + PI / 2;
					real_t walkAng = fmodf(walkAnim, 4 * PI);

					real_t walkPitch = GRYPHON_WALK_MULT * (PI / 8) * sin(walkAng / 2);
					entity->pitch += walkPitch;

					entity->x += GRYPHON_WALK_MULT * sin(-walkAng / 2) * cos(body->yaw);
					entity->y += GRYPHON_WALK_MULT * sin(-walkAng / 2) * sin(body->yaw);
					entity->z += GRYPHON_WALK_MULT * cos(walkAng / 2);

					if ( (walkAng / 2) >= 2 * PI / 4 && (walkAng / 2) < 3 * PI / 2 && GRYPHON_WALK_MULT > 0.25 )
					{
						entity->sprite = 2418;
					}
				}
				if ( GRYPHON_DIVE_ANIM > 0.5 && GRYPHON_STATE == 1 )
				{
					entity->sprite = 2417;
				}

				if ( GRYPHON_ATK_ANIM > 1.0 )
				{
					entity->pitch += sin((PI) * (1.0 - std::min(3.0, GRYPHON_ATK_ANIM)) / 2.0) * PI / 2;
					if ( GRYPHON_ATK_ANIM < 3.5 )
					{
						entity->sprite = 2418;
					}
				}

				if ( my->sprite == 2430 )
				{
					entity->sprite += 16;
				}
				if ( entity->sprite == 2433 )
				{
					entity->focalx += 1.0;
					entity->focaly += 0.5;
				}
				if ( entity->sprite == 2418 || entity->sprite == 2434 )
				{
					if ( entity->sprite == 2434 )
					{
						entity->focaly -= 0.75;
					}
					entity->focalx += 1.0;
					entity->focalz -= 1.0;
				}
				break;
			case GRYPHON_RIGHTLEG:
				entity->x += limbs[GRYPHON][bodypartXYZ][0] * cos(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(body->yaw + PI / 2);
				entity->y += limbs[GRYPHON][bodypartXYZ][0] * sin(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(body->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;

				entity->pitch += GRYPHON_STATE_ANIM * (PI / 32 + (PI / 32) * sin(GRYPHON_FLY_ANIM + PI / 2));

				entity->sprite = *cvar_gryphon_leg ? 2420 : 2419;
				if ( GRYPHON_STATE == 0 )
				{
					real_t walkAnim = GRYPHON_WALK_ANIM + PI;
					real_t walkAng = fmodf(walkAnim, 2 * PI);
					if ( walkAng >= (8 * PI / 16) && walkAng < 20 * PI / 16 && GRYPHON_WALK_MULT > 0.25 )
					{
						entity->sprite = 2420;
					}

					if ( walkAng < PI )
					{
						real_t walkPitch = GRYPHON_WALK_MULT * (PI / 8) * sin(std::min(1.5 * PI, walkAng * 1.5));
						entity->pitch += walkPitch;
					}
					else if ( walkAng < 1.5 * PI )
					{
						real_t walkPitch = GRYPHON_WALK_MULT * -PI / 8;
						entity->pitch += walkPitch;
					}
					else if ( walkAng >= 1.5 * PI )
					{
						real_t walkPitch = GRYPHON_WALK_MULT * (PI / 8) * sin(walkAng);
						entity->pitch += walkPitch;
					}

					entity->x += GRYPHON_WALK_MULT * ((0.5 + 0.5 * sin(walkAng - PI / 4 - PI / 8))) * cos(body->yaw);
					entity->y += GRYPHON_WALK_MULT * ((0.5 + 0.5 * sin(walkAng - PI / 4 - PI / 8))) * sin(body->yaw);
				}
				else if ( GRYPHON_STATE == 2 )
				{
					real_t walkAnim = GRYPHON_WALK_ANIM * 1.25 - PI;
					real_t walkAng = fmodf(walkAnim, 4 * PI);

					real_t walkPitch = GRYPHON_WALK_MULT * (PI / 8) * sin(walkAng / 2);
					entity->pitch += walkPitch;

					entity->x += GRYPHON_WALK_MULT * 0.5 * sin(-walkAng / 2) * cos(body->yaw);
					entity->y += GRYPHON_WALK_MULT * 0.5 * sin(-walkAng / 2) * sin(body->yaw);
					entity->z += GRYPHON_WALK_MULT * 0.5 * cos(walkAng / 2);

					if ( (walkAng / 2) >= 2 * PI / 4 && (walkAng / 2) < 3 * PI / 2 && GRYPHON_WALK_MULT > 0.25 )
					{
						entity->sprite = 2420;
					}
				}
				if ( GRYPHON_DIVE_ANIM > 0.5 && GRYPHON_STATE == 1 )
				{
					entity->sprite = 2419;
				}

				if ( GRYPHON_ATK_ANIM > 1.0 )
				{
					entity->pitch += sin((PI) * (1.0 - std::min(3.0, GRYPHON_ATK_ANIM)) / 2.0) * PI / 2;
					if ( GRYPHON_ATK_ANIM < 3.5 )
					{
						entity->sprite = 2420;
					}
				}

				if ( my->sprite == 2430 )
				{
					entity->sprite += 16;
				}
				if ( entity->sprite == 2435 )
				{
					entity->focalx += 1.0;
					entity->focaly -= 0.5;
				}
				if ( entity->sprite == 2420 || entity->sprite == 2436 )
				{
					if ( entity->sprite == 2436 )
					{
						entity->focaly += 0.75;
					}
					entity->focalx += 1.0;
					entity->focalz -= 1.0;
				}
				break;
			case GRYPHON_LEFTHIND:
				entity->x += limbs[GRYPHON][bodypartXYZ][0] * cos(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(body->yaw + PI / 2);
				entity->y += limbs[GRYPHON][bodypartXYZ][0] * sin(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(body->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;

				entity->x += 6.25 * cos(body->yaw) * sin(body->pitch - PI / 2 - PI / 32);
				entity->y += 6.25 * sin(body->yaw) * sin(body->pitch - PI / 2 - PI / 32);
				entity->z += 6.25 * sin(body->pitch - PI - PI / 32);

				entity->pitch -= GRYPHON_STATE_ANIM * ((PI / 16) * sin(GRYPHON_FLY_ANIM + PI + PI / 4));
				entity->roll -= GRYPHON_STATE_ANIM * (PI / 64 + (PI / 64) * sin(GRYPHON_FLY_ANIM + PI / 4));

				entity->sprite = *cvar_gryphon_leg ? 2422 : 2421;
				if ( GRYPHON_STATE == 0 )
				{
					real_t walkAnim = GRYPHON_WALK_ANIM + PI / 2 + PI / 4;
					real_t walkAng = fmodf(walkAnim, 2 * PI);

					real_t walkPitch = GRYPHON_WALK_MULT * (PI / 8) * sin(std::min(2 * PI, walkAng * 2));
					entity->pitch += walkPitch;
					if ( walkPitch > PI / 32 && walkPitch < PI )
					{
						entity->sprite = 2422;
					}

					real_t dist = GRYPHON_WALK_MULT * (1.0 + 1.0 * sin(walkAnim + PI / 2));
					entity->x -= dist * cos(body->yaw);
					entity->y -= dist * sin(body->yaw);
				}
				else if ( GRYPHON_STATE == 2 )
				{
					real_t walkAnim = GRYPHON_WALK_ANIM * 1.25;
					real_t walkAng = fmodf(walkAnim, 4 * PI);

					if ( walkAng < 2 * PI )
					{
						real_t walkPitch = GRYPHON_WALK_MULT * PI / 4 * sin(std::min(PI / 2, walkAng));
						entity->pitch += walkPitch;

						if ( walkAng < 1.5 * PI && GRYPHON_WALK_MULT > 0.25 )
						{
							entity->sprite = 2422;
						}
						if ( walkAng < 1.25 * PI )
						{
							real_t dist = GRYPHON_WALK_MULT * (2.0 * sin(std::min(PI / 2, walkAng)));
							entity->x -= dist * cos(body->yaw);
							entity->y -= dist * sin(body->yaw);
						}
						else
						{
							real_t dist = GRYPHON_WALK_MULT * (2.0 - 2.0 * sin((PI / 2) * (walkAng - 1.25 * PI) / (0.75 * PI)));
							entity->x -= dist * cos(body->yaw);
							entity->y -= dist * sin(body->yaw);
						}
					}
					else
					{
						real_t walkPitch = GRYPHON_WALK_MULT * PI / 8 * sin(walkAng / 2);
						entity->pitch += walkPitch;

						real_t dist = GRYPHON_WALK_MULT * (1.0 * sin((walkAnim - 2 * PI) / 2));
						entity->x += dist * cos(body->yaw);
						entity->y += dist * sin(body->yaw);
					}
				}

				if ( my->sprite == 2430 )
				{
					entity->sprite += 16;
				}
				if ( entity->sprite == 2422 || entity->sprite == 2438 )
				{
					entity->focalx += 0.25;
					entity->focalz += 0.5;
					entity->focaly += 0.5;
					//entity->x += 0.5 * cos(body->yaw + PI / 2);
					//entity->y += 0.5 * sin(body->yaw + PI / 2);
				}
				break;
			case GRYPHON_RIGHTHIND:
				entity->x += limbs[GRYPHON][bodypartXYZ][0] * cos(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(body->yaw + PI / 2);
				entity->y += limbs[GRYPHON][bodypartXYZ][0] * sin(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(body->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;

				entity->x += 6.25 * cos(body->yaw) * sin(body->pitch - PI / 2 - PI / 32);
				entity->y += 6.25 * sin(body->yaw) * sin(body->pitch - PI / 2 - PI / 32);
				entity->z += 6.25 * sin(body->pitch - PI - PI / 32);

				entity->pitch += GRYPHON_STATE_ANIM * (PI / 16 + (PI / 16) * sin(GRYPHON_FLY_ANIM + PI + PI / 4));
				entity->roll += GRYPHON_STATE_ANIM * (PI / 64 + (PI / 64) * sin(GRYPHON_FLY_ANIM + PI + PI / 4));

				entity->sprite = *cvar_gryphon_leg ? 2424 : 2423;
				if ( GRYPHON_STATE == 0 )
				{
					real_t walkAnim = GRYPHON_WALK_ANIM + 3 * PI / 2 + PI / 4;
					real_t walkAng = fmodf(walkAnim, 2 * PI);

					real_t walkPitch = GRYPHON_WALK_MULT * (PI / 8) * sin(std::min(2 * PI, walkAng * 2));
					entity->pitch += walkPitch;
					if ( walkPitch > PI / 32 && walkPitch < PI )
					{
						entity->sprite = 2424;
					}

					real_t dist = GRYPHON_WALK_MULT * (1.0 + 1.0 * sin(walkAnim + PI / 2));
					entity->x -= dist * cos(body->yaw);
					entity->y -= dist * sin(body->yaw);
				}
				else if ( GRYPHON_STATE == 2 )
				{
					real_t walkAnim = GRYPHON_WALK_ANIM * 1.25 + PI / 2;
					real_t walkAng = fmodf(walkAnim, 4 * PI);

					if ( walkAng < 2 * PI )
					{
						real_t walkPitch = GRYPHON_WALK_MULT * PI / 4 * sin(std::min(PI / 2, walkAng));
						entity->pitch += walkPitch;

						if ( walkAng < 1.75 * PI && GRYPHON_WALK_MULT > 0.25 )
						{
							entity->sprite = 2424;
						}
						if ( walkAng < 1.5 * PI )
						{
							real_t dist = GRYPHON_WALK_MULT * (2.0 * sin(std::min(PI / 2, walkAng)));
							entity->x -= dist * cos(body->yaw);
							entity->y -= dist * sin(body->yaw);
						}
						else
						{
							real_t dist = GRYPHON_WALK_MULT * (2.0 - 2.0 * sin((PI / 2) * (walkAng - 1.5 * PI) / (0.5 * PI)));
							entity->x -= dist * cos(body->yaw);
							entity->y -= dist * sin(body->yaw);
						}
					}
					else
					{
						real_t walkPitch = GRYPHON_WALK_MULT * PI / 8 * sin(walkAng / 2);
						entity->pitch += walkPitch;

						real_t dist = GRYPHON_WALK_MULT * (1.0 * sin((walkAnim - 2 * PI) / 2));
						entity->x += dist * cos(body->yaw);
						entity->y += dist * sin(body->yaw);
					}
				}

				if ( my->sprite == 2430 )
				{
					entity->sprite += 16;
				}
				if ( entity->sprite == 2424 || entity->sprite == 2440 )
				{
					entity->focalx += 0.25;
					entity->focalz += 0.5;
					entity->focaly -= 0.5;
					//entity->x += -0.5 * cos(body->yaw + PI / 2);
					//entity->y += -0.5 * sin(body->yaw + PI / 2);
				}

				break;
			case GRYPHON_TAIL:
			{
				entity->sprite = my->sprite == 2414 ? 2425 : 2441;
				entity->scalex = 1.01;
				entity->scaley = 1.01;
				entity->scalez = 1.01;

				entity->x += limbs[GRYPHON][bodypartXYZ][0] * cos(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(body->yaw + PI / 2);
				entity->y += limbs[GRYPHON][bodypartXYZ][0] * sin(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(body->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;


				real_t ang = PI / 2 - 2 * PI / 32;
				if ( entity->sprite == 2441 )
				{
					ang += 1 * PI / 32;
					entity->focalz += 0.9;
					//entity->focalx += 0.25;
				}
				entity->x += 8.5 * cos(body->yaw) * sin(body->pitch - ang);
				entity->y += 8.5 * sin(body->yaw) * sin(body->pitch - ang);
				entity->z += 8.5 * sin(body->pitch - 2 * PI / 4 - ang);

				entity->pitch += GRYPHON_STATE_ANIM * (PI / 16 + (PI / 16) * sin(GRYPHON_FLY_ANIM + PI + PI / 4));
				entity->roll += GRYPHON_STATE_ANIM * ((PI / 64) * cos(GRYPHON_FLY_ANIM + PI + PI / 4));
				break;
			}
			case GRYPHON_LEFTWING:
			{
				entity->roll += GRYPHON_STATE_ANIM * 0.5 * sin(GRYPHON_FLY_ANIM + PI / 2);
				real_t flyPitch = GRYPHON_STATE_ANIM * (PI / 4 + (PI / 4) * cos(GRYPHON_FLY_ANIM + PI / 4 + PI));
				entity->pitch += flyPitch;

				//entity->roll -= GRYPHON_DIVE_ANIM * 2 * PI / 8;
				//entity->pitch += GRYPHON_DIVE_ANIM * PI / 2;

				if ( GRYPHON_STATE == 2 )
				{
					entity->roll += (PI / 16) * (1.0 - GRYPHON_STATE_ANIM) * GRYPHON_WALK_MULT* sin((GRYPHON_WALK_ANIM / 2) + PI);
				}
				else if ( GRYPHON_STATE == 0 )
				{
					entity->roll += (PI / 32) * (1.0 - GRYPHON_STATE_ANIM) * GRYPHON_WALK_MULT * sin((GRYPHON_WALK_ANIM * 2) + PI);
				}

				entity->x += limbs[GRYPHON][bodypartXYZ][0] * cos(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(body->yaw + PI / 2);
				entity->y += limbs[GRYPHON][bodypartXYZ][0] * sin(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(body->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;

				entity->sprite = *cvar_gryphon_leg ? 2427 : 2426;
				if ( my->sprite == 2430 )
				{
					entity->sprite += 16;
				}

				real_t pitch = body->pitch;
				if ( entity->sprite == 2427 || entity->sprite == 2443 )
				{
					real_t ang = PI / 32 - (PI / 4) * GRYPHON_STATE_ANIM;
					entity->x += 4.25 * cos(body->yaw) * sin(pitch - PI / 4 - ang);
					entity->y += 4.25 * sin(body->yaw) * sin(pitch - PI / 4 - ang);
					entity->z += 4.25 * sin(pitch - 3 * PI / 4 - ang);

					entity->x += 0.0 * cos(body->yaw + PI / 2);
					entity->y += 0.0 * sin(body->yaw + PI / 2);

					entity->focalx += 3.25;
					entity->focalz -= 1.75;
					entity->focalz += 0.5;
					entity->focaly -= 7.0;
				}
				else
				{
					entity->x += 4.75 * cos(body->yaw) * sin(pitch - PI / 4 - 3 * PI / 32);
					entity->y += 4.75 * sin(body->yaw) * sin(pitch - PI / 4 - 3 * PI / 32);
					entity->z += 4.75 * sin(pitch - 3 * PI / 4 - 3 * PI / 32);
				}
				break;
			}
			case GRYPHON_RIGHTWING:
			{
				entity->roll -= GRYPHON_STATE_ANIM * 0.5 * sin(GRYPHON_FLY_ANIM + PI / 2);
				real_t flyPitch = GRYPHON_STATE_ANIM * (PI / 4 + (PI / 4) * cos(GRYPHON_FLY_ANIM + PI / 4 + PI));
				entity->pitch += flyPitch;

				//entity->roll += GRYPHON_DIVE_ANIM * 2 * PI / 8;
				//entity->pitch += GRYPHON_DIVE_ANIM * PI / 2;

				if ( GRYPHON_STATE == 2 )
				{
					entity->roll -= (PI / 16) * (1.0 - GRYPHON_STATE_ANIM) * GRYPHON_WALK_MULT * sin((GRYPHON_WALK_ANIM / 2) + PI);
				}
				else if ( GRYPHON_STATE == 0 )
				{
					entity->roll -= (PI / 32) * (1.0 - GRYPHON_STATE_ANIM) * GRYPHON_WALK_MULT * sin((GRYPHON_WALK_ANIM * 2) + PI);
				}

				entity->x += limbs[GRYPHON][bodypartXYZ][0] * cos(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * cos(body->yaw + PI / 2);
				entity->y += limbs[GRYPHON][bodypartXYZ][0] * sin(body->yaw) + limbs[GRYPHON][bodypartXYZ][1] * sin(body->yaw + PI / 2);
				entity->z += limbs[GRYPHON][bodypartXYZ][2];
				entity->focalx = limbs[GRYPHON][bodypartFocal][0];
				entity->focaly = limbs[GRYPHON][bodypartFocal][1];
				entity->focalz = limbs[GRYPHON][bodypartFocal][2];
				entity->z += GRYPHON_FLOAT_Z;

				entity->sprite = *cvar_gryphon_leg ? 2429 : 2428;
				if ( my->sprite == 2430 )
				{
					entity->sprite += 16;
				}

				real_t pitch = body->pitch;
				if ( entity->sprite == 2429 || entity->sprite == 2445 )
				{
					real_t ang = PI / 32 - (PI / 4) * GRYPHON_STATE_ANIM;
					entity->x += 4.25 * cos(body->yaw) * sin(pitch - PI / 4 - ang);
					entity->y += 4.25 * sin(body->yaw) * sin(pitch - PI / 4 - ang);
					entity->z += 4.25 * sin(pitch - 3 * PI / 4 - ang);

					entity->x += 0.0 * cos(body->yaw + PI / 2);
					entity->y += 0.0 * sin(body->yaw + PI / 2);
					//entity->z -= 0.5 * cos(entity->pitch);

					entity->focalx += 3.25;
					entity->focalz -= 1.75;
					entity->focalz += 0.5;
					entity->focaly += 7.0;
				}
				else
				{
					entity->x += 4.75 * cos(body->yaw) * sin(pitch - PI / 4 - 3 * PI / 32);
					entity->y += 4.75 * sin(body->yaw) * sin(pitch - PI / 4 - 3 * PI / 32);
					entity->z += 4.75 * sin(pitch - 3 * PI / 4 - 3 * PI / 32);
				}
				break;
			}
			default:
				break;
		}
	}
	if ( MONSTER_ATTACK > 0 && MONSTER_ATTACK <= MONSTER_POSE_MAGIC_CAST3 )
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
