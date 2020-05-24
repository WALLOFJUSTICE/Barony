/*-------------------------------------------------------------------------------

BARONY
File: mod_tools.hpp
Desc: misc modding tools

Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once
#include "main.hpp"
#include "stat.hpp"
#include "json.hpp"
#include "files.hpp"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

class MonsterStatCustomManager
{
public:
	std::mt19937 monsterStatSeed;
	static const std::vector<std::string> itemStatusStrings;
	MonsterStatCustomManager() :
		monsterStatSeed(rand())
	{
	};

	int getSlotFromKeyName(std::string keyName)
	{
		if ( keyName.compare("weapon") == 0 )
		{
			return ITEM_SLOT_WEAPON;
		}
		else if ( keyName.compare("shield") == 0 )
		{
			return ITEM_SLOT_SHIELD;
		}
		else if ( keyName.compare("helmet") == 0 )
		{
			return ITEM_SLOT_HELM;
		}
		else if ( keyName.compare("breastplate") == 0 )
		{
			return ITEM_SLOT_ARMOR;
		}
		else if ( keyName.compare("gloves") == 0 )
		{
			return ITEM_SLOT_GLOVES;
		}
		else if ( keyName.compare("shoes") == 0 )
		{
			return ITEM_SLOT_BOOTS;
		}
		else if ( keyName.compare("cloak") == 0 )
		{
			return ITEM_SLOT_CLOAK;
		}
		else if ( keyName.compare("ring") == 0 )
		{
			return ITEM_SLOT_RING;
		}
		else if ( keyName.compare("amulet") == 0 )
		{
			return ITEM_SLOT_AMULET;
		}
		else if ( keyName.compare("mask") == 0 )
		{
			return ITEM_SLOT_MASK;
		}
		return 0;
	}

	class ItemEntry
	{
	public:
		ItemType type = WOODEN_SHIELD;
		Status status = BROKEN;
		Sint16 beatitude = 0;
		Sint16 count = 1;
		Uint32 appearance = 0;
		bool identified = 0;
		int percentChance = 100;
		ItemEntry() {};
		ItemEntry(const Item& itemToRead)
		{
			readFromItem(itemToRead);
		}
		void readFromItem(const Item& itemToRead)
		{
			type = itemToRead.type;
			status = itemToRead.status;
			beatitude = itemToRead.beatitude;
			count = itemToRead.count;
			appearance = itemToRead.appearance;
			identified = itemToRead.identified;
		}
		void setValueFromAttributes(rapidjson::Document& d, rapidjson::Value& outObject)
		{
			rapidjson::Value key1("type", d.GetAllocator());
			rapidjson::Value val1(itemNameStrings[type + 2], d.GetAllocator());
			outObject.AddMember(key1, val1, d.GetAllocator());

			rapidjson::Value key2("status", d.GetAllocator());
			rapidjson::Value val2(itemStatusStrings.at(status).c_str(), d.GetAllocator());
			outObject.AddMember(key2, val2, d.GetAllocator());

			rapidjson::Value key3("beatitude", d.GetAllocator());
			rapidjson::Value val3(beatitude);
			outObject.AddMember(key3, val3, d.GetAllocator());

			rapidjson::Value key4("count", d.GetAllocator());
			rapidjson::Value val4(count);
			outObject.AddMember(key4, val4, d.GetAllocator());

			rapidjson::Value key5("appearance", d.GetAllocator());
			rapidjson::Value val5(appearance);
			outObject.AddMember(key5, val5, d.GetAllocator());

			rapidjson::Value key6("identified", d.GetAllocator());
			rapidjson::Value val6(identified);
			outObject.AddMember(key6, val6, d.GetAllocator());
		}
		bool readKeyToItemEntry(rapidjson::Value::ConstMemberIterator& itr)
		{
			std::string name = itr->name.GetString();
			if ( name.compare("type") == 0 )
			{
				std::string itemName = itr->value.GetString();
				for ( int i = 0; i < NUMITEMS; ++i )
				{
					if ( itemName.compare(itemNameStrings[i + 2]) == 0 )
					{
						this->type = static_cast<ItemType>(i);
						return true;
					}
				}
			}
			else if ( name.compare("status") == 0 )
			{
				std::string status = itr->value.GetString();
				for ( Uint32 i = 0; i < itemStatusStrings.size(); ++i )
				{
					if ( status.compare(itemStatusStrings.at(i)) == 0 )
					{
						this->status = static_cast<Status>(i);
						return true;
					}
				}
			}
			else if ( name.compare("beatitude") == 0 )
			{
				this->beatitude = static_cast<Sint16>(itr->value.GetInt());
				return true;
			}
			else if ( name.compare("count") == 0 )
			{
				this->count = static_cast<Sint16>(itr->value.GetInt());
				return true;
			}
			else if ( name.compare("appearance") == 0 )
			{
				this->appearance = static_cast<Uint32>(itr->value.GetInt());
				return true;
			}
			else if ( name.compare("identified") == 0 )
			{
				this->identified = itr->value.GetBool();
				return true;
			}
			else if ( name.compare("percent_chance") == 0 )
			{
				this->percentChance = itr->value.GetInt();
				return true;
			}
			return false;
		}
	};

	class StatEntry
	{
		std::mt19937 StatEntrySeed;
	public:
		char name[128] = "";
		int type = NOTHING;
		sex_t sex = sex_t::MALE;
		Uint32 appearance = 0;
		Sint32 HP = 10;
		Sint32 MAXHP = 10;
		Sint32 OLDHP = 10;
		Sint32 MP = 10;
		Sint32 MAXMP = 10;
		Sint32 STR = 0;
		Sint32 DEX = 0;
		Sint32 CON = 0;
		Sint32 INT = 0;
		Sint32 PER = 0;
		Sint32 CHR = 0;
		Sint32 EXP = 0;
		Sint32 LVL = 0;
		Sint32 GOLD = 0;
		Sint32 HUNGER = 0;
		Sint32 RANDOM_STR = 0;
		Sint32 RANDOM_DEX = 0;
		Sint32 RANDOM_CON = 0;
		Sint32 RANDOM_INT = 0;
		Sint32 RANDOM_PER = 0;
		Sint32 RANDOM_CHR = 0;
		Sint32 RANDOM_MAXHP = 0;
		Sint32 RANDOM_HP = 0;
		Sint32 RANDOM_MAXMP = 0;
		Sint32 RANDOM_MP = 0;
		Sint32 RANDOM_LVL = 0;
		Sint32 RANDOM_GOLD = 0;

		Sint32 PROFICIENCIES[NUMPROFICIENCIES];

		std::vector<std::pair<ItemEntry, int>> equipped_items;
		std::vector<ItemEntry> inventory_items;
		std::vector<std::pair<std::string, int>> followerVariants;
		int numFollowers = 0;
		bool useDefaultEquipment = true;
		bool useDefaultInventoryItems = true;
		bool disableMiniboss = true;
		bool forceFriendlyToPlayer = false;
		bool forceEnemyToPlayer = false;
		bool disableItemDrops = false;

		StatEntry(const Stat* myStats) :
			StatEntrySeed(rand())
		{
			readFromStats(myStats);
		}
		StatEntry() :
			StatEntrySeed(rand())
		{
			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				PROFICIENCIES[i] = 0;
			}
		};

		std::string getFollowerVariant()
		{
			if ( followerVariants.size() > 0 )
			{
				std::vector<int> variantChances(followerVariants.size(), 0);
				int index = 0;
				for ( auto& pair : followerVariants )
				{
					variantChances.at(index) = pair.second;
					++index;
				}

				std::discrete_distribution<> variantWeightedDistribution(variantChances.begin(), variantChances.end());
				int result = variantWeightedDistribution(StatEntrySeed);
				return followerVariants.at(result).first;
			}
			return "none";
		}

		void readFromStats(const Stat* myStats)
		{
			strcpy(name, myStats->name);
			type = myStats->type;
			sex = myStats->sex;
			appearance = myStats->appearance;
			HP = myStats->HP;
			MAXHP = myStats->MAXHP;
			OLDHP = HP;
			MP = myStats->MP;
			MAXMP = myStats->MAXMP;
			STR = myStats->STR;
			DEX = myStats->DEX;
			CON = myStats->CON;
			INT = myStats->INT;
			PER = myStats->PER;
			CHR = myStats->CHR;
			EXP = myStats->EXP;
			LVL = myStats->LVL;
			GOLD = myStats->GOLD;

			RANDOM_STR = myStats->RANDOM_STR;
			RANDOM_DEX = myStats->RANDOM_DEX;
			RANDOM_CON = myStats->RANDOM_CON;
			RANDOM_INT = myStats->RANDOM_INT;
			RANDOM_PER = myStats->RANDOM_PER;
			RANDOM_CHR = myStats->RANDOM_CHR;
			RANDOM_MAXHP = myStats->RANDOM_MAXHP;
			RANDOM_HP = myStats->RANDOM_HP;
			RANDOM_MAXMP = myStats->RANDOM_MAXMP;
			RANDOM_MP = myStats->RANDOM_MP;
			RANDOM_LVL = myStats->RANDOM_LVL;
			RANDOM_GOLD = myStats->RANDOM_GOLD;

			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				PROFICIENCIES[i] = 0;
			}
			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				PROFICIENCIES[i] = myStats->PROFICIENCIES[i];
			}
		}

		void setStats(Stat* myStats)
		{
			strcpy(myStats->name, name);
			myStats->type = static_cast<Monster>(type);
			myStats->sex = static_cast<sex_t>(sex);
			myStats->appearance = appearance;
			myStats->HP = HP;
			myStats->MAXHP = MAXHP;
			myStats->OLDHP = myStats->HP;
			myStats->MP = MP;
			myStats->MAXMP = MAXMP;
			myStats->STR = STR;
			myStats->DEX = DEX;
			myStats->CON = CON;
			myStats->INT = INT;
			myStats->PER = PER;
			myStats->CHR = CHR;
			myStats->EXP = EXP;
			myStats->LVL = LVL;
			myStats->GOLD = GOLD;

			myStats->RANDOM_STR = RANDOM_STR;
			myStats->RANDOM_DEX = RANDOM_DEX;
			myStats->RANDOM_CON = RANDOM_CON;
			myStats->RANDOM_INT = RANDOM_INT;
			myStats->RANDOM_PER = RANDOM_PER;
			myStats->RANDOM_CHR = RANDOM_CHR;
			myStats->RANDOM_MAXHP = RANDOM_MAXHP;
			myStats->RANDOM_HP = RANDOM_HP;
			myStats->RANDOM_MAXMP = RANDOM_MAXMP;
			myStats->RANDOM_MP = RANDOM_MP;
			myStats->RANDOM_LVL = RANDOM_LVL;
			myStats->RANDOM_GOLD = RANDOM_GOLD;

			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				myStats->PROFICIENCIES[i] = PROFICIENCIES[i];
			}
		}

		void setItems(Stat* myStats)
		{
			std::unordered_set<int> equippedSlots;
			for ( auto& it : equipped_items )
			{
				equippedSlots.insert(it.second);
				if ( it.first.percentChance < 100 )
				{
					if ( rand() % 100 <= it.first.percentChance )
					{
						continue;
					}
				}
				switch ( it.second )
				{
					case ITEM_SLOT_WEAPON:
						myStats->weapon = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_SHIELD:
						myStats->shield = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_HELM:
						myStats->helmet = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_ARMOR:
						myStats->breastplate = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_GLOVES:
						myStats->gloves = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_BOOTS:
						myStats->shoes = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_CLOAK:
						myStats->cloak = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_RING:
						myStats->ring = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_AMULET:
						myStats->amulet = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					case ITEM_SLOT_MASK:
						myStats->mask = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						break;
					default:
						break;
				}
			}
			for ( int equipSlots = 0; equipSlots < 10; ++equipSlots )
			{
				if ( !useDefaultEquipment )
				{
					// disable any default item slot spawning.
					myStats->EDITOR_ITEMS[equipSlots * ITEM_SLOT_NUMPROPERTIES] = 0;
				}
				else
				{
					if ( equippedSlots.find(equipSlots * ITEM_SLOT_NUMPROPERTIES) != equippedSlots.end() )
					{
						// disable item slots we (attempted) to fill in.
						myStats->EDITOR_ITEMS[equipSlots * ITEM_SLOT_NUMPROPERTIES] = 0;
					}
				}
			}
			for ( auto& it : inventory_items )
			{
				if ( it.percentChance < 100 )
				{
					if ( rand() % 100 <= it.percentChance )
					{
						continue;
					}
				}
				newItem(it.type, it.status, it.beatitude, it.count, it.appearance, it.identified, &myStats->inventory);
			}
			if ( !useDefaultInventoryItems )
			{
				for ( int invSlots = ITEM_SLOT_INV_1; invSlots <= ITEM_SLOT_INV_6; invSlots = invSlots + ITEM_SLOT_NUMPROPERTIES )
				{
					myStats->EDITOR_ITEMS[invSlots] = 0;
				}
			}
		}

		void setStatsAndEquipmentToMonster(Stat* myStats)
		{
			//myStats->clearStats();
			setStats(myStats);
			setItems(myStats);
			if ( disableMiniboss )
			{
				myStats->MISC_FLAGS[STAT_FLAG_DISABLE_MINIBOSS] = 1;
			}
			if ( forceFriendlyToPlayer )
			{
				myStats->MISC_FLAGS[STAT_FLAG_FORCE_ALLEGIANCE_TO_PLAYER] = 
					Stat::MonsterForceAllegiance::MONSTER_FORCE_PLAYER_ALLY;
			}
			if ( forceEnemyToPlayer )
			{
				myStats->MISC_FLAGS[STAT_FLAG_FORCE_ALLEGIANCE_TO_PLAYER] =
					Stat::MonsterForceAllegiance::MONSTER_FORCE_PLAYER_ENEMY;
			}
			if ( disableItemDrops )
			{
				myStats->MISC_FLAGS[STAT_FLAG_NO_DROP_ITEMS] = 1;
			}
		}

		void setStatsAndEquipmentToPlayer(Stat* myStats, int player)
		{
			//if ( player == 0 )
			//{
			//	TextSourceScript tmpScript;
			//	tmpScript.playerClearInventory(true);
			//}
			//else
			//{
			//	// other players
			//	myStats->freePlayerEquipment();
			//	myStats->clearStats();
			//	TextSourceScript tmpScript;
			//	tmpScript.updateClientInformation(player, true, true, TextSourceScript::CLIENT_UPDATE_ALL);
			//}
		}
	};

	void writeAllFromStats(Stat* myStats)
	{
		rapidjson::Document d;
		d.SetObject();
		rapidjson::Value version;
		version.SetInt(1);
		addMemberToRoot(d, "version", version);
		readAttributesFromStats(myStats, d);
		readItemsFromStats(myStats, d);
		
		// misc properties
		rapidjson::Value propsObject;
		propsObject.SetObject();
		addMemberToRoot(d, "properties", propsObject);
		addMemberToSubkey(d, "properties", "populate_empty_equipped_items_with_default", rapidjson::Value(true));
		addMemberToSubkey(d, "properties", "populate_default_inventory", rapidjson::Value(true));
		addMemberToSubkey(d, "properties", "disable_miniboss_chance", rapidjson::Value(false));
		addMemberToSubkey(d, "properties", "force_player_friendly", rapidjson::Value(false));
		addMemberToSubkey(d, "properties", "force_player_enemy", rapidjson::Value(false));
		addMemberToSubkey(d, "properties", "disable_item_drops", rapidjson::Value(false));

		// follower details
		rapidjson::Value followersObject;
		followersObject.SetObject();
		addMemberToRoot(d, "followers", followersObject);
		addMemberToSubkey(d, "followers", "num_followers", rapidjson::Value(0));
		rapidjson::Value followerVariantsObject;
		followerVariantsObject.SetObject();
		addMemberToSubkey(d, "followers", "follower_variants", followerVariantsObject);

		writeToFile(d, monstertypename[myStats->type]);
	}

	void readItemsFromStats(Stat* myStats, rapidjson::Document& d)
	{
		rapidjson::Value equippedItemsObject;
		equippedItemsObject.SetObject();
		addMemberToRoot(d, "equipped_items", equippedItemsObject);
		addMemberFromItem(d, "equipped_items", "weapon", myStats->weapon);
		addMemberFromItem(d, "equipped_items", "shield", myStats->shield);
		addMemberFromItem(d, "equipped_items", "helmet", myStats->helmet);
		addMemberFromItem(d, "equipped_items", "breastplate", myStats->breastplate);
		addMemberFromItem(d, "equipped_items", "gloves", myStats->gloves);
		addMemberFromItem(d, "equipped_items", "shoes", myStats->shoes);
		addMemberFromItem(d, "equipped_items", "cloak", myStats->cloak);
		addMemberFromItem(d, "equipped_items", "ring", myStats->ring);
		addMemberFromItem(d, "equipped_items", "amulet", myStats->amulet);
		addMemberFromItem(d, "equipped_items", "mask", myStats->mask);

		rapidjson::Value invItemsArray;
		invItemsArray.SetArray();
		addMemberToRoot(d, "inventory_items", invItemsArray);
		for ( node_t* node = myStats->inventory.first; node; node = node->next )
		{
			Item* item = (Item*)node->element;
			if ( item )
			{
				addArrayMemberFromItem(d, "inventory_items", item);
			}
		}
	}

	void readAttributesFromStats(Stat* myStats, rapidjson::Document& d)
	{
		rapidjson::Value statsObject;
		statsObject.SetObject();
		addMemberToRoot(d, "stats", statsObject);

		StatEntry statEntry(myStats);
		addMemberToSubkey(d, "stats", "name", rapidjson::Value(statEntry.name, d.GetAllocator()));
		addMemberToSubkey(d, "stats", "type", rapidjson::Value(monstertypename[statEntry.type], d.GetAllocator()));
		addMemberToSubkey(d, "stats", "sex", rapidjson::Value(statEntry.sex));
		addMemberToSubkey(d, "stats", "appearance", rapidjson::Value(statEntry.appearance));
		addMemberToSubkey(d, "stats", "HP", rapidjson::Value(statEntry.HP));
		addMemberToSubkey(d, "stats", "MAXHP", rapidjson::Value(statEntry.MAXHP));
		addMemberToSubkey(d, "stats", "MP", rapidjson::Value(statEntry.MP));
		addMemberToSubkey(d, "stats", "MAXMP", rapidjson::Value(statEntry.MAXMP));
		addMemberToSubkey(d, "stats", "STR", rapidjson::Value(statEntry.STR));
		addMemberToSubkey(d, "stats", "DEX", rapidjson::Value(statEntry.DEX));
		addMemberToSubkey(d, "stats", "CON", rapidjson::Value(statEntry.CON));
		addMemberToSubkey(d, "stats", "INT", rapidjson::Value(statEntry.INT));
		addMemberToSubkey(d, "stats", "PER", rapidjson::Value(statEntry.PER));
		addMemberToSubkey(d, "stats", "CHR", rapidjson::Value(statEntry.CHR));
		addMemberToSubkey(d, "stats", "EXP", rapidjson::Value(statEntry.EXP));
		addMemberToSubkey(d, "stats", "LVL", rapidjson::Value(statEntry.LVL));
		addMemberToSubkey(d, "stats", "GOLD", rapidjson::Value(statEntry.GOLD));

		rapidjson::Value miscStatsObject;
		miscStatsObject.SetObject();
		addMemberToRoot(d, "misc_stats", miscStatsObject);

		addMemberToSubkey(d, "misc_stats", "RANDOM_STR", rapidjson::Value(statEntry.RANDOM_STR));
		addMemberToSubkey(d, "misc_stats", "RANDOM_DEX", rapidjson::Value(statEntry.RANDOM_DEX));
		addMemberToSubkey(d, "misc_stats", "RANDOM_CON", rapidjson::Value(statEntry.RANDOM_CON));
		addMemberToSubkey(d, "misc_stats", "RANDOM_INT", rapidjson::Value(statEntry.RANDOM_INT));
		addMemberToSubkey(d, "misc_stats", "RANDOM_PER", rapidjson::Value(statEntry.RANDOM_PER));
		addMemberToSubkey(d, "misc_stats", "RANDOM_CHR", rapidjson::Value(statEntry.RANDOM_CHR));
		addMemberToSubkey(d, "misc_stats", "RANDOM_MAXHP", rapidjson::Value(statEntry.RANDOM_MAXHP));
		addMemberToSubkey(d, "misc_stats", "RANDOM_HP", rapidjson::Value(statEntry.RANDOM_HP));
		addMemberToSubkey(d, "misc_stats", "RANDOM_MAXMP", rapidjson::Value(statEntry.RANDOM_MAXMP));
		addMemberToSubkey(d, "misc_stats", "RANDOM_MP", rapidjson::Value(statEntry.RANDOM_MP));
		addMemberToSubkey(d, "misc_stats", "RANDOM_LVL", rapidjson::Value(statEntry.RANDOM_LVL));
		addMemberToSubkey(d, "misc_stats", "RANDOM_GOLD", rapidjson::Value(statEntry.RANDOM_GOLD));

		rapidjson::Value profObject;
		profObject.SetObject();
		addMemberToRoot(d, "proficiencies", profObject);

		for ( int i = 0; i < NUMPROFICIENCIES; ++i )
		{
			addMemberToSubkey(d, "proficiencies", getSkillLangEntry(i), rapidjson::Value(statEntry.PROFICIENCIES[i]));
		}
	}

	bool readKeyToStatEntry(StatEntry& statEntry, rapidjson::Value::ConstMemberIterator& itr)
	{
		std::string name = itr->name.GetString();
		if ( name.compare("name") == 0 )
		{
			strcpy(statEntry.name, itr->value.GetString());
			return true;
		}
		else if ( name.compare("type") == 0 )
		{
			std::string val = itr->value.GetString();
			for ( int i = 0; i < NUMMONSTERS; ++i )
			{
				if ( val.compare(monstertypename[i]) == 0 )
				{
					statEntry.type = i;
				}
			}
			return true;
		}
		else if ( name.compare("sex") == 0 )
		{
			statEntry.sex = static_cast<sex_t>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("appearance") == 0 )
		{
			statEntry.appearance = static_cast<Uint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("HP") == 0 )
		{
			statEntry.HP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("MAXHP") == 0 )
		{
			statEntry.MAXHP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("MP") == 0 )
		{
			statEntry.MP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("MAXMP") == 0 )
		{
			statEntry.MAXMP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("STR") == 0 )
		{
			statEntry.STR = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("DEX") == 0 )
		{
			statEntry.DEX = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("CON") == 0 )
		{
			statEntry.CON = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("INT") == 0 )
		{
			statEntry.INT = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("PER") == 0 )
		{
			statEntry.PER = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("CHR") == 0 )
		{
			statEntry.CHR = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("EXP") == 0 )
		{
			statEntry.EXP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("LVL") == 0 )
		{
			statEntry.LVL = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("GOLD") == 0 )
		{
			statEntry.GOLD = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_STR") == 0 )
		{
			statEntry.RANDOM_STR = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_DEX") == 0 )
		{
			statEntry.RANDOM_DEX = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_CON") == 0 )
		{
			statEntry.RANDOM_CON = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_INT") == 0 )
		{
			statEntry.RANDOM_INT = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_PER") == 0 )
		{
			statEntry.RANDOM_PER = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_CHR") == 0 )
		{
			statEntry.RANDOM_CHR = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_MAXHP") == 0 )
		{
			statEntry.RANDOM_MAXHP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_HP") == 0 )
		{
			statEntry.RANDOM_HP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_MAXMP") == 0 )
		{
			statEntry.RANDOM_MAXMP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_MP") == 0 )
		{
			statEntry.RANDOM_MP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_LVL") == 0 )
		{
			statEntry.RANDOM_LVL = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_GOLD") == 0 )
		{
			statEntry.RANDOM_GOLD = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else
		{
			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				if ( name.compare(getSkillLangEntry(i)) == 0 )
				{
					statEntry.PROFICIENCIES[i] = static_cast<Sint32>(itr->value.GetInt());
					return true;
				}
			}
		}
		return false;
	}

	void addMemberFromItem(rapidjson::Document& d, std::string rootKey, std::string key, Item* item)
	{
		if ( item )
		{
			rapidjson::Value itemObject(rapidjson::kObjectType);
			ItemEntry itemEntry(*item);
			itemEntry.setValueFromAttributes(d, itemObject);
			addMemberToSubkey(d, rootKey, key.c_str(), itemObject);
		}
	}
	void addMemberToSubkey(rapidjson::Document& d, std::string subkey, std::string name, rapidjson::Value& value)
	{
		rapidjson::Value key(name.c_str(), d.GetAllocator()); // copy string name
		rapidjson::Value val(value, d.GetAllocator());
		d[subkey.c_str()].AddMember(key, val, d.GetAllocator());
	}
	void addMemberToRoot(rapidjson::Document& d, std::string name, rapidjson::Value& value)
	{
		rapidjson::Value key(name.c_str(), d.GetAllocator()); // copy string name
		rapidjson::Value val(value, d.GetAllocator());
		d.AddMember(key, val, d.GetAllocator());
	}

	void addArrayMemberFromItem(rapidjson::Document& d, std::string rootKey, Item* item)
	{
		if ( item )
		{
			rapidjson::Value itemObject(rapidjson::kObjectType);
			ItemEntry itemEntry(*item);
			itemEntry.setValueFromAttributes(d, itemObject);
			addArrayMemberToSubkey(d, rootKey, itemObject);
		}
	}
	void addArrayMemberToSubkey(rapidjson::Document& d, std::string subkey, rapidjson::Value& value)
	{
		rapidjson::Value val(value, d.GetAllocator());        // some value
		d[subkey.c_str()].PushBack(val, d.GetAllocator());
	}

	void writeToFile(rapidjson::Document& d, std::string monsterFileName)
	{
		int filenum = 0;
		std::string testPath = "/data/monster_" + monsterFileName + "_export" + std::to_string(filenum) + ".json";
		while ( PHYSFS_getRealDir(testPath.c_str()) != nullptr && filenum < 1000 )
		{
			++filenum;
			testPath = "/data/monster_" + monsterFileName + "_export" + std::to_string(filenum) + ".json";
		}
		std::string outputPath = "." + testPath;

		FILE* fp = fopen(outputPath.c_str(), "wb");
		if ( !fp )
		{
			return;
		}
		char buf[65536];
		rapidjson::FileWriteStream os(fp, buf, sizeof(buf));
		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
		d.Accept(writer);

		fclose(fp);
	}

	StatEntry* readFromFile(std::string monsterFileName)
	{
		std::string filePath = "/data/";
		filePath.append(monsterFileName);
		if ( filePath.find(".json") == std::string::npos )
		{
			filePath.append(".json");
		}
		if ( PHYSFS_getRealDir(filePath.c_str()) )
		{
			std::string inputPath = PHYSFS_getRealDir(filePath.c_str());
			inputPath.append(filePath);

			FILE* fp = fopen(inputPath.c_str(), "rb");
			if ( !fp )
			{
				return nullptr;
			}
			char buf[65536];
			rapidjson::FileReadStream is(fp, buf, sizeof(buf));
			fclose(fp);

			rapidjson::Document d;
			d.ParseStream(is);

			StatEntry* statEntry = new StatEntry();

			if ( !d.HasMember("version") )
			{
				printlog("[JSON]: Error no versioning value in json file %s", inputPath.c_str());
				return nullptr;
			}
			int version = d["version"].GetInt();
			const rapidjson::Value& stats = d["stats"];
			for ( rapidjson::Value::ConstMemberIterator stat_itr = stats.MemberBegin(); stat_itr != stats.MemberEnd(); ++stat_itr )
			{
				readKeyToStatEntry(*statEntry, stat_itr);
			}
			const rapidjson::Value& miscStats = d["misc_stats"];
			for ( rapidjson::Value::ConstMemberIterator stat_itr = miscStats.MemberBegin(); stat_itr != miscStats.MemberEnd(); ++stat_itr )
			{
				readKeyToStatEntry(*statEntry, stat_itr);
			}
			const rapidjson::Value& proficiencies = d["proficiencies"];
			for ( rapidjson::Value::ConstMemberIterator stat_itr = proficiencies.MemberBegin(); stat_itr != proficiencies.MemberEnd(); ++stat_itr )
			{
				readKeyToStatEntry(*statEntry, stat_itr);
			}
			const rapidjson::Value& equipped_items = d["equipped_items"];
			for ( rapidjson::Value::ConstMemberIterator itemSlot_itr = equipped_items.MemberBegin(); itemSlot_itr != equipped_items.MemberEnd(); ++itemSlot_itr )
			{
				std::string slotName = itemSlot_itr->name.GetString();
				if ( itemSlot_itr->value.MemberCount() > 0 )
				{
					ItemEntry item;
					for ( rapidjson::Value::ConstMemberIterator item_itr = itemSlot_itr->value.MemberBegin(); item_itr != itemSlot_itr->value.MemberEnd(); ++item_itr )
					{
						item.readKeyToItemEntry(item_itr);
					}
					statEntry->equipped_items.push_back(std::make_pair(item, getSlotFromKeyName(slotName)));
				}
			}
			const rapidjson::Value& inventory_items = d["inventory_items"];
			for ( rapidjson::Value::ConstValueIterator itemSlot_itr = inventory_items.Begin(); itemSlot_itr != inventory_items.End(); ++itemSlot_itr )
			{
				ItemEntry item;
				for ( rapidjson::Value::ConstMemberIterator item_itr = itemSlot_itr->MemberBegin(); item_itr != itemSlot_itr->MemberEnd(); ++item_itr )
				{
					item.readKeyToItemEntry(item_itr);
				}
				statEntry->inventory_items.push_back(item);
			}
			if ( d.HasMember("followers") )
			{
				const rapidjson::Value& numFollowersVal = d["followers"]["num_followers"];
				statEntry->numFollowers = numFollowersVal.GetInt();
				const rapidjson::Value& followers = d["followers"]["follower_variants"];

				statEntry->followerVariants.clear();
				for ( rapidjson::Value::ConstMemberIterator follower_itr = followers.MemberBegin(); follower_itr != followers.MemberEnd(); ++follower_itr )
				{
					statEntry->followerVariants.push_back(std::make_pair(follower_itr->name.GetString(), follower_itr->value.GetInt()));
				}
			}
			if ( d.HasMember("properties") )
			{
				if ( d["properties"].HasMember("populate_empty_equipped_items_with_default") )
				{
					statEntry->useDefaultEquipment = d["properties"]["populate_empty_equipped_items_with_default"].GetBool();
				}
				if ( d["properties"].HasMember("populate_default_inventory") )
				{
					statEntry->useDefaultInventoryItems = d["properties"]["populate_default_inventory"].GetBool();
				}
				if ( d["properties"].HasMember("disable_miniboss_chance") )
				{
					statEntry->disableMiniboss = d["properties"]["disable_miniboss_chance"].GetBool();
				}
				if ( d["properties"].HasMember("force_player_friendly") )
				{
					statEntry->forceFriendlyToPlayer = d["properties"]["force_player_friendly"].GetBool();
				}
				if ( d["properties"].HasMember("force_player_enemy") )
				{
					statEntry->forceEnemyToPlayer = d["properties"]["force_player_enemy"].GetBool();
				}
				if ( d["properties"].HasMember("disable_item_drops") )
				{
					statEntry->disableItemDrops = d["properties"]["disable_item_drops"].GetBool();
				}
			}
			printlog("[JSON]: Successfully read json file %s", inputPath.c_str());
			return statEntry;
		}
		return nullptr;
	}
};
extern MonsterStatCustomManager monsterStatCustomManager;

class MonsterCurveCustomManager
{
public:
	std::mt19937 curveSeed;
	MonsterCurveCustomManager() :
		curveSeed(rand())
	{};

	class MonsterCurveEntry
	{
	public:
		int monsterType = NOTHING;
		int levelmin = 0;
		int levelmax = 99;
		int chance = 1;
		int fallbackMonsterType = NOTHING;
		std::vector<std::pair<std::string, int>> variants;
		MonsterCurveEntry(std::string monsterStr, int levelNumMin, int levelNumMax, int chanceNum, std::string fallbackMonsterStr)
		{
			monsterType = getMonsterTypeFromString(monsterStr);
			fallbackMonsterType = getMonsterTypeFromString(fallbackMonsterStr);
			levelmin = levelNumMin;
			levelmax = levelNumMax;
			chance = chanceNum;
		};
		void addVariant(std::string variantName, int chance)
		{
			variants.push_back(std::make_pair(variantName, chance));
		}
	};

	class LevelCurve
	{
	public:
		std::string mapName = "";
		std::vector<MonsterCurveEntry> monsterCurve;
	};

	std::vector<LevelCurve> allLevelCurves;

	void readFromFile()
	{
		allLevelCurves.clear();
		if ( PHYSFS_getRealDir("/data/monstercurve.json") )
		{
			std::string inputPath = PHYSFS_getRealDir("/data/monstercurve.json");
			inputPath.append("/data/monstercurve.json");

			FILE* fp = fopen(inputPath.c_str(), "rb");
			if ( !fp )
			{
				return;
			}
			char buf[65536];
			rapidjson::FileReadStream is(fp, buf, sizeof(buf));
			fclose(fp);

			rapidjson::Document d;
			d.ParseStream(is);
			if ( !d.HasMember("version") )
			{
				printlog("[JSON]: Error no versioning value in json file %s", inputPath.c_str());
				return;
			}
			int version = d["version"].GetInt();

			if ( d.HasMember("levels") )
			{
				const rapidjson::Value& levels = d["levels"];
				for ( rapidjson::Value::ConstValueIterator level_itr = levels.Begin(); level_itr != levels.End(); ++level_itr )
				{
					const rapidjson::Value& attribute = *level_itr;
					for ( rapidjson::Value::ConstMemberIterator map_itr = attribute.MemberBegin(); map_itr != attribute.MemberEnd(); ++map_itr )
					{
						LevelCurve newCurve;
						newCurve.mapName = map_itr->name.GetString();
						for ( rapidjson::Value::ConstValueIterator monsters_itr = map_itr->value.Begin(); monsters_itr != map_itr->value.End(); ++monsters_itr )
						{
							const rapidjson::Value& monster = *monsters_itr;

							MonsterCurveEntry newMonster(monster["name"].GetString(), 
								monster["dungeon_depth_minimum"].GetInt(), 
								monster["dungeon_depth_maximum"].GetInt(),
								monster["chance"].GetInt(), 
								"");

							if ( monster.HasMember("variants") )
							{
								for ( rapidjson::Value::ConstMemberIterator var_itr = monster["variants"].MemberBegin();
									var_itr != monster["variants"].MemberEnd(); ++var_itr )
								{
									newMonster.addVariant(var_itr->name.GetString(), var_itr->value.GetInt());
								}
							}
							newCurve.monsterCurve.push_back(newMonster);
						}
						allLevelCurves.push_back(newCurve);
					}
				}
			}
			printCurve(allLevelCurves);
			printlog("[JSON]: Successfully read json file %s", inputPath.c_str());
		}
	}

	static int getMonsterTypeFromString(std::string monsterStr)
	{
		if ( monsterStr.compare("") == 0 )
		{
			return NOTHING;
		}
		for ( int i = NOTHING; i < NUMMONSTERS; ++i )
		{
			if ( monsterStr.compare(monstertypename[i]) == 0 )
			{
				return i;
			}
		}
		return NOTHING;
	}
	void printCurve(std::vector<LevelCurve> toPrint)
	{
		return;
		for ( LevelCurve curve : toPrint )
		{
			printlog("Map Name: %s", curve.mapName.c_str());
			for ( MonsterCurveEntry monsters : curve.monsterCurve )
			{
				printlog("[MonsterCurveCustomManager]: Monster: %s | lvl: %d-%d | chance: %d | fallback type: %s", monstertypename[monsters.monsterType],
					monsters.levelmin, monsters.levelmax, monsters.chance, monstertypename[monsters.fallbackMonsterType]);
			}
		}
	}
	bool curveExistsForCurrentMapName(std::string currentMap)
	{
		if ( currentMap.compare("") == 0 )
		{
			return false;
		}
		for ( LevelCurve curve : allLevelCurves )
		{
			if ( curve.mapName.compare(currentMap) == 0 )
			{
				//printlog("[MonsterCurveCustomManager]: curveExistsForCurrentMapName: true");
				return true;
			}
		}
		return false;
	}
	int rollMonsterFromCurve(std::string currentMap)
	{
		std::vector<int> monsterCurveChances(NUMMONSTERS, 0);

		for ( LevelCurve curve : allLevelCurves )
		{
			if ( curve.mapName.compare(currentMap) == 0 )
			{
				for ( MonsterCurveEntry& monster : curve.monsterCurve )
				{
					if ( currentlevel >= monster.levelmin && currentlevel <= monster.levelmax )
					{
						if ( monster.monsterType != NOTHING )
						{
							monsterCurveChances[monster.monsterType] += monster.chance;
						}
					}
					else
					{
						if ( monster.fallbackMonsterType != NOTHING )
						{
							monsterCurveChances[monster.fallbackMonsterType] += monster.chance;
						}
					}
				}
				std::discrete_distribution<> monsterWeightedDistribution(monsterCurveChances.begin(), monsterCurveChances.end());
				int result = monsterWeightedDistribution(curveSeed);
				//printlog("[MonsterCurveCustomManager]: Rolled: %d", result);
				return result;
			}
		}
		printlog("[MonsterCurveCustomManager]: Error: default to skeleton.");
		return SKELETON;
	}
	std::string rollMonsterVariant(std::string currentMap, int monsterType)
	{
		for ( LevelCurve curve : allLevelCurves )
		{
			if ( curve.mapName.compare(currentMap) == 0 )
			{
				for ( MonsterCurveEntry& monster : curve.monsterCurve )
				{
					if ( monster.monsterType == monsterType && monster.variants.size() > 0 )
					{
						std::vector<int> variantChances(monster.variants.size(), 0);
						int index = 0;
						for ( auto& pair : monster.variants )
						{
							variantChances.at(index) = pair.second;
							++index;
						}

						std::discrete_distribution<> variantWeightedDistribution(variantChances.begin(), variantChances.end());
						int result = variantWeightedDistribution(curveSeed);
						return monster.variants.at(result).first;
					}
				}
			}
		}
		return "default";
	}
};
extern MonsterCurveCustomManager monsterCurveCustomManager;