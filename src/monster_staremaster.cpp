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

void initStareMaster(Entity* my, Stat* myStats)
{
	node_t* node;

	my->z = limbs[STAREMASTER][0][2];
	my->flags[BURNABLE] = false;
	my->initMonster(2477);
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

	// body
	Entity* entity = newEntity(2478, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 2;
	entity->sizey = 2;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->yaw = my->yaw;
	entity->z = 0;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[STAREMASTER][1][0];
	entity->focaly = limbs[STAREMASTER][1][1];
	entity->focalz = limbs[STAREMASTER][1][2];
	entity->behavior = &actStareMasterLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// eye
	entity = newEntity(2479, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 2;
	entity->sizey = 2;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->yaw = my->yaw;
	entity->z = 0;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[STAREMASTER][3][0];
	entity->focaly = limbs[STAREMASTER][3][1];
	entity->focalz = limbs[STAREMASTER][3][2];
	entity->behavior = &actStareMasterLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// tail
	entity = newEntity(2480, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 2;
	entity->sizey = 2;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->yaw = my->yaw;
	entity->z = 6;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[STAREMASTER][5][0];
	entity->focaly = limbs[STAREMASTER][5][1];
	entity->focalz = limbs[STAREMASTER][5][2];
	entity->behavior = &actStareMasterLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// arm left
	entity = newEntity(2481, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 2;
	entity->sizey = 2;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->yaw = my->yaw;
	entity->z = 6;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[STAREMASTER][7][0];
	entity->focaly = limbs[STAREMASTER][7][1];
	entity->focalz = limbs[STAREMASTER][7][2];
	entity->behavior = &actStareMasterLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// arm right
	entity = newEntity(2482, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 2;
	entity->sizey = 2;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->yaw = my->yaw;
	entity->z = 6;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[STAREMASTER][9][0];
	entity->focaly = limbs[STAREMASTER][9][1];
	entity->focalz = limbs[STAREMASTER][9][2];
	entity->behavior = &actStareMasterLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);
}

void actStareMasterLimb(Entity* my)
{
	my->actMonsterLimb(false);
}

void stareMasterDie(Entity* my)
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

#define STARE_BODY 2
#define STARE_EYE 3
#define STARE_TAIL 4
#define STARE_LEFTARM 5
#define STARE_RIGHTARM 6
#define STARE_EYE_SPRITE body->skill[0]
#define STARE_FLOATX body->fskill[0]
#define STARE_FLOATY body->fskill[1]
#define STARE_FLOATZ body->fskill[2]
#define STARE_FLOATANIM body->fskill[3]
#define STARE_ATTACK_ANIM body->fskill[4]
#define STARE_ATTACK_LUNGE body->fskill[5]
#define STARE_ATTACK_SPIN body->fskill[6]
#define STARE_EYE_CHANGE body->fskill[7]
#define STARE_ATTACK_STARE body->fskill[8]
#define STARE_RECOIL body->fskill[9]
#define STARE_MAGIC_ANIM body->fskill[10]
#define STARE_ARM_TWISTANIM entity->fskill[0]
#define STARE_EYE_BLINK entity->fskill[0]

#define EYE_SPRITE_NORMAL 2479
#define EYE_SPRITE_BEAM 2483
#define EYE_SPRITE_ALT 2484

void actStareParticle(Entity* my)
{
	my->removeLightField();
	++my->skill[0];

	if ( my->ticks >= 20 )
	{
		if ( Entity* fx = spawnMagicParticleCustom(my, 1866, 0.25, 1.0) )
		{
			fx->yaw = my->yaw;
			fx->roll = 0;
			fx->pitch = PI / 2;
			fx->x = my->x;
			fx->y = my->y;
			fx->z = my->z;
			fx->vel_x = my->vel_x / 8;
			fx->vel_y = my->vel_y / 8;
			fx->ditheringDisabled = true;
		}
	}

	real_t dist = clipMove(&my->x, &my->y, my->vel_x, my->vel_y, my);
	if ( my->skill[0] >= 25 /*|| dist != sqrt(my->vel_x * my->vel_x + my->vel_y * my->vel_y)*/ )
	{
		list_RemoveNode(my->mynode);
		return;
	}

	if ( my->ticks < 25 )
	{
		my->scalex = 40.0 * sin(PI * my->ticks / 25.0);
	}
	my->roll += 0.25;
	my->focalx = limbs[STAREMASTER][16][0];
	my->focaly = limbs[STAREMASTER][16][1];
	my->focalz = limbs[STAREMASTER][16][2];
	//my->scalez = std::min(40.0, my->scaley + 2.0);
	
	if ( !my->actmagicNoParticle )
	{
		if ( my->ticks < 20 )
		{
			if ( Entity* fx = spawnMagicParticleCustom(my, 1866, 0.25, 1.0) )
			{
				fx->yaw = my->yaw;
				fx->roll = 0;
				fx->pitch = PI / 2;
				fx->vel_x = my->vel_x / 4;
				fx->vel_y = my->vel_y / 4;
				fx->ditheringDisabled = true;
			}
		}
		if ( !my->actmagicNoLight )
		{
			my->light = addLight(my->x / 16, my->y / 16, "mistform_glow");

			if ( multiplayer != CLIENT )
			{
				if ( Entity* timer = uidToEntity(my->parent) )
				{
					Entity* caster = uidToEntity(timer->parent);
					if ( !caster )
					{
						caster = timer;
					}
					int numTargets = 0;
					std::vector<list_t*> entLists = TileEntityList.getEntitiesWithinRadiusAroundEntity(my, 3);
					for ( auto it : entLists )
					{
						node_t* node;
						for ( node = it->first; node != nullptr; node = node->next )
						{
							Entity* entity = (Entity*)node->element;
							Stat* stats = (entity->behavior == &actMonster || entity->behavior == &actPlayer) ? entity->getStats() : nullptr;
							//if ( stats )
							{
								if ( entityDist(my, entity) > 8.0 )
								{
									continue;
								}

								auto props = getParticleEmitterHitProps(timer->getUID(), entity);
								if ( !props )
								{
									continue;
								}
								if ( props->hits > 0 && (ticks - props->tick) < 10 )
								{
									continue;
								}

								if ( stats )
								{
									if ( caster && caster->getStats() )
									{
										if ( caster == entity ) { continue; }

										if ( caster && caster->behavior == &actMonster )
										{
											if ( caster->checkFriend(entity) )
											{
												continue;
											}
										}
										else
										{
											//if ( !(svFlags & SV_FLAG_FRIENDLYFIRE) )
											{
												if ( caster->checkFriend(entity) && caster->friendlyFireProtection(entity) )
												{
													continue;
												}
											}
										}
									}
									if ( entity->monsterIsTargetable(true) )
									{
										++props->hits;
										props->tick = ticks;
										//if ( caster->checkEnemy(entity) )
										{
											int duration = getSpellEffectDurationFromID(SPELL_STARE_BEAM, caster, nullptr, timer, timer->actmagicSpellbookBonus / 100.0);
											int slowDuration = std::max(duration, stats->EFFECTS_TIMERS[EFF_SLOW]);
											if ( entity->setEffect(EFF_SLOW, true, slowDuration, false) )
											{

											}

											bool prevEffect = stats->getEffectActive(EFF_WEAKNESS);
											int strength = std::min(7, stats->getEffectActive(EFF_WEAKNESS) + 1);
											int weakDuration = duration + stats->EFFECTS_TIMERS[EFF_WEAKNESS];
											if ( entity->setEffect(EFF_WEAKNESS, (Uint8)strength, weakDuration, false, true, true) )
											{
												if ( !prevEffect )
												{
													messagePlayerColor(entity->isEntityPlayer(), MESSAGE_STATUS, makeColorRGB(255, 0, 0), Language::get(7040));
												}
											}
											int damage = getSpellDamageFromID(SPELL_STARE_BEAM, caster, nullptr, timer, timer->actmagicSpellbookBonus / 100.0);
											applyGenericMagicDamage(caster, entity, *timer, SPELL_STARE_BEAM, damage, true);
										}
									}
								}
								else if ( (entity->isDamageableCollider() && entity->isColliderDamageableByMelee())
									|| entity->behavior == &actDoor
									|| entity->behavior == &actFurniture
									|| entity->behavior == &::actChest
									|| entity->behavior == &::actIronDoor )
								{
									int damage = getSpellDamageFromID(SPELL_STARE_BEAM, caster, nullptr, timer, timer->actmagicSpellbookBonus / 100.0);
									applyGenericMagicDamage(caster, entity, *timer, SPELL_STARE_BEAM, damage, true);
									++props->hits;
									props->tick = ticks;
								}
							}
						}
					}
				}

				/*if ( numTargets > 0 )
				{
					while ( numTargets > 0 )
					{
						--numTargets;
						magicOnSpellCastEvent(caster, caster, nullptr, SPELL_IGNITE, spell_t::SPELL_LEVEL_EVENT_DEFAULT, 1);
					}
				}*/
			}
		}
	}
}

void createStareAOE(Entity* my, bool updateClients)
{
	if ( !my ) { return; }
	for ( int j = 0; j < 3; ++j )
	{
		for ( int i = 0; i < 2; ++i )
		{
			if ( Entity* fx = createParticleAOEIndicator(my, my->x, my->y, my->z, TICKS_PER_SECOND + 20, 16) )
			{
				if ( j == 0 && i == 0 )
				{
					fx->light = addLight(fx->x / 16, fx->y / 16, "magic_foci_yellow_flicker");
				}
				fx->x = my->x + 8.0 * cos(my->yaw);
				fx->y = my->y + 8.0 * sin(my->yaw);
				fx->z = my->z;
				fx->yaw = my->yaw;
				real_t angle = 0.0;
				fx->x += 1.0 * cos(-PI / 2 + angle + j * 2 * PI / 3) * cos(my->yaw + PI / 2);
				fx->y += 1.0 * cos(-PI / 2 + angle + j * 2 * PI / 3) * sin(my->yaw + PI / 2);
				fx->z += 1.0 * sin(-PI / 2 + angle + j * 2 * PI / 3);

				fx->yaw = my->yaw;
				//fx->roll = -PI / 2;
				fx->yaw += i * PI;
				fx->pitch = PI / 2;
				fx->actSpriteFollowUID = my ? my->getUID() : 0;
				fx->actSpriteFollowYaw = 1 + i;
				fx->actSpriteFollowForwardDist = 8.0;
				fx->actSpriteFollowSideDist = 1.0 * cos(-PI / 2 + angle + j * 2 * PI / 3);
				//fx->actSpriteFollowUID = 0;
				fx->actSpriteCheckParentExists = 0;
				//fx->roll = my->yaw;
				fx->scalex = 0.5;
				fx->scaley = 0.5;
				fx->vel_x = 0.05 * cos(my->yaw);
				fx->vel_y = 0.05 * sin(my->yaw);
				//fx->vel_z = -.05;
				fx->actSpriteVelXY = 1;
				//fx->actSpritePitchRotate = 0.25;
				if ( auto indicator = AOEIndicators_t::getIndicator(fx->skill[10]) )
				{
					//indicator->arc = PI / 2;
					indicator->indicatorColor = makeColorRGB(255, 255, 0);
					indicator->cacheType = AOEIndicators_t::CACHE_STAREMASTER_STARE;
					indicator->loop = false;
					indicator->gradient = 4;
					indicator->framesPerTick = 2;
					indicator->ticksPerUpdate = 1;
					indicator->delayTicks = 0;
					indicator->expireAlphaRate = 0.95;
				}
			}
		}
	}

	if ( updateClients && multiplayer == SERVER )
	{
		serverSpawnMiscParticles(my, PARTICLE_EFFECT_STARE_MESMERIZE, 0);
	}
}

Entity* createStareParticle(Entity* caster)
{
	if ( !caster ) { return nullptr; }
	Entity* particle = nullptr;
	for ( int i = 0; i < 3; ++i )
	{
		Entity* entity = newEntity(1865, 1, map.entities, nullptr); //Particle entity.

		int size = 1;
		entity->x = caster->x + 4.0 * cos(caster->yaw) + (local_rng.rand() % size - size / 2) / 20.f;
		entity->y = caster->y + 4.0 * sin(caster->yaw) + (local_rng.rand() % size - size / 2) / 20.f;
		entity->z = -4;// +(local_rng.rand() % size - size / 2) / 20.f;

		real_t scale = 1.f;
		entity->scalex = scale;
		entity->scaley = scale;
		entity->scalez = scale;
		entity->sizex = 1;
		entity->sizey = 1;
		entity->yaw = caster->yaw;
		entity->pitch = 0;// PI / 2;
		entity->roll = 0;

		/*if ( i == 0 )
		{
			entity->z -= 1.0;
		}
		else if ( i == 1 )
		{
			entity->z += 1.0;
			entity->x += 1.0 * cos(entity->yaw + PI / 2);
			entity->y += 1.0 * sin(entity->yaw + PI / 2);
		}
		else if ( i == 2 )
		{
			entity->z += 1.0;
			entity->x += 1.0 * cos(entity->yaw - PI / 2);
			entity->y += 1.0 * sin(entity->yaw - PI / 2);
		}*/
		real_t angle = (ticks % 20) * PI / 10.0;
		entity->x += 1.0 * cos(-PI / 2 + angle + i * 2 * PI / 3) * cos(entity->yaw + PI / 2);
		entity->y += 1.0 * cos(-PI / 2 + angle + i * 2 * PI / 3) * sin(entity->yaw + PI / 2);
		entity->z += 1.0 * sin(-PI / 2 + angle + i * 2 * PI / 3);

		entity->flags[NOUPDATE] = true;
		entity->flags[PASSABLE] = true;
		entity->flags[UNCLICKABLE] = true;
		entity->flags[NOUPDATE] = true;
		entity->flags[UPDATENEEDED] = false;
		entity->flags[NOCLIP_CREATURES] = true;
		entity->lightBonus = vec4(0.25f, 0.25f, 0.25f, 0.f);
		entity->behavior = &actStareParticle;
		entity->ditheringDisabled = true;

		if ( i != 0 )
		{
			entity->actmagicNoParticle = 1;
		}

		if ( i == 0 )
		{
			playSoundEntityLocal(entity, 876, 92);
		}

		entity->vel_x = 4.0 * cos(caster->yaw);
		entity->vel_y = 4.0 * sin(caster->yaw);

		if ( multiplayer != CLIENT )
		{
			entity_uids--;
		}
		entity->setUID(-3);

		if ( !particle )
		{
			particle = entity;
		}
	}
	return particle;
}

void castStareBeam(Entity* my)
{
	if ( !my ) { return; }

	Entity* spellTimer = createParticleTimer(my, TICKS_PER_SECOND + 50, -1);
	spellTimer->particleTimerCountdownAction = PARTICLE_TIMER_ACTION_STARE_GAZE;
	spellTimer->particleTimerCountdownSprite = -1;
	spellTimer->particleTimerVariable1 = local_rng.rand() % 3;
	if ( spellTimer->particleTimerVariable1 == 0 )
	{
		spellTimer->yaw = my->yaw - (PI / 64) * 4.0;
	}
	else if ( spellTimer->particleTimerVariable1 == 1 )
	{
		spellTimer->yaw = my->yaw;
	}
	else if ( spellTimer->particleTimerVariable1 == 2 )
	{
		spellTimer->yaw = my->yaw + (PI / 64) * 4.0;
	}
	spellTimer->x = my->x;
	spellTimer->y = my->y;
	
	if ( multiplayer == SERVER )
	{
		serverSpawnMiscParticlesAtLocation(my->x, my->y, 0, PARTICLE_EFFECT_STARE_GAZE, spellTimer->particleTimerVariable1, 0, my->yaw * 256.0, my->getUID());
	}
}

bool entityWithinStareAngle(Entity* my, Entity* target)
{
	if ( !my || !target ) { return false; }

	real_t tangent = atan2(my->y - target->y, my->x - target->x);
	while ( tangent >= 2 * PI )
	{
		tangent -= 2 * PI;
	}
	while ( tangent < 0 )
	{
		tangent += 2 * PI;
	}
	real_t tangent2 = atan2(target->y - my->y, target->x - my->x);
	while ( tangent2 >= 2 * PI )
	{
		tangent2 -= 2 * PI;
	}
	while ( tangent2 < 0 )
	{
		tangent2 += 2 * PI;
	}

	real_t playerYaw = target->yaw;
	while ( playerYaw >= 2 * PI )
	{
		playerYaw -= 2 * PI;
	}
	while ( playerYaw < 0 )
	{
		playerYaw += 2 * PI;
	}

	real_t myYaw = my->yaw;
	while ( myYaw >= 2 * PI )
	{
		myYaw -= 2 * PI;
	}
	while ( myYaw < 0 )
	{
		myYaw += 2 * PI;
	}

	real_t interactAngle = PI / 3;
	bool range1 = false;
	bool range2 = false;
	if ( (abs(tangent - playerYaw) < (interactAngle)) || (abs(tangent - playerYaw) > (2 * PI - interactAngle)) )
	{
		range1 = true;
	}

	real_t interactAngle2 = PI / 2;
	if ( (abs(tangent2 - myYaw) < (interactAngle2)) || (abs(tangent2 - myYaw) > (2 * PI - interactAngle2)) )
	{
		range2 = true;
	}

	return range1 && range2;
}

void Entity::stareMasterChooseWeapon(const Entity* target, double dist)
{
	Stat* myStats = getStats();
	if ( !myStats )
	{
		return;
	}

	int formDelay = 0;
	if ( myStats->getAttribute("staremaster_form_delay") != "" )
	{
		formDelay = std::stoi(myStats->getAttribute("staremaster_form_delay"));
		formDelay = std::max(formDelay - 1, 0);
		myStats->setAttribute("staremaster_form_delay", std::to_string(formDelay));
	}

	if ( formDelay <= 0 && monsterSpecialTimer == 0 && (ticks % 5 == 0) /*&& monsterAttack == 0*/ )
	{
		std::vector<int> options;
		int bonusFromHealth = 0;
		if ( myStats->HP < (myStats->MAXHP * 0.4) )
		{
			bonusFromHealth = 2;
		}
		else if ( myStats->HP < (myStats->MAXHP * 0.8) )
		{
			bonusFromHealth = 1;
		}
		if ( (dist > TOUCHRANGE && local_rng.rand() % 3 == 0) )
		{
			if ( monsterSpecialState != STAREMASTER_MODE_NORMAL )
			{
				options.push_back(STAREMASTER_MODE_NORMAL);
			}
			
			if ( monsterSpecialState != STAREMASTER_MODE_BEAM )
			{
				options.push_back(STAREMASTER_MODE_BEAM);
				int bonus = bonusFromHealth;
				while ( bonus )
				{
					--bonus;
					options.push_back(STAREMASTER_MODE_BEAM);
				}
			}
		}
		else
		{
			if ( monsterSpecialState != STAREMASTER_MODE_NORMAL && local_rng.rand() % 5 == 0 )
			{
				options.push_back(STAREMASTER_MODE_NORMAL);
			}

			if ( monsterSpecialState != STAREMASTER_MODE_BEAM )
			{
				options.push_back(STAREMASTER_MODE_BEAM);
				int bonus = bonusFromHealth;
				while ( bonus )
				{
					--bonus;
					options.push_back(STAREMASTER_MODE_BEAM);
				}
			}

			if ( monsterSpecialState != STAREMASTER_MODE_ALTERNATE )
			{
				options.push_back(STAREMASTER_MODE_ALTERNATE);
				int bonus = bonusFromHealth;
				while ( bonus )
				{
					--bonus;
					options.push_back(STAREMASTER_MODE_ALTERNATE);
				}
			}
		}

		if ( options.size() )
		{
			monsterSpecialState = options[local_rng.rand() % options.size()];
			myStats->setAttribute("staremaster_form_delay", std::to_string(100));
			myStats->setAttribute("staremaster_atk_cycle", "");
		}
	}

	return;
}

std::vector<int> attacks = {
		MONSTER_POSE_MELEE_WINDUP1,
		MONSTER_POSE_MELEE_WINDUP2,
		MONSTER_POSE_MAGIC_WINDUP1,
		MONSTER_POSE_MAGIC_WINDUP2,
		MONSTER_POSE_MAGIC_WINDUP3,
		MONSTER_POSE_SPECIAL_WINDUP1,
		MONSTER_POSE_RANGED_WINDUP1,
		MONSTER_POSE_RANGED_WINDUP2,
		MONSTER_POSE_RANGED_WINDUP3
};

int stareMasterGetAttackPose(Entity* my)
{
	if ( !my ) { return 0; }
	Stat* myStats = my->getStats();
	if ( !myStats ) { return 0; }

	Entity* target = uidToEntity(my->monsterTarget);
	real_t dist = target ? entityDist(my, target) : 0.0;

	std::vector<int> options;

	int atkCycle = 0;
	if ( myStats->getAttribute("staremaster_atk_cycle") != "" )
	{
		atkCycle = std::stoi(myStats->getAttribute("staremaster_atk_cycle"));
	}

	if ( my->monsterSpecialState == 0 || my->monsterSpecialState == STAREMASTER_MODE_NORMAL )
	{
		if ( my->monsterSpecialTimer == MONSTER_SPECIAL_COOLDOWN_STAREMASTER_FORMCHANGE_NORMAL )
		{
			return MONSTER_POSE_RANGED_WINDUP1;
		}
		else
		{
			options.push_back(MONSTER_POSE_MAGIC_WINDUP1);
			if ( dist <= TOUCHRANGE )
			{
				options.push_back(MONSTER_POSE_MELEE_WINDUP1);
				options.push_back(MONSTER_POSE_MELEE_WINDUP1);
				options.push_back(MONSTER_POSE_MELEE_WINDUP2);
			}
		}
	}
	else if ( my->monsterSpecialState == STAREMASTER_MODE_BEAM )
	{
		if ( my->monsterSpecialTimer == MONSTER_SPECIAL_COOLDOWN_STAREMASTER_FORMCHANGE_BEAM )
		{
			return MONSTER_POSE_RANGED_WINDUP2;
		}
		else
		{
			options.push_back(MONSTER_POSE_MAGIC_WINDUP1);
			options.push_back(MONSTER_POSE_MAGIC_WINDUP1);
			options.push_back(MONSTER_POSE_MAGIC_WINDUP3);
			if ( dist <= 92 )
			{
				options.push_back(MONSTER_POSE_MAGIC_WINDUP2);
				options.push_back(MONSTER_POSE_MAGIC_WINDUP2);
			}
			if ( dist <= TOUCHRANGE )
			{
				options.push_back(MONSTER_POSE_MELEE_WINDUP2);
			}
		}
	}
	else if ( my->monsterSpecialState == STAREMASTER_MODE_ALTERNATE )
	{
		if ( my->monsterSpecialTimer == MONSTER_SPECIAL_COOLDOWN_STAREMASTER_FORMCHANGE_ALT )
		{
			return MONSTER_POSE_RANGED_WINDUP3;
		}
		else
		{
			if ( atkCycle == 0 || atkCycle == 5 )
			{
				options.push_back(MONSTER_POSE_SPECIAL_WINDUP1);
			}
			else
			{
				if ( target && target->getStats() && !target->getStats()->getEffectActive(EFF_MESMERIZED)
					&& atkCycle != 1 && atkCycle != 2 )
				{
					options.push_back(MONSTER_POSE_SPECIAL_WINDUP1);
					options.push_back(MONSTER_POSE_SPECIAL_WINDUP1);
				}
				//options.push_back(MONSTER_POSE_SPECIAL_WINDUP1);
				options.push_back(MONSTER_POSE_MAGIC_WINDUP1);
				options.push_back(MONSTER_POSE_MAGIC_WINDUP1);
				options.push_back(MONSTER_POSE_MAGIC_WINDUP3);
				if ( dist <= TOUCHRANGE )
				{
					options.push_back(MONSTER_POSE_MELEE_WINDUP2);
				}
			}
		}
	}

	++atkCycle;
	myStats->setAttribute("staremaster_atk_cycle", std::to_string(atkCycle));

	if ( options.size() )
	{
		return options[local_rng.rand() % options.size()];
	}

	return MONSTER_POSE_MELEE_WINDUP1;
}

void stareMasterAnimate(Entity* my, Stat* myStats, double dist)
{
	node_t* node;
	Entity* entity = nullptr;
	int bodypart;

	my->flags[INVISIBLE] = true; // hide the "AI" bodypart

	my->sizex = 4;
	my->sizey = 4;

	//myStats->setEffectActive(EFF_STUNNED, true);
	/*if ( keystatus[SDLK_g] )
	{
		keystatus[SDLK_g] = 0;
		if ( myStats->getEffectActive(EFF_PARALYZED) )
		{
			myStats->clearEffect(EFF_PARALYZED);
		}
		else
		{
			myStats->setEffectActive(EFF_PARALYZED, true);
		}
	}*/

	if ( multiplayer != CLIENT )
	{
		my->z = limbs[STAREMASTER][0][2];
		my->creatureHandleLiftZ();

		myStats->setEffectActive(EFF_LEVITATING, 1);
		myStats->EFFECTS_TIMERS[EFF_LEVITATING] = 0;
	}

	static ConsoleVariable<int> cvar_stare_atk("/stare_atk", 0);
	if ( *cvar_stare_atk )
	{
		if ( *cvar_stare_atk - 1 < attacks.size() )
		{
			my->monsterAttack = attacks[*cvar_stare_atk - 1];
			my->monsterAttackTime = 0;
		}
		*cvar_stare_atk = 0;
	}

	//my->yaw += 0.05;

	Entity* body = nullptr;
	for ( bodypart = 0, node = my->children.first; node != nullptr; node = node->next, ++bodypart )
	{
		if ( bodypart < STARE_BODY )
		{
			continue;
		}

		entity = (Entity*)node->element;
		if ( bodypart == STARE_BODY )
		{
			body = entity;
		}

		entity->x = my->x;
		entity->y = my->y;
		entity->z = my->z;
		entity->yaw = my->yaw;

		static ConsoleVariable<int> cvar_stare("/stare", 0);
		if ( *cvar_stare == bodypart )
		{
			entity->pitch += 0.05;
		}
		else
		{
			entity->pitch = 0.0;
		}

		if ( bodypart == STARE_BODY )
		{
			if ( my->monsterAttack > 3 
				&& my->monsterAttack != MONSTER_POSE_SPECIAL_WINDUP2 
				&& my->monsterAttack != MONSTER_POSE_RANGED_WINDUP1
				&& my->monsterAttack != MONSTER_POSE_RANGED_WINDUP2
				&& my->monsterAttack != MONSTER_POSE_RANGED_WINDUP3
				&& my->monsterAttack != MONSTER_POSE_SPECIAL_WINDUP1 )
			{
				STARE_ATTACK_ANIM += 0.05;
				STARE_ATTACK_ANIM = std::min(STARE_ATTACK_ANIM, 1.0);
			}
			else
			{
				STARE_ATTACK_ANIM -= 0.035;
				STARE_ATTACK_ANIM = std::max(STARE_ATTACK_ANIM, 0.0);
			}

			if ( my->monsterAttack == MONSTER_POSE_SPECIAL_WINDUP1 )
			{
				STARE_ATTACK_STARE += 0.06;
				STARE_ATTACK_STARE = std::min(STARE_ATTACK_STARE, 1.0);

				if ( my->monsterAttackTime == 0 )
				{
					playSoundEntityLocal(my, 170, 32);
				}

				if ( my->monsterAttackTime >= 30 && my->monsterAttackTime % 30 == 1 )
				{
					playSoundEntityLocal(my, 166, 128);
					createStareAOE(my);
					if ( multiplayer != CLIENT )
					{
						if ( my->monsterAttackTime == 31 )
						{
							Entity* spellTimer = createParticleTimer(my, 3 * TICKS_PER_SECOND + 10, -1);
							spellTimer->particleTimerCountdownAction = PARTICLE_TIMER_ACTION_STAREMASTER_MESMERIZE;
							spellTimer->particleTimerCountdownSprite = -1;
							spellTimer->x = my->x;
							spellTimer->y = my->y;
						}
					}
				}
				if ( my->monsterAttackTime >= 180 )
				{
					my->monsterAttack = 0;
				}
			}
			else
			{
				STARE_ATTACK_STARE -= 0.05;
				STARE_ATTACK_STARE = std::max(STARE_ATTACK_STARE, 0.0);
			}

			if ( my->monsterAttack == MONSTER_POSE_RANGED_WINDUP1
				|| my->monsterAttack == MONSTER_POSE_RANGED_WINDUP2
				|| my->monsterAttack == MONSTER_POSE_RANGED_WINDUP3 )
			{
				if ( my->monsterAttackTime == 0 )
				{
					STARE_EYE_CHANGE = 0.025;
					if ( my->monsterAttack == MONSTER_POSE_RANGED_WINDUP1 )
					{
						STARE_EYE_SPRITE = EYE_SPRITE_NORMAL;
					}
					else if ( my->monsterAttack == MONSTER_POSE_RANGED_WINDUP2 )
					{
						STARE_EYE_SPRITE = EYE_SPRITE_BEAM;
					}
					else if ( my->monsterAttack == MONSTER_POSE_RANGED_WINDUP3 )
					{
						STARE_EYE_SPRITE = EYE_SPRITE_ALT;
					}
				}
				else
				{
					STARE_EYE_CHANGE += 0.05;
					STARE_EYE_CHANGE = std::min(STARE_EYE_CHANGE, 1.0);

					if ( my->monsterAttackTime >= 20 )
					{
						STARE_ATTACK_LUNGE = std::min(1.0, 0.0);
						my->monsterAttack = 0;
						if ( multiplayer != CLIENT )
						{
							my->monsterHitTime = std::max(my->monsterHitTime, HITRATE);
						}
					}
				}
			}
			else if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP2 )
			{
				STARE_EYE_CHANGE += 0.05;
				STARE_EYE_CHANGE = std::min(STARE_EYE_CHANGE, 1.0);
			}
			else
			{
				STARE_EYE_CHANGE -= 0.05;
				STARE_EYE_CHANGE = std::max(STARE_EYE_CHANGE, 0.0);
			}

			if ( my->monsterAttack == MONSTER_POSE_MELEE_WINDUP1 || my->monsterAttack == MONSTER_POSE_MELEE_WINDUP2 )
			{
				if ( my->monsterAttackTime == 0 )
				{
					STARE_ATTACK_LUNGE = 0.0;
					STARE_ATTACK_SPIN = 0.0;
				}
				if ( STARE_ATTACK_ANIM >= 0.0 )
				{
					STARE_ATTACK_LUNGE += 0.06;
					STARE_ATTACK_LUNGE = std::min(STARE_ATTACK_LUNGE, 2.0);
				}

				if ( my->monsterAttack == MONSTER_POSE_MELEE_WINDUP1 )
				{
					if ( my->monsterAttackTime == 20 )
					{
						if ( multiplayer != CLIENT )
						{
							Sint32 tmp = my->monsterAttack;
							Sint32 tmp2 = my->monsterAttackTime;
							my->attack(1, 0, nullptr);
							my->monsterAttack = tmp;
							my->monsterAttackTime = tmp2;
						}
					}
					if ( my->monsterAttackTime >= 35 )
					{
						STARE_ATTACK_LUNGE = std::min(1.0, 0.0);
						my->monsterAttack = 0;
					}
				}

				if ( my->monsterAttack == MONSTER_POSE_MELEE_WINDUP2 )
				{
					if ( STARE_ATTACK_LUNGE >= 0.5 )
					{
						STARE_ATTACK_SPIN = std::max(STARE_ATTACK_SPIN, 0.01);
					}
				}
			}
			
			if ( !(my->monsterAttack == MONSTER_POSE_MELEE_WINDUP1 || my->monsterAttack == MONSTER_POSE_MELEE_WINDUP2) )
			{
				STARE_ATTACK_LUNGE -= 0.05;
				STARE_ATTACK_LUNGE = std::max(STARE_ATTACK_LUNGE, 0.0);
			}

			if ( STARE_ATTACK_SPIN >= 0.0005 )
			{
				STARE_ATTACK_SPIN += std::max(0.01, (1.0 - STARE_ATTACK_SPIN) / 20);
				STARE_ATTACK_SPIN = std::min(STARE_ATTACK_SPIN, 1.0);
			}

			if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP1
				|| my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP2
				|| my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP3 )
			{
				STARE_MAGIC_ANIM += 0.05;
				STARE_MAGIC_ANIM = std::min(STARE_MAGIC_ANIM, 1.0);
			}
			else
			{
				STARE_MAGIC_ANIM -= 0.05;
				STARE_MAGIC_ANIM = std::max(STARE_MAGIC_ANIM, 0.0);
			}

			if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP1
				|| my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP3 )
			{
				if ( my->monsterAttackTime == 0 )
				{
					playSoundEntityLocal(my, 170, 32);
				}

				if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP3 )
				{
					if ( my->monsterAttackTime == 25 )
					{
						if ( multiplayer != CLIENT )
						{
							if ( my->monsterSpecialState == STAREMASTER_MODE_ALTERNATE )
							{
								if ( local_rng.rand() % 3 == 0 )
								{
									castSpell(my->getUID(), getSpellFromID(SPELL_INCOHERENCE), true, false);
								}
								else
								{
									castSpell(my->getUID(), getSpellFromID(SPELL_CONFUSE), true, false);
								}
							}
							else
							{
								switch ( local_rng.rand() % 4 )
								{
								case 0:
									castSpell(my->getUID(), getSpellFromID(SPELL_CONFUSE), true, false);
									break;
								default:
									castSpell(my->getUID(), getSpellFromID(SPELL_PSYCHIC_SPEAR), true, false);
									break;
								}
							}
						}
					}
				}
				else if ( my->monsterSpecialState == STAREMASTER_MODE_BEAM )
				{
					if ( my->monsterAttackTime == 25 || my->monsterAttackTime == 35
						|| my->monsterAttackTime == 50 )
					{
						if ( multiplayer != CLIENT )
						{
							castSpell(my->getUID(), getSpellFromID(SPELL_MAGICMISSILE), true, false);
						}
					}
				}
				else if ( my->monsterAttackTime == 35 )
				{
					if ( multiplayer != CLIENT )
					{
						castSpell(my->getUID(), getSpellFromID(SPELL_MAGICMISSILE), true, false);
					}
				}

				if ( my->monsterAttackTime >= 50 )
				{
					//STARE_ATTACK_LUNGE = 0.0;
					my->monsterAttack = 0;
				}
			}
			if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP2 )
			{
				if ( my->monsterAttackTime == 0 )
				{
					createParticleDot(my);
					playSoundEntityLocal(my, 170, 32);

					if ( Entity* fx = createParticleAOEIndicator(my, my->x, my->y, 0.0, TICKS_PER_SECOND, 16) )
					{
						real_t scale = 1.0;
						fx->scalex = scale;
						fx->scaley = scale;
						fx->actSpriteCheckParentExists = 0;
						//fx->actSpriteFollowUID = my->getUID();
						if ( auto indicator = AOEIndicators_t::getIndicator(fx->skill[10]) )
						{
							//indicator->arc = PI / 2;
							indicator->indicatorColor = makeColorRGB(92, 255, 200);
							indicator->cacheType = AOEIndicators_t::CACHE_STAREMASTER_STARE;
							indicator->loop = false;
							indicator->gradient = 4;
							indicator->framesPerTick = 2;
							indicator->ticksPerUpdate = 1;
							indicator->delayTicks = 0;
							indicator->expireAlphaRate = 0.95;
						}
					}
				}

				if ( my->monsterAttackTime == 50 )
				{
					if ( multiplayer != CLIENT )
					{
						my->setEffect(EFF_STUNNED, true, 50, false);
						if ( Entity* target = uidToEntity(my->monsterTarget) )
						{
							real_t tangent = atan2(target->y - my->y, target->x - my->x);
							my->yaw = tangent;
							my->lookAtEntity(*target);
						}
						castStareBeam(my);
					}
					STARE_RECOIL = 1.0;
				}

				if ( my->monsterAttackTime >= 125 )
				{
					//STARE_ATTACK_LUNGE = 0.0;
					my->monsterAttack = 0;
					if ( multiplayer != CLIENT )
					{
						my->monsterHitTime = std::min(my->monsterHitTime, HITRATE);
					}
				}
			}

			STARE_RECOIL -= 0.025;
			STARE_RECOIL = std::max(STARE_RECOIL, 0.0);

			if ( my->monsterAttack == MONSTER_POSE_MELEE_WINDUP2 )
			{
				if ( my->monsterAttackTime == 15 )
				{
					if ( multiplayer != CLIENT )
					{
						Entity* spellTimer = createParticleTimer(my, 10, -1);
						spellTimer->particleTimerCountdownAction = PARTICLE_TIMER_ACTION_STAREMASTER_PUSH;
						spellTimer->particleTimerCountdownSprite = -1;
						spellTimer->x = my->x;
						spellTimer->y = my->y;
						spellTimer->yaw = my->yaw;

						Sint32 tmp = my->monsterAttack;
						Sint32 tmp2 = my->monsterAttackTime;

						my->attack(2, 0, nullptr);
						my->monsterAttack = tmp;
						my->monsterAttackTime = tmp2;
					}

					for ( int i = 0; i < 2; ++i )
					{
						if ( Entity* fx = createParticleAOEIndicator(my, my->x, my->y, my->z - 3, TICKS_PER_SECOND, 16) )
						{
							fx->yaw = my->yaw + PI / 2;
							fx->pitch = i * PI;
							real_t scale = 3.0;
							fx->scalex = scale;
							fx->scaley = scale;
							fx->actSpriteCheckParentExists = 0;
							fx->actSpriteFollowUID = my->getUID();
							if ( auto indicator = AOEIndicators_t::getIndicator(fx->skill[10]) )
							{
								//indicator->arc = PI / 2;
								indicator->indicatorColor = makeColorRGB(255, 255, 255);
								indicator->cacheType = AOEIndicators_t::CACHE_STAREMASTER_SWIPE;
								indicator->arc = PI / 3;
								indicator->loop = false;
								indicator->gradient = 4;
								indicator->framesPerTick = 2;
								indicator->ticksPerUpdate = 1;
								indicator->delayTicks = 0;
								indicator->expireAlphaRate = 0.95;
							}
						}
					}
				}
				if ( my->monsterAttackTime >= 40 )
				{
					STARE_ATTACK_LUNGE = std::min(1.0, 0.0);
					my->monsterAttack = 0;
				}
			}
		}

		switch ( bodypart )
		{
			case STARE_BODY:
			{
				entity->scalex = 1.05;
				entity->scaley = 1.05;
				entity->scalez = 1.05;
				entity->x += limbs[STAREMASTER][2][0] * cos(my->yaw) + limbs[STAREMASTER][2][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[STAREMASTER][2][0] * sin(my->yaw) + limbs[STAREMASTER][2][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[STAREMASTER][2][2];
				entity->focalx = limbs[STAREMASTER][1][0];
				entity->focaly = limbs[STAREMASTER][1][1];
				entity->focalz = limbs[STAREMASTER][1][2];

				STARE_FLOATANIM += 0.15 * limbs[STAREMASTER][12][1];
				STARE_FLOATZ = 0.0;
				STARE_FLOATZ += (1.0 - STARE_EYE_CHANGE) * limbs[STAREMASTER][12][0] * sin(STARE_FLOATANIM);
				STARE_FLOATZ += STARE_ATTACK_ANIM * 1.0 * sin(2 * STARE_FLOATANIM);
				STARE_FLOATZ -= STARE_EYE_CHANGE * 3.0 * sin(2 * STARE_FLOATANIM);

				STARE_FLOATZ -= 2.0 * sin(STARE_ATTACK_STARE * PI);
				STARE_FLOATZ += STARE_ATTACK_STARE * 0.5 * sin(2 * STARE_FLOATANIM);

				real_t attackFloat = sin((STARE_ATTACK_ANIM) * PI);
				if ( attackFloat >= 0.0 )
				{
					STARE_FLOATZ += 3.0 * attackFloat;
				}
				else
				{
					STARE_FLOATZ += 1.5 * attackFloat;
				}

				STARE_FLOATX = 0.0;
				STARE_FLOATY = 0.0;
				real_t lungeFloat = sin(STARE_ATTACK_LUNGE * PI);
				if ( lungeFloat >= 0.0 )
				{
					STARE_FLOATX += -3.0 * (lungeFloat) * cos(my->yaw);
					STARE_FLOATY += -3.0 * (lungeFloat) * sin(my->yaw);
				}
				else
				{
					STARE_FLOATX += -2.0 * (lungeFloat) * cos(my->yaw);
					STARE_FLOATY += -2.0 * (lungeFloat) * sin(my->yaw);
				}

				STARE_FLOATX += -3.0 * STARE_RECOIL * sin(3 * PI * STARE_RECOIL) * cos(my->yaw);
				STARE_FLOATY += -3.0 * STARE_RECOIL * sin(3 * PI * STARE_RECOIL) * sin(my->yaw);

				entity->pitch -= (2 * PI) * STARE_ATTACK_SPIN;

				entity->pitch -= (PI / 8) * STARE_ATTACK_ANIM;
				entity->pitch -= (PI / 8) * sin((PI / 4) * STARE_EYE_CHANGE);
				entity->pitch += (PI / 8) * sin((PI / 4) * STARE_ATTACK_STARE);

				entity->x += STARE_FLOATX;
				entity->y += STARE_FLOATY;
				entity->z += STARE_FLOATZ;
				break;
			}
			case STARE_EYE:
			{
				if ( STARE_EYE_SPRITE == 0 )
				{
					STARE_EYE_SPRITE = entity->sprite;
				}

				entity->x += limbs[STAREMASTER][4][0] * cos(my->yaw) + limbs[STAREMASTER][4][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[STAREMASTER][4][0] * sin(my->yaw) + limbs[STAREMASTER][4][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[STAREMASTER][4][2];
				entity->focalx = limbs[STAREMASTER][3][0];
				entity->focaly = limbs[STAREMASTER][3][1];
				entity->focalz = limbs[STAREMASTER][3][2];

				STARE_EYE_BLINK += limbs[STAREMASTER][15][0];
				STARE_EYE_BLINK = fmod(STARE_EYE_BLINK, 12.0);
				real_t lookAngle = PI * limbs[STAREMASTER][15][1] * (std::max(0.0, 1.0 - 2.0 * STARE_EYE_CHANGE)) * (1.0 - STARE_MAGIC_ANIM);

				entity->pitch = 0.0;
				if ( STARE_EYE_CHANGE >= 0.005 
					&& (my->monsterAttack == MONSTER_POSE_RANGED_WINDUP1
						|| my->monsterAttack == MONSTER_POSE_RANGED_WINDUP2
						|| my->monsterAttack == MONSTER_POSE_RANGED_WINDUP3) )
				{
					entity->pitch -= 2 * PI * STARE_EYE_CHANGE;
					if ( STARE_EYE_CHANGE >= 0.5 )
					{
						if ( entity->sprite != STARE_EYE_SPRITE )
						{
							entity->sprite = STARE_EYE_SPRITE;
						}
					}
				}

				if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP2 )
				{
					entity->yaw += (PI / 32) * sin(8 * STARE_FLOATANIM) * STARE_MAGIC_ANIM;
				}
				
				if ( STARE_EYE_BLINK >= 1.0 && STARE_EYE_BLINK <= 2.0 )
				{
					entity->pitch += (limbs[STAREMASTER][15][2] * PI)*sin((STARE_EYE_BLINK - 1.0) * PI) 
						* (std::max(0.0, 1.0 - 2.0 * STARE_EYE_CHANGE)) * (1.0 - STARE_MAGIC_ANIM);
				}
				else if ( STARE_EYE_BLINK >= 3.0 && STARE_EYE_BLINK <= 4.0 )
				{
					entity->pitch += (limbs[STAREMASTER][15][2] * PI)*sin((STARE_EYE_BLINK - 3.0) * PI) 
						* (std::max(0.0, 1.0 - 2.0 * STARE_EYE_CHANGE)) * (1.0 - STARE_MAGIC_ANIM);
				}
				else if ( STARE_EYE_BLINK >= 5.0 && STARE_EYE_BLINK <= 6.0 )
				{
					entity->yaw += -(lookAngle)*sin((STARE_EYE_BLINK - 5.0) * PI / 2);
				}
				else if ( STARE_EYE_BLINK >= 6.0 && STARE_EYE_BLINK <= 7.0 )
				{
					entity->yaw += -lookAngle;
				}
				else if ( STARE_EYE_BLINK >= 7.0 && STARE_EYE_BLINK <= 8.0 )
				{
					entity->yaw += -lookAngle;
					entity->yaw += (lookAngle * 2) * sin((STARE_EYE_BLINK - 7.0) * PI / 2);
				}
				else if ( STARE_EYE_BLINK >= 8.0 && STARE_EYE_BLINK <= 9.0 )
				{
					entity->yaw += lookAngle;
				}
				else if ( STARE_EYE_BLINK >= 9.0 && STARE_EYE_BLINK <= 10.0 )
				{
					entity->yaw += lookAngle;
					entity->yaw -= (lookAngle)*sin((STARE_EYE_BLINK - 9.0) * PI / 2);
				}

				entity->x += STARE_FLOATX;
				entity->y += STARE_FLOATY;
				entity->z += STARE_FLOATZ;
				break;
			}
			case STARE_TAIL:
				entity->x += limbs[STAREMASTER][6][0] * cos(my->yaw) + limbs[STAREMASTER][6][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[STAREMASTER][6][0] * sin(my->yaw) + limbs[STAREMASTER][6][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[STAREMASTER][6][2];
				entity->focalx = limbs[STAREMASTER][5][0];
				entity->focaly = limbs[STAREMASTER][5][1];
				entity->focalz = limbs[STAREMASTER][5][2];

				entity->x += limbs[STAREMASTER][11][1] * cos(body->yaw) * sin(body->pitch + PI * limbs[STAREMASTER][11][0]);
				entity->y += limbs[STAREMASTER][11][1] * sin(body->yaw) * sin(body->pitch + PI * limbs[STAREMASTER][11][0]);
				entity->z += limbs[STAREMASTER][11][1] * sin(body->pitch - PI / 2 + PI * limbs[STAREMASTER][11][0]);

				entity->pitch = body->pitch + PI * limbs[STAREMASTER][12][2] * sin(STARE_FLOATANIM);

				entity->x += STARE_FLOATX;
				entity->y += STARE_FLOATY;
				entity->z += STARE_FLOATZ;
				break;
			case STARE_LEFTARM:
				entity->x += limbs[STAREMASTER][8][0] * cos(my->yaw) + limbs[STAREMASTER][8][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[STAREMASTER][8][0] * sin(my->yaw) + limbs[STAREMASTER][8][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[STAREMASTER][8][2];
				entity->focalx = limbs[STAREMASTER][7][0];
				entity->focaly = limbs[STAREMASTER][7][1];
				entity->focalz = limbs[STAREMASTER][7][2];

				entity->x += limbs[STAREMASTER][13][1] * cos(body->yaw) * sin(body->pitch + PI * limbs[STAREMASTER][13][0]);
				entity->y += limbs[STAREMASTER][13][1] * sin(body->yaw) * sin(body->pitch + PI * limbs[STAREMASTER][13][0]);
				entity->z += limbs[STAREMASTER][13][1] * sin(body->pitch - PI / 2 + PI * limbs[STAREMASTER][13][0]);

				entity->pitch = body->pitch;
				entity->roll = (1.0 - STARE_ATTACK_ANIM) * 0.15 * sin(STARE_FLOATANIM);
				entity->yaw += (1.0 - STARE_ATTACK_STARE) * (1.0 - STARE_ATTACK_ANIM) * 1 * sin(STARE_FLOATANIM);
				entity->yaw += (PI / 8) * STARE_ATTACK_STARE * 1.0 * sin(4 * STARE_FLOATANIM);

				STARE_ARM_TWISTANIM += limbs[STAREMASTER][14][0];
				STARE_ARM_TWISTANIM = fmod(STARE_ARM_TWISTANIM, 4.0);
				if ( STARE_ARM_TWISTANIM >= 1.0 && STARE_ARM_TWISTANIM <= 3.0 )
				{
					entity->yaw += (1.0 - STARE_ATTACK_STARE) * (1.0 - STARE_ATTACK_ANIM) * PI * sin(PI * (STARE_ARM_TWISTANIM - 1.0));
				}

				entity->pitch -= (PI / 8) * STARE_ATTACK_ANIM * (std::max(0.0, 1.0 - STARE_ATTACK_LUNGE));
				entity->pitch -= (PI / 8) * STARE_ATTACK_ANIM * sin(4 * STARE_FLOATANIM) * (std::max(0.0, 1.0 - STARE_ATTACK_LUNGE));
				entity->yaw += (PI / 8) * STARE_ATTACK_ANIM * cos(4 * STARE_FLOATANIM) * (std::max(0.0, 1.0 - STARE_ATTACK_LUNGE));

				entity->pitch -= (PI / 2) * sin(std::max(1.0, STARE_ATTACK_LUNGE) * 2 * PI);
				entity->pitch -= (PI / 2) * STARE_ATTACK_STARE + (PI / 8) * (STARE_ATTACK_STARE * sin(STARE_FLOATANIM));

				entity->x += STARE_FLOATX;
				entity->y += STARE_FLOATY;
				entity->z += STARE_FLOATZ;

				if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP1 
					|| my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP2
					|| my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP3
					|| my->monsterAttack == MONSTER_POSE_SPECIAL_WINDUP1 )
				{
					if ( ticks % 10 == 0 )
					{
						Entity* gib = spawnGib(entity, 16);
						gib->flags[INVISIBLE] = false;
						gib->flags[SPRITE] = true;
						gib->flags[NOUPDATE] = true;
						gib->flags[UPDATENEEDED] = false;
						gib->lightBonus = vec4(0.2f, 0.2f, 0.2f, 0.f);
						gib->x = entity->x + 2.0 * cos(my->yaw);
						gib->y = entity->y + 2.0 * sin(my->yaw);
						gib->z = entity->z + 2.0;
						gib->scalex = 0.25f; //MAKE 'EM SMALL PLEASE!
						gib->scaley = 0.25f;
						gib->scalez = 0.25f;
						gib->sprite = 16; //TODO: Originally. 22. 16 -- spark sprite instead?
						gib->yaw = ((local_rng.rand() % 6) * 60) * PI / 180.0;
						gib->pitch = (local_rng.rand() % 360) * PI / 180.0;
						gib->roll = (local_rng.rand() % 360) * PI / 180.0;
						gib->vel_x = cos(entity->yaw) * .1;
						gib->vel_y = sin(entity->yaw) * .1;
						gib->vel_z = -.15;
						gib->fskill[3] = 0.01;
						gib->fskill[4] = 0.01; // GIB_SHRINK
						gib->skill[4] = 25; // GIB_LIFESPAN
					}
				}
				break;
			case STARE_RIGHTARM:
				entity->x += limbs[STAREMASTER][10][0] * cos(my->yaw) + limbs[STAREMASTER][10][1] * cos(my->yaw + PI / 2);
				entity->y += limbs[STAREMASTER][10][0] * sin(my->yaw) + limbs[STAREMASTER][10][1] * sin(my->yaw + PI / 2);
				entity->z += limbs[STAREMASTER][10][2];
				entity->focalx = limbs[STAREMASTER][9][0];
				entity->focaly = limbs[STAREMASTER][9][1];
				entity->focalz = limbs[STAREMASTER][9][2];

				entity->x += limbs[STAREMASTER][13][1] * cos(body->yaw) * sin(body->pitch + PI * limbs[STAREMASTER][13][0]);
				entity->y += limbs[STAREMASTER][13][1] * sin(body->yaw) * sin(body->pitch + PI * limbs[STAREMASTER][13][0]);
				entity->z += limbs[STAREMASTER][13][1] * sin(body->pitch - PI / 2 + PI * limbs[STAREMASTER][13][0]);

				entity->pitch = body->pitch;
				entity->roll = (1.0 - STARE_ATTACK_ANIM) * 0.25 * cos(STARE_FLOATANIM);
				entity->yaw += (1.0 - STARE_ATTACK_ANIM) * 0.75 * cos(STARE_FLOATANIM);

				STARE_ARM_TWISTANIM += limbs[STAREMASTER][14][1];
				STARE_ARM_TWISTANIM = fmod(STARE_ARM_TWISTANIM, 4.0);
				if ( STARE_ARM_TWISTANIM >= 1.0 && STARE_ARM_TWISTANIM <= 3.0 )
				{
					entity->yaw += (1.0 - STARE_ATTACK_STARE) * (1.0 - STARE_ATTACK_ANIM) * PI * sin(PI * (STARE_ARM_TWISTANIM - 1.0));
				}

				entity->pitch -= (PI / 8) * STARE_ATTACK_ANIM * (std::max(0.0, 1.0 - STARE_ATTACK_LUNGE));
				entity->pitch -= (PI / 8) * STARE_ATTACK_ANIM * sin(4 * STARE_FLOATANIM) * (std::max(0.0, 1.0 - STARE_ATTACK_LUNGE));
				entity->yaw -= (PI / 8) * STARE_ATTACK_ANIM * cos(4 * STARE_FLOATANIM) * (std::max(0.0, 1.0 - STARE_ATTACK_LUNGE));

				entity->pitch -= (PI / 2) * sin(std::max(1.0, STARE_ATTACK_LUNGE) * 2 * PI);
				entity->pitch -= (PI / 2) * STARE_ATTACK_STARE + (PI / 8) * (STARE_ATTACK_STARE * sin(STARE_FLOATANIM));

				entity->x += STARE_FLOATX;
				entity->y += STARE_FLOATY;
				entity->z += STARE_FLOATZ;

				if ( my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP1 
					|| my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP2
					|| my->monsterAttack == MONSTER_POSE_MAGIC_WINDUP3
					|| my->monsterAttack == MONSTER_POSE_SPECIAL_WINDUP1 )
				{
					if ( ticks % 10 == 0 )
					{
						Entity* gib = spawnGib(entity, 16);
						gib->flags[INVISIBLE] = false;
						gib->flags[SPRITE] = true;
						gib->flags[NOUPDATE] = true;
						gib->flags[UPDATENEEDED] = false;
						gib->lightBonus = vec4(0.2f, 0.2f, 0.2f, 0.f);
						gib->x = entity->x + 2.0 * cos(my->yaw);
						gib->y = entity->y + 2.0 * sin(my->yaw);
						gib->z = entity->z + 2.0;
						gib->scalex = 0.25f; //MAKE 'EM SMALL PLEASE!
						gib->scaley = 0.25f;
						gib->scalez = 0.25f;
						gib->sprite = 16; //TODO: Originally. 22. 16 -- spark sprite instead?
						gib->yaw = ((local_rng.rand() % 6) * 60) * PI / 180.0;
						gib->pitch = (local_rng.rand() % 360) * PI / 180.0;
						gib->roll = (local_rng.rand() % 360) * PI / 180.0;
						gib->vel_x = cos(entity->yaw) * .1;
						gib->vel_y = sin(entity->yaw) * .1;
						gib->vel_z = -.15;
						gib->fskill[3] = 0.01;
						gib->fskill[4] = 0.01; // GIB_SHRINK
						gib->skill[4] = 25; // GIB_LIFESPAN
					}
				}
				break;
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