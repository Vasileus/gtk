/* GTK - The GIMP Toolkit
 * Copyright (C) 2014, Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author(s): Carlos Garnacho <carlosg@gnome.org>
 */
#include "config.h"
#include <gtk/gtkgesturedrag.h>

#define CAPTURE_THRESHOLD_MS 150

typedef struct _GtkGestureDragPrivate GtkGestureDragPrivate;
typedef struct _EventData EventData;

struct _GtkGestureDragPrivate
{
  gdouble start_x;
  gdouble start_y;
  gdouble last_x;
  gdouble last_y;
};

enum {
  DRAG_BEGIN,
  DRAG_UPDATE,
  DRAG_END,
  N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE (GtkGestureDrag, gtk_gesture_drag, GTK_TYPE_GESTURE_SINGLE)

static void
gtk_gesture_drag_begin (GtkGesture       *gesture,
                        GdkEventSequence *sequence)
{
  GtkGestureDragPrivate *priv;

  priv = gtk_gesture_drag_get_instance_private (GTK_GESTURE_DRAG (gesture));
  gtk_gesture_get_point (gesture, sequence, &priv->start_x, &priv->start_y);
  g_signal_emit (gesture, signals[DRAG_BEGIN], 0, priv->start_x, priv->start_y);
}

static void
gtk_gesture_drag_update (GtkGesture       *gesture,
                         GdkEventSequence *sequence)
{
  GtkGestureDragPrivate *priv;
  gdouble x, y;

  priv = gtk_gesture_drag_get_instance_private (GTK_GESTURE_DRAG (gesture));
  gtk_gesture_get_point (gesture, sequence, &priv->last_x, &priv->last_y);
  x = priv->last_x - priv->start_x;
  y = priv->last_y - priv->start_y;

  g_signal_emit (gesture, signals[DRAG_UPDATE], 0, x, y);
}

static void
gtk_gesture_drag_end (GtkGesture       *gesture,
                      GdkEventSequence *sequence)
{
  GtkGestureDragPrivate *priv;
  gdouble x, y;

  priv = gtk_gesture_drag_get_instance_private (GTK_GESTURE_DRAG (gesture));
  gtk_gesture_get_point (gesture, sequence, &priv->last_x, &priv->last_y);
  x = priv->last_x - priv->start_x;
  y = priv->last_y - priv->start_y;

  g_signal_emit (gesture, signals[DRAG_END], 0, x, y);
}

static void
gtk_gesture_drag_class_init (GtkGestureDragClass *klass)
{
  GtkGestureClass *gesture_class = GTK_GESTURE_CLASS (klass);

  gesture_class->begin = gtk_gesture_drag_begin;
  gesture_class->update = gtk_gesture_drag_update;
  gesture_class->end = gtk_gesture_drag_end;

  signals[DRAG_BEGIN] =
    g_signal_new ("drag-begin",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkGestureDragClass, drag_begin),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
  signals[DRAG_UPDATE] =
    g_signal_new ("drag-update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkGestureDragClass, drag_update),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
  signals[DRAG_END] =
    g_signal_new ("drag-end",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkGestureDragClass, drag_end),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
}

static void
gtk_gesture_drag_init (GtkGestureDrag *gesture)
{
}

/**
 * gtk_gesture_drag_new:
 * @widget: a #GtkWidget
 *
 * Returns a newly created #GtkGesture that recognizes drags
 *
 * Returns: a newly created #GtkGestureDrag
 *
 * Since: 3.14
 **/
GtkGesture *
gtk_gesture_drag_new (GtkWidget *widget)
{
  return g_object_new (GTK_TYPE_GESTURE_DRAG,
                       "widget", widget,
                       NULL);
}

/**
 * gtk_gesture_drag_get_start_point:
 * @gesture: a #GtkGesture
 * @x: X coordinate for the drag start point
 * @y: Y coordinate for the drag start point
 *
 * If the @gesture is active, this function returns %TRUE
 * and fills in @x and @y with the drag start coordinates,
 * in window-relative coordinates.
 *
 * Returns: #TRUE if the gesture is active.
 *
 * Since: 3.14
 **/
gboolean
gtk_gesture_drag_get_start_point (GtkGestureDrag *gesture,
                                  gint           *x,
                                  gint           *y)
{
  GtkGestureDragPrivate *priv;

  if (!gtk_gesture_is_recognized (GTK_GESTURE (gesture)))
    return FALSE;

  priv = gtk_gesture_drag_get_instance_private (gesture);

  if (x)
    *x = priv->start_x;

  if (y)
    *y = priv->start_y;

  return TRUE;
}

/**
 * gtk_gesture_drag_get_current_point:
 * @gesture: a #GtkGesture
 * @x: X coordinate for the current point
 * @y: Y coordinate for the current point
 *
 * If the @gesture is active, this function returns %TRUE and
 * fills in @x and @y with the coordinates of the current point,
 * as an offset to the starting drag point.
 *
 * Returns: #TRUE if the gesture is active.
 *
 * Since: 3.14
 **/
gboolean
gtk_gesture_drag_get_current_point (GtkGestureDrag *gesture,
                                    gint           *x,
                                    gint           *y)
{
  GtkGestureDragPrivate *priv;

  if (!gtk_gesture_is_recognized (GTK_GESTURE (gesture)))
    return FALSE;

  priv = gtk_gesture_drag_get_instance_private (gesture);

  if (x)
    *x = priv->last_x;

  if (y)
    *y = priv->last_y;

  return TRUE;
}