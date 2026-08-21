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
#include "../colors.hpp"

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

bool applyShrineEffect(std::string effect_str, Entity* target, Entity* shrine, int tier);
bool applySupplicationEffect(std::string tier_str, Entity* target, Entity* shrine);
void spawnHeatOrbitSpin(Entity* target, int sprite, bool light);

std::vector<std::pair<std::string, ShrineEffects_t::ShrineEffectsPools>> ShrineEffects_t::shrineEffectsTable =
{
	{"BODY_1", ShrineEffects_t::ShrineEffectsPools::BODY_1},
	{"BODY_2", ShrineEffects_t::ShrineEffectsPools::BODY_2},
	{"BODY_3", ShrineEffects_t::ShrineEffectsPools::BODY_3},
	{"MIND_1", ShrineEffects_t::ShrineEffectsPools::MIND_1},
	{"MIND_2", ShrineEffects_t::ShrineEffectsPools::MIND_2},
	{"MIND_3", ShrineEffects_t::ShrineEffectsPools::MIND_3},
	{"BODY_CHALLENGE_1", ShrineEffects_t::ShrineEffectsPools::BODY_CHALLENGE_1},
	{"BODY_CHALLENGE_2", ShrineEffects_t::ShrineEffectsPools::BODY_CHALLENGE_2},
	{"BODY_CHALLENGE_3", ShrineEffects_t::ShrineEffectsPools::BODY_CHALLENGE_3},
	{"MIND_CHALLENGE_1", ShrineEffects_t::ShrineEffectsPools::MIND_CHALLENGE_1},
	{"MIND_CHALLENGE_2", ShrineEffects_t::ShrineEffectsPools::MIND_CHALLENGE_2},
	{"MIND_CHALLENGE_3", ShrineEffects_t::ShrineEffectsPools::MIND_CHALLENGE_3},
	{"REWARD_ITEM_1", ShrineEffects_t::ShrineEffectsPools::REWARD_ITEM_1},
	{"REWARD_ITEM_2", ShrineEffects_t::ShrineEffectsPools::REWARD_ITEM_2},
	{"REWARD_ITEM_3", ShrineEffects_t::ShrineEffectsPools::REWARD_ITEM_3},
	{"BODY_BUFF_1", ShrineEffects_t::ShrineEffectsPools::BODY_BUFF_1},
	{"BODY_BUFF_2", ShrineEffects_t::ShrineEffectsPools::BODY_BUFF_2},
	{"BODY_BUFF_3", ShrineEffects_t::ShrineEffectsPools::BODY_BUFF_3},
	{"MIND_BUFF_1", ShrineEffects_t::ShrineEffectsPools::MIND_BUFF_1},
	{"MIND_BUFF_2", ShrineEffects_t::ShrineEffectsPools::MIND_BUFF_2},
	{"MIND_BUFF_3", ShrineEffects_t::ShrineEffectsPools::MIND_BUFF_3},
	{"FOOD_BUFF_1", ShrineEffects_t::ShrineEffectsPools::FOOD_BUFF_1},
	{"FOOD_BUFF_2", ShrineEffects_t::ShrineEffectsPools::FOOD_BUFF_2},
	{"FOOD_BUFF_3", ShrineEffects_t::ShrineEffectsPools::FOOD_BUFF_3},
	{"STATUS_BUFF_HP_1", ShrineEffects_t::ShrineEffectsPools::STATUS_BUFF_HP_1},
	{"STATUS_BUFF_HP_2", ShrineEffects_t::ShrineEffectsPools::STATUS_BUFF_HP_2},
	{"STATUS_BUFF_HP_3", ShrineEffects_t::ShrineEffectsPools::STATUS_BUFF_HP_3},
	{"STATUS_BUFF_MP_1", ShrineEffects_t::ShrineEffectsPools::STATUS_BUFF_MP_1},
	{"STATUS_BUFF_MP_2", ShrineEffects_t::ShrineEffectsPools::STATUS_BUFF_MP_2},
	{"STATUS_BUFF_MP_3", ShrineEffects_t::ShrineEffectsPools::STATUS_BUFF_MP_3},
	{"MIRACLE_1", ShrineEffects_t::ShrineEffectsPools::MIRACLE_1},
	{"MIRACLE_2", ShrineEffects_t::ShrineEffectsPools::MIRACLE_2},
	{"MIRACLE_3", ShrineEffects_t::ShrineEffectsPools::MIRACLE_3},
	{"SONG_1", ShrineEffects_t::ShrineEffectsPools::SONG_1},
	{"SONG_2", ShrineEffects_t::ShrineEffectsPools::SONG_2},
	{"SONG_3", ShrineEffects_t::ShrineEffectsPools::SONG_3},
	{"SONG_4", ShrineEffects_t::ShrineEffectsPools::SONG_4},
	{"SONG_5", ShrineEffects_t::ShrineEffectsPools::SONG_5}
};

std::set<std::string> ShrineEffects_t::shrineEffects = 
{
	"WEAKNESS",
	"SHAPESHIFT",
	"INCOHERENCE",
	"ENFEEBLE",
	"DIVINE_FIRE",
	"SCAPEGOAT",
	"DIZZY",
	"EXORCISED",
	"DUSTED",
	"SILENCE",
	"ADORCISED",
	"PSYCHIC_SPEAR",
	"LIGHTNING_BOLT",
	"EARTH_SPRITE",
	"LEVEL_DRAIN",
	"VOID_MP_DRAIN",
	"DEGENERATION",
	"DISPIRITED",
	"BURDENED",
	"LIGHTEN_LOAD", "GREATER_MIGHT", "STURDINESS", 
	"STAMINA", "AGILITY",
	"NIMBLENESS", "COUNSEL", 
	"MENTALITY", "AGILITY",
	"SLOW_DIGESTION", "BLESSED_MEALS",
	"SMOKE_HP", "RESOLVE_HP",
	"SMOKE_MP", "RESOLVE_MP",
	"DONATION", "SACRED_PATH",
	"SCRY_TREASURES", "HIDDEN_KNOWLEDGE",
	"HEALING_POTION",
	"MANA_POTION",
	"ENSEMBLE_1",
	"ENSEMBLE_1_INSTRUMENT",
	"ENSEMBLE_1_SCROLL",
	"ENSEMBLE_2",
	"ENSEMBLE_2_INSTRUMENT",
	"ENSEMBLE_2_SCROLL",
	"ENSEMBLE_3",
	"ENSEMBLE_3_INSTRUMENT",
	"ENSEMBLE_3_SCROLL",
	"ENSEMBLE_4",
	"ENSEMBLE_4_INSTRUMENT",
	"ENSEMBLE_4_SCROLL",
	"ENSEMBLE_5",
	"ENSEMBLE_5_INSTRUMENT",
	"ENSEMBLE_5_SCROLL",
	"TIER_ITEM"
};

int ShrinePlayerMessageManager_t::processed_on_floor = -1;
std::map<Uint32, std::vector<ShrinePlayerMessageManager_t::ShrinePlayerMessages_t>> ShrinePlayerMessageManager_t::shrines;
void ShrinePlayerMessageManager_t::reset()
{
	shrines.clear();
}
void ShrinePlayerMessageManager_t::insert(Uint32 uid, const int player, const char* lang, std::string tierString, std::pair<std::string, int> resultString, int messageDelay)
{
	if ( multiplayer != CLIENT )
	{
		shrines[uid].push_back(ShrinePlayerMessages_t(SHRINE_MESSAGE_WARNING, player, lang, tierString, resultString));
		shrines[uid].back().tick += messageDelay;
	}
}
void ShrinePlayerMessageManager_t::insert(MessageType messageType, Uint32 uid, const int player, const char* lang, int messageDelay)
{
	if ( multiplayer != CLIENT )
	{
		shrines[uid].push_back(ShrinePlayerMessages_t(messageType, player, lang, "", std::pair<std::string, int>("", 0)));
		shrines[uid].back().tick += messageDelay;
	}
}

void ShrinePlayerMessageManager_t::update(Uint32 uid)
{
	if ( processed_on_floor != currentlevel )
	{
		shrines.clear();
	}
	processed_on_floor = currentlevel;
	if ( multiplayer != CLIENT )
	{
		if ( shrines.find(uid) != shrines.end() )
		{
			for ( auto it = shrines[uid].begin(); it != shrines[uid].end(); )
			{
				if ( ::ticks >= it->tick + 3 * TICKS_PER_SECOND )
				{
					if ( it->lang_str != "" )
					{
						if ( Entity* shrine = uidToEntity(uid) )
						{
							if ( it->messageType == SHRINE_MESSAGE_REJECT )
							{
								messagePlayerColor(it->player, MESSAGE_WORLD, makeColorRGB(255, 0, 255), it->lang_str.c_str());
								playSoundNotificationPlayer(it->player, 921, 92);
							}
							else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC )
							{
								if ( it->messageType == SHRINE_MESSAGE_GENERAL )
								{
									messagePlayerColor(it->player, MESSAGE_WORLD, makeColorRGB(255, 255, 0), it->lang_str.c_str());
								}
								else
								{
									messagePlayerColor(it->player, MESSAGE_WORLD, makeColorRGB(255, 0, 255), it->lang_str.c_str());
									playSoundPlayer(it->player, 920, 92);
								}
							}
							else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
							{
								if ( it->messageType == SHRINE_MESSAGE_GENERAL )
								{
									messagePlayerColor(it->player, MESSAGE_WORLD, makeColorRGB(255, 255, 255), it->lang_str.c_str());
								}
								else
								{
									messagePlayerColor(it->player, MESSAGE_WORLD, makeColorRGB(255, 0, 255), it->lang_str.c_str());
									playSoundNotificationPlayer(it->player, 919, 92);
								}
							}
							else
							{
								messagePlayerColor(it->player, MESSAGE_WORLD, makeColorRGB(255, 0, 255), it->lang_str.c_str());
								playSoundNotificationPlayer(it->player, 919, 92);
							}
						}
					}
					it = shrines[uid].erase(it);
				}
				else if ( ::ticks == it->tick )
				{
					if ( it->tier_string != "" )
					{
						if ( Entity* shrine = uidToEntity(uid) )
						{
							if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
							{
								bool supplication = applySupplicationEffect(it->tier_string, players[it->player]->entity, shrine);
								bool effect = false;
								if ( it->result_pair.first != "" )
								{
									effect = applyShrineEffect(it->result_pair.first, players[it->player]->entity, shrine, it->result_pair.second);
								}
								if ( supplication || effect )
								{
									spawnHeatOrbitSpin(players[it->player]->entity, 263, false);
									playSoundEntity(players[it->player]->entity, 166, 128);
									playSoundEntity(players[it->player]->entity, 827, 128);
								}
							}
						}
					}
					++it;
				}
				else
				{
					++it;
				}
			}
		}
	}
}

bool processShrineLockoutOnEffect(Entity* shrine, Entity* target, std::string tierString, bool onInteract)
{
	if ( !shrine ) { return false; }
	if ( !target ) { return false; }
	auto lockoutStatus = (GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus)
		(shrine->eternalShrinePlayerLockout >> (target->skill[2] * 2) & 0b11);

	static ConsoleVariable<bool> cvar_eternal_shrine_lockout("/eternal_shrine_lockout", false);
	if ( svFlags & SV_FLAG_CHEATS )
	{
		if ( *cvar_eternal_shrine_lockout )
		{
			if ( lockoutStatus > GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_NONE )
			{
				Uint32 newvalue = GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_NONE << (target->skill[2] * 2);
				Uint32 mask = (0b11) << (target->skill[2] * 2);
				shrine->eternalShrinePlayerLockout &= ~(mask); // zero out the player slot
				shrine->eternalShrinePlayerLockout |= newvalue; // apply new value
				return false;
			}
		}
	}

	if ( lockoutStatus > GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_NONE )
	{
		if ( lockoutStatus == GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_OFFERING
			&& !onInteract )
		{
			Uint32 newvalue = GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_COMPLETED << (target->skill[2] * 2);
			Uint32 mask = (0b11) << (target->skill[2] * 2);
			shrine->eternalShrinePlayerLockout &= ~(mask); // zero out the player slot
			shrine->eternalShrinePlayerLockout |= newvalue; // apply new value

			int tier = 1 + std::min(2, currentlevel / 10);
			if ( tierString != "" )
			{
				tier = std::max(tier, std::stoi(tierString.substr(0, 1)));
			}
			if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
			{
				playSoundEntity(shrine, 915, 156);
				bool effect = applyShrineEffect("DIVINE_FIRE", target, shrine, tier);
				if ( effect )
				{
					ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_REJECT,
						shrine->getUID(), target->skill[2], Language::get(7147), 0);
					return true;
				}
			}
			else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
			{
				playSoundEntity(shrine, 886, 156);
				bool effect = applyShrineEffect("LIGHTNING_BOLT", target, shrine, tier);
				if ( effect )
				{
					ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_REJECT,
						shrine->getUID(), target->skill[2], Language::get(7147), 0);
					return true;
				}
			}
			else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
			{
				playSoundEntity(shrine, 884, 156);
				bool effect = applyShrineEffect("PSYCHIC_SPEAR", target, shrine, tier);
				if ( effect )
				{
					ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_REJECT,
						shrine->getUID(), target->skill[2], Language::get(7147), 0);
					return true;
				}
			}
			else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC )
			{
				//playSoundEntity(shrine, 915, 156);
				bool effect = applyShrineEffect("SILENCE", target, shrine, tier);
				if ( effect )
				{
					ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_REJECT,
						shrine->getUID(), target->skill[2], Language::get(7147), 0);
					return true;
				}
			}
		}

		if ( !onInteract || (onInteract && lockoutStatus != GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_OFFERING) )
		{
			if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
			{
				messagePlayer(target->skill[2], MESSAGE_INTERACTION, Language::get(7148));
			}
			else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC )
			{
				messagePlayer(target->skill[2], MESSAGE_INTERACTION, Language::get(7149));
			}
			else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
			{
				messagePlayer(target->skill[2], MESSAGE_INTERACTION, Language::get(7150));
			}
			else if ( shrine->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
			{
				messagePlayer(target->skill[2], MESSAGE_INTERACTION, Language::get(7151));
			}

			return true;
		}
	}

	return false;
}

