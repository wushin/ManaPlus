/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2014  The ManaPlus Developers
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

#ifndef USE_SDL2

#include "utils/sdlhelper.h"

#include "logger.h"
#include "main.h"

#include "utils/stringutils.h"

#ifdef USE_X11
#include "utils/glxhelper.h"
#endif  // USE_X11

#include <SDL_syswm.h>
#include <SDL_video.h>

#include "debug.h"

bool SDL::getAllVideoModes(StringVect &modeList)
{
    /* Get available fullscreen/hardware modes */
    SDL_Rect *const *const modes = SDL_ListModes(nullptr,
        SDL_FULLSCREEN | SDL_HWSURFACE);

#ifdef ANDROID
    const std::string modeString =
        toString(static_cast<int>(modes[0]->w)).append("x")
       .append(toString(static_cast<int>(modes[0]->h)));
    logger->log("support mode: " + modeString);
    modeList.push_back(modeString);
    return true;
#else
    /* Check which modes are available */
    if (modes == static_cast<SDL_Rect **>(nullptr))
    {
        logger->log1("No modes available");
        return false;
    }
    else if (modes == reinterpret_cast<SDL_Rect **>(-1))
    {
        logger->log1("All resolutions available");
        return true;
    }
    else
    {
        for (int i = 0; modes[i]; ++ i)
        {
            const std::string modeString =
                toString(static_cast<int>(modes[i]->w)).append("x")
                .append(toString(static_cast<int>(modes[i]->h)));
            logger->log("support mode: " + modeString);
            modeList.push_back(modeString);
        }
        return true;
    }
#endif
}

void SDL::SetWindowTitle(const SDL_Surface *const window A_UNUSED,
                         const char *const title)
{
    SDL_WM_SetCaption(title, nullptr);
}

void SDL::SetWindowIcon(const SDL_Surface *const window A_UNUSED,
                        SDL_Surface *const icon)
{
    SDL_WM_SetIcon(icon, nullptr);
}

void SDL::grabInput(const SDL_Surface *const window A_UNUSED, const bool grab)
{
    SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
}

void SDL::setGamma(const SDL_Surface *const window A_UNUSED, const float gamma)
{
    SDL_SetGamma(gamma, gamma, gamma);
}

void SDL::setVsync(const int val)
{
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, val);
}

bool SDL::getWindowWMInfo(const SDL_Surface *const window A_UNUSED,
                          SDL_SysWMinfo *const info)
{
    return SDL_GetWMInfo(info);
}

SDL_Thread *SDL::createThread(int (SDLCALL *fn)(void *),
                              const char *const name A_UNUSED,
                              void *const data)
{
    return SDL_CreateThread(fn, data);
}

#if defined(USE_X11) && defined(USE_OPENGL)
void *SDL::createGLContext(SDL_Surface *const window A_UNUSED,
                           const int major,
                           const int minor)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWMInfo(&info);
    void *context = GlxHelper::createContext(info.info.x11.window,
        info.info.x11.display, major, minor);
    if (!context && (major > 3 || (major == 3 && minor > 3)))
    {
        logger->log("Try fallback to OpenGL 3.3 context");
        context = GlxHelper::createContext(info.info.x11.window,
            info.info.x11.display, 3, 3);
        if (!context)
        {
            logger->log("Try fallback to OpenGL 3.0 context");
            context = GlxHelper::createContext(info.info.x11.window,
                info.info.x11.display, 3, 0);
        }
    }
    return context;
}

void SDL::makeCurrentContext(void *const context)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWMInfo(&info);
    GlxHelper::makeCurrent(info.info.x11.window,
        info.info.x11.display,
        context);
}
#else
void *SDL::createGLContext(SDL_Surface *const window A_UNUSED,
                           const int major A_UNUSED,
                           const int minor A_UNUSED)
{
    return nullptr;
}

void SDL::makeCurrentContext(void *const context A_UNUSED)
{
}
#endif

#endif  // USE_SDL2
