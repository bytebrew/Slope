/*
 * Copyright (C) 2015  Elvis Teixeira
 *
 * This source code is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any
 * later version.
 *
 * This source code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "slope/figure_p.h"
#include "slope/metrics_p.h"
#include "slope/item.h"
#include "slope/list.h"
#include <cairo.h>
#include <cairo-svg.h>
#include <cairo-pdf.h>
#include <stdlib.h>

slope_figure_t* slope_figure_create()
{
    slope_figure_t *figure = malloc(sizeof(slope_figure_t));
    figure->metrics = NULL;
    figure->change_callback = NULL;
    slope_color_set_name(&figure->back_color, SLOPE_WHITE);
    figure->fill_back = SLOPE_TRUE;
    return figure;
}


void slope_figure_destroy (slope_figure_t *figure)
{
    if (figure == NULL) {
        return;
    }
    slope_list_destroy(figure->metrics);
    free(figure);
}


void slope_figure_add_metrics (slope_figure_t *figure,
                              slope_metrics_t *metrics)
{
    metrics->figure = figure;
    figure->metrics = slope_list_append(figure->metrics,
                                       metrics);
    if (figure->change_callback) {
        (*figure->change_callback)(NULL);
    }
}


slope_list_t* slope_figure_get_metrics_list (slope_figure_t *figure)
{
    if (figure == NULL) {
        return NULL;
    }
    return figure->metrics;
}


void slope_figure_draw (slope_figure_t *figure, cairo_t *cr,
                       const slope_rect_t *rect)
{
    cairo_stroke(cr);
    cairo_save(cr);
    /* use an easy font */
    cairo_select_font_face(cr, "Sans",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 11);
    slope_cairo_rectangle(cr, rect);
    cairo_clip(cr);
    /* fill background */
    if (figure->fill_back) {
        slope_cairo_set_color(cr, &figure->back_color);
        cairo_paint(cr);
    }
    cairo_stroke(cr);

    /* draw contents */
    slope_iterator_t *met_iter =
        slope_list_first(figure->metrics);
    while (met_iter) {
        slope_metrics_t *met = (slope_metrics_t*)
            slope_iterator_item(met_iter);
        if (slope_metrics_get_visible(met)) {
            __slope_metrics_draw(met, cr, rect);
        }
        slope_iterator_next(&met_iter);
    }
    cairo_restore(cr);
}


int slope_figure_write_to_png (slope_figure_t *figure,
                               const char *filename,
                               int width, int height)
{
    int success = SLOPE_TRUE;

    cairo_surface_t *surf = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surf);

    slope_rect_t rect;
    slope_rect_set(&rect, 0.0, 0.0, width, height);
    slope_figure_draw(figure, cr, &rect);
    if (cairo_surface_write_to_png(surf, filename) != CAIRO_STATUS_SUCCESS)
        success = SLOPE_FALSE;

    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    return success;
}


int slope_figure_write_to_svg (slope_figure_t *figure,
                               const char *filename,
                               int width, int height)
{
    int success = SLOPE_TRUE;

    cairo_surface_t *surf = cairo_svg_surface_create(
        filename, width, height);

    if (cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS)
        success = SLOPE_FALSE;

    cairo_t *cr = cairo_create(surf);
    slope_rect_t rect;
    slope_rect_set(&rect, 0.0, 0.0, width, height);
    slope_figure_draw(figure, cr, &rect);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    return success;
}

int slope_figure_write_to_pdf (slope_figure_t *figure,
                               const char *filename,
                               slope_paper_size_t paper_size,
                               slope_paper_orientation_t orientation)
{
    int success = SLOPE_TRUE;
    int width, height;

    switch (paper_size)
    {
    case SLOPE_A0:
        width = 2384;
        height = 3371;
        break;
    case SLOPE_A1:
        width = 1685;
        height = 2384;
        break;
    case SLOPE_A2:
        width = 1190;
        height = 1684;
        break;
    case SLOPE_A3:
        width = 842;
        height = 1190;
        break;
    case SLOPE_A4:
        width = 595;
        height = 842;
        break;
    case SLOPE_LETTER:
        width = 612;
        height = 792;
        break;
    case SLOPE_B4:
        width = 420;
        height = 595;
        break;
    case SLOPE_B5:
        width = 516;
        height = 729;
        break;
    }
    if (orientation == SLOPE_LANDSCAPE)
    {
        int tmp = width;
        width = height;
        height = tmp;
    }

    cairo_surface_t *surf = cairo_pdf_surface_create(
        filename, width, height);

    if (cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS)
        success = SLOPE_FALSE;

    cairo_t *cr = cairo_create(surf);
    slope_rect_t rect;
    slope_rect_set(&rect, 0.0, 0.0, width, height);
    slope_figure_draw(figure, cr, &rect);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    return success;
}


void slope_figure_set_change_callback (slope_figure_t *figure,
                                      slope_callback_t callback)
{
    if (figure == NULL) {
        return;
    }
    figure->change_callback = callback;
}


void slope_figure_notify_appearence_change (slope_figure_t *figure,
                                           slope_item_t *item)
{
    (void) item;
    if (figure == NULL) {
        return;
    }
    if (figure->change_callback) {
        (*figure->change_callback)(figure);
    }
}


void slope_figure_notify_item_change (slope_figure_t *figure,
                                     slope_item_t *item)
{
    if (figure == NULL) {
        return;
    }
    slope_metrics_t *metrics = slope_item_get_metrics(item);
    slope_metrics_update(metrics);
    if (figure->change_callback) {
        (*figure->change_callback)(figure);
    }
}

/* slope/figure.h */