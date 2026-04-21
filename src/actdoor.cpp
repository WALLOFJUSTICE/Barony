/*-------------------------------------------------------------------------------

	BARONY
	File: actdoor.cpp
	Desc: behavior function for doors

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "entity.hpp"
#include "engine/audio/sound.hpp"
#include "net.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "interface/interface.hpp"
#include "items.hpp"
#include "prng.hpp"
#include "mod_tools.hpp"

/*-------------------------------------------------------------------------------

	act*

	The following function describes an entity behavior. The function
	takes a pointer to the entity that uses it as an argument.

-------------------------------------------------------------------------------*/

void actDoor(Entity* my)
{
	if (!my)
	{
		return;
	}

	Entity* entity;
	int i, c;

	auto& rng = my->entity_rng ? *my->entity_rng : local_rng;

	if ( !my->doorInit )
	{
		my->createWorldUITooltip();

		my->doorInit = 1;
		my->doorStartAng = my->yaw;
		my->doorHealth = 15 + rng.rand() % 5;
		my->doorMaxHealth = my->doorHealth;
		my->doorOldHealth = my->doorHealth;
		my->doorPreventLockpickExploit = 1;
		my->doorLockpickHealth = 20;
		if ( my->doorForceLockedUnlocked == 2 )
		{
			my->doorLocked = 0; // force unlocked.
		}
		else if ( rng.rand() % 20 == 0 || (!strncmp(map.name, "The Great Castle", 16) && rng.rand() % 2 == 0) || my->doorForceLockedUnlocked == 1 )   // 5% chance
		{
			my->doorLocked = 1;
			my->doorPreventLockpickExploit = 0;
		}
		my->doorOldStatus = my->doorStatus;
		my->scalex = 1.01;
		my->scaley = 1.01;
		my->scalez = 1.01;
		my->flags[BURNABLE] = true;
	}
	else
	{
		if ( multiplayer != CLIENT )
		{
			// burning
			if ( my->flags[BURNING] )
			{
				if ( ticks % 30 == 0 )
				{
					my->doorHealth--;
				}
			}

			my->doorOldHealth = my->doorHealth;

			// door mortality :p
			if ( my->doorHealth <= 0 )
			{
				for ( c = 0; c < 5; c++ )
				{
					entity = spawnGib(my);
					entity->flags[INVISIBLE] = false;
					entity->sprite = 187; // Splinter.vox
					entity->x = floor(my->x / 16) * 16 + 8;
					entity->y = floor(my->y / 16) * 16 + 8;
					entity->z = 0;
					entity->z += -7 + local_rng.rand() % 14;
					if ( !my->doorDir )
					{
						// horizontal door
						entity->y += -4 + local_rng.rand() % 8;
						if ( my->doorSmacked )
						{
							entity->yaw = PI;
						}
						else
						{
							entity->yaw = 0;
						}
					}
					else
					{
						// vertical door
						entity->x += -4 + local_rng.rand() % 8;
						if ( my->doorSmacked )
						{
							entity->yaw = PI / 2;
						}
						else
						{
							entity->yaw = 3 * PI / 2;
						}
					}
					entity->pitch = (local_rng.rand() % 360) * PI / 180.0;
					entity->roll = (local_rng.rand() % 360) * PI / 180.0;
					entity->vel_x = cos(entity->yaw) * (1.2 + (local_rng.rand() % 10) / 50.0);
					entity->vel_y = sin(entity->yaw) * (1.2 + (local_rng.rand() % 10) / 50.0);
					entity->vel_z = -.25;
					entity->fskill[3] = 0.04;
					serverSpawnGibForClient(entity);
				}
				playSoundEntity(my, 177, 64);
				list_RemoveNode(my->mynode);
				return;
			}

			// using door
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if ( selectedEntity[i] == my || client_selected[i] == my )
				{
					if ( Player::getPlayerInteractEntity(i) && inrange[i])
					{
						Entity* playerEntity = Player::getPlayerInteractEntity(i);
						if ( !my->doorLocked )   // door unlocked
						{
							if ( !my->doorDir && !my->doorStatus )
							{
								// open door
								my->doorStatus = 1 + (playerEntity->x > my->x);
								playSoundEntity(my, 21, 96);
								messagePlayer(i, MESSAGE_INTERACTION, Language::get(464));
								Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_DOOR_OPENED, "door", 1);
							}
							else if ( my->doorDir && !my->doorStatus )
							{
								// open door
								my->doorStatus = 1 + (playerEntity->y < my->y);
								playSoundEntity(my, 21, 96);
								messagePlayer(i, MESSAGE_INTERACTION, Language::get(464));
								Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_DOOR_OPENED, "door", 1);
							}
							else
							{
								// close door
								my->doorStatus = 0;
								playSoundEntity(my, 22, 96);
								messagePlayer(i, MESSAGE_INTERACTION, Language::get(465));
								Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_DOOR_CLOSED, "door", 1);
							}
						}
						else
						{
							// door locked
							messagePlayer(i, MESSAGE_INTERACTION, Language::get(466));
							playSoundEntity(my, 152, 64);
						}
					}
				}
			}
		}

		// door swinging
		if ( !my->doorStatus )
		{
			// closing door
			if ( my->yaw > my->doorStartAng )
			{
				my->yaw = std::max(my->doorStartAng, my->yaw - 0.15);
			}
			else if ( my->yaw < my->doorStartAng )
			{
				my->yaw = std::min(my->doorStartAng, my->yaw + 0.15);
			}
		}
		else
		{
			// opening door
			if ( my->doorStatus == 1 )
			{
				if ( my->yaw > my->doorStartAng + PI / 2 )
				{
					my->yaw = std::max(my->doorStartAng + PI / 2, my->yaw - 0.15);
				}
				else if ( my->yaw < my->doorStartAng + PI / 2 )
				{
					my->yaw = std::min(my->doorStartAng + PI / 2, my->yaw + 0.15);
				}
			}
			else if ( my->doorStatus == 2 )
			{
				if ( my->yaw > my->doorStartAng - PI / 2 )
				{
					my->yaw = std::max(my->doorStartAng - PI / 2, my->yaw - 0.15);
				}
				else if ( my->yaw < my->doorStartAng - PI / 2 )
				{
					my->yaw = std::min(my->doorStartAng - PI / 2, my->yaw + 0.15);
				}
			}
		}

		// setting collision
		if ( my->yaw == my->doorStartAng && my->flags[PASSABLE] )
		{
			// don't set impassable if someone's inside, otherwise do
			node_t* node;
			bool somebodyinside = false;
			std::vector<list_t*> entLists;
			if ( multiplayer == CLIENT )
			{
				entLists.push_back(map.entities); // clients use old map.entities method
			}
			else
			{
				entLists = TileEntityList.getEntitiesWithinRadiusAroundEntity(my, 2);
			}
			real_t oldmyx = my->x;
			real_t oldmyy = my->y;
			my->x = (static_cast<int>(my->x) >> 4) * 16.0 + 8.0; // door positioning isn't centred on tile so adjust
			my->y = (static_cast<int>(my->y) >> 4) * 16.0 + 8.0;
			for ( std::vector<list_t*>::iterator it = entLists.begin(); it != entLists.end() && !somebodyinside; ++it )
			{
				list_t* currentList = *it;
				for ( node = currentList->first; node != nullptr; node = node->next )
				{
					Entity* entity = (Entity*)node->element;
					if ( entity == my || (entity->flags[PASSABLE] && entity->behavior != &actDeathGhost) 
						|| entity->behavior == &actDoorFrame )
					{
						continue;
					}

					bool insideEntity = false;
					if ( entity->behavior == &actDoor || entity->behavior == &actIronDoor )
					{
						real_t oldx = entity->x;
						real_t oldy = entity->y;
						entity->x = (static_cast<int>(entity->x) >> 4) * 16.0 + 8.0; // door positioning isn't centred on tile so adjust
						entity->y = (static_cast<int>(entity->y) >> 4) * 16.0 + 8.0;
						insideEntity = entityInsideEntity(my, entity);
						entity->x = oldx;
						entity->y = oldy;
					}
					else
					{
						insideEntity = entityInsideEntity(my, entity);
					}

					if ( insideEntity )
					{
						somebodyinside = true;
						break;
					}
				}
			}
			my->x = oldmyx;
			my->y = oldmyy;
			if ( !somebodyinside )
			{
				my->focaly = 0;
				if ( my->doorStartAng == 0 )
				{
					my->y -= 5;
				}
				else
				{
					my->x -= 5;
				}
				my->flags[PASSABLE] = false;
			}
		}
		else if ( my->yaw != my->doorStartAng && !my->flags[PASSABLE] )
		{
			my->focaly = -5;
			if ( my->doorStartAng == 0 )
			{
				my->y += 5;
			}
			else
			{
				my->x += 5;
			}
			my->flags[PASSABLE] = true;
		}

		// update for clients
		if ( multiplayer == SERVER )
		{
			if ( my->doorOldStatus != my->doorStatus )
			{
				my->doorOldStatus = my->doorStatus;
				serverUpdateEntitySkill(my, 3);
			}
		}
	}
}

