/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#include "net/tmwa/npchandler.h"

#include "localplayer.h"

#include "gui/npcdialog.h"
#include "gui/viewport.h"

#include "net/messagein.h"
#include "net/net.h"
#include "net/npchandler.h"

#include "net/tmwa/protocol.h"

#include "net/ea/eaprotocol.h"

#include "debug.h"

extern Net::NpcHandler *npcHandler;

namespace TmwAthena
{

NpcHandler::NpcHandler() :
    mRequestLang(false)
{
    static const Uint16 _messages[] =
    {
        SMSG_NPC_CHOICE,
        SMSG_NPC_MESSAGE,
        SMSG_NPC_NEXT,
        SMSG_NPC_CLOSE,
        SMSG_NPC_INT_INPUT,
        SMSG_NPC_STR_INPUT,
        SMSG_NPC_COMMAND,
        0
    };
    handledMessages = _messages;
    npcHandler = this;
}

void NpcHandler::handleMessage(Net::MessageIn &msg)
{
    int npcId = getNpc(msg, msg.getId() == SMSG_NPC_CHOICE
        || msg.getId() == SMSG_NPC_MESSAGE);

    if (msg.getId() != SMSG_NPC_STR_INPUT)
        mRequestLang = false;

    switch (msg.getId())
    {
        case SMSG_NPC_CHOICE:
            processNpcChoice(msg);
            break;

        case SMSG_NPC_MESSAGE:
            processNpcMessage(msg);
            break;

        case SMSG_NPC_CLOSE:
            processNpcClose(msg);
            break;

        case SMSG_NPC_NEXT:
            processNpcNext(msg);
            break;

        case SMSG_NPC_INT_INPUT:
            processNpcIntInput(msg);
            break;

        case SMSG_NPC_STR_INPUT:
            if (mRequestLang)
                processLangReuqest(msg, npcId);
            else
                processNpcStrInput(msg);
            break;

        case SMSG_NPC_COMMAND:
            processNpcCommand(msg, npcId);
            break;

        default:
            break;
    }

    mDialog = nullptr;
}

void NpcHandler::talk(int npcId)
{
    MessageOut outMsg(CMSG_NPC_TALK);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(0); // Unused
}

void NpcHandler::nextDialog(int npcId)
{
    MessageOut outMsg(CMSG_NPC_NEXT_REQUEST);
    outMsg.writeInt32(npcId);
}

void NpcHandler::closeDialog(int npcId)
{
    MessageOut outMsg(CMSG_NPC_CLOSE);
    outMsg.writeInt32(npcId);

    NpcDialogs::iterator it = mNpcDialogs.find(npcId);
    if (it != mNpcDialogs.end())
    {
        if ((*it).second.dialog)
            (*it).second.dialog->close();
        mNpcDialogs.erase(it);
    }
}

void NpcHandler::listInput(int npcId, unsigned char value)
{
    MessageOut outMsg(CMSG_NPC_LIST_CHOICE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(static_cast<unsigned char>(value));
}

void NpcHandler::integerInput(int npcId, int value)
{
    MessageOut outMsg(CMSG_NPC_INT_RESPONSE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt32(value);
}

void NpcHandler::stringInput(int npcId, const std::string &value)
{
    MessageOut outMsg(CMSG_NPC_STR_RESPONSE);
    outMsg.writeInt16(static_cast<Sint16>(value.length() + 9));
    outMsg.writeInt32(npcId);
    outMsg.writeString(value, static_cast<int>(value.length()));
    outMsg.writeInt8(0); // Prevent problems with string reading
}

void NpcHandler::buy(int beingId)
{
    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(beingId);
    outMsg.writeInt8(0); // Buy
}

void NpcHandler::sell(int beingId)
{
    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(beingId);
    outMsg.writeInt8(1); // Sell
}

void NpcHandler::buyItem(int beingId A_UNUSED, int itemId,
                         unsigned char color, int amount)
{
    MessageOut outMsg(CMSG_NPC_BUY_REQUEST);
    if (serverVersion > 0)
    {
        outMsg.writeInt16(10); // One item (length of packet)
        outMsg.writeInt16(static_cast<Sint16>(amount));
        outMsg.writeInt16(static_cast<Sint16>(itemId));
        outMsg.writeInt8(color);
        outMsg.writeInt8(0);
    }
    else
    {
        outMsg.writeInt16(8); // One item (length of packet)
        outMsg.writeInt16(static_cast<Sint16>(amount));
        outMsg.writeInt16(static_cast<Sint16>(itemId));
    }
}

void NpcHandler::sellItem(int beingId A_UNUSED, int itemId, int amount)
{
    MessageOut outMsg(CMSG_NPC_SELL_REQUEST);
    outMsg.writeInt16(8); // One item (length of packet)
    outMsg.writeInt16(static_cast<Sint16>(itemId + INVENTORY_OFFSET));
    outMsg.writeInt16(static_cast<Sint16>(amount));
}

int NpcHandler::getNpc(Net::MessageIn &msg, bool haveLength)
{
    if (haveLength)
        msg.readInt16();  // length

    const int npcId = msg.readInt32();

    NpcDialogs::const_iterator diag = mNpcDialogs.find(npcId);
    mDialog = nullptr;

    if (diag == mNpcDialogs.end())
    {
        // Empty dialogs don't help
        if (msg.getId() == SMSG_NPC_CLOSE)
        {
            closeDialog(npcId);
            return npcId;
        }
        else if (msg.getId() == SMSG_NPC_NEXT)
        {
            nextDialog(npcId);
            return npcId;
        }
        else
        {
            mDialog = new NpcDialog(npcId);
            if (player_node)
                player_node->stopWalking(false);
            Wrapper wrap;
            wrap.dialog = mDialog;
            mNpcDialogs[npcId] = wrap;
        }
    }
    else
    {
        mDialog = diag->second.dialog;
    }
    return npcId;
}

void NpcHandler::processNpcCommand(Net::MessageIn &msg, int npcId)
{
    const int cmd = msg.readInt16();
    switch (cmd)
    {
        case 0:
            mRequestLang = true;
            break;

        case 1:
            if (viewport)
                viewport->moveCameraToActor(npcId);
            break;

        case 2:
            if (viewport)
            {
                const int id = msg.readInt32();
                const int x = msg.readInt16();
                const int y = msg.readInt16();
                if (!id)
                    viewport->moveCameraToPosition(x, y);
                else
                    viewport->moveCameraToActor(id, x, y);
            }
            break;

        case 3:
            if (viewport)
                viewport->returnCamera();
            break;

        case 4:
            if (viewport)
            {
                msg.readInt32(); // id
                const int x = msg.readInt16();
                const int y = msg.readInt16();
                viewport->moveCameraRelative(x, y);
            }
            break;

        default:
            logger->log("unknown npc command: %d", cmd);
            break;
    }
}

void NpcHandler::processLangReuqest(Net::MessageIn &msg A_UNUSED, int npcId)
{
    mRequestLang = false;
    stringInput(npcId, getLangSimple());
}

} // namespace TmwAthena
