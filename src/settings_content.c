/*
 * ConnMan GTK GUI
 *
 * Copyright (C) 2015 Intel Corporation. All rights reserved.
 * Author: Jaakko Hannikainen <jaakko.hannikainen@intel.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "settings.h"
#include "settings_content.h"
#include "settings_content_callback.h"
#include "style.h"
#include "util.h"

gboolean never_write(struct settings_content *content)
{
	return FALSE;
}

gboolean always_write(struct settings_content *content)
{
	return TRUE;
}

gboolean write_if_selected(struct settings_content *content)
{
	return  !!g_object_get_data(G_OBJECT(content->content), "selected");
}

static gboolean content_valid_always(struct settings_content *content)
{
	return TRUE;
}

static GVariant *content_value_null(struct settings_content *content)
{
	return NULL;
}

void settings_add_content(struct settings_page *page,
                          struct settings_content *content)
{
	gtk_grid_attach(GTK_GRID(page->grid), content->content,
	                0, page->index, 1, 1);
	page->index++;
}

static GVariant *content_value_entry(struct settings_content *content)
{
	GtkWidget *entry = g_object_get_data(G_OBJECT(content->content),
	                                     "entry");
	const gchar *str = gtk_entry_get_text(GTK_ENTRY(entry));
	GVariant *var = g_variant_new("s", str);
	return var;
}

static GVariant *content_value_switch(struct settings_content *content)
{
	GtkWidget *toggle = g_object_get_data(G_OBJECT(content->content),
	                                      "toggle");
	gboolean active = gtk_switch_get_active(GTK_SWITCH(toggle));
	GVariant *var = g_variant_new("b", active);
	return var;
}

static GVariant *content_value_list(struct settings_content *content)
{
	GtkWidget *list = g_object_get_data(G_OBJECT(content->content), "box");
	GtkComboBox *box = GTK_COMBO_BOX(list);
	GVariant *var = g_variant_new("s", gtk_combo_box_get_active_id(box));
	return var;
}

void free_content(GtkWidget *widget, gpointer user_data)
{
	struct settings_content *content = user_data;
	content->free(content);
}

static GtkWidget *create_label(const gchar *text)
{
	GtkWidget *label;
	label = gtk_label_new(text);

	STYLE_ADD_CONTEXT(label);
	gtk_style_context_add_class(gtk_widget_get_style_context(label),
	                            "dim-label");
	gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
	return label;
}

static struct settings_content *create_base_content(struct settings *sett,
						    settings_writable writable,
						    const gchar *key,
						    const gchar *subkey)
{
	struct settings_content *content = g_malloc(sizeof(*content));
	content->content = gtk_grid_new();
	content->valid = content_valid_always;
	content->value = content_value_null;
	content->free = g_free;
	content->sett = sett;
	content->writable = writable;
	content->key = key;
	content->subkey = subkey;

	g_object_set_data(G_OBJECT(content->content), "content", content);

	g_signal_connect(content->content, "destroy",
	                 G_CALLBACK(free_content), content);

	gtk_widget_set_margin_bottom(content->content, MARGIN_SMALL);

	gtk_grid_set_column_homogeneous(GTK_GRID(content->content), TRUE);

	return content;
}

static void add_left_aligned(GtkGrid *grid, GtkWidget *a, GtkWidget *b)
{
	gtk_widget_set_margin_start(a, MARGIN_LARGE);
	gtk_widget_set_margin_end(a, MARGIN_SMALL);
	gtk_widget_set_margin_start(b, MARGIN_SMALL);

	gtk_widget_set_halign(a, GTK_ALIGN_START);
	gtk_widget_set_halign(b, GTK_ALIGN_START);
	gtk_widget_set_hexpand(a, TRUE);
	gtk_widget_set_hexpand(b, TRUE);

	gtk_grid_attach(grid, a, 0, 0, 1, 1);
	gtk_grid_attach(grid, b, 1, 0, 1, 1);
}

GtkWidget *settings_add_text(struct settings_page *page, const gchar *label,
                             const gchar *key, const gchar *subkey)
{
	GtkWidget *label_w, *value_w;
	gchar *value;
	struct settings_content *content;

	content = create_base_content(NULL, never_write, NULL, NULL);

	value = service_get_property_string(page->sett->serv, key, subkey);

	label_w = create_label(label);
	value_w = gtk_label_new(value);

	g_free(value);

	g_object_set_data(G_OBJECT(content->content), "value", value_w);

	gtk_widget_set_hexpand(content->content, TRUE);
	label_align_text_left(GTK_LABEL(value_w));

	add_left_aligned(GTK_GRID(content->content), label_w, value_w);

	settings_add_content(page, content);

	if(key) {
		struct content_callback *cb;
		cb = create_text_callback(value_w);
		settings_set_callback(page->sett, key, subkey, cb);
	}

	return value_w;
}

GtkWidget *settings_add_entry(struct settings *sett, struct settings_page *page,
			      settings_writable writable, const gchar *label,
                              const gchar *key, const gchar *subkey,
                              const gchar *ekey, const gchar *esubkey,
                              settings_field_validator valid)
{
	GtkWidget *label_w, *entry;
	gchar *value;
	struct settings_content *content;

	content = create_base_content(sett, writable, ekey, esubkey);
	if(valid)
		content->valid = valid;
	content->free = g_free;
	content->value = content_value_entry;

	label_w = create_label(label);
	entry = gtk_entry_new();

	value = service_get_property_string(page->sett->serv, key, subkey);
	gtk_entry_set_text(GTK_ENTRY(entry), value);
	g_free(value);
	g_object_set_data(G_OBJECT(content->content), "entry", entry);

	add_left_aligned(GTK_GRID(content->content), label_w, entry);

	gtk_widget_show_all(content->content);

	settings_add_content(page, content);
	hash_table_set_dual_key(sett->contents, key, subkey, content);

	return entry;
}

GtkWidget *settings_add_switch(struct settings *sett,
			       struct settings_page *page,
			       settings_writable writable, const gchar *label,
                               const gchar *key, const gchar *subkey)
{
	GtkWidget *label_w, *toggle;
	struct settings_content *content;
	gboolean value;

	content = create_base_content(sett, writable, key, subkey);
	content->value = content_value_switch;
	value = service_get_property_boolean(page->sett->serv, key, subkey);

	label_w = create_label(label);
	toggle = gtk_switch_new();

	gtk_switch_set_active(GTK_SWITCH(toggle), value);

	g_object_set_data(G_OBJECT(content->content), "toggle", toggle);

	add_left_aligned(GTK_GRID(content->content), label_w, toggle);

	gtk_widget_show_all(content->content);

	settings_add_content(page, content);
	hash_table_set_dual_key(sett->contents, key, subkey, content);

	return toggle;
}

static void free_combo_box(void *data)
{
	struct settings_content *content = data;
	GtkWidget *box = g_object_get_data(G_OBJECT(content->content), "box");
	g_hash_table_unref(g_object_get_data(G_OBJECT(box), "items"));
	g_free(content);
}

GtkWidget *settings_add_combo_box(struct settings *sett,
				  struct settings_page *page,
				  settings_writable writable,
				  const gchar *label,
				  const gchar *key, const gchar *subkey,
				  const gchar *ekey, const gchar *esubkey)
{
	struct settings_content *content;
	GtkWidget *label_w, *notebook, *box;
	GHashTable *items;

	content = create_base_content(sett, writable, key, subkey);
	items = g_hash_table_new_full(g_str_hash, g_str_equal,
	                              g_free, NULL);

	content->free = free_combo_box;
	content->value = content_value_list;

	notebook = gtk_notebook_new();
	box = gtk_combo_box_text_new();
	label_w = create_label(label);

	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
	gtk_widget_set_hexpand(notebook, TRUE);
	gtk_widget_set_vexpand(notebook, FALSE);
	gtk_grid_set_row_spacing(GTK_GRID(content->content), MARGIN_LARGE);

	g_object_set_data(G_OBJECT(box), "notebook", notebook);
	g_object_set_data(G_OBJECT(box), "items", items);
	g_object_set_data(G_OBJECT(content->content), "box", box);

	g_signal_connect(box, "changed", G_CALLBACK(combo_box_changed),
	                 notebook);

	add_left_aligned(GTK_GRID(content->content), label_w, box);
	gtk_grid_attach(GTK_GRID(content->content), notebook, 0, 1, 2, 1);

	gtk_widget_show_all(content->content);

	settings_add_content(page, content);
	hash_table_set_dual_key(sett->contents, key, subkey, content);

	if(key) {
		struct content_callback *cb;
		cb = create_list_callback(box);
		settings_set_callback(page->sett, key, subkey, cb);
	}

	return box;
}
