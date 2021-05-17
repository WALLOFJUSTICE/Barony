#include "MainMenu.hpp"
#include "Frame.hpp"
#include "Image.hpp"
#include "Field.hpp"
#include "Button.hpp"
#include "Slider.hpp"
#include "Text.hpp"

#include "../net.hpp"
#include "../player.hpp"
#include "../menu.hpp"
#include "../interface/interface.hpp"
#include "../draw.hpp"
#include "../engine/audio/sound.hpp"

#include <cassert>

// ALL NEW menu options:
bool arachnophobia_filter = false;
bool vertical_splitscreen = false;
float master_volume = 100.f;

enum class FadeDestination : Uint8 {
	None = 0,
	RootMainMenu = 1,
	IntroStoryScreen = 2,
};

static Frame* main_menu_frame = nullptr;
static int main_menu_buttons_height = 0;
static Uint32 main_menu_ticks = 0u;
static float main_menu_cursor_bob = 0.f;
static int main_menu_cursor_x = 0;
static int main_menu_cursor_y = 0;
static FadeDestination main_menu_fade_destination = FadeDestination::None;

static const char* bigfont_outline = "fonts/pixelmix.ttf#18#2";
static const char* bigfont_no_outline = "fonts/pixelmix.ttf#18#0";
static const char* smallfont_outline = "fonts/pixel_maz.ttf#32#2";
static const char* smallfont_no_outline = "fonts/pixel_maz.ttf#32#2";
static const char* menu_option_font = "fonts/pixel_maz.ttf#48#2";

static void updateMenuCursor(Widget& widget) {
	Frame* buttons = static_cast<Frame*>(&widget);
	for (auto button : buttons->getButtons()) {
		if (button->isSelected()) {
			main_menu_cursor_x = button->getSize().x - 80;
			main_menu_cursor_y = button->getSize().y - 9 + buttons->getSize().y;
		}
	}

	// bob cursor
	const float bobrate = (float)PI * 2.f / (float)TICKS_PER_SECOND;
	main_menu_cursor_bob += bobrate;
	if (main_menu_cursor_bob >= (float)PI * 2.f) {
		main_menu_cursor_bob -= (float)PI * 2.f;
	}

	// update cursor position
	auto cursor = main_menu_frame->findImage("cursor");
	if (cursor) {
		int diff = main_menu_cursor_y - cursor->pos.y;
		if (diff > 0) {
			diff = std::max(1, diff / 2);
		} else if (diff < 0) {
			diff = std::min(-1, diff / 2);
		}
		cursor->pos = SDL_Rect{
			main_menu_cursor_x + (int)(sinf(main_menu_cursor_bob) * 16.f) - 16,
			diff + cursor->pos.y,
			37 * 2,
			23 * 2
		};
	}
}

/******************************************************************************/

static int story_text_pause = 0;
static int story_text_scroll = 0;
static int story_text_section = 0;
static bool story_text_end = false;

static const char* intro_text =
u8"Long ago, the bustling town of Hamlet was the envy of all its neighbors,\nfor it was the most thriving city in all the land.#"
u8"Its prosperity was unmatched for generations until the evil Baron Herx came\nto power.#"
u8"The Baron, in his endless greed, forced the people to dig the hills for gold,\nthough the ground had never given such treasure before.#"
u8"Straining under the yoke of their master, the people planned his demise.\nThey tricked him with a promise of gold and sealed him within the mines.#"
u8"Free of their cruel master, the people returned to their old way of life.\nBut disasters shortly began to befall the village.#"
u8"Monsters and other evils erupted from the ground, transforming the village\ninto a ghost town.#"
u8"Many adventurers have descended into the mines to break the Baron's curse,\nbut none have returned.#"
u8"The town of Hamlet cries for redemption, and only a hero can save it\nfrom its curse...";

static void createStoryScreen() {
	main_menu_frame->addImage(
		main_menu_frame->getSize(),
		0xffffffff,
		"images/ui/Main Menus/Story/intro1.png",
		"backdrop"
	);

	story_text_pause = 0;
	story_text_scroll = 0;
	story_text_section = 0;
	story_text_end = false;

	auto back_button = main_menu_frame->addButton("back");
	back_button->setText("Skip story  ");
	back_button->setColor(makeColor(0, 0, 0, 0));
	back_button->setHighlightColor(makeColor(0, 0, 0, 0));
	back_button->setBorderColor(makeColor(0, 0, 0, 0));
	back_button->setTextColor(0xffffffff);
	back_button->setTextHighlightColor(0xffffffff);
	back_button->setFont(smallfont_outline);
	back_button->setJustify(Button::justify_t::RIGHT);
	back_button->setSize(SDL_Rect{Frame::virtualScreenX - 400, Frame::virtualScreenY - 50, 400, 50});
	back_button->setCallback([](Button& b){
		fadeout = true;
		main_menu_fade_destination = FadeDestination::RootMainMenu;
		});
	back_button->setWidgetBack("back");
	back_button->select();

	auto font = Font::get(bigfont_outline); assert(font);

	auto textbox1 = main_menu_frame->addFrame("story_text_box");
	textbox1->setSize(SDL_Rect{100, Frame::virtualScreenY - font->height() * 4, Frame::virtualScreenX - 200, font->height() * 3});
	textbox1->setActualSize(SDL_Rect{0, 0, textbox1->getSize().w, textbox1->getSize().h});
	textbox1->setColor(makeColor(0, 0, 0, 127));
	textbox1->setBorder(0);

	auto textbox2 = textbox1->addFrame("story_text_box");
	textbox2->setScrollBarsEnabled(false);
	textbox2->setSize(SDL_Rect{0, font->height() / 2, Frame::virtualScreenX - 200, font->height() * 2});
	textbox2->setActualSize(SDL_Rect{0, 0, textbox2->getSize().w, font->height() * 16});
	textbox2->setHollow(true);
	textbox2->setBorder(0);

	auto field = textbox2->addField("text", 1024);
	field->setFont(bigfont_outline);
	field->setSize(textbox2->getActualSize());
	field->setHJustify(Field::justify_t::CENTER);
	field->setVJustify(Field::justify_t::TOP);
	field->setColor(makeColor(255, 255, 255, 255));

	textbox1->setTickCallback([](Widget& widget){
		auto textbox1 = static_cast<Frame*>(&widget);
		auto story_font = Font::get(bigfont_outline); assert(story_font);
		if (story_text_scroll > 0) {
			auto textbox2 = textbox1->findFrame("story_text_box");
			assert(textbox2);
			auto size = textbox2->getActualSize();
			++size.y;
			textbox2->setActualSize(size);
			--story_text_scroll;
			if (story_text_section % 2 == 0) {
				auto backdrop = main_menu_frame->findImage("backdrop");
				if (backdrop) {
					Uint8 c = 255 * (fabs(story_text_scroll - story_font->height()) / story_font->height());
					backdrop->color = makeColor(c, c, c, 255);
					if (c == 0) {
						char c = backdrop->path[backdrop->path.size() - 5];
						backdrop->path[backdrop->path.size() - 5] = c + 1;
					}
				}
			}
		} else {
			if (story_text_pause > 0) {
				--story_text_pause;
				if (story_text_pause == 0) {
					if (story_text_end == true) {
						fadeout = true;
						main_menu_fade_destination = FadeDestination::RootMainMenu;
					} else {
						story_text_scroll = story_font->height() * 2;
					}
				}
			} else {
				if (main_menu_ticks % 2 == 0) {
					auto textbox2 = textbox1->findFrame("story_text_box");
					assert(textbox2);
					auto text = textbox2->findField("text");
					assert(text);
					size_t len = strlen(text->getText());
					if (len < strlen(intro_text)) {
						char buf[1024] = { '\0' };
						strcpy(buf, text->getText());
						char c = intro_text[len];
						if (c == '#') {
							++story_text_section;
							story_text_pause = TICKS_PER_SECOND * 5;
							c = '\n';
						}
						buf[len] = c;
						buf[len + 1] = '\0';
						text->setText(buf);
					} else {
						story_text_pause = TICKS_PER_SECOND * 5;
						story_text_end = true;
					}
				}
			}
		}
		});
}

/******************************************************************************/

static std::string settings_tab_name;
static AllSettings allSettings;

struct Setting {
	enum class Type : Uint8 {
		Boolean = 0,
		Slider = 1,
		Customize = 2,
		BooleanWithCustomize = 3,
		Dropdown = 4,
	};
	Type type;
	const char* name;
};

static void settingsSave() {
	auto_hotbar_new_items = allSettings.add_items_to_hotbar_enabled;
	right_click_protect = !allSettings.use_on_release_enabled;
	disable_messages = !allSettings.show_messages_enabled;
	hide_playertags = !allSettings.show_player_nametags_enabled;
	nohud = !allSettings.show_hud_enabled;
	broadcast = !allSettings.show_ip_address_enabled;
	spawn_blood = !allSettings.content_control_enabled;
	colorblind = allSettings.colorblind_mode_enabled;
	arachnophobia_filter = allSettings.arachnophobia_filter_enabled;
	shaking = allSettings.shaking_enabled;
	bobbing = allSettings.bobbing_enabled;
	flickerLights = allSettings.light_flicker_enabled;
	xres = allSettings.resolution_x;
	yres = allSettings.resolution_y;
	verticalSync = allSettings.vsync_enabled;
	vertical_splitscreen = allSettings.vertical_split_enabled;
	vidgamma = allSettings.gamma / 100.f;
	fov = allSettings.fov;
	fpsLimit = allSettings.fps;
	master_volume = allSettings.master_volume;
	sfxvolume = (allSettings.gameplay_volume / 100.f) * 128.f;
	sfxAmbientVolume = (allSettings.ambient_volume / 100.f) * 128.f;
	sfxEnvironmentVolume = (allSettings.environment_volume / 100.f) * 128.f;
	musvolume = (allSettings.music_volume / 100.f) * 128.f;
	minimapPingMute = !allSettings.minimap_pings_enabled;
	mute_player_monster_sounds = !allSettings.player_monster_sounds_enabled;
	mute_audio_on_focus_lost = !allSettings.out_of_focus_audio_enabled;
	hotbar_numkey_quick_add = allSettings.numkeys_in_inventory_enabled;
	mousespeed = allSettings.mouse_sensitivity;
	reversemouse = allSettings.reverse_mouse_enabled;
	smoothmouse = allSettings.smooth_mouse_enabled;
	disablemouserotationlimit = !allSettings.rotation_speed_limit_enabled;
	gamepad_rightx_sensitivity = allSettings.turn_sensitivity_x * 10.f;
	gamepad_righty_sensitivity = allSettings.turn_sensitivity_y * 10.f;
	svFlags = allSettings.classic_mode_enabled ? svFlags | SV_FLAG_CLASSIC : svFlags & ~(SV_FLAG_CLASSIC);
	svFlags = allSettings.hardcore_mode_enabled ? svFlags | SV_FLAG_HARDCORE : svFlags & ~(SV_FLAG_HARDCORE);
	svFlags = allSettings.friendly_fire_enabled ? svFlags | SV_FLAG_FRIENDLYFIRE : svFlags & ~(SV_FLAG_FRIENDLYFIRE);
	svFlags = allSettings.keep_inventory_enabled ? svFlags | SV_FLAG_KEEPINVENTORY : svFlags & ~(SV_FLAG_KEEPINVENTORY);
	svFlags = allSettings.hunger_enabled ? svFlags | SV_FLAG_HUNGER : svFlags & ~(SV_FLAG_HUNGER);
	svFlags = allSettings.minotaur_enabled ? svFlags | SV_FLAG_MINOTAURS : svFlags & ~(SV_FLAG_MINOTAURS);
	svFlags = allSettings.random_traps_enabled ? svFlags | SV_FLAG_TRAPS : svFlags & ~(SV_FLAG_TRAPS);
	svFlags = allSettings.extra_life_enabled ? svFlags | SV_FLAG_LIFESAVING : svFlags & ~(SV_FLAG_LIFESAVING);
	svFlags = allSettings.cheats_enabled ? svFlags | SV_FLAG_CHEATS : svFlags & ~(SV_FLAG_CHEATS);
	saveConfig("default.cfg");
}

