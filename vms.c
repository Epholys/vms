/*! \file
 *  \brief vms definitions.
 */

#include <stdio.h>
#include "vms.h"

vms *vms_construct(uint16_t width, uint16_t height, uint16_t px_size)
{
    xcb_connection_t    *connection;
    xcb_screen_t        *screen;
    xcb_drawable_t       window;
    xcb_gcontext_t       foreground;
    uint32_t             mask = 0;
    uint32_t             values[2];

    if(width % 8 != 0 || height % 8 != 0)
    {
        fprintf(stderr, "Screen's dimensions are not multiples of 8");
        return NULL;
    }

    /* Open the connection to the X server */
    connection = xcb_connect(NULL, NULL);
    
    /* Get the first screen */
    screen = xcb_setup_roots_iterator( xcb_get_setup(connection) ).data;
 
    /* Create black (foreground) graphic context */
    window = screen->root;

    foreground = xcb_generate_id(connection);

    mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    values[0] = screen->black_pixel;
    values[1] = 0;
    xcb_create_gc(connection, foreground, window, mask, values);
    
    /* Create the window */
    window = xcb_generate_id(connection);
    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    values[0] = screen->white_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE;
    xcb_create_window (connection,                      /* Connection    */
                       screen->root_depth,              /* depth         */
                       window,                          /* window Id     */
                       screen->root,                    /* parent window */
                       0, 0,                            /* x, y          */
                       width*px_size, height*px_size,   /* width, height */
                       0,                               /* border_width  */
                       XCB_WINDOW_CLASS_INPUT_OUTPUT,   /* class         */
                       screen->root_visual,             /* visual        */
                       mask, values);                   /* masks         */

    /* Map the window on the screen */
    xcb_map_window(connection, window);
     
    /* We flush the request */
    xcb_flush (connection);
    
    vms *vscreen;
    vscreen = malloc(sizeof(vms));
    vscreen->connection = connection;
    vscreen->window     = window;
    vscreen->foreground = foreground;
    vscreen->width      = width;
    vscreen->height     = height;
    vscreen->px_size    = px_size;
    vscreen->bitmap = calloc(width*height/8, 1);
    vscreen->first_draw = true;

    return vscreen;
}

void vms_free(vms *vscreen)
{
    xcb_disconnect(vscreen->connection);
    free(vscreen->bitmap);
    free(vscreen);
    vscreen = NULL;
}

void vms_clear(vms *vscreen)
{
    memset(vscreen->bitmap, 0, vscreen->width * vscreen->height / 8);
    xcb_clear_area(vscreen->connection,
                   0,
                   vscreen->window,
                   0, 0,
                   vscreen->width, vscreen->height);
}

void vms_draw(vms *vscreen)
{
    uint32_t n_px = vscreen->width * vscreen->height;
    xcb_generic_event_t *event;

    uint16_t width = vscreen->width;
    uint16_t px_size = vscreen->px_size;

    
    DYNARR_DECLARE(xcb_rectangle_t, pixels);
    DYNARR_CONSTRUCT(xcb_rectangle_t, &pixels);

    for(uint32_t j=0; j<n_px/8; ++j)
    {
        for(uint8_t i=0; i<8; ++i)
        {
            if(vscreen->bitmap[j] & (1 << i))
            {
                xcb_rectangle_t rect = {(j*8 + i) % width * px_size,
                                        (j*8 + i) / width * px_size,
                                        px_size, px_size};
                DYNARR_PUSH(&pixels, rect);
            }
        }
    }

    while(vscreen->first_draw && (event = xcb_wait_for_event(vscreen->connection))) {
        switch(event->response_type & ~0x80)
        {
        case XCB_EXPOSE:
            vscreen->first_draw = false;
            break;
        default:
            break;
        }
        free(event);
    }

    xcb_poly_fill_rectangle(vscreen->connection,
                            vscreen->window,
                            vscreen->foreground,
                            pixels.size,
                            pixels.elements);

    xcb_flush(vscreen->connection);
    
    DYNARR_DESTRUCT(&pixels);
}


void vms_px_set(vms *vscreen, uint16_t x, uint16_t y)
{
    if(x < vscreen->width && y < vscreen->height)
        vscreen->bitmap[(x + vscreen->width*y) / 8] ^= (1 << (x % 8));
}