void actDoorFrame(Entity* my)
{
	// dummy function
	// intended to make it easier
	// to determine whether an entity
	// is part of a door frame
	if ( my->flags[INVISIBLE] == false )
	{
		my->flags[PASSABLE] = true; // the actual frame should ALWAYS be passable
	}
}

void Entity::doorHandleDamageMagic(int damage, Entity &magicProjectile, Entity *caster, bool messages, bool doSound)
{
	if ( behavior == &::actIronDoor )
	{
		damage = 0;
	}
	updateEntityOldHPBeforeMagicHit(*this, magicProjectile);
	doorHealth -= damage; //Decrease door health.
	if ( caster )
	{
		if ( caster->behavior == &actPlayer )
		{
			if ( doorHealth <= 0 )
			{
				if ( messages )
				{
					if ( magicProjectile.behavior == &actBomb )
					{
						messagePlayer(caster->skill[2], MESSAGE_COMBAT, Language::get(3617), items[magicProjectile.skill[21]].getIdentifiedName(), 
							behavior == &::actIronDoor ? Language::get(6414) : Language::get(674));
					}
					else
					{
						messagePlayer(caster->skill[2], MESSAGE_COMBAT, Language::get(387));
					}
				}
				Compendium_t::Events_t::eventUpdateWorld(caster->skill[2], Compendium_t::CPDM_DOOR_BROKEN, "door", 1);

				if ( doorOldHealth > 0 )
				{
					players[caster->skill[2]]->mechanics.incrementBreakableCounter(Player::PlayerMechanics_t::BreakableEvent::GBREAK_COMMON, this);
				}
			}
			else if ( damage > 0 )
			{
				if ( messages )
				{
					if ( magicProjectile.behavior == &actBomb )
					{
						messagePlayer(caster->skill[2], MESSAGE_COMBAT_BASIC, Language::get(3618), items[magicProjectile.skill[21]].getIdentifiedName(), 
							behavior == &::actIronDoor ? Language::get(6414) : Language::get(674));
					}
					else
					{
						messagePlayer(caster->skill[2], MESSAGE_COMBAT_BASIC, Language::get(378), 
							behavior == &::actIronDoor ? Language::get(6414) : Language::get(674));
					}
				}
			}
		}
		updateEnemyBar(caster, this, behavior == &::actIronDoor ? Language::get(6414) : Language::get(674), doorHealth, doorMaxHealth,
			false, DamageGib::DMG_DEFAULT);
	}
	if ( !doorDir )
	{
		doorSmacked = (magicProjectile.x > this->x);
	}
	else
	{
		doorSmacked = (magicProjectile.y < this->y);
	}
	if ( doSound )
	{
		playSoundEntity(this, 28, 128);
	}
}

