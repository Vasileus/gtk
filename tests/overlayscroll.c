/* simple.c
 * Copyright (C) 1997  Red Hat, Inc
 * Author: Elliot Lee
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */
#include <gtk/gtk.h>

typedef struct {
  GtkWidget *rv;
  gint64 last_scroll_time;
  gboolean over;
  gboolean dragging;
} IndicatorData;

IndicatorData *
get_indicator_data (GtkWidget *sb)
{
  IndicatorData *data;

  data = (IndicatorData *) g_object_get_data (G_OBJECT (sb), "indicator-data");

  if (!data)
    {
      data = g_new0 (IndicatorData, 1);
      g_object_set_data_full (G_OBJECT (sb), "indicator-data", data, g_free);
    }

  return data;
}

static gboolean
conceil_scrollbar (gpointer data)
{
  GtkWidget *sb = data;
  IndicatorData *id = get_indicator_data (sb);

  if (g_get_monotonic_time () - id->last_scroll_time >= 1000000 &&
      !id->over &&
      !id->dragging)
    {
      if (gtk_revealer_get_reveal_child (GTK_REVEALER (id->rv)))
        gtk_revealer_set_reveal_child (GTK_REVEALER (id->rv), FALSE);
    }

  return G_SOURCE_CONTINUE;
}

static void
value_changed (GtkAdjustment *adj, GtkWidget *sb)
{
  IndicatorData *id = get_indicator_data (sb);

  if (!gtk_revealer_get_reveal_child (GTK_REVEALER (id->rv)))
    gtk_revealer_set_reveal_child (GTK_REVEALER (id->rv), TRUE);

  id->last_scroll_time = g_get_monotonic_time ();  
}

static gboolean
enter_notify (GtkWidget *sb, GdkEventCrossing *event)
{
  GtkStyleContext *context = gtk_widget_get_style_context (sb);
  IndicatorData *id = get_indicator_data (sb);

  gtk_style_context_add_class (context, "locked");
  gtk_widget_queue_resize (sb);
  id->over = TRUE;

  return G_SOURCE_CONTINUE;
}

static gboolean
leave_notify (GtkWidget *sb, GdkEventCrossing *event)
{
  GtkStyleContext *context = gtk_widget_get_style_context (sb);
  IndicatorData *id = get_indicator_data (sb);

  gtk_style_context_remove_class (context, "locked");
  gtk_widget_queue_resize (sb);
  id->over = FALSE;

  return G_SOURCE_CONTINUE;
}

static void
style_changed (GtkStyleContext *context, GtkWidget *sb)
{
  IndicatorData *id = get_indicator_data (sb);

  if (gtk_style_context_has_class (context, "dragging"))
    id->dragging = TRUE;
  else
    id->dragging = FALSE;
}

static gchar *
get_content (void)
{
  GString *s;
  gint i;

  s = g_string_new ("");
  for (i = 1; i <= 150; i++)
    g_string_append_printf (s, "Line %d\n", i);

  return g_string_free (s, FALSE);
}

static const gchar data[] =
  ".scrollbar.overlay-indicator.dragging,"
  ".scrollbar.overlay-indicator.locked {"
  "   -GtkRange-slider-width: 15;"
  "}"
  ".scrollbar.overlay-indicator.dragging.trough,"
  ".scrollbar.overlay-indicator.locked.trough {"
  "   background-color: alpha(black,0.1);"
  "}"
  ".scrollbar.overlay-indicator {"
  "   -GtkRange-slider-width: 10;"
  "}"
  ".scrollbar.overlay-indicator.trough {"
  "   background-color: transparent;"
  "}";

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  gchar *content;
  GtkWidget *box;
  GtkWidget *sw;
  GtkWidget *tv;
  GtkWidget *ov;
  GtkWidget *sb;
  GtkWidget *rv;
  GtkAdjustment *adj;
  GtkCssProvider *provider;
  IndicatorData *id;

  gtk_init (&argc, &argv);

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider, data, -1, NULL);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 20);
  gtk_container_add (GTK_CONTAINER (window), box);

  ov = gtk_overlay_new ();
  gtk_box_pack_start (GTK_BOX (box), ov, TRUE, TRUE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_NEVER,
                                  GTK_POLICY_EXTERNAL);

  gtk_container_add (GTK_CONTAINER (ov), sw);

  content = get_content ();

  tv = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (tv), GTK_WRAP_WORD);
  gtk_container_add (GTK_CONTAINER (sw), tv);
  gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv)),
                            content, -1);
  g_free (content);

  adj = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (tv));

  sb = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, adj);

  rv = gtk_revealer_new ();
  gtk_revealer_set_transition_type (GTK_REVEALER (rv), GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
  gtk_revealer_set_transition_duration (GTK_REVEALER (rv), 750);
  gtk_container_add (GTK_CONTAINER (rv), sb);
  gtk_overlay_add_overlay (GTK_OVERLAY (ov), rv);
  gtk_widget_set_halign (rv, GTK_ALIGN_END);
  gtk_widget_set_valign (rv, GTK_ALIGN_FILL);

  id = get_indicator_data (sb);
  id->rv = rv;

  gtk_style_context_add_class (gtk_widget_get_style_context (sb), "overlay-indicator");
  g_signal_connect (sb, "enter-notify-event", G_CALLBACK (enter_notify), NULL);
  g_signal_connect (sb, "leave-notify-event", G_CALLBACK (leave_notify), NULL);
  g_signal_connect (gtk_widget_get_style_context (sb), "changed", G_CALLBACK (style_changed), sb);
  g_signal_connect (adj, "value-changed", G_CALLBACK (value_changed), sb);
  g_timeout_add (500, conceil_scrollbar, sb);

#if 1
  sb = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, adj);
  gtk_container_add (GTK_CONTAINER (box), sb);
#endif

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
