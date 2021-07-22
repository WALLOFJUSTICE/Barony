#include "input.hpp"
#include "player.hpp"

#include <algorithm>

Input Input::inputs[MAXPLAYERS];

const float Input::sensitivity = 1.f;
const float Input::deadzone = 0.2f;
const float Input::analogToggleThreshold = .5;
const Uint32 Input::BUTTON_HELD_TICKS = TICKS_PER_SECOND / 4;
const Uint32 Input::BUTTON_ANALOG_REPEAT_TICKS = TICKS_PER_SECOND / 4;
std::unordered_map<std::string, SDL_Scancode> Input::scancodeNames;
std::unordered_map<int, SDL_GameController*> Input::gameControllers;
std::unordered_map<int, SDL_Joystick*> Input::joysticks;
bool Input::keys[SDL_NUM_SCANCODES] = { false };
bool Input::mouseButtons[8] = { false };
std::string Input::lastInputOfAnyKind;

void Input::defaultBindings() {
	// these bindings should probably not be accessible to the player to change.
	inputs[0].bind("MenuTab", "Tab");
	for (int c = 0; c < MAXPLAYERS; ++c) {
		inputs[c].bind("MenuUp", (std::string("Pad") + std::to_string(c) + std::string("DpadY-")).c_str());
		inputs[c].bind("MenuLeft", (std::string("Pad") + std::to_string(c) + std::string("DpadX-")).c_str());
		inputs[c].bind("MenuRight", (std::string("Pad") + std::to_string(c) + std::string("DpadX+")).c_str());
		inputs[c].bind("MenuDown", (std::string("Pad") + std::to_string(c) + std::string("DpadY+")).c_str());
		inputs[c].bind("MenuConfirm", (std::string("Pad") + std::to_string(c) + std::string("ButtonA")).c_str());
		inputs[c].bind("MenuCancel", (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str());
#ifdef NINTENDO
		inputs[c].bind("MenuAlt1", (std::string("Pad") + std::to_string(c) + std::string("ButtonY")).c_str());
		inputs[c].bind("MenuAlt2", (std::string("Pad") + std::to_string(c) + std::string("ButtonX")).c_str());
#else
		inputs[c].bind("MenuAlt1", (std::string("Pad") + std::to_string(c) + std::string("ButtonX")).c_str());
		inputs[c].bind("MenuAlt2", (std::string("Pad") + std::to_string(c) + std::string("ButtonY")).c_str());
#endif
		inputs[c].bind("MenuStart", (std::string("Pad") + std::to_string(c) + std::string("ButtonStart")).c_str());
		inputs[c].bind("MenuSelect", (std::string("Pad") + std::to_string(c) + std::string("ButtonBack")).c_str());
		inputs[c].bind("MenuPageLeft", (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftBumper")).c_str());
		inputs[c].bind("MenuPageRight", (std::string("Pad") + std::to_string(c) + std::string("ButtonRightBumper")).c_str());
		inputs[c].bind("AltMenuUp", (std::string("Pad") + std::to_string(c) + std::string("StickLeftY-")).c_str());
		inputs[c].bind("AltMenuLeft", (std::string("Pad") + std::to_string(c) + std::string("StickLeftX-")).c_str());
		inputs[c].bind("AltMenuRight", (std::string("Pad") + std::to_string(c) + std::string("StickLeftX+")).c_str());
		inputs[c].bind("AltMenuDown", (std::string("Pad") + std::to_string(c) + std::string("StickLeftY+")).c_str());
		inputs[c].bind("MenuScrollUp", (std::string("Pad") + std::to_string(c) + std::string("StickRightY-")).c_str());
		inputs[c].bind("MenuScrollLeft", (std::string("Pad") + std::to_string(c) + std::string("StickRightX-")).c_str());
		inputs[c].bind("MenuScrollRight", (std::string("Pad") + std::to_string(c) + std::string("StickRightX+")).c_str());
		inputs[c].bind("MenuScrollDown", (std::string("Pad") + std::to_string(c) + std::string("StickRightY+")).c_str());

#ifdef NINTENDO
		inputs[c].bind("HotbarFacebarLeft", (std::string("Pad") + std::to_string(c) + std::string("ButtonY")).c_str());
		inputs[c].bind("HotbarFacebarUp", (std::string("Pad") + std::to_string(c) + std::string("ButtonX")).c_str());
		inputs[c].bind("HotbarFacebarRight", (std::string("Pad") + std::to_string(c) + std::string("ButtonA")).c_str());
#else
		inputs[c].bind("HotbarFacebarLeft", (std::string("Pad") + std::to_string(c) + std::string("ButtonX")).c_str());
		inputs[c].bind("HotbarFacebarUp", (std::string("Pad") + std::to_string(c) + std::string("ButtonY")).c_str());
		inputs[c].bind("HotbarFacebarRight", (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str());
#endif
		inputs[c].bind("HotbarFacebarModifierLeft", (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftBumper")).c_str());
		inputs[c].bind("HotbarFacebarModifierRight", (std::string("Pad") + std::to_string(c) + std::string("ButtonRightBumper")).c_str());
		inputs[c].bind("HotbarFacebarCancel", (std::string("Pad") + std::to_string(c) + std::string("DpadY+")).c_str());

		inputs[c].bind("HotbarInventoryDrop", (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str());
		inputs[c].bind("HotbarInventoryClearSlot", (std::string("Pad") + std::to_string(c) + std::string("ButtonY")).c_str());

		inputs[c].bind("InventoryMoveUp", (std::string("Pad") + std::to_string(c) + std::string("DpadY-")).c_str());
		inputs[c].bind("InventoryMoveLeft", (std::string("Pad") + std::to_string(c) + std::string("DpadX-")).c_str());
		inputs[c].bind("InventoryMoveRight", (std::string("Pad") + std::to_string(c) + std::string("DpadX+")).c_str());
		inputs[c].bind("InventoryMoveDown", (std::string("Pad") + std::to_string(c) + std::string("DpadY+")).c_str());

		inputs[c].bind("InventoryMoveUpAnalog", (std::string("Pad") + std::to_string(c) + std::string("StickRightY-")).c_str());
		inputs[c].bind("InventoryMoveLeftAnalog", (std::string("Pad") + std::to_string(c) + std::string("StickRightX-")).c_str());
		inputs[c].bind("InventoryMoveRightAnalog", (std::string("Pad") + std::to_string(c) + std::string("StickRightX+")).c_str());
		inputs[c].bind("InventoryMoveDownAnalog", (std::string("Pad") + std::to_string(c) + std::string("StickRightY+")).c_str());

		inputs[c].bind("InventoryCharacterRotateLeft", (std::string("Pad") + std::to_string(c) + std::string("StickRightX-")).c_str());
		inputs[c].bind("InventoryCharacterRotateRight", (std::string("Pad") + std::to_string(c) + std::string("StickRightX+")).c_str());

		inputs[c].bind("HotbarCyclePrev", (std::string("Pad") + std::to_string(c) + std::string("DpadX-")).c_str());
		inputs[c].bind("HotbarCycleNext", (std::string("Pad") + std::to_string(c) + std::string("DpadX+")).c_str());
		inputs[c].bind("HotbarCyclePrevAlt", "MouseWheelUp");
		inputs[c].bind("HotbarCycleNextAlt", "MouseWheelDown");
		inputs[c].bind("HotbarSlot1", "1");
		inputs[c].bind("HotbarSlot2", "2");
		inputs[c].bind("HotbarSlot3", "3");
		inputs[c].bind("HotbarSlot4", "4");
		inputs[c].bind("HotbarSlot5", "5");
		inputs[c].bind("HotbarSlot6", "6");
		inputs[c].bind("HotbarSlot7", "7");
		inputs[c].bind("HotbarSlot8", "8");
		inputs[c].bind("HotbarSlot9", "9");
		inputs[c].bind("HotbarSlot10", "0");
	}
}

void Input::addGameController(int id, GameController& controller) {
	gameControllers.emplace(id, controller.sdl_device);
	for (auto& input : inputs) {
		input.refresh();
	}
}

float Input::analog(const char* binding) const {
	auto b = bindings.find(binding);
	return b != bindings.end() ? (*b).second.analog : 0.f;
}

bool Input::binary(const char* binding) const {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	return b != bindings.end() ? (*b).second.binary : false;
}

bool Input::binaryToggle(const char* binding) const {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	return b != bindings.end() ? (*b).second.binary && !(*b).second.consumed : false;
}

bool Input::analogToggle(const char* binding) const {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	return b != bindings.end() ? (*b).second.analog > analogToggleThreshold && !(*b).second.analogConsumed : false;
}

bool Input::binaryReleaseToggle(const char* binding) const {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	return b != bindings.end() ? (*b).second.binaryRelease && !(*b).second.binaryReleaseConsumed : false;
}

bool Input::consumeAnalogToggle(const char* binding) {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	if ( b != bindings.end() && (*b).second.analog > analogToggleThreshold && !(*b).second.analogConsumed ) {
		(*b).second.analogConsumed = true;
		return true;
	}
	else {
		return false;
	}
}

bool Input::consumeBinaryToggle(const char* binding) {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	if (b != bindings.end() && (*b).second.binary && !(*b).second.consumed) {
		(*b).second.consumed = true;
		if ( (*b).second.type == binding_t::bindtype_t::MOUSE_BUTTON
			&& ((*b).second.mouseButton == SDL_BUTTON_WHEELDOWN
				|| (*b).second.mouseButton == SDL_BUTTON_WHEELUP) )
		{
			mouseButtons[(*b).second.mouseButton] = false; // manually need to clear this
		}
		return true;
	} else {
		return false;
	}
}

bool Input::consumeBinaryReleaseToggle(const char* binding) {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	if ( b != bindings.end() && (*b).second.binaryRelease && !(*b).second.binaryReleaseConsumed ) {
		(*b).second.binaryReleaseConsumed = true;
		return true;
	}
	else {
		return false;
	}
}

bool Input::binaryHeldToggle(const char* binding) const {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	return b != bindings.end() 
		? ((*b).second.binary && !(*b).second.consumed && (ticks - (*b).second.binaryHeldTicks) > BUTTON_HELD_TICKS)
		: false;
}

bool Input::analogHeldToggle(const char* binding) const {
	auto b = bindings.find(binding);
	if ( b != bindings.end() )
	{
		if ( (*b).second.type == binding_t::bindtype_t::KEYBOARD && ::inputs.bPlayerUsingKeyboardControl(player) == false )
		{
			return false;
		}
	}
	return b != bindings.end()
		? ((*b).second.analog > analogToggleThreshold && !(*b).second.analogConsumed && (ticks - (*b).second.analogHeldTicks) > BUTTON_HELD_TICKS)
		: false;
}

const char* Input::binding(const char* binding) const {
	auto b = bindings.find(binding);
	return b != bindings.end() ? (*b).second.input.c_str() : "";
}

void Input::refresh() {
	for (auto& pair : bindings) {
		bind(pair.first.c_str(), pair.second.input.c_str());
	}
}

Input::binding_t Input::input(const char* binding) const {
	auto b = bindings.find(binding);
	return b != bindings.end() ? (*b).second : Input::binding_t();
}

std::string Input::getGlyphPathForInput(const char* binding) const
{
	return getGlyphPathForInput(input(binding));
}

std::string Input::getGlyphPathForInput(binding_t binding) const
{
	std::string rootPath = "images/ui/Glyphs/";
	if ( binding.type == binding_t::bindtype_t::CONTROLLER_BUTTON )
	{
#ifdef NINTENDO
		switch ( binding.padButton )
		{
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A:
				return rootPath + "G_Switch_A00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B:
				return rootPath + "G_Switch_B00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X:
				return rootPath + "G_Switch_X00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y:
				return rootPath + "G_Switch_Y00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				return rootPath + "G_Switch_L00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
				return rootPath + "G_Switch_R00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK:
				return rootPath + "G_Switch_LStick00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK:
				return rootPath + "G_Switch_LStick00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START:
				return rootPath + "G_Switch_+00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK:
				return rootPath + "G_Switch_-A00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP:
				return rootPath + "G_Up00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				return rootPath + "G_Left00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				return rootPath + "G_Down00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				return rootPath + "G_Right00.png";
			default:
				return "";
		}
#else
		switch ( binding.padButton )
		{
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A:
				return rootPath + "G_Xbox_A00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B:
				return rootPath + "G_Xbox_B00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X:
				return rootPath + "G_Xbox_X00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y:
				return rootPath + "G_Xbox_Y00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				return rootPath + "G_Switch_L00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
				return rootPath + "G_Switch_R00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK:
				return rootPath + "G_Switch_LStick00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK:
				return rootPath + "G_Switch_LStick00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START:
				return rootPath + "G_Switch_+00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK:
				return rootPath + "G_Switch_-A00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP:
				return rootPath + "G_Up00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				return rootPath + "G_Left00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				return rootPath + "G_Down00.png";
			case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				return rootPath + "G_Right00.png";
			default:
				return "";
		}
#endif
	}
	else if ( binding.type == binding_t::bindtype_t::KEYBOARD )
	{
		return "";
	}
	return "";
}

void Input::bind(const char* binding, const char* input) {
	auto b = bindings.find(binding);
	if (b == bindings.end()) {
		auto result = bindings.emplace(binding, binding_t());
		b = result.first;
	}
	(*b).second.input.assign(input);
	if (input == nullptr) {
		(*b).second.type = binding_t::INVALID;
		return;
	}

	size_t len = strlen(input);
	if (len >= 3 && strncmp(input, "Pad", 3) == 0) {
		// game controller

		char* type = nullptr;
		Uint32 index = strtol((const char*)(input + 3), &type, 10);
		auto& list = gameControllers;
		auto find = list.find(index);
		if (find != list.end()) {
			SDL_GameController* pad = (*find).second;
			(*b).second.pad = pad;
			if (strncmp(type, "Button", 6) == 0) {
				if (strcmp((const char*)(type + 6), "A") == 0) {
#ifdef NINTENDO
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_B;
#else
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_A;
#endif
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "B") == 0) {
#ifdef NINTENDO
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_A;
#else
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_B;
#endif
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "X") == 0) {
#ifdef NINTENDO
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_Y;
#else
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_X;
#endif
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "Y") == 0) {
#ifdef NINTENDO
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_X;
#else
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_Y;
#endif
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "Back") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_BACK;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "Start") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_START;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "LeftStick") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_LEFTSTICK;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "RightStick") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "LeftBumper") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "RightBumper") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else {
					(*b).second.type = binding_t::INVALID;
					return;
				}
			} else if (strncmp(type, "StickLeft", 9) == 0) {
				if (strcmp((const char*)(type + 9), "X-") == 0) {
					(*b).second.padAxisNegative = true;
					(*b).second.padAxis = SDL_CONTROLLER_AXIS_LEFTX;
					(*b).second.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 9), "X+") == 0) {
					(*b).second.padAxisNegative = false;
					(*b).second.padAxis = SDL_CONTROLLER_AXIS_LEFTX;
					(*b).second.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 9), "Y-") == 0) {
					(*b).second.padAxisNegative = true;
					(*b).second.padAxis = SDL_CONTROLLER_AXIS_LEFTY;
					(*b).second.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 9), "Y+") == 0) {
					(*b).second.padAxisNegative = false;
					(*b).second.padAxis = SDL_CONTROLLER_AXIS_LEFTY;
					(*b).second.type = binding_t::CONTROLLER_AXIS;
					return;
				} else {
					(*b).second.type = binding_t::INVALID;
					return;
				}
			} else if (strncmp(type, "StickRight", 10) == 0) {
				if (strcmp((const char*)(type + 10), "X-") == 0) {
					(*b).second.padAxisNegative = true;
					(*b).second.padAxis = SDL_CONTROLLER_AXIS_RIGHTX;
					(*b).second.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 10), "X+") == 0) {
					(*b).second.padAxisNegative = false;
					(*b).second.padAxis = SDL_CONTROLLER_AXIS_RIGHTX;
					(*b).second.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 10), "Y-") == 0) {
					(*b).second.padAxisNegative = true;
					(*b).second.padAxis = SDL_CONTROLLER_AXIS_RIGHTY;
					(*b).second.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 10), "Y+") == 0) {
					(*b).second.padAxisNegative = false;
					(*b).second.padAxis = SDL_CONTROLLER_AXIS_RIGHTY;
					(*b).second.type = binding_t::CONTROLLER_AXIS;
					return;
				} else {
					(*b).second.type = binding_t::INVALID;
					return;
				}
			} else if (strncmp(type, "Dpad", 4) == 0) {
				if (strcmp((const char*)(type + 4), "X-") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 4), "X+") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 4), "Y-") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_DPAD_UP;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 4), "Y+") == 0) {
					(*b).second.padButton = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
					(*b).second.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else {
					(*b).second.type = binding_t::INVALID;
					return;
				}
			} else if (strncmp(type, "LeftTrigger", 11) == 0) {
				(*b).second.padAxisNegative = false;
				(*b).second.padAxis = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
				(*b).second.type = binding_t::CONTROLLER_AXIS;
				return;
			} else if (strncmp(type, "RightTrigger", 12) == 0) {
				(*b).second.padAxisNegative = false;
				(*b).second.padAxis = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
				(*b).second.type = binding_t::CONTROLLER_AXIS;
				return;
			} else {
				(*b).second.type = binding_t::INVALID;
				return;
			}
		} else {
			(*b).second.type = binding_t::INVALID;
			return;
		}
	} else if (len >= 3 && strncmp(input, "Joy", 3) == 0) {
		// joystick

		char* type = nullptr;
		Uint32 index = strtol((const char*)(input + 3), &type, 10);
		auto& list = joysticks;
		auto find = list.find(index);
		if (find != list.end()) {
			SDL_Joystick* joystick = (*find).second;
			(*b).second.joystick = joystick;
			if (strncmp(type, "Button", 6) == 0) {
				(*b).second.type = binding_t::JOYSTICK_BUTTON;
				(*b).second.joystickButton = strtol((const char*)(type + 6), nullptr, 10);
				return;
			} else if (strncmp(type, "Axis-", 5) == 0) {
				(*b).second.type = binding_t::JOYSTICK_AXIS;
				(*b).second.joystickAxisNegative = true;
				(*b).second.joystickAxis = strtol((const char*)(type + 5), nullptr, 10);
				return;
			} else if (strncmp(type, "Axis+", 5) == 0) {
				(*b).second.type = binding_t::JOYSTICK_AXIS;
				(*b).second.joystickAxisNegative = false;
				(*b).second.joystickAxis = strtol((const char*)(type + 5), nullptr, 10);
				return;
			} else if (strncmp(type, "Hat", 3) == 0) {
				(*b).second.type = binding_t::JOYSTICK_HAT;
				(*b).second.joystickHat = strtol((const char*)(type + 3), nullptr, 10);
				if (type[3]) {
					if (strncmp((const char*)(type + 4), "LeftUp", 6) == 0) {
						(*b).second.joystickHatState = SDL_HAT_LEFTUP;
						return;
					} else if (strncmp((const char*)(type + 4), "Up", 2) == 0) {
						(*b).second.joystickHatState = SDL_HAT_UP;
						return;
					} else if (strncmp((const char*)(type + 4), "RightUp", 7) == 0) {
						(*b).second.joystickHatState = SDL_HAT_RIGHTUP;
						return;
					} else if (strncmp((const char*)(type + 4), "Right", 5) == 0) {
						(*b).second.joystickHatState = SDL_HAT_RIGHT;
						return;
					} else if (strncmp((const char*)(type + 4), "RightDown", 9) == 0) {
						(*b).second.joystickHatState = SDL_HAT_RIGHTDOWN;
						return;
					} else if (strncmp((const char*)(type + 4), "Down", 4) == 0) {
						(*b).second.joystickHatState = SDL_HAT_DOWN;
						return;
					} else if (strncmp((const char*)(type + 4), "LeftDown", 8) == 0) {
						(*b).second.joystickHatState = SDL_HAT_LEFTDOWN;
						return;
					} else if (strncmp((const char*)(type + 4), "Left", 4) == 0) {
						(*b).second.joystickHatState = SDL_HAT_LEFT;
						return;
					} else if (strncmp((const char*)(type + 4), "Centered", 8) == 0) {
						(*b).second.joystickHatState = SDL_HAT_CENTERED;
						return;
					} else {
						(*b).second.type = binding_t::INVALID;
						return;
					}
				}
			} else {
				(*b).second.type = binding_t::INVALID;
				return;
			}
		}

		return;
	} else if (len >= 5 && strncmp(input, "Mouse", 5) == 0) {
		// mouse
		(*b).second.type = binding_t::MOUSE_BUTTON;
		if ( (strncmp((const char*)(input + 5), "WheelUp", 7) == 0) )
		{
			(*b).second.mouseButton = SDL_BUTTON_WHEELUP;
			return;
		}
		else if ( (strncmp((const char*)(input + 5), "WheelDown", 9) == 0) )
		{
			(*b).second.mouseButton = SDL_BUTTON_WHEELDOWN;
			return;
		}
		Uint32 index = strtol((const char*)(input + 5), nullptr, 10);
		int result = std::min(std::max(0U, index), 4U);
		(*b).second.mouseButton = result;
		return;
	} else {
		// keyboard
		(*b).second.type = binding_t::KEYBOARD;
		(*b).second.scancode = getScancodeFromName(input);
		return;
	}
}

