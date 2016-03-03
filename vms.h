/******************************************************************************
 *
 * vms - Monochrome virtual machine screen for X11 using XCB
 * Copyright 2016 Thomas Munoz (epholys@gmail.com)
 *
 * vms is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vms is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with vms. If not, see <http://www.gnu.org/licenses/>.
 * 
 *****************************************************************************/

/*! \file
 *  \brief The vms structs and function declarations.
 *
 * NB: "Virtual pixels" (or "vpx") are the individual pixels on the virtual
 * screen. "Real pixels" (or "rpx") are the pixels on your monitor.
 */

#ifndef VMS_H
#define VMS_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>

#include "lib/dynarr/dynarr.h"

/*! \brief The virtual screen structure, containing all necessary datas.
 *
 * NB: "Virtual pixels" (or "vpx") are the individual pixels on the virtual
 * screen. "Real pixels" (or "rpx") are the pixels on your monitor.
 */
typedef struct
{
    xcb_connection_t *connection;  /*!< \brief Connection to the X11 server. */
	xcb_drawable_t    window;      /*!< \brief X window.                     */
	xcb_gcontext_t    foreground;  /*!< \brief Pixels' black drawing context.*/
	
	uint16_t           width;      /*!< \brief Vscreen's width, in vpx.       */
	uint16_t		   height;     /*!< \brief Vscreen's height, in vpx.      */
	uint16_t           px_size;    /*!< \brief Virtual pixels' size, in rpx. */
	uint8_t           *bitmap;     /*!< \brief 1-bit pixel framebuffer.      */

	bool               first_draw; /*!< \brief Is the window drawn for the 1st
                                    * time? */
} vms;

/* \brief Does all the necessary to create a vms (connects to the X server,
 * creates a window, a graphic context, etc.)
 *
 * \param[in] width The vms' width, in vpx. MUST BE a multiple of 8.
 * \param[in] height The vms' height, in vpx. MUST BE a multiple of 8.
 * \param[in] width The virtual pixels' size, in rpx.
 *
 * \return A vms pointer, to free with vms_free(). If width or height are not
 * valid, will return NULL.
 */
vms *vms_construct(uint16_t width, uint16_t height, uint16_t px_size);

/* \brief Disconnects from the X server, frees \em vscreen's framebuffer and \em
 * vscreen itself.
 *
 * \param[in] vscreen The vms pointer to free.
 */
void vms_free(vms *vscreen);

/* \brief (Re)draws the pixels from \em vscreen's bitmap on the window.
 *
 * \param[in] vscreen The vms to draw.
 */
void vms_draw(vms *vscreen);

/* \brief Clears the vms by memsetting \em vscreen's bitmap to 0.
 *
 * \param[in] vscreen The vms to clear.
 */
void vms_clear(vms *vscreen);

/* \brief Modifies \em vscreen's bitmap to (un)set the pixel at coordinate (x,y).
 *
 * \param[in] vscreen The vms to modify.
 * \param[in] x The x coordinate. If invalid, vms_data_px() does nothing.
 * \param[in] y The y coordinate. If invalid, vms_data_px() does nothing.
 */
void vms_px_set(vms *vscreen, uint16_t x, uint16_t y);

#endif /* VMS_H */