static void settingsReset() {
	allSettings.add_items_to_hotbar_enabled = true;
	allSettings.use_on_release_enabled = true;
	allSettings.show_messages_enabled = true;
	allSettings.show_player_nametags_enabled = true;
	allSettings.show_hud_enabled = true;
	allSettings.show_ip_address_enabled = true;
	allSettings.content_control_enabled = false;
	allSettings.colorblind_mode_enabled = false;
	allSettings.arachnophobia_filter_enabled = false;
	allSettings.shaking_enabled = true;
	allSettings.bobbing_enabled = true;
	allSettings.light_flicker_enabled = true;
	allSettings.resolution_x = 1280;
	allSettings.resolution_y = 720;
	allSettings.vsync_enabled = true;
	allSettings.vertical_split_enabled = false;
	allSettings.gamma = 100.f;
	allSettings.fov = 65;
	allSettings.fps = 60;
	allSettings.master_volume = 100.f;
	allSettings.gameplay_volume = 100.f;
	allSettings.ambient_volume = 100.f;
	allSettings.environment_volume = 100.f;
	allSettings.music_volume = 100.f;
	allSettings.minimap_pings_enabled = true;
	allSettings.player_monster_sounds_enabled = true;
	allSettings.out_of_focus_audio_enabled = true;
	allSettings.numkeys_in_inventory_enabled = true;
	allSettings.mouse_sensitivity = 32.f;
	allSettings.reverse_mouse_enabled = false;
	allSettings.smooth_mouse_enabled = false;
	allSettings.rotation_speed_limit_enabled = true;
	allSettings.turn_sensitivity_x = 50.f;
	allSettings.turn_sensitivity_y = 50.f;
	allSettings.classic_mode_enabled = false;
	allSettings.hardcore_mode_enabled = false;
	allSettings.friendly_fire_enabled = true;
	allSettings.keep_inventory_enabled = false;
	allSettings.hunger_enabled = true;
	allSettings.minotaur_enabled = true;
	allSettings.random_traps_enabled = true;
	allSettings.extra_life_enabled = false;
	allSettings.cheats_enabled = false;
}

static int settingsAddSubHeader(Frame& frame, int y, const char* name, const char* text) {
	std::string fullname = std::string("subheader_") + name;
	auto image = frame.addImage(
		SDL_Rect{0, y, 1080, 42},
		0xffffffff,
		"images/ui/Main Menus/Settings/Settings_SubHeading_Backing00.png",
		(fullname + "_image").c_str()
	);
	auto field = frame.addField((fullname + "_field").c_str(), 128);
	field->setSize(image->pos);
	field->setFont(bigfont_outline);
	field->setText(text);
	field->setJustify(Field::justify_t::CENTER);
	Text* text_image = Text::get(text, field->getFont());
	int w = text_image->getWidth();
	auto fleur_left = frame.addImage(
		SDL_Rect{ (1080 - w) / 2 - 26 - 8, y + 6, 26, 30 },
		0xffffffff,
		"images/ui/Main Menus/Settings/Settings_SubHeading_Fleur00.png",
		(fullname + "_fleur_left").c_str()
	);
	auto fleur_right = frame.addImage(
		SDL_Rect{ (1080 + w) / 2 + 8, y + 6, 26, 30 },
		0xffffffff,
		"images/ui/Main Menus/Settings/Settings_SubHeading_Fleur00.png",
		(fullname + "_fleur_right").c_str()
	);
	return image->pos.h + 6;
}

static int settingsAddOption(Frame& frame, int y, const char* name, const char* text, const char* tip) {
	std::string fullname = std::string("setting_") + name;
	auto image = frame.addImage(
		SDL_Rect{0, y, 382, 52},
		0xffffffff,
		"images/ui/Main Menus/Settings/Settings_Left_Backing00.png",
		(fullname + "_image").c_str()
	);
	auto field = frame.addField((fullname + "_field").c_str(), 128);
	auto size = image->pos; size.x += 24; size.w -= 24;
	field->setSize(size);
	field->setFont(bigfont_outline);
	field->setText(text);
	field->setHJustify(Field::justify_t::LEFT);
	field->setVJustify(Field::justify_t::CENTER);
	field->setGuide(tip);
	return size.h + 10;
}

static int settingsAddBooleanOption(
	Frame& frame,
	int y,
	const char* name,
	const char* text,
	const char* tip,
	bool on,
	void (*callback)(Button&))
{
	std::string fullname = std::string("setting_") + name;
	int result = settingsAddOption(frame, y, name, text, tip);
	auto button = frame.addButton((fullname + "_button").c_str());
	button->setSize(SDL_Rect{
		390,
		y + 2,
		158,
		48});
	button->setFont(smallfont_outline);
	button->setText("Off      On");
	button->setJustify(Button::justify_t::CENTER);
	button->setCallback(callback);
	button->setPressed(on);
	button->setBackground("images/ui/Main Menus/Settings/Settings_SwitchOff00.png");
	button->setBackgroundActivated("images/ui/Main Menus/Settings/Settings_SwitchOn00.png");
	button->setStyle(Button::style_t::STYLE_TOGGLE);
	button->setHighlightColor(makeColor(255,255,255,255));
	button->setColor(makeColor(127,127,127,255));
	button->setTextHighlightColor(makeColor(255,255,255,255));
	button->setTextColor(makeColor(127,127,127,255));
	button->setWidgetBack("discard_and_exit");
	button->setWidgetPageLeft("tab_left");
	button->setWidgetPageRight("tab_right");
	button->addWidgetAction("MenuAlt1", "restore_defaults");
	button->addWidgetAction("MenuStart", "confirm_and_exit");
	return result;
}

static int settingsAddBooleanWithCustomizeOption(
	Frame& frame,
	int y,
	const char* name,
	const char* text,
	const char* tip,
	bool on,
	void (*callback)(Button&),
	void (*customize_callback)(Button&))
{
	std::string fullname = std::string("setting_") + name;
	int result = settingsAddBooleanOption(frame, y, name, text, tip, on, callback);
	auto button = frame.addButton((fullname + "_customize_button").c_str());
	button->setSize(SDL_Rect{
		574,
		y + 4,
		158,
		44});
	button->setFont(smallfont_outline);
	button->setText("Customize");
	button->setJustify(Button::justify_t::CENTER);
	button->setCallback(customize_callback);
	button->setBackground("images/ui/Main Menus/Settings/Settings_Button_Customize00.png");
	button->setHighlightColor(makeColor(255,255,255,255));
	button->setColor(makeColor(127,127,127,255));
	button->setTextHighlightColor(makeColor(255,255,255,255));
	button->setTextColor(makeColor(127,127,127,255));
	button->setWidgetLeft((fullname + "_button").c_str());
	button->setWidgetBack("discard_and_exit");
	button->setWidgetPageLeft("tab_left");
	button->setWidgetPageRight("tab_right");
	button->addWidgetAction("MenuAlt1", "restore_defaults");
	button->addWidgetAction("MenuStart", "confirm_and_exit");
	auto boolean = frame.findButton((fullname + "_button").c_str()); assert(boolean);
	boolean->setWidgetRight((fullname + "_customize_button").c_str());
	boolean->setWidgetBack("discard_and_exit");
	boolean->setWidgetPageLeft("tab_left");
	boolean->setWidgetPageRight("tab_right");
	boolean->addWidgetAction("MenuAlt1", "restore_defaults");
	boolean->addWidgetAction("MenuStart", "confirm_and_exit");
	return result;
}

static int settingsAddCustomize(
	Frame& frame,
	int y,
	const char* name,
	const char* text,
	const char* tip,
	void (*callback)(Button&))
{
	std::string fullname = std::string("setting_") + name;
	int result = settingsAddOption(frame, y, name, text, tip);
	auto button = frame.addButton((fullname + "_customize_button").c_str());
	button->setSize(SDL_Rect{
		390,
		y + 4,
		158,
		44});
	button->setFont(smallfont_outline);
	button->setText("Customize");
	button->setJustify(Button::justify_t::CENTER);
	button->setCallback(callback);
	button->setBackground("images/ui/Main Menus/Settings/Settings_Button_Customize00.png");
	button->setHighlightColor(makeColor(255,255,255,255));
	button->setColor(makeColor(127,127,127,255));
	button->setTextHighlightColor(makeColor(255,255,255,255));
	button->setTextColor(makeColor(127,127,127,255));
	button->setWidgetBack("discard_and_exit");
	button->setWidgetPageLeft("tab_left");
	button->setWidgetPageRight("tab_right");
	button->addWidgetAction("MenuAlt1", "restore_defaults");
	button->addWidgetAction("MenuStart", "confirm_and_exit");
	return result;
}

static int settingsAddDropdown(
	Frame& frame,
	int y,
	const char* name,
	const char* text,
	const char* tip,
	const std::vector<const char*>& items,
	void (*callback)(Button&))
{
	std::string fullname = std::string("setting_") + name;
	int result = settingsAddOption(frame, y, name, text, tip);
	auto button = frame.addButton((fullname + "_dropdown").c_str());
	button->setSize(SDL_Rect{
		390,
		y + 4,
		158,
		44});
	button->setFont(smallfont_outline);
	button->setText(items[0]);
	button->setJustify(Button::justify_t::CENTER);
	button->setCallback(callback);
	button->setBackground("images/ui/Main Menus/Settings/Settings_Button_Customize00.png");
	button->setHighlightColor(makeColor(255,255,255,255));
	button->setColor(makeColor(127,127,127,255));
	button->setTextHighlightColor(makeColor(255,255,255,255));
	button->setTextColor(makeColor(127,127,127,255));
	button->setWidgetBack("discard_and_exit");
	button->setWidgetPageLeft("tab_left");
	button->setWidgetPageRight("tab_right");
	button->addWidgetAction("MenuAlt1", "restore_defaults");
	button->addWidgetAction("MenuStart", "confirm_and_exit");
	return result;
}