void actIronDoor(Entity* my)
{
	if ( my )
	{
		my->actIronDoor();
	}
}

void Entity::actIronDoor()
{
	Entity* entity;
	int i, c;

	auto& rng = entity_rng ? *entity_rng : local_rng;

	if ( !doorInit )
	{
		createWorldUITooltip();

		doorInit = 1;
		doorStartAng = yaw;
		doorHealth = 100;
		doorMaxHealth = doorHealth;
		doorOldHealth = doorHealth;
		doorPreventLockpickExploit = 1;
		doorLockpickHealth = 50;
		if ( doorForceLockedUnlocked == 2 )
		{
			doorLocked = 0; // force unlocked.
		}
		else if ( doorForceLockedUnlocked == 1 )
		{
			doorLocked = 1;
			doorPreventLockpickExploit = 0;
		}
		else if ( doorForceLockedUnlocked <= 0 )
		{
			if ( rng.rand() % 20 == 0 )
			{
				// locked
				doorLocked = 1;
				doorPreventLockpickExploit = 0;
			}
			else
			{
				doorLocked = 0; // force unlocked.
			}
		}
		doorOldStatus = doorStatus;
		scalex = 1.01;
		scaley = 1.01;
		scalez = 1.01;
		flags[BURNABLE] = false;
	}
	else
	{
		if ( multiplayer != CLIENT )
		{
			doorOldHealth = doorHealth;

			// door mortality :p
			if ( doorHealth <= 0 )
			{
				for ( c = 0; c < 5 && false; c++ )
				{
					entity = spawnGib(this);
					entity->flags[INVISIBLE] = false;
					entity->sprite = 187; // Splinter.vox
					entity->x = floor(x / 16) * 16 + 8;
					entity->y = floor(y / 16) * 16 + 8;
					entity->z = 0;
					entity->z += -7 + local_rng.rand() % 14;
					if ( !doorDir )
					{
						// horizontal door
						entity->y += -4 + local_rng.rand() % 8;
						if ( doorSmacked )
						{
							entity->yaw = PI;
						}
						else
						{
							entity->yaw = 0;
						}
					}
					else
					{
						// vertical door
						entity->x += -4 + local_rng.rand() % 8;
						if ( doorSmacked )
						{
							entity->yaw = PI / 2;
						}
						else
						{
							entity->yaw = 3 * PI / 2;
						}
					}
					entity->pitch = (local_rng.rand() % 360) * PI / 180.0;
					entity->roll = (local_rng.rand() % 360) * PI / 180.0;
					entity->vel_x = cos(entity->yaw) * (1.2 + (local_rng.rand() % 10) / 50.0);
					entity->vel_y = sin(entity->yaw) * (1.2 + (local_rng.rand() % 10) / 50.0);
					entity->vel_z = -.25;
					entity->fskill[3] = 0.04;
					serverSpawnGibForClient(entity);
				}
				playSoundEntity(this, 76, 64);
				list_RemoveNode(mynode);
				return;
			}

			if ( doorUnlockWhenPowered == 1 )
			{
				if ( circuit_status == CIRCUIT_ON )
				{
					if ( doorLocked == 1 )
					{
						doorLocked = 0;
						playSoundEntity(this, 91, 64);
					}
				}
				else if ( circuit_status == CIRCUIT_OFF )
				{
					if ( doorStatus == 0 ) // closed
					{
						if ( doorLocked == 0 )
						{
							doorLocked = 1;
							playSoundEntity(this, 57, 64);
						}
					}
				}
			}

			// using door
			for ( i = 0; i < MAXPLAYERS; i++ )
			{
				if ( selectedEntity[i] == this || client_selected[i] == this )
				{
					if ( Player::getPlayerInteractEntity(i) && inrange[i] )
					{
						Entity* playerEntity = Player::getPlayerInteractEntity(i);
						if ( !doorLocked )   // door unlocked
						{
							if ( !doorDir && !doorStatus )
							{
								// open door
								doorStatus = 1 + (playerEntity->x > x);
								playSoundEntity(this, 21, 96);
								messagePlayer(i, MESSAGE_INTERACTION, Language::get(6404));
								Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_DOOR_OPENED, "iron door", 1);
							}
							else if ( doorDir && !doorStatus )
							{
								// open door
								doorStatus = 1 + (playerEntity->y < y);
								playSoundEntity(this, 21, 96);
								messagePlayer(i, MESSAGE_INTERACTION, Language::get(6404));
								Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_DOOR_OPENED, "iron door", 1);
							}
							else
							{
								// close door
								doorStatus = 0;
								playSoundEntity(this, 22, 96);
								if ( doorUnlockWhenPowered == 1 && circuit_status == CIRCUIT_OFF )
								{
									doorLocked = 1;
									playSoundEntity(this, 57, 64);
									messagePlayer(i, MESSAGE_INTERACTION, Language::get(6406));
								}
								else
								{
									messagePlayer(i, MESSAGE_INTERACTION, Language::get(6405));
								}
								Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_DOOR_CLOSED, "iron door", 1);
							}
						}
						else
						{
							// door locked
							messagePlayer(i, MESSAGE_INTERACTION, Language::get(6407));
							playSoundEntity(this, 152, 64);
						}
					}
				}
			}
		}

		// door swinging
		static const real_t doorSpeed = 0.15;
		if ( !doorStatus )
		{
			// closing door
			if ( yaw > doorStartAng )
			{
				yaw = std::max(doorStartAng, yaw - 0.15);
			}
			else if ( yaw < doorStartAng )
			{
				yaw = std::min(doorStartAng, yaw + 0.15);
			}
		}
		else
		{
			// opening door
			if ( doorStatus == 1 )
			{
				if ( yaw > doorStartAng + PI / 2 )
				{
					yaw = std::max(doorStartAng + PI / 2, yaw - doorSpeed);
				}
				else if ( yaw < doorStartAng + PI / 2 )
				{
					yaw = std::min(doorStartAng + PI / 2, yaw + doorSpeed);
				}
			}
			else if ( doorStatus == 2 )
			{
				if ( yaw > doorStartAng - PI / 2 )
				{
					yaw = std::max(doorStartAng - PI / 2, yaw - doorSpeed);
				}
				else if ( yaw < doorStartAng - PI / 2 )
				{
					yaw = std::min(doorStartAng - PI / 2, yaw + doorSpeed);
				}
			}
		}

		// setting collision
		if ( yaw == doorStartAng && flags[PASSABLE] )
		{
			// don't set impassable if someone's inside, otherwise do
			node_t* node;
			bool somebodyinside = false;
			std::vector<list_t*> entLists;
			if ( multiplayer == CLIENT )
			{
				entLists.push_back(map.entities); // clients use old map.entities method
			}
			else
			{
				entLists = TileEntityList.getEntitiesWithinRadiusAroundEntity(this, 2);
			}
			real_t oldmyx = x;
			real_t oldmyy = y;
			x = (static_cast<int>(x) >> 4) * 16.0 + 8.0; // door positioning isn't centred on tile so adjust
			y = (static_cast<int>(y) >> 4) * 16.0 + 8.0;
			for ( std::vector<list_t*>::iterator it = entLists.begin(); it != entLists.end() && !somebodyinside; ++it )
			{
				list_t* currentList = *it;
				for ( node = currentList->first; node != nullptr; node = node->next )
				{
					Entity* entity = (Entity*)node->element;
					if ( entity == this || (entity->flags[PASSABLE] && entity->behavior != &actDeathGhost)
						|| entity->behavior == &actDoorFrame )
					{
						continue;
					}

					bool insideEntity = false;
					if ( entity->behavior == &actDoor || entity->behavior == &::actIronDoor )
					{
						real_t oldx = entity->x;
						real_t oldy = entity->y;
						entity->x = (static_cast<int>(entity->x) >> 4) * 16.0 + 8.0; // door positioning isn't centred on tile so adjust
						entity->y = (static_cast<int>(entity->y) >> 4) * 16.0 + 8.0;
						insideEntity = entityInsideEntity(this, entity);
						entity->x = oldx;
						entity->y = oldy;
					}
					else
					{
						insideEntity = entityInsideEntity(this, entity);
					}

					if ( insideEntity )
					{
						somebodyinside = true;
						break;
					}
				}
			}
			x = oldmyx;
			y = oldmyy;
			if ( !somebodyinside )
			{
				focaly = 0;
				if ( doorStartAng == 0 )
				{
					y -= 5;
				}
				else
				{
					x -= 5;
				}
				flags[PASSABLE] = false;
			}
		}
		else if ( yaw != doorStartAng && !flags[PASSABLE] )
		{
			focaly = -5;
			if ( doorStartAng == 0 )
			{
				y += 5;
			}
			else
			{
				x += 5;
			}
			flags[PASSABLE] = true;
		}

		// update for clients
		if ( multiplayer == SERVER )
		{
			if ( doorOldStatus != doorStatus )
			{
				doorOldStatus = doorStatus;
				serverUpdateEntitySkill(this, 3);
			}
		}
	}
}

