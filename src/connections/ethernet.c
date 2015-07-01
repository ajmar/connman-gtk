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

#include <gio/gio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "ethernet.h"
#include "technology.h"
#include "service.h"
#include "style.h"

struct ethernet_service {
	struct service parent;

	GtkGrid *header;
	GtkWidget *interface;

	GtkGrid *properties;
	GtkWidget *ipv4;
	GtkWidget *ipv4gateway;
	GtkWidget *ipv6;
	GtkWidget *ipv6gateway;
	GtkWidget *mac;
};

void technology_ethernet_init(struct technology *item, GVariant *properties,
		GDBusProxy *proxy) {
	gtk_image_set_from_icon_name(GTK_IMAGE(item->list_item->icon),
			"network-wired-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_image_set_from_icon_name(GTK_IMAGE(item->settings->icon),
			"network-wired", GTK_ICON_SIZE_DIALOG);
}

static GtkWidget *add_label(GtkGrid *grid, gint y, const gchar *text) {
	GtkWidget *label, *value;

	label = gtk_label_new(text);
	gtk_style_context_add_class(gtk_widget_get_style_context(label), "dim-label");
	gtk_widget_set_hexpand(label, TRUE);
	STYLE_ADD_MARGIN(label, MARGIN_SMALL);
	gtk_widget_set_margin_start(label, MARGIN_LARGE);
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(grid, label, 0, y, 1, 1);

	value = gtk_label_new(NULL);
	g_object_ref(value);
	gtk_widget_set_halign(value, GTK_ALIGN_START);
	gtk_grid_attach(grid, value, 1, y, 1, 1);

	return value;
}

void service_ethernet_init(struct service *serv, GDBusProxy *proxy,
		const gchar *path, GVariant *properties) {
	struct ethernet_service *item = (struct ethernet_service *)serv;
	item->header = GTK_GRID(gtk_grid_new());
	g_object_ref(item->header);
	item->interface = gtk_label_new(NULL);
	g_object_ref(item->interface);
	STYLE_ADD_MARGIN(item->interface, MARGIN_SMALL);
	gtk_widget_set_margin_start(item->interface, MARGIN_LARGE);
	gtk_widget_set_margin_bottom(item->interface, 0);
	gtk_grid_attach(item->header, item->interface,  0, 0, 1, 1);

	item->properties = GTK_GRID(gtk_grid_new());
	g_object_ref(item->properties);
	STYLE_ADD_MARGIN(GTK_WIDGET(item->properties), MARGIN_LARGE);
	gtk_widget_set_margin_top(GTK_WIDGET(item->properties), 0);
	gtk_grid_set_column_homogeneous(item->properties, TRUE);

	item->ipv4 = add_label(item->properties, 0, _("IPv4 address"));
	item->ipv4gateway = add_label(item->properties, 1, _("IPv4 gateway"));
	item->ipv6 = add_label(item->properties, 2, _("IPv6 address"));
	item->ipv6gateway = add_label(item->properties, 3, _("IPv6 gateway"));
	item->mac = add_label(item->properties, 4, _("MAC address"));

	gtk_grid_attach(GTK_GRID(serv->contents), GTK_WIDGET(item->header),
			0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(serv->contents), GTK_WIDGET(item->properties),
			0, 1, 1, 1);

	gtk_widget_show_all(serv->contents);

	service_ethernet_update(serv);
}

struct service *service_ethernet_create(void) {
	struct ethernet_service *serv = g_malloc(sizeof(*serv));
	return (struct service *)serv;
}

void service_ethernet_free(struct service *serv) {
	struct ethernet_service *item = (struct ethernet_service *)serv;
	g_object_unref(item->interface);
	g_object_unref(item->header);
	g_object_unref(item->properties);
	g_object_unref(item->ipv4);
	g_object_unref(item->ipv4gateway);
	g_object_unref(item->ipv6);
	g_object_unref(item->ipv6gateway);
	g_object_unref(item->mac);
	g_free(item);
}

static void service_ethernet_set_property(struct service *serv, GtkWidget *label,
		const gchar *key, const gchar *subkey) {
	GVariant *variant;
	const gchar *value;

	variant = service_get_property(serv, key, subkey);
	if(variant) {
		value = g_variant_get_string(variant, NULL);
		gtk_label_set_text(GTK_LABEL(label), value);
		g_variant_unref(variant);
	}
}

void service_ethernet_update(struct service *serv) {
	struct ethernet_service *item = (struct ethernet_service *)serv;

	service_ethernet_set_property(serv, item->ipv4, "IPv4", "Address");
	service_ethernet_set_property(serv, item->ipv4gateway, "IPv4", "Gateway");
	service_ethernet_set_property(serv, item->ipv6, "IPv6", "Address");
	service_ethernet_set_property(serv, item->ipv6gateway, "IPv6", "Gateway");
	service_ethernet_set_property(serv, item->interface, "Ethernet", "Interface");
	service_ethernet_set_property(serv, item->mac, "Ethernet", "Address");
}