static int settingsAddSlider(
	Frame& frame,
	int y,
	const char* name,
	const char* text,
	const char* tip,
	float value,
	float minValue,
	float maxValue,
	bool percent,
	void (*callback)(Slider&))
{
	std::string fullname = std::string("setting_") + name;
	int result = settingsAddOption(frame, y, name, text, tip);
	auto box = frame.addImage(
		SDL_Rect{402, y + 4, 132, 44},
		0xffffffff,
		"images/ui/Main Menus/Settings/Settings_Value_Backing00.png",
		(fullname + "_box").c_str()
	);
	auto field = frame.addField((fullname + "_text").c_str(), 8);
	field->setSize(box->pos);
	field->setJustify(Field::justify_t::CENTER);
	field->setFont(smallfont_outline);
	if (percent) {
		field->setTickCallback([](Widget& widget){
			auto field = static_cast<Field*>(&widget); assert(field);
			auto frame = static_cast<Frame*>(widget.getParent());
			auto name = std::string(widget.getName());
			auto setting = name.substr(sizeof("setting_") - 1, name.size() - (sizeof("_text") - 1) - (sizeof("setting_") - 1));
			auto slider = frame->findSlider((std::string("setting_") + setting + std::string("_slider")).c_str()); assert(slider);
			char buf[8]; snprintf(buf, sizeof(buf), "%d%%", (int)slider->getValue());
			field->setText(buf);
			});
	} else {
		field->setTickCallback([](Widget& widget){
			auto field = static_cast<Field*>(&widget); assert(field);
			auto frame = static_cast<Frame*>(widget.getParent());
			auto name = std::string(widget.getName());
			auto setting = name.substr(sizeof("setting_") - 1, name.size() - (sizeof("_text") - 1) - (sizeof("setting_") - 1));
			auto slider = frame->findSlider((std::string("setting_") + setting + std::string("_slider")).c_str()); assert(slider);
			char buf[8]; snprintf(buf, sizeof(buf), "%d", (int)slider->getValue());
			field->setText(buf);
			});
	}
	auto slider = frame.addSlider((fullname + "_slider").c_str());
	slider->setMinValue(minValue);
	slider->setMaxValue(maxValue);
	slider->setValue(value);
	slider->setRailSize(SDL_Rect{field->getSize().x + field->getSize().w + 32, y + 14, 450, 24});
	slider->setHandleSize(SDL_Rect{0, 0, 52, 42});
	slider->setCallback(callback);
	slider->setColor(makeColor(127,127,127,255));
	slider->setHighlightColor(makeColor(255,255,255,255));
	slider->setHandleImage("images/ui/Main Menus/Settings/Settings_ValueSlider_Slide00.png");
	slider->setRailImage("images/ui/Main Menus/Settings/Settings_ValueSlider_Backing00.png");
	slider->setWidgetBack("discard_and_exit");
	slider->setWidgetPageLeft("tab_left");
	slider->setWidgetPageRight("tab_right");
	slider->addWidgetAction("MenuAlt1", "restore_defaults");
	slider->addWidgetAction("MenuStart", "confirm_and_exit");
	if (callback) {
		(*callback)(*slider);
	}
	return result;
}

static Frame* settingsSubwindowSetup(Button& button) {
	if (settings_tab_name == button.getName()) {
		return nullptr;
	}
	playSound(139, 64); // click sound
	settings_tab_name = button.getName();

	assert(main_menu_frame);
	auto settings = main_menu_frame->findFrame("settings"); assert(settings);
	auto settings_subwindow = settings->findFrame("settings_subwindow");
	if (settings_subwindow) {
		settings_subwindow->removeSelf();
	}
	settings_subwindow = settings->addFrame("settings_subwindow");
	settings_subwindow->setScrollBarsEnabled(true);
	settings_subwindow->setSize(SDL_Rect{17, 71 * 2, 547 * 2, 224 * 2});
	settings_subwindow->setActualSize(SDL_Rect{0, 0, 547 * 2, 224 * 2});
	settings_subwindow->setHollow(true);
	settings_subwindow->setBorder(0);
	settings_subwindow->setTickCallback([](Widget& widget){
		auto frame = static_cast<Frame*>(&widget);
		auto& images = frame->getImages();
		for (auto image : images) {
			if (image->path == "images/ui/Main Menus/Settings/Settings_Left_BackingSelect00.png") {
				image->path = "images/ui/Main Menus/Settings/Settings_Left_Backing00.png";
			}
		}
		auto selectedWidget = widget.findSelectedWidget();
		if (selectedWidget) {
			std::string setting;
			auto name = std::string(selectedWidget->getName());
			if (selectedWidget->getType() == Widget::WIDGET_SLIDER) {
				setting = name.substr(sizeof("setting_") - 1, name.size() - (sizeof("_slider") - 1) - (sizeof("setting_") - 1));
			} else if (selectedWidget->getType() == Widget::WIDGET_BUTTON) {
				auto button = static_cast<Button*>(selectedWidget);
				auto customize = "images/ui/Main Menus/Settings/Settings_Button_Customize00.png";
				if (strcmp(button->getBackground(), customize) == 0) {
					setting = name.substr(sizeof("setting_") - 1, name.size() - (sizeof("_customize_button") - 1) - (sizeof("setting_") - 1));
				} else {
					setting = name.substr(sizeof("setting_") - 1, name.size() - (sizeof("_button") - 1) - (sizeof("setting_") - 1));
				}
			}
			if (!setting.empty()) {
				auto image = frame->findImage((std::string("setting_") + setting + std::string("_image")).c_str());
				if (image) {
					image->path = "images/ui/Main Menus/Settings/Settings_Left_BackingSelect00.png";
				}
				auto field = frame->findField((std::string("setting_") + setting + std::string("_field")).c_str());
				if (field) {
					auto settings = static_cast<Frame*>(frame->getParent());
					auto tooltip = settings->findField("tooltip"); assert(tooltip);
					tooltip->setText(field->getGuide());
				}
			}
		}
		});
	auto rock_background = settings_subwindow->addImage(
		settings_subwindow->getActualSize(),
		makeColor(127, 127, 127, 251),
		"images/ui/Main Menus/Settings/Settings_BGTile00.png",
		"background"
	);
	rock_background->tiled = true;
	auto slider = settings_subwindow->addSlider("scroll_slider");
	slider->setOrientation(Slider::SLIDER_VERTICAL);
	slider->setRailSize(SDL_Rect{1038, 16, 30, 440});
	slider->setRailImage("images/ui/Main Menus/Settings/Settings_Slider_Backing00.png");
	slider->setHandleSize(SDL_Rect{0, 0, 34, 34});
	slider->setHandleImage("images/ui/Main Menus/Settings/Settings_Slider_Boulder00.png");
	slider->setCallback([](Slider& slider){
		Frame* frame = static_cast<Frame*>(slider.getParent());
		auto actualSize = frame->getActualSize();
		actualSize.y = slider.getValue();
		frame->setActualSize(actualSize);
		auto railSize = slider.getRailSize();
		railSize.y = 16 + actualSize.y;
		slider.setRailSize(railSize);
		});
	slider->setTickCallback([](Widget& widget){
		Slider* slider = static_cast<Slider*>(&widget);
		Frame* frame = static_cast<Frame*>(slider->getParent());
		auto actualSize = frame->getActualSize();
		slider->setValue(actualSize.y);
		auto railSize = slider->getRailSize();
		railSize.y = 16 + actualSize.y;
		slider->setRailSize(railSize);
		});
	return settings_subwindow;
}

static std::pair<std::string, std::string> getFullSettingNames(const Setting& setting) {
	switch (setting.type) {
	case Setting::Type::Boolean:
		return std::make_pair(
			std::string("setting_") + std::string(setting.name) + std::string("_button"),
			std::string(""));
	case Setting::Type::Slider:
		return std::make_pair(
			std::string("setting_") + std::string(setting.name) + std::string("_slider"),
			std::string(""));
	case Setting::Type::Customize:
		return std::make_pair(
			std::string("setting_") + std::string(setting.name) + std::string("_customize_button"),
			std::string(""));
	case Setting::Type::BooleanWithCustomize:
		return std::make_pair(
			std::string("setting_") + std::string(setting.name) + std::string("_button"),
			std::string("setting_") + std::string(setting.name) + std::string("_customize_button"));
	default:
		assert(0 && "Unknown setting type!");
		return std::make_pair(
			std::string(""),
			std::string(""));
	}
}

static void settingsSelect(Frame& frame, const Setting& setting) {
	auto names = getFullSettingNames(setting);
	auto widget = frame.findWidget(names.first.c_str(), false); assert(widget);
	widget->select();
}

static void settingsSubwindowFinalize(Frame& frame, int y) {
	const int height = std::max(224 * 2, y);
	frame.setActualSize(SDL_Rect{0, 0, 547 * 2, height});
	auto rock_background = frame.findImage("background"); assert(rock_background);
	rock_background->pos = frame.getActualSize();
	auto slider = frame.findSlider("scroll_slider"); assert(slider);
	slider->setValue(0.f);
	slider->setMinValue(0.f);
	slider->setMaxValue(height - 224 * 2);
}

static void hookSettingToSetting(Frame& frame, const Setting& setting1, const Setting& setting2) {
	auto names1 = getFullSettingNames(setting1);
	auto names2 = getFullSettingNames(setting2);
	auto widget11 = frame.findWidget(names1.first.c_str(), false); assert(widget11);
	auto widget12 = names1.second.empty() ? nullptr : frame.findWidget(names1.second.c_str(), false);
	auto widget21 = frame.findWidget(names2.first.c_str(), false); assert(widget21);
	auto widget22 = names2.second.empty() ? nullptr : frame.findWidget(names2.second.c_str(), false);
	widget11->setWidgetDown(names2.first.c_str());
	widget21->setWidgetUp(names1.first.c_str());
	if (widget12) {
		if (widget22) {
			widget12->setWidgetDown(names2.second.c_str());
			widget22->setWidgetUp(names1.second.c_str());
		} else {
			widget12->setWidgetDown(names2.first.c_str());
		}
	} else {
		if (widget22) {
			widget22->setWidgetUp(names1.first.c_str());
		}
	}
}

static void hookSettings(Frame& frame, const std::vector<Setting>& settings) {
	for (auto it = settings.begin(); std::next(it) != settings.end(); ++it) {
		auto& setting1 = (*it);
		auto& setting2 = (*std::next(it));
		hookSettingToSetting(frame, setting1, setting2);
	}
}

