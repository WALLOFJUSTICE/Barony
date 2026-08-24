/*-------------------------------------------------------------------------------

	BARONY
	File: monster_goatman.cpp
	Desc: implements all of the goatman monster's code

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
#include "prng.hpp"
#include "scores.hpp"
#include "mod_tools.hpp"

real_t getNormalHeightMonsterS(Entity& my)
{
	return -1.25;
}

void initMonsterS(Entity* my, Stat* myStats)
{
	if ( !my )
	{
		return;
	}
	node_t* node;
	bool spawnedBoss = false;

	my->flags[BURNABLE] = false;
	my->initMonster(1536);
	my->z = getNormalHeightMonsterS(*my);

	if ( multiplayer != CLIENT )
	{
		MONSTER_SPOTSND = 853;
		MONSTER_SPOTVAR = 3;
		MONSTER_IDLESND = 850;
		MONSTER_IDLEVAR = 3;
	}

	if ( multiplayer != CLIENT && !MONSTER_INIT )
	{
		auto& rng = my->entity_rng ? *my->entity_rng : local_rng;

		if ( myStats != nullptr )
		{
			if ( myStats->sex == FEMALE )
			{
				my->sprite = 1537;
			}
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

			// generate the default inventory items for the monster, provided the editor sprite allowed enough default slots
			switch ( defaultItems )
			{
			case 6:
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
				break;
			default:
				break;
			}

			enum SalamanderType
			{
				SALAMANDER_RADIANT,
				SALAMANDER_ZEALOT
			};
			SalamanderType type = SALAMANDER_ZEALOT;

			if ( myStats->getAttribute("salamander_type") == "" )
			{
				if ( strstr(myStats->name, "priest")
					|| strstr(myStats->name, "acolyte")
					|| strstr(myStats->name, "scout")
					|| strstr(myStats->name, "regent") )
				{
					myStats->setAttribute("salamander_type", "radiant");
				}
				else if ( strstr(myStats->name, "guard")
					|| strstr(myStats->name, "sergeant")
					|| strstr(myStats->name, "squire") )
				{
					myStats->setAttribute("salamander_type", "zealot");
				}
				else if ( rng.rand() % 4 == 0 )
				{
					myStats->setAttribute("salamander_type", "radiant");
				}
				else
				{
					myStats->setAttribute("salamander_type", "zealot");

					if ( myStats->leader_uid == 0 && !my->flags[USERFLAG2] && !strcmp(myStats->name, "")
						&& rng.rand() % 2 == 0 )
					{
						Entity* entity = summonMonster(SALAMANDER, my->x, my->y);
						if ( entity )
						{
							entity->parent = my->getUID();
							if ( Stat* followerStats = entity->getStats() )
							{
								followerStats->leader_uid = entity->parent;
								followerStats->setAttribute("salamander_type", "radiant");
							}
							my->parent = entity->getUID(); // so I know my ally
							entity->seedEntityRNG(rng.getU32());
						}
					}
				}
			}

			if ( myStats->getAttribute("salamander_type").find("radiant") != std::string::npos )
			{
				type = SALAMANDER_RADIANT;
			}

			//give weapon
			if ( myStats->weapon == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_WEAPON] == 1 )
			{
				if ( type == SALAMANDER_RADIANT )
				{
					myStats->weapon = newItem(SILVER_GLAIVE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
				else
				{
					switch ( rng.rand() % 4 )
					{
					case 0:
						myStats->weapon = newItem(SILVER_AXE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 1:
						myStats->weapon = newItem(SILVER_AXE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 2:
						myStats->weapon = newItem(SILVER_SWORD, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 3:
						myStats->weapon = newItem(SILVER_MACE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					default:
						break;
					}
				}
			}

			//give shield
			if ( myStats->shield == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_SHIELD] == 1 )
			{
				if ( myStats->weapon && isRangedWeapon(*myStats->weapon) )
				{
					my->monsterGenerateQuiverItem(myStats);
				}
				else
				{
					if ( rng.rand() % 5 == 0 || (type == SALAMANDER_ZEALOT) )
					{
						myStats->shield = newItem(SILVER_SHIELD, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
					}
				}
			}

			// give helmet
			if ( myStats->helmet == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_HELM] == 1 )
			{
				if ( type == SALAMANDER_ZEALOT )
				{
					switch ( rng.rand() % 3 )
					{
					case 0:
						break;
					case 1:
						myStats->helmet = newItem(CHAIN_COIF, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 2:
						myStats->helmet = newItem(SILVER_HELM, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					}
				}
			}

			// give cloak
			/*if ( myStats->cloak == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_CLOAK] == 1 )
			{
				if ( type == SALAMANDER_ZEALOT )
				{
					switch ( rng.rand() % 10 )
					{
						case 0:
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
							break;
						case 6:
						case 7:
						case 8:
						case 9:
							myStats->cloak = newItem(CLOAK, WORN, 0, 1, rng.rand(), false, nullptr);
							break;
					}
				}
			}*/

			// give breast
			if ( myStats->breastplate == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_ARMOR] == 1 )
			{
				if ( type == SALAMANDER_ZEALOT )
				{
					switch ( rng.rand() % 4 )
					{
					case 0:
					case 1:
						break;
					case 2:
						myStats->breastplate = newItem(CHAIN_HAUBERK, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 3:
						myStats->breastplate = newItem(SILVER_BREASTPIECE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					}
				}
			}

			// give gloves
			if ( myStats->gloves == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_GLOVES] == 1 )
			{
				switch ( rng.rand() % 3 )
				{
				case 0:
					break;
				case 1:
					myStats->gloves = newItem(CHAIN_GLOVES, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
					break;
				case 2:
					myStats->gloves = newItem(SILVER_GAUNTLETS, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
					break;
				}
			}

			// give boots
			if ( myStats->shoes == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_BOOTS] == 1 )
			{
				switch ( rng.rand() % 3 )
				{
				case 0:
					break;
				case 1:
					myStats->shoes = newItem(CHAIN_BOOTS, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
					break;
				case 2:
					myStats->shoes = newItem(SILVER_BOOTS, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
					break;
				}
			}
		}
	}

	// torso
	const int torso_sprite = (my->sprite == 1536 || my->sprite == 1537) ? 1560 : 
		(my->sprite == 1538 || my->sprite == 1539) ? 1561 : 1562;
	Entity* entity = newEntity(torso_sprite, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->scalex = 1.01;
	entity->scaley = 1.01;
	entity->scalez = 1.01;
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[SALAMANDER][1][0]; // 0
	entity->focaly = limbs[SALAMANDER][1][1]; // 0
	entity->focalz = limbs[SALAMANDER][1][2]; // 0
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// right leg
	entity = newEntity((my->sprite == 1536 || my->sprite == 1537) ? 1555 :
		(my->sprite == 1538 || my->sprite == 1539) ? 1557 : 1559, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[SALAMANDER][2][0]; // 0
	entity->focaly = limbs[SALAMANDER][2][1]; // 0
	entity->focalz = limbs[SALAMANDER][2][2]; // 2
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// left leg
	entity = newEntity((my->sprite == 1536 || my->sprite == 1537) ? 1554 :
		(my->sprite == 1538 || my->sprite == 1539) ? 1556 : 1558, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[SALAMANDER][3][0]; // 0
	entity->focaly = limbs[SALAMANDER][3][1]; // 0
	entity->focalz = limbs[SALAMANDER][3][2]; // 2
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// right arm
	entity = newEntity((my->sprite == 1536 || my->sprite == 1537) ? 1544 :
		(my->sprite == 1538 || my->sprite == 1539) ? 1548 : 1552, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[SALAMANDER][4][0]; // 0
	entity->focaly = limbs[SALAMANDER][4][1]; // 0
	entity->focalz = limbs[SALAMANDER][4][2]; // 1.5
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// left arm
	entity = newEntity((my->sprite == 1536 || my->sprite == 1537) ? 1542 :
		(my->sprite == 1538 || my->sprite == 1539) ? 1546 : 1550, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[SALAMANDER][5][0]; // 0
	entity->focaly = limbs[SALAMANDER][5][1]; // 0
	entity->focalz = limbs[SALAMANDER][5][2]; // 1.5
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// world weapon
	entity = newEntity(-1, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[SALAMANDER][6][0]; // 1.5
	entity->focaly = limbs[SALAMANDER][6][1]; // 0
	entity->focalz = limbs[SALAMANDER][6][2]; // -.5
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	entity->pitch = .25;
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// shield
	entity = newEntity(-1, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[SALAMANDER][7][0]; // 2
	entity->focaly = limbs[SALAMANDER][7][1]; // 0
	entity->focalz = limbs[SALAMANDER][7][2]; // 0
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// cloak
	entity = newEntity(-1, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[SALAMANDER][8][0]; // 0
	entity->focaly = limbs[SALAMANDER][8][1]; // 0
	entity->focalz = limbs[SALAMANDER][8][2]; // 4
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// helmet
	entity = newEntity(-1, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->scalex = 1.01;
	entity->scaley = 1.01;
	entity->scalez = 1.01;
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[SALAMANDER][9][0]; // 0
	entity->focaly = limbs[SALAMANDER][9][1]; // 0
	entity->focalz = limbs[SALAMANDER][9][2]; // -2
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// mask
	entity = newEntity(-1, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[SALAMANDER][10][0]; // 0
	entity->focaly = limbs[SALAMANDER][10][1]; // 0
	entity->focalz = limbs[SALAMANDER][10][2]; // .25
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// tail
	entity = newEntity(-1, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[SALAMANDER][11][0];
	entity->focaly = limbs[SALAMANDER][11][1];
	entity->focalz = limbs[SALAMANDER][11][2];
	entity->behavior = &actMonsterSLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	if ( multiplayer == CLIENT || MONSTER_INIT )
	{
		return;
	}
}

void actMonsterSLimb(Entity* my)
{
	my->actMonsterLimb(true);
}

void monsterSDie(Entity* my)
{
	Entity* gib = spawnGib(my);
	gib->skill[5] = 1; // poof
	gib->sprite = my->sprite;
	serverSpawnGibForClient(gib);
	for ( int c = 0; c < 8; c++ )
	{
		Entity* gib = spawnGib(my);
		serverSpawnGibForClient(gib);
	}

	my->removeLightField();
	my->spawnBlood();

	playSoundEntity(my, 856 + local_rng.rand() % 4, 128);

	my->removeMonsterDeathNodes();

	list_RemoveNode(my->mynode);
	return;
}

#define MONSTER_SWALKSPEED .13

void Entity::salamanderChooseWeapon(const Entity* target, double dist)
{
	Stat* myStats = getStats();
	if ( !myStats )
	{
		return;
	}

	if ( monsterSpecialState == SALAMANDER_STRAFE )
	{
		if ( monsterStrafeDirection == 0 && local_rng.rand() % 10 == 0 && ticks % 10 == 0 )
		{
			setBugbearStrafeDir(true);
			//monsterStrafeDirection = -1 + ((local_rng.rand() % 2 == 0) ? 2 : 0);
		}
	}

	if ( monsterSpecialState != 0 || monsterSpecialTimer != 0 /*|| monsterAttack != 0*/ )
	{
		if ( monsterSpecialTimer < MONSTER_SPECIAL_COOLDOWN_SALAMANDER_FLAMES / 2 )
		{
			if ( monsterStrafeDirection != 0 )
			{
				if ( monsterAttack == 0 )
				{
					monsterSpecialState = 0;
					monsterStrafeDirection = 0;
				}
			}
		}
		return;
	}

	int roll = 4;
	/*if ( target && target->hasRangedWeapon() && dist > TOUCHRANGE * 1.5 )
	{
		roll = 2;
	}*/

	if ( monsterSpecialTimer == 0
		&& (ticks % 10 == 0) )
	{

		int specialRoll = -1;
		int bonusFromHP = 0;
		specialRoll = local_rng.rand() % 40;
		if ( myStats->HP <= myStats->MAXHP * 0.8 )
		{
			bonusFromHP += 2; // +% chance if on low health
		}
		if ( myStats->HP <= myStats->MAXHP * 0.4 )
		{
			bonusFromHP += 3; // +extra % chance if on lower health
		}

		int requiredRoll = (2 + bonusFromHP);

		if ( dist < STRIKERANGE )
		{
			requiredRoll += 5;
		}

		bool strafeChance = true;

		if ( specialRoll < requiredRoll )
		{
			Entity* leader = nullptr;
			if ( myStats->leader_uid != 0 )
			{
				leader = uidToEntity(myStats->leader_uid);
			}
			else if ( parent != 0 )
			{
				leader = uidToEntity(parent);
			}

			if ( leader && leader->monsterSpecialState == SALAMANDER_STRAFE )
			{
				if ( local_rng.rand() % 2 != 0 )
				{
					strafeChance = false;
				}
			}

			if ( !(dist < STRIKERANGE * 2 || hasRangedWeapon()) )
			{
				strafeChance = false;
			}

			bool setStrafe = false;
			if ( myStats->getAttribute("salamander_type") == "zealot" )
			{
				int chance = 0;
				if ( !myStats->getEffectActive(EFF_SALAMANDER_HEART) && myStats->EFFECTS_TIMERS[EFF_SALAMANDER_HEART] == 0 )
				{
					real_t hp = (myStats->HP / (real_t)myStats->MAXHP);
					if ( hp < 0.6 )
					{
						chance = 1;
					}
					else if ( hp < 0.8 )
					{
						chance = 2;
					}
					else
					{
						chance = 2;
					}
				}
				if ( chance && local_rng.rand() % chance == 0 )
				{
					monsterSpecialState = SALAMANDER_CAST;
					strafeChance = false;
				}
			}

			if ( strafeChance )
			{
				setStrafe = true;
				monsterSpecialState = SALAMANDER_STRAFE;
				monsterSpecialTimer = MONSTER_SPECIAL_COOLDOWN_SALAMANDER_FLAMES;
			}

			if ( setStrafe )
			{
				if ( leader )
				{
					if ( leader->monsterStrafeDirection != 0 )
					{
						if ( local_rng.rand() % 2 == 0 )
						{
							monsterStrafeDirection = 0;
							return;
						}
					}
				}
				setBugbearStrafeDir(true);

				if ( myStats->getAttribute("salamander_type") == "radiant" )
				{
					if ( !myStats->getEffectActive(EFF_SALAMANDER_HEART) && myStats->EFFECTS_TIMERS[EFF_SALAMANDER_HEART] == 0 )
					{
						setEffect(EFF_SALAMANDER_HEART, (Uint8)2, 20 * TICKS_PER_SECOND, true, true, true);
						castSpell(getUID(), getSpellFromID(SPELL_IGNITE), true, false, false);
						playSoundEntity(this, 167, 128);
					}
				}
			}
		}
	}

	if ( myStats->type == SALAMANDER )
	{
		if ( myStats->getAttribute("salamander_type") == "zealot" && monsterSpecialState == SALAMANDER_CAST )
		{
			setEffect(EFF_SALAMANDER_HEART, (Uint8)4, 5 * TICKS_PER_SECOND, true, true, true);
		}
	}
}

void monsterSMoveBodyparts(Entity* my, Stat* myStats, double dist)
{
	node_t* node;
	Entity* entity = nullptr, * entity2 = nullptr;
	Entity* additionalLimb = nullptr;
	Entity* rightbody = nullptr;
	Entity* weaponarm = nullptr;
	int bodypart;
	bool wearingring = false;

	my->focalx = limbs[SALAMANDER][0][0];
	my->focaly = limbs[SALAMANDER][0][1];
	my->focalz = limbs[SALAMANDER][0][2];

	/*if ( keystatus[SDLK_g] )
	{
		keystatus[SDLK_g] = 0;
		my->sprite += 1;
		if ( my->sprite > 1541 )
		{
			my->sprite = 1536;
		}
	}
	if ( keystatus[SDLK_h] )
	{
		keystatus[SDLK_h] = 0;
		my->setEffect(EFF_SALAMANDER_HEART, (Uint8)1, 5 * TICKS_PER_SECOND, true, true, true);
		castSpell(my->getUID(), getSpellFromID(SPELL_IGNITE), true, false, false);
		playSoundEntity(my, 167, 128);
	}
	if ( keystatus[SDLK_j] )
	{
		keystatus[SDLK_j] = 0;
		my->attack(MONSTER_POSE_RANGED_WINDUP3, 0, nullptr);
	}*/

	my->removeLightField();

	bool debugModel = monsterDebugModels(my, &dist);

	// set invisibility //TODO: isInvisible()?
	if ( multiplayer != CLIENT )
	{
		if ( myStats->ring != nullptr )
			if ( myStats->ring->type == RING_INVISIBILITY )
			{
				wearingring = true;
			}
		if ( myStats->cloak != nullptr )
			if ( myStats->cloak->type == CLOAK_INVISIBILITY )
			{
				wearingring = true;
			}
		if ( myStats->getEffectActive(EFF_INVISIBLE) || wearingring == true )
		{
			my->flags[INVISIBLE] = true;
			my->flags[BLOCKSIGHT] = false;
			bodypart = 0;
			for ( node = my->children.first; node != nullptr; node = node->next )
			{
				if ( bodypart < LIMB_HUMANOID_TORSO )
				{
					bodypart++;
					continue;
				}
				if ( bodypart >= 7 )
				{
					break;
				}
				entity = (Entity*)node->element;
				if ( !entity->flags[INVISIBLE] )
				{
					entity->flags[INVISIBLE] = true;
					serverUpdateEntityBodypart(my, bodypart);
				}
				bodypart++;
			}
		}
		else
		{
			my->flags[INVISIBLE] = false;
			my->flags[BLOCKSIGHT] = true;
			bodypart = 0;
			for ( node = my->children.first; node != nullptr; node = node->next )
			{
				if ( bodypart < LIMB_HUMANOID_TORSO )
				{
					bodypart++;
					continue;
				}
				if ( bodypart >= 7 )
				{
					break;
				}
				entity = (Entity*)node->element;
				if ( entity->flags[INVISIBLE] )
				{
					entity->flags[INVISIBLE] = false;
					serverUpdateEntityBodypart(my, bodypart);
					serverUpdateEntityFlag(my, INVISIBLE);
				}
				bodypart++;
			}
		}

		// sleeping
		if ( myStats->getEffectActive(EFF_ASLEEP) )
		{
			my->z = 3.0;
			my->pitch = PI / 4;
		}
		else
		{
			my->z = getNormalHeightMonsterS(*my);

			if ( my->monsterAttack == 0 )
			{
				if ( debugModel )
				{
					my->pitch = my->fskill[0];
					if ( my->fskill[1] > 0.0 )
					{
						my->fskill[1] = std::max(0.0, my->fskill[1] - 0.05);
						my->z += -3.0 * sqrt(sin(PI * my->fskill[1]));
					}
				}
				else
				{
					my->pitch = 0;
				}
			}
		}

		my->creatureHandleLiftZ();
	}

	bool hovering = false;
	if ( (myStats && myStats->getEffectActive(EFF_SALAMANDER_HEART) >= 1 && myStats->getEffectActive(EFF_SALAMANDER_HEART) <= 2)
		|| (my->sprite == 1538 || my->sprite == 1539) )
	{
		hovering = true;
		my->light = addLight(my->x / 16, my->y / 16, "magic_foci_idle_red");
	}

	if ( multiplayer != CLIENT )
	{
		int particle = 0;
		// radiant -> stone
		//particle = 1;
		//playSoundEntity(this, 826, 128);
		if ( (myStats && myStats->getEffectActive(EFF_SALAMANDER_HEART) >= 1 && myStats->getEffectActive(EFF_SALAMANDER_HEART) <= 2) )
		{
			if ( my->sprite == 1536 || my->sprite == 1540 )
			{
				my->sprite = 1538;
				//particle = 2;
			}
			else if ( my->sprite == 1537 || my->sprite == 1541 )
			{
				my->sprite = 1539;
				//particle = 2;
			}
		}
		else if ( (myStats && myStats->getEffectActive(EFF_SALAMANDER_HEART) >= 3 && myStats->getEffectActive(EFF_SALAMANDER_HEART) <= 4) )
		{
			if ( my->sprite == 1536 || my->sprite == 1538 )
			{
				my->sprite = 1540;
				particle = 1;
				playSoundEntity(my, 826, 128);
			}
			else if ( my->sprite == 1537 || my->sprite == 1539 )
			{
				my->sprite = 1541;
				particle = 1;
				playSoundEntity(my, 826, 128);
			}
		}
		else
		{
			if ( my->sprite == 1538 || my->sprite == 1540 )
			{
				my->sprite = 1536;
				particle = 2;
				playSoundEntity(my, 827, 128);
			}
			else if ( my->sprite == 1539 || my->sprite == 1541 )
			{
				my->sprite = 1537;
				particle = 2;
				playSoundEntity(my, 827, 128);
			}
		}

		if ( particle )
		{
			for ( int i = 0; i < 2; ++i )
			{
				if ( Entity* fx = createParticleAestheticOrbit(my, 263, TICKS_PER_SECOND / 2, PARTICLE_EFFECT_HEAT_ORBIT_SPIN) )
				{
					fx->flags[SPRITE] = true;
					fx->x = my->x;
					fx->y = my->y;
					fx->z = 7.5;
					fx->fskill[0] = fx->x;
					fx->fskill[1] = fx->y;
					fx->vel_z = -0.5;
					fx->actmagicOrbitDist = 5;
					fx->fskill[2] = my->yaw + PI / 4.0 + i * PI;
					fx->yaw = fx->fskill[2];
					fx->fskill[4] = 0.25;
					if ( particle == 1 )
					{
						fx->lightBonus = vec4{ 0.f, 0.f, 0.f, 0.f };
						fx->actmagicNoLight = 1;
					}

				}
			}
			serverSpawnMiscParticles(my, PARTICLE_EFFECT_HEAT_ORBIT_SPIN, 263, particle, TICKS_PER_SECOND / 2);
		}
	}

	Entity* shieldarm = nullptr;
	Entity* helmet = nullptr;

	//Move bodyparts
	for ( bodypart = 0, node = my->children.first; node != nullptr; node = node->next, bodypart++ )
	{
		if ( bodypart < LIMB_HUMANOID_TORSO )
		{
			// post-swing head animation. client doesn't need to adjust the entity pitch, server will handle.
			if ( my->monsterAttack != MONSTER_POSE_RANGED_WINDUP3 && bodypart == 1 && multiplayer != CLIENT )
			{
				limbAnimateToLimit(my, ANIMATE_PITCH, 0.1, 0, false, 0.0);
			}
			continue;
		}
		entity = (Entity*)node->element;
		entity->x = my->x;
		entity->y = my->y;
		entity->z = my->z;
		if ( MONSTER_ATTACK == MONSTER_POSE_MAGIC_WINDUP1 && bodypart == LIMB_HUMANOID_RIGHTARM )
		{
			// don't let the creatures's yaw move the casting arm
		}
		else
		{
			entity->yaw = my->yaw;
		}

		if ( bodypart == LIMB_HUMANOID_RIGHTLEG || bodypart == LIMB_HUMANOID_LEFTARM )
		{
			if ( bodypart == LIMB_HUMANOID_RIGHTLEG )
			{
				rightbody = (Entity*)node->next->element;
			}
			if ( hovering )
			{
				// hands stationary, legs pitched back and little swing.
				real_t limbSpeed = 0.03;
				if ( bodypart == LIMB_HUMANOID_LEFTARM ) // left arm
				{
					if ( entity->pitch < 0 )
					{
						entity->pitch += 1 / fmax(limbSpeed * .1, 10.0);
						if ( entity->pitch > 0 )
						{
							entity->pitch = 0;
						}
					}
					else if ( entity->pitch > 0 )
					{
						entity->pitch -= 1 / fmax(limbSpeed * .1, 10.0);
						if ( entity->pitch < 0 )
						{
							entity->pitch = 0;
						}
					}
				}
				else if ( bodypart == LIMB_HUMANOID_RIGHTLEG )
				{
					if ( entity->pitch < 0 )
					{
						entity->pitch += 5 * limbSpeed * MONSTER_SWALKSPEED; // speed up to reach target.
					}
					if ( !rightbody->skill[3] )
					{
						entity->pitch -= limbSpeed * MONSTER_SWALKSPEED;
						if ( entity->pitch < PI / 6.f )
						{
							entity->pitch = PI / 6.f;
						}
					}
					else
					{
						entity->pitch += limbSpeed * MONSTER_SWALKSPEED;
						if ( entity->pitch > PI / 3.f )
						{
							entity->pitch = PI / 3.f;
						}
					}
				}
			}
			else
			{
				my->humanoidAnimateWalk(entity, node, bodypart, MONSTER_SWALKSPEED, dist, 0.4);
			}
		}
		else if ( bodypart == LIMB_HUMANOID_LEFTLEG || bodypart == LIMB_HUMANOID_RIGHTARM || bodypart == LIMB_HUMANOID_CLOAK )
		{
			// left leg, right arm, cloak.
			if ( bodypart == LIMB_HUMANOID_RIGHTARM )
			{
				weaponarm = entity;
				if ( MONSTER_ATTACK > 0 )
				{
					if ( my->monsterAttack == MONSTER_POSE_RANGED_WINDUP3 )
					{
						Entity* rightbody = nullptr;
						// set rightbody to left leg.
						node_t* rightbodyNode = list_Node(&my->children, LIMB_HUMANOID_LEFTLEG);
						if ( rightbodyNode )
						{
							rightbody = (Entity*)rightbodyNode->element;
						}
						else
						{
							return;
						}

						if ( my->monsterAttackTime == 0 )
						{
							// init rotations
							weaponarm->pitch = 0;
							my->monsterArmbended = 0;
							my->monsterWeaponYaw = 0;
							weaponarm->roll = 0;
							weaponarm->skill[1] = 0;
							playSoundEntityLocal(my, 170, 32);
							createParticleDot(my);
							if ( multiplayer != CLIENT )
							{
								//myStats->setEffectActive(EFF_ROOTED, 1);
								//myStats->EFFECTS_TIMERS[EFF_ROOTED] = 40;
							}
						}
						if ( multiplayer != CLIENT )
						{
							// move the head and weapon yaw
							limbAnimateToLimit(my, ANIMATE_PITCH, -0.1, 11 * PI / 6, true, 0.05);
							limbAnimateToLimit(my, ANIMATE_WEAPON_YAW, 0.05, 2 * PI / 8, false, 0.0);
						}
						limbAnimateToLimit(weaponarm, ANIMATE_PITCH, -0.25, 7 * PI / 4, true, 0.0);
						//limbAnimateToLimit(weaponarm, ANIMATE_ROLL, -0.25, 7 * PI / 4, false, 0.0);

						if ( my->monsterAttackTime >= 4 * ANIMATE_DURATION_WINDUP / (monsterGlobalAnimationMultiplier / 10.0) )
						{
							if ( multiplayer != CLIENT )
							{
								my->attack(MONSTER_POSE_MAGIC_WINDUP3, 0, nullptr);
							}
						}
					}
					else if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP3 )
					{
						if ( multiplayer != CLIENT )
						{
							if ( my->monsterAttackTime == 1 )
							{
								if ( myStats->getAttribute("salamander_type") == "zealot" )
								{
									bool found = false;
									if ( local_rng.rand() % 3 == 0 || (myStats->getEffectActive(EFF_DIVINE_ZEAL) && local_rng.rand() % 2 == 0) )
									{
										if ( myStats->getAttribute("salamander_spell") == "" )
										{
											if ( Entity* ally = uidToEntity(my->parent) )
											{
												if ( ally->behavior == &actMonster )
												{
													if ( Stat* allyStats = ally->getStats() )
													{
														if ( allyStats->type == SALAMANDER )
														{
															if ( local_rng.rand() % 3 == 0 )
															{
																myStats->setAttribute("salamander_spell", std::to_string(SPELL_SIGIL));
															}
															else
															{
																myStats->setAttribute("salamander_spell", std::to_string(SPELL_SANCTUARY));
															}
														}
													}
												}
											}
											if ( myStats->getAttribute("salamander_spell") == "" )
											{
												myStats->setAttribute("salamander_spell", std::to_string(SPELL_SIGIL));
											}
										}

										int spellID = std::stoi(myStats->getAttribute("salamander_spell"));
										if ( spellID == SPELL_SANCTUARY && !myStats->getEffectActive(EFF_SANCTUARY) )
										{
											CastSpellProps_t props;
											props.target_x = my->x;
											props.target_y = my->y;
											castSpell(my->getUID(), getSpellFromID(spellID), true, false, false, &props);
											found = true;
										}
										if ( spellID == SPELL_SIGIL && !myStats->getEffectActive(EFF_SIGIL) )
										{
											CastSpellProps_t props;
											props.target_x = my->x;
											props.target_y = my->y;
											castSpell(my->getUID(), getSpellFromID(spellID), true, false, false, &props);
											found = true;
										}
									}

									if ( !found )
									{
										Entity* target = my;
										if ( myStats->getEffectActive(EFF_DIVINE_ZEAL) )
										{
											if ( Entity* ally = uidToEntity(my->parent) )
											{
												if ( ally->behavior == &actMonster )
												{
													if ( Stat* allyStats = ally->getStats() )
													{
														if ( allyStats->type == SALAMANDER && !allyStats->getEffectActive(EFF_DIVINE_ZEAL) )
														{
															target = ally;
														}
													}
												}
											}
										}
										CastSpellProps_t props;
										props.targetUID = target->getUID();
										castSpell(my->getUID(), getSpellFromID(SPELL_DIVINE_ZEAL), true, false, false, &props);
									}
								}
								else
								{
									//castSpell(my->getUID(), getSpellFromID(SPELL_FOCI_FIRE), true, false, false, nullptr);
									castSpell(my->getUID(), getSpellFromID(SPELL_BREATHE_FIRE), true, false, false, nullptr);
								}
							}
						}
						if ( weaponarm->pitch >= 3 * PI / 2 )
						{
							my->monsterArmbended = 1;
						}

						if ( weaponarm->skill[1] == 0 )
						{
							// chop forwards
							if ( limbAnimateToLimit(weaponarm, ANIMATE_PITCH, 0.4, PI / 3, false, 0.0) )
							{
								weaponarm->skill[1] = 1;
							}
						}
						else if ( weaponarm->skill[1] == 1 )
						{
							if ( limbAnimateToLimit(weaponarm, ANIMATE_PITCH, -0.25, 7 * PI / 4, false, 0.0) )
							{
								weaponarm->skill[1] = 2;
								weaponarm->pitch = 0.0;
								my->monsterWeaponYaw = 0;
								weaponarm->roll = 0;
							}
						}
						else
						{
							my->monsterArmbended = 0;
						}
						if ( my->monsterAttackTime >= 4 * ANIMATE_DURATION_WINDUP / (monsterGlobalAnimationMultiplier / 10.0) )
						{
							weaponarm->skill[1] = 0;
							my->monsterArmbended = 0;
							my->monsterAttack = 0;
							weaponarm->skill[0] = rightbody->skill[0];
							weaponarm->pitch = rightbody->pitch;
						}
					}
					else if ( my->monsterAttack == MONSTER_POSE_MAGIC_CAST1 )
					{
						if ( weaponarm->pitch >= 3 * PI / 2 )
						{
							my->monsterArmbended = 1;
						}

						if ( weaponarm->skill[1] == 0 )
						{
							// chop forwards
							if ( limbAnimateToLimit(weaponarm, ANIMATE_PITCH, 0.4, PI / 3, false, 0.0) )
							{
								weaponarm->skill[1] = 1;
							}
						}
						else if ( weaponarm->skill[1] >= 1 )
						{
							if ( limbAnimateToLimit(weaponarm, ANIMATE_PITCH, -0.25, 7 * PI / 4, false, 0.0) )
							{
								weaponarm->skill[1] = 0;
								weaponarm->skill[0] = rightbody->skill[0];
								my->monsterWeaponYaw = 0;
								weaponarm->pitch = rightbody->pitch;
								weaponarm->roll = 0;
								my->monsterArmbended = 0;
								my->monsterAttack = 0;
							}
						}
					}
					else
					{
						my->handleWeaponArmAttack(entity);
					}
				}
			}
			else if ( bodypart == LIMB_HUMANOID_CLOAK )
			{
				entity->pitch = entity->fskill[0];
			}

			if ( hovering )
			{
				// hands stationary, legs pitched back and little swing.
				double limbSpeed = 0.03;
				if ( bodypart == LIMB_HUMANOID_RIGHTARM && (MONSTER_ATTACK == 0 && MONSTER_ATTACKTIME == 0) ) // right arm relaxed, not attacking.
				{
					entity->skill[0] = rightbody->skill[0];
					if ( entity->pitch < 0 )
					{
						entity->pitch += 1 / fmax(limbSpeed * .1, 10.0);
						if ( entity->pitch > 0 )
						{
							entity->pitch = 0;
						}
					}
					else if ( entity->pitch > 0 )
					{
						entity->pitch -= 1 / fmax(limbSpeed * .1, 10.0);
						if ( entity->pitch < 0 )
						{
							entity->pitch = 0;
						}
					}
				}
				else if ( bodypart == LIMB_HUMANOID_LEFTLEG ) // leftleg
				{
					if ( entity->pitch < 0 )
					{
						entity->pitch += 5 * limbSpeed * MONSTER_SWALKSPEED; // speed up to reach target.
					}
					entity->skill[0] = 1;
					if ( entity->skill[3] == 1 ) // throwaway skill.
					{
						entity->pitch -= limbSpeed * MONSTER_SWALKSPEED;
						if ( entity->pitch < PI / 6.f )
						{
							entity->skill[3] = 0;
							entity->pitch = PI / 6.f;
						}
					}
					else
					{
						entity->pitch += limbSpeed * MONSTER_SWALKSPEED;
						if ( entity->pitch > PI / 3.f )
						{
							entity->skill[3] = 1;
							entity->pitch = PI / 3.f;
						}
					}
				}

				if ( bodypart == LIMB_HUMANOID_CLOAK )
				{
					if ( dist > 0.1 )
					{
						if ( entity->skill[0] == 0 )
						{
							entity->pitch -= dist * MONSTER_SWALKSPEED * 0.05;
							if ( entity->pitch < -PI / 4.0 )
							{
								entity->skill[0] = 1;
							}
						}
						else
						{
							entity->pitch += dist * MONSTER_SWALKSPEED * 0.05;
							if ( entity->pitch > -PI / 8.0 )
							{
								entity->skill[0] = 0;
							}
						}
					}
					else
					{
						entity->pitch += MONSTER_SWALKSPEED / 5;
						entity->pitch = std::min(entity->pitch, 0.0);
					}
				}
			}
			else
			{
				my->humanoidAnimateWalk(entity, node, bodypart, MONSTER_SWALKSPEED, dist, 0.4);
			}

			if ( bodypart == LIMB_HUMANOID_CLOAK )
			{
				entity->fskill[0] = entity->pitch;
				entity->roll = my->roll - fabs(entity->pitch) / 2;
				entity->pitch = 0;
			}
		}
		switch ( bodypart )
		{
			// torso
		case LIMB_HUMANOID_TORSO:
			entity->scalex = 1.0;
			entity->scaley = 1.0;
			entity->scalez = 1.0;
			entity->focalx = limbs[SALAMANDER][1][0];
			entity->focaly = limbs[SALAMANDER][1][1];
			entity->focalz = limbs[SALAMANDER][1][2];
			if ( multiplayer != CLIENT )
			{
				if ( myStats->breastplate == nullptr || !itemModel(myStats->breastplate, false, my) )
				{
					entity->sprite =
						(my->sprite == 1536 || my->sprite == 1537) ? 1560 :
						(my->sprite == 1538 || my->sprite == 1539) ? 1561 : 1562;
				}
				else
				{
					entity->sprite = itemModel(myStats->breastplate, false, my);
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			my->setHumanoidLimbOffset(entity, SALAMANDER, LIMB_HUMANOID_TORSO);
			break;
			// right leg
		case LIMB_HUMANOID_RIGHTLEG:
			entity->focalx = limbs[SALAMANDER][2][0];
			entity->focaly = limbs[SALAMANDER][2][1];
			entity->focalz = limbs[SALAMANDER][2][2];
			if ( multiplayer != CLIENT )
			{
				if ( myStats->shoes == nullptr )
				{
					entity->sprite =
						(my->sprite == 1536 || my->sprite == 1537) ? 1555 :
						(my->sprite == 1538 || my->sprite == 1539) ? 1557 : 1559;
				}
				else
				{
					my->setBootSprite(entity, SPRITE_BOOT_RIGHT_OFFSET);
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			my->setHumanoidLimbOffset(entity, SALAMANDER, LIMB_HUMANOID_RIGHTLEG);
			break;
			// left leg
		case LIMB_HUMANOID_LEFTLEG:
			entity->focalx = limbs[SALAMANDER][3][0];
			entity->focaly = limbs[SALAMANDER][3][1];
			entity->focalz = limbs[SALAMANDER][3][2];
			if ( multiplayer != CLIENT )
			{
				if ( myStats->shoes == nullptr )
				{
					entity->sprite = (my->sprite == 1536 || my->sprite == 1537) ? 1554 :
						(my->sprite == 1538 || my->sprite == 1539) ? 1556 : 1558;
				}
				else
				{
					my->setBootSprite(entity, SPRITE_BOOT_LEFT_OFFSET);
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			my->setHumanoidLimbOffset(entity, SALAMANDER, LIMB_HUMANOID_LEFTLEG);
			break;
			// right arm
		case LIMB_HUMANOID_RIGHTARM:
		{
			if ( multiplayer != CLIENT )
			{
				if ( myStats->gloves == nullptr )
				{
					entity->sprite = (my->sprite == 1536 || my->sprite == 1537) ? 1544 :
						(my->sprite == 1538 || my->sprite == 1539) ? 1548 : 1552;
				}
				else
				{
					if ( setGloveSprite(myStats, entity, SPRITE_GLOVE_RIGHT_OFFSET) != 0 )
					{
						// successfully set sprite for the human model
					}
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}

			if ( multiplayer == CLIENT )
			{
				if ( entity->skill[7] == 0 )
				{
					if ( entity->sprite == 1544 || entity->sprite == 1548 || entity->sprite == 1552 )
					{
						// these are the default arms.
						// chances are they may be wrong if sent by the server, 
					}
					else
					{
						// otherwise we're being sent gloves armor etc so it's probably right.
						entity->skill[7] = entity->sprite;
					}
				}
				if ( entity->skill[7] == 0 )
				{
					// we set this ourselves until proper initialisation.
					entity->sprite = (my->sprite == 1536 || my->sprite == 1537) ? 1544 :
						(my->sprite == 1538 || my->sprite == 1539) ? 1548 : 1552;
				}
				else
				{
					entity->sprite = entity->skill[7];
				}
			}

			node_t* weaponNode = list_Node(&my->children, 7);
			if ( weaponNode )
			{
				Entity* weapon = (Entity*)weaponNode->element;
				if ( (MONSTER_ARMBENDED || (weapon->flags[INVISIBLE] && my->monsterState == MONSTER_STATE_WAIT)) && !hovering )
				{
					// if weapon invisible and I'm not attacking, relax arm.
					entity->focalx = limbs[SALAMANDER][4][0]; // 0
					entity->focaly = limbs[SALAMANDER][4][1];
					entity->focalz = limbs[SALAMANDER][4][2]; // 2

					/*if ( entity->sprite == 1523 )
					{
						entity->focaly += 0.25;
					}*/
				}
				else
				{
					// else flex arm.
					entity->focalx = limbs[SALAMANDER][4][0] + 0.75;
					entity->focaly = limbs[SALAMANDER][4][1] + 0.25;
					entity->focalz = limbs[SALAMANDER][4][2] - 0.75;
					if ( entity->sprite == 1544 || entity->sprite == 1548 || entity->sprite == 1552 )
					{
						entity->sprite += 1;
					}
					else
					{
						entity->sprite += 2;
					}
				}
			}
			my->setHumanoidLimbOffset(entity, SALAMANDER, LIMB_HUMANOID_RIGHTARM);
			entity->yaw += MONSTER_WEAPONYAW;
			break;
			// left arm
		}
		case LIMB_HUMANOID_LEFTARM:
		{
			if ( multiplayer != CLIENT )
			{
				if ( myStats->gloves == nullptr )
				{
					entity->sprite = (my->sprite == 1536 || my->sprite == 1537) ? 1542 :
						(my->sprite == 1538 || my->sprite == 1539) ? 1546 : 1550;
				}
				else
				{
					if ( setGloveSprite(myStats, entity, SPRITE_GLOVE_LEFT_OFFSET) != 0 )
					{
						// successfully set sprite for the human model
					}
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}

			if ( multiplayer == CLIENT )
			{
				if ( entity->skill[7] == 0 )
				{
					if ( entity->sprite == 1542 || entity->sprite == 1546 || entity->sprite == 1550 )
					{
						// these are the default arms.
						// chances are they may be wrong if sent by the server, 
					}
					else
					{
						// otherwise we're being sent gloves armor etc so it's probably right.
						entity->skill[7] = entity->sprite;
					}
				}
				if ( entity->skill[7] == 0 )
				{
					// we set this ourselves until proper initialisation.
					entity->sprite = (my->sprite == 1536 || my->sprite == 1537) ? 1542 :
						(my->sprite == 1538 || my->sprite == 1539) ? 1546 : 1550;
				}
				else
				{
					entity->sprite = entity->skill[7];
				}
			}

			shieldarm = entity;
			node_t* shieldNode = list_Node(&my->children, 8);
			if ( shieldNode )
			{
				Entity* shield = (Entity*)shieldNode->element;
				if ( (shield->flags[INVISIBLE] && my->monsterState == MONSTER_STATE_WAIT) && !hovering )
				{
					entity->focalx = limbs[SALAMANDER][5][0]; // 0
					entity->focaly = limbs[SALAMANDER][5][1];
					entity->focalz = limbs[SALAMANDER][5][2]; // 2

					/*if ( entity->sprite == 1521 )
					{
						entity->focaly -= 0.25;
					}*/
				}
				else
				{
					entity->focalx = limbs[SALAMANDER][5][0] + 0.75;
					entity->focaly = limbs[SALAMANDER][5][1] - 0.25;
					entity->focalz = limbs[SALAMANDER][5][2] - 0.75;
					if ( entity->sprite == 1542 || entity->sprite == 1546 || entity->sprite == 1550 )
					{
						entity->sprite += 1;
					}
					else
					{
						entity->sprite += 2;
					}
				}
			}
			my->setHumanoidLimbOffset(entity, SALAMANDER, LIMB_HUMANOID_LEFTARM);
			if ( my->monsterDefend && my->monsterAttack == 0 )
			{
				MONSTER_SHIELDYAW = PI / 5;
			}
			else
			{
				MONSTER_SHIELDYAW = 0;
			}
			entity->yaw += MONSTER_SHIELDYAW;
			break;
		}
		// weapon
		case LIMB_HUMANOID_WEAPON:
			if ( multiplayer != CLIENT )
			{
				if ( myStats->weapon == nullptr || myStats->getEffectActive(EFF_INVISIBLE) || wearingring ) //TODO: isInvisible()?
				{
					entity->flags[INVISIBLE] = true;
				}
				else
				{
					entity->sprite = itemModel(myStats->weapon);
					if ( itemCategory(myStats->weapon) == SPELLBOOK )
					{
						entity->flags[INVISIBLE] = true;
					}
					else
					{
						entity->flags[INVISIBLE] = false;
					}
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->skill[11] != entity->flags[INVISIBLE] )
						{
							entity->skill[11] = entity->flags[INVISIBLE];
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			else
			{
				if ( entity->sprite <= 0 )
				{
					entity->flags[INVISIBLE] = true;
				}
			}
			if ( weaponarm != nullptr )
			{
				my->handleHumanoidWeaponLimb(entity, weaponarm);
			}
			break;
			// shield
		case LIMB_HUMANOID_SHIELD:
			if ( multiplayer != CLIENT )
			{
				if ( myStats->shield == nullptr )
				{
					entity->flags[INVISIBLE] = true;
					entity->sprite = 0;
				}
				else
				{
					entity->flags[INVISIBLE] = false;
					entity->sprite = itemModel(myStats->shield);
					if ( itemTypeIsQuiver(myStats->shield->type) )
					{
						entity->handleQuiverThirdPersonModel(*myStats);
					}
				}
				if ( myStats->getEffectActive(EFF_INVISIBLE) || wearingring ) //TODO: isInvisible()?
				{
					entity->flags[INVISIBLE] = true;
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->skill[11] != entity->flags[INVISIBLE] )
						{
							entity->skill[11] = entity->flags[INVISIBLE];
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			else
			{
				if ( entity->sprite <= 0 )
				{
					entity->flags[INVISIBLE] = true;
				}
			}
			my->handleHumanoidShieldLimb(entity, shieldarm);
			break;
			// cloak
		case LIMB_HUMANOID_CLOAK:
			entity->focalx = limbs[SALAMANDER][8][0];
			entity->focaly = limbs[SALAMANDER][8][1];
			entity->focalz = limbs[SALAMANDER][8][2];
			if ( multiplayer != CLIENT )
			{
				if ( myStats->cloak == nullptr || myStats->getEffectActive(EFF_INVISIBLE) || wearingring ) //TODO: isInvisible()?
				{
					entity->flags[INVISIBLE] = true;
				}
				else
				{
					entity->flags[INVISIBLE] = false;
					entity->sprite = itemModel(myStats->cloak);
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->skill[11] != entity->flags[INVISIBLE] )
						{
							entity->skill[11] = entity->flags[INVISIBLE];
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			else
			{
				if ( entity->sprite <= 0 )
				{
					entity->flags[INVISIBLE] = true;
				}
			}
			entity->x -= cos(my->yaw);
			entity->y -= sin(my->yaw);
			entity->yaw += PI / 2;
			break;
			// helm
		case LIMB_HUMANOID_HELMET:
			helmet = entity;
			entity->focalx = limbs[SALAMANDER][9][0]; // 0
			entity->focaly = limbs[SALAMANDER][9][1]; // 0
			entity->focalz = limbs[SALAMANDER][9][2]; // -2
			entity->pitch = my->pitch;
			entity->roll = 0;
			if ( multiplayer != CLIENT )
			{
				entity->sprite = itemModel(myStats->helmet);
				if ( myStats->helmet == nullptr || myStats->getEffectActive(EFF_INVISIBLE) || wearingring ) //TODO: isInvisible()?
				{
					entity->flags[INVISIBLE] = true;
				}
				else
				{
					entity->flags[INVISIBLE] = false;
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->skill[11] != entity->flags[INVISIBLE] )
						{
							entity->skill[11] = entity->flags[INVISIBLE];
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			else
			{
				if ( entity->sprite <= 0 )
				{
					entity->flags[INVISIBLE] = true;
				}
			}
			my->setHelmetLimbOffset(entity);
			break;
			// mask
		case LIMB_HUMANOID_MASK:
			entity->focalx = limbs[SALAMANDER][10][0]; // 0
			entity->focaly = limbs[SALAMANDER][10][1]; // 0
			entity->focalz = limbs[SALAMANDER][10][2]; // .25
			entity->pitch = my->pitch;
			entity->roll = PI / 2;
			if ( multiplayer != CLIENT )
			{
				if ( myStats->mask == nullptr || myStats->getEffectActive(EFF_INVISIBLE) || wearingring ) //TODO: isInvisible()?
				{
					entity->flags[INVISIBLE] = true;
				}
				else
				{
					entity->flags[INVISIBLE] = false;
				}
				if ( myStats->mask != nullptr )
				{
					if ( myStats->mask->type == TOOL_GLASSES )
					{
						entity->sprite = 165; // GlassesWorn.vox
					}
					else if ( myStats->mask->type == MONOCLE )
					{
						entity->sprite = 1196; // monocleWorn.vox
					}
					else
					{
						entity->sprite = itemModel(myStats->mask);
					}
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->skill[11] != entity->flags[INVISIBLE] )
						{
							entity->skill[11] = entity->flags[INVISIBLE];
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			else
			{
				if ( entity->sprite <= 0 )
				{
					entity->flags[INVISIBLE] = true;
				}
			}
			if ( EquipmentModelOffsets.modelOffsetExists(SALAMANDER, entity->sprite, my->sprite) )
			{
				my->setHelmetLimbOffset(entity);
				my->setHelmetLimbOffsetWithMask(helmet, entity);
			}
			else
			{
				entity->focalx = limbs[SALAMANDER][10][0] + .35; // .35
				entity->focaly = limbs[SALAMANDER][10][1] - 2; // -2
				entity->focalz = limbs[SALAMANDER][10][2]; // .25
			}
			break;
		// tail
		case 12:
		{
			entity->focalx = limbs[SALAMANDER][11][0];
			entity->focaly = limbs[SALAMANDER][11][1];
			entity->focalz = limbs[SALAMANDER][11][2];
			entity->x += limbs[SALAMANDER][12][0] * cos(my->yaw + PI / 2) + limbs[SALAMANDER][12][1] * cos(my->yaw);
			entity->y += limbs[SALAMANDER][12][0] * sin(my->yaw + PI / 2) + limbs[SALAMANDER][12][1] * sin(my->yaw);
			entity->z += limbs[SALAMANDER][12][2];
			entity->pitch = 0.15;

			if ( multiplayer != CLIENT )
			{
				entity->flags[INVISIBLE] = my->flags[INVISIBLE];
				entity->flags[INVISIBLE_DITHER] = entity->flags[INVISIBLE];
				entity->sprite = 1563;
				switch ( my->sprite )
				{
				case 1536:
					entity->sprite = 1563;
					break;
				case 1537:
					entity->sprite = 1564;
					break;
				case 1538:
					entity->sprite = 1565;
					break;
				case 1539:
					entity->sprite = 1566;
					break;
				case 1540:
					entity->sprite = 1567;
					break;
				case 1541:
					entity->sprite = 1568;
					break;
				default:
					break;
				}
				if ( myStats->getEffectActive(EFF_INVISIBLE) || wearingring ) //TODO: isInvisible()?
				{
					entity->flags[INVISIBLE] = true;
				}
				else
				{
					entity->flags[INVISIBLE] = false;
				}
				if ( multiplayer == SERVER )
				{
					// update sprites for clients
					if ( entity->ticks >= *cvar_entity_bodypart_sync_tick )
					{
						bool updateBodypart = false;
						if ( entity->skill[10] != entity->sprite )
						{
							entity->skill[10] = entity->sprite;
							updateBodypart = true;
						}
						if ( entity->skill[11] != entity->flags[INVISIBLE] )
						{
							entity->skill[11] = entity->flags[INVISIBLE];
							updateBodypart = true;
						}
						if ( entity->getUID() % (TICKS_PER_SECOND * 10) == ticks % (TICKS_PER_SECOND * 10) )
						{
							updateBodypart = true;
						}
						if ( updateBodypart )
						{
							serverUpdateEntityBodypart(my, bodypart);
						}
					}
				}
			}
			else
			{
				if ( entity->sprite <= 0 )
				{
					entity->flags[INVISIBLE] = true;
				}
			}

			if ( entity->sprite == 1564
				|| entity->sprite == 1566
				|| entity->sprite == 1568 )
			{
				entity->focalx += 0.5;
				entity->focalz -= 0.25;
			}

			bool moving = false;
			if ( dist > 0.1 )
			{
				moving = true;
			}
			if ( entity->skill[0] == 0 )
			{
				if ( moving )
				{
					entity->fskill[0] += std::min(dist * MONSTER_SWALKSPEED, 2.f * MONSTER_SWALKSPEED); // move proportional to move speed
				}
				else if ( my->monsterAttack != 0 )
				{
					entity->fskill[0] += MONSTER_SWALKSPEED; // move fixed speed when attacking if stationary
				}
				else
				{
					entity->fskill[0] += 0.01; // otherwise move slow idle
				}

				if ( entity->fskill[0] > PI / 3 || ((!moving || my->monsterAttack != 0) && entity->fskill[0] > PI / 5) )
				{
					// switch direction if angle too great, angle is shorter if attacking or stationary
					entity->skill[0] = 1;
				}
			}
			else // reverse of the above
			{
				if ( moving )
				{
					if ( hovering )
					{
						entity->fskill[0] -= std::min(std::max(0.15, dist * MONSTER_SWALKSPEED), 2.f * MONSTER_SWALKSPEED);
					}
					else
					{
						entity->fskill[0] -= std::min(dist * MONSTER_SWALKSPEED, 2.f * MONSTER_SWALKSPEED);
					}
				}
				else if ( my->monsterAttack != 0 )
				{
					entity->fskill[0] -= MONSTER_SWALKSPEED;
				}
				else
				{
					entity->fskill[0] -= 0.007;
				}

				if ( entity->fskill[0] < -0.0 )
				{
					entity->skill[0] = 0;
					entity->skill[1] = entity->skill[1] != 0 ? 0 : 1;
				}
			}
			//entity->yaw += -entity->fskill[0];
			real_t dir = entity->skill[1] == 0 ? 1 : -1;
			entity->pitch += 0.5 * sin(entity->fskill[0]);
			entity->roll = dir * -0.25 * sin(entity->fskill[0]);
		}
		break;
		}
	}
	// rotate shield a bit
	node_t* shieldNode = list_Node(&my->children, 8);
	if ( shieldNode )
	{
		Entity* shieldEntity = (Entity*)shieldNode->element;
		if ( shieldEntity->sprite != items[TOOL_TORCH].index && shieldEntity->sprite != items[TOOL_LANTERN].index && shieldEntity->sprite != items[TOOL_CRYSTALSHARD].index )
		{
			shieldEntity->yaw -= PI / 6;
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

//void Entity::goatmanChooseWeapon(const Entity* target, double dist)
//{
//	if ( monsterSpecialState != 0 )
//	{
//		//Holding a weapon assigned from the special attack. Don't switch weapons.
//		//messagePlayer()
//		return;
//	}
//
//	//TODO: I don't like this function getting called every frame. Find a better place to put it.
//	//Although if I do that, can't do this dirty little hack for the goatman's special...
//
//	//TODO: If applying attack animations that will involve holding a potion for several frames while this code has a chance to run, do a check here to cancel the function if holding a potion.
//
//	Stat *myStats = getStats();
//	if ( !myStats )
//	{
//		return;
//	}
//
//	if ( myStats->weapon && (itemCategory(myStats->weapon) == SPELLBOOK) )
//	{
//		return;
//	}
//
//	int specialRoll = -1;
//	bool usePotionSpecial = false;
//
//	/*
//	 * For the goatman's special:
//	 * * If specialRoll == 0, want to use a booze or healing potion (prioritize healing potion if damaged enough).
//	 * * If no have potion, try to use THROWN in melee.
//	 * * If in melee, if potion is not a healing potion, check if have any THROWN and then 50% chance to use those instead.
//	 */
//
//	node_t* hasPotion = nullptr;
//	bool isHealingPotion = false;
//
//	if ( monsterSpecialTimer == 0 && (ticks % 10 == 0) && monsterAttack == 0 )
//	{
//		//messagePlayer(clientnum, "Cooldown done!");
//		specialRoll = local_rng.rand()%10;
//
//		if ( specialRoll == 0 )
//		{
//			if ( myStats->HP <= myStats->MAXHP / 3 * 2 )
//			{
//				//Try to get a health potion.
//				hasPotion = itemNodeInInventory(myStats, POTION_EXTRAHEALING, static_cast<Category>(-1));
//				if ( !hasPotion )
//				{
//					hasPotion = itemNodeInInventory(myStats, POTION_HEALING, static_cast<Category>(-1));
//					if ( hasPotion )
//					{
//						//Equip and chuck it now.
//						bool swapped = swapMonsterWeaponWithInventoryItem(this, myStats, hasPotion, false, false);
//						if ( !swapped )
//						{
//							//printlog("Error in Entity::goatmanChooseWeapon(): failed to swap healing potion into hand!");
//							//Don't return, want to try equipping either a potion of booze, or one of the other weapon routes (e.h. a THROWN special if in melee or just an axe if worst comes to worst).
//						}
//						else
//						{
//							monsterSpecialState = GOATMAN_POTION;
//							//monsterHitTime = 2 * HITRATE;
//							return;
//						}
//					}
//				}
//				else
//				{
//					//Equip and chuck it now.
//					bool swapped = swapMonsterWeaponWithInventoryItem(this, myStats, hasPotion, false, false);
//					if ( !swapped )
//					{
//						//printlog("Error in Entity::goatmanChooseWeapon(): failed to swap healing potion into hand!");
//						//Don't return, want to try equipping either a potion of booze, or one of the other weapon routes (e.h. a THROWN special if in melee or just an axe if worst comes to worst).
//					}
//					else
//					{
//						monsterSpecialState = GOATMAN_POTION;
//						//monsterHitTime = 2 * HITRATE;
//						return;
//					}
//				}
//			}
//
//			if ( !hasPotion )
//			{
//				//Couldn't find a healing potion? Try for a potion of booze.
//				hasPotion = itemNodeInInventory(myStats, POTION_BOOZE, static_cast<Category>(-1));
//				if ( hasPotion )
//				{
//					//Equip and chuck it now.
//					bool swapped = swapMonsterWeaponWithInventoryItem(this, myStats, hasPotion, false, false);
//					if ( !swapped )
//					{
//						//printlog("Error in Entity::goatmanChooseWeapon(): failed to swap healing potion into hand!");
//						//Don't return, want to try equipping either a potion of booze, or one of the other weapon routes (e.h. a THROWN special if in melee or just an axe if worst comes to worst).
//					}
//					else
//					{
//						monsterSpecialState = GOATMAN_POTION;
//						//monsterHitTime = 2 * HITRATE;
//						return;
//					}
//				}
//			}
//		}
//	}
//
//	bool inMeleeRange = monsterInMeleeRange(target, dist);
//
//	if ( inMeleeRange )
//	{
//		if ( monsterSpecialTimer == 0 && (ticks % 10 == 0) && monsterAttack == 0 && specialRoll == 0 )
//		{
//			bool tryChakram = true;
//			if ( hasPotion && local_rng.rand()%10 )
//			{
//				tryChakram = false;
//			}
//
//			if ( tryChakram )
//			{
//				//Grab a chakram instead.
//				node_t* thrownNode = itemNodeInInventory(myStats, -1, THROWN);
//				if ( thrownNode )
//				{
//					bool swapped = swapMonsterWeaponWithInventoryItem(this, myStats, thrownNode, false, false);
//					if ( !swapped )
//					{
//						//printlog("Error in Entity::goatmanChooseWeapon(): failed to swap THROWN into hand! Cursed? (%d)", myStats->weapon->beatitude);
//						//Don't return, make sure holding a melee weapon at least.
//					}
//					else
//					{
//						monsterSpecialState = GOATMAN_THROW;
//						return;
//					}
//				}
//			}
//		}
//
//		//Switch to a melee weapon if not already wielding one. Unless monster special state is overriding the AI.
//		if ( !myStats->weapon || !isMeleeWeapon(*myStats->weapon) )
//		{
//			node_t* weaponNode = getMeleeWeaponItemNodeInInventory(myStats);
//			if ( !weaponNode )
//			{
//				if ( myStats->weapon && myStats->weapon->type == MAGICSTAFF_SLOW )
//				{
//					monsterUnequipSlotFromCategory(myStats, &myStats->weapon, MAGICSTAFF);
//				}
//				return; //Resort to fists.
//			}
//
//			bool swapped = swapMonsterWeaponWithInventoryItem(this, myStats, weaponNode, false, false);
//			if ( !swapped )
//			{
//				//printlog("Error in Entity::goatmanChooseWeapon(): failed to swap melee weapon into hand! Cursed? (%d)", myStats->weapon->beatitude);
//				//Don't return so that monsters will at least equip ranged weapons in melee range if they don't have anything else.
//			}
//			else
//			{
//				return;
//			}
//		}
//		else
//		{
//			return;
//		}
//	}
//
//	//if ( hasPotion )
//	//{
//	//	//Try to equip the potion first. If fails, then equip normal ranged.
//	//	bool swapped = swapMonsterWeaponWithInventoryItem(this, myStats, hasPotion, false, false);
//	//	if ( !swapped )
//	//	{
//	//		printlog("Error in Entity::goatmanChooseWeapon(): failed to swap non-healing potion into hand! (non-melee block) Cursed? (%d)", myStats->weapon->beatitude);
//	//	}
//	//	else
//	//	{
//	//		monsterSpecialState = GOATMAN_POTION;
//	//		return;
//	//	}
//	//}
//
//	//Switch to a thrown weapon or a ranged weapon. Potions are reserved as a special attack.
//	if ( !myStats->weapon || isMeleeWeapon(*myStats->weapon) )
//	{
//		//First search the inventory for a THROWN weapon.
//		node_t *weaponNode = nullptr;
//		if ( monsterSpecialTimer == 0 && (ticks % 10 == 0) && monsterAttack == 0 && local_rng.rand() % 10 == 0 )
//		{
//			weaponNode = itemNodeInInventory(myStats, -1, THROWN);
//			if ( weaponNode )
//			{
//				if ( swapMonsterWeaponWithInventoryItem(this, myStats, weaponNode, false, false) )
//				{
//					monsterSpecialState = GOATMAN_THROW;
//					return;
//				}
//			}
//		}
//		if ( !weaponNode )
//		{
//			//If couldn't find any, search the inventory for a ranged weapon.
//			weaponNode = getRangedWeaponItemNodeInInventory(myStats, true);
//		}
//
//		bool swapped = swapMonsterWeaponWithInventoryItem(this, myStats, weaponNode, false, false);
//		return;
//	}
//
//	return;
//}
//




