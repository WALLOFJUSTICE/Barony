/*-------------------------------------------------------------------------------

	BARONY
	File: ui.hpp
	Desc: contains interface related declarations

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include "../main.hpp"
#include "../game.hpp"
#include "../files.hpp"
#include "../draw.hpp"
#include "interface.hpp"
#include "../colors.hpp"

class UIToastNotification
{
	double scalex = 0.25;
	double scaley = 0.25;
	int posx = 260;
	int posy = 110;
	int showHeight = 0;
	int dockHeight = 32;
	SDL_Surface* notificationImage = nullptr;
	bool isInit = false;
	int textx = 8;
	int texty = 0;
	int bodyx = 12;
	int bodyy = 16;
	const int kImageBorderHeight = 256;
	const int kImageBorderWidth = 256;

	bool mainCardHide = false;
	bool mainCardIsHidden = false;
	const int cardWidth = 332;
	int animx = cardWidth;
	int anim_ticks = 0;
	const int anim_duration = 50;

	bool dockedCardHide = true;
	bool dockedCardIsHidden = true;
	const int dockedCardWidth = 48;
	int docked_animx = dockedCardWidth;
	int docked_anim_ticks = 0;
	const int docked_anim_duration = 25;

	Uint32 cardState = UI_CARD_STATE_SHOW;
	bool temporaryCardHide = false;
	bool idleDisappear = true;
	Uint32 lastInteractedTick = 0;

	std::string displayedText;
	std::string mainCardText;
	std::string secondaryCardText;
	std::string headerCardText;
	std::string actionText;

public:
	UIToastNotification(SDL_Surface* image) {
		notificationImage = image;
		showHeight = kImageBorderHeight * scaley + 16;
	};
	~UIToastNotification() {};

	Uint32 actionFlags = 0;
	enum ActionFlags : Uint32
	{
		UI_NOTIFICATION_CLOSE = 1,
		UI_NOTIFICATION_ACTION_BUTTON = 2,
		UI_NOTIFICATION_AUTO_HIDE = 4
	};
	enum CardType : Uint32
	{
		UI_CARD_DEFAULT,
		UI_CARD_EOS_ACCOUNT
	};
	CardType cardType = CardType::UI_CARD_DEFAULT;
	enum CardState : Uint32
	{
		UI_CARD_STATE_SHOW,
		UI_CARD_STATE_DOCKED
	};

	void getDimensions(int& outPosX, int& outPosY, int& outPosW, int& outPosH)
	{
		outPosX = posx;
		outPosY = posy;
		outPosW = cardWidth;
		outPosH = showHeight;
	}
	void setPosY(int y)
	{
		posy = y;
	}
	void setMainText(std::string& text)
	{
		mainCardText = text;
	}
	void setSecondaryText(std::string& text)
	{
		secondaryCardText = text;
	}
	void setHeaderText(std::string& text)
	{
		headerCardText = text;
	}
	void setDisplayedText(std::string& text)
	{
		displayedText = text;
	}
	void setActionText(std::string& text)
	{
		actionText = text;
	}

	void draw()
	{
		if ( !isInit )
		{
			return;
		}

		if ( subwindow || fadeout )
		{
			if ( !fadeout && !(actionFlags & UI_NOTIFICATION_AUTO_HIDE) )
			{
				// don't hide.
			}
			else
			{
				temporaryCardHide = true;
				lastInteractedTick = ticks;
				if ( cardState == UI_CARD_STATE_SHOW )
				{
					mainCardHide = true;
					dockedCardHide = false;
				}
			}
		}
		else
		{
			temporaryCardHide = false;
		}

		if ( cardState == UI_CARD_STATE_SHOW )
		{
			drawMainCard();
		}
		else if ( cardState == UI_CARD_STATE_DOCKED )
		{
			drawDockedCard();
		}
	}

	void drawDockedCard()
	{
		SDL_Rect r;
		r.w = 32;
		r.h = kImageBorderHeight * scaley;
		r.x = xres - r.w + docked_animx;
		r.y = yres - r.h - posy;
		drawWindowFancy(r.x, r.y - 8, xres + 16 + docked_animx, r.y + 24);

		if ( !temporaryCardHide && mouseInBounds(r.x, xres + docked_animx, r.y - 8, r.y + 24) )
		{
			if ( mousestatus[SDL_BUTTON_LEFT] )
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				dockedCardHide = true;
				mainCardHide = false;
				lastInteractedTick = ticks;
			}
		}
		ttfPrintTextColor(ttf16, r.x + 8, r.y + texty,
			SDL_MapRGBA(mainsurface->format, 255, 255, 0, 255), true, "<");

		if ( temporaryCardHide )
		{
			animate(docked_animx, docked_anim_ticks, docked_anim_duration, dockedCardWidth, false, dockedCardIsHidden);
		}
		else
		{
			animate(docked_animx, docked_anim_ticks, docked_anim_duration, dockedCardWidth, dockedCardHide, dockedCardIsHidden);
			if ( dockedCardHide && dockedCardIsHidden )
			{
				cardState = UI_CARD_STATE_SHOW;
			}
		}
	}

	void drawMainCard()
	{
		SDL_Rect r;
		r.w = kImageBorderWidth * scalex;
		r.h = kImageBorderHeight * scaley;
		r.x = xres - r.w - posx + animx + 12;
		r.y = yres - r.h - posy;
		drawWindowFancy(r.x - 8, r.y - 8, xres - 8 + animx, r.y + r.h + 8);
		drawCloseButton(&r);
		ttfPrintTextColor(ttf12, r.x + r.w + textx, r.y + texty, SDL_MapRGBA(mainsurface->format, 255, 255, 0, 255), true,
			headerCardText.c_str());

		ttfPrintTextColor(ttf12, r.x + r.w + bodyx, r.y + bodyy, SDL_MapRGBA(mainsurface->format, 255, 255, 255, 255), true,
			displayedText.c_str());
		drawImageScaled(notificationImage, nullptr, &r);
		drawActionButton(&r);

		if ( temporaryCardHide )
		{
			animate(animx, anim_ticks, anim_duration, cardWidth, true, mainCardIsHidden);
		}
		else
		{
			animate(animx, anim_ticks, anim_duration, cardWidth, mainCardHide, mainCardIsHidden);
			if ( mainCardHide && mainCardIsHidden )
			{
				cardState = UI_CARD_STATE_DOCKED;
				displayedText = mainCardText;
			}
			else
			{
				if ( ticks - lastInteractedTick > TICKS_PER_SECOND * 10 )
				{
					mainCardHide = true;
					dockedCardHide = false;
				}
			}
		}
	}
	void animate(int& xout, int& current_ticks, int duration, int width, bool hideElement, bool& isHidden)
	{
		// scale duration to FPS - tested @ 144hz
		double scaledDuration = (duration / (144.f / std::max(1.0, fps)));

		double t = current_ticks / static_cast<double>(scaledDuration);
		double result = -width * t * t * (3.0f - 2.0f * t); // bezier from 0 to width as t (0-1)
		xout = floor(result) + width;
		isHidden = false;
		if ( hideElement )
		{
			current_ticks = std::max(current_ticks - 1, 0);
			if ( current_ticks == 0 )
			{
				isHidden = true;
			}
		}
		else
		{
			current_ticks = std::min(current_ticks + 1, static_cast<int>(scaledDuration));
		}
	}

	bool drawCloseButton(SDL_Rect* src)
	{
		if ( !(actionFlags & ActionFlags::UI_NOTIFICATION_CLOSE) )
		{
			return false;
		}
		SDL_Rect closeBtn;
		closeBtn.x = xres - 8 - 12 - 8 + animx;
		closeBtn.y = src->y - 8 + 4;
		closeBtn.w = 16;
		closeBtn.h = 16;
		if ( !temporaryCardHide && mouseInBounds(closeBtn.x, closeBtn.x + closeBtn.w, closeBtn.y, closeBtn.y + closeBtn.h) )
		{
			drawDepressed(closeBtn.x, closeBtn.y, closeBtn.x + closeBtn.w, closeBtn.y + closeBtn.h);
			if ( mousestatus[SDL_BUTTON_LEFT] )
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				ttfPrintText(ttf12, closeBtn.x + 1, closeBtn.y + 2, "x");
				mainCardHide = true;
				dockedCardHide = false;
				lastInteractedTick = ticks;
				return true;
			}
		}
		else
		{
			drawWindow(closeBtn.x, closeBtn.y, closeBtn.x + closeBtn.w, closeBtn.y + closeBtn.h);
		}
		ttfPrintText(ttf12, closeBtn.x + 1, closeBtn.y + 2, "x");
		return false;
	}

	bool drawActionButton(SDL_Rect* src)
	{
		SDL_Rect actionBtn;
		actionBtn.x = src->x + 4;
		actionBtn.y = src->y + src->h - 4 - TTF12_HEIGHT;
		actionBtn.w = src->w - 8;
		actionBtn.h = 20;
		if ( !temporaryCardHide && mouseInBounds(actionBtn.x, actionBtn.x + actionBtn.w, actionBtn.y, actionBtn.y + actionBtn.h) )
		{
			//drawDepressed(actionBtn.x, actionBtn.y, actionBtn.x + actionBtn.w, actionBtn.y + actionBtn.h);
			drawWindowFancy(actionBtn.x, actionBtn.y, actionBtn.x + actionBtn.w, actionBtn.y + actionBtn.h);
			drawRect(&actionBtn, uint32ColorBaronyBlue(*mainsurface), 32);
			if ( mousestatus[SDL_BUTTON_LEFT] )
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				ttfPrintText(ttf12, actionBtn.x - 2, actionBtn.y + 5, actionText.c_str());
				lastInteractedTick = ticks;
				return true;
			}
		}
		else
		{
			drawWindowFancy(actionBtn.x, actionBtn.y, actionBtn.x + actionBtn.w, actionBtn.y + actionBtn.h);
			drawRect(&actionBtn, SDL_MapRGB(mainsurface->format, 255, 255, 255), 32);
		}
		ttfPrintText(ttf12, actionBtn.x - 2, actionBtn.y + 5, actionText.c_str());
		return false;
	}
	void init()
	{
		if ( isInit )
		{
			return;
		}
		displayedText = mainCardText;
		mainCardIsHidden = false;
		mainCardHide = false;
		isInit = true;
		lastInteractedTick = ticks;
	}
};


class UIToastNotificationManager_t
{
	SDL_Surface* communityLink1 = nullptr;
public:
	UIToastNotificationManager_t() {};
	~UIToastNotificationManager_t() { SDL_FreeSurface(communityLink1); };
	enum ImageTypes
	{
		GENERIC_TOAST_IMAGE
	};
	SDL_Surface* getImage(ImageTypes image)
	{
		if ( image == GENERIC_TOAST_IMAGE )
		{
			return communityLink1;
		}
		return nullptr;
	};

	bool bIsInit = false;
	void init()
	{
		bIsInit = true;
		communityLink1 = loadImage("images/system/CommunityLink1.png");
	}
	void drawNotifications();

	UIToastNotification* addNotification(ImageTypes image);
	std::vector<UIToastNotification> allNotifications;
};
extern UIToastNotificationManager_t UIToastNotificationManager;