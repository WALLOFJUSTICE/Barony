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

static void genericgui_deselect_fn(Widget& widget) {
	if ( widget.isSelected() )
	{
		if ( !inputs.getVirtualMouse(widget.getOwner())->draw_cursor )
		{
			widget.deselect();
		}
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
	animSubmit = 0.0;
	animAction = 0.0;

	ascensionType = ASCENSION_SPELL;

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
	if ( wasOpen )
	{
		if ( inputs.getUIInteraction(playernum)->selectedItem )
		{
			inputs.getUIInteraction(playernum)->selectedItem = nullptr;
			inputs.getUIInteraction(playernum)->toggleclick = false;
		}
		inputs.getUIInteraction(playernum)->selectedItemFromChest = 0;
	}
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
			eternalShrine->skill[6] = 0;
			serverUpdateEntitySkill(eternalShrine, 6);
		}
	}

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

void GenericGUIMenu::EternalShrineGUI_t::openEternalShrine()
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
			animSubmit = 0.0;
			animAction = 0.0;

			ascensionType = ASCENSION_SPELL;

			pipsTarget = 0;
			pipsAdd = 0;
			pipsTick = 0;
			pipsFlashTick = 0;
			pipsAnimThisTick = 0;
			pipsAddSpeed = 0;

			isInteractable = false;
			bFirstTimeSnapCursor = false;
		}
		selectEternalShrineSlot(ETERNALSHRINE_SLOT_SEND, 0);
		player->hud.compactLayoutMode = Player::HUD_t::COMPACT_LAYOUT_INVENTORY;
		player->inventory_mode = INVENTORY_MODE_ITEM;
		bOpen = true;
	}
	if ( inputs.getUIInteraction(playernum)->selectedItem )
	{
		inputs.getUIInteraction(playernum)->selectedItem = nullptr;
		inputs.getUIInteraction(playernum)->toggleclick = false;
	}
	inputs.getUIInteraction(playernum)->selectedItemFromChest = 0;
	clearItemDisplayed();
}

