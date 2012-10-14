/*
 *  The ManaPlus Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#ifndef TAB_H
#define TAB_H

#include <guichan/basiccontainer.hpp>
#include <guichan/mouselistener.hpp>

#include <guichan/widgets/label.hpp>

#include <guichan/widgetlistener.hpp>

#include "localconsts.h"

class GraphicsVertexes;
class ImageRect;
class Label;
class Skin;
class TabbedArea;
class Theme;

/**
 * A tab, the same as the Guichan tab in 0.8, but extended to allow
 * transparency.
 */
class Tab : public gcn::BasicContainer,
            public gcn::MouseListener,
            public gcn::WidgetListener
{
    public:
        Tab();

        A_DELETE_COPY(Tab);

        ~Tab();

        /**
         * Update the alpha value to the graphic components.
         */
        void updateAlpha();

        /**
         * Draw the tabbed area.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Set the normal color for the tab's text.
         */
        void setTabColor(const gcn::Color *const color)
        { mTabColor = color; }

        /**
         * Set the highlighted color for the tab's text.
         */
        void setHighlightedTabColor(const gcn::Color *const color)
        { mTabHighlightedColor = color; }

        /**
         * Set the selected color for the tab's text.
         */
        void setSelectedTabColor(const gcn::Color *const color)
        { mTabSelectedColor = color; }

        /**
         * Set the flash color for the tab's text.
         */
        void setFlashTabColor(const gcn::Color *const color)
        { mFlashColor = color; }

        /**
         * Set the player flash color for the tab's text.
         */
        void setPlayerFlashTabColor(const gcn::Color *const color)
        { mPlayerFlashColor = color; }

        /**
         * Set tab flashing state
         */
        void setFlash(const int flash)
        { mFlash = flash; }

        int getFlash() const
        { return mFlash; }

        void widgetResized(const gcn::Event &event) override;

        void widgetMoved(const gcn::Event &event) override;

        void setLabelFont(gcn::Font *const font);

        Label *getLabel() const
        { return mLabel; }

        void adjustSize();

        void setTabbedArea(TabbedArea* tabbedArea);

        TabbedArea* getTabbedArea();

        void setCaption(const std::string& caption);

        const std::string &getCaption() const;

        void mouseEntered(gcn::MouseEvent &mouseEvent) override;

        void mouseExited(gcn::MouseEvent &mouseEvent) override;

    protected:
        friend class TabbedArea;

        virtual void setCurrent()
        { }

        Label* mLabel;

        bool mHasMouse;

        TabbedArea* mTabbedArea;

    private:
        /** Load images if no other instances exist yet */
        void init();

        static Skin *tabImg[4];    /**< Tab state graphics */
        static int mInstances;     /**< Number of tab instances */
        static float mAlpha;

        const gcn::Color *mTabColor;
        const gcn::Color *mTabHighlightedColor;
        const gcn::Color *mTabSelectedColor;
        const gcn::Color *mFlashColor;
        const gcn::Color *mPlayerFlashColor;
        int mFlash;
        GraphicsVertexes *mVertexes;
        bool mRedraw;
        int mMode;
};

#endif