void Input::update() {
	for (auto& pair : bindings) {
		auto& binding = pair.second;
		float oldAnalog = binding.analog;
		binding.analog = analogOf(binding);
		bool oldBinary = binding.binary;
		binding.binary = binaryOf(binding);
		if (oldBinary != binding.binary) {
			// unconsume the input whenever it's released or pressed again.
			if ( oldBinary && !binding.binary && !binding.consumed )
			{
				// detected a 'rising' edge of the binding being released
				binding.binaryRelease = true;
			}
			else
			{
				binding.binaryRelease = false;
			}

			binding.binaryReleaseConsumed = false;
			binding.consumed = false;

			if ( binding.binary && binding.binaryHeldTicks == 0 )
			{
				// start the held detection counter
				binding.binaryHeldTicks = ticks;
			}
			else if ( !binding.binary )
			{
				// button not pressed, reset the held counter
				binding.binaryHeldTicks = 0;
			}
		}
		
		const bool analogHigh = binding.analog > analogToggleThreshold;
		if ( (oldAnalog <= analogToggleThreshold && analogHigh)
			|| (oldAnalog > analogToggleThreshold && !analogHigh))
		{
			binding.analogConsumed = false;
			if ( analogHigh && binding.analogHeldTicks == 0 )
			{
				// start the held detection counter
				binding.analogHeldTicks = ticks;
			}
			else if ( !analogHigh )
			{
				// button not pressed, reset the held counter
				binding.analogHeldTicks = 0;
			}
		}
		else if ( analogHigh )
		{
			if ( binding.analogConsumed && (ticks - binding.analogHeldTicks) > BUTTON_ANALOG_REPEAT_TICKS )
			{
				binding.analogConsumed = false;
				binding.analogHeldTicks = ticks;
			}
		}
	}
}