void settingsUI(Button& button) {
	Frame* settings_subwindow;
	if ((settings_subwindow = settingsSubwindowSetup(button)) == nullptr) {
		settingsSelect(*settings_subwindow, {Setting::Type::BooleanWithCustomize, "add_items_to_hotbar"});
		return;
	}
	int y = 0;

	y += settingsAddSubHeader(*settings_subwindow, y, "inventory", "Inventory Options");
	y += settingsAddBooleanWithCustomizeOption(*settings_subwindow, y, "add_items_to_hotbar", "Add Items to Hotbar",
		"Automatically fill the hotbar with recently collected items.",
		allSettings.add_items_to_hotbar_enabled, [](Button& button){allSettings.add_items_to_hotbar_enabled = button.isPressed();},
		nullptr);
	y += settingsAddCustomize(*settings_subwindow, y, "inventory_sorting", "Inventory Sorting",
		"Customize the way items are automatically sorted in your inventory.",
		nullptr);
#ifndef NINTENDO
	y += settingsAddBooleanOption(*settings_subwindow, y, "use_on_release", "Use on Release",
		"Activate an item as soon as the Use key is released in the inventory window.",
		allSettings.use_on_release_enabled, [](Button& button){allSettings.use_on_release_enabled = button.isPressed();});
#endif

	y += settingsAddSubHeader(*settings_subwindow, y, "hud", "HUD Options");
	y += settingsAddCustomize(*settings_subwindow, y, "minimap_settings", "Minimap Settings",
		"Customize the appearance of the in-game minimap.",
		nullptr);
	y += settingsAddBooleanWithCustomizeOption(*settings_subwindow, y, "show_messages", "Show Messages",
		"Customize which messages will be logged to the player, if any.",
		allSettings.show_messages_enabled, [](Button& button){allSettings.show_messages_enabled = button.isPressed();},
		nullptr);
	y += settingsAddBooleanOption(*settings_subwindow, y, "show_player_nametags", "Show Player Nametags",
		"Display the name of each player character above their avatar.",
		allSettings.show_player_nametags_enabled, [](Button& button){allSettings.show_player_nametags_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "show_hud", "Show HUD",
		"Toggle the display of health and other status bars in game when the inventory is closed.",
		allSettings.show_hud_enabled, [](Button& button){allSettings.show_hud_enabled = button.isPressed();});
#ifndef NINTENDO
	y += settingsAddBooleanOption(*settings_subwindow, y, "show_ip_address", "Show IP Address",
		"Hide the display of IP addresses and other location data for privacy purposes.",
		allSettings.show_ip_address_enabled, [](Button& button){allSettings.show_ip_address_enabled = button.isPressed();});
#endif

#ifndef NINTENDO
	hookSettings(*settings_subwindow,
		{{Setting::Type::BooleanWithCustomize, "add_items_to_hotbar"},
		{Setting::Type::Customize, "inventory_sorting"},
		{Setting::Type::Boolean, "use_on_release"},
		{Setting::Type::Customize, "minimap_settings"},
		{Setting::Type::BooleanWithCustomize, "show_messages"},
		{Setting::Type::Boolean, "show_player_nametags"},
		{Setting::Type::Boolean, "show_hud"},
		{Setting::Type::Boolean, "show_ip_address"}});
#else
	hookSettings(*settings_subwindow,
		{{Setting::Type::BooleanWithCustomize, "add_items_to_hotbar"},
		{Setting::Type::Customize, "inventory_sorting"},
		{Setting::Type::Customize, "minimap_settings"},
		{Setting::Type::BooleanWithCustomize, "show_messages"},
		{Setting::Type::Boolean, "show_player_nametags"},
		{Setting::Type::Boolean, "show_hud"}});
#endif

	settingsSubwindowFinalize(*settings_subwindow, y);
	settingsSelect(*settings_subwindow, {Setting::Type::BooleanWithCustomize, "add_items_to_hotbar"});
}

void settingsVideo(Button& button) {
	Frame* settings_subwindow;
	if ((settings_subwindow = settingsSubwindowSetup(button)) == nullptr) {
		settingsSelect(*settings_subwindow, {Setting::Type::Boolean, "content_control"});
		return;
	}
	int y = 0;

	y += settingsAddSubHeader(*settings_subwindow, y, "accessibility", "Accessibility");
	y += settingsAddBooleanOption(*settings_subwindow, y, "content_control", "Content Control",
		"Disable the appearance of blood and other explicit kinds of content in the game",
		allSettings.content_control_enabled, [](Button& button){allSettings.content_control_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "colorblind_mode", "Colorblind Mode",
		"Change the appearance of certain UI elements to improve visibility for certain colorblind individuals.",
		allSettings.colorblind_mode_enabled, [](Button& button){allSettings.colorblind_mode_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "arachnophobia_filter", "Arachnophobia Filter",
		"Replace all giant spiders in the game with hostile crustaceans.",
		allSettings.arachnophobia_filter_enabled, [](Button& button){allSettings.arachnophobia_filter_enabled = button.isPressed();});

	y += settingsAddSubHeader(*settings_subwindow, y, "effects", "Effects");
	y += settingsAddBooleanOption(*settings_subwindow, y, "shaking", "Shaking",
		"Toggle the camera's ability to twist and roll when the player stumbles or receives damage.",
		allSettings.shaking_enabled, [](Button& button){allSettings.shaking_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "bobbing", "Bobbing",
		"Toggle the camera's ability to bob steadily as the player moves.",
		allSettings.bobbing_enabled, [](Button& button){allSettings.bobbing_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "light_flicker", "Light Flicker",
		"Toggle the flickering appearance of torches and other light fixtures in the game world.",
		allSettings.light_flicker_enabled, [](Button& button){allSettings.light_flicker_enabled = button.isPressed();});

	y += settingsAddSubHeader(*settings_subwindow, y, "display", "Display");
#ifndef NINTENDO
	y += settingsAddDropdown(*settings_subwindow, y, "resolution", "Resolution", "Change the current window resolution.",
		{"1280 x 720", "1920 x 1080"},
		nullptr);
	y += settingsAddDropdown(*settings_subwindow, y, "window_mode", "Window Mode", "Change the current display mode.",
		{"Fullscreen", "Borderless", "Windowed"},
		nullptr);
	y += settingsAddBooleanOption(*settings_subwindow, y, "vsync", "Vertical Sync",
		"Prevent screen-tearing by locking the game's refresh rate to the current display.",
		allSettings.vsync_enabled, [](Button& button){allSettings.vsync_enabled = button.isPressed();});
#endif
	y += settingsAddBooleanOption(*settings_subwindow, y, "vertical_split", "Vertical Splitscreen",
		"For splitscreen with two-players: divide the screen along a vertical line rather than a horizontal one.",
		allSettings.vertical_split_enabled, [](Button& button){allSettings.vertical_split_enabled = button.isPressed();});
	y += settingsAddSlider(*settings_subwindow, y, "gamma", "Gamma",
		"Adjust the brightness of the visuals in-game.",
		allSettings.gamma, 50, 200, true, [](Slider& slider){allSettings.gamma = slider.getValue();});
	y += settingsAddSlider(*settings_subwindow, y, "fov", "Field of View",
		"Adjust the vertical field-of-view of the in-game camera.",
		allSettings.fov, 40, 100, false, [](Slider& slider){allSettings.fov = slider.getValue();});
#ifndef NINTENDO
	y += settingsAddSlider(*settings_subwindow, y, "fps", "FPS limit",
		"Control the frame-rate limit of the game window.",
		allSettings.fps, 30, 300, false, [](Slider& slider){allSettings.fps = slider.getValue();});
#endif

#ifndef NINTENDO
	hookSettings(*settings_subwindow,
		{{Setting::Type::Boolean, "content_control"},
		{Setting::Type::Boolean, "colorblind_mode"},
		{Setting::Type::Boolean, "arachnophobia_filter"},
		{Setting::Type::Boolean, "shaking"},
		{Setting::Type::Boolean, "bobbing"},
		{Setting::Type::Boolean, "light_flicker"},
		{Setting::Type::Dropdown, "resolution"},
		{Setting::Type::Dropdown, "window_mode"},
		{Setting::Type::Boolean, "vsync"},
		{Setting::Type::Boolean, "vertical_split"},
		{Setting::Type::Slider, "gamma"},
		{Setting::Type::Slider, "fov"},
		{Setting::Type::Slider, "fps"}});
#else
	hookSettings(*settings_subwindow,
		{{Setting::Type::Boolean, "content_control"},
		{Setting::Type::Boolean, "colorblind_mode"},
		{Setting::Type::Boolean, "arachnophobia_filter"},
		{Setting::Type::Boolean, "shaking"},
		{Setting::Type::Boolean, "bobbing"},
		{Setting::Type::Boolean, "light_flicker"},
		{Setting::Type::Boolean, "vertical_split"},
		{Setting::Type::Slider, "gamma"},
		{Setting::Type::Slider, "fov"}});
#endif

	settingsSubwindowFinalize(*settings_subwindow, y);
	settingsSelect(*settings_subwindow, {Setting::Type::Boolean, "content_control"});
}

void settingsAudio(Button& button) {
	Frame* settings_subwindow;
	if ((settings_subwindow = settingsSubwindowSetup(button)) == nullptr) {
		settingsSelect(*settings_subwindow, {Setting::Type::Slider, "master_volume"});
		return;
	}
	int y = 0;

	y += settingsAddSubHeader(*settings_subwindow, y, "volume", "Volume");
	y += settingsAddSlider(*settings_subwindow, y, "master_volume", "Master Volume",
		"Adjust the volume of all sound sources equally.",
		allSettings.master_volume, 0, 100, true, [](Slider& slider){allSettings.master_volume = slider.getValue();});
	y += settingsAddSlider(*settings_subwindow, y, "gameplay_volume", "Gameplay Volume",
		"Adjust the volume of most game sound effects.",
		allSettings.gameplay_volume, 0, 100, true, [](Slider& slider){allSettings.gameplay_volume = slider.getValue();});
	y += settingsAddSlider(*settings_subwindow, y, "ambient_volume", "Ambient Volume",
		"Adjust the volume of ambient subterranean wind.",
		allSettings.ambient_volume, 0, 100, true, [](Slider& slider){allSettings.ambient_volume = slider.getValue();});
	y += settingsAddSlider(*settings_subwindow, y, "environment_volume", "Environment Volume",
		"Adjust the volume of flowing water and lava.",
		allSettings.environment_volume, 0, 100, true, [](Slider& slider){allSettings.environment_volume = slider.getValue();});
	y += settingsAddSlider(*settings_subwindow, y, "music_volume", "Music Volume",
		"Adjust the volume of the game's soundtrack.",
		allSettings.music_volume, 0, 100, true, [](Slider& slider){allSettings.music_volume = slider.getValue();});

	y += settingsAddSubHeader(*settings_subwindow, y, "options", "Options");
	y += settingsAddBooleanOption(*settings_subwindow, y, "minimap_pings", "Minimap Pings",
		"Toggle the ability to hear pings on the minimap",
		allSettings.minimap_pings_enabled, [](Button& button){allSettings.minimap_pings_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "player_monster_sounds", "Player Monster Sounds",
		"Toggle the chance to emit monstrous mumbles when playing a non-human character.",
		allSettings.player_monster_sounds_enabled, [](Button& button){allSettings.player_monster_sounds_enabled = button.isPressed();});
#ifndef NINTENDO
	y += settingsAddBooleanOption(*settings_subwindow, y, "out_of_focus_audio", "Out-of-Focus Audio",
		"Enable audio sources even when the game window is out-of-focus.",
		allSettings.out_of_focus_audio_enabled, [](Button& button){allSettings.out_of_focus_audio_enabled = button.isPressed();});
#endif

#ifndef NINTENDO
	hookSettings(*settings_subwindow,
		{{Setting::Type::Slider, "master_volume"},
		{Setting::Type::Slider, "gameplay_volume"},
		{Setting::Type::Slider, "ambient_volume"},
		{Setting::Type::Slider, "environment_volume"},
		{Setting::Type::Slider, "music_volume"},
		{Setting::Type::Boolean, "minimap_pings"},
		{Setting::Type::Boolean, "player_monster_sounds"},
		{Setting::Type::Boolean, "out_of_focus_audio"}});
#else
	hookSettings(*settings_subwindow,
		{{Setting::Type::Slider, "master_volume"},
		{Setting::Type::Slider, "gameplay_volume"},
		{Setting::Type::Slider, "ambient_volume"},
		{Setting::Type::Slider, "environment_volume"},
		{Setting::Type::Slider, "music_volume"},
		{Setting::Type::Boolean, "minimap_pings"},
		{Setting::Type::Boolean, "player_monster_sounds"}});
#endif

	settingsSubwindowFinalize(*settings_subwindow, y);
	settingsSelect(*settings_subwindow, {Setting::Type::Slider, "master_volume"});
}

void settingsControls(Button& button) {
	Frame* settings_subwindow;
	if ((settings_subwindow = settingsSubwindowSetup(button)) == nullptr) {
		settingsSelect(*settings_subwindow, {Setting::Type::Customize, "bindings"});
		return;
	}
	int y = 0;

#ifndef NINTENDO
	y += settingsAddSubHeader(*settings_subwindow, y, "general", "General Settings");
	y += settingsAddCustomize(*settings_subwindow, y, "bindings", "Bindings",
		"Modify controls for mouse, keyboard, gamepads, and other peripherals.",
		nullptr);

	y += settingsAddSubHeader(*settings_subwindow, y, "mouse_and_keyboard", "Mouse & Keyboard");
	y += settingsAddBooleanOption(*settings_subwindow, y, "numkeys_in_inventory", "Number Keys in Inventory",
		"Allow the player to bind inventory items to the hotbar using the number keys on their keyboard.",
		allSettings.numkeys_in_inventory_enabled, [](Button& button){allSettings.numkeys_in_inventory_enabled = button.isPressed();});
	y += settingsAddSlider(*settings_subwindow, y, "mouse_sensitivity", "Mouse Sensitivity",
		"Control the speed by which mouse movement affects camera movement.",
		allSettings.mouse_sensitivity, 0, 100, false, [](Slider& slider){allSettings.mouse_sensitivity = slider.getValue();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "reverse_mouse", "Reverse Mouse",
		"Reverse mouse up and down movement for controlling the orientation of the player.",
		allSettings.reverse_mouse_enabled, [](Button& button){allSettings.reverse_mouse_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "smooth_mouse", "Smooth Mouse",
		"Smooth the movement of the mouse over a few frames of input.",
		allSettings.smooth_mouse_enabled, [](Button& button){allSettings.smooth_mouse_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "rotation_speed_limit", "Rotation Speed Limit",
		"Limit how fast the player can rotate by moving the mouse.",
		allSettings.rotation_speed_limit_enabled, [](Button& button){allSettings.rotation_speed_limit_enabled = button.isPressed();});
#endif

#ifdef NINTENDO
	y += settingsAddSubHeader(*settings_subwindow, y, "gamepad", "Controller Settings");
	y += settingsAddCustomize(*settings_subwindow, y, "bindings", "Bindings",
		"Modify controller bindings.",
		nullptr);
#else
	y += settingsAddSubHeader(*settings_subwindow, y, "gamepad", "Gamepad Settings");
#endif
	y += settingsAddSlider(*settings_subwindow, y, "turn_sensitivity_x", "Turn Sensitivity X",
		"Affect the horizontal sensitivity of the control stick used for turning.",
		allSettings.turn_sensitivity_x, 0, 100, true, [](Slider& slider){allSettings.turn_sensitivity_x = slider.getValue();});
	y += settingsAddSlider(*settings_subwindow, y, "turn_sensitivity_y", "Turn Sensitivity Y",
		"Affect the vertical sensitivity of the control stick used for turning.",
		allSettings.turn_sensitivity_y, 0, 100, true, [](Slider& slider){allSettings.turn_sensitivity_y = slider.getValue();});

#ifndef NINTENDO
	hookSettings(*settings_subwindow,
		{{Setting::Type::Customize, "bindings"},
		{Setting::Type::Boolean, "numkeys_in_inventory"},
		{Setting::Type::Slider, "mouse_sensitivity"},
		{Setting::Type::Boolean, "reverse_mouse"},
		{Setting::Type::Boolean, "smooth_mouse"},
		{Setting::Type::Boolean, "rotation_speed_limit"},
		{Setting::Type::Slider, "turn_sensitivity_x"},
		{Setting::Type::Slider, "turn_sensitivity_y"}});
#else
	hookSettings(*settings_subwindow,
		{{Setting::Type::Customize, "bindings"},
		{Setting::Type::Slider, "turn_sensitivity_x"},
		{Setting::Type::Slider, "turn_sensitivity_y"}});
#endif

	settingsSubwindowFinalize(*settings_subwindow, y);
	settingsSelect(*settings_subwindow, {Setting::Type::Customize, "bindings"});
}

void settingsGame(Button& button) {
	Frame* settings_subwindow;
	if ((settings_subwindow = settingsSubwindowSetup(button)) == nullptr) {
		settingsSelect(*settings_subwindow, {Setting::Type::Boolean, "classic_mode"});
		return;
	}
	int y = 0;

	y += settingsAddSubHeader(*settings_subwindow, y, "game", "Game Settings");
	y += settingsAddBooleanOption(*settings_subwindow, y, "classic_mode", "Classic Mode",
		"Toggle this option to make the game end after the battle with Baron Herx.",
		allSettings.classic_mode_enabled, [](Button& button){allSettings.classic_mode_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "hardcore_mode", "Hardcore Mode",
		"Greatly increases the difficulty of all combat encounters.",
		allSettings.hardcore_mode_enabled, [](Button& button){allSettings.hardcore_mode_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "friendly_fire", "Friendly Fire",
		"Enable players to harm eachother and their allies.",
		allSettings.friendly_fire_enabled, [](Button& button){allSettings.friendly_fire_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "keep_inventory", "Keep Inventory after Death",
		"When a player dies, they retain their inventory when revived on the next level.",
		allSettings.keep_inventory_enabled, [](Button& button){allSettings.keep_inventory_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "hunger", "Hunger",
		"Toggle player hunger. When hunger is off, eating food heals the player directly.",
		allSettings.hunger_enabled, [](Button& button){allSettings.hunger_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "minotaur", "Minotaur",
		"Toggle the minotaur's ability to spawn on many levels after a certain amount of time.",
		allSettings.minotaur_enabled, [](Button& button){allSettings.minotaur_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "random_traps", "Random Traps",
		"Toggle the random placement of traps throughout each level.",
		allSettings.random_traps_enabled, [](Button& button){allSettings.random_traps_enabled = button.isPressed();});
	y += settingsAddBooleanOption(*settings_subwindow, y, "extra_life", "Extra Life",
		"Start the game with an Amulet of Life-saving, to prevent one death.",
		allSettings.extra_life_enabled, [](Button& button){allSettings.extra_life_enabled = button.isPressed();});
#ifndef NINTENDO
	y += settingsAddBooleanOption(*settings_subwindow, y, "cheats", "Cheats",
		"Toggle the ability to activate cheatcodes during gameplay.",
		allSettings.cheats_enabled, [](Button& button){allSettings.cheats_enabled = button.isPressed();});
#endif

#ifndef NINTENDO
	hookSettings(*settings_subwindow,
		{{Setting::Type::Boolean, "classic_mode"},
		{Setting::Type::Boolean, "hardcore_mode"},
		{Setting::Type::Boolean, "friendly_fire"},
		{Setting::Type::Boolean, "keep_inventory"},
		{Setting::Type::Boolean, "hunger"},
		{Setting::Type::Boolean, "minotaur"},
		{Setting::Type::Boolean, "random_traps"},
		{Setting::Type::Boolean, "extra_life"},
		{Setting::Type::Boolean, "cheats"}});
#else
	hookSettings(*settings_subwindow,
		{{Setting::Type::Boolean, "classic_mode"},
		{Setting::Type::Boolean, "hardcore_mode"},
		{Setting::Type::Boolean, "friendly_fire"},
		{Setting::Type::Boolean, "keep_inventory"},
		{Setting::Type::Boolean, "hunger"},
		{Setting::Type::Boolean, "minotaur"},
		{Setting::Type::Boolean, "random_traps"},
		{Setting::Type::Boolean, "extra_life"}});
#endif

	settingsSubwindowFinalize(*settings_subwindow, y);
	settingsSelect(*settings_subwindow, {Setting::Type::Boolean, "classic_mode"});
}

/******************************************************************************/

void recordsAdventureArchives(Button& button) {
	playSound(139, 64); // click sound
}

void recordsLeaderboards(Button& button) {
	playSound(139, 64); // click sound
}

void recordsDungeonCompendium(Button& button) {
	playSound(139, 64); // click sound
}

void recordsStoryIntroduction(Button& button) {
	playSound(139, 64); // click sound

	destroyMainMenu();
	main_menu_frame = gui->addFrame("main_menu");
	main_menu_frame->setSize(SDL_Rect{0, 0, Frame::virtualScreenX, Frame::virtualScreenY});
	main_menu_frame->setActualSize(SDL_Rect{0, 0, main_menu_frame->getSize().w, main_menu_frame->getSize().h});
	main_menu_frame->setHollow(true);
	main_menu_frame->setBorder(0);
	main_menu_frame->setTickCallback([](Widget&){++main_menu_ticks;});

	fadeout = true;
	main_menu_fade_destination = FadeDestination::IntroStoryScreen;
}

void recordsCredits(Button& button) {
	playSound(139, 64); // click sound

	destroyMainMenu();
	main_menu_frame = gui->addFrame("main_menu");
	main_menu_frame->setSize(SDL_Rect{0, 0, Frame::virtualScreenX, Frame::virtualScreenY});
	main_menu_frame->setActualSize(SDL_Rect{0, 0, main_menu_frame->getSize().w, main_menu_frame->getSize().h});
	main_menu_frame->setHollow(true);
	main_menu_frame->setBorder(0);
	main_menu_frame->setTickCallback([](Widget&){++main_menu_ticks;});

	auto back_button = main_menu_frame->addButton("back");
	back_button->setText("Return to Main Menu  ");
	back_button->setColor(makeColor(0, 0, 0, 0));
	back_button->setHighlightColor(makeColor(0, 0, 0, 0));
	back_button->setBorderColor(makeColor(0, 0, 0, 0));
	back_button->setTextColor(0xffffffff);
	back_button->setTextHighlightColor(0xffffffff);
	back_button->setFont(smallfont_outline);
	back_button->setJustify(Button::justify_t::RIGHT);
	back_button->setSize(SDL_Rect{Frame::virtualScreenX - 400, Frame::virtualScreenY - 50, 400, 50});
	back_button->setCallback([](Button& b){
		destroyMainMenu();
		createMainMenu();
		mainHallOfRecords(b);
		auto buttons = main_menu_frame->findFrame("buttons"); assert(buttons);
		auto credits = buttons->findButton("CREDITS"); assert(credits);
		credits->select();
		});
	back_button->setWidgetBack("back");
	back_button->select();

	auto font = Font::get(bigfont_outline); assert(font);

	auto credits = main_menu_frame->addFrame("credits");
	credits->setSize(SDL_Rect{0, 0, Frame::virtualScreenX, Frame::virtualScreenY});
	credits->setActualSize(SDL_Rect{0, 0, Frame::virtualScreenX, Frame::virtualScreenY + font->height() * 80});
	credits->setScrollBarsEnabled(false);
	credits->setHollow(true);
	credits->setBorder(0);
	credits->setTickCallback([](Widget& widget){
		auto credits = static_cast<Frame*>(&widget);
		auto size = credits->getActualSize();
		size.y += 1;
		if (size.y >= size.h) {
			size.y = 0;
		}
		credits->setActualSize(size);
		});

	// titles
	auto text1 = credits->addField("text1", 1024);
	text1->setFont(bigfont_outline);
	text1->setColor(makeColor(255, 191, 32, 255));
	text1->setHJustify(Field::justify_t::CENTER);
	text1->setVJustify(Field::justify_t::TOP);
	text1->setSize(SDL_Rect{0, Frame::virtualScreenY, Frame::virtualScreenX, font->height() * 80});
	text1->setText(
		u8"Project lead, programming, and design\n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"Music and sound design\n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"Programming\n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"Art and design\n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"Programming and design\n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"Additional art\n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"Additional writing\n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"Special thanks\n"
		u8" \n"
		u8" \n"
		u8" \n"
		u8" \n"
		u8" \n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"A big shout-out to our open-source community!\n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"Barony is a product of Turning Wheel LLC\n"
		u8" \n"
		u8" \n"
		u8" \n \n \n \n \n"
		u8"This game is dedicated to all of our friends, family, and fans\n"
		u8"who encouraged and supported us on our journey to finish it.\n"
		u8" \n"
		u8" \n"
	);

	// entries
	auto text2 = credits->addField("text2", 1024);
	text2->setFont(bigfont_outline);
	text2->setColor(0xffffffff);
	text2->setHJustify(Field::justify_t::CENTER);
	text2->setVJustify(Field::justify_t::TOP);
	text2->setSize(SDL_Rect{0, Frame::virtualScreenY, Frame::virtualScreenX, font->height() * 80});
	text2->setText(
		u8" \n"
		u8"Sheridan Rathbun\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Chris Kukla\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Ciprian Elies\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Josiah Colborn\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Benjamin Potter\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Matthew Griebner\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Frasier Panton\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Our Kickstarter Backers\n"
		u8"Sterling Rathbun\n"
		u8"Kevin White\n"
		u8"Jesse Riddle\n"
		u8"Julian Seeger\n"
		u8"Mathias Golinelli\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Learn more at http://www.github.com/TurningWheel/Barony\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8"Copyright \u00A9 2021, all rights reserved\n"
		u8"http://www.baronygame.com/\n"
		u8" \n \n \n \n \n"
		u8" \n"
		u8" \n"
		u8" \n"
		u8"Thank you!\n"
	);
}

void recordsBackToMainMenu(Button& button) {
	playSound(139, 64); // click sound

	assert(main_menu_frame);

	// revert notification section
	auto notification = main_menu_frame->findFrame("notification"); assert(notification);
	auto image = notification->findImage("background"); assert(image);
	image->path = "images/ui/Main Menus/Main/UI_MainMenu_EXNotification.png";
	notification->setSize(SDL_Rect{
		(Frame::virtualScreenX - 236 * 2) / 2,
		notification->getSize().y,
		236 * 2,
		49 * 2
		});
	notification->setActualSize(SDL_Rect{0, 0, notification->getSize().w, notification->getSize().h});
	image->pos = notification->getActualSize();
	notification->remove("text");

	// enable banners
	for (int c = 0; c < 2; ++c) {
		std::string name = std::string("banner") + std::to_string(c + 1);
		auto banner = main_menu_frame->findFrame(name.c_str());
		banner->setDisabled(false);
	}

	// delete existing buttons
	auto old_buttons = main_menu_frame->findFrame("buttons");
	old_buttons->removeSelf();

	// put original options back
	struct Option {
		const char* name;
		void (*callback)(Button&);
	};
#ifdef NINTENDO
	Option options[] = {
		{"PLAY GAME", mainPlayGame},
		{"HALL OF RECORDS", mainHallOfRecords},
		{"SETTINGS", mainSettings}
	};
#else
	Option options[] = {
		{"PLAY GAME", mainPlayGame},
		{"PLAY MODDED GAME", mainPlayModdedGame},
		{"HALL OF RECORDS", mainHallOfRecords},
		{"SETTINGS", mainSettings},
		{"QUIT", mainQuit}
	};
#endif
	const int num_options = sizeof(options) / sizeof(options[0]);

	int y = main_menu_buttons_height;

	auto buttons = main_menu_frame->addFrame("buttons");
	buttons->setTickCallback(updateMenuCursor);
	buttons->setSize(SDL_Rect{0, y, Frame::virtualScreenX, 36 * num_options});
	buttons->setActualSize(SDL_Rect{0, 0, buttons->getSize().w, buttons->getSize().h});
	buttons->setHollow(true);
	buttons->setBorder(0);
	for (int c = 0; c < num_options; ++c) {
		auto button = buttons->addButton(options[c].name);
		button->setCallback(options[c].callback);
		button->setBorder(8);
		button->setHJustify(Button::justify_t::LEFT);
		button->setVJustify(Button::justify_t::CENTER);
		button->setText(options[c].name);
		button->setFont(menu_option_font);
		button->setBackground("images/ui/Main Menus/Main/UI_MainMenu_SelectorBar00.png");
		button->setColor(makeColor(255, 255, 255, 127));
		button->setHighlightColor(makeColor(255, 255, 255, 255));
		button->setTextColor(makeColor(180, 180, 180, 255));
		button->setTextHighlightColor(makeColor(180, 133, 13, 255));
		button->setSize(SDL_Rect{
			(Frame::virtualScreenX - 164 * 2) / 2,
			y - buttons->getSize().y,
			164 * 2,
			16 * 2
			});
		int back = c - 1 < 0 ? num_options - 1 : c - 1;
		int forward = c + 1 >= num_options ? 0 : c + 1;
		button->setWidgetDown(options[forward].name);
		button->setWidgetUp(options[back].name);
		y += button->getSize().h;
		y += 4;
	}
	y += 16;

	auto records = buttons->findButton("HALL OF RECORDS");
	if (records) {
		records->select();
	}
}

/******************************************************************************/

void mainPlayGame(Button& button) {
	playSound(139, 64); // click sound
}

void mainPlayModdedGame(Button& button) {
	playSound(139, 64); // click sound
}

void mainHallOfRecords(Button& button) {
	playSound(139, 64); // click sound

	assert(main_menu_frame);

	// change "notification" section into Hall of Records banner
	auto notification = main_menu_frame->findFrame("notification"); assert(notification);
	auto image = notification->findImage("background"); assert(image);
	image->path = "images/ui/Main Menus/AdventureArchives/UI_AdventureArchives_TitleGraphic00.png";
	notification->setSize(SDL_Rect{
		(Frame::virtualScreenX - 204 * 2) / 2,
		notification->getSize().y,
		204 * 2,
		43 * 2
		});
	notification->setActualSize(SDL_Rect{0, 0, notification->getSize().w, notification->getSize().h});
	image->pos = notification->getActualSize();

	// add banner text to notification
	auto banner_text = notification->addField("text", 64);
	banner_text->setJustify(Field::justify_t::CENTER);
	banner_text->setText("HALL OF RECORDS");
	banner_text->setFont(menu_option_font);
	banner_text->setColor(makeColor(180, 135, 27, 255));
	banner_text->setSize(SDL_Rect{19 * 2, 15 * 2, 166 * 2, 12 * 2});

	// disable banners
	for (int c = 0; c < 2; ++c) {
		std::string name = std::string("banner") + std::to_string(c + 1);
		auto banner = main_menu_frame->findFrame(name.c_str());
		banner->setDisabled(true);
	}

	// delete existing buttons
	auto old_buttons = main_menu_frame->findFrame("buttons");
	old_buttons->removeSelf();

	struct Option {
		const char* name;
		void (*callback)(Button&);
	};
	Option options[] = {
		{"ADVENTURE ARCHIVES", recordsAdventureArchives},
		{"DUNGEON COMPENDIUM", recordsDungeonCompendium},
		{"STORY INTRODUCTION", recordsStoryIntroduction},
		{"CREDITS", recordsCredits},
		{"BACK TO MAIN MENU", recordsBackToMainMenu}
	};
	const int num_options = sizeof(options) / sizeof(options[0]);

	int y = main_menu_buttons_height;

	auto buttons = main_menu_frame->addFrame("buttons");
	buttons->setTickCallback(updateMenuCursor);
	buttons->setSize(SDL_Rect{0, y, Frame::virtualScreenX, 36 * (num_options + 1)});
	buttons->setActualSize(SDL_Rect{0, 0, buttons->getSize().w, buttons->getSize().h});
	buttons->setHollow(true);
	buttons->setBorder(0);
	for (int c = 0; c < num_options; ++c) {
		auto button = buttons->addButton(options[c].name);
		button->setCallback(options[c].callback);
		button->setBorder(8);
		button->setHJustify(Button::justify_t::LEFT);
		button->setVJustify(Button::justify_t::CENTER);
		button->setText(options[c].name);
		button->setFont(menu_option_font);
		button->setBackground("images/ui/Main Menus/Main/UI_MainMenu_SelectorBar00.png");
		button->setColor(makeColor(255, 255, 255, 127));
		button->setHighlightColor(makeColor(255, 255, 255, 255));
		button->setTextColor(makeColor(180, 180, 180, 255));
		button->setTextHighlightColor(makeColor(180, 133, 13, 255));
		button->setSize(SDL_Rect{
			(Frame::virtualScreenX - 164 * 2) / 2,
			y - buttons->getSize().y,
			164 * 2,
			16 * 2
			});
		int back = c - 1 < 0 ? num_options - 1 : c - 1;
		int forward = c + 1 >= num_options ? 0 : c + 1;
		button->setWidgetDown(options[forward].name);
		button->setWidgetUp(options[back].name);
		button->setWidgetBack("BACK TO MAIN MENU");
		y += button->getSize().h;
		y += 4;
		if (c == num_options - 2) {
			y += button->getSize().h;
			y += 4;
		}
	}
	y += 16;

	auto archives = buttons->findButton("ADVENTURE ARCHIVES");
	if (archives) {
		archives->select();
	}
}

void mainSettings(Button& button) {
	playSound(139, 64); // click sound

	settings_tab_name = "";

	allSettings.add_items_to_hotbar_enabled = auto_hotbar_new_items;
	allSettings.use_on_release_enabled = !right_click_protect;
	allSettings.show_messages_enabled = !disable_messages;
	allSettings.show_player_nametags_enabled = !hide_playertags;
	allSettings.show_hud_enabled = !nohud;
	allSettings.show_ip_address_enabled = !broadcast;
	allSettings.content_control_enabled = !spawn_blood;
	allSettings.colorblind_mode_enabled = colorblind;
	allSettings.arachnophobia_filter_enabled = arachnophobia_filter;
	allSettings.shaking_enabled = shaking;
	allSettings.bobbing_enabled = bobbing;
	allSettings.light_flicker_enabled = flickerLights;
	allSettings.resolution_x = xres;
	allSettings.resolution_y = yres;
	allSettings.vsync_enabled = verticalSync;
	allSettings.vertical_split_enabled = vertical_splitscreen;
	allSettings.gamma = vidgamma * 100.f;
	allSettings.fov = fov;
	allSettings.fps = fpsLimit;
	allSettings.master_volume = master_volume;
	allSettings.gameplay_volume = (float)sfxvolume / 128.f * 100.f;
	allSettings.ambient_volume = (float)sfxAmbientVolume / 128.f * 100.f;
	allSettings.environment_volume = (float)sfxEnvironmentVolume / 128.f * 100.f;
	allSettings.music_volume = (float)musvolume / 128.f * 100.f;
	allSettings.minimap_pings_enabled = !minimapPingMute;
	allSettings.player_monster_sounds_enabled = !mute_player_monster_sounds;
	allSettings.out_of_focus_audio_enabled = !mute_audio_on_focus_lost;
	allSettings.numkeys_in_inventory_enabled = hotbar_numkey_quick_add;
	allSettings.mouse_sensitivity = mousespeed;
	allSettings.reverse_mouse_enabled = reversemouse;
	allSettings.smooth_mouse_enabled = smoothmouse;
	allSettings.rotation_speed_limit_enabled = !disablemouserotationlimit;
	allSettings.turn_sensitivity_x = gamepad_rightx_sensitivity / 10;
	allSettings.turn_sensitivity_y = gamepad_righty_sensitivity / 10;
	allSettings.classic_mode_enabled = svFlags & SV_FLAG_CLASSIC;
	allSettings.hardcore_mode_enabled = svFlags & SV_FLAG_HARDCORE;
	allSettings.friendly_fire_enabled = svFlags & SV_FLAG_FRIENDLYFIRE;
	allSettings.keep_inventory_enabled = svFlags & SV_FLAG_KEEPINVENTORY;
	allSettings.hunger_enabled = svFlags & SV_FLAG_HUNGER;
	allSettings.minotaur_enabled = svFlags & SV_FLAG_MINOTAURS;
	allSettings.random_traps_enabled = svFlags & SV_FLAG_TRAPS;
	allSettings.extra_life_enabled = svFlags & SV_FLAG_LIFESAVING;
	allSettings.cheats_enabled = svFlags & SV_FLAG_CHEATS;

	auto settings = main_menu_frame->addFrame("settings");
	settings->setSize(SDL_Rect{(Frame::virtualScreenX - 1126) / 2, (Frame::virtualScreenY - 718) / 2, 1126, 718});
	settings->setActualSize(SDL_Rect{0, 0, settings->getSize().w, settings->getSize().h});
	settings->setHollow(true);
	settings->setBorder(0);
	settings->addImage(
		SDL_Rect{
			(settings->getActualSize().w - 553 * 2) / 2,
			0,
			553 * 2,
			357 * 2
		},
		0xffffffff,
		"images/ui/Main Menus/Settings/Settings_Window02.png",
		"background"
	);
	auto timber = settings->addImage(
		SDL_Rect{0, 66 * 2, 1126, 586},
		0xffffffff,
		"images/ui/Main Menus/Settings/Settings_TimberEdge00.png",
		"timber"
	);
	timber->ontop = true;

	settings->setTickCallback([](Widget& widget){
		auto settings = static_cast<Frame*>(&widget);
		const char* tabs[] = {
			"UI",
			"Video",
			"Audio",
			"Controls"
		};
		for (auto name : tabs) {
			auto button = settings->findButton(name);
			if (button) {
				if (name == settings_tab_name) {
					button->setBackground("images/ui/Main Menus/Settings/Settings_Button_SubTitleSelect00.png");
				} else {
					button->setBackground("images/ui/Main Menus/Settings/Settings_Button_SubTitle00.png");
				}
			}
		}
		});

	static const char* pixel_maz_outline = "fonts/pixel_maz.ttf#46#2";

	auto window_title = settings->addField("window_title", 64);
	window_title->setFont(pixel_maz_outline);
	window_title->setSize(SDL_Rect{394, 26, 338, 24});
	window_title->setJustify(Field::justify_t::CENTER);
	window_title->setText("SETTINGS");

	auto tab_left = settings->addButton("tab_left");
	tab_left->setBackground("images/ui/Main Menus/Settings/Settings_Button_L00.png");
	tab_left->setSize(SDL_Rect{32, 68, 38, 58});
	tab_left->setColor(makeColor(255, 255, 255, 191));
	tab_left->setHighlightColor(makeColor(255, 255, 255, 255));
	tab_left->setWidgetBack("discard_and_exit");
	tab_left->setWidgetPageLeft("tab_left");
	tab_left->setWidgetPageRight("tab_right");
	tab_left->setWidgetRight("UI");
	tab_left->setWidgetDown("restore_defaults");
	tab_left->addWidgetAction("MenuAlt1", "restore_defaults");
	tab_left->addWidgetAction("MenuStart", "confirm_and_exit");
	tab_left->setCallback([](Button&){
		auto settings = main_menu_frame->findFrame("settings"); assert(settings);
		const char* tabs[] = {
			"UI",
			"Video",
			"Audio",
			"Controls"
		};
		const char* prevtab = nullptr;
		for (auto tab : tabs) {
			auto button = settings->findButton(tab); assert(button);
			const char* name = "images/ui/Main Menus/Settings/Settings_Button_SubTitleSelect00.png";
			if (strcmp(button->getBackground(), name) == 0) {
				if (prevtab) {
					auto prevbutton = settings->findButton(prevtab); assert(prevbutton);
					prevbutton->select();
					prevbutton->activate();
				}
				return; 
			}
			prevtab = tab;
		}
		});

	auto tab_right = settings->addButton("tab_right");
	tab_right->setBackground("images/ui/Main Menus/Settings/Settings_Button_R00.png");
	tab_right->setSize(SDL_Rect{1056, 68, 38, 58});
	tab_right->setColor(makeColor(255, 255, 255, 191));
	tab_right->setHighlightColor(makeColor(255, 255, 255, 255));
	tab_right->setWidgetBack("discard_and_exit");
	tab_right->setWidgetPageLeft("tab_left");
	tab_right->setWidgetPageRight("tab_right");
	tab_right->setWidgetLeft("Controls");
	tab_right->setWidgetDown("confirm_and_exit");
	tab_right->addWidgetAction("MenuAlt1", "restore_defaults");
	tab_right->addWidgetAction("MenuStart", "confirm_and_exit");
	tab_right->setCallback([](Button&){
		auto settings = main_menu_frame->findFrame("settings"); assert(settings);
		const char* tabs[] = {
			"Controls",
			"Audio",
			"Video",
			"UI",
		};
		const char* nexttab = nullptr;
		for (auto tab : tabs) {
			auto button = settings->findButton(tab); assert(button);
			const char* name = "images/ui/Main Menus/Settings/Settings_Button_SubTitleSelect00.png";
			if (strcmp(button->getBackground(), name) == 0) {
				if (nexttab) {
					auto nextbutton = settings->findButton(nexttab); assert(nextbutton);
					nextbutton->select();
					nextbutton->activate();
				}
				return;
			}
			nexttab = tab;
		}
		});

	struct Option {
		const char* name;
		void (*callback)(Button&);
	};
	Option tabs[] = {
		{"UI", settingsUI},
		{"Video", settingsVideo},
		{"Audio", settingsAudio},
		{"Controls", settingsControls}
	};
	int num_tabs = sizeof(tabs) / sizeof(tabs[0]);
	for (int c = 0; c < num_tabs; ++c) {
		auto button = settings->addButton(tabs[c].name);
		button->setCallback(tabs[c].callback);
		button->setText(tabs[c].name);
		button->setFont(pixel_maz_outline);
		button->setBackground("images/ui/Main Menus/Settings/Settings_Button_SubTitle00.png");
		button->setBackgroundActivated("images/ui/Main Menus/Settings/Settings_Button_SubTitleSelect00.png");
		button->setSize(SDL_Rect{76 + (272 - 76) * c, 64, 184, 64});
		button->setColor(makeColor(255, 255, 255, 191));
		button->setHighlightColor(makeColor(255, 255, 255, 255));
		button->setWidgetPageLeft("tab_left");
		button->setWidgetPageRight("tab_right");
		button->addWidgetAction("MenuAlt1", "restore_defaults");
		button->addWidgetAction("MenuStart", "confirm_and_exit");
		if (c > 0) {
			button->setWidgetLeft(tabs[c - 1].name);
		} else {
			button->setWidgetLeft("tab_left");
		}
		if (c < num_tabs - 1) {
			button->setWidgetRight(tabs[c + 1].name);
		} else {
			button->setWidgetRight("tab_right");
		}
		button->setWidgetBack("discard_and_exit");
		if (c <= num_tabs / 2) {
			button->setWidgetDown("restore_defaults");
		} else if (c == num_tabs - 2) {
			button->setWidgetDown("discard_and_exit");
		} else if (c == num_tabs - 1) {
			button->setWidgetDown("confirm_and_exit");
		}
	}
	auto first_tab = settings->findButton(tabs[0].name);
	if (first_tab) {
		first_tab->select();
		first_tab->activate();
	}

	auto tooltip = settings->addField("tooltip", 256);
	tooltip->setSize(SDL_Rect{92, 590, 948, 32});
	tooltip->setFont(smallfont_no_outline);
	tooltip->setText("");

	auto restore_defaults = settings->addButton("restore_defaults");
	restore_defaults->setBackground("images/ui/Main Menus/Settings/Settings_Button_Basic00.png");
	restore_defaults->setSize(SDL_Rect{84, 630, 164, 62});
	restore_defaults->setText("Restore\nDefaults");
	restore_defaults->setJustify(Button::justify_t::CENTER);
	restore_defaults->setFont(smallfont_outline);
	restore_defaults->setColor(makeColor(255, 255, 255, 191));
	restore_defaults->setHighlightColor(makeColor(255, 255, 255, 255));
	restore_defaults->setWidgetBack("discard_and_exit");
	restore_defaults->setWidgetPageLeft("tab_left");
	restore_defaults->setWidgetPageRight("tab_right");
	restore_defaults->setWidgetUp("UI");
	restore_defaults->setWidgetRight("discard_and_exit");
	restore_defaults->addWidgetAction("MenuAlt1", "restore_defaults");
	restore_defaults->addWidgetAction("MenuStart", "confirm_and_exit");
	restore_defaults->setCallback([](Button& button){
		settingsReset();
		});

	auto discard_and_exit = settings->addButton("discard_and_exit");
	discard_and_exit->setBackground("images/ui/Main Menus/Settings/Settings_Button_Basic00.png");
	discard_and_exit->setSize(SDL_Rect{700, 630, 164, 62});
	discard_and_exit->setText("Discard\n& Exit");
	discard_and_exit->setJustify(Button::justify_t::CENTER);
	discard_and_exit->setFont(smallfont_outline);
	discard_and_exit->setColor(makeColor(255, 255, 255, 191));
	discard_and_exit->setHighlightColor(makeColor(255, 255, 255, 255));
	discard_and_exit->setCallback([](Button&){
		assert(main_menu_frame);
		auto buttons = main_menu_frame->findFrame("buttons"); assert(buttons);
		auto settings_button = buttons->findButton("SETTINGS"); assert(settings_button);
		settings_button->select();
		auto settings = main_menu_frame->findFrame("settings");
		if (settings) {
			settings->removeSelf();
		}
		});
	discard_and_exit->setWidgetBack("discard_and_exit");
	discard_and_exit->setWidgetPageLeft("tab_left");
	discard_and_exit->setWidgetPageRight("tab_right");
	discard_and_exit->setWidgetUp("Controls");
	discard_and_exit->setWidgetLeft("restore_defaults");
	discard_and_exit->setWidgetRight("confirm_and_exit");
	discard_and_exit->addWidgetAction("MenuAlt1", "restore_defaults");
	discard_and_exit->addWidgetAction("MenuStart", "confirm_and_exit");

	auto confirm_and_exit = settings->addButton("confirm_and_exit");
	confirm_and_exit->setBackground("images/ui/Main Menus/Settings/Settings_Button_Basic00.png");
	confirm_and_exit->setSize(SDL_Rect{880, 630, 164, 62});
	confirm_and_exit->setText("Confirm\n& Exit");
	confirm_and_exit->setJustify(Button::justify_t::CENTER);
	confirm_and_exit->setFont(smallfont_outline);
	confirm_and_exit->setColor(makeColor(255, 255, 255, 191));
	confirm_and_exit->setHighlightColor(makeColor(255, 255, 255, 255));
	confirm_and_exit->setCallback([](Button&){
		assert(main_menu_frame);
		settingsSave();
		auto buttons = main_menu_frame->findFrame("buttons"); assert(buttons);
		auto settings_button = buttons->findButton("SETTINGS"); assert(settings_button);
		settings_button->select();
		auto settings = main_menu_frame->findFrame("settings");
		if (settings) {
			settings->removeSelf();
		}
		});
	confirm_and_exit->setWidgetBack("discard_and_exit");
	confirm_and_exit->setWidgetPageLeft("tab_left");
	confirm_and_exit->setWidgetPageRight("tab_right");
	confirm_and_exit->setWidgetUp("tab_right");
	confirm_and_exit->setWidgetLeft("discard_and_exit");
	confirm_and_exit->addWidgetAction("MenuAlt1", "restore_defaults");
	confirm_and_exit->addWidgetAction("MenuStart", "confirm_and_exit");
}

void mainQuit(Button& button) {
	playSound(139, 64); // click sound
}

/******************************************************************************/

void doMainMenu() {
	if (!main_menu_frame) {
		createMainMenu();
	}

	assert(main_menu_frame);

	if (main_menu_fade_destination != FadeDestination::None) {
		if (fadeout && fadealpha >= 255) {
			if (main_menu_fade_destination == FadeDestination::RootMainMenu) {
				destroyMainMenu();
				createMainMenu();
			}
			if (main_menu_fade_destination == FadeDestination::IntroStoryScreen) {
				createStoryScreen();
			}
			fadeout = false;
			main_menu_fade_destination = FadeDestination::None;
		}
	}
}

void createMainMenu() {
	main_menu_frame = gui->addFrame("main_menu");

	auto frame = main_menu_frame;
	frame->setSize(SDL_Rect{0, 0, Frame::virtualScreenX, Frame::virtualScreenY});
	frame->setActualSize(SDL_Rect{0, 0, frame->getSize().w, frame->getSize().h});
	frame->setHollow(true);
	frame->setBorder(0);
	frame->setTickCallback([](Widget&){++main_menu_ticks;});

	int y = 16;

	auto title_img = Image::get("images/system/title.png");
	auto title = frame->addImage(
		SDL_Rect{
			(int)(Frame::virtualScreenX - (int)title_img->getWidth() * 2.0 / 3.0) / 2,
			y,
			(int)(title_img->getWidth() * 2.0 / 3.0),
			(int)(title_img->getHeight() * 2.0 / 3.0)
		},
		makeColor(255, 255, 255, 255),
		title_img->getName(),
		"title"
	);
	y += title->pos.h;

	auto notification = frame->addFrame("notification");
	notification->setSize(SDL_Rect{
		(Frame::virtualScreenX - 236 * 2) / 2,
		y,
		236 * 2,
		49 * 2
		});
	notification->setActualSize(SDL_Rect{0, 0, notification->getSize().w, notification->getSize().h});
	notification->addImage(notification->getActualSize(), 0xffffffff,
		"images/ui/Main Menus/Main/UI_MainMenu_EXNotification.png", "background");
	y += notification->getSize().h;
	y += 16;

	struct Option {
		const char* name;
		void (*callback)(Button&);
	};
#ifdef NINTENDO
	Option options[] = {
		{"PLAY GAME", mainPlayGame},
		{"HALL OF RECORDS", mainHallOfRecords},
		{"SETTINGS", mainSettings}
	};
#else
	Option options[] = {
		{"PLAY GAME", mainPlayGame},
		{"PLAY MODDED GAME", mainPlayModdedGame},
		{"HALL OF RECORDS", mainHallOfRecords},
		{"SETTINGS", mainSettings},
		{"QUIT", mainQuit}
	};
#endif

	const int num_options = sizeof(options) / sizeof(options[0]);

	main_menu_buttons_height = y;

	auto buttons = frame->addFrame("buttons");
	buttons->setTickCallback(updateMenuCursor);
	buttons->setSize(SDL_Rect{0, y, Frame::virtualScreenX, 36 * num_options});
	buttons->setActualSize(SDL_Rect{0, 0, buttons->getSize().w, buttons->getSize().h});
	buttons->setHollow(true);
	buttons->setBorder(0);
	for (int c = 0; c < num_options; ++c) {
		auto button = buttons->addButton(options[c].name);
		button->setCallback(options[c].callback);
		button->setBorder(8);
		button->setHJustify(Button::justify_t::LEFT);
		button->setVJustify(Button::justify_t::CENTER);
		button->setText(options[c].name);
		button->setFont(menu_option_font);
		button->setBackground("images/ui/Main Menus/Main/UI_MainMenu_SelectorBar00.png");
		button->setColor(makeColor(255, 255, 255, 127));
		button->setHighlightColor(makeColor(255, 255, 255, 255));
		button->setTextColor(makeColor(180, 180, 180, 255));
		button->setTextHighlightColor(makeColor(180, 133, 13, 255));
		button->setSize(SDL_Rect{
			(Frame::virtualScreenX - 164 * 2) / 2,
			y - buttons->getSize().y,
			164 * 2,
			16 * 2
			});
		int back = c - 1 < 0 ? num_options - 1 : c - 1;
		int forward = c + 1 >= num_options ? 0 : c + 1;
		button->setWidgetDown(options[forward].name);
		button->setWidgetUp(options[back].name);
		y += button->getSize().h;
		y += 4;
	}
	y += 16;

	auto play = buttons->findButton("PLAY GAME");
	if (play) {
		play->select();
		if (main_menu_cursor_x == 0 && main_menu_cursor_y == 0) {
			main_menu_cursor_x = play->getSize().x - 80;
			main_menu_cursor_y = play->getSize().y - 9 + buttons->getSize().y;
		}
	}

	frame->addImage(
		SDL_Rect{
			main_menu_cursor_x + (int)(sinf(main_menu_cursor_bob) * 16.f) - 16,
			main_menu_cursor_y,
			37 * 2,
			23 * 2
		},
		0xffffffff,
		"images/ui/Main Menus/UI_Pointer_Spear00.png",
		"cursor"
	);

	for (int c = 0; c < 2; ++c) {
		std::string name = std::string("banner") + std::to_string(c + 1);
		auto banner = frame->addFrame(name.c_str());
		banner->setSize(SDL_Rect{
			(Frame::virtualScreenX - 472) / 2,
			y,
			472,
			76
			});
		banner->setActualSize(SDL_Rect{0, 0, banner->getSize().w, banner->getSize().h});
		std::string background = std::string("images/ui/Main Menus/Main/UI_MainMenu_EXBanner") + std::to_string(c + 1) + std::string(".png");
		banner->addImage(banner->getActualSize(), 0xffffffff, background.c_str());
		y += banner->getSize().h;
		y += 16;
	}

	auto copyright = frame->addField("copyright", 64);
	copyright->setFont(bigfont_outline);
	copyright->setText(u8"Copyright \u00A9 2021, Turning Wheel LLC");
	copyright->setJustify(Field::justify_t::CENTER);
	copyright->setSize(SDL_Rect{
		(Frame::virtualScreenX - 512) / 2,
		Frame::virtualScreenY - 50,
		512,
		50
		});
	copyright->setColor(0xffffffff);

	auto version = frame->addField("version", 32);
	version->setFont(smallfont_outline);
	version->setText(VERSION);
	version->setHJustify(Field::justify_t::RIGHT);
	version->setVJustify(Field::justify_t::BOTTOM);
	version->setSize(SDL_Rect{
		Frame::virtualScreenX - 200,
		Frame::virtualScreenY - 54,
		200,
		50
		});
	version->setColor(0xffffffff);

#ifndef NINTENDO
	int num_online_players = 1337; // TODO change me!
	std::string online_players_text = std::string("Players online: ") + std::to_string(num_online_players);
	auto online_players = frame->addField("online_players", 32);
	online_players->setFont(smallfont_outline);
	online_players->setText(online_players_text.c_str());
	online_players->setHJustify(Field::justify_t::RIGHT);
	online_players->setVJustify(Field::justify_t::TOP);
	online_players->setSize(SDL_Rect{
		Frame::virtualScreenX - 200,
		4,
		200,
		50
		});
	online_players->setColor(0xffffffff);
#endif
}

void destroyMainMenu() {
	main_menu_frame->removeSelf();
	main_menu_frame = nullptr;
}