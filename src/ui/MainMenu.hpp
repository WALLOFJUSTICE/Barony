#pragma once

class Button;

namespace MainMenu {
	// Here be new menu options:
	extern float master_volume; // range is [0 - 100]
	extern bool arachnophobia_filter; // if true, all spiders are crabs
	extern bool vertical_splitscreen; // if true, 2-player splitscreen has a vertical rather than horizontal layout

	struct InventorySorting {
		bool hotbarWeapons = true;
		bool hotbarArmor = true;
		bool hotbarAmulets = true;
		bool hotbarBooks = true;
		bool hotbarTools = true;
		bool hotbarThrown = true;
		bool hotbarGems = false;
		bool hotbarPotions = true;
		bool hotbarScrolls = true;
		bool hotbarStaves = true;
		bool hotbarFood = true;
		bool hotbarSpells = true;
		int sortWeapons = 0;
		int sortArmor = 0;
		int sortAmulets = 0;
		int sortBooks = 0;
		int sortTools = 0;
		int sortThrown = 0;
		int sortGems = 0;
		int sortPotions = 0;
		int sortScrolls = 0;
		int sortStaves = 0;
		int sortFood = 0;
		int sortEquipped = 0;
		inline void save();
		static inline InventorySorting load();
		static inline InventorySorting reset();
	};

	struct AllSettings {
		bool add_items_to_hotbar_enabled;
		InventorySorting inventory_sorting;
		bool use_on_release_enabled;
		//Whatever minimap;
		bool show_messages_enabled;
		//Whatever show_messages;
		bool show_player_nametags_enabled;
		bool show_hud_enabled;
		bool show_ip_address_enabled;
		bool content_control_enabled;
		bool colorblind_mode_enabled;
		bool arachnophobia_filter_enabled;
		bool shaking_enabled;
		bool bobbing_enabled;
		bool light_flicker_enabled;
		int resolution_x;
		int resolution_y;
		bool vsync_enabled;
		bool vertical_split_enabled;
		float gamma;
		float fov;
		float fps;
		float master_volume;
		float gameplay_volume;
		float ambient_volume;
		float environment_volume;
		float music_volume;
		bool minimap_pings_enabled;
		bool player_monster_sounds_enabled;
		bool out_of_focus_audio_enabled;
		//Whatever bindings;
		bool numkeys_in_inventory_enabled;
		float mouse_sensitivity;
		bool reverse_mouse_enabled;
		bool smooth_mouse_enabled;
		bool rotation_speed_limit_enabled;
		float turn_sensitivity_x;
		float turn_sensitivity_y;
		bool classic_mode_enabled;
		bool hardcore_mode_enabled;
		bool friendly_fire_enabled;
		bool keep_inventory_enabled;
		bool hunger_enabled;
		bool minotaur_enabled;
		bool random_traps_enabled;
		bool extra_life_enabled;
		bool cheats_enabled;
	};

	void settingsUI(Button&);
	void settingsVideo(Button&);
	void settingsAudio(Button&);
	void settingsControls(Button&);
	void settingsGame(Button&);

	void recordsAdventureArchives(Button&);
	void recordsLeaderboards(Button&);
	void recordsDungeonCompendium(Button&);
	void recordsStoryIntroduction(Button&);
	void recordsCredits(Button&);
	void recordsBackToMainMenu(Button&);

	void playNew(Button&);
	void playContinue(Button&);

	void mainPlayGame(Button&);
	void mainPlayModdedGame(Button&);
	void mainHallOfRecords(Button&);
	void mainSettings(Button&);
	void mainQuit(Button&);

	void doMainMenu();
	void createMainMenu();
	void destroyMainMenu();
	void createDummyMainMenu();

	enum class LobbyType {
		LobbyLocal,
		LobbyHosted,
		LobbyJoined
	};

	void characterCardGameSettingsMenu(int index);
	void characterCardLobbySettingsMenu(int index);
	void characterCardRaceMenu(int index);
	void characterCardClassMenu(int index);
	void createCharacterCard(int index);
	void createStartButton(int index);
	void createInviteButton(int index);

	void createLobby(LobbyType);
	void createLobbyBrowser();
}