void Input::updateReleasedBindings()
{
	for ( auto& pair : bindings ) 
	{
		pair.second.binaryReleaseConsumed = true;
	}
}

bool Input::binaryOf(binding_t& binding) {
	if (binding.type == binding_t::CONTROLLER_AXIS ||
		binding.type == binding_t::CONTROLLER_BUTTON) {
		SDL_GameController* pad = binding.pad;
		if (binding.type == binding_t::CONTROLLER_BUTTON) {
			return SDL_GameControllerGetButton(pad, binding.padButton) == 1;
		} else {
			if (binding.padAxisNegative) {
				return SDL_GameControllerGetAxis(pad, binding.padAxis) < -16384;
			} else {
				return SDL_GameControllerGetAxis(pad, binding.padAxis) > 16384;
			}
		}
	} else if (
		binding.type == binding_t::JOYSTICK_AXIS ||
		binding.type == binding_t::JOYSTICK_BUTTON ||
		binding.type == binding_t::JOYSTICK_HAT) {
		SDL_Joystick* joystick = binding.joystick;
		if (binding.type == binding_t::JOYSTICK_BUTTON) {
			return SDL_JoystickGetButton(joystick, binding.joystickButton) == 1;
		} else if (binding.type == binding_t::JOYSTICK_AXIS) {
			if (binding.joystickAxisNegative) {
				return SDL_JoystickGetAxis(joystick, binding.joystickAxis) < -16384;
			} else {
				return SDL_JoystickGetAxis(joystick, binding.joystickAxis) > 16384;
			}
		} else {
			return SDL_JoystickGetHat(joystick, binding.joystickHat) == binding.joystickHatState;
		}
	} else if (binding.type == binding_t::MOUSE_BUTTON) {
		return mouseButtons[binding.mouseButton];
	} else if (binding.type == binding_t::KEYBOARD) {
		SDL_Scancode key = binding.scancode;
		if (key != SDL_SCANCODE_UNKNOWN) {
			return keys[(int)key];
		}
	}

	return false;
}

