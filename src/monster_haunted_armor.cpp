/*-------------------------------------------------------------------------------

	BARONY
	File: monster_human.cpp
	Desc: implements all of the human monster's code

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
#include "classdescriptions.hpp"
#include "player.hpp"
#include "prng.hpp"
#include "scores.hpp"
#include "mod_tools.hpp"

enum HauntedType
{
	HAUNTED_SILVER,
	HAUNTED_BLACKIRON,
	HAUNTED_SILVER_ARMOR,
	HAUNTED_BLACKIRON_ARMOR
};

void initHauntedArmor(Entity* my, Stat* myStats)
{
	node_t* node;

	my->flags[BURNABLE] = false;
	my->flags[INVISIBLE] = true;
	my->initMonster(2468);
	my->sprite = 2468;
	my->z = -1;

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

			my->createPathBoundariesNPC();

			bool isDefaultStats = isMonsterStatsDefault(*myStats);

			// apply random stat increases if set in stat_shared.cpp or editor
			setRandomMonsterStats(myStats, rng);

			// generate 6 items max, less if there are any forced items from boss variants
			int customItemsToGenerate = ITEM_CUSTOM_SLOT_LIMIT;

			// generates equipment and weapons if available from editor
			createMonsterEquipment(myStats, rng);

			// create any custom inventory items from editor if available
			createCustomInventory(myStats, customItemsToGenerate, rng);

			// count if any custom inventory items from editor
			int customItems = countCustomItems(myStats);
			//max limit of 6 custom items per entity.

			// count any inventory items set to default in edtior
			int defaultItems = countDefaultItems(myStats);

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

			myStats->setEffectActive(EFF_LEVITATING, 1);
			myStats->EFFECTS_TIMERS[EFF_LEVITATING] = 0;

			HauntedType type = HAUNTED_SILVER;

			if ( myStats->getAttribute("haunted_type") == "" )
			{
				if ( rng.rand() % 2 == 0 )
				{
					myStats->setAttribute("haunted_type", "silver");
					type = HAUNTED_SILVER;
				}
				else
				{
					myStats->setAttribute("haunted_type", "blackiron");
					type = HAUNTED_BLACKIRON;
				}
			}
			else
			{
				if ( myStats->getAttribute("haunted_type").find("armor") != std::string::npos )
				{
					if ( myStats->getAttribute("haunted_type").find("silver") != std::string::npos )
					{
						type = HAUNTED_SILVER_ARMOR;
					}
					else if ( myStats->getAttribute("haunted_type").find("blackiron") != std::string::npos )
					{
						type = HAUNTED_BLACKIRON_ARMOR;
					}
				}
			}

			std::vector<Item*> slots;

			//give weapon
			if ( myStats->weapon == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_WEAPON] == 1 )
			{
				/*if ( type == HAUNTED_BLACKIRON_ARMOR )
				{
					myStats->weapon = newItem(BLACKIRON_TRIDENT, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
				else if ( type == HAUNTED_SILVER_ARMOR )
				{
					myStats->weapon = newItem(SILVER_GLAIVE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
				else */
				if ( type == HAUNTED_SILVER )
				{
					switch ( rng.rand() % 5 )
					{
					case 0:
						myStats->weapon = newItem(SILVER_GLAIVE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
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
					case 4:
						myStats->weapon = newItem(LONGBOW, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					default:
						break;
					}
				}
				else if ( type == HAUNTED_BLACKIRON )
				{
					switch ( rng.rand() % 5 )
					{
					case 0:
						myStats->weapon = newItem(BLACKIRON_TRIDENT, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 1:
						myStats->weapon = newItem(BLACKIRON_AXE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 2:
						myStats->weapon = newItem(BLACKIRON_SWORD, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 3:
						myStats->weapon = newItem(BLACKIRON_MACE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
						break;
					case 4:
						myStats->weapon = newItem(BLACKIRON_CROSSBOW, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
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
					if ( type == HAUNTED_BLACKIRON_ARMOR || type == HAUNTED_BLACKIRON )
					{
						myStats->shield = newItem(BLACKIRON_SHIELD, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
					}
					else if ( type == HAUNTED_SILVER_ARMOR || type == HAUNTED_SILVER )
					{
						myStats->shield = newItem(SILVER_SHIELD, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
					}
				}
			}

			// give helmet
			if ( myStats->helmet == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_HELM] == 1 )
			{
				if ( type == HAUNTED_BLACKIRON_ARMOR || type == HAUNTED_BLACKIRON )
				{
					myStats->helmet = newItem(BLACKIRON_HELM, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
				else if ( type == HAUNTED_SILVER_ARMOR || type == HAUNTED_SILVER )
				{
					myStats->helmet = newItem(SILVER_HELM, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
			}

			// give cloak
			/*if ( myStats->cloak == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_CLOAK] == 1 )
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
						myStats->cloak = newItem(CLOAK, WORN, 0, 1, rng.rand(), false, nullptr);
						break;
					case 9:
						myStats->cloak = newItem(CLOAK_MAGICREFLECTION, WORN, 0, 1, rng.rand(), false, nullptr);
						break;
				}
			}*/

			// give armor
			if ( myStats->breastplate == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_ARMOR] == 1 )
			{
				if ( type == HAUNTED_BLACKIRON_ARMOR || type == HAUNTED_BLACKIRON )
				{
					myStats->breastplate = newItem(BLACKIRON_BREASTPIECE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
				else if ( type == HAUNTED_SILVER_ARMOR || type == HAUNTED_SILVER )
				{
					myStats->breastplate = newItem(SILVER_BREASTPIECE, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
			}

			// give gloves
			if ( myStats->gloves == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_GLOVES] == 1 )
			{
				if ( type == HAUNTED_BLACKIRON_ARMOR || type == HAUNTED_BLACKIRON )
				{
					myStats->gloves = newItem(BLACKIRON_GAUNTLETS, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
				else if ( type == HAUNTED_SILVER_ARMOR || type == HAUNTED_SILVER )
				{
					myStats->gloves = newItem(SILVER_GAUNTLETS, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
			}

			// give boots
			if ( myStats->shoes == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_BOOTS] == 1 )
			{
				if ( type == HAUNTED_BLACKIRON_ARMOR || type == HAUNTED_BLACKIRON )
				{
					myStats->shoes = newItem(BLACKIRON_BOOTS, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
				else if ( type == HAUNTED_SILVER_ARMOR || type == HAUNTED_SILVER )
				{
					myStats->shoes = newItem(SILVER_BOOTS, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
			}

			// give mask
			if ( myStats->mask == nullptr && myStats->EDITOR_ITEMS[ITEM_SLOT_MASK] == 1 )
			{
				if ( type == HAUNTED_BLACKIRON_ARMOR || type == HAUNTED_BLACKIRON )
				{
					myStats->mask = newItem(MASK_BLACKIRON_VISOR, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, rng.rand(), false, nullptr);
				}
				else if ( type == HAUNTED_SILVER_ARMOR || type == HAUNTED_SILVER )
				{
					myStats->mask = newItem(MASK_STEEL_VISOR, static_cast<Status>(WORN + rng.rand() % 3), -1 + rng.rand() % 3, 1, 3, false, nullptr);
				}
			}

			if ( myStats->weapon )
			{
				slots.push_back(myStats->weapon);
			}
			if ( myStats->shield )
			{
				slots.push_back(myStats->shield);
			}
			if ( myStats->helmet )
			{
				slots.push_back(myStats->helmet);
			}
			if ( myStats->breastplate )
			{
				slots.push_back(myStats->breastplate);
			}
			if ( myStats->gloves )
			{
				slots.push_back(myStats->gloves);
			}
			if ( myStats->shoes )
			{
				slots.push_back(myStats->shoes);
			}
			if ( myStats->mask )
			{
				slots.push_back(myStats->mask);
			}

			int index = -1;
			int dropSlot = -1;
			if ( slots.size() )
			{
				dropSlot = rng.rand() % slots.size();
				for ( auto slot : slots )
				{
					++index;
					if ( index == dropSlot )
					{
					}
					else
					{
						slot->isDroppable = false;
					}
				}
			}

			if ( myStats->amulet )
			{
				myStats->amulet->isDroppable = false;
			}
			if ( myStats->ring )
			{
				myStats->ring->isDroppable = false;
			}
		}
	}

	// torso
	Entity* entity = newEntity(106, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[HAUNTED_ARMOR][1][0]; // 0
	entity->focaly = limbs[HAUNTED_ARMOR][1][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][1][2]; // 0
	entity->behavior = &actHauntedArmorLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// right leg
	entity = newEntity(107, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[HAUNTED_ARMOR][2][0]; // 0
	entity->focaly = limbs[HAUNTED_ARMOR][2][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][2][2]; // 2
	entity->behavior = &actHauntedArmorLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// left leg
	entity = newEntity(108, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[HAUNTED_ARMOR][3][0]; // 0
	entity->focaly = limbs[HAUNTED_ARMOR][3][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][3][2]; // 2
	entity->behavior = &actHauntedArmorLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// right arm
	entity = newEntity(109, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[HAUNTED_ARMOR][4][0]; // 0
	entity->focaly = limbs[HAUNTED_ARMOR][4][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][4][2]; // 1.5
	entity->behavior = &actHauntedArmorLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// left arm
	entity = newEntity(110, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[HAUNTED_ARMOR][5][0]; // 0
	entity->focaly = limbs[HAUNTED_ARMOR][5][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][5][2]; // 1.5
	entity->behavior = &actHauntedArmorLimb;
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
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[HAUNTED_ARMOR][6][0]; // 1.5
	entity->focaly = limbs[HAUNTED_ARMOR][6][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][6][2]; // -.5
	entity->behavior = &actHauntedArmorLimb;
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
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[HAUNTED_ARMOR][7][0]; // 2
	entity->focaly = limbs[HAUNTED_ARMOR][7][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][7][2]; // 0
	entity->behavior = &actHauntedArmorLimb;
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
	entity->scalex = 1.01;
	entity->scaley = 1.01;
	entity->scalez = 1.01;
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[HAUNTED_ARMOR][8][0]; // 0
	entity->focaly = limbs[HAUNTED_ARMOR][8][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][8][2]; // 4
	entity->behavior = &actHauntedArmorLimb;
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
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[HAUNTED_ARMOR][9][0]; // 0
	entity->focaly = limbs[HAUNTED_ARMOR][9][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][9][2]; // -1.75
	entity->behavior = &actHauntedArmorLimb;
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
	entity->scalex = .99;
	entity->scaley = .99;
	entity->scalez = .99;
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	//entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->noColorChangeAllyLimb = 1.0;
	entity->focalx = limbs[HAUNTED_ARMOR][10][0]; // 0
	entity->focaly = limbs[HAUNTED_ARMOR][10][1]; // 0
	entity->focalz = limbs[HAUNTED_ARMOR][10][2]; // .5
	entity->behavior = &actHauntedArmorLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);
}

void actHauntedArmorLimb(Entity* my)
{
	if ( my->light )
	{
		list_RemoveNode(my->light->node);
		my->light = nullptr;
	}

	my->light = addLight(my->x / 16, my->y / 16, "summoned_skeleton_glow");
	my->actMonsterLimb(false);
}

void hauntedArmorDie(Entity* my)
{
	int index = -1;
	for ( auto bodypart : my->bodyparts )
	{
		++index;
		if ( bodypart->flags[INVISIBLE] )
		{
			continue;
		}
		if ( Entity* entity = spawnGib(my, bodypart->sprite) )
		{
			entity->x = bodypart->x;
			entity->y = bodypart->y;
			entity->z = bodypart->z;
			entity->skill[5] = 1; // poof
			serverSpawnGibForClient(entity);
		}
	}

	//my->spawnBlood(681);

	playSoundEntity(my, 28, 128);

	my->removeMonsterDeathNodes();

	list_RemoveNode(my->mynode);
	return;
}

void Entity::hauntedArmorChooseWeapon(const Entity* target, double dist)
{
	if ( monsterSpecialState != 0 )
	{
		//Holding a weapon assigned from the special attack. Don't switch weapons.
		return;
	}

	Stat* myStats = getStats();
	if ( !myStats )
	{
		return;
	}

	if ( myStats->weapon && (itemCategory(myStats->weapon) == SPELLBOOK) )
	{
		return;
	}

	if ( monsterStrafeDirection == 0 && local_rng.rand() % 10 == 0 && ticks % 10 == 0 )
	{
		setBugbearStrafeDir(true);
		//monsterStrafeDirection = -1 + ((local_rng.rand() % 2 == 0) ? 2 : 0);
	}

	HauntedType type = HAUNTED_SILVER;
	if ( myStats->getAttribute("haunted_type") != "" )
	{
		if ( myStats->getAttribute("haunted_type").find("silver") != std::string::npos )
		{
			type = HAUNTED_SILVER;
		}
		else if ( myStats->getAttribute("haunted_type").find("blackiron") != std::string::npos )
		{
			type = HAUNTED_BLACKIRON;
		}
	}

	if ( monsterSpecialTimer == 0 && (ticks % 10 == 0) && monsterAttack == 0
		&& local_rng.rand() % 3 == 0 )
	{
		if ( (dist > TOUCHRANGE && local_rng.rand() % 3 == 0) || dist > 64.0 )
		{
			if ( local_rng.rand() % 2 == 0 && type == HAUNTED_BLACKIRON )
			{
				monsterSpecialState = HAUNTED_ARMOR_SPECIAL_CAST1; // attack
			}
			else
			{
				monsterSpecialState = HAUNTED_ARMOR_SPECIAL_CAST2; // buff
			}
		}
		else
		{
			if ( type == HAUNTED_SILVER )
			{
				monsterSpecialState = HAUNTED_ARMOR_SPECIAL_CAST2; // buff
			}
			else if ( type == HAUNTED_BLACKIRON )
			{
				monsterSpecialState = HAUNTED_ARMOR_SPECIAL_CAST2; // buff
			}
		}
	}

	return;
}

void hauntedArmorSelectSpell(Entity* my, Stat* myStats)
{
	if ( !my || !myStats ) { return; }

	myStats->setAttribute("npc_spell_id", "");
	myStats->setAttribute("npc_spell_target", "");

	HauntedType type = HAUNTED_SILVER;
	if ( myStats->getAttribute("haunted_type") != "" )
	{
		if ( myStats->getAttribute("haunted_type").find("silver") != std::string::npos )
		{
			type = HAUNTED_SILVER;
		}
		else if ( myStats->getAttribute("haunted_type").find("blackiron") != std::string::npos )
		{
			type = HAUNTED_BLACKIRON;
		}
	}

	int spellID = SPELL_NONE;
	struct SpellOption
	{
		int spellID = SPELL_NONE;
		int effectID = 0;
		bool buff = false;
		bool targetOther = false;
	};
	std::vector<SpellOption> options; // spellID then if self
	bool setProps = false;
	CastSpellProps_t props;

	if ( my->monsterSpecialState == HAUNTED_ARMOR_SPECIAL_CAST1 ) // buff
	{
		if ( local_rng.rand() % 4 )
		{
			myStats->setAttribute("npc_spell_id", std::to_string(SPELL_SHADOW_TAG));
		}
		else
		{
			myStats->setAttribute("npc_spell_id", std::to_string(SPELL_POISON));
		}
		myStats->setAttribute("npc_spell_target", "");
		return;
	}
	else if ( my->monsterSpecialState == HAUNTED_ARMOR_SPECIAL_CAST2 ) // buff
	{
		if ( type == HAUNTED_SILVER )
		{
			/*if ( !myStats->getEffectActive(EFF_NIMBLENESS) )
			{
				options.push_back({ SPELL_PROF_NIMBLENESS, EFF_NIMBLENESS, true });
			}*/
			if ( !(myStats->getEffectActive(EFF_GUARD_BODY) || myStats->getEffectActive(EFF_GUARD_SPIRIT)
				/*|| myStats->getEffectActive(EFF_DIVINE_GUARD)*/) )
			{
				options.push_back({ SPELL_GUARD_BODY, EFF_GUARD_BODY, true });
				//options.push_back({ SPELL_GUARD_SPIRIT, true });
				//options.push_back({ SPELL_DIVINE_GUARD, true });
			}
			options.push_back({ SPELL_HEAL_PULSE, -1, true, true });
		}
		else if ( type == HAUNTED_BLACKIRON )
		{
			if ( !myStats->getEffectActive(EFF_GREATER_MIGHT) )
			{
				options.push_back({ SPELL_PROF_GREATER_MIGHT, EFF_GREATER_MIGHT, true, true });
			}
			if ( !myStats->getEffectActive(EFF_ENVENOM_WEAPON) )
			{
				options.push_back({ SPELL_ENVENOM_WEAPON, EFF_ENVENOM_WEAPON, true });
			}
			options.push_back({ SPELL_MAXIMISE, EFF_MAXIMISE, true, true });
		}
	}

	if ( options.size() )
	{
		auto& pick = options[local_rng.rand() % options.size()];
		spellID = pick.spellID;
		if ( pick.buff )
		{
			setProps = true;
			if ( pick.targetOther )
			{
				props.targetUID = my->getUID();
				if ( true /*(spellID == SPELL_MAXIMISE && myStats->getEffectActive(EFF_MAXIMISE)) 
					|| spellID == SPELL_HEAL_MINOR || spellID == SPELL_PROF_GREATER_MIGHT*/ )
				{
					std::vector<Entity*> targets;
					// search other targets
					node_t* node = nullptr;
					for ( node_t* node = map.creatures->first; node; node = node->next )
					{
						if ( Entity* entity = getSpellTarget(node, 92, my, true, TARGET_FRIEND) )
						{
							if ( pick.effectID >= 0 )
							{
								if ( entity->getStats() )
								{
									if ( entity->getStats()->getEffectActive(pick.effectID) )
									{
										continue;
									}
								}
							}
							if ( entity == my )
							{
								targets.push_back(entity);
							}
							else
							{
								real_t tangent = atan2(entity->y - my->y, entity->x - my->x);
								lineTraceTarget(my, my->x, my->y, tangent, 92.0, 0, false, entity);
								if ( hit.entity == entity )
								{
									targets.push_back(entity);
								}
							}
						}
					}
					if ( targets.size() )
					{
						props.targetUID = targets[local_rng.rand() % targets.size()]->getUID();
					}
				}
			}
		}
		else
		{
			setProps = props.setToMonsterCast(my, spellID);
		}
	}

	if ( setProps )
	{
		myStats->setAttribute("npc_spell_id", std::to_string(spellID));
		myStats->setAttribute("npc_spell_target", std::to_string(props.targetUID));
	}
}

static std::vector<std::map<int, int>> degradeOrder =
{ 
{
	{ LIMB_HUMANOID_LEFTARM, 1 },
	{ LIMB_HUMANOID_RIGHTLEG, 2 },
	{ LIMB_HUMANOID_MASK, 3 },
	{ LIMB_HUMANOID_SHIELD, 4 },
	{ LIMB_HUMANOID_RIGHTARM, 5 },
	{ LIMB_HUMANOID_LEFTLEG, 6 }
},
{
	{ LIMB_HUMANOID_LEFTARM, 1 },
	{ LIMB_HUMANOID_RIGHTLEG, 2 },
	{ LIMB_HUMANOID_RIGHTARM, 3 },
	{ LIMB_HUMANOID_HELMET, 4 },
	{ LIMB_HUMANOID_MASK, 5 },
	{ LIMB_HUMANOID_LEFTLEG, 6 }
},
{
	{ LIMB_HUMANOID_RIGHTLEG, 1 },
	{ LIMB_HUMANOID_LEFTARM, 2 },
	{ LIMB_HUMANOID_LEFTLEG, 3 },
	{ LIMB_HUMANOID_HELMET, 4 },
	{ LIMB_HUMANOID_RIGHTARM, 5 },
	{ LIMB_HUMANOID_TORSO, 6 }
}
};

void hauntedArmorDegrade(Entity* my, Entity* limb, int bodypart, int prevState, int currentState)
{
	if ( !limb || !my ) { return; }
	if ( multiplayer == CLIENT ) { return; }
	Stat* myStats = my->getStats();
	if ( !myStats ) { return; }
	bool fx = false;
	if ( prevState != currentState )
	{
		fx = true;
	}

	int variant = 0;
	if ( myStats->getAttribute("haunted_degrade_type") == "" )
	{
		variant = local_rng.rand() % 3;
		myStats->setAttribute("haunted_degrade_type", std::to_string(variant));
	}
	else
	{
		variant = stoi(myStats->getAttribute("haunted_degrade_type"));
	}

	auto find = degradeOrder[variant].find(bodypart);
	if ( find != degradeOrder[variant].end() )
	{
		if ( currentState >= find->second )
		{
			if ( fx )
			{
				switch ( bodypart )
				{
				case LIMB_HUMANOID_TORSO:
					if ( myStats->breastplate )
					{
						if ( myStats->breastplate->isDroppable )
						{
							if ( dropItemMonster(myStats->breastplate, my, myStats, myStats->breastplate->count) )
							{
								fx = false;
							}
						}
					}
					break;
					case LIMB_HUMANOID_RIGHTLEG:
					{
						if ( degradeOrder[variant].find(LIMB_HUMANOID_LEFTLEG) != degradeOrder[variant].end()
							&& degradeOrder[variant][LIMB_HUMANOID_LEFTLEG] < find->second )
						{
							if ( myStats->shoes )
							{
								if ( myStats->shoes->isDroppable )
								{
									if ( dropItemMonster(myStats->shoes, my, myStats, myStats->shoes->count) )
									{
										fx = false;
									}
								}
							}
						}
						break;
					}
					case LIMB_HUMANOID_LEFTLEG:
					{
						if ( degradeOrder[variant].find(LIMB_HUMANOID_RIGHTLEG) != degradeOrder[variant].end()
							&& degradeOrder[variant][LIMB_HUMANOID_RIGHTLEG] < find->second )
						{
							if ( myStats->shoes )
							{
								if ( myStats->shoes->isDroppable )
								{
									if ( dropItemMonster(myStats->shoes, my, myStats, myStats->shoes->count) )
									{
										fx = false;
									}
								}
							}
						}
						break;
					}
					case LIMB_HUMANOID_RIGHTARM:
					{
						if ( degradeOrder[variant].find(LIMB_HUMANOID_LEFTARM) != degradeOrder[variant].end()
							&& degradeOrder[variant][LIMB_HUMANOID_LEFTARM] < find->second )
						{
							if ( myStats->gloves )
							{
								if ( myStats->gloves->isDroppable )
								{
									if ( dropItemMonster(myStats->gloves, my, myStats, myStats->gloves->count) )
									{
										fx = false;
									}
								}
							}
						}
						break;
					}
					case LIMB_HUMANOID_LEFTARM:
					{
						if ( degradeOrder[variant].find(LIMB_HUMANOID_RIGHTLEG) != degradeOrder[variant].end()
							&& degradeOrder[variant][LIMB_HUMANOID_RIGHTLEG] < find->second )
						{
							if ( myStats->gloves )
							{
								if ( myStats->gloves->isDroppable )
								{
									if ( dropItemMonster(myStats->gloves, my, myStats, myStats->gloves->count) )
									{
										fx = false;
									}
								}
							}
						}
						break;
					}
					case LIMB_HUMANOID_WEAPON:
						if ( myStats->weapon )
						{
							if ( myStats->weapon->isDroppable )
							{
								if ( dropItemMonster(myStats->weapon, my, myStats, myStats->weapon->count) )
								{
									fx = false;
								}
							}
						}
						break;
					case LIMB_HUMANOID_SHIELD:
						if ( myStats->shield )
						{
							if ( myStats->shield->isDroppable )
							{
								if ( dropItemMonster(myStats->shield, my, myStats, myStats->shield->count) )
								{
									fx = false;
								}
							}
						}
						break;
					case LIMB_HUMANOID_CLOAK:
						if ( myStats->mask )
						{
							if ( myStats->mask->isDroppable )
							{
								if ( dropItemMonster(myStats->mask, my, myStats, myStats->mask->count) )
								{
									fx = false;
								}
							}
						}
						break;
					case LIMB_HUMANOID_HELMET:
						if ( myStats->mask )
						{
							if ( myStats->mask->isDroppable )
							{
								if ( dropItemMonster(myStats->mask, my, myStats, myStats->mask->count) )
								{
									fx = false;
								}
							}
						}
						break;
					case LIMB_HUMANOID_MASK:
						if ( myStats->mask )
						{
							if ( myStats->mask->isDroppable )
							{
								if ( dropItemMonster(myStats->mask, my, myStats, myStats->mask->count) )
								{
									fx = false;
								}
							}
						}
						break;
					default:
						break;
				}

			}
			if ( fx && !limb->flags[INVISIBLE] && currentState == find->second )
			{
				//playSoundEntity(my, 76, 64);
				Entity* gib = spawnGib(limb, limb->sprite);
				gib->skill[5] = 1;
				gib->sprite = limb->sprite;
				gib->x = limb->x;
				gib->y = limb->y;
				gib->z = limb->z;
				serverSpawnGibForClient(gib);
			}
			limb->flags[INVISIBLE] = true;
		}
	}
}

#define HUMANWALKSPEED .12
#define HAUNTED_ARMOR_CAST_ANIM body->fskill[5]
#define HAUNTED_ARMOR_BOB body->fskill[6]
void hauntedArmorMoveBodyparts(Entity* my, Stat* myStats, double dist)
{
	node_t* node;
	Entity* entity = nullptr, *entity2 = nullptr;
	Entity* rightbody = nullptr;
	Entity* weaponarm = nullptr;
	int bodypart;
	int hauntedHPState = 0;
	int hauntedPrevState = 0;
	my->flags[INVISIBLE] = true;

	if ( multiplayer != CLIENT )
	{
		// sleeping
		if ( myStats->getEffectActive(EFF_ASLEEP) )
		{
			my->z = 1.5;
			my->pitch = PI / 4;
		}
		else
		{
			my->z = -1;
			if ( my->monsterAttack == 0 )
			{
				my->pitch = 0;
			}
		}

		// levitation
		bool levitating = isLevitating(myStats);
		if ( levitating )
		{
			my->z -= 1; // floating
		}
		my->creatureHandleLiftZ();

		myStats->setEffectActive(EFF_LEVITATING, 1);
		myStats->EFFECTS_TIMERS[EFF_LEVITATING] = 0;
	}

	Entity* shieldarm = nullptr;
	Entity* helmet = nullptr;

	// dummy armor stuff
	static ConsoleVariable<float> cvar_haunted_armor_spd("/haunted_armor_spd", 40.f);
	real_t spd = *cvar_haunted_armor_spd;
	//my->z -= 2.0 + 1.0 * cos(PI + 2 * PI * ((ticks % (int)(spd * 2)) / (spd * 2)));
	//myStats->setEffectActive(EFF_ROOTED, 1);
	/*if ( keystatus[SDLK_g] )
	{
		keystatus[SDLK_g] = 0;
		my->attack(MONSTER_POSE_RANGED_WINDUP3, 0, nullptr);
	}*/
	bool debugModel = monsterDebugModels(my, &dist);
	Entity* body = nullptr;
	HauntedType type = HAUNTED_SILVER;
	if ( myStats && myStats->getAttribute("haunted_type") != "" )
	{
		if ( myStats->getAttribute("haunted_type").find("silver") != std::string::npos )
		{
			type = HAUNTED_SILVER;
		}
		else if ( myStats->getAttribute("haunted_type").find("blackiron") != std::string::npos )
		{
			type = HAUNTED_BLACKIRON;
		}

		real_t percentHP = myStats->HP / (real_t)std::max(1, myStats->MAXHP);

		int hauntedHPState = myStats->getAttribute("haunted_state") != "" ? stoi(myStats->getAttribute("haunted_state")) : 0;
		int hauntedPrevState = hauntedHPState;
		if ( percentHP < 0.2 )
		{
			if ( hauntedHPState < 6 )
			{
				hauntedHPState = 6;
				myStats->setAttribute("haunted_state", "6");
			}
		}
		else if ( percentHP < 0.3 )
		{
			if ( hauntedHPState < 5 )
			{
				hauntedHPState = 5;
				myStats->setAttribute("haunted_state", "5");
			}
		}
		else if ( percentHP < 0.5 )
		{
			if ( hauntedHPState < 4 )
			{
				hauntedHPState = 4;
				myStats->setAttribute("haunted_state", "4");
			}
		}
		else if ( percentHP < 0.65 )
		{
			if ( hauntedHPState < 3 )
			{
				hauntedHPState = 3;
				myStats->setAttribute("haunted_state", "3");
			}
		}
		else if ( percentHP < 0.8 )
		{
			if ( hauntedHPState < 2 )
			{
				hauntedHPState = 2;
				myStats->setAttribute("haunted_state", "2");
			}
		}
		else if ( percentHP < 0.9 )
		{
			if ( hauntedHPState < 1 )
			{
				hauntedHPState = 1;
				myStats->setAttribute("haunted_state", "1");
			}
		}
		else
		{
			myStats->setAttribute("haunted_state", "0");
		}

	}

	// move bodyparts
	for (bodypart = 0, node = my->children.first; node != nullptr; node = node->next, bodypart++)
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

		real_t anim = (ticks % (int)spd) / std::max(1.0, spd);
		real_t anim2 = ((ticks + (int)(spd / 2)) % (int)spd) / std::max(1.0, spd);
		if ( bodypart == LIMB_HUMANOID_TORSO || bodypart == LIMB_HUMANOID_CLOAK )
		{
			entity->z -= 0.5 + 0.5 * cos(PI / 2 + 2 * PI * anim * anim);
			entity->yaw += -sin(PI / 2 + 2 * PI * anim) * PI / 32;

			if ( bodypart == LIMB_HUMANOID_TORSO )
			{
				body = entity;
				if ( my->monsterAttack == MONSTER_POSE_RANGED_WINDUP3 )
				{
					HAUNTED_ARMOR_CAST_ANIM = std::min(1.0, HAUNTED_ARMOR_CAST_ANIM + 0.1);
				}
				else
				{
					HAUNTED_ARMOR_CAST_ANIM = std::max(0.0, HAUNTED_ARMOR_CAST_ANIM - 0.1);
				}

				HAUNTED_ARMOR_BOB = 2.0 + 1.0 * cos(PI + 2 * PI * ((ticks % (int)(spd * 2)) / (spd * 2)));
			}
		}
		else if ( bodypart == LIMB_HUMANOID_MASK || bodypart == LIMB_HUMANOID_HELMET )
		{
			//entity->z -= 0.5 + 0.5 * cos(PI / 4 + 2 * PI * (ticks % 100) / 100.0);
			entity->z -= 1.0 + 0.5 * cos(PI / 4 + PI / 2 + 2 * PI * anim * anim);
		}
		else if ( bodypart == LIMB_HUMANOID_RIGHTARM || bodypart == LIMB_HUMANOID_WEAPON )
		{
			entity->x += (0.5 - 0.25 * sin(2 * PI * anim * anim)) * cos(my->yaw + PI / 2);
			entity->y += (0.5 - 0.25 * sin(2 * PI * anim * anim)) * sin(my->yaw + PI / 2);
			entity->z += 1.0 + 0.5 * cos(PI + PI / 4 + 2 * PI * anim2 * anim2);

			entity->x += (0.5 + (0.5 * -sin(PI / 2 + 2 * PI * anim2))) * cos(my->yaw);
			entity->y += (0.5 + (0.5 * -sin(PI / 2 + 2 * PI * anim2))) * sin(my->yaw);

			if ( body )
			{
				entity->x += HAUNTED_ARMOR_CAST_ANIM * 2.0 * cos(my->yaw + PI / 2);
				entity->y += HAUNTED_ARMOR_CAST_ANIM * 2.0 * sin(my->yaw + PI / 2);
				entity->z -= HAUNTED_ARMOR_CAST_ANIM * 2.0;
			}
		}
		else if ( bodypart == LIMB_HUMANOID_LEFTARM || bodypart == LIMB_HUMANOID_SHIELD )
		{
			entity->x -= (1.0 - 0.5 * sin(2 * PI * anim * anim)) * cos(my->yaw + PI / 2);
			entity->y -= (1.0 - 0.5 * sin(2 * PI * anim * anim)) * sin(my->yaw + PI / 2);
			entity->z += 1.0 + -0.5 * cos(PI / 4 + 2 * PI * anim * anim);

			entity->x += (1.0 + (1.0 * -sin(PI / 2 + 2 * PI * anim))) * cos(my->yaw);
			entity->y += (1.0 + (1.0 * -sin(PI / 2 + 2 * PI * anim))) * sin(my->yaw);

			if ( body )
			{
				entity->x += HAUNTED_ARMOR_CAST_ANIM * 2.0 * cos(my->yaw - PI / 2);
				entity->y += HAUNTED_ARMOR_CAST_ANIM * 2.0 * sin(my->yaw - PI / 2);
				entity->z -= HAUNTED_ARMOR_CAST_ANIM * 2.0;
			}
		}
		else if ( bodypart == LIMB_HUMANOID_LEFTLEG )
		{
			entity->z += 1.0;
			if ( anim2 >= 0.5 )
			{
				entity->z -= 1.0 * sin(2 * PI * (anim2 - 0.5));
			}

			entity->pitch = (PI / 8) * sin(-PI / 16 + anim2 * 2 * PI * anim2);
			entity->x -= 1.5 * cos(PI + 2 * PI * anim2) * cos(my->yaw);
			entity->y -= 1.5 * cos(PI + 2 * PI * anim2) * sin(my->yaw);
		}
		else if ( bodypart == LIMB_HUMANOID_RIGHTLEG )
		{
			entity->z += 1.0;
			if ( anim >= 0.5 )
			{
				entity->z -= 1.0 * sin(2 * PI * (anim - 0.5));
			}

			entity->pitch = (PI / 8) * sin(-PI / 16 + anim * 2 * PI * anim);
			entity->x -= 1.25 * cos(PI + 2 * PI * anim) * cos(my->yaw);
			entity->y -= 1.25 * cos(PI + 2 * PI * anim) * sin(my->yaw);
		}
		/*else if ( bodypart == LIMB_HUMANOID_LEFTLEG )
		{
			entity->x -= 0.25 * cos(my->yaw + PI / 2);
			entity->y -= 0.25 * sin(my->yaw + PI / 2);

			entity->x += (sin(PI * (ticks % 100) / 100.0)) * cos(my->yaw);
			entity->y += (sin(PI * (ticks % 100) / 100.0)) * sin(my->yaw);

			entity->x += 0.05 * cos(my->yaw + PI / 2 + 3 * (PI / 8) - 2 * PI * (ticks % 100) / 100.0);
			entity->y += 0.05 * sin(my->yaw + PI / 2 + 3 * (PI / 8) - 2 * PI * (ticks % 100) / 100.0);
			entity->z += 0.5 + 0.5 * cos(2 * PI * (ticks % 100) / 100.0);
		}
		else if ( bodypart == LIMB_HUMANOID_RIGHTLEG )
		{
			entity->x += 0.25 * cos(my->yaw + PI / 2);
			entity->y += 0.25 * sin(my->yaw + PI / 2);

			entity->x -= (sin(PI * (ticks % 100) / 100.0)) * cos(my->yaw);
			entity->y -= (sin(PI * (ticks % 100) / 100.0)) * sin(my->yaw);

			entity->x -= 0.05 * cos(my->yaw + PI / 2 + 3 * (PI / 8) - 2 * PI * (ticks % 100) / 100.0);
			entity->y -= 0.05 * sin(my->yaw + PI / 2 + 3 * (PI / 8) - 2 * PI * (ticks % 100) / 100.0);
			entity->z += 0.5 + 0.5 * cos(2 * PI * (ticks % 100) / 100.0);
		}*/

		if ( bodypart == LIMB_HUMANOID_RIGHTLEG || bodypart == LIMB_HUMANOID_LEFTARM )
		{
			//my->humanoidAnimateWalk(entity, node, bodypart, HUMANWALKSPEED, dist, 0.4);
		}
		else if ( bodypart == LIMB_HUMANOID_LEFTLEG || bodypart == LIMB_HUMANOID_RIGHTARM || bodypart == LIMB_HUMANOID_CLOAK )
		{
			// left leg, right arm, cloak.
			if ( bodypart == LIMB_HUMANOID_RIGHTARM )
			{
				weaponarm = entity;
				if ( my->monsterAttack > 0 )
				{
					if ( my->monsterAttack == MONSTER_POSE_RANGED_WINDUP3 )
					{
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
								myStats->setEffectActive(EFF_STUNNED, 1);
								myStats->EFFECTS_TIMERS[EFF_STUNNED] = 40;
								hauntedArmorSelectSpell(my, myStats);

								my->monsterLookDir = my->yaw;

								Uint32 targetUID = 0;
								if ( myStats->getAttribute("npc_spell_target") != "" )
								{
									targetUID = std::stol(myStats->getAttribute("npc_spell_target"));
									if ( targetUID != 0 && targetUID != my->getUID() )
									{
										if ( Entity* target = uidToEntity(targetUID) )
										{
											my->lookAtEntity(*target);
										}
									}
								}
							}
						}
						if ( multiplayer != CLIENT )
						{
							// move the head and weapon yaw
							limbAnimateToLimit(my, ANIMATE_PITCH, -0.1, 11 * PI / 6, false, 0.0);
							limbAnimateToLimit(my, ANIMATE_WEAPON_YAW, 0.05, 2 * PI / 8, false, 0.0);
						}
						limbAnimateToLimit(weaponarm, ANIMATE_PITCH, -0.25, 7 * PI / 4, true, 0.0);
						//limbAnimateToLimit(weaponarm, ANIMATE_ROLL, -0.25, 7 * PI / 4, false, 0.0);

						if ( my->monsterAttackTime >= 3 * ANIMATE_DURATION_WINDUP / (monsterGlobalAnimationMultiplier / 10.0) )
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
								CastSpellProps_t props;
								bool setProps = false;
								int spellID = SPELL_NONE;
								if ( myStats->getAttribute("npc_spell_id") != "" )
								{
									spellID = std::stoi(myStats->getAttribute("npc_spell_id"));
									if ( myStats->getAttribute("npc_spell_target") != "" )
									{
										Uint32 targetUid = std::stol(myStats->getAttribute("npc_spell_target"));
										if ( targetUid == 0 )
										{
											props.targetUID = my->getUID();
											setProps = true;

											if ( spellID == SPELL_MAXIMISE && my->hasRangedWeapon(true) )
											{
												spellID = SPELL_MINIMISE;
											}
										}
										else if ( Entity* target = uidToEntity(targetUid) )
										{
											if ( spellID == SPELL_HEAL_MINOR && targetUid != my->getUID() )
											{
												spellID = SPELL_HEAL_OTHER;
											}
											props.targetUID = targetUid;
											setProps = true;

											if ( spellID == SPELL_MAXIMISE && target->hasRangedWeapon(true) )
											{
												spellID = SPELL_MINIMISE;
											}
										}
									}
									else
									{
										setProps = true;
									}
								}
								if ( setProps )
								{
									castSpell(my->getUID(), getSpellFromID(spellID), true, false, false, &props);
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
						else if ( weaponarm->skill[1] >= 1 )
						{
							if ( limbAnimateToLimit(weaponarm, ANIMATE_PITCH, -0.25, 7 * PI / 4, false, 0.0) )
							{
								weaponarm->skill[0] = 0;
								weaponarm->pitch = 0.0;
								my->monsterWeaponYaw = 0;
								weaponarm->roll = 0;
								my->monsterArmbended = 0;
								my->monsterAttack = 0;
							}
						}
					}
					else
					{
						my->handleWeaponArmAttack(weaponarm);
					}
				}
			}
			else if ( bodypart == LIMB_HUMANOID_CLOAK )
			{
				entity->pitch = entity->fskill[0];
			}

			//my->humanoidAnimateWalk(entity, node, bodypart, HUMANWALKSPEED, dist, 0.4);

			if ( bodypart == LIMB_HUMANOID_CLOAK )
			{
				entity->fskill[0] = entity->pitch;
				entity->roll = my->roll - fabs(entity->pitch) / 2;
				entity->pitch = 0;
			}
		}

		if ( body )
		{
			entity->z -= HAUNTED_ARMOR_BOB;
		}

		switch ( bodypart )
		{
			// torso
			case LIMB_HUMANOID_TORSO:
				entity->scalex = 1.0;
				entity->scaley = 1.0;
				entity->scalez = 1.0;
				entity->focalx = limbs[HAUNTED_ARMOR][1][0];
				entity->focaly = limbs[HAUNTED_ARMOR][1][1];
				entity->focalz = limbs[HAUNTED_ARMOR][1][2];
				if ( multiplayer != CLIENT )
				{
					if ( myStats->breastplate == nullptr || !itemModel(myStats->breastplate, false, my) )
					{
						entity->sprite = 2469;
						entity->flags[INVISIBLE] = true;
					}
					else
					{
						entity->sprite = itemModel(myStats->breastplate, false, my);
						entity->flags[INVISIBLE] = false;
					}

					hauntedArmorDegrade(my, entity, bodypart, hauntedPrevState, hauntedHPState);

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

				my->setHumanoidLimbOffset(entity, HAUNTED_ARMOR, LIMB_HUMANOID_TORSO);
				break;
			// right leg
			case LIMB_HUMANOID_RIGHTLEG:
				if ( multiplayer != CLIENT )
				{
					if ( myStats->shoes == nullptr )
					{
						entity->sprite = 2475;
						entity->flags[INVISIBLE] = true;
					}
					else
					{
						my->setBootSprite(entity, SPRITE_BOOT_RIGHT_OFFSET);
						entity->flags[INVISIBLE] = false;
					}

					hauntedArmorDegrade(my, entity, bodypart, hauntedPrevState, hauntedHPState);

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

				my->setHumanoidLimbOffset(entity, HAUNTED_ARMOR, LIMB_HUMANOID_RIGHTLEG);
				break;
			// left leg
			case LIMB_HUMANOID_LEFTLEG:
				if ( multiplayer != CLIENT )
				{
					if ( myStats->shoes == nullptr )
					{
						entity->sprite = 2474;
						entity->flags[INVISIBLE] = true;
					}
					else
					{
						my->setBootSprite(entity, SPRITE_BOOT_LEFT_OFFSET);
						entity->flags[INVISIBLE] = false;
					}

					hauntedArmorDegrade(my, entity, bodypart, hauntedPrevState, hauntedHPState);

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

				my->setHumanoidLimbOffset(entity, HAUNTED_ARMOR, LIMB_HUMANOID_LEFTLEG);
				break;
			// right arm
			case LIMB_HUMANOID_RIGHTARM:
			{
				if ( multiplayer != CLIENT )
				{
					entity->flags[INVISIBLE] = true;
					if ( myStats->gloves == nullptr )
					{
						entity->sprite = 2472;
					}
					else
					{
						if ( setGloveSprite(myStats, entity, SPRITE_GLOVE_RIGHT_OFFSET) != 0 )
						{
							// successfully set sprite for the model
							entity->flags[INVISIBLE] = false;
						}
					}

					hauntedArmorDegrade(my, entity, bodypart, hauntedPrevState, hauntedHPState);

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

				node_t* tempNode = list_Node(&my->children, LIMB_HUMANOID_WEAPON);
				if ( tempNode )
				{
					Entity* weapon = (Entity*)tempNode->element;
					if ( MONSTER_ARMBENDED || (weapon->flags[INVISIBLE] && my->monsterState != MONSTER_STATE_ATTACK) )
					{
						// if weapon invisible and I'm not attacking, relax arm.
						entity->focalx = limbs[HAUNTED_ARMOR][4][0]; // 0
						entity->focaly = limbs[HAUNTED_ARMOR][4][1]; // 0
						entity->focalz = limbs[HAUNTED_ARMOR][4][2]; // 1.5
					}
					else
					{
						// else flex arm.
						entity->focalx = limbs[HAUNTED_ARMOR][4][0] + 0.75;
						entity->focaly = limbs[HAUNTED_ARMOR][4][1];
						entity->focalz = limbs[HAUNTED_ARMOR][4][2] - 0.75;
						entity->sprite += 2;
					}
				}

				my->setHumanoidLimbOffset(entity, HAUNTED_ARMOR, LIMB_HUMANOID_RIGHTARM);

				if ( my->monsterAttack == MONSTER_POSE_MELEE_WINDUP1 || my->monsterAttack == MONSTER_POSE_MELEE_WINDUP2
					|| my->monsterAttack == MONSTER_POSE_MELEE_WINDUP3 )
				{
					entity->fskill[5] = std::min(1.0, entity->fskill[5] + 0.1);
				}
				else
				{
					entity->fskill[5] = std::max(0.0, entity->fskill[5] - 0.1);
				}

				entity->x += 12.0 * sin(PI * 0.5 * entity->fskill[5]) * cos(my->yaw);
				entity->y += 12.0 * sin(PI * 0.5 * entity->fskill[5]) * sin(my->yaw);

				entity->yaw += MONSTER_WEAPONYAW;

				if ( ticks % 4 == 2 )
				{
					Entity* fx = spawnMagicParticleCustom(entity, 96, 0.5, 1.0);
					fx->vel_x = 0.25 * cos(my->yaw + PI / 2 + PI / 4);
					fx->vel_y = 0.25 * sin(my->yaw + PI / 2 + PI / 4);
					fx->vel_z = -0.05;
					fx->z -= 1.0;
					fx->flags[SPRITE] = true;
					fx->ditheringDisabled = true;
				}
				break;
			}
			// left arm
			case LIMB_HUMANOID_LEFTARM:
			{
				shieldarm = entity;
				if ( multiplayer != CLIENT )
				{
					entity->flags[INVISIBLE] = true;
					if ( myStats->gloves == nullptr )
					{
						entity->sprite = 2474;
					}
					else
					{
						if ( setGloveSprite(myStats, entity, SPRITE_GLOVE_LEFT_OFFSET) != 0 )
						{
							// successfully set sprite for the model
							entity->flags[INVISIBLE] = false;
						}
					}

					hauntedArmorDegrade(my, entity, bodypart, hauntedPrevState, hauntedHPState);

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

				node_t* tempNode = list_Node(&my->children, LIMB_HUMANOID_SHIELD);
				if ( tempNode )
				{
					Entity* shield = (Entity*)tempNode->element;
					if ( shield->flags[INVISIBLE] && (my->monsterState != MONSTER_STATE_ATTACK) )
					{
						// if shield invisible and I'm not attacking, relax arm.
						entity->focalx = limbs[HAUNTED_ARMOR][5][0]; // 0
						entity->focaly = limbs[HAUNTED_ARMOR][5][1]; // 0
						entity->focalz = limbs[HAUNTED_ARMOR][5][2]; // 1.5
					}
					else
					{
						// else flex arm.
						entity->focalx = limbs[HAUNTED_ARMOR][5][0] + 0.75;
						entity->focaly = limbs[HAUNTED_ARMOR][5][1];
						entity->focalz = limbs[HAUNTED_ARMOR][5][2] - 0.75;
						entity->sprite += 2;
					}
				}
				my->setHumanoidLimbOffset(entity, HAUNTED_ARMOR, LIMB_HUMANOID_LEFTARM);
				if ( my->monsterDefend && my->monsterAttack == 0 )
				{
					MONSTER_SHIELDYAW = PI / 5;
				}
				else
				{
					MONSTER_SHIELDYAW = 0;
				}
				entity->yaw += MONSTER_SHIELDYAW;

				if ( ticks % 4 == 0 )
				{
					Entity* fx = spawnMagicParticleCustom(entity, 96, 0.5, 1.0);
					fx->vel_x = 0.25 * cos(my->yaw - PI / 2 - PI / 4);
					fx->vel_y = 0.25 * sin(my->yaw - PI / 2 - PI / 4);
					fx->vel_z = -0.05;
					fx->z -= 1.0;
					fx->flags[SPRITE] = true;
					fx->ditheringDisabled = true;
				}
				break;
			}
			// weapon
			case LIMB_HUMANOID_WEAPON:
				if ( multiplayer != CLIENT )
				{
					if ( myStats->weapon == nullptr ) //TODO: isInvisible()?
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

					hauntedArmorDegrade(my, entity, bodypart, hauntedPrevState, hauntedHPState);

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
				if ( multiplayer != CLIENT )
				{
					if ( myStats->cloak == nullptr ) //TODO: isInvisible()?
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
				entity->focalx = limbs[HAUNTED_ARMOR][9][0]; // 0
				entity->focaly = limbs[HAUNTED_ARMOR][9][1]; // 0
				entity->focalz = limbs[HAUNTED_ARMOR][9][2]; // -1.75
				entity->pitch = my->pitch;
				entity->roll = 0;
				if ( multiplayer != CLIENT )
				{
					entity->sprite = itemModel(myStats->helmet);
					if ( myStats->helmet == nullptr ) //TODO: isInvisible()?
					{
						entity->flags[INVISIBLE] = true;
					}
					else
					{
						entity->flags[INVISIBLE] = false;
					}

					hauntedArmorDegrade(my, entity, bodypart, hauntedPrevState, hauntedHPState);

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


				if ( ticks % 2 == 0 )
				{
					Entity* fx = spawnMagicParticleCustom(entity, 96, 0.5, 1.0);
					fx->vel_x = 0.25 * cos(entity->yaw + PI);
					fx->vel_y = 0.25 * sin(entity->yaw + PI);
					fx->vel_z = -0.1;
					fx->z -= 1.0;
					fx->flags[SPRITE] = true;
					fx->ditheringDisabled = true;
				}
				break;
			// mask
			case LIMB_HUMANOID_MASK:
				entity->focalx = limbs[HAUNTED_ARMOR][10][0]; // 0
				entity->focaly = limbs[HAUNTED_ARMOR][10][1]; // 0
				entity->focalz = limbs[HAUNTED_ARMOR][10][2]; // .5
				entity->pitch = my->pitch;
				entity->roll = PI / 2;
				if ( multiplayer != CLIENT )
				{
					if ( myStats->mask == nullptr ) //TODO: isInvisible()?
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

					hauntedArmorDegrade(my, entity, bodypart, hauntedPrevState, hauntedHPState);

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

				if ( entity->sprite != 165 && entity->sprite != 1196 )
				{
					if ( entity->sprite == items[MASK_SHAMAN].index )
					{
						entity->roll = 0;
						my->setHelmetLimbOffset(entity);
						my->setHelmetLimbOffsetWithMask(helmet, entity);
					}
					else if ( EquipmentModelOffsets.modelOffsetExists(HAUNTED_ARMOR, entity->sprite, my->sprite) )
					{
						my->setHelmetLimbOffset(entity);
						my->setHelmetLimbOffsetWithMask(helmet, entity);
					}
					else
					{
						entity->focalx = limbs[HAUNTED_ARMOR][10][0] + .35; // .35
						entity->focaly = limbs[HAUNTED_ARMOR][10][1] - 2; // -2
						entity->focalz = limbs[HAUNTED_ARMOR][10][2]; // .5
					}
				}
				else
				{
					entity->focalx = limbs[HAUNTED_ARMOR][10][0] + .25; // .25
					entity->focaly = limbs[HAUNTED_ARMOR][10][1] - 2.25; // -2.25
					entity->focalz = limbs[HAUNTED_ARMOR][10][2]; // .5
				}
				break;
		}

	}
	// rotate shield a bit
	node_t* shieldNode = list_Node(&my->children, LIMB_HUMANOID_SHIELD);
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