/*
 *  The ManaPlus Client
 *  Copyright (C) 2010  The Mana Developers
 *  Copyright (C) 2011-2015  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/popups/beingpopup.h"

#include "being/being.h"
#include "being/homunculusinfo.h"
#include "being/petinfo.h"
#include "being/playerinfo.h"
#include "being/playerrelations.h"

#include "gui/gui.h"

#include "gui/fonts/font.h"

#include "gui/widgets/label.h"

#include "resources/chatobject.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "debug.h"

BeingPopup *beingPopup = nullptr;

BeingPopup::BeingPopup() :
    Popup("BeingPopup", "beingpopup.xml"),
    mBeingName(new Label(this, "A")),
    mBeingParty(new Label(this, "A")),
    mBeingGuild(new Label(this, "A")),
    mBeingRank(new Label(this, "A")),
    mBeingComment(new Label(this, "A")),
    mBeingBuyBoard(new Label(this, "A")),
    mBeingSellBoard(new Label(this, "A")),
    mBeingRoom(new Label(this, "A"))
{
    // Being Name
    mBeingName->setFont(boldFont);
    mBeingName->setPosition(0, 0);

    const int fontHeight = mBeingName->getHeight();
    setMinHeight(fontHeight);

    // Being's party
    mBeingParty->setPosition(0, fontHeight);
    // Being's party
    mBeingGuild->setPosition(0, 2 * fontHeight);
    mBeingRank->setPosition(0, 3 * fontHeight);
    mBeingBuyBoard->setPosition(0, 4 * fontHeight);
    mBeingSellBoard->setPosition(0, 4 * fontHeight);
    mBeingComment->setPosition(0, 5 * fontHeight);
    mBeingRoom->setPosition(0, 6 * fontHeight);

    mBeingParty->setForegroundColorAll(getThemeColor(Theme::POPUP),
        getThemeColor(Theme::POPUP_OUTLINE));
    mBeingGuild->setForegroundColorAll(getThemeColor(Theme::POPUP),
        getThemeColor(Theme::POPUP_OUTLINE));
    mBeingRank->setForegroundColorAll(getThemeColor(Theme::POPUP),
        getThemeColor(Theme::POPUP_OUTLINE));
    mBeingBuyBoard->setForegroundColorAll(getThemeColor(Theme::POPUP),
        getThemeColor(Theme::POPUP_OUTLINE));
    mBeingSellBoard->setForegroundColorAll(getThemeColor(Theme::POPUP),
        getThemeColor(Theme::POPUP_OUTLINE));
    mBeingComment->setForegroundColorAll(getThemeColor(Theme::POPUP),
        getThemeColor(Theme::POPUP_OUTLINE));
    mBeingRoom->setForegroundColorAll(getThemeColor(Theme::POPUP),
        getThemeColor(Theme::POPUP_OUTLINE));
}

BeingPopup::~BeingPopup()
{
}

void BeingPopup::postInit()
{
    add(mBeingName);
    add(mBeingParty);
    add(mBeingGuild);
    add(mBeingRank);
    add(mBeingComment);
    add(mBeingBuyBoard);
    add(mBeingSellBoard);
    add(mBeingRoom);
}

void BeingPopup::show(const int x, const int y, Being *const b)
{
    if (!b)
    {
        setVisible(false);
        return;
    }

    Label *label1 = mBeingParty;
    Label *label2 = mBeingGuild;
    Label *label3 = mBeingRank;
    Label *label4 = mBeingBuyBoard;
    Label *label5 = mBeingSellBoard;
    Label *label6 = mBeingComment;
    Label *label7 = mBeingRoom;

    b->updateComment();

    if (b->getType() == ActorType::Npc && b->getComment().empty())
    {
        setVisible(false);
        return;
    }

    mBeingName->setCaption(b->getName() + b->getGenderSignWithSpace());
    if (gui)
    {
        if (player_relations.isGoodName(b))
            mBeingName->setFont(boldFont);
        else
            mBeingName->setFont(gui->getSecureFont());
    }
    if (b->isAdvanced())
    {
        mBeingName->setForegroundColorAll(getThemeColor(
            Theme::PLAYER_ADVANCED), getThemeColor(
            Theme::PLAYER_ADVANCED_OUTLINE));
    }
    else
    {
        mBeingName->setForegroundColorAll(getThemeColor(Theme::POPUP),
            getThemeColor(Theme::POPUP_OUTLINE));
    }

    mBeingName->adjustSize();
    label1->setCaption("");
    label2->setCaption("");
    label3->setCaption("");
    label4->setCaption("");
    label5->setCaption("");
    label6->setCaption("");
    label7->setCaption("");

#ifdef EATHENA_SUPPORT
    const ActorType::Type type = b->getType();
    if (type == ActorType::Pet)
    {
        PetInfo *const info = PlayerInfo::getPet();
        if (info)
        {
            // TRANSLATORS: being popup label
            label1->setCaption(strprintf(_("Hungry: %d"),
                info->hungry));
            label1->adjustSize();
            // TRANSLATORS: being popup label
            label2->setCaption(strprintf(_("Intimacy: %d"),
                info->intimacy));
            label2->adjustSize();
            label3 = nullptr;
            label4 = nullptr;
            label5 = nullptr;
            label6 = nullptr;
            label7 = nullptr;
        }
    }
    else if (type == ActorType::Homunculus)
    {
        HomunculusInfo *const info = PlayerInfo::getHomunculus();
        if (info)
        {
            // TRANSLATORS: being popup label
            label1->setCaption(strprintf(_("Hungry: %d"),
                info->hungry));
            label1->adjustSize();
            // TRANSLATORS: being popup label
            label2->setCaption(strprintf(_("Intimacy: %d"),
                info->intimacy));
            label2->adjustSize();
            label3 = nullptr;
            label4 = nullptr;
            label5 = nullptr;
            label6 = nullptr;
            label7 = nullptr;
        }
    }
    else
#endif
    {
        if (!(b->getPartyName().empty()))
        {
            // TRANSLATORS: being popup label
            label1->setCaption(strprintf(_("Party: %s"),
                b->getPartyName().c_str()));
            label1->adjustSize();
        }
        else
        {
            label7 = label6;
            label6 = label5;
            label5 = label4;
            label4 = label3;
            label3 = label2;
            label2 = label1;
            label1 = nullptr;
        }

        if (!(b->getGuildName().empty()))
        {
            // TRANSLATORS: being popup label
            label2->setCaption(strprintf(_("Guild: %s"),
                b->getGuildName().c_str()));
            label2->adjustSize();
        }
        else
        {
            label7 = label6;
            label6 = label5;
            label5 = label4;
            label4 = label3;
            label3 = label2;
            label2 = nullptr;
        }

        if (b->getPvpRank() > 0)
        {
            // TRANSLATORS: being popup label
            label3->setCaption(strprintf(_("Pvp rank: %u"),
                b->getPvpRank()));
            label3->adjustSize();
        }
        else
        {
            label7 = label6;
            label6 = label5;
            label5 = label4;
            label4 = label3;
            label3 = nullptr;
        }

        if (!b->getBuyBoard().empty())
        {
            // TRANSLATORS: being popup label
            label4->setCaption(strprintf(_("Buy shop: %s"),
                b->getBuyBoard().c_str()));
            label4->adjustSize();
        }
        else
        {
            label7 = label6;
            label6 = label5;
            label5 = label4;
            label4 = nullptr;
        }

        if (!b->getSellBoard().empty())
        {
            // TRANSLATORS: being popup label
            label5->setCaption(strprintf(_("Sell shop: %s"),
                b->getSellBoard().c_str()));
            label5->adjustSize();
        }
        else
        {
            label7 = label6;
            label6 = label5;
            label5 = nullptr;
        }

        if (!b->getComment().empty())
        {
            // TRANSLATORS: being popup label
            label6->setCaption(strprintf(_("Comment: %s"),
                b->getComment().c_str()));
            label6->adjustSize();
        }
        else
        {
            label7 = label6;
            label6 = nullptr;
        }
#ifdef EATHENA_SUPPORT
        const ChatObject *const chat = b->getChat();
        if (chat)
        {
            // TRANSLATORS: being popup label
            label7->setCaption(strprintf(_("Chat room: %s"),
                chat->title.c_str()));
            label7->adjustSize();
        }
        else
        {
            label7 = nullptr;
        }
#endif
    }

    int minWidth = mBeingName->getWidth();
    if (label1 && label1->getWidth() > minWidth)
        minWidth = label1->getWidth();
    if (label2 && label2->getWidth() > minWidth)
        minWidth = label2->getWidth();
    if (label3 && label3->getWidth() > minWidth)
        minWidth = label3->getWidth();
    if (label4 && label4->getWidth() > minWidth)
        minWidth = label4->getWidth();
    if (label5 && label5->getWidth() > minWidth)
        minWidth = label5->getWidth();
    if (label6 && label6->getWidth() > minWidth)
        minWidth = label6->getWidth();
    if (label7 && label7->getWidth() > minWidth)
        minWidth = label7->getWidth();

    const int height1 = getFont()->getHeight();
    int height = height1;
    if (label1)
        height += height1;
    if (label2)
        height += height1;
    if (label3)
        height += height1;
    if (label4)
        height += height1;
    if (label5)
        height += height1;
    if (label6)
        height += height1;
    if (label7)
        height += height1;

    setContentSize(minWidth, height);
    position(x, y);
    return;
}

#ifdef USE_PROFILER
void BeingPopup::logic()
{
    BLOCK_START("BeingPopup::logic")
    logicChildren();
    BLOCK_END("BeingPopup::logic")
}
#endif