void GenericGUIMenu::EternalShrineGUI_t::updateEternalShrine()
{
	const int playernum = parentGUI.getPlayer();
	auto player = players[playernum];

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
			if ( player->entity && (entityDist(player->entity, eternalShrineStation) > TOUCHRANGE) )
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

	bool viewActionReady = false;
	if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
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
		if ( submittedItem != EternalShrineSubmitStatus::SUBMIT_DONE )
		{
			isInteractable = false;
		}
		if ( animFilter > 0.01 )
		{
			isInteractable = false;
		}

		if ( pipsQueued > 0 )
		{
			pipsTotal += pipsQueued;
			pipsQueued = 0;
			pipsAddSpeed = 20;
			pipsTotal = std::min(10, pipsTotal);
		}

		if ( animx >= .9999 && isInteractable )
		{
			viewActionReady = true;
			if ( sendItem1Uid == 0 )
			{
				animSendItem1 = 0.0;
			}

			const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
			real_t setpointDiffX = fpsScale * std::max(.01, (1.0 - animAction)) / 10.0;
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
	if ( animx >= .9999 )
	{
		const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
		if ( keystatus[SDLK_g] )
		{
			keystatus[SDLK_g] = 0;
			if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
			{
				currentView = ASSIST_SHRINE_VIEW_ACTION;
				submittedItem = EternalShrineSubmitStatus::SUBMIT_DONE;
			}
			else
			{
				currentView = ASSIST_SHRINE_VIEW_OFFERING;
				submittedItem = EternalShrineSubmitStatus::SUBMIT_NONE;
			}
		}
		if ( keystatus[SDLK_h] )
		{
			keystatus[SDLK_h] = 0;
			if ( keystatus[SDLK_LSHIFT] )
			{
				pipsTotal -= 1;
				pipsTotal = std::max(0, pipsTotal);
			}
			else
			{
				pipsTotal += 1;
				if ( pipsTotal > 10 )
				{
					pipsTotal = 0;
				}
			}
		}
		if ( keystatus[SDLK_j] )
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
		}

		if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
		{
			real_t setpointDiffX = fpsScale * std::max(.01, (1.0 - animFilter)) / 10.0;
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
			real_t setpointDiffX = fpsScale * std::max(.01, animFilter) / 10.0;
			animFilter -= setpointDiffX;
			animFilter = std::max(0.0, animFilter);
		}

		shroudTopFrame->setOpacity(animFilter * 100.0);

		shroudBadge->pos.y = shroudTopFrame->getSize().h - shroudBadge->pos.h;
		shroudBadge->pos.y += animSubmit * 16;
		//shroudBadge->pos.y += (animSubmit) * 16 * sin(PI * (ticks % 200 / 100.0));

		if ( currentView == ASSIST_SHRINE_VIEW_OFFERING )
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
		offeringPrompt->setDisabled(false);
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
	if ( sendItem1Uid != 0 )
	{
		if ( eternalShrineSendItem1 = uidToItem(sendItem1Uid) )
		{
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
			title->setText(Language::get(6983));
		}
		else
		{
			title->setText("");
		}

		SDL_Rect textPos{ 0, 21, baseFrame->getSize().w, 24 };
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
		if ( closeBtn->isDisabled() && usingGamepad && sendItem1Uid == 0 )
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
	actionBtn->setText(Language::get(7001));

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
			pos.x = baseFramePos.w / 2 - pos.w / 2;
			pos.y = 260;
			actionBtn->setSize(pos);

			SDL_Color color;
			getColor(actionBtn->getColor(), &color.r, &color.g, &color.b, &color.a);

			color.a = (Uint8)(255 * alphaRatio);
			actionBtn->setColor(makeColor(color.r, color.g, color.b, color.a));
			actionBtn->setHighlightColor(makeColor(255, 255, 255, color.a));
			actionBtn->setTextColor(actionBtn->getColor());
			actionBtn->setTextHighlightColor(makeColor(201, 162, 100, 255 * alphaRatio));

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

				ascensionImg->pos.x = 88;
				ascensionImg->pos.y = 194;
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
				if ( !isInteractable )
				{
					SDL_Color color;
					getColor(hudColors.characterSheetFaintText, &color.r, &color.g, &color.b, &color.a);
					color.a = (Uint8)(255 * alphaRatio);
					actionBtn->setTextColor(makeColor(color.r, color.g, color.b, color.a));
				}
				else
				{
					actionBtn->setTextColor(makeColor(255, 255, 255, 255 * alphaRatio));
				}
				actionBtn->setDisabled(true);
				if ( isInteractable )
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

		offeringBtn->setInvisible(false);

		auto pos = offeringBtn->getSize();
		pos.x = baseFramePos.w / 2 - pos.w / 2;
		pos.y = 250;
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
			if ( !isInteractable/*potionResultFrame->isDisabled() || alchemyMissingIngredientQty(nullptr)*/ )
			{
				SDL_Color color;
				getColor(hudColors.characterSheetFaintText, &color.r, &color.g, &color.b, &color.a);
				color.a = (Uint8)(255 * alphaRatio);
				offeringBtn->setTextColor(makeColor(color.r, color.g, color.b, color.a));
			}
			else
			{
				offeringBtn->setTextColor(makeColor(255, 255, 255, 255 * alphaRatio));
			}
			offeringBtn->setDisabled(true);
			if ( isInteractable/*!potionResultFrame->isDisabled() && !alchemyMissingIngredientQty(nullptr)*/ )
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

	if ( itemType != -1 && itemDesc.size() > 1 
		&& (submittedItem == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE
			|| currentView == ASSIST_SHRINE_VIEW_ACTION) )
	{
		if ( isInteractable )
		{
			//const real_t fpsScale = getFPSScale(50.0); // ported from 50Hz
			//real_t setpointDiffX = fpsScale * std::max(.01, (1.0 - animTooltip)) / 2.0;
			//animTooltip += setpointDiffX;
			//animTooltip = std::min(1.0, animTooltip);
			animTooltip = 1.0;
			animTooltipTicks = ticks;
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

	bool tryBrew = false;
	bool activateSelection = false;
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
					parentGUI.closeGUI();
					Player::soundCancel();
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

	if ( activateSelection && players[playernum] && players[playernum]->entity
		/*&& animRecvItem < 0.001*/ )
	{
		if ( itemActionType != ETERNAL_ITEM_OK && itemActionType != ETERNAL_ITEM_NONE )
		{
			playSound(90, 64);
		}
		if ( (player->GUI.activeModule == Player::GUI_t::MODULE_ETERNALSHRINE || (tryBrew && player->GUI.activeModule == Player::GUI_t::MODULE_INVENTORY))
			&& (itemActionType == ETERNAL_ITEM_OK || tryBrew)
			)
		{
			ItemType oldPotion1Type = WOODEN_SHIELD;
			ItemType oldPotion2Type = WOODEN_SHIELD;
			if ( (getSelectedEternalShrineX() >= ETERNALSHRINE_SLOT_SEND && getSelectedEternalShrineX() < 0
				&& getSelectedEternalShrineY() == 0) || tryBrew )
			{
				if ( !tryBrew && getSelectedEternalShrineX() == ETERNALSHRINE_SLOT_SEND )
				{
					sendItem1Uid = 0;
					animSendItem1 = 0.0;
					animSendItem1Frame->setDisabled(true);
					Player::soundCancel();
				}
				//else if ( tryBrew || getSelectedMailSlotX() == MAIL_SLOT_RECV )
				//{
				//	// recv it
				//	if ( !recvItemFrame->isDisabled()
				//		&& mailReceiveItem.type != POTION_EMPTY
				//		&& recvItemUid == 0 )
				//	{
				//		recvItemUid = 0;

				//		parentGUI.mailboxClaimItem();

				//		if ( recvItemUid != 0 )
				//		{
				//			if ( auto item = uidToItem(recvItemUid) )
				//			{
				//				auto slotType = player->paperDoll.getSlotForItem(*item);
				//				if ( recvItemUid == sendItem1Uid )
				//				{
				//					animRecvItemDestX = animSendItem1DestX;
				//					animRecvItemDestY = animSendItem1DestY;
				//					animRecvItem = 1.0;
				//				}
				//				else if ( slotType != Player::PaperDoll_t::SLOT_MAX ) // on paper doll
				//				{
				//					animRecvItemDestX = animRecvItemStartX;
				//					animRecvItemDestY = animRecvItemStartY - player->inventoryUI.getSlotSize();
				//					animRecvItem = 1.0;
				//				}
				//				else if ( auto slotFrame = player->inventoryUI.getInventorySlotFrame(item->x, item->y) )
				//				{
				//					getInventoryItemAlchemyAnimSlotPos(slotFrame, player, item->x, item->y, animRecvItemDestX, animRecvItemDestY, mailItemAnimOffsetY);
				//					animRecvItemDestY += 2;
				//					animRecvItemDestY += (player->inventoryUI.bCompactView ? -2 : 0);
				//					animRecvItem = 1.0;
				//				}
				//			}
				//		}
				//		if ( animRecvItem < .999 )
				//		{
				//			recvItemUid = 0;
				//		}
				//	}
				//}
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
		SDL_Rect offeringBtnPos{ 0, 0, 188, 26 };
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
			if ( gui.submittedItem == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE )
			{
				if ( gui.sendItem1Uid != 0 )
				{
					if ( Item* item = uidToItem(gui.sendItem1Uid) )
					{
						consumeItem(item, player);
						gui.sendItem1Uid = 0;
						gui.submittedItem = GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_WAITING;
						gui.submitTick = ticks;
						gui.pipsQueued += 2;
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
			/*if ( gui.submittedItem == GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_NONE )
			{
				if ( gui.sendItem1Uid != 0 )
				{
					if ( Item* item = uidToItem(gui.sendItem1Uid) )
					{
						consumeItem(item, player);
						gui.sendItem1Uid = 0;
						GenericGUI[player].eternalShrineGUI.submittedItem = GenericGUIMenu::EternalShrineGUI_t::EternalShrineSubmitStatus::SUBMIT_WAITING;
						GenericGUI[player].eternalShrineGUI.submitTick = ticks;
					}
				}
			}*/
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
					int type = gui.ascensionType;
					--type;
					if ( type < EternalShrineAscensionType::ASCENSION_SPELL )
					{
						type = EternalShrineAscensionType::THAUMATURGY_SPELL;
					}
					gui.ascensionType = (EternalShrineAscensionType)type;
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
					int type = gui.ascensionType;
					++type;
					if ( type > EternalShrineAscensionType::THAUMATURGY_SPELL )
					{
						type = 0;
					}
					gui.ascensionType = (EternalShrineAscensionType)type;
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
	if ( item->status == BROKEN )
	{
		return false;
	}

	if ( item->type == READABLE_BOOK || itemCategory(item) == SCROLL )
	{
		return true;
	}

	if ( parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
	{
		if ( itemCategory(item) == SPELL_CAT )
		{
			return true;
		}
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
	if ( !bSendItemAllowed )
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
	if ( itemCategory(item) == SCROLL || item->type == READABLE_BOOK )
	{
		bool isEquipped = itemIsEquipped(item, player);
		if ( (!item->identified || isEquipped) )
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
	else if ( item->type == SPELL_ITEM && parentGUI.guiType == GUICurrentType::GUI_TYPE_ETERNALSHRINE_ASCENSION )
	{
		resultAction = ETERNAL_ITEM_OK;
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