std::map<int, std::map<ShrineEffects_t::ShrineEffectsPools, std::vector<std::string>>> ShrineEffects_t::shrineEffectPools;
std::map<int, std::map<std::string, std::vector<std::pair<ShrineEffects_t::ShrineEffectsPools, int>>>> ShrineEffects_t::shrineOutcomes;
std::map<int, std::map<std::string, std::vector<std::pair<ShrineEffects_t::ShrineEffectsPools, int>>>> ShrineEffects_t::shrineRewards;
std::map<std::string, std::set<std::string>> ShrineEffects_t::supplicationExcludeStrings;
std::map<int, ShrineEffects_t::SkillItemPools_t> ShrineEffects_t::skillItemMap;
Uint32 ShrineEffects_t::shrineJsonHashRead = 0;
void ShrineEffects_t::buildShrineEffects()
{
	std::string filename = "data/shrine_effects.json";

	if ( !PHYSFS_getRealDir(filename.c_str()) )
	{
		//printlog("[JSON]: Error: Could not locate json file %s", filename.c_str());
		return;
	}

	std::string inputPath = PHYSFS_getRealDir(filename.c_str());
	inputPath.append(PHYSFS_getDirSeparator());
	inputPath.append(filename.c_str());

	File* fp = FileIO::open(inputPath.c_str(), "rb");
	if ( !fp )
	{
		printlog("[JSON]: Error: Could not locate json file %s", inputPath.c_str());
		return;
	}

	static char buf[32000];
	FileReadStreamCustomWrapper is(fp, buf, sizeof(buf)); // custom parser to read chunks at a time
	//rapidjson::StringStream is(buf);

	rapidjson::Document d;
	d.ParseStream(is);
	FileIO::close(fp);
	if ( !d.IsObject() )
	{
		return;
	}
	if ( !d.HasMember("version") || !d.HasMember("effects") || !d.HasMember("outcomes") || !d.HasMember("rewards") )
	{
		printlog("[JSON]: Error: No 'version' value in json file, or JSON syntax incorrect! %s", inputPath.c_str());
		return;
	}

	int version = d["version"].GetInt();

	shrineEffectPools[GUI_TYPE_ETERNALSHRINE_ANVIL].clear();
	shrineEffectPools[GUI_TYPE_ETERNALSHRINE_SUPPLICATION].clear();
	shrineEffectPools[GUI_TYPE_ETERNALSHRINE_MUSIC].clear();
	shrineEffectPools[GUI_TYPE_ETERNALSHRINE_ASCENSION].clear();

	Uint32 hash = 0;
	Uint32 shift = 0;

	Player::PlayerMechanics_t::divineFavorPipBreakpoints.clear();
	Player::PlayerMechanics_t::divineFavorPipBreakpoints.resize(11, 1000);
	Player::PlayerMechanics_t::divineFavorPipBreakpoints[0] = 0;
	if ( d.HasMember("divine_favor_pip_points") )
	{
		for ( int i = 0; i <= Player::DIVINE_FAVOR_PIPS_MAX; ++i )
		{
			std::string str = std::to_string(i);
			if ( d["divine_favor_pip_points"].HasMember(str.c_str()) )
			{
				if ( d["divine_favor_pip_points"][str.c_str()].IsInt() )
				{
					Player::PlayerMechanics_t::divineFavorPipBreakpoints[i] = d["divine_favor_pip_points"][str.c_str()].GetInt();
					hash += (Uint32)((Uint32)(i + 1) << (shift % 32)); ++shift;
					hash += (Uint32)((Uint32)(Player::PlayerMechanics_t::divineFavorPipBreakpoints[i] + 55555) << (shift % 32)); ++shift;
				}
			}
		}
	}

	auto& effects_all = d["effects"];
	int poolIndex = -1;
	if ( effects_all.HasMember("anvil") )
	{
		auto& effects = effects_all["anvil"];
		for ( auto& pair : shrineEffectsTable )
		{
			auto& str = pair.first;
			++poolIndex;
			if ( effects.HasMember(str.c_str()) )
			{
				if ( effects[str.c_str()].IsArray() )
				{
					for ( auto arr_itr = effects[str.c_str()].Begin(); arr_itr != effects[str.c_str()].End(); ++arr_itr )
					{
						if ( arr_itr->IsString() )
						{
							if ( shrineEffects.find(arr_itr->GetString()) != shrineEffects.end() )
							{
								hash += djb2Hash(const_cast<char*>(arr_itr->GetString()));
								shrineEffectPools[GUI_TYPE_ETERNALSHRINE_ANVIL][pair.second].push_back(arr_itr->GetString());
							}
							else
							{
								printlog("[JSON]: shrine_effects.json warning: no effect key found for '%s'!", arr_itr->GetString());
							}
						}
					}
				}
			}
			else
			{
				hash += (Uint32)((Uint32)poolIndex << (shift % 32)); ++shift;
			}
		}
	}
	if ( effects_all.HasMember("chorale") )
	{
		auto& effects = effects_all["chorale"];
		for ( auto& pair : shrineEffectsTable )
		{
			auto& str = pair.first;
			++poolIndex;
			if ( effects.HasMember(str.c_str()) )
			{
				if ( effects[str.c_str()].IsArray() )
				{
					for ( auto arr_itr = effects[str.c_str()].Begin(); arr_itr != effects[str.c_str()].End(); ++arr_itr )
					{
						if ( arr_itr->IsString() )
						{
							if ( shrineEffects.find(arr_itr->GetString()) != shrineEffects.end() )
							{
								hash += djb2Hash(const_cast<char*>(arr_itr->GetString()));
								shrineEffectPools[GUI_TYPE_ETERNALSHRINE_MUSIC][pair.second].push_back(arr_itr->GetString());
							}
							else
							{
								printlog("[JSON]: shrine_effects.json warning: no effect key found for '%s'!", arr_itr->GetString());
							}
						}
					}
				}
			}
			else
			{
				hash += (Uint32)((Uint32)poolIndex << (shift % 32)); ++shift;
			}
		}
	}

	if ( effects_all.HasMember("ascension") )
	{
		auto& effects = effects_all["ascension"];
		for ( auto& pair : shrineEffectsTable )
		{
			auto& str = pair.first;
			++poolIndex;
			if ( effects.HasMember(str.c_str()) )
			{
				if ( effects[str.c_str()].IsArray() )
				{
					for ( auto arr_itr = effects[str.c_str()].Begin(); arr_itr != effects[str.c_str()].End(); ++arr_itr )
					{
						if ( arr_itr->IsString() )
						{
							if ( shrineEffects.find(arr_itr->GetString()) != shrineEffects.end() )
							{
								hash += djb2Hash(const_cast<char*>(arr_itr->GetString()));
								shrineEffectPools[GUI_TYPE_ETERNALSHRINE_ASCENSION][pair.second].push_back(arr_itr->GetString());
							}
							else
							{
								printlog("[JSON]: shrine_effects.json warning: no effect key found for '%s'!", arr_itr->GetString());
							}
						}
					}
				}
			}
			else
			{
				hash += (Uint32)((Uint32)poolIndex << (shift % 32)); ++shift;
			}
		}
	}

	supplicationExcludeStrings.clear();
	if ( effects_all.HasMember("supplication") )
	{
		auto& effects = effects_all["supplication"];
		for ( auto& pair : shrineEffectsTable )
		{
			auto& str = pair.first;
			++poolIndex;
			if ( effects.HasMember(str.c_str()) )
			{
				if ( effects[str.c_str()].IsArray() )
				{
					for ( auto arr_itr = effects[str.c_str()].Begin(); arr_itr != effects[str.c_str()].End(); ++arr_itr )
					{
						if ( arr_itr->IsString() )
						{
							if ( shrineEffects.find(arr_itr->GetString()) != shrineEffects.end() )
							{
								hash += djb2Hash(const_cast<char*>(arr_itr->GetString()));
								shrineEffectPools[GUI_TYPE_ETERNALSHRINE_SUPPLICATION][pair.second].push_back(arr_itr->GetString());
							}
							else
							{
								printlog("[JSON]: shrine_effects.json warning: no effect key found for '%s'!", arr_itr->GetString());
							}
						}
					}
				}
			}
			else
			{
				hash += (Uint32)((Uint32)poolIndex << (shift % 32)); ++shift;
			}
		}

		std::vector<std::pair<std::string, std::string>> exclusions = {
			{"EXCLUDE_HP", "hp" },
			{"EXCLUDE_MP", "mp" },
			{"EXCLUDE_CURE", "cure" },
			{"EXCLUDE_FOOD", "food" },
			{"EXCLUDE_FORTUNE", "fortune"}
		};
		for ( auto& pair : exclusions )
		{
			std::string str = pair.first;
			std::string key = pair.second;
			if ( effects.HasMember(str.c_str()) )
			{
				if ( effects[str.c_str()].IsArray() )
				{
					for ( auto arr_itr = effects[str.c_str()].Begin(); arr_itr != effects[str.c_str()].End(); ++arr_itr )
					{
						if ( arr_itr->IsString() )
						{
							hash += djb2Hash(const_cast<char*>(arr_itr->GetString()));
							supplicationExcludeStrings[key.c_str()].insert(arr_itr->GetString());
						}
					}
				}
			}
		}
	}

	shrineOutcomes[GUI_TYPE_ETERNALSHRINE_ANVIL].clear();
	shrineOutcomes[GUI_TYPE_ETERNALSHRINE_SUPPLICATION].clear();
	shrineOutcomes[GUI_TYPE_ETERNALSHRINE_MUSIC].clear();
	shrineOutcomes[GUI_TYPE_ETERNALSHRINE_ASCENSION].clear();

	shrineRewards[GUI_TYPE_ETERNALSHRINE_ANVIL].clear();
	shrineRewards[GUI_TYPE_ETERNALSHRINE_SUPPLICATION].clear();
	shrineRewards[GUI_TYPE_ETERNALSHRINE_MUSIC].clear();
	shrineRewards[GUI_TYPE_ETERNALSHRINE_ASCENSION].clear();

	std::map<std::string, ShrineEffectsPools> effectsTableLookup;
	for ( auto& entry : shrineEffectsTable )
	{
		effectsTableLookup[entry.first] = entry.second;
	}

	auto& outcomes_all = d["outcomes"];
	if ( outcomes_all.HasMember("anvil") )
	{
		auto& shrineOutcome = shrineOutcomes[GUI_TYPE_ETERNALSHRINE_ANVIL];
		char buf[32];
		for ( int i = 0; i <= 4; ++i )
		{
			for ( int j = 0; j <= 3; ++j )
			{
				snprintf(buf, sizeof(buf), "%d-%d", i, j);
				auto& outcome = outcomes_all["anvil"];
				auto& entry = shrineOutcome[buf];
				if ( outcome.HasMember(buf) )
				{
					if ( outcome[buf].IsArray() )
					{
						for ( auto arr_itr = outcome[buf].Begin(); arr_itr != outcome[buf].End(); ++arr_itr )
						{
							if ( arr_itr->IsObject() )
							{
								for ( auto itr = arr_itr->MemberBegin(); itr != arr_itr->MemberEnd(); ++itr )
								{
									if ( effectsTableLookup.find(itr->name.GetString()) == effectsTableLookup.end() )
									{
										printlog("[JSON]: shrine_effects.json warning: no effect pool found for '%s'!", itr->name.GetString());
									}
									else
									{
										entry.push_back(std::make_pair(effectsTableLookup[itr->name.GetString()], itr->value.GetInt()));

										hash += (Uint32)((Uint32)entry.back().first << (shift % 32)); ++shift;
										hash += (Uint32)((Uint32)entry.back().second << (shift % 32)); ++shift;
									}
								}
							}
						}
					}
				}
				int sum = 0;
				for ( auto& chance : entry )
				{
					sum += chance.second;
				}
				if ( sum < 100 )
				{
					entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 100 - sum));
				}
				else
				{
					entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 0));
				}
			}
		}
	}
	if ( outcomes_all.HasMember("supplication") )
	{
		auto& shrineOutcome = shrineOutcomes[GUI_TYPE_ETERNALSHRINE_SUPPLICATION];
		char buf[32];
		for ( int i = 0; i <= 4; ++i )
		{
			snprintf(buf, sizeof(buf), "%d", i);
			auto& outcome = outcomes_all["supplication"];
			auto& entry = shrineOutcome[buf];
			if ( outcome.HasMember(buf) )
			{
				if ( outcome[buf].IsArray() )
				{
					for ( auto arr_itr = outcome[buf].Begin(); arr_itr != outcome[buf].End(); ++arr_itr )
					{
						if ( arr_itr->IsObject() )
						{
							for ( auto itr = arr_itr->MemberBegin(); itr != arr_itr->MemberEnd(); ++itr )
							{
								if ( effectsTableLookup.find(itr->name.GetString()) == effectsTableLookup.end() )
								{
									printlog("[JSON]: shrine_effects.json warning: no effect pool found for '%s'!", itr->name.GetString());
								}
								else
								{
									entry.push_back(std::make_pair(effectsTableLookup[itr->name.GetString()], itr->value.GetInt()));

									hash += (Uint32)((Uint32)entry.back().first << (shift % 32)); ++shift;
									hash += (Uint32)((Uint32)entry.back().second << (shift % 32)); ++shift;
								}
							}
						}
					}
				}
			}
			int sum = 0;
			for ( auto& chance : entry )
			{
				sum += chance.second;
			}
			if ( sum < 100 )
			{
				entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 100 - sum));
			}
			else
			{
				entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 0));
			}
		}
	}

	if ( outcomes_all.HasMember("ascension") )
	{
		auto& shrineOutcome = shrineOutcomes[GUI_TYPE_ETERNALSHRINE_ASCENSION];
		char buf[32];
		for ( int i = 0; i <= 4; ++i )
		{
			snprintf(buf, sizeof(buf), "%d", i);
			auto& outcome = outcomes_all["ascension"];
			auto& entry = shrineOutcome[buf];
			if ( outcome.HasMember(buf) )
			{
				if ( outcome[buf].IsArray() )
				{
					for ( auto arr_itr = outcome[buf].Begin(); arr_itr != outcome[buf].End(); ++arr_itr )
					{
						if ( arr_itr->IsObject() )
						{
							for ( auto itr = arr_itr->MemberBegin(); itr != arr_itr->MemberEnd(); ++itr )
							{
								if ( effectsTableLookup.find(itr->name.GetString()) == effectsTableLookup.end() )
								{
									printlog("[JSON]: shrine_effects.json warning: no effect pool found for '%s'!", itr->name.GetString());
								}
								else
								{
									entry.push_back(std::make_pair(effectsTableLookup[itr->name.GetString()], itr->value.GetInt()));

									hash += (Uint32)((Uint32)entry.back().first << (shift % 32)); ++shift;
									hash += (Uint32)((Uint32)entry.back().second << (shift % 32)); ++shift;
								}
							}
						}
					}
				}
			}
			int sum = 0;
			for ( auto& chance : entry )
			{
				sum += chance.second;
			}
			if ( sum < 100 )
			{
				entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 100 - sum));
			}
			else
			{
				entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 0));
			}
		}
	}

	auto& rewards_all = d["rewards"];
	if ( rewards_all.HasMember("supplication") )
	{
		auto& shrineOutcome = shrineRewards[GUI_TYPE_ETERNALSHRINE_SUPPLICATION];
		char buf[32];

		const std::vector<std::string> effectPairs = {
			"hp-mp",
			"hp-cure",
			"hp-food",
			"hp-fortune",
			"mp-cure",
			"mp-food",
			"mp-fortune",
			"cure-food",
			"cure-fortune",
			"food-fortune",
			"fortune"
		};
		for ( int i = 0; i <= 4; ++i )
		{
			for ( auto& effectPair : effectPairs )
			{
				snprintf(buf, sizeof(buf), "%d-%s", i, effectPair.c_str());
				auto& outcome = rewards_all["supplication"];
				auto& entry = shrineOutcome[buf];
				if ( outcome.HasMember(buf) )
				{
					if ( outcome[buf].IsArray() )
					{
						for ( auto arr_itr = outcome[buf].Begin(); arr_itr != outcome[buf].End(); ++arr_itr )
						{
							if ( arr_itr->IsObject() )
							{
								for ( auto itr = arr_itr->MemberBegin(); itr != arr_itr->MemberEnd(); ++itr )
								{
									if ( effectsTableLookup.find(itr->name.GetString()) == effectsTableLookup.end() )
									{
										printlog("[JSON]: shrine_effects.json warning: no effect pool found for '%s'!", itr->name.GetString());
									}
									else
									{
										entry.push_back(std::make_pair(effectsTableLookup[itr->name.GetString()], itr->value.GetInt()));

										hash += (Uint32)((Uint32)entry.back().first << (shift % 32)); ++shift;
										hash += (Uint32)((Uint32)entry.back().second << (shift % 32)); ++shift;
									}
								}
							}
						}
					}
				}
				int sum = 0;
				for ( auto& chance : entry )
				{
					sum += chance.second;
				}
				if ( sum < 100 )
				{
					entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 100 - sum));
				}
				else
				{
					entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 0));
				}
			}
		}
	}
	if ( rewards_all.HasMember("chorale") )
	{
		auto& shrineOutcome = shrineRewards[GUI_TYPE_ETERNALSHRINE_MUSIC];
		char buf[32];

		for ( int i = 0; i <= 4; ++i )
		{
			snprintf(buf, sizeof(buf), "%d", i);
			auto& outcome = rewards_all["chorale"];
			auto& entry = shrineOutcome[buf];
			if ( outcome.HasMember(buf) )
			{
				if ( outcome[buf].IsArray() )
				{
					for ( auto arr_itr = outcome[buf].Begin(); arr_itr != outcome[buf].End(); ++arr_itr )
					{
						if ( arr_itr->IsObject() )
						{
							for ( auto itr = arr_itr->MemberBegin(); itr != arr_itr->MemberEnd(); ++itr )
							{
								if ( effectsTableLookup.find(itr->name.GetString()) == effectsTableLookup.end() )
								{
									printlog("[JSON]: shrine_effects.json warning: no effect pool found for '%s'!", itr->name.GetString());
								}
								else
								{
									entry.push_back(std::make_pair(effectsTableLookup[itr->name.GetString()], itr->value.GetInt()));

									hash += (Uint32)((Uint32)entry.back().first << (shift % 32)); ++shift;
									hash += (Uint32)((Uint32)entry.back().second << (shift % 32)); ++shift;
								}
							}
						}
					}
				}
			}
			int sum = 0;
			for ( auto& chance : entry )
			{
				sum += chance.second;
			}
			if ( sum < 100 )
			{
				entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 100 - sum));
			}
			else
			{
				entry.push_back(std::make_pair(ShrineEffectsPools::EFFECT_EMPTY, 0));
			}
		}
	}

	skillItemMap.clear();
	if ( d.HasMember("skill_items") )
	{
		for ( int skillID = 0; skillID < NUMPROFICIENCIES; ++skillID )
		{
			for ( auto itr = d["skill_items"].MemberBegin(); itr != d["skill_items"].MemberEnd(); ++itr )
			{
				if ( itr->value.HasMember("id") && itr->value["id"].IsInt() && itr->value["id"].GetInt() == skillID )
				{
					auto& allPool = skillItemMap[skillID];
					for ( int tier = 0; tier < 3; ++tier )
					{
						char tierString[16];
						snprintf(tierString, sizeof(tierString), "%d", tier);
						if ( !itr->value.HasMember(tierString) )
						{
							continue;
						}

						auto& pool = allPool.pool[tier + 1];
						for ( auto item_itr = itr->value[tierString].MemberBegin(); item_itr != itr->value[tierString].MemberEnd(); ++item_itr )
						{
							std::string itemName = item_itr->name.GetString();

							hash += djb2Hash(const_cast<char*>(itemName.c_str()));
							hash += (Uint32)((Uint32)item_itr->value.GetInt() << (shift % 32)); ++shift;

							pool.push_back(std::vector<Item>());
							auto& def = pool.back();
							if ( ItemTooltips.itemNameStringToItemID.find(itemName) != ItemTooltips.itemNameStringToItemID.end() )
							{
								auto& item = def.emplace_back(Item());
								item.type = (ItemType)ItemTooltips.itemNameStringToItemID[itemName];
								item.beatitude = item_itr->value.GetInt();
								item.status = EXCELLENT;
								item.count = 1;
								item.identified = false;
								item.appearance = 0;
							}
							else
							{
								if ( itemName == "cat_lockpicks" )
								{
									auto& item = def.emplace_back(Item());
									item.type = TOOL_LOCKPICK;
									item.beatitude = item_itr->value.GetInt();
									item.status = EXCELLENT;
									item.count = 5;
									item.identified = false;
									item.appearance = 0;

									{
										auto& item = def.emplace_back(Item());
										item.type = TOOL_BEARTRAP;
										item.beatitude = item_itr->value.GetInt();
										item.status = EXCELLENT;
										item.count = 3;
										item.identified = false;
										item.appearance = 0;
									}
								}
								else if ( itemName == "cat_gold" )
								{
									auto& item = def.emplace_back(Item());
									item.type = GEM_ROCK;
									item.beatitude = 0;
									item.status = EXCELLENT;
									item.count = item_itr->value.GetInt();
									item.identified = false;
									item.appearance = 0;
								}
								else if ( itemName == "cat_water" )
								{
									auto& item = def.emplace_back(Item());
									item.type = POTION_WATER;
									item.beatitude = item_itr->value.GetInt();
									item.status = EXCELLENT;
									item.count = 5;
									item.identified = false;
									item.appearance = 0;
								}
								else if ( itemName == "cat_acid" )
								{
									auto& item = def.emplace_back(Item());
									item.type = POTION_ACID;
									item.beatitude = item_itr->value.GetInt();
									item.status = EXCELLENT;
									item.count = 5;
									item.identified = false;
									item.appearance = 0;
								}
								else if ( itemName == "cat_revenant_skull" )
								{
									auto& item = def.emplace_back(Item());
									item.type = WOODEN_SHIELD;
									item.beatitude = 0;
									item.status = EXCELLENT;
									item.count = item_itr->value.GetInt();
									item.identified = false;
									item.appearance = REVENANT_SKULL;
								}
								else if ( itemName == "cat_earth_sprite" )
								{
									auto& item = def.emplace_back(Item());
									item.type = WOODEN_SHIELD;
									item.beatitude = 0;
									item.status = EXCELLENT;
									item.count = item_itr->value.GetInt();
									item.identified = false;
									item.appearance = EARTH_ELEMENTAL;
								}
								else if ( itemName == "cat_spellbook" )
								{
									int difficulty = item_itr->value.GetInt() * 20;
									bool first = true;
									for ( auto& spellDef : allGameSpells )
									{
										if ( auto spell = spellDef.second )
										{
											if ( spell->difficulty == difficulty
												&& !spell->hide_from_ui
												&& spell->drop_table >= 0
												&& spell->skillID == skillID )
											{
												int spellbook = getSpellbookFromSpellID(spell->ID);
												if ( items[spellbook].category == SPELLBOOK )
												{
													if ( !first )
													{
														pool.push_back(std::vector<Item>());
													}
													first = false;
													auto& def = pool.back();

													auto& item = def.emplace_back(Item());
													item.type = (ItemType)spellbook;
													item.beatitude = 0;
													item.status = EXCELLENT;
													item.count = 1;
													item.identified = false;
													item.appearance = 0;
												}
												else
												{
													if ( !first )
													{
														pool.push_back(std::vector<Item>());
													}
													first = false;
													auto& def = pool.back();

													auto& item = def.emplace_back(Item());
													item.type = TOME_SORCERY;
													if ( skillID == PRO_THAUMATURGY )
													{
														item.type = TOME_THAUMATURGY;
													}
													else if ( skillID == PRO_MYSTICISM )
													{
														item.type = TOME_MYSTICISM;
													}
													item.beatitude = 0;
													item.status = EXCELLENT;
													item.count = 1;
													item.identified = false;
													item.appearance = spellTomeIDToAppearance[spell->ID];
												}
											}
										}
									}
								}
								else if ( itemName == "cat_instrument" )
								{
									auto& item = def.emplace_back(Item());
									item.type = INSTRUMENT_FLUTE;
									item.beatitude = item_itr->value.GetInt();
									item.status = EXCELLENT;
									item.count = 1;
									item.identified = false;
									item.appearance = 0;

									{
										pool.push_back(std::vector<Item>());
										auto& def = pool.back();
										auto& item = def.emplace_back(Item());
										item.type = INSTRUMENT_LUTE;
										item.beatitude = item_itr->value.GetInt();
										item.status = EXCELLENT;
										item.count = 1;
										item.identified = false;
										item.appearance = 0;
									}

									{
										pool.push_back(std::vector<Item>());
										auto& def = pool.back();
										auto& item = def.emplace_back(Item());
										item.type = INSTRUMENT_HORN;
										item.beatitude = item_itr->value.GetInt();
										item.status = EXCELLENT;
										item.count = 1;
										item.identified = false;
										item.appearance = 0;
									}

									{
										pool.push_back(std::vector<Item>());
										auto& def = pool.back();
										auto& item = def.emplace_back(Item());
										item.type = INSTRUMENT_DRUM;
										item.beatitude = item_itr->value.GetInt();
										item.status = EXCELLENT;
										item.count = 1;
										item.identified = false;
										item.appearance = 0;
									}

									{
										pool.push_back(std::vector<Item>());
										auto& def = pool.back();
										auto& item = def.emplace_back(Item());
										item.type = INSTRUMENT_LYRE;
										item.beatitude = item_itr->value.GetInt();
										item.status = EXCELLENT;
										item.count = 1;
										item.identified = false;
										item.appearance = 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	printlog("[JSON]: Successfully read json file %s", inputPath.c_str());

	shrineJsonHashRead = hash;
	if ( shrineJsonHashRead != kShrineJsonHash )
	{
		printlog("[JSON]: Notice: shrine_effects.json unknown hash, achievements are disabled: %d", kShrineJsonHash);
	}
	else
	{
		printlog("[JSON]: shrine_effects.json hash verified successfully.");
	}
}

static bool debug_anvil_results = false;
static ConsoleCommand ccmd_eternal_shrine_anvil_view("/eternal_shrine_anvil_debug_view", "", 
	[](int argc, const char* argv[]) {
	if ( !(svFlags & SV_FLAG_CHEATS) )
	{
		messagePlayer(clientnum, MESSAGE_MISC, Language::get(277));
		return;
	}

	for ( auto node = map.entities->first; node; node = node->next )
	{
		if ( Entity* entity = (Entity*)node->element )
		{
			if ( entity->behavior == &actEternalShrine && entity->eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
			{
				debug_anvil_results = true;
				Item** slot = &stats[clientnum]->weapon;
				if ( argc >= 2 && argv[1][0] == '1' )
				{
					slot = &stats[clientnum]->shield;
				}
				if ( slot && *slot )
				{
					Status prev = (*slot)->status;
					for ( int status = BROKEN; status <= EXCELLENT; ++status )
					{
						(*slot)->status = (Status)status;
						ShrineEffects_t::getTierStringFromEffect(clientnum, *entity, local_rng, (*slot));
					}
					(*slot)->status = prev;
				}
				debug_anvil_results = false;
				break;
			}
		}
	}
});

std::string ShrineEffects_t::getTierStringFromEffect(const int player, Entity& my, BaronyRNG& rng, Item* item)
{
	if ( player < 0 || player >= MAXPLAYERS ) { return "0-0"; }
	if ( !players[player]->entity ) { return "0-0"; }

	std::string result = "0-0";
	if ( players[player]->mechanics.getDivineFavorPips() >= 9 )
	{
		result = "4-";
	}
	else if ( players[player]->mechanics.getDivineFavorPips() >= 7 )
	{
		result = "3-";
	}
	else if ( players[player]->mechanics.getDivineFavorPips() >= 5 )
	{
		result = "2-";
	}
	else if ( players[player]->mechanics.getDivineFavorPips() >= 3 )
	{
		result = "1-";
	}
	else
	{
		result = "0-";
	}

	if ( my.eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC )
	{
		result = result.substr(0, 1);
		return result;
	}

	if ( my.eternalShrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
	{
		result = result.substr(0, 1);
		return result;
	}

	if ( my.eternalShrineType == GUI_TYPE_ETERNALSHRINE_ANVIL )
	{
		if ( !item )
		{
			result += std::to_string(rng.rand() % 4);
			return result;
		}

		std::vector<unsigned int> chances;
		chances.resize(4, 0);
		enum ChanceDifficulty
		{
			NONE,
			EASY,
			MED,
			HARD
		};

		struct ModFuncs
		{
			void mod_item(Item* item, const int repairMod, const int blessMod) {
				if ( !item ) { return; }
				
			};
			enum ModType
			{
				MOD_NORMAL,
				MOD_CURSE
			};
			int repairMod = 0;
			int blessMod = 0;
			bool deleteItem = false;
			bool setItem = false;
			bool enhanceItem = false;
			bool ignoreRepairItem(Item* item)
			{
				if ( item )
				{
					if ( items[item->type].category == THROWN
						&& item->type != BOOMERANG )
					{
						if ( item->status != BROKEN )
						{
							return true;
						}
					}
					if ( itemTypeIsQuiver(item->type) )
					{
						if ( item->status != BROKEN )
						{
							return true;
						}
					}
				}

				return false;
			}
			bool isArtifact(Item* item)
			{
				if ( item )
				{
					if ( item->type == ARTIFACT_AXE
						|| item->type == ARTIFACT_SWORD
						|| item->type == ARTIFACT_MACE
						|| item->type == ARTIFACT_SPEAR
						|| item->type == ARTIFACT_AXE
						|| item->type == ARTIFACT_BOW
						|| item->type == ARTIFACT_BREASTPIECE
						|| item->type == ARTIFACT_HELM
						|| item->type == ARTIFACT_BOOTS
						|| item->type == ARTIFACT_CLOAK
						|| item->type == ARTIFACT_GLOVES
						|| item->type == MASK_ARTIFACT_VISOR )
					{
						return true;
					}
				}
				return false;
			}
			void setDeleteItem()
			{
				deleteItem = true;
			}
			bool setItemMod(Item* item, int _repairMod, int _blessMod)
			{
				repairMod = _repairMod;
				blessMod = _blessMod;

				if ( _blessMod == 0 )
				{
					if ( ignoreRepairItem(item) )
					{
						return false;
					}
				}

				return true;
			}
			void setItemProp(Item* item, int _repairNew, int _blessNew)
			{
				setItem = true;

				if ( ignoreRepairItem(item) )
				{
					// no repair
					repairMod = item->status;
				}
				else if ( isArtifact(item) || (item && item->type == BOOMERANG) )
				{
					repairMod = std::min((int)item->status + 1, _repairNew);
				}
				else
				{
					repairMod = _repairNew;
				}

				if ( isArtifact(item) )
				{
					blessMod = std::min((int)std::max(0, (int)item->beatitude) + 1, _blessNew);
				}
				else
				{
					blessMod = _blessNew;
				}
			}
			bool setEnhanceItem(Item* item)
			{
				enhanceItem = false;
				if ( GenericGUIMenu::isItemEnhanceable(item, -1) )
				{
					enhanceItem = true;
				}
				return enhanceItem;
			}

			bool run(Item* item)
			{
				if ( !item ) { return false; }
				bool artifact = isArtifact(item);
				
				if ( deleteItem )
				{
					item->count = 0;
					return true;
				}

				if ( enhanceItem )
				{
					if ( GenericGUIMenu::isItemEnhanceable(item, -1) )
					{
						int type = GenericGUIMenu::getItemEnhanceResult(item);
						if ( type >= 0 )
						{
							item->type = ItemType(type);
						}
					}
				}

				if ( item->type == ENCHANTED_FEATHER )
				{
					if ( item->appearance % ENCHANTED_FEATHER_MAX_DURABILITY < 100 )
					{
						int durability = item->appearance % ENCHANTED_FEATHER_MAX_DURABILITY;
						int repairAmount = 100 - durability;
						item->appearance += repairAmount;
					}
				}
				else if ( item->type == MAGICSTAFF_SCEPTER || (MAGICSTAFFS_USE_CHARGE && itemCategory(item) == MAGICSTAFF) )
				{
					if ( item->appearance % MAGICSTAFF_SCEPTER_CHARGE_MAX < 100 )
					{
						int durability = item->appearance % MAGICSTAFF_SCEPTER_CHARGE_MAX;
						int repairAmount = ((MAGICSTAFF_SCEPTER_CHARGE_MAX - 1) - durability);
						item->appearance += repairAmount;
					}
				}
				else if ( item->type == TOOL_SENTRYBOT || item->type == TOOL_SPELLBOT || item->type == TOOL_DUMMYBOT
					|| item->type == TOOL_GYROBOT )
				{
					if ( !item->tinkeringBotIsMaxHealth() )
					{
						item->appearance = ITEM_TINKERING_APPEARANCE;
					}
				}

				if ( setItem )
				{
					item->status = (Status)repairMod;
					item->beatitude = blessMod;
					return true;
				}
				
				if ( repairMod != 0 )
				{
					if ( !ignoreRepairItem(item) )
					{
						if ( artifact || (item && item->type == BOOMERANG) )
						{
							item->status = (Status)std::max((int)BROKEN, std::min((int)EXCELLENT, (item->status + std::min(1, repairMod))));
						}
						else
						{
							item->status = (Status)std::max((int)BROKEN, std::min((int)EXCELLENT, (item->status + repairMod)));
						}
					}
				}
				if ( blessMod != 0 )
				{
					if ( artifact )
					{
						item->beatitude += std::min(1, blessMod);
					}
					else
					{
						item->beatitude += blessMod;
					}
				}
				return true;
			}
		};

		bool canCharge = false;
		if ( item->type == ENCHANTED_FEATHER )
		{
			if ( item->appearance % ENCHANTED_FEATHER_MAX_DURABILITY < 100 )
			{
				canCharge = true;
			}
		}
		else if ( item->type == MAGICSTAFF_SCEPTER || (MAGICSTAFFS_USE_CHARGE && itemCategory(item) == MAGICSTAFF) )
		{
			if ( item->appearance % MAGICSTAFF_SCEPTER_CHARGE_MAX < 100 )
			{
				canCharge = true;
			}
		}
		else if ( item->type == TOOL_SENTRYBOT || item->type == TOOL_SPELLBOT || item->type == TOOL_DUMMYBOT
			|| item->type == TOOL_GYROBOT )
		{
			if ( !item->tinkeringBotIsMaxHealth() )
			{
				canCharge = true;
			}
		}

		std::map<ChanceDifficulty, ModFuncs> chance_funcs;

		if ( result.find("4-") != std::string::npos )
		{
			if ( item->beatitude < 0 )
			{
				chance_funcs[HARD].setItemProp(item, std::min((int)EXCELLENT, (int)item->status + 4), std::min(4, abs(item->beatitude)));
				chances[HARD] = 2;

				//chance_funcs[MED].setItemProp(item, std::min((int)EXCELLENT, (int)item->status + 4), 0);
				//chances[MED] = 3;

				//chance_funcs[EASY].setItemProp(item, std::min((int)EXCELLENT, (int)item->status + 2), 0);
				//chances[EASY] = 3;

				//chance_funcs[NONE].setItemMod(-1, 1);
				//chances[NONE] = 5;
			}
			else
			{
				if ( item->status + 0 <= (int)EXCELLENT )
				{
					chances[HARD] = 2;
					chance_funcs[HARD].setEnhanceItem(item);
					chance_funcs[HARD].setItemMod(item, 4, 2);
				}
				if ( item->status + 0 <= (int)EXCELLENT )
				{
					chances[MED] = 3;
					chance_funcs[MED].setEnhanceItem(item);
					chance_funcs[MED].setItemMod(item, 4, item->beatitude == 0 ? 3 : 1);
				}
				if ( item->status + 0 <= (int)EXCELLENT )
				{
					chances[EASY] = 3;
					chance_funcs[EASY].setEnhanceItem(item);
					chance_funcs[EASY].setItemMod(item, 4, item->beatitude == 0 ? 2 : 1);
				}
				/*if ( item->status + 0 <= (int)EXCELLENT )
				{
					chances[NONE] = 2;
					chance_funcs[NONE].setItemMod(4, 0);
				}*/
			}
		}
		else if ( result.find("3-") != std::string::npos )
		{
			if ( item->beatitude < 0 )
			{
				chance_funcs[HARD].setItemProp(item, std::min((int)EXCELLENT, (int)item->status + 4), std::min(3, abs(item->beatitude)));
				chances[HARD] = 2;

				//chance_funcs[MED].setItemProp(item, std::min((int)EXCELLENT, (int)item->status + 3), 0);
				//chances[MED] = 3;

				//chance_funcs[EASY].setItemProp(item, std::min((int)EXCELLENT, (int)item->status + 1), 0);
				//chances[EASY] = 4;

				//chance_funcs[NONE].setItemMod(-1, 1);
				//chances[NONE] = 5;
			}
			else
			{
				if ( item->status + 0 <= (int)EXCELLENT )
				{
					chances[HARD] = 2;
					chance_funcs[HARD].setEnhanceItem(item);
					chance_funcs[HARD].setItemMod(item, 4, item->beatitude == 0 ? 2 : 1);
				}
				if ( item->status + 0 <= (int)EXCELLENT )
				{
					chances[MED] = 2;
					chance_funcs[MED].setItemMod(item, 4, 1);
				}
				if ( item->status + 2 <= (int)EXCELLENT )
				{
					chances[EASY] = 4;
					bool enhance = chance_funcs[EASY].setEnhanceItem(item);
					bool res = chance_funcs[EASY].setItemMod(item, 3, 0);

					if ( !enhance && !res )
					{
						chances[EASY] = 0;
					}
				}
				/*if ( item->status + 3 <= (int)EXCELLENT )
				{
					chances[NONE] = 2;
					chance_funcs[EASY].setEnhanceItem(item);
					chance_funcs[NONE].setItemMod(3, 0);
				}*/
			}
		}
		else if ( result.find("2-") != std::string::npos )
		{
			if ( item->beatitude < 0 )
			{
				chance_funcs[HARD].setItemProp(item, item->status, std::min(2, abs(item->beatitude)));
				chances[HARD] = 2;

				chance_funcs[MED].setItemProp(item, std::min((int)EXCELLENT, (int)item->status + 4), 0);
				chances[MED] = 2;

				//chance_funcs[EASY].setItemProp(item, item->status, 0);
				//chances[EASY] = 5;

				//chance_funcs[NONE].setItemMod(-1, 1);
				//chances[NONE] = 5;
			}
			else
			{
				if ( item->status + 0 <= (int)EXCELLENT )
				{
					chances[HARD] = 2;
					if ( chance_funcs[HARD].setEnhanceItem(item) )
					{
						chance_funcs[HARD].setItemMod(item, 2, 0);
					}
					else
					{
						chance_funcs[HARD].setItemMod(item, 2, 1);
					}
				}
				if ( item->status + 4 <= (int)EXCELLENT )
				{
					chances[MED] = 2;
					bool res = chance_funcs[MED].setItemMod(item, 4, 0);
					if ( !res ) { chances[MED] = 0; }
				}
				if ( item->status + 3 <= (int)EXCELLENT )
				{
					chances[EASY] = 4;
					bool res = chance_funcs[EASY].setItemMod(item, 3, 0);
					if ( !res ) { chances[EASY] = 0; }
				}
				if ( item->status + 2 <= (int)EXCELLENT )
				{
					chances[NONE] = 2;
					bool res = chance_funcs[NONE].setItemMod(item, 2, 0);
					if ( !res ) { chances[NONE] = 0; }
				}
			}
		}
		else if ( result.find("1-") != std::string::npos )
		{
			if ( item->beatitude < 0 )
			{
				chance_funcs[HARD].setItemProp(item, item->status, std::min(1, abs(item->beatitude)));
				chances[HARD] = 1;

				chance_funcs[MED].setItemProp(item, std::min((int)EXCELLENT, (int)item->status + 4), 0);
				chances[MED] = 3;

				//chance_funcs[EASY].setItemProp(item, item->status, 0);
				//chances[EASY] = 3;
				
				//chance_funcs[NONE].setItemMod(-1, 1);
				//chances[NONE] = 5;
			}
			else
			{
				if ( item->status <= (int)EXCELLENT )
				{
					chances[HARD] = 1;
					if ( chance_funcs[HARD].setEnhanceItem(item) )
					{
						chance_funcs[HARD].setItemMod(item, 1, 0);
					}
					else
					{
						chance_funcs[HARD].setItemMod(item, 1, 1);
					}
				}
				if ( item->status + 3 <= (int)EXCELLENT )
				{
					chances[MED] = 2;
					bool res = chance_funcs[MED].setItemMod(item, 4, 0);
					if ( !res ) { chances[MED] = 0; }
				}
				if ( item->status + 2 <= (int)EXCELLENT )
				{
					chances[EASY] = 5;
					bool res = chance_funcs[EASY].setItemMod(item, 2, 0);
					if ( !res ) { chances[EASY] = 0; }
				}
				if ( item->status + 1 <= (int)EXCELLENT )
				{
					chances[NONE] = 2;
					bool res = chance_funcs[NONE].setItemMod(item, 2, 0);
					if ( !res ) { chances[NONE] = 0; }
				}
			}
		}
		else
		{
			if ( item->beatitude < 0 )
			{
				chance_funcs[HARD].setItemProp(item, BROKEN, 0);
				chances[HARD] = 1;
			}
			else
			{
				if ( item->status + 2 <= (int)EXCELLENT )
				{
					chances[MED] = 3;
					bool res = chance_funcs[MED].setItemMod(item, 2, 0);
					if ( !res ) { chances[MED] = 0; }
				}
				if ( item->status + 1 <= (int)EXCELLENT )
				{
					chances[EASY] = 5;
					bool res = chance_funcs[EASY].setItemMod(item, 2, 0);
					if ( !res ) { chances[EASY] = 0; }
				}

				chances[NONE] = 2;
				if ( !canCharge )
				{
					if ( chance_funcs[NONE].ignoreRepairItem(item) )
					{
						if ( !chance_funcs[NONE].setEnhanceItem(item) )
						{
							chance_funcs[NONE].setDeleteItem();
						}
					}
					else
					{
						chance_funcs[NONE].setDeleteItem();
					}
				}
				else
				{
					chance_funcs[NONE].setItemMod(item, 1, 0);
				}
			}
		}
		bool anychances = false;
		int chanceIndex = -1;
		int chanceTotal = 0;
		for ( auto chance : chances )
		{
			chanceTotal += chance;
		}
		for ( auto chance : chances )
		{
			++chanceIndex;
			if ( chance )
			{
				anychances = true;
			}
			if ( debug_anvil_results )
			{
				if ( chanceIndex == NONE )
				{
					messagePlayer(player, MESSAGE_MISC, "Tier %c Anvil %s: [NONE]: %.1f%%", 
						result[0], ItemTooltips.getItemStatusAdjective(item->type, item->status).c_str(), chance * 100 / (real_t)chanceTotal);
				}
				else if ( chanceIndex == EASY )
				{
					messagePlayer(player, MESSAGE_MISC, "Tier %c Anvil %s: [EASY]: %.1f%%",
						result[0], ItemTooltips.getItemStatusAdjective(item->type, item->status).c_str(), chance * 100 / (real_t)chanceTotal);
				}
				else if ( chanceIndex == MED )
				{
					messagePlayer(player, MESSAGE_MISC, "Tier %c Anvil %s: [MED]: %.1f%%",
						result[0], ItemTooltips.getItemStatusAdjective(item->type, item->status).c_str(), chance * 100 / (real_t)chanceTotal);
				}
				else if ( chanceIndex == HARD )
				{
					messagePlayer(player, MESSAGE_MISC, "Tier %c Anvil %s: [HARD]: %.1f%%",
						result[0], ItemTooltips.getItemStatusAdjective(item->type, item->status).c_str(), chance * 100 / (real_t)chanceTotal);
				}
			}
		}
		if ( debug_anvil_results )
		{
			return result;
		}
		if ( anychances )
		{
			int pick = rng.discrete(chances.data(), chances.size());
			chance_funcs[(ChanceDifficulty)pick].run(item);
			result += std::to_string(pick);
			return result;

		}
		else
		{
			result += std::to_string(rng.rand() % 4);
			return result;
		}
	}
	else if ( my.eternalShrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
	{
		std::string str = "";
		int numModifiers = 0;
		if ( numModifiers < 2 && ((stats[player]->HP / (real_t)stats[player]->MAXHP) <= 0.75) )
		{
			str += str == "" ? "hp" : "-hp";
			++numModifiers;
		}
		if ( numModifiers < 2 && ((stats[player]->MP / (real_t)stats[player]->MAXMP) <= 0.75) )
		{
			str += str == "" ? "mp" : "-mp";
			++numModifiers;
		}
		if ( numModifiers < 2 )
		{
			if ( players[player]->entity->flags[BURNING] )
			{
				str += str == "" ? "cure" : "-cure";
				++numModifiers;
			}
			else if ( stats[player]->getEffectActive(EFF_WITHDRAWAL) )
			{
				str += str == "" ? "cure" : "-cure";
				++numModifiers;
			}
			else
			{
				for ( int i = 0; i < NUMEFFECTS; ++i )
				{
					if ( stats[player]->getEffectActive(i) && stats[player]->statusEffectRemovedByCureAilment(i, players[player]->entity) )
					{
						str += str == "" ? "cure" : "-cure";
						++numModifiers;
						break;
					}
				}
			}
		}

		bool foodAllowed = false;
		if ( stats[player]->HUNGER + players[player]->mechanics.client_hunger_score <= 2500 )
		{
			foodAllowed = true;
		}
		if ( stats[player]->type == SKELETON && (svFlags & SV_FLAG_HUNGER) )
		{
			foodAllowed = false;
		}

		if ( numModifiers < 2 && foodAllowed )
		{
			str += str == "" ? "food" : "-food";
			++numModifiers;
		}
		if ( numModifiers < 2 )
		{
			str += str == "" ? "fortune" : "-fortune";
			++numModifiers;
		}
		result += str;
	}

	return result;
}

std::pair<std::string, int> ShrineEffects_t::rollResult(int shrineType, ShrineEffectResults resultType, int player, std::string tierString, BaronyRNG& rng, Item* item)
{
	std::pair<std::string, int> result = { "", 1 };

	auto& resultMap = resultType == SHRINE_RESULT_OUTCOME ? shrineOutcomes : shrineRewards;

	if ( resultMap.find(shrineType) == resultMap.end() ) { return result; }
	if ( shrineEffectPools.find(shrineType) == shrineEffectPools.end() ) { return result; }
	auto& results = resultMap[shrineType];

	ShrineEffectsPools poolResult = ShrineEffectsPools::EFFECT_EMPTY;
	std::string resultsLookupString = tierString;

	if ( shrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION && resultType == SHRINE_RESULT_OUTCOME )
	{
		resultsLookupString = tierString.substr(0, 1);
	}
	else if ( shrineType == GUI_TYPE_ETERNALSHRINE_ASCENSION && resultType == SHRINE_RESULT_OUTCOME )
	{
		resultsLookupString = tierString.substr(0, 1);
	}
	else if ( shrineType == GUI_TYPE_ETERNALSHRINE_MUSIC && resultType == SHRINE_RESULT_REWARD )
	{
		resultsLookupString = tierString.substr(0, 1);
	}

	if ( results.find(resultsLookupString) == results.end() ) { return result; }

	int negativeActionModifier = 0;
	if ( player >= 0 && player < MAXPLAYERS )
	{
		negativeActionModifier = players[player]->mechanics.getDivinePenaltyModifier();
	}

	std::vector<unsigned int> chances;
	bool anychances = false;
	for ( auto& pair : results[resultsLookupString] )
	{
		if ( shrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION 
			&& resultType == SHRINE_RESULT_OUTCOME
			&& pair.first == ShrineEffectsPools::EFFECT_EMPTY )
		{
			if ( tierString == "0-fortune"
				|| tierString == "1-fortune"
				|| tierString == "2-fortune"
				|| tierString == "3-fortune"
				|| tierString == "4-fortune" )
			{
				// ignore empty effects
				chances.push_back(0);
				continue;
			}
		}

		if ( resultType == SHRINE_RESULT_OUTCOME && pair.first == ShrineEffectsPools::EFFECT_EMPTY )
		{
			// reduce nothing outcomes
			if ( negativeActionModifier < 0 )
			{
				if ( pair.second > 0 )
				{
					int chance = std::max(1, pair.second + 2 * negativeActionModifier); // e.g -25 is minus 50 weighting
					chances.push_back(chance);
					if ( chance > 0 )
					{
						anychances = true;
					}
					continue;
				}
			}
		}
		else if ( shrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION
			&& resultType == SHRINE_RESULT_REWARD && pair.first == ShrineEffectsPools::EFFECT_EMPTY )
		{
			// increase nothing outcomes
			if ( negativeActionModifier < 0 )
			{
				int chance = std::max(1, pair.second - 4 * negativeActionModifier); // e.g -25 is plus 100 weighting
				chances.push_back(chance);
				if ( chance > 0 )
				{
					anychances = true;
				}
				continue;
			}
		}

		chances.push_back(pair.second);
		if ( pair.second > 0 )
		{
			anychances = true;
		}
	}

	if ( !anychances )
	{
		return result;
	}

	if ( chances.size() > 0 )
	{
		int pick = rng.discrete(chances.data(), chances.size());
		poolResult = results[resultsLookupString][pick].first;
	}

	if ( poolResult == ShrineEffectsPools::EFFECT_EMPTY ) { return result; }

	auto& effects = shrineEffectPools[shrineType];
	if ( effects.find(poolResult) == effects.end() )
	{
		return result;
	}

	chances.clear();
	anychances = false;
	for ( auto& str : effects[poolResult] )
	{
		if ( shrineType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
		{
			bool skip = false;
			for ( auto& pair : supplicationExcludeStrings )
			{
				if ( tierString.find(pair.first) != std::string::npos )
				{
					if ( pair.second.find(str) != pair.second.end() )
					{
						chances.push_back(0);
						skip = true;
						break;
					}
				}
			}
			if ( skip )
			{
				continue;
			}
		}

		if ( shrineType == GUI_TYPE_ETERNALSHRINE_ANVIL
			&& resultType == SHRINE_RESULT_OUTCOME )
		{
			if ( str == "ADORCISED" )
			{
				if ( !item )
				{
					chances.push_back(0);
					continue;
				}
				if ( item )
				{
					if ( !(itemCategory(item) == WEAPON || itemCategory(item) == THROWN 
						|| itemCategory(item) == POTION || itemCategory(item) == FOOD) )
					{
						chances.push_back(0);
						continue;
					}
				}
			}
		}

		chances.push_back(1);
		anychances = true;
	}

	if ( !anychances )
	{
		return result;
	}

	if ( chances.size() > 0 )
	{
		int pick = rng.discrete(chances.data(), chances.size());
		std::string effect_string = effects[poolResult][pick];
		result.first = effect_string;
		result.second = 1; // tier
		if ( poolResult == ShrineEffectsPools::BODY_2
			|| poolResult == ShrineEffectsPools::MIND_2
			|| poolResult == ShrineEffectsPools::BODY_CHALLENGE_2
			|| poolResult == ShrineEffectsPools::MIND_CHALLENGE_2
			|| poolResult == ShrineEffectsPools::REWARD_ITEM_2
			|| poolResult == ShrineEffectsPools::BODY_BUFF_2
			|| poolResult == ShrineEffectsPools::MIND_BUFF_2
			|| poolResult == ShrineEffectsPools::FOOD_BUFF_2
			|| poolResult == ShrineEffectsPools::STATUS_BUFF_HP_2
			|| poolResult == ShrineEffectsPools::STATUS_BUFF_MP_2
			|| poolResult == ShrineEffectsPools::MIRACLE_2
			|| poolResult == ShrineEffectsPools::SONG_2 )
		{
			result.second = 2;
		}
		else if ( poolResult == ShrineEffectsPools::BODY_3
			|| poolResult == ShrineEffectsPools::MIND_3
			|| poolResult == ShrineEffectsPools::BODY_CHALLENGE_3
			|| poolResult == ShrineEffectsPools::MIND_CHALLENGE_3
			|| poolResult == ShrineEffectsPools::REWARD_ITEM_3
			|| poolResult == ShrineEffectsPools::BODY_BUFF_3
			|| poolResult == ShrineEffectsPools::MIND_BUFF_3
			|| poolResult == ShrineEffectsPools::FOOD_BUFF_3
			|| poolResult == ShrineEffectsPools::STATUS_BUFF_HP_3
			|| poolResult == ShrineEffectsPools::STATUS_BUFF_MP_3
			|| poolResult == ShrineEffectsPools::MIRACLE_3
			|| poolResult == ShrineEffectsPools::SONG_3 )
		{
			result.second = 3;
		}
		else if ( poolResult == ShrineEffectsPools::SONG_4 )
		{
			result.second = 4;
		}
		else if ( poolResult == ShrineEffectsPools::SONG_5 )
		{
			result.second = 5;
		}

		if ( resultType == SHRINE_RESULT_OUTCOME )
		{
			// increase outcomes
			if ( negativeActionModifier < 0 )
			{
				if ( result.second <= 3 )
				{
					if ( negativeActionModifier <= -40 )
					{
						result.second = std::min(5, result.second + 2);
					}
					else if ( negativeActionModifier <= -30 )
					{
						result.second = std::min(4, result.second + 2);
					}
					else if ( negativeActionModifier <= -20 )
					{
						result.second = std::min(3, result.second + 2);
					}
					else if ( negativeActionModifier <= -10 )
					{
						result.second = std::min(3, result.second + 1);
					}
				}
			}
		}

		return result;
	}

	return result;
}

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

	for ( auto target : targets )
	{
		bool anyEffect = false;
		for ( auto eff : instrumentsPlaying )
		{
			Uint8 tier = eff.second;
			if ( tier ) // tier
			{
				if ( target )
				{
					if ( Stat* targetStats = target->getStats() )
					{
						int effectID = eff.first;
						int dur = targetStats->getEffectActive(effectID) ? targetStats->EFFECTS_TIMERS[effectID] : 0;
						Uint8 tierStrength = Stat::kEnsembleBreakPointTier1 + tier * 5 + 1;
						Uint8 effectStrength = std::max(tierStrength, targetStats->getEffectActive(effectID));
						if ( target->setEffect(effectID, effectStrength, std::max(dur, duration), false) )
						{
							if ( !anyEffect )
							{
								if ( target->behavior == &actPlayer )
								{
									players[target->skill[2]]->mechanics.eternalShrineEnsemble = true;
									playSoundEntity(target, 168, 64);
								}
								createEnsembleTargetParticleCircling(target);
								serverSpawnMiscParticles(target, PARTICLE_EFFECT_ENSEMBLE_OTHER_CAST, 0);
							}
							anyEffect = true;
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
		else if ( parent->eternalShrineType == GUI_TYPE_ETERNALSHRINE_MUSIC )
		{
			my->x += 6.0 * cos(parent->yaw);
			my->y += 6.0 * sin(parent->yaw);
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
			//playSoundEntityLocal(parent, 883, 92);
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

				Entity* target = uidToEntity(parent->eternalShrineTarget);

				std::string tierString = ShrineEffects_t::getTierStringFromEffect(
					achievementObserver.checkUidIsFromPlayer(parent->eternalShrineTarget), 
					*parent, 
					parent->entity_rng ? *parent->entity_rng : local_rng, item);

				if ( processShrineLockoutOnEffect(parent, target, tierString, false) )
				{
					if ( item->node )
					{
						list_RemoveNode(item->node);
					}
					else
					{
						free(item);
					}
					createParticleFociLight(parent, SPELL_NONE, true);
				}
				else
				{
					auto result = ShrineEffects_t::rollResult(parent->eternalShrineType, ShrineEffects_t::SHRINE_RESULT_OUTCOME, 
						achievementObserver.checkUidIsFromPlayer(parent->eternalShrineTarget),
						tierString, parent->entity_rng ? *parent->entity_rng : local_rng, item);

					if ( item->count > 0 )
					{
						int divineFavorCost = 1;
						if ( tierString != "" )
						{
							divineFavorCost += std::stoi(tierString.substr(0, 1));
						}
						if ( result.first != "" )
						{
							divineFavorCost = std::max(0, divineFavorCost - result.second);
						}

						int blessDiff = item->beatitude - parent->eternalShrineItemBeatitude;
						int repairDiff = (int)item->status - parent->eternalShrineItemStatus;

						bool transformed = (int)item->type != parent->eternalShrineItemType;
						if ( transformed )
						{
							if ( target && target->behavior == &actPlayer )
							{
								messagePlayerColor(target->skill[2], MESSAGE_WORLD, makeColorRGB(255, 255, 0), Language::get(7186));
							}
						}

						if ( blessDiff > 0 )
						{
							if ( !transformed && target && target->behavior == &actPlayer )
							{
								messagePlayerColor(target->skill[2], MESSAGE_WORLD, makeColorRGB(255, 255, 0), Language::get(7146));
							}
							divineFavorCost += blessDiff;
						}
						else if ( repairDiff > 0  || item->appearance != (Uint32)parent->eternalShrineItemAppearance )
						{
							if ( !transformed && target && target->behavior == &actPlayer )
							{
								messagePlayerColor(target->skill[2], MESSAGE_WORLD, makeColorRGB(255, 255, 0), Language::get(7145));
							}
							divineFavorCost += repairDiff / 3;
						}

						if ( item->type == ARTIFACT_AXE
							|| item->type == ARTIFACT_SWORD
							|| item->type == ARTIFACT_MACE
							|| item->type == ARTIFACT_SPEAR
							|| item->type == ARTIFACT_AXE
							|| item->type == ARTIFACT_BOW
							|| item->type == ARTIFACT_BREASTPIECE
							|| item->type == ARTIFACT_HELM
							|| item->type == ARTIFACT_BOOTS
							|| item->type == ARTIFACT_CLOAK
							|| item->type == ARTIFACT_GLOVES
							|| item->type == MASK_ARTIFACT_VISOR )
						{
							divineFavorCost *= 2;
						}

						if ( achievementObserver.checkUidIsFromPlayer(parent->eternalShrineTarget) >= 0 )
						{
							players[achievementObserver.checkUidIsFromPlayer(parent->eternalShrineTarget)]->mechanics.divineFavorModPips(-divineFavorCost);
						}
					}

					bool adorcise = result.first == "ADORCISED";
					if ( item->count == 0 )
					{
						if ( target && target->behavior == &actPlayer )
						{
							int favorTotal = 0;
							for ( int i = 0; i < parent->eternalShrineItemCount; ++i )
							{
								int divineFavor = players[target->skill[2]]->mechanics.getDivineFavorFromItem(item, 1);
								if ( divineFavor >= 0 )
								{
									favorTotal += divineFavor;
									players[target->skill[2]]->mechanics.divineFavorModItem(divineFavor);
								}
							}

							if ( favorTotal >= 2000 )
							{
								ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_WARNING,
									parent->getUID(), target->skill[2], Language::get(7185), 0);
							}
							else
							{
								ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_WARNING,
									parent->getUID(), target->skill[2], Language::get(7144), 0);
							}
						}

						if ( item->node )
						{
							list_RemoveNode(item->node);
						}
						else
						{
							free(item);
						}
						createParticleFociLight(parent, SPELL_NONE, true);
					}
					else if ( adorcise && spellEffectAdorcise(*parent, spellElementMap[SPELL_ADORCISM], parent->x, parent->y, item) )
					{
						if ( item->node )
						{
							list_RemoveNode(item->node);
						}
						else
						{
							free(item);
						}

						if ( target && target->behavior == &actPlayer )
						{
							//int tickDelay = 0;
							//if ( target->getStats() && target->getStats()->EFFECTS_TIMERS[EFF_STASIS] )
							//{
							//	tickDelay = std::max(0, target->getStats()->EFFECTS_TIMERS[EFF_STASIS] - 3 * TICKS_PER_SECOND);
							//}
							//ShrinePlayerMessageManager_t::insert(parent->getUID(), target->skill[2], Language::get(7130), "", std::make_pair("", 0), tickDelay);
							messagePlayerColor(target->skill[2], MESSAGE_WORLD, makeColorRGB(255, 255, 0), Language::get(7187));
						}
					}
					else if ( Entity* dropped = dropItemMonster(item, parent, nullptr, item->count) )
					{
						dropped->x = parent->x;
						dropped->y = parent->y;
						dropped->z = -4.0;
						dropped->vel_z *= 0.5;
						dropped->x += 1.0 * cos(parent->yaw) - 1.0 * cos(parent->yaw + PI / 2);
						dropped->y += 1.0 * sin(parent->yaw) - 1.0 * sin(parent->yaw + PI / 2);
						dropped->itemEternalShrineResult = parent->eternalShrineType;

						bool effect = applyShrineEffect(result.first, target, parent, result.second);
						if ( effect && target && target->behavior == &actPlayer )
						{
							int tickDelay = 0;
							if ( target->getStats() && target->getStats()->EFFECTS_TIMERS[EFF_STASIS] )
							{
								tickDelay = std::max(0, target->getStats()->EFFECTS_TIMERS[EFF_STASIS] - 3 * TICKS_PER_SECOND);
							}
							ShrinePlayerMessageManager_t::insert(parent->getUID(), target->skill[2], Language::get(7130), "", std::make_pair("", 0), tickDelay);
						}
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

						bool effect = applyShrineEffect(result.first, target, parent, result.second);
						if ( effect && target && target->behavior == &actPlayer )
						{
							int tickDelay = 0;
							if ( target->getStats() && target->getStats()->EFFECTS_TIMERS[EFF_STASIS] )
							{
								tickDelay = std::max(0, target->getStats()->EFFECTS_TIMERS[EFF_STASIS] - 3 * TICKS_PER_SECOND);
							}
							ShrinePlayerMessageManager_t::insert(parent->getUID(), target->skill[2], Language::get(7130), "", std::make_pair("", 0), tickDelay);
						}
					}

					if ( target )
					{
						Uint32 newvalue = GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_COMPLETED << (target->skill[2] * 2);
						Uint32 mask = (0b11) << (target->skill[2] * 2);
						parent->eternalShrinePlayerLockout &= ~(mask); // zero out the player slot
						parent->eternalShrinePlayerLockout |= newvalue; // apply new value
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
			real_t prevRise = animRise;
			animRise += std::max((1.0 - animRise), 0.01) / 10.0;
			animRise = std::min(1.0, animRise);

			if ( prevRise < 0.05 && animRise > 0.05 )
			{
				playSoundEntityLocal(my, 896, 128);
			}
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

	static ConsoleVariable<std::string> cvar_eternal_shrine_effect("/eternal_shrine_effect", "");
	if ( *cvar_eternal_shrine_effect != "" )
	{
		uppercaseString(*cvar_eternal_shrine_effect);
		int tier = 1 + local_rng.rand() % 3;
		auto find = (*cvar_eternal_shrine_effect).find(' ');
		if ( find != std::string::npos )
		{
			char c = (*cvar_eternal_shrine_effect).at(find + 1);
			if ( c >= '1' && c <= '5' )
			{
				tier = 1 + (c - '1');
			}
			(*cvar_eternal_shrine_effect) = (*cvar_eternal_shrine_effect).substr(0, find);
		}


		if ( multiplayer != CLIENT )
		{
			if ( svFlags & SV_FLAG_CHEATS )
			{
				for ( auto& str : ShrineEffects_t::shrineEffects )
				{
					if ( strstr(str.c_str(), (*cvar_eternal_shrine_effect).c_str()) )
					{
						bool result = applyShrineEffect(str, players[0]->entity, my, tier);
						messagePlayer(clientnum, MESSAGE_MISC, "Tier: %d", tier);
						/*if ( result )
						{
							ShrinePlayerMessageManager_t::insert(my->getUID(), 0, Language::get(7130), "", std::make_pair("", 0), 0);
						}*/
						break;
					}
				}
			}
		}
		*cvar_eternal_shrine_effect = "";
	}

	ShrinePlayerMessageManager_t::update(my->getUID());

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
	auto& interactTimer = my->fskill[2];
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
				std::string tierString = ShrineEffects_t::getTierStringFromEffect(
					achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget),
					*my,
					my->entity_rng ? *my->entity_rng : local_rng);
				Entity* target = uidToEntity(my->eternalShrineTarget);
				if ( processShrineLockoutOnEffect(my, target, tierString, false) )
				{

				}
				else
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

					if ( target )
					{
						Uint32 newvalue = GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_COMPLETED << (target->skill[2] * 2);
						Uint32 mask = (0b11) << (target->skill[2] * 2);
						my->eternalShrinePlayerLockout &= ~(mask); // zero out the player slot
						my->eternalShrinePlayerLockout |= newvalue; // apply new value
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

				Entity* target = uidToEntity(my->eternalShrineTarget);

				std::string tierString = ShrineEffects_t::getTierStringFromEffect(
					achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget),
					*my,
					my->entity_rng ? *my->entity_rng : local_rng, item);

				if ( processShrineLockoutOnEffect(my, target, tierString, false) )
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
				else
				{
					auto resultOutcome = ShrineEffects_t::rollResult(my->eternalShrineType, ShrineEffects_t::SHRINE_RESULT_OUTCOME, 
						achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget),
						tierString, my->entity_rng ? *my->entity_rng : local_rng);
					bool effect = false;
					if ( resultOutcome.first != "" )
					{
						effect = applyShrineEffect(resultOutcome.first, target, my, resultOutcome.second);
					}

					if ( !effect )
					{
						playSoundEntity(my, 883, 92);
					}
					else
					{
						//playSoundEntity(my, 884, 92);
						playSoundEntity(my, 883, 92);

						if ( target )
						{
							int tickDelay = 0;
							if ( target->getStats() && target->getStats()->EFFECTS_TIMERS[EFF_STASIS] )
							{
								tickDelay = std::max(0, target->getStats()->EFFECTS_TIMERS[EFF_STASIS] - 3 * TICKS_PER_SECOND);
							}
							ShrinePlayerMessageManager_t::insert(my->getUID(), target->skill[2], Language::get(7130), "", std::make_pair("", 0), tickDelay);
						}
					}

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

						if ( items[item->type].category == TOME_SPELL )
						{
							int spellID = item->getTomeSpellID();
							if ( spellID != SPELL_NONE )
							{
								if ( auto spell = getSpellFromID(spellID) )
								{
									int divineFavorCost = 1 + (spell->difficulty) / 5;
									if ( resultOutcome.first != "" && effect )
									{
										divineFavorCost = std::max(0, divineFavorCost - resultOutcome.second);
									}
									if ( achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget) >= 0 )
									{
										players[achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget)]->mechanics.divineFavorModPips(-divineFavorCost);
									}
								}
							}
						}

						serverUpdateEntitySkill(my, 7);
						if ( my->eternalShrineItemCount <= 0 )
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
						else if ( Entity* dropped = dropItemMonster(item, my, nullptr, item->count) )
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

					if ( target )
					{
						Uint32 newvalue = GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_COMPLETED << (target->skill[2] * 2);
						Uint32 mask = (0b11) << (target->skill[2] * 2);
						my->eternalShrinePlayerLockout &= ~(mask); // zero out the player slot
						my->eternalShrinePlayerLockout |= newvalue; // apply new value
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

		int shrineCurrentView = (int)(my->eternalShrineOfferingItemTypeModel & 0xF);
		if ( (interacting
			&& (shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_ACTION
				|| shrineCurrentView == GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING))
			|| my->eternalShrineOfferingItemVisible >= GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING )
		{
			if ( interactTimer <= 1.0 )
			{
				interactTimer = 2.0;
				playSoundEntityLocal(my, 777, 128);
			}
		}
		else
		{
			interactTimer = 0.0;
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

				if ( multiplayer != CLIENT )
				{
					std::string tierString = ShrineEffects_t::getTierStringFromEffect(
						achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget),
						*my,
						my->entity_rng ? *my->entity_rng : local_rng);

					Entity* target = uidToEntity(my->eternalShrineTarget);
					if ( target )
					{
						if ( processShrineLockoutOnEffect(my, target, tierString, false) )
						{
							
						}
						else
						{
							auto resultOutcome = ShrineEffects_t::rollResult(my->eternalShrineType, ShrineEffects_t::SHRINE_RESULT_OUTCOME, 
								achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget),
								tierString, my->entity_rng ? *my->entity_rng : local_rng);
							bool effect = false;
							if ( resultOutcome.first != "" )
							{
								effect = applyShrineEffect(resultOutcome.first, target, my, resultOutcome.second);
							}

							auto resultReward = ShrineEffects_t::rollResult(my->eternalShrineType, ShrineEffects_t::SHRINE_RESULT_REWARD, 
								achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget),
								tierString, my->entity_rng ? *my->entity_rng : local_rng);
							if ( !effect )
							{
								playSoundEntity(my, 914, 156);

								bool supplication = applySupplicationEffect(tierString, target, my);
								bool effect = applyShrineEffect(resultReward.first, target, my, resultReward.second);
								if ( supplication || effect )
								{
									spawnHeatOrbitSpin(target, 263, false);
									playSoundEntity(target, 166, 128);
									playSoundEntity(target, 827, 128);
								}
							}
							else
							{
								playSoundEntity(my, 915, 156);
								int tickDelay = 2 * TICKS_PER_SECOND;
								if ( target->getStats() && target->getStats()->EFFECTS_TIMERS[EFF_STASIS] )
								{
									tickDelay = std::max(tickDelay, target->getStats()->EFFECTS_TIMERS[EFF_STASIS] + 25);
								}
								ShrinePlayerMessageManager_t::insert(my->getUID(), target->skill[2],
									nullptr, tierString, resultReward, tickDelay);
								ShrinePlayerMessageManager_t::insert(my->getUID(), target->skill[2],
									Language::get(7143), "", resultOutcome, tickDelay);

								playSoundEntity(target, 827, 128);
								spawnHeatOrbitSpin(target, 288, true);
							}

							int divineFavorCost = 1;
							if ( tierString != "" )
							{
								divineFavorCost += std::stoi(tierString.substr(0, 1));
							}
							if ( resultReward.first != "" )
							{
								divineFavorCost = std::max(0, divineFavorCost + resultReward.second);
								divineFavorCost = std::min(5, divineFavorCost);
							}
							if ( resultOutcome.first != "" && effect )
							{
								divineFavorCost = std::max(0, divineFavorCost - resultOutcome.second);
							}
							if ( achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget) >= 0 )
							{
								players[achievementObserver.checkUidIsFromPlayer(my->eternalShrineTarget)]->mechanics.divineFavorModPips(-divineFavorCost);
							}

							if ( target )
							{
								Uint32 newvalue = GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_COMPLETED << (target->skill[2] * 2);
								Uint32 mask = (0b11) << (target->skill[2] * 2);
								my->eternalShrinePlayerLockout &= ~(mask); // zero out the player slot
								my->eternalShrinePlayerLockout |= newvalue; // apply new value
							}

							/*for ( int i = 0; i < 4; ++i )
							{
								if ( Entity* fx = createParticleAestheticOrbit(target, 288, 25, PARTICLE_EFFECT_IGNITE_ORBIT) )
								{
									fx->x = target->x;
									fx->y = target->y;
									fx->fskill[0] = fx->x;
									fx->fskill[1] = fx->y;
									fx->flags[SPRITE] = true;
									fx->fskill[2] = (i / 4.0) * 2 * PI + PI / 4;
									fx->fskill[3] += (local_rng.rand() % 10) * PI / 10.0;
									fx->z = 7.5;
									fx->vel_z = -0.4 + (local_rng.rand() % 10) * -.025;
									fx->actmagicOrbitDist = 4;
									fx->actmagicNoLight = 0;
								}
							}*/
						}
					}
				}
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
				if ( processShrineLockoutOnEffect(my, players[i]->entity, "", true) )
				{
					// do nothing
				}
				else if ( my->eternalShrineInteracting != 0 || my->eternalShrineState >= GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE )
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
							SDLNet_Write16(players[i]->mechanics.getDivineFavorBase(), &net_packet->data[8]);
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

	int prevPips = players[player]->mechanics.getDivineFavorPips();
	int divineFavor = players[player]->mechanics.getDivineFavorFromItem(item, 1);
	if ( divineFavor >= 0 )
	{
		players[player]->mechanics.divineFavorModItem(divineFavor);
	}
	int newPips = players[player]->mechanics.getDivineFavorPips();
	bool poorOffering = divineFavor <= 5 || item->status == BROKEN || item->beatitude < 0;

	if ( shrine )
	{
		int playerProgress = (shrine->eternalShrinePlayerStates >> (player * 2)) & 0b11;
		if ( !players[player]->isLocalPlayer() )
		{
			playerProgress = GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_CONFIRMED;
			if ( poorOffering || players[player]->mechanics.getDivineFavorPips() == 0 )
			{
				Uint32 newvalue = GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_OFFERING << (player * 2);
				Uint32 mask = (0b11) << (player * 2);
				shrine->eternalShrinePlayerLockout &= ~(mask); // zero out the player slot
				shrine->eternalShrinePlayerLockout |= newvalue; // apply new value
			}
		}
		else if ( playerProgress == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_WAITING
			|| playerProgress == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE )
		{
			playerProgress = GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_CONFIRMED;
			if ( poorOffering || players[player]->mechanics.getDivineFavorPips() == 0 )
			{
				Uint32 newvalue = GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_OFFERING << (player * 2);
				Uint32 mask = (0b11) << (player * 2);
				shrine->eternalShrinePlayerLockout &= ~(mask); // zero out the player slot
				shrine->eternalShrinePlayerLockout |= newvalue; // apply new value
			}
		}

		if ( poorOffering )
		{
			players[player]->mechanics.updateDivineEvent(shrine, 
				Player::PlayerMechanics_t::DivineEvent::DIVINE_POOR_OFFERINGS);
		}
		else
		{
			players[player]->mechanics.updateDivineEvent(shrine,
				Player::PlayerMechanics_t::DivineEvent::DIVINE_OFFERINGS);
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
			SDLNet_Write16(players[player]->mechanics.getDivineFavorBase(), &net_packet->data[9]);
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

	int numChances = 0;
	for ( auto chance : chances )
	{
		if ( chance )
		{
			++numChances;
		}
	}

	std::string tierString = ShrineEffects_t::getTierStringFromEffect(
		player,
		*shrine,
		shrine->entity_rng ? *shrine->entity_rng : local_rng);
	auto resultReward = ShrineEffects_t::rollResult(shrine->eternalShrineType, ShrineEffects_t::SHRINE_RESULT_REWARD,
		player,
		tierString, shrine->entity_rng ? *shrine->entity_rng : local_rng);

	shrine->eternalShrineOrchestrionInstruments = 0;

	auto& rng = shrine->entity_rng ? *shrine->entity_rng : local_rng;

	bool receiveItem = false;
	bool applyMusic = false;

	auto lockoutStatus = (GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus)
		(shrine->eternalShrinePlayerLockout >> (player * 2) & 0b11);
	if ( lockoutStatus > GenericGUIMenu::EternalShrineGUI_t::EternalShrineLockoutStatus::LOCKOUT_NONE )
	{
		resultReward.first = "";
	}

	// calculate reward
	if ( resultReward.first != "" )
	{
		int numSongs = 0;
		if ( resultReward.first.find("ENSEMBLE_1") != std::string::npos )
		{
			numSongs = 1;
			// no solo drum/horn
			if ( chances[DRUM] && (chances[FLUTE] || chances[LUTE] || chances[LYRE]) )
			{
				chances[DRUM] = 0;
				--numChances;
			}
			if ( chances[HORN] && (chances[FLUTE] || chances[LUTE] || chances[LYRE]) )
			{
				chances[HORN] = 0;
				--numChances;
			}
		}
		else if ( resultReward.first.find("ENSEMBLE_2") != std::string::npos )
		{
			numSongs = 2;
		}
		else if ( resultReward.first.find("ENSEMBLE_3") != std::string::npos )
		{
			numSongs = 3;
		}
		else if ( resultReward.first.find("ENSEMBLE_4") != std::string::npos )
		{
			numSongs = 4;
		}
		else if ( resultReward.first.find("ENSEMBLE_5") != std::string::npos )
		{
			numSongs = 5;
		}
		int tier = resultReward.second;
		std::vector<InstrumentOrder> newSongs;
		while ( numSongs > 0 )
		{
			if ( numChances > 0 )
			{
				int pick = rng.discrete(chances.data(), chances.size());
				chances[pick] = 0;
				instrumentsPlaying[pick].second = std::max(tier, instrumentsPlaying[pick].second);
				applyMusic = true;
				newSongs.push_back((InstrumentOrder)pick);
				--numChances;

			}
			--numSongs;
		}

		int messageDelay = 0;
		if ( newSongs.size() == 1 )
		{
			char buf[64];
			snprintf(buf, sizeof(buf), Language::get(7141), items[INSTRUMENT_FLUTE + (int)newSongs[0]].getIdentifiedName());
			ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_GENERAL, shrine->getUID(), player, buf, messageDelay);
		}
		else if ( newSongs.size() == 2 )
		{
			ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_GENERAL, shrine->getUID(), player, Language::get(7137), messageDelay);
		}
		else if ( newSongs.size() == 3 )
		{
			ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_GENERAL, shrine->getUID(), player, Language::get(7138), messageDelay);
		}
		else if ( newSongs.size() == 4 )
		{
			ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_GENERAL, shrine->getUID(), player, Language::get(7139), messageDelay);
		}
		else if ( newSongs.size() >= 5 )
		{
			ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_GENERAL, shrine->getUID(), player, Language::get(7140), messageDelay);
		}

		int divineFavorCost = 1 + std::max(tier, (int)newSongs.size()) / 2;

		if ( resultReward.first.find("INSTRUMENT") != std::string::npos )
		{
			receiveItem = true;
			shrine->eternalShrineItemType = INSTRUMENT_FLUTE + rng.rand() % 5;
			shrine->eternalShrineItemStatus = std::max((int)BROKEN, std::min((int)EXCELLENT, tier - 1));
			shrine->eternalShrineItemBeatitude = 0;
			shrine->eternalShrineItemCount = 1;
			shrine->eternalShrineItemAppearance = local_rng.rand();
			shrine->eternalShrineItemIdentified = 0;

			ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_WARNING, shrine->getUID(), player, Language::get(7142), 2 * TICKS_PER_SECOND);

			++divineFavorCost;
		}
		else if ( resultReward.first.find("SCROLL") != std::string::npos )
		{
			receiveItem = true;

			std::vector<ItemType> pool = { SCROLL_LITURGY, SCROLL_MINSTRELS, SCROLL_STAMINA, SCROLL_MENTALITY, SCROLL_AGILITY };
			int pick = rng.rand() % pool.size();
			shrine->eternalShrineItemType = pool[pick];
			shrine->eternalShrineItemStatus = EXCELLENT;
			shrine->eternalShrineItemBeatitude = std::max(0, std::min(2, ((tier - 1) / 2)));
			shrine->eternalShrineItemCount = 1 + std::max(0, std::min(2, (tier / 2)));
			shrine->eternalShrineItemAppearance = local_rng.rand();
			shrine->eternalShrineItemIdentified = 0;

			ShrinePlayerMessageManager_t::insert(ShrinePlayerMessageManager_t::SHRINE_MESSAGE_WARNING, shrine->getUID(), player, Language::get(7142), 2 * TICKS_PER_SECOND);

			++divineFavorCost;
		}

		players[player]->mechanics.divineFavorModPips(-divineFavorCost);
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

	if ( applyMusic )
	{
		if ( shrine->eternalShrineOrchestrionInstruments != 0 )
		{
			shrine->eternalShrineOrchestrionInstruments |= (1 << 31); // signal to reapply music
		}
		int duration = 5 * 60 * TICKS_PER_SECOND;
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

	shrine->eternalShrineItemType = 0;
	shrine->eternalShrineItemAppearance = 0;
	shrine->eternalShrineItemStatus = DECREPIT;
	shrine->eternalShrineItemBeatitude = 0;
	shrine->eternalShrineItemCount = 1;
	shrine->eternalShrineItemIdentified = false;

	if ( error )
	{
		if ( shrine )
		{
			//shrine->eternalShrineState = 0;
			//serverUpdateEntitySkill(shrine, 4); // eternalShrineState
			//shrine->eternalShrineOfferingItemTypeModel = 0;
			//serverUpdateEntitySkill(shrine, 16);

			shrine->eternalShrineItemCount = 0;

			shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE;
			shrine->eternalShrineTarget = achievementObserver.playerUids[player];
			serverUpdateEntitySkill(shrine, 4); // eternalShrineState
			shrine->eternalShrineOfferingItemTypeModel = 0;
			shrine->eternalShrineOfferingItemTypeModel |= ((GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING) & 0xF);
			serverUpdateEntitySkill(shrine, 16);
		}
		return false;
	}


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
		struct Chance
		{
			int skillID = -1;
			int spellID = -1;
			int difficulty = -1;
			Chance(int _skill, int _spell, int _diff)
			{
				skillID = _skill;
				spellID = _spell;
				difficulty = _diff;
			}
		};
		std::vector<Chance> chances;
		std::vector<unsigned int> chance_weights;
		int maxDifficulty = std::max(0, (((players[player]->mechanics.getDivineFavorPips() + 1) / 2) - 1) * 20);
		if ( players[player]->mechanics.getDivineFavorPips() >= Player::DIVINE_FAVOR_PIPS_MAX )
		{
			maxDifficulty = 100;
		}
		int minDifficulty = 0;

		int skillLVL = stats[player]->getModifiedProficiency(skillID) + statGetINT(stats[player], players[player]->entity);
		int highestDifficulty = 0;
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
						&& spell->skillID == skillID
						&& skillLVL >= spell->difficulty )
					{
						chances.emplace_back(Chance(spell->skillID, spell->ID, spell->difficulty));
						chance_weights.push_back(1 + spell->difficulty / 5);
						highestDifficulty = std::max(spell->difficulty, highestDifficulty);
					}
				}
			}
		}

		bool anychances = false;
		for ( int i = 0; i < chances.size(); ++i )
		{
			if ( std::min(80, chances[i].difficulty) == std::min(80, highestDifficulty) )
			{
				anychances = true;
			}
			else
			{
				chance_weights[i] = 0;
			}
		}

		Uint32 appearance = 0;
		if ( anychances && chances.size() && chances.size() == chance_weights.size() )
		{
			auto& rng = shrine->entity_rng ? *shrine->entity_rng : local_rng;
			int pick = rng.discrete(chance_weights.data(), chance_weights.size());
			{
				itemType = TOME_SORCERY;
				appearance = spellTomeIDToAppearance[chances[pick].spellID];
				if ( chances[pick].skillID == PRO_MYSTICISM )
				{
					itemType = TOME_MYSTICISM;
				}
				else if ( chances[pick].skillID == PRO_THAUMATURGY )
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
		shrine->eternalShrineItemCount = 0;

		shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_ACTIVE;
		shrine->eternalShrineTarget = achievementObserver.playerUids[player];
		serverUpdateEntitySkill(shrine, 4); // eternalShrineState
		shrine->eternalShrineOfferingItemTypeModel = 0;
		shrine->eternalShrineOfferingItemTypeModel |= ((GenericGUIMenu::EternalShrineGUI_t::ASSIST_SHRINE_VIEW_WAITING) & 0xF);
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

int getSupplicationHungerScore(const int player)
{
	if ( player < 0 || player >= MAXPLAYERS ) { return 0; }
	if ( players[player]->isLocalPlayer() )
	{
		int total = 0;
		bool tinopener = false;
		for ( node_t* node = stats[player]->inventory.first; node; node = node->next )
		{
			if ( Item* item = (Item*)node->element )
			{
				if ( item->type == TOOL_TINOPENER )
				{
					tinopener = true;
					break;
				}
			}
		}


		for ( node_t* node = stats[player]->inventory.first; node; node = node->next )
		{
			if ( Item* item = (Item*)node->element )
			{
				if ( stats[player]->type == AUTOMATON )
				{
					if ( itemIsConsumableByAutomaton(*item) )
					{
						total += Item::getAutomatonFoodSatiation(item->type) * item->count;
					}
					continue;
				}
				if ( int foodVal = Item::getBaseFoodSatiation(item->type) )
				{
					if ( item->type == FOOD_TIN )
					{
						if ( !tinopener && stats[player]->type != GOATMAN )
						{
							continue;
						}
					}
					total += foodVal * item->count;
				}
				else if ( item->type == FOOD_BLOOD )
				{
					if ( playerRequiresBloodToSustain(player) )
					{
						total += 250 * item->count;
					}
				}
				else if ( item->type == SCROLL_FOOD )
				{
					total += 1800 * item->count;
				}
			}
		}
		return total;
	}

	return 0;
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
		int hungerScore = getSupplicationHungerScore(player);
		SDLNet_Write32(hungerScore, &net_packet->data[10]);

		net_packet->address.host = net_server.host;
		net_packet->address.port = net_server.port;
		net_packet->len = 14;
		sendPacketSafe(net_sock, -1, net_packet, 0);

		shrine->eternalShrineState = GenericGUIMenu::EternalShrineGUI_t::ETERNAL_SHRINE_STATE_CLIENT_WAITING_RESULT;
	}
	else
	{
		players[player]->mechanics.client_hunger_score = getSupplicationHungerScore(player);
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

	if ( player.inventory_mode == INVENTORY_MODE_SPELL )
	{
		player.inventoryUI.cycleInventoryTab();
	}
	else
	{
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
	pipsTotal = players[playernum]->mechanics.getDivineFavorPips();

	if ( inputs.getUIInteraction(playernum)->selectedItem )
	{
		inputs.getUIInteraction(playernum)->selectedItem = nullptr;
		inputs.getUIInteraction(playernum)->toggleclick = false;
	}
	inputs.getUIInteraction(playernum)->selectedItemFromChest = 0;
	clearItemDisplayed();
}

#ifdef USE_FMOD
std::vector<FMOD::Channel*> buttonSound(MAXPLAYERS, nullptr);
#endif

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

	if ( !bOpen )
	{
		holdButtonProcessedOnTick = 0;
#ifdef USE_FMOD
		bool isplaying = false;
		buttonSound[playernum]->isPlaying(&isplaying);
		if ( isplaying )
		{
			buttonSound[playernum]->stop();
		}
#endif
	}

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

	if ( bOpen )
	{
		if ( player->mechanics.getDivineFavorPips() < pipsTotal )
		{
			pipsTotal = player->mechanics.getDivineFavorPips();
		}
	}

	bool viewActionReady = false;
	if ( currentView == ASSIST_SHRINE_VIEW_OFFERING || !bOpen )
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

		if ( player->mechanics.getDivineFavorPips() > pipsTotal )
		{
			int diff = player->mechanics.getDivineFavorPips() - pipsTotal;
			pipsTotal = player->mechanics.getDivineFavorPips();
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
		//if ( keystatus[SDLK_g] )
		//{
		//	keystatus[SDLK_g] = 0;
		//	if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
		//	{
		//		currentView = ASSIST_SHRINE_VIEW_ACTION;
		//		submittedItem = EternalShrineSubmitStatus::SUBMIT_DONE;
		//	}
		//	else if ( currentView == ASSIST_SHRINE_VIEW_ACTION )
		//	{
		//		currentView = ASSIST_SHRINE_VIEW_WAITING;
		//		submittedItem = EternalShrineSubmitStatus::SUBMIT_NONE;
		//	}
		//	else
		//	{
		//		currentView = ASSIST_SHRINE_VIEW_OFFERING;
		//		submittedItem = EternalShrineSubmitStatus::SUBMIT_NONE;
		//	}

		//	Uint32 newvalue = submittedItem << (playernum * 2);
		//	Uint32 mask = (0b11) << (playernum * 2);
		//	if ( eternalShrineStation )
		//	{
		//		eternalShrineStation->eternalShrinePlayerStates &= ~(mask); // zero out the player slot
		//		eternalShrineStation->eternalShrinePlayerStates |= newvalue; // apply new value
		//		if ( multiplayer == SERVER )
		//		{
		//			serverUpdateEntitySkill(eternalShrineStation, 17);
		//		}
		//	}
		//}

		//if ( keystatus[SDLK_h] )
		//{
		//	keystatus[SDLK_h] = 0;
		//	if ( keystatus[SDLK_LSHIFT] )
		//	{
		//		//pipsTotal -= 1;
		//		//pipsTotal = std::max(0, pipsTotal);
		//		player->mechanics.divine_favor -= 1;
		//		player->mechanics.divine_favor = std::max(0, player->mechanics.divine_favor);
		//	}
		//	else
		//	{
		//		player->mechanics.divine_favor += 1;
		//		if ( player->mechanics.divine_favor > 10 )
		//		{
		//			player->mechanics.divine_favor = 0;
		//		}
		//		/*pipsTotal += 1;
		//		if ( pipsTotal > 10 )
		//		{
		//			pipsTotal = 0;
		//		}*/
		//	}
		//}
		/*if ( keystatus[SDLK_j] )
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
		}*/

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
			if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ANVIL )
			{
				title->setText(Language::get(7177));
			}
			else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
			{
				title->setText(Language::get(7179));
			}
			else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_MUSIC )
			{
				title->setText(Language::get(7178));
			}
			else if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
			{
				title->setText(Language::get(7176));
			}
		}
		else
		{
			title->setText("");
		}

		SDL_Rect textPos{ 0, 23, baseFrame->getSize().w, 24 };
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
		if ( closeBtn->isDisabled() && usingGamepad && sendItem1Uid == 0 && isInteractable )
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
		if ( usingGamepad )
		{
			if ( ascensionType == ASCENSION_SPELL )
			{
				actionBtn->setText(Language::get(7183));
			}
			else
			{
				actionBtn->setText(Language::get(7182));
			}
		}
		else
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
	}
	else if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ANVIL )
	{
		if ( usingGamepad )
		{
			actionBtn->setText(Language::get(7184));
		}
		else
		{
			actionBtn->setText(Language::get(7001));
		}
	}
	else if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_SUPPLICATION )
	{
		if ( usingGamepad )
		{
			actionBtn->setText(Language::get(7180));
		}
		else
		{
			actionBtn->setText(Language::get(7083));
		}
	}
	else if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_MUSIC )
	{
		if ( usingGamepad )
		{
			actionBtn->setText(Language::get(7181));
		}
		else
		{
			actionBtn->setText(Language::get(7084));
		}
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
			pos.w = 82;
			pos.x = baseFramePos.w / 2 - pos.w / 2;
			static ConsoleVariable<int> cvar_eternal_shrine_action_height("/eternal_shrine_action_height", -28);
			pos.y = 260;
			if ( usingGamepad )
			{
				pos.y += *cvar_eternal_shrine_action_height;
			}
			pos.h = 26;
			actionBtn->setSize(pos);

			SDL_Color color;
			getColor(actionBtn->getColor(), &color.r, &color.g, &color.b, &color.a);

			color.a = (Uint8)(255 * alphaRatio);

			actionBtn->setBackground("*#images/ui/Shrines/Button_TakeAll_00.png");
			actionBtn->setBackgroundHighlighted("*#images/ui/Shrines/Button_TakeAllHigh_00.png");
			actionBtn->setBackgroundActivated("*#images/ui/Shrines/Button_TakeAllPress_00.png");

			actionBtn->setColor(makeColor(color.r, color.g, color.b, color.a));
			actionBtn->setHighlightColor(makeColor(255, 255, 255, color.a));
			actionBtn->setTextColor(actionBtn->getColor());
			actionBtn->setTextHighlightColor(makeColor(201, 162, 100, 255 * alphaRatio));
			if ( usingGamepad && !inputs.getVirtualMouse(playernum)->draw_cursor )
			{
				pos.w = 108;
				pos.h = 46;
				pos.x = baseFramePos.w / 2 - pos.w / 2;
				actionBtn->setSize(pos);

				if ( !((sendItem1Uid != 0 || !bSendItemAllowed) && isInteractable) )
				{
					actionBtn->setColor(makeColor(color.r, color.g, color.b, 0));
					actionBtn->setHighlightColor(makeColor(255, 255, 255, 0));
					actionBtn->setTextColor(actionBtn->getColor());
					actionBtn->setTextHighlightColor(makeColor(201, 162, 100, 0));
				}

				actionBtn->setBackground("*#images/ui/Shrines/HUD_CharSheet_ButtonCompact_00.png");
				actionBtn->setBackgroundHighlighted("*#images/ui/Shrines/HUD_CharSheet_ButtonHighCompact_00.png");
				actionBtn->setBackgroundActivated("*#images/ui/Shrines/HUD_CharSheet_ButtonCompactPress_00.png");
			}


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
				if ( (sendItem1Uid != 0 || !bSendItemAllowed) && isInteractable )
				{
					actionBtn->setTextColor(makeColor(255, 255, 255, 255 * alphaRatio));
				}
				else
				{
					//SDL_Color color;
					//getColor(hudColors.characterSheetFaintText, &color.r, &color.g, &color.b, &color.a);
					//color.a = (Uint8)(255 * alphaRatio);
					//actionBtn->setTextColor(makeColor(color.r, color.g, color.b, color.a));
				}
				actionBtn->setDisabled(true);
				if ( (sendItem1Uid != 0 || !bSendItemAllowed) && isInteractable )
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

					static ConsoleVariable<int> cvar_eternal_shrine_action_glyphx("/eternal_shrine_action_glyphx", -82);
					static ConsoleVariable<int> cvar_eternal_shrine_action_glyphy("/eternal_shrine_action_glyphy", -20);
					offeringGlyph->pos.x += *cvar_eternal_shrine_action_glyphx;
					offeringGlyph->pos.y += *cvar_eternal_shrine_action_glyphy;
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
			static ConsoleVariable<int> cvar_eternal_shrine_offering_height("/eternal_shrine_offering_height", -12);
			pos.y = 250 + *cvar_eternal_shrine_offering_height;
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
				if ( (sendItem1Uid != 0) && isInteractable )
				{
					offeringBtn->setTextColor(makeColor(255, 255, 255, 255 * alphaRatio));
				}
				else
				{
					SDL_Color color;
					getColor(hudColors.characterSheetFaintText, &color.r, &color.g, &color.b, &color.a);
					color.a = (Uint8)(255 * alphaRatio);
					offeringBtn->setTextColor(makeColor(color.r, color.g, color.b, color.a));
				}

				offeringBtn->setDisabled(true);
				if ( (sendItem1Uid != 0) && isInteractable )
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
		if ( isInteractable && ((currentView == ASSIST_SHRINE_VIEW_ACTION && viewActionReady && bSendItemAllowed)
			|| submittedItem == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE) )
		{
			//const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
			//real_t setpointDiffX = fpsScale * std::max(.01, (1.0 - animTooltip)) / 2.0;
			//animTooltip += setpointDiffX;
			//animTooltip = std::min(1.0, animTooltip);
			animTooltip = 1.0;
			animTooltipTicks = ticks;
		}

		if ( player->bUseCompactGUIWidth() )
		{
			// the inventory tooltip provides the prompt before item is placed
			if ( !strcmp(activateSelectionPrompt->getText(), Language::get(4172)) )
			{
				animTooltip = 0.0;
			}
		}
		if ( currentView == ASSIST_SHRINE_VIEW_ACTION )
		{
			if ( parentGUI.guiType == GUI_TYPE_ETERNALSHRINE_ASCENSION )
			{
				if ( !(player->inventoryUI.spellPanel.bOpen && player->inventoryUI.spellPanel.isInteractable) )
				{
					animTooltip = 0.0;
				}
			}
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

				if ( sendItem1Uid != 0 )
				{
					animTooltip = 0.0;
				}
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

	bool tryShrineRequestAction = false;
	bool activateSelection = false;
	bool actionHoldButton = false;
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
					if ( players[playernum]->inventory_mode != INVENTORY_MODE_SPELL )
					{
						// prevent double sound
						Player::soundCancel();
					}
					parentGUI.closeGUI();
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
				if ( Input::inputs[playernum].binaryToggle("MenuAlt2") )
				{
					if ( currentView == ASSIST_SHRINE_VIEW_ACTION )
					{
						if ( !offeringGlyph->disabled && isInteractable && viewActionReady )
						{
							actionHoldButton = true;

							SDL_Color color;
							getColor(actionBtn->getTextColor(), &color.r, &color.g, &color.b, &color.a);
							actionBtn->setTextColor(makeColor(201, 162, 100, color.a));

							{
								if ( holdButtonProcessedOnTick == 0 )
								{
									holdButtonProcessedOnTick = ::ticks;
									Player::soundActivate();
#ifdef USE_FMOD
									bool isplaying = false;
									buttonSound[playernum]->isPlaying(&isplaying);
									if ( isplaying )
									{
										buttonSound[playernum]->stop();
									}
									buttonSound[playernum] = playSound(922, 128);
#endif
								}
							}

							if ( holdButtonProcessedOnTick != 0 )
							{
								if ( ticks - holdButtonProcessedOnTick >= 1.75 * TICKS_PER_SECOND )
								{
									activateSelection = true;
									tryShrineRequestAction = true;
									Input::inputs[playernum].consumeBinaryToggle("MenuAlt2");
								}
							}
						}
						else
						{
							Input::inputs[playernum].consumeBinaryToggle("MenuAlt2");
						}
					}
					else if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
					{
						if ( !offeringGlyph->disabled && isInteractable )
						{
							activateSelection = true;
							tryShrineRequestAction = true;
						}
						Input::inputs[playernum].consumeBinaryToggle("MenuAlt2");
					}
					else
					{
						Input::inputs[playernum].consumeBinaryToggle("MenuAlt2");
					}
				}
				else if ( Input::inputs[playernum].binaryToggle("MenuPageRight") || Input::inputs[playernum].binaryToggle("MenuPageLeft") )
				{
					bool left = Input::inputs[playernum].consumeBinaryToggle("MenuPageLeft");
					bool right = Input::inputs[playernum].consumeBinaryToggle("MenuPageRight");
					if ( (left || right)  )
					{
						if ( left && (isInteractable && !arrowLeftBtn->isInvisible()) )
						{
							arrowLeftBtn->activate();
							return;
						}
						if ( right && (isInteractable && !arrowRightBtn->isInvisible()) )
						{
							arrowRightBtn->activate();
							return;
						}
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
	}

	if ( !actionHoldButton )
	{
		if ( holdButtonProcessedOnTick != 0 )
		{
			holdButtonProcessedOnTick = 0;
#ifdef USE_FMOD
			bool isplaying = false;
			buttonSound[playernum]->isPlaying(&isplaying);
			if ( isplaying )
			{
				buttonSound[playernum]->stop();
			}
#endif
			Player::soundCancel();
		}
	}
	else
	{
		if ( actionBtn->getDrawCallback() == nullptr )
		{
			actionBtn->setDrawCallback([](const Widget& widget, SDL_Rect rect) {

				const Frame* parent = static_cast<const Frame*>(widget.getParent());

				const int player = widget.getOwner();
				SDL_Rect drawRect = rect;
				drawRect.x += 108 / 2;
				drawRect.y += 46 / 2;
				drawRect.w = 108;
				drawRect.h = 46;
				real_t opacity = 255;
				if ( parent && parent->getOpacity() < 100.0 )
				{
					opacity *= parent->getOpacity() / 100.0;
				}
				real_t progress = 0.0;
				if ( GenericGUI[player].eternalShrineGUI.holdButtonProcessedOnTick != 0 )
				{
					progress = (::ticks - GenericGUI[player].eternalShrineGUI.holdButtonProcessedOnTick) / (1.75 * TICKS_PER_SECOND);
				}
				progress = std::min(1.0, progress);
				if ( progress > 0.0 )
				{
					drawClockwiseSquareMesh("images/ui/shrines/ButtonHoldHighlight.png",
						progress,
						drawRect, makeColor(255, 255, 255, opacity));
				}
			});
		}
	}

	if ( activateSelection && players[playernum] && players[playernum]->entity
		/*&& animRecvItem < 0.001*/ )
	{
		if ( itemActionType != ETERNAL_ITEM_OK && !tryShrineRequestAction && itemActionType != ETERNAL_ITEM_NONE )
		{
			playSound(90, 64);
		}
		if ( (player->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE 
			|| (tryShrineRequestAction && (player->GUI.activeModule == Player::GUI_t::MODULE_INVENTORY || player->GUI.activeModule == Player::GUI_t::MODULE_SPELLS)))
			&& (itemActionType == ETERNAL_ITEM_OK || tryShrineRequestAction)
			)
		{
			if ( (getSelectedEternalShrineX() >= ETERNALSHRINE_SLOT_SEND && getSelectedEternalShrineX() < 0
				&& getSelectedEternalShrineY() == 0) || tryShrineRequestAction )
			{
				if ( !tryShrineRequestAction && getSelectedEternalShrineX() == ETERNALSHRINE_SLOT_SEND )
				{
					sendItem1Uid = 0;
					animSendItem1 = 0.0;
					animSendItem1Frame->setDisabled(true);
					Player::soundCancel();
				}
				else if ( tryShrineRequestAction )
				{
					if ( currentView == ASSIST_SHRINE_VIEW_ACTION )
					{
						if ( viewActionReady )
						{
							if ( !actionBtn->isInvisible() )
							{
								actionBtn->activate();
								return;
							}
						}
					}
					else if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
					{
						if ( !(player->GUI.activeModule == Player::GUI_t::MODULE_SPELLS && parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION) )
						{
							if ( !offeringBtn->isInvisible() )
							{
								offeringBtn->activate();
								if ( sendItem1Uid == 0 )
								{
									player->inventoryUI.tooltipDelayTick = ::ticks + TICKS_PER_SECOND;
									// consumed
									if ( player->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE )
									{
										if ( player->inventory_mode == INVENTORY_MODE_SPELL )
										{
											player->GUI.activateModule(Player::GUI_t::MODULE_SPELLS);
										}
										else
										{
											player->GUI.activateModule(Player::GUI_t::MODULE_INVENTORY);
										}
									}
								}
								return;
							}
						}
					}
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
		SDL_Rect offeringBtnPos{ 0, 0, 112, 26 };
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

					if ( players[player]->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE )
					{
						if ( players[player]->inventory_mode == INVENTORY_MODE_SPELL )
						{
							players[player]->GUI.activateModule(Player::GUI_t::MODULE_SPELLS);
						}
						else
						{
							players[player]->GUI.activateModule(Player::GUI_t::MODULE_INVENTORY);
						}
					}
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

					if ( players[player]->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE )
					{
						if ( players[player]->inventory_mode == INVENTORY_MODE_SPELL )
						{
							players[player]->GUI.activateModule(Player::GUI_t::MODULE_SPELLS);
						}
						else
						{
							players[player]->GUI.activateModule(Player::GUI_t::MODULE_INVENTORY);
						}
					}
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

	if ( itemCategory(item) == SPELL_CAT )
	{
		if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if ( currentView == ASSIST_SHRINE_VIEW_ACTION )
	{
		if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ANVIL )
		{
			if ( items[item->type].item_slot != ItemEquippableSlot::NO_EQUIP )
			{
				return true;
			}
			if ( item->type == ENCHANTED_FEATHER )
			{
				return true;
			}
			return false;
		}
		return true;
	}
	else
	{
		return true;
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
	if ( !bSendItemAllowed || submittedItem == SUBMIT_WAITING )
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
	else if ( inventoryItemAllowedInGUI(item) )
	{
		bool isEquipped = itemIsEquipped(item, player);
		if ( isEquipped )
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

void spawnHeatOrbitSpin(Entity* target, int sprite, bool light)
{
	for ( int i = 0; i < 2; ++i )
	{
		if ( Entity* fx = createParticleAestheticOrbit(target, sprite, TICKS_PER_SECOND / 2, PARTICLE_EFFECT_HEAT_ORBIT_SPIN) )
		{
			fx->flags[SPRITE] = true;
			fx->x = target->x;
			fx->y = target->y;
			fx->z = 7.5;
			fx->fskill[0] = fx->x;
			fx->fskill[1] = fx->y;
			fx->vel_z = -0.5;
			fx->actmagicOrbitDist = 5;
			fx->fskill[2] = target->yaw + PI / 4.0 + i * PI;
			fx->yaw = fx->fskill[2];
			fx->fskill[4] = 0.25;
			if ( !light )
			{
				fx->lightBonus = vec4{ 0.f, 0.f, 0.f, 0.f };
				fx->actmagicNoLight = 1;
			}

		}
	}
	serverSpawnMiscParticles(target, PARTICLE_EFFECT_HEAT_ORBIT_SPIN, sprite, !light ? 1 : 0, TICKS_PER_SECOND / 2);
}

bool applyShrineEffect(std::string effect_str, Entity* target, Entity* shrine, int tier)
{
	if ( !target ) { return false; }
	if ( !shrine ) { return false; }
	Stat* myStats = target->getStats();
	if ( !myStats ) { return false; }

	if ( effect_str == "EFFECT_EMPTY" || effect_str == "" )
	{
		return false;
	}

	int player = target->isEntityPlayer();
	bool result = false;

	if ( effect_str == "DIVINE_FIRE" )
	{
		int damageTicks = 8;
		int burnDuration = damageTicks * 40 + 20;
		if ( target->setEffect(EFF_HOLY_FIRE, (Uint8)(1 + tier), burnDuration, true, true, true) )
		{
			if ( Entity* fx = createParticleAestheticOrbit(target, 288, burnDuration + 20, PARTICLE_EFFECT_HOLY_FIRE) )
			{
				fx->flags[SPRITE] = true;
				fx->flags[INVISIBLE] = true;
				fx->skill[3] = shrine->getUID();
			}
			serverSpawnMiscParticles(target, PARTICLE_EFFECT_HOLY_FIRE, 288, 0, burnDuration + 20);
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7129));
			}

			spawnHeatOrbitSpin(target, 288, true);
			playSoundEntity(target, 164, 128);
			result = true;
		}
	}
	else if ( effect_str == "INCOHERENCE" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = 3;
		if ( tier == 2 )
		{
			effectStrength = 5;
		}
		if ( tier >= 3 )
		{
			effectStrength = 7;
		}
		if ( target->setEffect(EFF_INCOHERENCE, effectStrength, duration, false) )
		{
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(6911));
			}
			playSoundEntity(target, 825, 64);
			spawnMagicEffectParticles(target->x, target->y, target->z, 2355);
			result = true;
		}
	}
	else if ( effect_str == "SILENCE" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		if ( tier >= 4 )
		{
			duration *= 2;
		}
		Uint8 effectStrength = 1;
		target->setEffect(EFF_STASIS, (Uint8)2, 3 * TICKS_PER_SECOND, true, true, true); // aesthetic stasis
		if ( target->setEffect(EFF_SILENCED, effectStrength, duration, true) )
		{
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7111));
			}
			playSoundEntity(target, 166, 128);
			playSoundEntity(target, 825, 64);
			createParticleFociDark(target, 0, true);
			spawnMagicEffectParticles(target->x, target->y, target->z, 1818);
			result = true;
		}
	}
	else if ( effect_str == "LEVEL_DRAIN" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = std::min(5, std::max(1, tier)); // -20 to -60%
		target->setEffect(EFF_STASIS, (Uint8)2, 3 * TICKS_PER_SECOND, true, true, true);  // aesthetic stasis
		if ( target->setEffect(EFF_LEVEL_DRAIN, effectStrength, duration, true) )
		{
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7123));
			}
			playSoundEntity(target, 166, 128);
			createParticleFociDark(target, 0, true);
			result = true;
		}
	}
	else if ( effect_str == "BURDENED" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = 2; // -10% max movespeed/DEX
		if ( tier == 2 )
		{
			effectStrength = 3; // -15% max movespeed/DEX
		}
		else if ( tier >= 3 )
		{
			effectStrength = 4; // -20% max movespeed/DEX
		}
		target->setEffect(EFF_STASIS, (Uint8)2, 3 * TICKS_PER_SECOND, true, true, true);  // aesthetic stasis
		if ( target->setEffect(EFF_BURDENED, effectStrength, duration, true) )
		{
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7127));
			}
			//playSoundEntity(target, 824, 64);
			playSoundEntity(target, 166, 128);
			createParticleFociDark(target, 0, true);
			result = true;
		}
	}
	else if ( effect_str == "VOID_MP_DRAIN" ) // drain MP
	{
		int duration = 5 * TICKS_PER_SECOND + 10; // drains once per second
		Uint8 effectStrength = 4; // 5% for 25% total, 3 onwards is 2.5% per tick
		if ( tier == 2 )
		{
			effectStrength = 5; // 7.5% for 37.5% total
		}
		else if ( tier == 3 )
		{
			effectStrength = 6; // 10% for 50% total
		}
		else if ( tier == 4 )
		{
			effectStrength = 8; // 15% for 75% total
		}
		else if ( tier >= 5 )
		{
			effectStrength = 10; // 20% for 100% total
		}
		if ( target->setEffect(EFF_STASIS, (Uint8)effectStrength, duration, true, true, true) )
		{
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7124));
			}
			playSoundEntity(target, 166, 128);
			createParticleFociDark(target, 0, true);
			result = true;
		}
	}
	else if ( effect_str == "SMOKE_HP"
		|| effect_str == "SMOKE_MP" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = effect_str == "SMOKE_HP" ? 2 : 1;
		if ( tier >= 3 )
		{
			effectStrength += 2;
		}
		if ( target->setEffect(EFF_SMOKE_HPMP_RGN, effectStrength, duration, false, true, true) )
		{
			Uint32 color = makeColorRGB(0, 255, 0);
			messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7115));
		}

		result = true;
	}
	else if ( effect_str == "RESOLVE_HP"
		|| effect_str == "RESOLVE_MP" )
	{
		int duration = 15 * 60 * TICKS_PER_SECOND;
		Uint8 strength = tier;
		if ( tier == 2 )
		{
			strength = 3;
		}
		if ( tier == 3 )
		{
			strength = 5;
		}
		Uint8 effectStrength = target->getStats() ? target->getStats()->getEffectActive(EFF_RESOLVE) : 0;
		Uint8 hp = effectStrength & 0xF;
		Uint8 mp = (effectStrength >> 4) & 0xF;
		if ( effect_str == "RESOLVE_HP" )
		{
			hp = std::max(hp, strength);
			if ( mp )
			{
				mp = std::max(mp, hp);
			}
		}
		else if ( effect_str == "RESOLVE_MP" )
		{
			mp = std::max(mp, strength);
			if ( hp )
			{
				hp = std::max(mp, hp);
			}
		}
		effectStrength = hp;
		effectStrength |= (mp & 0xF) << 4;
		if ( target->setEffect(EFF_RESOLVE, effectStrength, duration, false, true, true) )
		{
			Uint32 color = makeColorRGB(0, 255, 0);
			messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7135));
		}

		result = true;
	}
	else if ( effect_str == "STAMINA" )
	{
		int effectStrength = 1;
		int duration = 5 * TICKS_PER_SECOND * 60;
		if ( target->setEffect(EFF_CON_BONUS, (Uint8)effectStrength, duration, false) )
		{
			messagePlayerColor(target->isEntityPlayer(), MESSAGE_STATUS, uint32ColorGreen, Language::get(7134), Language::get(6282));
			result = true;
		}
	}
	else if ( effect_str == "AGILITY" )
	{
		int effectStrength = 1;
		int duration = 5 * TICKS_PER_SECOND * 60;
		if ( target->setEffect(EFF_AGILITY, (Uint8)effectStrength, duration, false) )
		{
			messagePlayerColor(target->isEntityPlayer(), MESSAGE_STATUS, uint32ColorGreen, Language::get(7134), Language::get(6283));
			result = true;
		}
	}
	else if ( effect_str == "MENTALITY" )
	{
		int effectStrength = 1;
		int duration = 5 * TICKS_PER_SECOND * 60;
		if ( target->setEffect(EFF_PWR, (Uint8)effectStrength, duration, false) )
		{
			messagePlayerColor(target->isEntityPlayer(), MESSAGE_STATUS, uint32ColorGreen, Language::get(7134), Language::get(6284));
			result = true;
		}
	}
	else if ( effect_str == "SLOW_DIGESTION" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = 3;
		if ( tier == 2 )
		{
			effectStrength = 5;
		}
		else if ( tier >= 3 )
		{
			effectStrength = 7;
		}
		if ( target->setEffect(EFF_SLOW_DIGEST, effectStrength, duration, false, true, true) )
		{
			Uint32 color = makeColorRGB(0, 255, 0);
			messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(2385));
			playSoundEntity(target, 168, 128);
			createParticleFociLight(target, 0, true);
			result = true;
		}
	}
	else if ( effect_str == "BLESSED_MEALS" )
	{
		int duration = 5; // 5 minutes
		CastSpellProps_t props;
		props.targetUID = target->getUID();
		props.optionalData = duration; // converts to 5 minutes at spell

		castSpell(shrine->getUID(), getSpellFromID(SPELL_BLESS_FOOD), false, true, false, &props);
		result = true;
	}
	else if ( effect_str == "SACRED_PATH" )
	{
		int charges = tier * 4;
		CastSpellProps_t props;
		props.targetUID = target->getUID();
		props.optionalData = charges;

		castSpell(shrine->getUID(), getSpellFromID(SPELL_SACRED_PATH), false, true, false, &props);
		result = true;
	}
	else if ( effect_str == "SCRY_TREASURES" )
	{
		CastSpellProps_t props;
		props.targetUID = target->getUID();
		props.optionalData = 2; // unique for scry treasures

		castSpell(shrine->getUID(), getSpellFromID(SPELL_SCRY_TREASURES), false, true, false, &props);
		result = true;
	}
	else if ( effect_str == "DEGENERATION" || effect_str == "DISPIRITED" )
	{
		int effectID = effect_str == "DEGENERATION" ? EFF_DEGENERATION : EFF_DISPIRITED;

		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = 3;
		if ( tier == 2 )
		{
			effectStrength = 5;
		}
		else if ( tier >= 3 )
		{
			effectStrength = 7;
		}
		target->setEffect(EFF_STASIS, (Uint8)2, 3 * TICKS_PER_SECOND, true, true, true);  // aesthetic stasis
		if ( target->setEffect(effectID, effectStrength, duration, false) )
		{
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				if ( effectID == EFF_DISPIRITED )
				{
					messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7118));
					playSoundEntity(target, 825, 64);
				}
				else if ( effectID == EFF_DEGENERATION )
				{
					messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7116));
					playSoundEntity(target, 824, 64);
				}
			}
			if ( effectID == EFF_DISPIRITED )
			{
				spawnMagicEffectParticles(target->x, target->y, target->z, 2355);
			}
			else
			{
				spawnMagicEffectParticles(target->x, target->y, target->z, 2367);
			}
			playSoundEntity(target, 166, 128);
			createParticleFociDark(target, 0, true);
			result = true;
		}
	}
	else if ( effect_str == "WEAKNESS" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = 3;
		if ( tier == 2 )
		{
			effectStrength = 5;
		}
		if ( tier >= 3 )
		{
			effectStrength = 7;
		}
		if ( target->setEffect(EFF_WEAKNESS, effectStrength, duration, false) )
		{
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(6528));
			}
			playSoundEntity(target, 824, 64);
			spawnMagicEffectParticles(target->x, target->y, target->z, 2367);
			result = true;
		}
	}
	else if ( effect_str == "ENFEEBLE" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = 1; // 10% per str
		if ( tier == 2 )
		{
			effectStrength = 3;
		}
		if ( tier >= 3 )
		{
			effectStrength = 5;
		}
		if ( target->setEffect(EFF_ENFEEBLE, effectStrength, duration, false) )
		{
			Uint32 color = makeColorRGB(255, 0, 0);
			if ( player >= 0 )
			{
				messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(7125));
			}
			playSoundEntity(target, 824, 64);
			spawnMagicEffectParticles(target->x, target->y, target->z, 2367);
			result = true;
		}
	}
	else if ( effect_str == "POLYMORPH" )
	{
		spellEffectPolymorph(target, shrine, false, 0, SKELETON);
		if ( target->getStats() && target->getStats()->getEffectActive(EFF_POLYMORPH)
			&& target->playerGetMonsterRaceFromPolymorph() == SKELETON )
		{
			result = true;
		}
	}
	else if ( effect_str == "SHAPESHIFT" )
	{
		Monster type = RAT;
		int duration = 5 * 60 * TICKS_PER_SECOND;
		if ( type != NOTHING && target->setEffect(EFF_SHAPESHIFT, true, duration, true) )
		{
			spawnExplosion(target->x, target->y, target->z);
			playSoundEntity(target, 400, 92);
			createParticleDropRising(target, 593, 1.f);
			serverSpawnMiscParticles(target, PARTICLE_EFFECT_RISING_DROP, 593);

			target->effectShapeshift = type;
			serverUpdateEntitySkill(target, 53);

			for ( node_t* node = map.creatures->first; node && myStats; node = node->next )
			{
				Entity* entity = (Entity*)(node->element);
				if ( !entity || entity == target )
				{
					continue;
				}
				if ( entity->behavior != &actMonster )
				{
					continue;
				}
				if ( entity->monsterTarget == target->getUID() && entity->checkEnemy(target) )
				{
					Monster oldType = myStats->type;
					myStats->type = type;
					if ( !entity->checkEnemy(target) ) // we're now friendly.
					{
						entity->monsterReleaseAttackTarget();
					}
					myStats->type = oldType;
				}
			}

			Uint32 color = makeColorRGB(0, 255, 0);
			messagePlayerColor(player, MESSAGE_STATUS, color, Language::get(3419), getMonsterLocalizedName((Monster)target->effectShapeshift).c_str());
		}
		result = true;
	}
	else if ( effect_str == "SCAPEGOAT" )
	{
		CastSpellProps_t props;
		props.targetUID = target->getUID();

		castSpell(shrine->getUID(), getSpellFromID(SPELL_TABOO), false, true, false, &props);
		result = true;
	}
	else if ( effect_str == "DIZZY" )
	{
		Uint8 effectStrength = 3;
		int duration = 75;
		if ( tier == 2 )
		{
			effectStrength = 5;
		}
		if ( tier >= 3 )
		{
			effectStrength = 5;
			duration = 125;
		}
		if ( target->setEffect(EFF_SPIN, effectStrength, duration, false, true, true) )
		{
			if ( target->setEffect(EFF_KNOCKBACK, true, duration, false) )
			{
				real_t tangent = atan2(target->y - shrine->y, target->x - shrine->x);
				tangent -= PI / 2;
				tangent += (local_rng.rand() % 5) * PI / 4;

				if ( target->behavior == &actPlayer )
				{
					real_t pushbackMultiplier = 0.5;
					if ( !players[target->skill[2]]->isLocalPlayer() )
					{
						target->monsterKnockbackVelocity = pushbackMultiplier;
						target->monsterKnockbackTangentDir = tangent;
						serverUpdateEntityFSkill(target, 11);
						serverUpdateEntityFSkill(target, 9);
					}
					else
					{
						target->monsterKnockbackVelocity = pushbackMultiplier;
						target->monsterKnockbackTangentDir = tangent;
					}
				}
			}
			createParticleSpin(target);
			serverSpawnMiscParticles(target, PARTICLE_EFFECT_SPIN, -1);
			messagePlayerColor(target->isEntityPlayer(), MESSAGE_STATUS, makeColorRGB(255, 0, 0), Language::get(7109));
			playSoundEntity(target, 758, 92);
			result = true;
		}
	}
	else if ( effect_str == "DUSTED" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		if ( target->setEffect(EFF_DUSTED, true, duration, true) )
		{
			messagePlayerColor(target->isEntityPlayer(), MESSAGE_STATUS, makeColorRGB(255, 0, 0), Language::get(6752));
			playSoundEntity(target, 825, 64);
			result = true;
		}
	}
	else if ( effect_str == "EARTH_SPRITE" )
	{
		int duration = 5 * 60 * TICKS_PER_SECOND;
		Uint8 effectStrength = 1;
		if ( tier == 2 )
		{
			effectStrength = 1;
		}
		if ( tier >= 3 )
		{
			effectStrength = 2;
		}
		//if ( target->setEffect(EFF_DISRUPTED, effectStrength, duration, true) )
		//{
		//	//playSoundEntity(target, 799, 64);
		//}
		messagePlayerColor(target->isEntityPlayer(), MESSAGE_STATUS, makeColorRGB(255, 0, 0), Language::get(7110));
		CastSpellProps_t props;
		props.caster_x = target->x;
		props.caster_y = target->y;
		props.target_x = target->x;
		props.target_y = target->y;
		castSpell(shrine->getUID(), getSpellFromID(SPELL_DISRUPT_EARTH), false, true, false, &props);
		castSpell(shrine->getUID(), getSpellFromID(SPELL_EARTH_ELEMENTAL), false, true, false, &props);
		result = true;
	}
	else if ( effect_str == "LIGHTNING_BOLT" ) // lightning bolt
	{
		Uint32 lifetime = 3 * TICKS_PER_SECOND;
		target->setEffect(EFF_STATIC, (Uint8)5, lifetime + TICKS_PER_SECOND, true);

		Entity* spellTimer = createParticleTimer(shrine, lifetime + TICKS_PER_SECOND, -1);
		spellTimer->particleTimerCountdownAction = PARTICLE_TIMER_ACTION_LIGHTNING;
		spellTimer->particleTimerCountdownSprite = 1757;
		spellTimer->yaw = shrine->yaw;
		spellTimer->x = target->x;
		spellTimer->y = target->y;
		spellTimer->flags[NOUPDATE] = false; // spawn for client
		spellTimer->flags[UPDATENEEDED] = true;
		Sint32 val = (1 << 31);
		val |= (Uint8)(19);
		val |= (((Uint16)(spellTimer->particleTimerDuration) & 0xFFF) << 8);
		val |= (Uint8)(spellTimer->particleTimerCountdownAction & 0xFF) << 20;
		spellTimer->skill[2] = val;
		spellTimer->particleTimerEffectLifetime = lifetime;
		spellTimer->actmagicSpellbookBonus = 0;
		if ( tier >= 2 )
		{
			spellTimer->actmagicSpellbookBonus = 75 * (tier - 1);
		}
		spellTimer->actmagicFromSpellbook = 0;
		spellTimer->actmagicOrbitHitTargetUID1 = target->getUID();
		floorMagicCreateLightningSequence(spellTimer, 0);
		spawnMagicEffectParticles(shrine->x, shrine->y, shrine->z, 170);
		playSoundEntity(target, 806, 128);
		result = true;
	}
	else if ( effect_str == "EXORCISED" ) // exorcise
	{
		if ( Entity* fx1 = createParticleAestheticOrbit(target, 2401, 3 * TICKS_PER_SECOND, PARTICLE_EFFECT_TURN_UNDEAD) )
		{
			fx1->yaw = target->yaw;
			fx1->fskill[4] = target->x;
			fx1->fskill[5] = target->y;
			fx1->x = target->x;
			fx1->y = target->y;
			fx1->fskill[6] = fx1->yaw;
			fx1->skill[3] = shrine->getUID();
			//if ( effectStrength >= 3 )
			{
				fx1->skill[6] = EFF_HOLY_FIRE;
			}
			fx1->actmagicSpellbookBonus = 0;
			if ( tier >= 2 )
			{
				fx1->actmagicSpellbookBonus += 250 * (tier - 1);
			}
			fx1->actmagicFromSpellbook = 0;
			result = true;
		}

		serverSpawnMiscParticles(target, PARTICLE_EFFECT_TURN_UNDEAD, 2401);
	}
	else if ( effect_str == "PSYCHIC_SPEAR" ) // psychic spear
	{
		spawnMagicEffectParticles(target->x, target->y, target->z, 2357);

		//for ( int i = 0; i < 3; ++i )
		{
			Entity* fx = createParticleAestheticOrbit(target, 2362, 5 * TICKS_PER_SECOND, PARTICLE_EFFECT_PSYCHIC_SPEAR);
			fx->yaw = target->yaw;
			fx->skill[3] = shrine->getUID();
			fx->pitch = 0;// PI / 4;
			fx->fskill[0] = fx->yaw + PI / 2 + (local_rng.rand() % 6) * PI / 3;
			fx->fskill[1] = PI / 4 + PI / 8;// +(i + 1) * 2 * PI / 3;
			fx->x = target->x - 8.0 * cos(fx->yaw);
			fx->y = target->y - 8.0 * sin(fx->yaw);
			fx->z = target->z;// -8.0;
			fx->scalex = 0.0;
			fx->scaley = 0.0;
			fx->scalez = 0.0;
			fx->actmagicSpellbookBonus = 0;
			if ( tier >= 2 )
			{
				fx->actmagicSpellbookBonus += 90;
				fx->actmagicSpellbookBonus += 180 * (tier - 1);
			}
			fx->actmagicFromSpellbook = 0;

			serverSpawnMiscParticles(target, PARTICLE_EFFECT_PSYCHIC_SPEAR, 2362, 0, 5 * TICKS_PER_SECOND, fx->yaw * 256.0);
			result = true;
		}
	}
	else if ( effect_str == "GREATER_MIGHT" || effect_str == "STURDINESS"
		|| effect_str == "NIMBLENESS" || effect_str == "COUNSEL" )
	{
		CastSpellProps_t props;
		props.targetUID = target->getUID();
		props.optionalData = std::min(4, tier);
		if ( effect_str == "GREATER_MIGHT" )
		{
			castSpell(shrine->getUID(), getSpellFromID(SPELL_PROF_GREATER_MIGHT), false, true, false, &props);
		}
		else if ( effect_str == "STURDINESS" )
		{
			castSpell(shrine->getUID(), getSpellFromID(SPELL_PROF_STURDINESS), false, true, false, &props);
		}
		else if ( effect_str == "NIMBLENESS" )
		{
			castSpell(shrine->getUID(), getSpellFromID(SPELL_PROF_NIMBLENESS), false, true, false, &props);
		}
		else if ( effect_str == "COUNSEL" )
		{
			castSpell(shrine->getUID(), getSpellFromID(SPELL_PROF_COUNSEL), false, true, false, &props);
		}
		result = true;
	}
	else if ( effect_str == "DONATION" 
		|| effect_str == "HIDDEN_KNOWLEDGE"
		|| effect_str == "HEALING_POTION"
		|| effect_str == "MANA_POTION" )
	{
		CastSpellProps_t props;
		props.targetUID = target->getUID();
		props.optionalData = std::min(3, tier);
		if ( effect_str == "HEALING_POTION" )
		{
			props.optionalData |= (1 << 4);
		}
		else if ( effect_str == "MANA_POTION" )
		{
			props.optionalData |= (2 << 4);
		}
		/*else if ( effect_str == "HIDDEN_KNOWLEDGE" )
		{
			props.optionalData |= (3 << 4);
		}*/
		castSpell(shrine->getUID(), getSpellFromID(SPELL_DONATION), false, true, false, &props);
		result = true;
	}
	else if ( effect_str == "TIER_ITEM" )
	{
		CastSpellProps_t props;
		props.targetUID = target->getUID();
		props.optionalData = std::min(3, tier);
		props.optionalData |= (4 << 4);
		castSpell(shrine->getUID(), getSpellFromID(SPELL_DONATION), false, true, false, &props);
		result = true;
	}
	else if ( effect_str == "LIGHTEN_LOAD" )
	{
		int effectStrength = std::min(3, tier) * 20;
		int duration = 5 * TICKS_PER_SECOND * 60;
		if ( target->setEffect(EFF_LIGHTEN_LOAD, (Uint8)effectStrength, duration, false) )
		{
			messagePlayerColor(target->isEntityPlayer(), MESSAGE_STATUS, uint32ColorGreen, Language::get(6681));
			playSoundEntity(target, 178, 128);
			spawnMagicEffectParticles(target->x, target->y, target->z, 170);
			result = true;
		}
	}
	else
	{
		return false;
	}
	return result;
}

