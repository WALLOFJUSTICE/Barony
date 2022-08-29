/*-------------------------------------------------------------------------------

	BARONY
	File: monster_shared.cpp
	Desc: contains shared monster implementation and helper functions

	Copyright 2013-2017 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "game.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "entity.hpp"
#include "prng.hpp"
#include "monster.hpp"

#include <cassert>

ConsoleVariable<bool> cvar_summonBosses("/summonbosses", false, "Always summon bosses");

void Entity::initMonster(int mySprite)
{
    if (multiplayer != CLIENT) {
	    sprite = mySprite;
		if ( Stat* myStats = this->getStats() )
		{
			if ( myStats->type != NOTHING )
			{
				int specialNPCModel = MonsterData_t::getSpecialNPCBaseModel(*myStats);
				if ( specialNPCModel != 0 )
				{
					sprite = specialNPCModel;
				}
			}
		}
	}

	//Common flags.
	flags[UPDATENEEDED] = true;
	flags[BLOCKSIGHT] = true;
	flags[INVISIBLE] = false;

	Monster monsterType = this->getMonsterTypeFromSprite();

    if (monsterType != SPIDER) {
	    focalx = limbs[monsterType][0][0];
	    focaly = limbs[monsterType][0][1];
	    focalz = limbs[monsterType][0][2];
	} else {
		if (arachnophobia_filter)
		{
		    focalx = limbs[CRAB][0][0];
		    focaly = limbs[CRAB][0][1];
		    focalz = limbs[CRAB][0][2];
		}
		else
		{
		    focalx = limbs[SPIDER][0][0];
		    focaly = limbs[SPIDER][0][1];
		    focalz = limbs[SPIDER][0][2];
		}
	}

	switch ( monsterType )
	{
		case GNOME:
			monsterFootstepType = MONSTER_FOOTSTEP_USE_BOOTS;
			monsterSpellAnimation = MONSTER_SPELLCAST_SMALL_HUMANOID;
			break;
		case KOBOLD:
			monsterFootstepType = MONSTER_FOOTSTEP_USE_BOOTS;
			monsterSpellAnimation = MONSTER_SPELLCAST_SMALL_HUMANOID;
			break;
		case HUMAN:
			monsterFootstepType = MONSTER_FOOTSTEP_USE_BOOTS;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case RAT:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case GOBLIN:
			monsterFootstepType = MONSTER_FOOTSTEP_USE_BOOTS;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case SLIME:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case TROLL:
			monsterFootstepType = MONSTER_FOOTSTEP_STOMP;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case OCTOPUS:
			// unused
			break;
		case SPIDER:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case GHOUL:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case SKELETON:
			monsterFootstepType = MONSTER_FOOTSTEP_SKELETON;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case SCORPION:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case CREATURE_IMP:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case CRAB:
			// unused
			break;
		case DEMON:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case SUCCUBUS:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case MIMIC:
			// unused
			break;
		case LICH:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case MINOTAUR:
			monsterFootstepType = MONSTER_FOOTSTEP_STOMP;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case DEVIL:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case SHOPKEEPER:
			monsterFootstepType = MONSTER_FOOTSTEP_LEATHER;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case SCARAB:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case CRYSTALGOLEM:
			monsterFootstepType = MONSTER_FOOTSTEP_STOMP;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case INCUBUS:
			monsterFootstepType = MONSTER_FOOTSTEP_LEATHER;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case VAMPIRE:
			monsterFootstepType = MONSTER_FOOTSTEP_LEATHER;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case SHADOW:
			monsterFootstepType = MONSTER_FOOTSTEP_USE_BOOTS;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case COCKATRICE:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		case INSECTOID:
			monsterFootstepType = MONSTER_FOOTSTEP_USE_BOOTS;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case GOATMAN:
			monsterFootstepType = MONSTER_FOOTSTEP_USE_BOOTS;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case AUTOMATON:
			monsterFootstepType = MONSTER_FOOTSTEP_USE_BOOTS;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case LICH_ICE:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case LICH_FIRE:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_HUMANOID;
			break;
		case SENTRYBOT:
		case SPELLBOT:
		case GYROBOT:
		case DUMMYBOT:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
		default:
			monsterFootstepType = MONSTER_FOOTSTEP_NONE;
			monsterSpellAnimation = MONSTER_SPELLCAST_NONE;
			break;
	}
	return;
}

Monster Entity::getMonsterTypeFromSprite() const
{
	Sint32 mySprite = this->sprite;
	return Entity::getMonsterTypeFromSprite(mySprite);
}

Monster Entity::getMonsterTypeFromSprite(const int sprite)
{
	Sint32 mySprite = sprite;
	static std::unordered_map<Sint32, Monster> spriteToMonster;
	if ( spriteToMonster.empty() ) {
		for ( int c = 0; c < NUMMONSTERS; ++c ) {
			const auto monster = static_cast<Monster>(c);
			for ( auto sprite : monsterSprites[c] ) {
				const auto result = spriteToMonster.emplace(sprite, monster);
				assert(result.second == true && "spriteToMonster conflict!");
			}
		}
	}
	else {
		auto find = spriteToMonster.find(mySprite);
		if ( find != spriteToMonster.end() ) {
			return find->second;
		}
	}
	return NOTHING;
}

void Entity::actMonsterLimb(bool processLight)
{
	//If no longer part of a monster, delete the limb.
	Entity *parentEnt = nullptr;
	if ( (parentEnt = uidToEntity(skill[2])) == nullptr )
	{
		if ( multiplayer == CLIENT )
		{
			if ( light )
			{
				list_RemoveNode(light->node);
				light = nullptr;
			}
		}
		list_RemoveNode(mynode);
		return;
	}

	//Do something magical beyond my comprehension.
	if ( multiplayer != CLIENT )
	{
		for ( int i = 0; i < MAXPLAYERS; ++i )
		{
			if ( inrange[i] )
			{
				if ( client_selected[i] == this || selectedEntity[i] == this )
				{
					parentEnt->skill[13] = i + 1;
				}
			}
		}
	}

	if ( processLight )
	{
		//Only run by monsters who can carry stuff (like torches). Sorry, rats.
		if ( light != nullptr )
		{
			list_RemoveNode(light->node);
			light = nullptr;
		}

		int carryingLightSource = 0;
		if ( flags[INVISIBLE] == false )
		{
			if ( sprite == 93 )   // torch
			{
				carryingLightSource = 6;
			}
			else if ( sprite == 94 )     // lantern
			{
				carryingLightSource = 9;
			}
			else if ( sprite == 529 )	// crystal shard
			{
				carryingLightSource = 4;
			}
		}

		if ( carryingLightSource != 0 )
		{
			light = lightSphereShadow(x / 16, y / 16, carryingLightSource, 50 + 15 * carryingLightSource);
		}
	}

	if ( parentEnt && parentEnt->behavior == &actMonster && parentEnt->monsterEntityRenderAsTelepath == 1 )
	{
		monsterEntityRenderAsTelepath = 1;
	}
	else
	{
		monsterEntityRenderAsTelepath = 0;
	}
}

void Entity::removeMonsterDeathNodes()
{
	removeLightField();
	int i = 0;
	node_t *nextnode = nullptr;
	for ( node_t *node = children.first; node != nullptr; node = nextnode )
	{
		nextnode = node->next;
		if ( node->element != nullptr && i >= 2 )
		{
			Entity* entity = (Entity*)node->element;
			if ( entity->light != nullptr )
			{
				list_RemoveNode(entity->light->node);
			}
			entity->light = nullptr;
			entity->flags[UPDATENEEDED] = false; //TODO: Do only demon & baphy need this?
			list_RemoveNode(entity->mynode);
		}
		list_RemoveNode(node);
		++i;
	}
}

void Entity::spawnBlood(int bloodSprite)
{
	if ( spawn_blood )
	{
		int tileX = std::min<unsigned int>(std::max<int>(0, this->x / 16), map.width - 1);
		int tileY = std::min<unsigned int>(std::max<int>(0, this->y / 16), map.height - 1);
		if ( map.tiles[tileY * MAPLAYERS + tileX * MAPLAYERS * map.height] )
		{
			if ( !checkObstacle(this->x, this->y, this, nullptr) )
			{
				Entity* entity = newEntity(bloodSprite, 1, map.entities, nullptr); //Blood/gib entity.
				entity->x = this->x;
				entity->y = this->y;
				entity->z = 8 + (local_rng.rand() % 20) / 100.0;
				entity->parent = getUID();
				entity->sizex = 2;
				entity->sizey = 2;
				entity->yaw = (local_rng.rand() % 360) * PI / 180.0;
				entity->flags[UPDATENEEDED] = true;
				entity->flags[PASSABLE] = true;
			}
		}
	}
}


MonsterData_t monsterData;
std::map<int, MonsterData_t::MonsterDataEntry_t> MonsterData_t::monsterDataEntries;
std::string MonsterData_t::iconDefaultString = "";
std::string& MonsterData_t::getAllyIconFromSprite(int sprite, int type)
{
	if ( type < NOTHING || type >= NUMMONSTERS )
	{
		type = Entity::getMonsterTypeFromSprite(sprite);
	}

	if ( type < NOTHING || type >= NUMMONSTERS )
	{
		return iconDefaultString;
	}

	return monsterDataEntries[type].iconSpritesAndPaths[sprite];
}

int MonsterData_t::getSpecialNPCBaseModel(Stat& myStats)
{
	std::string npcValue = myStats.getAttribute("special_npc");
	if ( npcValue != "" )
	{
		return monsterDataEntries[myStats.type].specialNPCs[npcValue].baseModel;
	}
	return 0;
}

std::string MonsterData_t::getSpecialNPCName(Stat& myStats)
{
	std::string npcValue = myStats.getAttribute("special_npc");
	if ( npcValue != "" )
	{
		return monsterDataEntries[myStats.type].specialNPCs[npcValue].name;
	}
	return "";
}