float Input::analogOf(binding_t& binding) {
	if (binding.type == binding_t::CONTROLLER_AXIS ||
		binding.type == binding_t::CONTROLLER_BUTTON) {
		SDL_GameController* pad = binding.pad;
		if (binding.type == binding_t::CONTROLLER_BUTTON) {
			return SDL_GameControllerGetButton(pad, binding.padButton) ? 1.f : 0.f;
		} else {
			if (binding.padAxisNegative) {
				float result = std::min(SDL_GameControllerGetAxis(pad, binding.padAxis) / 32768.f, 0.f) * -1.f;
				return (fabs(result) > deadzone) ? result : 0.f;
			} else {
				float result = std::max(SDL_GameControllerGetAxis(pad, binding.padAxis) / 32767.f, 0.f);
				return (fabs(result) > deadzone) ? result : 0.f;
			}
		}
	} else if (
		binding.type == binding_t::JOYSTICK_AXIS ||
		binding.type == binding_t::JOYSTICK_BUTTON ||
		binding.type == binding_t::JOYSTICK_HAT) {
		SDL_Joystick* joystick = binding.joystick;
		if (binding.type == binding_t::JOYSTICK_BUTTON) {
			return SDL_JoystickGetButton(joystick, binding.joystickButton) ? 1.f : 0.f;
		} else if (binding.type == binding_t::JOYSTICK_AXIS) {
			if (binding.joystickAxisNegative) {
				float result = std::min(SDL_JoystickGetAxis(joystick, binding.joystickAxis) / 32768.f, 0.f) * -1.f;
				return (fabs(result) > deadzone) ? result : 0.f;
			} else {
				float result = std::max(SDL_JoystickGetAxis(joystick, binding.joystickAxis) / 32767.f, 0.f);
				return (fabs(result) > deadzone) ? result : 0.f;
			}
		} else {
			return SDL_JoystickGetHat(joystick, binding.joystickHat) == binding.joystickHatState ? 1.f : 0.f;
		}
	} else if (binding.type == binding_t::MOUSE_BUTTON) {
		return mouseButtons[binding.mouseButton] ? 1.f : 0.f;
	} else if (binding.type == binding_t::KEYBOARD) {
		SDL_Scancode key = binding.scancode;
		if (key != SDL_SCANCODE_UNKNOWN) {
			return keys[(int)key] ? 1.f : 0.f;
		}
	}

	return 0.f;
}

SDL_Scancode Input::getScancodeFromName(const char* name) {
	auto search = scancodeNames.find(name);
	if (search == scancodeNames.end()) {
		SDL_Scancode scancode = SDL_GetScancodeFromName(name);
		if (scancode != SDL_SCANCODE_UNKNOWN) {
			scancodeNames.emplace(name, scancode);
		}
		return scancode;
	} else {
		return (*search).second;
	}
}