bool applySupplicationEffect(std::string tier_str, Entity* target, Entity* shrine)
{
	if ( tier_str.length() <= 1 ) { return false; }
	if ( !target ) { return false; }
	if ( !shrine ) { return false; }
	Stat* myStats = target->getStats();
	if ( !myStats ) { return false; }
	if ( myStats->HP <= 0 ) { return false; }

	int player = target->isEntityPlayer();
	bool result = false;

	int tier = std::stoi(tier_str.substr(0, 1));

	bool anyHeal = false;
	bool anyMP = false;
	bool anyEffect = false;
	bool anyCureHeal = false;
	int numEffectsCured = 0;
	if ( tier_str.find("hp") != std::string::npos )
	{
		int flatHeal = 20 + tier * 10;
		int cap = 50 + tier * 50;
		real_t currentPercent = std::min(0.75, myStats->HP / (real_t)std::max(1, myStats->MAXHP));
		real_t percentHealing = std::max(0.0, 0.75 - currentPercent) * myStats->MAXHP;
		int healing = std::min(cap, std::max(flatHeal, (int)percentHealing));
		anyHeal = true;
		anyEffect = true;

		Uint8 prevStasis = target->getStats() ? target->getStats()->getEffectActive(EFF_STASIS) : false;
		if ( prevStasis )
		{
			target->getStats()->clearEffect(EFF_STASIS);
		}
		target->modHP(healing);
		if ( prevStasis )
		{
			target->getStats()->setEffectActive(EFF_STASIS, prevStasis);
		}
	}
	if ( tier_str.find("mp") != std::string::npos )
	{
		int flatHeal = 20 + tier * 10;
		int cap = 50 + tier * 50;
		real_t currentPercent = std::min(0.75, myStats->MP / (real_t)std::max(1, myStats->MAXMP));
		real_t percentHealing = std::max(0.0, 0.75 - currentPercent) * myStats->MAXMP;
		int healing = std::min(cap, std::max(flatHeal, (int)percentHealing));
		anyMP = true;
		anyEffect = true;

		int mpAmount = target->modMP(healing);
		target->playerInsectoidIncrementHungerToMP(mpAmount);
	}
	if ( tier_str.find("cure") != std::string::npos )
	{
		for ( int c = 0; c < NUMEFFECTS; c++ )   //This does a whole lot more than just cure ailments.
		{
			if ( myStats->statusEffectRemovedByCureAilment(c, target) )
			{
				if ( myStats->getEffectActive(c) )
				{
					myStats->clearEffect(c);
					if ( myStats->EFFECTS_TIMERS[c] > 0 )
					{
						myStats->EFFECTS_TIMERS[c] = 1;
					}
					++numEffectsCured;
				}
			}
		}

		if ( myStats->getEffectActive(EFF_WITHDRAWAL) )
		{
			++numEffectsCured;
			target->setEffect(EFF_WITHDRAWAL, false, EFFECT_WITHDRAWAL_BASE_TIME, true);
			serverUpdatePlayerGameplayStats(player, STATISTICS_FUNCTIONAL, 1);
		}

		if ( numEffectsCured > 0 )
		{
			anyEffect = true;
		}

		if ( !anyHeal )
		{
			int flatHeal = 10 + 5 * tier;
			if ( tier_str.find("hp") == std::string::npos )
			{
				if ( target->getStats()->HP < target->getStats()->MAXHP )
				{
					Uint8 prevStasis = target->getStats() ? target->getStats()->getEffectActive(EFF_STASIS) : false;
					if ( prevStasis )
					{
						target->getStats()->clearEffect(EFF_STASIS);
					}
					target->modHP(flatHeal);
					if ( prevStasis )
					{
						target->getStats()->setEffectActive(EFF_STASIS, prevStasis);
					}
					anyCureHeal = true;
					anyEffect = true;
				}
			}
			if ( tier_str.find("mp") == std::string::npos )
			{
				if ( target->getStats()->MP < target->getStats()->MAXMP )
				{
					int mpAmount = target->modMP(flatHeal);
					target->playerInsectoidIncrementHungerToMP(mpAmount);
					anyCureHeal = true;
					anyEffect = true;
				}
			}
		}

		serverUpdateEffects(player);
	}
	if ( tier_str.find("food") != std::string::npos )
	{
		auto& rng = shrine->entity_rng ? *shrine->entity_rng : local_rng;
		std::vector<Item*> items_list;
		if ( stats[player]->type == AUTOMATON )
		{
			if ( tier == 0 || tier == 1 )
			{
				items_list.push_back(newItem(
					(ItemType)SCROLL_LIGHT,
					(Status)EXCELLENT,
					0,
					1,
					local_rng.rand(),
					false, nullptr));

				items_list.push_back(newItem(
					(ItemType)SCROLL_LIGHT,
					(Status)EXCELLENT,
					0,
					1,
					local_rng.rand(),
					false, nullptr));

				if ( tier == 1 )
				{
					items_list.push_back(newItem(
						(ItemType)SCROLL_LIGHT,
						(Status)EXCELLENT,
						0,
						1,
						local_rng.rand(),
						false, nullptr));
				}
			}
			else if ( tier >= 3 )
			{
				items_list.push_back(newItem(
					(ItemType)SCROLL_FIRE,
					(Status)EXCELLENT,
					0,
					1,
					local_rng.rand(),
					false, nullptr));
				if ( tier >= 4 )
				{
					items_list.push_back(newItem(
						(ItemType)SCROLL_FIRE,
						(Status)EXCELLENT,
						0,
						1,
						local_rng.rand(),
						false, nullptr));
				}
				if ( tier >= 5 )
				{
					items_list.push_back(newItem(
						(ItemType)SCROLL_FIRE,
						(Status)EXCELLENT,
						0,
						1,
						local_rng.rand(),
						false, nullptr));
				}
			}
		}
		else if ( tier == 0 )
		{
			items_list.push_back(newItem(
				(ItemType)FOOD_APPLE,
				(Status)EXCELLENT,
				0,
				3,
				local_rng.rand(),
				false, nullptr));
		}
		else if ( tier == 1 )
		{
			ItemType type = FOOD_SHROOM;
			if ( rng.rand() % 2 == 0 )
			{
				type == FOOD_NUT;
			}
			items_list.push_back(newItem(
				(ItemType)type,
				(Status)EXCELLENT,
				1,
				3,
				local_rng.rand(),
				false, nullptr));

			type = FOOD_SHROOM;
			if ( rng.rand() % 2 == 0 )
			{
				type == FOOD_NUT;
			}
			items_list.push_back(newItem(
				(ItemType)type,
				(Status)EXCELLENT,
				1,
				3,
				local_rng.rand(),
				false, nullptr));
		}
		else if ( tier == 2 )
		{
			ItemType type = FOOD_SHROOM;
			if ( rng.rand() % 2 == 0 )
			{
				type == FOOD_NUT;
			}
			items_list.push_back(newItem(
				(ItemType)type,
				(Status)EXCELLENT,
				1,
				6,
				local_rng.rand(),
				false, nullptr));

			items_list.push_back(newItem(
				(ItemType)(FOOD_RATION_SPICY + rng.rand() % 6),
				(Status)EXCELLENT,
				1,
				3,
				local_rng.rand(),
				false, nullptr));

			if ( rng.rand() % 10 == 0 )
			{
				items_list.push_back(newItem(
					(ItemType)(MASK_MARIGOLD),
					(Status)EXCELLENT,
					0,
					1,
					local_rng.rand(),
					false, nullptr));
			}
		}
		else if ( tier == 3 )
		{
			std::vector<unsigned int> chances = { 1, 1, 1, 1, 1, 1 };
			for ( int i = 0; i < 3; ++i )
			{
				int pick = rng.discrete(chances.data(), chances.size());
				chances[pick] = 0;
				ItemType type = (ItemType)(FOOD_RATION_SPICY + pick);

				items_list.push_back(newItem(
					(ItemType)(type),
					(Status)EXCELLENT,
					1,
					3,
					local_rng.rand(),
					false, nullptr));
			}

			if ( rng.rand() % 5 == 0 )
			{
				items_list.push_back(newItem(
					(ItemType)(MASK_MARIGOLD),
					(Status)EXCELLENT,
					0,
					1,
					local_rng.rand(),
					false, nullptr));
			}
		}
		else if ( tier == 4 )
		{
			std::vector<unsigned int> chances = { 1, 1, 1, 1, 1, 1 };
			for ( int i = 0; i < 3; ++i )
			{
				int pick = rng.discrete(chances.data(), chances.size());
				chances[pick] = 0;
				ItemType type = (ItemType)(FOOD_RATION_SPICY + pick);

				items_list.push_back(newItem(
					(ItemType)(type),
					(Status)EXCELLENT,
					2,
					6,
					local_rng.rand(),
					false, nullptr));
			}

			if ( rng.rand() % 3 == 0 )
			{
				items_list.push_back(newItem(
					(ItemType)(MASK_MARIGOLD),
					(Status)EXCELLENT,
					1,
					1,
					local_rng.rand(),
					false, nullptr));
			}
		}

		int dropIndex = -1;
		for ( auto item : items_list )
		{
			++dropIndex;
			if ( items[item->type].category == FOOD )
			{
				if ( playerRequiresBloodToSustain(player) )
				{
					item->type = FOOD_BLOOD;
				}
				/*if ( stats[player]->type == SKELETON && (svFlags & SV_FLAG_HUNGER) )
				{
					item->type = BONE_THROWING;
					item->beatitude = 0;
					item->status = DECREPIT;
				}*/
			}
			if ( Entity* dropped = dropItemMonster(item, shrine, nullptr, item->count) )
			{
				dropped->x = shrine->x;
				dropped->y = shrine->y;

				if ( items_list.size() == 2 )
				{
					dropped->x += 3.0 * cos(shrine->yaw + PI / 2 + PI * dropIndex);
					dropped->y += 3.0 * sin(shrine->yaw + PI / 2 + PI * dropIndex);
				}
				else if ( items_list.size() == 3 )
				{
					dropped->x += 3.0 * cos(shrine->yaw + (2 * PI / 3) * dropIndex);
					dropped->y += 3.0 * sin(shrine->yaw + (2 * PI / 3) * dropIndex);
				}
				else if ( items_list.size() == 4 )
				{
					dropped->x += 3.0 * cos(shrine->yaw + PI / 4 + (PI / 2) * dropIndex);
					dropped->y += 3.0 * sin(shrine->yaw + PI / 4 + (PI / 2) * dropIndex);
				}

				dropped->z = -4.0;
				dropped->yaw = shrine->yaw + PI;
				//dropped->x += 4.0 * cos(my->yaw) - 0.0 * cos(my->yaw + PI / 2);
				//dropped->y += 4.0 * sin(my->yaw) - 0.0 * sin(my->yaw + PI / 2);
				dropped->vel_x = 0.0;
				dropped->vel_y = 0.0;
				dropped->itemEternalShrineResult = shrine->eternalShrineType;

				dropped->itemNotMoving = 0;
				dropped->itemNotMovingClient = 0;
				//dropped->vel_z = -0.25;
				dropped->itemLevitate = 1.0;
				dropped->itemLevitateStartZ = dropped->z;

				playSoundEntity(shrine, 909, 128);
				anyEffect = true;
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

	if ( anyEffect )
	{
		if ( anyHeal || anyMP || numEffectsCured > 0 || anyCureHeal )
		{
			if ( anyHeal )
			{
				if ( anyMP )
				{
					messagePlayerColor(player, MESSAGE_STATUS, makeColorRGB(0, 255, 0), Language::get(7153)); // renewed
				}
				else if ( (numEffectsCured > 0 || anyCureHeal) )
				{
					messagePlayerColor(player, MESSAGE_STATUS, makeColorRGB(0, 255, 0), Language::get(7156)); // cleansed and healthy
				}
				else
				{
					messagePlayerColor(player, MESSAGE_STATUS, makeColorRGB(0, 255, 0), Language::get(7154)); // healthier
				}
			}
			else if ( anyMP )
			{
				if ( (numEffectsCured > 0 || anyCureHeal) )
				{
					messagePlayerColor(player, MESSAGE_STATUS, makeColorRGB(0, 255, 0), Language::get(7157)); // cleansed and energized
				}
				else
				{
					messagePlayerColor(player, MESSAGE_STATUS, makeColorRGB(0, 255, 0), Language::get(7152)); // energized
				}
			}
			else if ((numEffectsCured > 0 || anyCureHeal) )
			{
				messagePlayerColor(player, MESSAGE_STATUS, makeColorRGB(0, 255, 0), Language::get(763)); // cleansed
			}
			playSoundEntity(target, 168, 128);
			spawnMagicEffectParticles(target->x, target->y, target->z, 169);
		}
	}
	return anyEffect;
}