void actForcefield(Entity* my)
{
	/*if ( PARTICLE_LIFE < 0 )
	{
		createParticleDemesneDoor(my->x, my->y, my->yaw);
		serverSpawnMiscParticlesAtLocation(my->x, my->y, my->yaw * 256.0, PARTICLE_EFFECT_DEMESNE_DOOR, 0);
		my->removeLightField();
		list_RemoveNode(my->mynode);
		if ( Entity* caster = uidToEntity(my->parent) )
		{
			messagePlayer(caster->isEntityPlayer(), MESSAGE_WORLD, Language::get(6691));
		}
		return;
	}*/

	if ( !my->light )
	{
		my->light = addLight(my->x / 16, my->y / 16, "demesne_door");
	}

	if ( my->skill[1] == 0 )
	{
		my->skill[1] = 1;
		createParticleDemesneDoor(my->x, my->y, my->yaw);
	}

	my->ditheringOverride = 4;

	/*if ( multiplayer != CLIENT )
	{
		--PARTICLE_LIFE;

		int mapx = my->x / 16;
		int mapy = my->y / 16;
		bool interrupted = false;
		auto entLists = TileEntityList.getEntitiesWithinRadiusAroundEntity(my, 1);
		for ( auto it : entLists )
		{
			node_t* node;
			for ( node = it->first; node != nullptr; node = node->next )
			{
				if ( Entity* entity = (Entity*)node->element )
				{
					if ( static_cast<int>(entity->x / 16) == mapx && static_cast<int>(entity->y / 16) == mapy )
					{
						if ( entity == my ) { continue; }
						if ( entity->behavior == &actDoor )
						{
							entity->doorHealth = 0;
						}
						if ( entity->behavior == &actGate && entity->gateStatus == 0 )
						{
							interrupted = true;
						}
						if ( entity->behavior == &actIronDoor && entity->doorStatus == 0 )
						{
							interrupted = true;
						}
						if ( entity->behavior == &actParticleDemesneDoor )
						{
							interrupted = true;
						}
						if ( entity->behavior == &actPlayer || entity->behavior == &actMonster )
						{
							if ( Stat* stats = entity->getStats() )
							{
								if ( stats->type != VAMPIRE )
								{
									if ( entityInsideEntity(my, entity) )
									{
										if ( auto hitProps = getParticleEmitterHitProps(my->getUID(), entity) )
										{
											if ( hitProps->hits == 0 )
											{
												hitProps->hits++;
												hitProps->tick = ticks;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			for ( auto& hitProp : particleTimerEmitterHitEntities[my->getUID()] )
			{
				if ( hitProp.second.hits == 1 )
				{
					if ( Entity* entity = uidToEntity(hitProp.first) )
					{
						if ( Stat* stats = entity->getStats() )
						{
							if ( stats->type != VAMPIRE )
							{
								if ( !entityInsideEntity(my, entity) )
								{
									hitProp.second.hits++;
									hitProp.second.tick = ticks;
									Entity* caster = uidToEntity(my->parent);
									if ( caster && (caster == entity || caster->checkFriend(entity)) )
									{
										int effectStrength = std::min(255,
											std::min(getSpellDamageSecondaryFromID(SPELL_DEMESNE_DOOR, caster, nullptr, my),
												std::max(1, getSpellDamageFromID(SPELL_DEMESNE_DOOR, caster, nullptr, my))));
										if ( entity->setEffect(EFF_DEMESNE_DOOR, (Uint8)effectStrength,
											getSpellEffectDurationSecondaryFromID(SPELL_DEMESNE_DOOR, caster, nullptr, my), false) )
										{
											magicOnSpellCastEvent(caster, caster, nullptr, SPELL_DEMESNE_DOOR, spell_t::SPELL_LEVEL_EVENT_EFFECT, 1);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if ( interrupted )
		{
			if ( Entity* caster = uidToEntity(my->parent) )
			{
				messagePlayer(caster->isEntityPlayer(), MESSAGE_WORLD, Language::get(6690));
			}

			createParticleDemesneDoor(my->x, my->y, my->yaw);
			serverSpawnMiscParticlesAtLocation(my->x, my->y, my->yaw * 256.0, PARTICLE_EFFECT_DEMESNE_DOOR, 0);
			my->removeLightField();
			list_RemoveNode(my->mynode);
			return;
		}
	}*/
}