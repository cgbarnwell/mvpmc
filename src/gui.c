/*
 *  Copyright (C) 2004, 2005, Jon Gettler
 *  http://mvpmc.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ident "$Id$"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#ifndef MVPMC_HOST
#include <sys/reboot.h>
#include <linux/reboot.h>
#endif

#include <mvp_widget.h>
#include <mvp_av.h>
#include <mvp_demux.h>
#include <mvp_osd.h>

#include "mvpmc.h"
#include "replaytv.h"
#include "colorlist.h"

volatile int running_replaytv = 0;
volatile int mythtv_livetv = 0;

#ifdef MVPMC_HOST
#define FONT_STANDARD	0
#define FONT_LARGE	0
#else
#define FONT_STANDARD	1000
#define FONT_LARGE	1001
#endif

static mvpw_menu_attr_t fb_attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_BLACK,
	.hilite_fg = MVPW_WHITE,
	.hilite_bg = MVPW_DARKGREY2,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_BLUE,
	.border_size = 2,
	.border = MVPW_DARKGREY2,
};

static mvpw_menu_attr_t settings_attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_BLACK,
	.hilite_fg = MVPW_WHITE,
	.hilite_bg = MVPW_DARKGREY2,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_BLUE,
	.border_size = 2,
	.border = MVPW_DARKGREY2,
};

static mvpw_menu_attr_t themes_attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.bg = MVPW_BLACK,
	.hilite_fg = MVPW_WHITE,
	.hilite_bg = MVPW_DARKGREY2,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_BLUE,
	.border_size = 2,
	.border = MVPW_DARKGREY2,
	.checkbox_fg = MVPW_GREEN,
	.checkboxes = 1,
};

mvpw_menu_attr_t mythtv_attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_BLACK,
	.bg = MVPW_LIGHTGREY,
	.hilite_fg = MVPW_WHITE,
	.hilite_bg = MVPW_DARKGREY2,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_BLUE,
	.border_size = 2,
	.border = MVPW_DARKGREY2,
};

static mvpw_menu_attr_t attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.bg = MVPW_BLACK,
	.hilite_fg = MVPW_BLACK,
	.hilite_bg = MVPW_WHITE,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_BLUE,
	.rounded = 1,
	.border_size = 0,
	.border = MVPW_BLACK,
};

static mvpw_menu_attr_t myth_main_attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.bg = MVPW_BLACK,
	.hilite_fg = MVPW_BLACK,
	.hilite_bg = MVPW_WHITE,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_BLUE,
	.border = MVPW_WHITE,
	.border_size = 0,
	.rounded = 1,
};

static mvpw_menu_item_attr_t popup_item_attr = {
	.selectable = 1,
	.fg = mvpw_color_alpha(MVPW_BLACK, 0x80),
	.bg = mvpw_color_alpha(MVPW_DARK_ORANGE, 0x80),
	.checkbox_fg = mvpw_color_alpha(MVPW_PURPLE, 0x80),
};

static mvpw_menu_item_attr_t mythtv_popup_item_attr = {
	.selectable = 1,
	.fg = MVPW_GREEN,
	.bg = MVPW_BLACK,
};

static mvpw_menu_item_attr_t item_attr = {
	.selectable = 1,
	.fg = MVPW_WHITE,
	.bg = MVPW_BLACK,
	.checkbox_fg = MVPW_GREEN,
};

static mvpw_menu_item_attr_t myth_menu_item_attr = {
	.selectable = 1,
	.fg = MVPW_WHITE,
	.bg = MVPW_BLACK,
	.checkbox_fg = MVPW_GREEN,
};

static mvpw_menu_item_attr_t settings_item_attr = {
	.selectable = 1,
	.fg = MVPW_BLACK,
	.bg = MVPW_LIGHTGREY,
	.checkbox_fg = MVPW_GREEN,
};

static mvpw_menu_item_attr_t themes_item_attr = {
	.selectable = 1,
	.fg = MVPW_GREEN,
	.bg = MVPW_BLACK,
	.checkbox_fg = MVPW_GREEN,
};

static mvpw_menu_item_attr_t sub_settings_item_attr = {
	.selectable = 1,
	.fg = MVPW_BLACK,
	.bg = MVPW_LIGHTGREY,
	.checkbox_fg = MVPW_GREEN,
};

static mvpw_menu_attr_t popup_attr = {
	.font = FONT_STANDARD,
	.fg = mvpw_color_alpha(MVPW_BLACK, 0x80),
	.bg = mvpw_color_alpha(MVPW_DARK_ORANGE, 0x80),
	.hilite_fg = mvpw_color_alpha(MVPW_BLACK, 0x80),
	.hilite_bg = mvpw_color_alpha(MVPW_WHITE, 0x80),
	.title_fg = mvpw_color_alpha(MVPW_WHITE, 0x80),
	.title_bg = mvpw_color_alpha(MVPW_DARKGREY, 0x80),
	.border = MVPW_WHITE,
	.border_size = 0,
};

static mvpw_menu_attr_t mythtv_popup_attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.hilite_fg = MVPW_BLACK,
	.hilite_bg = MVPW_WHITE,
	.title_fg = MVPW_BLACK,
	.title_bg = MVPW_LIGHTGREY,
	.border = MVPW_DARKGREY2,
	.border_size = 2,
};

static mvpw_text_attr_t mythtv_info_attr = {
	.wrap = 1,
	.justify = MVPW_TEXT_LEFT,
	.margin = 4,
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.border = MVPW_BLACK,
	.border_size = 0,
};

static mvpw_text_attr_t description_attr = {
	.wrap = 1,
	.justify = MVPW_TEXT_LEFT,
	.margin = 4,
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.bg = MVPW_BLACK,
	.border = MVPW_BLACK,
	.border_size = 0,
};

static mvpw_text_attr_t display_attr = {
	.wrap = 0,
	.justify = MVPW_TEXT_LEFT,
	.margin = 6,
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.bg = mvpw_color_alpha(MVPW_BLACK, 0x80),
	.border = MVPW_BLACK,
	.border_size = 0,
};

static mvpw_text_attr_t mythtv_program_attr = {
	.wrap = 0,
	.justify = MVPW_TEXT_LEFT,
	.margin = 6,
	.font = FONT_STANDARD,
	.fg = MVPW_CYAN,
	.bg = mvpw_color_alpha(MVPW_BLACK, 0x80),
	.border = MVPW_BLACK,
	.border_size = 0,
};

static mvpw_text_attr_t mythtv_description_attr = {
	.wrap = 1,
	.justify = MVPW_TEXT_LEFT,
	.margin = 6,
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.bg = mvpw_color_alpha(MVPW_BLACK, 0x80),
	.border = MVPW_BLACK,
	.border_size = 0,
};

static mvpw_text_attr_t splash_attr = {
	.wrap = 1,
	.justify = MVPW_TEXT_CENTER,
	.margin = 6,
	.font = FONT_STANDARD,
	.fg = MVPW_GREEN,
	.bg = MVPW_BLACK,
	.border = MVPW_BLACK,
	.border_size = 0,
};

static mvpw_text_attr_t ct_text_box_attr = {
	.wrap = 1,
	.justify = MVPW_TEXT_CENTER,
	.margin = 6,
	.font = FONT_LARGE,
};

static mvpw_text_attr_t ct_fgbg_box_attr = {
	.wrap = 1,
	.justify = MVPW_TEXT_LEFT,
	.margin = 6,
	.font = FONT_LARGE,
	.fg = MVPW_GREEN,
};

static mvpw_dialog_attr_t warn_attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.bg = MVPW_DARKGREY,
	.title_fg = MVPW_BLACK,
	.title_bg = MVPW_WHITE,
	.modal = 1,
	.border = MVPW_RED,
	.border_size = 4,
};

static mvpw_dialog_attr_t about_attr = {
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.bg = MVPW_DARKGREY,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_DARKGREY,
	.modal = 1,
	.border = MVPW_BLUE,
	.border_size = 2,
};

static mvpw_graph_attr_t offset_graph_attr = {
	.min = 0,
	.max = 100,
	.fg = mvpw_color_alpha(MVPW_RED, 0x80),
};

static mvpw_graph_attr_t busy_graph_attr = {
	.min = 0,
	.max = 10,
	.gradient = 1,
	.left = MVPW_BLACK,
	.right = MVPW_RED,
};

static mvpw_text_attr_t busy_text_attr = {
	.wrap = 0,
	.justify = MVPW_TEXT_CENTER,
	.margin = 6,
	.font = FONT_STANDARD,
	.fg = MVPW_WHITE,
	.border = MVPW_DARKGREY2,
	.border_size = 2,
};

static mvpw_graph_attr_t splash_graph_attr = {
	.min = 0,
	.max = 18,
	.fg = mvpw_color_alpha(MVPW_RED, 0x80),
	.gradient = 1,
	.left = MVPW_BLACK,
	.right = MVPW_RED,
};

static mvpw_graph_attr_t demux_audio_graph_attr = {
	.min = 0,
	.max = 1024*1024*2,
	.fg = mvpw_color_alpha(MVPW_BLUE, 0x80),
	.bg = mvpw_color_alpha(MVPW_BLACK, 0x80),
};

static mvpw_graph_attr_t demux_video_graph_attr = {
	.min = 0,
	.max = 1024*1024*2,
	.fg = mvpw_color_alpha(MVPW_BLUE, 0x80),
	.bg = mvpw_color_alpha(MVPW_BLACK, 0x80),
};

/* 
 * replaytv attributes 
 */

// device menu attributes
mvpw_menu_attr_t  rtv_device_menu_attr = {
	.font = FONT_LARGE,
	.fg = MVPW_BLACK,
	.hilite_fg = MVPW_WHITE,
	.hilite_bg = MVPW_DARKGREY2,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_MIDNIGHTBLUE,
   .rounded = 0,
};

// device menu attributes
mvpw_menu_attr_t  rtv_show_browser_menu_attr = {
	.font = FONT_LARGE,
	.fg = MVPW_BLACK,
	.hilite_fg = MVPW_WHITE,
	.hilite_bg = MVPW_DARKGREY2,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_MIDNIGHTBLUE,
   .rounded = 0,
};

// show browser popup window menu attributes
mvpw_menu_attr_t rtv_show_popup_attr = {
	.font = FONT_LARGE,
	.fg = MVPW_BLACK,
	.hilite_fg = MVPW_BLACK,
	.hilite_bg = MVPW_WHITE,
	.title_fg = MVPW_WHITE,
	.title_bg = MVPW_MIDNIGHTBLUE,
   .rounded = 0,
};

mvpw_graph_attr_t rtv_discspace_graph_attr = {
	.min = 0,
	.max = 100,
   .fg = 0xff191988, //FIREBRICK3 (2/3 intensity)
   .bg = MVPW_LIGHTGREY,
   .border = 0,
   .border_size = 0,
};

// discovery splash window text attributes
mvpw_text_attr_t rtv_discovery_splash_attr = {
   .wrap = 1,
   .justify = MVPW_TEXT_LEFT,
   .margin = 6,
   .font = FONT_LARGE,
   .fg = MVPW_GREEN,
   .bg = MVPW_BLACK,
   .border = MVPW_GREEN,
   .border_size = 2,
};

// device description window text attributes
mvpw_text_attr_t rtv_device_descr_attr = {
   .wrap = 1,
   .justify = MVPW_TEXT_LEFT,
   .margin = 0,
   .font = FONT_LARGE,
   .fg = 0xff8b8b7a,    //LIGHTCYAN4,
   .bg = MVPW_BLACK,
};

// show episode description window text attributes
mvpw_text_attr_t rtv_episode_descr_attr = {
   .wrap = 1,
   .justify = MVPW_TEXT_LEFT,
   .margin = 0,
   .font = FONT_LARGE,
   .fg = 0xff8b8b7a,    //LIGHTCYAN4,
   .bg = MVPW_BLACK,
};

// OSD show title attributes
mvpw_text_attr_t rtv_osd_show_title_attr = {
	.wrap = 0,
	.justify = MVPW_TEXT_LEFT,
	.margin = 0,
	.font = FONT_LARGE,
	.fg = MVPW_CYAN,
	.bg = mvpw_color_alpha(MVPW_BLACK, 0x80),
};

// OSD show description attributes
mvpw_text_attr_t rtv_osd_show_desc_attr = {
	.wrap = 1,
	.justify = MVPW_TEXT_LEFT,
	.margin = 0,
	.font = FONT_LARGE,
	.fg = MVPW_WHITE,
	.bg = mvpw_color_alpha(MVPW_BLACK, 0x80),
};

// popup message window attributes
mvpw_text_attr_t rtv_message_window_attr = {
   .wrap = 1,
   .justify = MVPW_TEXT_LEFT,
   .margin = 12,
   .font = FONT_LARGE,
   .fg = MVPW_WHITE,
   .bg = MVPW_LIGHTGREY,
   .border =  MVPW_MIDNIGHTBLUE,
   .border_size = 4,
};

// seek_osd attributes (jump, seek, comercial skip)
mvpw_text_attr_t rtv_seek_osd_attr = {
	.wrap = 1,
	.justify = MVPW_TEXT_LEFT,
	.margin = 0,
	.font = FONT_LARGE,
	.fg = MVPW_WHITE,
	.bg = mvpw_color_alpha(MVPW_BLACK, 0x80),
   .border = MVPW_BLACK,
   .border_size = 0,
};


/*
 * Only the attribute structures in the following list will be changeable
 * via a theme XML file.
 */
theme_attr_t theme_attr[] = {
	{ .name = "about",
	  .type = WIDGET_DIALOG,
	  .attr.dialog = &about_attr },
	{ .name = "busy_graph",
	  .type = WIDGET_GRAPH,
	  .attr.graph = &busy_graph_attr },
	{ .name = "busy_text",
	  .type = WIDGET_TEXT,
	  .attr.text = &busy_text_attr },
	{ .name = "description",
	  .type = WIDGET_TEXT,
	  .attr.text = &description_attr },
	{ .name = "display",
	  .type = WIDGET_TEXT,
	  .attr.text = &display_attr },
	{ .name = "file_browser",
	  .type = WIDGET_MENU,
	  .attr.menu = &fb_attr },
	{ .name = "main_menu",
	  .type = WIDGET_MENU,
	  .attr.menu = &attr },
	{ .name = "myth_browser",
	  .type = WIDGET_MENU,
	  .attr.menu = &mythtv_attr },
	{ .name = "myth_delete",
	  .type = WIDGET_MENU,
	  .attr.menu = &mythtv_popup_attr },
	{ .name = "myth_description",
	  .type = WIDGET_TEXT,
	  .attr.text = &mythtv_description_attr },
	{ .name = "myth_info",
	  .type = WIDGET_TEXT,
	  .attr.text = &mythtv_info_attr },
	{ .name = "myth_menu",
	  .type = WIDGET_MENU,
	  .attr.menu = &myth_main_attr },
	{ .name = "myth_program",
	  .type = WIDGET_TEXT,
	  .attr.text = &mythtv_program_attr },
	{ .name = "offset_graph",
	  .type = WIDGET_GRAPH,
	  .attr.graph = &offset_graph_attr },
	{ .name = "popup",
	  .type = WIDGET_MENU,
	  .attr.menu = &popup_attr },
	{ .name = "settings",
	  .type = WIDGET_MENU,
	  .attr.menu = &settings_attr },
	{ .name = "splash",
	  .type = WIDGET_TEXT,
	  .attr.text = &splash_attr },
	{ .name = "splash_graph",
	  .type = WIDGET_GRAPH,
	  .attr.graph = &splash_graph_attr },
	{ .name = "themes",
	  .type = WIDGET_MENU,
	  .attr.menu = &themes_attr },
	{ .name = "warning",
	  .type = WIDGET_DIALOG,
	  .attr.dialog = &warn_attr },

	{ .name = "rtv_device_menu",
	  .type = WIDGET_MENU,
	  .attr.menu = &rtv_device_menu_attr },
	{ .name = "rtv_show_browser",
	  .type = WIDGET_MENU,
	  .attr.menu = &rtv_show_browser_menu_attr },
	{ .name = "rtv_show_popup",
	  .type = WIDGET_MENU,
	  .attr.menu = &rtv_show_popup_attr },
	{ .name = "rtv_discspace_graph",
	  .type = WIDGET_GRAPH,
	  .attr.graph = &rtv_discspace_graph_attr },
	{ .name = "rtv_discovery_splash",
	  .type = WIDGET_TEXT,
	  .attr.text = &rtv_discovery_splash_attr },
	{ .name = "rtv_device_description",
	  .type = WIDGET_TEXT,
	  .attr.text = &rtv_device_descr_attr },
	{ .name = "rtv_episode_description",
	  .type = WIDGET_TEXT,
	  .attr.text = &rtv_episode_descr_attr },
	{ .name = "rtv_osd_show_title",
	  .type = WIDGET_TEXT,
	  .attr.text = &rtv_osd_show_title_attr },
	{ .name = "rtv_osd_show_description",
	  .type = WIDGET_TEXT,
	  .attr.text = &rtv_osd_show_desc_attr },
	{ .name = "rtv_message_window",
	  .type = WIDGET_TEXT,
	  .attr.text = &rtv_message_window_attr },
	{ .name = "rtv_seek_osd",
	  .type = WIDGET_TEXT,
	  .attr.text = &rtv_seek_osd_attr },

	/* must be NULL-terminated */
	{ .name = NULL }
};

static int init_done = 0;

uint32_t root_color = 0;
int root_bright = 0;
mvp_widget_t *root;
mvp_widget_t *iw;

static mvp_widget_t *splash;
static mvp_widget_t *splash_title;
static mvp_widget_t *splash_graph;
static mvp_widget_t *main_menu;
static mvp_widget_t *mvpmc_logo;
static mvp_widget_t *settings;
static mvp_widget_t *sub_settings;
static mvp_widget_t *about;
static mvp_widget_t *setup_image;
static mvp_widget_t *fb_image;
static mvp_widget_t *mythtv_image;
static mvp_widget_t *replaytv_image;
static mvp_widget_t *about_image;
static mvp_widget_t *exit_image;
static mvp_widget_t *warn_widget;
static mvp_widget_t *busy_widget;
static mvp_widget_t *busy_graph;
static mvp_widget_t *themes_menu;

static mvp_widget_t *ct_text_box;
static mvp_widget_t *ct_fg_box;
static mvp_widget_t *ct_bg_box;

mvp_widget_t *file_browser;
mvp_widget_t *mythtv_browser;
mvp_widget_t *mythtv_menu;
mvp_widget_t *mythtv_logo;
mvp_widget_t *mythtv_date;
mvp_widget_t *mythtv_description;
mvp_widget_t *mythtv_channel;
mvp_widget_t *mythtv_record;
mvp_widget_t *mythtv_popup;
mvp_widget_t *mythtv_info;
mvp_widget_t *pause_widget;
mvp_widget_t *mute_widget;
mvp_widget_t *ffwd_widget;
mvp_widget_t *zoom_widget;
mvp_widget_t *osd_widget;
mvp_widget_t *offset_widget;
mvp_widget_t *offset_bar;
mvp_widget_t *bps_widget;
mvp_widget_t *time_widget;
mvp_widget_t *spu_widget;

mvp_widget_t *shows_widget;
mvp_widget_t *episodes_widget;
mvp_widget_t *freespace_widget;

mvp_widget_t *popup_menu;
mvp_widget_t *audio_stream_menu;
mvp_widget_t *video_stream_menu;
mvp_widget_t *subtitle_stream_menu;
mvp_widget_t *osd_menu;
mvp_widget_t *bright_menu;

mvp_widget_t *mythtv_program_widget;
mvp_widget_t *mythtv_osd_program;
mvp_widget_t *mythtv_osd_description;

mvp_widget_t *fb_program_widget;

mvp_widget_t *clock_widget;
mvp_widget_t *demux_video;
mvp_widget_t *demux_audio;

mvp_widget_t *screensaver;

mvp_widget_t *playlist_widget;

mvp_widget_t *fb_progress;
mvp_widget_t *fb_name;
mvp_widget_t *fb_time;
mvp_widget_t *fb_size;
mvp_widget_t *fb_offset_widget;
mvp_widget_t *fb_offset_bar;

static int screensaver_enabled = 0;
volatile int screensaver_timeout = 60;
volatile int screensaver_default = -1;

static struct {
   int  bg_idx;
   int  fg_idx;
} ct_globals;

static pthread_t busy_thread;
static pthread_cond_t busy_cond = PTHREAD_COND_INITIALIZER;

static volatile int busy = 0;

static void screensaver_event(mvp_widget_t *widget, int activate);

mvp_widget_t *focus_widget;

mvpw_screen_info_t si;

enum {
	MM_EXIT,
	MM_MYTHTV,
	MM_FILESYSTEM,
	MM_ABOUT,
	MM_SETTINGS,
	MM_REPLAYTV,
	MM_THEMES,
};

enum {
	SETTINGS_MODE,
	SETTINGS_OUTPUT,
	SETTINGS_FLICKER,
	SETTINGS_ASPECT,
	SETTINGS_SCREENSAVER,
	SETTINGS_COLORTEST,
};

enum {
	MENU_AUDIO_STREAM,
	MENU_VIDEO_STREAM,
	MENU_SUBTITLES,
	MENU_OSD,
	MENU_BRIGHT,
};

osd_widget_t osd_widgets[MAX_OSD_WIDGETS];

void
add_osd_widget(mvp_widget_t *widget, int type, int visible,
	       void (*callback)(mvp_widget_t*))
{
	int i = 0;

	while ((osd_widgets[i].widget != NULL) &&
	       (osd_widgets[i].type != type))
		i++;

	osd_widgets[i].type = type;
	osd_widgets[i].visible = visible;
	osd_widgets[i].widget = widget;
	osd_widgets[i].callback = callback;
}

int
osd_widget_toggle(int type)
{
	int i = 0, on;
	void (*callback)(mvp_widget_t*);

	while ((osd_widgets[i].type != type) && (i < MAX_OSD_WIDGETS))
		i++;

	if (i == MAX_OSD_WIDGETS)
		return -1;

	on = osd_widgets[i].visible = !osd_widgets[i].visible;
	callback = osd_widgets[i].callback;

	if (callback) {
		if (on) {
			callback(osd_widgets[i].widget);
			mvpw_set_timer(osd_widgets[i].widget, callback, 1000);
			mvpw_show(osd_widgets[i].widget);
			mvpw_expose(osd_widgets[i].widget);
		} else {
			mvpw_set_timer(osd_widgets[i].widget, NULL, 0);
			mvpw_hide(osd_widgets[i].widget);
		}
	}

	return on;
}

static void settings_select_callback(mvp_widget_t*, char*, void*);
static void sub_settings_select_callback(mvp_widget_t*, char*, void*);

static void
splash_update(char *str)
{
	mvpw_set_text_str(splash, str);
	mvpw_expose(splash);
	mvpw_graph_incr(splash_graph, 1);
	mvpw_event_flush();
}

static void
root_callback(mvp_widget_t *widget, char key)
{
	video_callback(widget, key);
}

static void
main_menu_callback(mvp_widget_t *widget, char key)
{
}

static void
mythtv_menu_callback(mvp_widget_t *widget, char key)
{
	if (key == MVPW_KEY_EXIT) {
		mvpw_hide(mythtv_browser);
		mvpw_hide(mythtv_menu);
		mvpw_hide(mythtv_logo);
		mvpw_hide(mythtv_channel);
		mvpw_hide(mythtv_date);
		mvpw_hide(mythtv_description);
		mvpw_hide(mythtv_record);
		mvpw_hide(shows_widget);
		mvpw_hide(episodes_widget);
		mvpw_hide(freespace_widget);

		mythtv_cleanup();

		mvpw_show(mythtv_image);
		mvpw_show(main_menu);
		mvpw_show(mvpmc_logo);
		mvpw_focus(main_menu);

		mythtv_main_menu = 0;
		mythtv_state = MYTHTV_STATE_MAIN;
	}

	if ((key == MVPW_KEY_FULL) || (key == MVPW_KEY_PREV_CHAN)) {
		if (video_playing) {
			mvpw_hide(mythtv_logo);
			mvpw_hide(mythtv_menu);
			mvpw_focus(root);

			av_move(0, 0, 0);

			if (mythtv_livetv == 2)
				mythtv_livetv = 1;
		}
	}

	if (key == MVPW_KEY_STOP) {
		if (mythtv_livetv) {
			mythtv_livetv_stop();
			mythtv_livetv = 0;
		} else {
			mythtv_stop();
		}
	}

	switch (key) {
	case MVPW_KEY_REPLAY:
	case MVPW_KEY_SKIP:
	case MVPW_KEY_REWIND:
	case MVPW_KEY_FFWD:
	case MVPW_KEY_LEFT:
	case MVPW_KEY_RIGHT:
	case MVPW_KEY_VOL_UP:
	case MVPW_KEY_VOL_DOWN:
	case MVPW_KEY_PAUSE:
	case MVPW_KEY_MUTE:
	case MVPW_KEY_ZERO ... MVPW_KEY_NINE:
		video_callback(widget, key);
		break;
	}
}

void
replaytv_back_to_mvp_main_menu(void) {
	replaytv_hide_device_menu();
	mvpw_show(replaytv_image);
	mvpw_show(main_menu);
	mvpw_show(mvpmc_logo);
	mvpw_focus(main_menu);
}

static void
iw_key_callback(mvp_widget_t *widget, char key)
{
	mvpw_hide(widget);
	mvpw_show(file_browser);
	mvpw_focus(file_browser);
}

static void
warn_key_callback(mvp_widget_t *widget, char key)
{
	mvpw_hide(widget);
}

static void
mythtv_info_key_callback(mvp_widget_t *widget, char key)
{
	mvpw_hide(widget);
}

static void
sub_settings_key_callback(mvp_widget_t *widget, char key)
{
	if (key == MVPW_KEY_EXIT) {
		mvpw_hide(settings);
		mvpw_hide(sub_settings);

		mvpw_show(main_menu);
		mvpw_show(mvpmc_logo);
		mvpw_show(setup_image);

		mvpw_focus(main_menu);
	}

	if ((key == MVPW_KEY_LEFT) || (key == MVPW_KEY_VOL_DOWN)) {
		/*
		 * XXX: fix this
		 */
		settings_attr.hilite_bg = MVPW_BLUE;
		mvpw_set_menu_attr(settings, &settings_attr);

		settings_attr.hilite_bg = MVPW_DARKGREY;
		mvpw_set_menu_attr(sub_settings, &settings_attr);

		mvpw_expose(settings);
		mvpw_expose(sub_settings);
		mvpw_focus(settings);
	}
}

static void
settings_key_callback(mvp_widget_t *widget, char key)
{
	if (key == MVPW_KEY_EXIT) {
		mvpw_hide(settings);
		mvpw_hide(sub_settings);

		mvpw_show(main_menu);
		mvpw_show(mvpmc_logo);
		mvpw_show(setup_image);

		mvpw_focus(main_menu);
	}

	if ((key == MVPW_KEY_RIGHT) || (key == MVPW_KEY_VOL_UP)) {
		settings_select_callback(NULL, NULL, NULL);
	}
}

static void
themes_key_callback(mvp_widget_t *widget, char key)
{
	switch (key) {
	case MVPW_KEY_EXIT:
		mvpw_hide(widget);

		mvpw_show(main_menu);
		mvpw_show(mvpmc_logo);
		mvpw_show(setup_image);

		mvpw_focus(main_menu);
		break;
	}
}

static void
themes_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	char buf[256];

	memset(buf, 0, sizeof(buf));
	readlink(DEFAULT_THEME, buf, sizeof(buf));

	if (strcmp(buf, item) != 0) {
		printf("switch to theme '%s'\n", item);
		unlink(DEFAULT_THEME);
		if (symlink(item, DEFAULT_THEME) != 0) {
			symlink(buf, DEFAULT_THEME);
			fprintf(stderr, "switch failed!\n");
			return;
		}
		exit(1);
	}
}

static void
fb_key_callback(mvp_widget_t *widget, char key)
{
	switch (key) {
	case MVPW_KEY_EXIT:
		mvpw_hide(widget);
		mvpw_hide(iw);
		mvpw_hide(fb_progress);

		mvpw_show(main_menu);
		mvpw_show(mvpmc_logo);
		mvpw_show(fb_image);
		mvpw_focus(main_menu);

		mvpw_set_idle(NULL);
		mvpw_set_timer(root, NULL, 0);
		mvpw_set_timer(fb_progress, NULL, 0);

		audio_clear();
		video_clear();
		break;
	case MVPW_KEY_STOP:
		if (current) {
			free(current);
			current = NULL;
		}
		mvpw_set_idle(NULL);
		mvpw_set_timer(root, NULL, 0);
		mvpw_hide(fb_progress);

		audio_clear();
		video_clear();
		break;
	case MVPW_KEY_FULL:
	case MVPW_KEY_PREV_CHAN:
		if (video_playing) {
			mvpw_hide(fb_progress);
			mvpw_hide(widget);
			mvpw_focus(root);

			av_move(0, 0, 0);

			if (mythtv_livetv == 2)
				mythtv_livetv = 1;
		}
		break;
	case MVPW_KEY_PLAY:
		fb_start_thumbnail();
		break;
	case MVPW_KEY_PAUSE:
		if (av_pause()) {
			mvpw_show(pause_widget);
			paused = 1;
		} else {
			mvpw_hide(pause_widget);
			mvpw_hide(mute_widget);
			paused = 0;
		}
		break;
	}
}

static void
playlist_key_callback(mvp_widget_t *widget, char key)
{
	switch (key) {
	case MVPW_KEY_EXIT:
		mvpw_hide(widget);
		mvpw_show(file_browser);
		mvpw_focus(file_browser);
		break;
	case MVPW_KEY_SKIP:
		audio_clear();
		av_reset();
		playlist_next();
		break;
	case MVPW_KEY_REPLAY:
		audio_clear();
		av_reset();
		playlist_prev();
		break;
	case MVPW_KEY_STOP:
		audio_clear();
		av_reset();
		playlist_stop();
		break;
	case MVPW_KEY_PAUSE:
		av_pause();
		break;
	}
}

static void
mythtv_popup_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	int which = (int)key;
	char buf[1024];

	switch (which) {
	case 0:
		printf("trying to forget recording\n");
		if ((mythtv_delete() == 0) && (mythtv_forget() == 0)) {
			mvpw_hide(mythtv_popup);
			mythtv_state = MYTHTV_STATE_EPISODES;
			mythtv_update(mythtv_browser);
		}
		break;
	case 1:
		printf("trying to delete recording\n");
		if (mythtv_delete() == 0) {
			mvpw_hide(mythtv_popup);
			mythtv_state = MYTHTV_STATE_EPISODES;
			mythtv_update(mythtv_browser);
		}
		break;
	case 2:
		printf("show info...\n");
		mythtv_proginfo(buf, sizeof(buf));
		mvpw_set_text_str(mythtv_info, buf);
		mvpw_show(mythtv_info);
		mvpw_focus(mythtv_info);
		break;
	case 3:
		mvpw_hide(mythtv_popup);
		break;
	}
}

static void
mythtv_key_callback(mvp_widget_t *widget, char key)
{
	if (key == MVPW_KEY_EXIT) {
		if (mythtv_state == MYTHTV_STATE_LIVETV) {
			printf("return from livetv to myth main menu!\n");
			mvpw_hide(mythtv_browser);
			mvpw_hide(mythtv_channel);
			mvpw_hide(mythtv_date);
			mvpw_hide(mythtv_description);
			mvpw_show(mythtv_logo);
			mvpw_show(mythtv_menu);
			mvpw_focus(mythtv_menu);

			mythtv_main_menu = 1;
			mythtv_state = MYTHTV_STATE_MAIN;
		} else {
			if (mythtv_back(widget) == 0) {
				mvpw_hide(mythtv_browser);
				mvpw_hide(mythtv_channel);
				mvpw_hide(mythtv_date);
				mvpw_hide(mythtv_description);
				mvpw_hide(mythtv_record);
				mvpw_hide(shows_widget);
				mvpw_hide(episodes_widget);
				mvpw_hide(freespace_widget);

				mvpw_show(mythtv_logo);
				mvpw_show(mythtv_menu);
				mvpw_focus(mythtv_menu);

				mythtv_main_menu = 1;
				mythtv_state = MYTHTV_STATE_MAIN;
			}
		}
	}

	if ((key == MVPW_KEY_MENU) &&
	    (mythtv_state == MYTHTV_STATE_EPISODES)) {
		printf("mythtv popup menu\n");
		mvpw_clear_menu(mythtv_popup);
		mythtv_popup_item_attr.select = mythtv_popup_select_callback;
		mythtv_popup_item_attr.fg = mythtv_popup_attr.fg;
		mythtv_popup_item_attr.bg = mythtv_popup_attr.bg;
		mvpw_add_menu_item(mythtv_popup,
				   "Delete, but allow future recordings",
				   (void*)0, &mythtv_popup_item_attr);
		mvpw_add_menu_item(mythtv_popup, "Delete",
				   (void*)1, &mythtv_popup_item_attr);
		mvpw_add_menu_item(mythtv_popup, "Show Info",
				   (void*)2, &mythtv_popup_item_attr);
		mvpw_add_menu_item(mythtv_popup, "Cancel",
				   (void*)3, &mythtv_popup_item_attr);
		mvpw_menu_hilite_item(mythtv_popup, (void*)3);
		mvpw_show(mythtv_popup);
		mvpw_focus(mythtv_popup);
	}

	if ((key == MVPW_KEY_FULL) || (key == MVPW_KEY_PREV_CHAN)) {
		if (video_playing) {
			mvpw_hide(mythtv_logo);
			mvpw_hide(mythtv_channel);
			mvpw_hide(mythtv_date);
			mvpw_hide(mythtv_description);
			mvpw_hide(mythtv_record);
			mvpw_hide(mythtv_browser);
			mvpw_hide(shows_widget);
			mvpw_hide(episodes_widget);
			mvpw_hide(freespace_widget);
			mvpw_focus(root);

			av_move(0, 0, 0);

			if (mythtv_livetv == 2)
				mythtv_livetv = 1;
		}
	}

	if (key == MVPW_KEY_PLAY) {
		mythtv_start_thumbnail();
	}

	if (key == MVPW_KEY_STOP) {
		if (mythtv_livetv) {
			mythtv_livetv_stop();
			mythtv_livetv = 0;
		} else {
			mythtv_stop();
		}
	}

	switch (key) {
	case MVPW_KEY_REPLAY:
	case MVPW_KEY_SKIP:
	case MVPW_KEY_REWIND:
	case MVPW_KEY_FFWD:
	case MVPW_KEY_LEFT:
	case MVPW_KEY_RIGHT:
	case MVPW_KEY_PAUSE:
	case MVPW_KEY_MUTE:
	case MVPW_KEY_ZERO ... MVPW_KEY_NINE:
		video_callback(widget, key);
		break;
	}
}

static void
mythtv_popup_key_callback(mvp_widget_t *widget, char key)
{
	if (key == MVPW_KEY_MENU) {
		mvpw_hide(mythtv_popup);
	}

	if (key == MVPW_KEY_EXIT) {
		mvpw_hide(mythtv_popup);
	}
}

static void
popup_key_callback(mvp_widget_t *widget, char key)
{
	if (key == MVPW_KEY_MENU) {
		mvpw_hide(popup_menu);
		mvpw_hide(audio_stream_menu);
		mvpw_hide(video_stream_menu);
		mvpw_hide(subtitle_stream_menu);
		mvpw_hide(osd_menu);
		mvpw_hide(bright_menu);
		mvpw_focus(root);
	}

	if (key == MVPW_KEY_EXIT) {
		if (mvpw_visible(popup_menu)) {
			mvpw_hide(popup_menu);
			mvpw_focus(root);
		} else {
			mvpw_hide(audio_stream_menu);
			mvpw_hide(video_stream_menu);
			mvpw_hide(subtitle_stream_menu);
			mvpw_hide(osd_menu);
			mvpw_hide(bright_menu);
			mvpw_show(popup_menu);
			mvpw_focus(popup_menu);
		}
	}
}

static int
file_browser_init(void)
{
	mvp_widget_t *contain, *widget;
	int h, w, w2;

	splash_update("Creating file browser");

	file_browser = mvpw_create_menu(NULL, 50, 30, si.cols-120, si.rows-190,
					fb_attr.bg, fb_attr.border,
					fb_attr.border_size);

	mvpw_set_menu_attr(file_browser, &fb_attr);

	mvpw_set_menu_title(file_browser, "/");
	mvpw_set_bg(file_browser, MVPW_LIGHTGREY);

	mvpw_set_key(file_browser, fb_key_callback);

	h = mvpw_font_height(display_attr.font) + (2 * display_attr.margin);
	w = 300;

	contain = mvpw_create_container(NULL, 50, 80,
					w, h*3, display_attr.bg,
					display_attr.border,
					display_attr.border_size);
	fb_progress = contain;

	widget = mvpw_create_text(contain, 0, 0, w, h, display_attr.bg,
				  display_attr.border,
				  display_attr.border_size);
	mvpw_set_text_attr(widget, &display_attr);
	mvpw_set_text_str(widget, "");
	mvpw_show(widget);
	fb_name = widget;

	widget = mvpw_create_text(contain, 0, 0, w/2, h,
				  display_attr.bg,
				  display_attr.border,
				  display_attr.border_size);
	mvpw_set_text_attr(widget, &display_attr);
	mvpw_set_text_str(widget, "");
	mvpw_show(widget);
	fb_time = widget;

	widget = mvpw_create_text(contain, 0, 0, w/2, h,
				  display_attr.bg,
				  display_attr.border,
				  display_attr.border_size);
	display_attr.justify = MVPW_TEXT_RIGHT;
	mvpw_set_text_attr(widget, &display_attr);
	display_attr.justify = MVPW_TEXT_LEFT;
	mvpw_set_text_str(widget, "");
	mvpw_show(widget);
	fb_size = widget;

	w2 = mvpw_font_width(fontid, "1000%");
	widget = mvpw_create_text(contain, 0, 0, w2, h,
				  display_attr.bg,
				  display_attr.border,
				  display_attr.border_size);
	mvpw_set_text_attr(widget, &display_attr);
	mvpw_set_text_str(widget, "0%");
	mvpw_show(widget);
	fb_offset_widget = widget;

	widget = mvpw_create_graph(contain, w, 0, w-w2, h/2,
				   0x80000000, 0, 0);
	mvpw_set_graph_attr(widget, &offset_graph_attr);
	mvpw_show(widget);
	fb_offset_bar = widget;

	mvpw_attach(fb_name, fb_time, MVPW_DIR_DOWN);
	mvpw_attach(fb_time, fb_size, MVPW_DIR_RIGHT);
	mvpw_attach(fb_time, fb_offset_widget, MVPW_DIR_DOWN);
	mvpw_attach(fb_offset_widget, fb_offset_bar, MVPW_DIR_RIGHT);

	mvpw_raise(file_browser);
	mvpw_attach(file_browser, fb_progress, MVPW_DIR_DOWN);

	return 0;
}

static int
playlist_init(void)
{
	splash_update("Creating playlist");

	playlist_widget = mvpw_create_menu(NULL, 50, 30,
					   si.cols-120, si.rows-190,
					   fb_attr.bg, fb_attr.border,
					   fb_attr.border_size);

	mvpw_set_menu_attr(playlist_widget, &fb_attr);

	mvpw_set_menu_title(playlist_widget, "Playlist");
	mvpw_set_bg(playlist_widget, MVPW_LIGHTGREY);

	mvpw_set_key(playlist_widget, playlist_key_callback);

	return 0;
}

static void
audio_stream_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	audio_switch_stream(widget, (int)key);
}

static void
video_stream_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	video_switch_stream(widget, (int)key);
}

static void
subtitle_stream_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	subtitle_switch_stream(widget, (int)key);
}

static void
osd_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	osd_type_t type = (osd_type_t)key;
	int on;

	printf("OSD callback on '%s' %d\n", item, (int)key);

	on = osd_widget_toggle(type);

	mvpw_check_menu_item(osd_menu, (void*)key, on);
	mvpw_expose(osd_menu);
}

static void
bright_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	int level = (int)key;

	if (root_bright == level)
		return;

	mvpw_check_menu_item(bright_menu, (void*)root_bright, 0);
	root_bright = level;
	mvpw_check_menu_item(bright_menu, (void*)root_bright, 1);

	if (level > 0) {
		root_color = mvpw_color_alpha(MVPW_WHITE, level*16);
	} else if (level < 0) {
		root_color = mvpw_color_alpha(MVPW_BLACK, level*-16);
	} else {
		root_color = 0;
	}

	mvpw_set_bg(root, root_color);
}

static void
popup_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	mvpw_hide(popup_menu);

	switch ((int)key) {
	case MENU_AUDIO_STREAM:
		popup_item_attr.select = audio_stream_select_callback;
		add_audio_streams(audio_stream_menu, &popup_item_attr);
		mvpw_show(audio_stream_menu);
		mvpw_focus(audio_stream_menu);
		break;
	case MENU_VIDEO_STREAM:
		popup_item_attr.select = video_stream_select_callback;
		add_video_streams(video_stream_menu, &popup_item_attr);
		mvpw_show(video_stream_menu);
		mvpw_focus(video_stream_menu);
		break;
	case MENU_SUBTITLES:
		popup_item_attr.select = subtitle_stream_select_callback;
		add_subtitle_streams(subtitle_stream_menu, &popup_item_attr);
		mvpw_show(subtitle_stream_menu);
		mvpw_focus(subtitle_stream_menu);
		break;
	case MENU_OSD:
		mvpw_show(osd_menu);
		mvpw_focus(osd_menu);
		break;
	case MENU_BRIGHT:
		mvpw_show(bright_menu);
		mvpw_focus(bright_menu);
		break;
	}
}

static void
colortest_draw(int fg_idx, int bg_idx)
{
	char buf[255];
 
	snprintf(buf, 254, "FG: %3d: %06X: %s", 
				fg_idx, 
				color_list[fg_idx].val & 0x00ffffff, 
				color_list[fg_idx].name);
	mvpw_set_text_str(ct_fg_box, buf);
	
	snprintf(buf, 254, "BG: %3d: %06X: %s", 
				bg_idx, 
				color_list[bg_idx].val & 0x00ffffff, 
				color_list[bg_idx].name);
	mvpw_set_text_str(ct_bg_box, buf);
	
	ct_text_box_attr.fg = color_list[fg_idx].val;
	mvpw_set_text_attr(ct_text_box, &ct_text_box_attr);
	mvpw_set_bg(ct_text_box, color_list[bg_idx].val);

	mvpw_hide(ct_fg_box);
	mvpw_show(ct_fg_box);
	mvpw_hide(ct_bg_box);
	mvpw_show(ct_bg_box);
	mvpw_hide(ct_text_box);
	mvpw_show(ct_text_box);

}

#define INCR_COLOR(cpos, cincr) ( (((cpos)+(cincr)) < 0) ? (color_list_size()-1) : (((cpos)+(cincr)) % (color_list_size()))  )

static void
colortest_callback(mvp_widget_t *widget, char key)
{
	static int	cur_dir = 1;
	static int *cur_idx = &ct_globals.fg_idx;

	int  incr_val = 1;
	int  jmp;

	if ( key == MVPW_KEY_EXIT ) {
		mvpw_hide(ct_fg_box);
		mvpw_hide(ct_bg_box);
		mvpw_hide(ct_text_box);
		mvpw_set_key(root, root_callback);
		mvpw_set_bg(root, MVPW_BLACK);
		mvpw_show(mvpmc_logo);
		mvpw_show(settings);
		mvpw_show(sub_settings);
		mvpw_focus(settings);
		return;
	}

	switch (key) {
	case MVPW_KEY_LEFT:
		incr_val = 1;
		cur_idx = &ct_globals.bg_idx;
		cur_dir	= -1;
		break;
	case MVPW_KEY_RIGHT:
		incr_val = 1;
		cur_idx	=	&ct_globals.bg_idx;
		cur_dir	= 1;
		break;
	case MVPW_KEY_DOWN:
		incr_val = 1;
		cur_idx	=	&ct_globals.fg_idx;
		cur_dir	= -1;
		break;
	case MVPW_KEY_UP:
		incr_val = 1;
		cur_idx	=	&ct_globals.fg_idx;
		cur_dir	= 1;
		break;
	case MVPW_KEY_ZERO:
		incr_val = 10;		  
		break;
	case MVPW_KEY_ONE ... MVPW_KEY_NINE:
		incr_val = key - MVPW_KEY_ZERO;		 
		break;

	default:
		break;
	} //switch

	jmp		= incr_val * cur_dir;
	*cur_idx = INCR_COLOR(*cur_idx, jmp);

	colortest_draw(ct_globals.fg_idx, ct_globals.bg_idx);
	return;
}

static void 
run_colortest(void)
{
	mvpw_hide(settings);
	mvpw_hide(sub_settings);
	mvpw_hide(mvpmc_logo);
	mvpw_set_key(root, colortest_callback);
	colortest_draw(ct_globals.fg_idx, ct_globals.bg_idx);
}

static void
sub_settings_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	if ((strcmp(item, "PAL") == 0) || (strcmp(item, "NTSC") == 0)) {
		if (av_set_mode((int)key) < 0)
			printf("set mode to %s failed\n", item);
		else
			printf("set mode to %s\n", item);
	}

	if ((strcmp(item, "Composite") == 0) || (strcmp(item, "S-Video") == 0)) {
		if (av_set_output((int)key) < 0)
			printf("set output to %s failed\n", item);
		else
			printf("set output to %s\n", item);
	}

	if ((strcmp(item, "4:3") == 0) || (strcmp(item, "16:9") == 0)) {
		if (av_set_aspect((int)key) < 0)
			printf("set aspect to %s failed\n", item);
		else
			printf("set aspect to %s\n", item);
	}

	if ((strcmp(item, "off") == 0) ||
	    (strstr(item, "minute") != 0) ||
	    (strstr(item, "second") != 0)) {
		screensaver_timeout = (int)key;
		printf("screensaver timeout changed to %d\n",
		       screensaver_timeout);
		mvpw_set_screensaver(screensaver, screensaver_timeout,
				     screensaver_event);
	}
}

static void
sub_settings_hilite_callback(mvp_widget_t *widget, char *item, void *key,
			     int hilite)
{
}

static void
settings_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	if ( (int)key == SETTINGS_COLORTEST ) {
		mvpw_hide(settings);
		mvpw_hide(sub_settings);
		run_colortest();
		return;
	}

	/*
	 * XXX: fix this
	 */
	settings_attr.hilite_bg = MVPW_BLUE;
	mvpw_set_menu_attr(sub_settings, &settings_attr);

	settings_attr.hilite_bg = MVPW_DARKGREY;
	mvpw_set_menu_attr(settings, &settings_attr);

	mvpw_expose(settings);
	mvpw_expose(sub_settings);

	mvpw_focus(sub_settings);
}

static void
settings_hilite_callback(mvp_widget_t *widget, char *item, void *key,
			 int hilite)
{
	char buf[256];

	if (hilite) {
		mvpw_clear_menu(sub_settings);
		sub_settings_item_attr.hilite = sub_settings_hilite_callback;
		sub_settings_item_attr.select = sub_settings_select_callback;
		switch ((int)key) {
		case SETTINGS_MODE:
			mvpw_add_menu_item(sub_settings, "PAL",
					   (void*)AV_MODE_PAL,
					   &sub_settings_item_attr);
			mvpw_add_menu_item(sub_settings, "NTSC",
					   (void*)AV_MODE_NTSC,
					   &sub_settings_item_attr);
			mvpw_menu_hilite_item(sub_settings,
					      (void*)av_get_mode());
			break;
		case SETTINGS_OUTPUT:
			mvpw_add_menu_item(sub_settings, "Composite",
					   (void*)AV_OUTPUT_COMPOSITE,
					   &sub_settings_item_attr);
			mvpw_add_menu_item(sub_settings, "S-Video",
					   (void*)AV_OUTPUT_SVIDEO,
					   &sub_settings_item_attr);
			mvpw_menu_hilite_item(sub_settings,
					      (void*)av_get_output());
			break;
		case SETTINGS_FLICKER:
			break;
		case SETTINGS_ASPECT:
			mvpw_add_menu_item(sub_settings, "4:3",
					   (void*)AV_ASPECT_4x3,
					   &sub_settings_item_attr);
			mvpw_add_menu_item(sub_settings, "16:9",
					   (void*)AV_ASPECT_16x9,
					   &sub_settings_item_attr);
			mvpw_menu_hilite_item(sub_settings,
					      (void*)av_get_aspect());
			break;
		case SETTINGS_SCREENSAVER:
			snprintf(buf, sizeof(buf), "%d seconds",
				 screensaver_default);
			mvpw_add_menu_item(sub_settings, "off",
					   (void*)0,
					   &sub_settings_item_attr);
			if ((screensaver_default > 0) &&
			    (screensaver_default < 60)) {
				mvpw_add_menu_item(sub_settings, buf,
						   (void*)screensaver_default,
						   &sub_settings_item_attr);
			}
			mvpw_add_menu_item(sub_settings, "1 minute",
					   (void*)60,
					   &sub_settings_item_attr);
			if ((screensaver_default > 60) &&
			    (screensaver_default < 300)) {
				mvpw_add_menu_item(sub_settings, buf,
						   (void*)screensaver_default,
						   &sub_settings_item_attr);
			}
			mvpw_add_menu_item(sub_settings, "5 minutes",
					   (void*)300,
					   &sub_settings_item_attr);
			if ((screensaver_default > 300)) {
				mvpw_add_menu_item(sub_settings, buf,
						   (void*)screensaver_default,
						   &sub_settings_item_attr);
			}
			mvpw_menu_hilite_item(sub_settings,
					      (void*)screensaver_timeout);
			break;
		}
	} else {
	}
}

static int
settings_init(void)
{
	int i, x, y, w, h, num_cols, num_rows, bufpos;
	char buf[255];

	splash_update("Creating settings");

	h = mvpw_font_height(description_attr.font) +
		(2 * description_attr.margin);
	w = (si.cols - 100) / 2;

	x = 50;
	y = (si.rows / 2) - (h * 2);

	/*
	 * XXX: fix this
	 */

	settings = mvpw_create_menu(NULL, x, y, w, h*5,
				    settings_attr.bg, settings_attr.border,
				    settings_attr.border_size);

	settings_attr.hilite_bg = MVPW_BLUE;
	mvpw_set_key(settings, settings_key_callback);
	mvpw_set_menu_attr(settings, &settings_attr);

	mvpw_set_bg(settings, MVPW_LIGHTGREY);

	settings_attr.hilite_bg = MVPW_DARKGREY;
	x += w;
	sub_settings = mvpw_create_menu(NULL, x, y, w, h*5,
					settings_attr.bg, settings_attr.border,
					settings_attr.border_size);
	mvpw_set_key(sub_settings, sub_settings_key_callback);
	mvpw_set_menu_attr(sub_settings, &settings_attr);
	mvpw_set_bg(sub_settings, MVPW_LIGHTGREY);

	mvpw_attach(settings, sub_settings, MVPW_DIR_RIGHT);

	settings_item_attr.hilite = settings_hilite_callback;
	settings_item_attr.select = settings_select_callback;
	mvpw_add_menu_item(settings, "TV Mode",
			   (void*)SETTINGS_MODE, &settings_item_attr);
	mvpw_add_menu_item(settings, "Output",
			   (void*)SETTINGS_OUTPUT, &settings_item_attr);
#if 0
	mvpw_add_menu_item(settings, "Flicker Control",
			   (void*)SETTINGS_FLICKER, &settings_item_attr);
#endif
	mvpw_add_menu_item(settings, "Aspect Ratio",
			   (void*)SETTINGS_ASPECT, &settings_item_attr);
	mvpw_add_menu_item(settings, "Screensaver Timeout",
			   (void*)SETTINGS_SCREENSAVER, &settings_item_attr);
	mvpw_add_menu_item(settings, "ColorTest",
			   (void*)SETTINGS_COLORTEST, &settings_item_attr);

	sub_settings_item_attr.hilite = sub_settings_hilite_callback;
	sub_settings_item_attr.select = sub_settings_select_callback;

	/*
	 * Init colottest
	 */
	ct_globals.fg_idx	 = find_color_idx("SNOW");
	ct_globals.bg_idx	 = find_color_idx("BLACK");

	ct_fg_box = mvpw_create_text(NULL, 100, 300, 500, 50, MVPW_BLACK, 0, 0);	
	mvpw_set_text_attr(ct_fg_box, &ct_fgbg_box_attr);

	ct_bg_box = mvpw_create_text(NULL, 100, 350, 500, 50, MVPW_BLACK, 0, 0);	
	mvpw_set_text_attr(ct_bg_box, &ct_fgbg_box_attr);

	num_cols = 16;
	num_rows = (96/16);
	h = (mvpw_font_height(ct_text_box_attr.font) + 10) * ((95 / num_cols) + 1);
	w = si.cols;

	buf[0] = '\n';
	bufpos = 1;
	for (i = 0; i < 95; i++) {
		if ( i && !(i % num_cols) ) {
			buf[bufpos++] = '\n';
		}	
		buf[bufpos++] = i+32;
	}
	buf[bufpos] = '\0';
	ct_text_box = mvpw_create_text(NULL, 0, 0, w, h, MVPW_BLACK, 0, 0);	
	mvpw_set_text_str(ct_text_box, buf);

	return 0;
}

static int
themes_init(void)
{
	int x, y, w, h;
	int i;
	char buf[256];

	splash_update("Creating themes");

	h = 6 * (mvpw_font_height(themes_attr.font) + 8);
	w = (si.cols - 250);

	x = (si.cols - w) / 2;
	y = (si.rows - h) / 2;

	themes_menu = mvpw_create_menu(NULL, x, y, w, h,
				       themes_attr.bg, themes_attr.border,
				       themes_attr.border_size);

	if (themes_attr.checkbox_fg)
		themes_item_attr.checkbox_fg = themes_attr.checkbox_fg;

	mvpw_set_menu_attr(themes_menu, &themes_attr);
	mvpw_set_menu_title(themes_menu, "Themes");
	mvpw_set_key(themes_menu, themes_key_callback);

	themes_item_attr.select = themes_select_callback;
	themes_item_attr.fg = themes_attr.fg;
	themes_item_attr.bg = themes_attr.bg;

	for (i=0; i<THEME_MAX; i++) {
		if (theme_list[i].path == NULL)
			break;
		memset(buf, 0, sizeof(buf));
		readlink(DEFAULT_THEME, buf, sizeof(buf));
		mvpw_add_menu_item(themes_menu, theme_list[i].path,
				   (void*)i, &themes_item_attr);
		if (strcmp(buf, theme_list[i].path) == 0) {
			mvpw_check_menu_item(themes_menu, (void*)i, 1);
		} else {
			mvpw_check_menu_item(themes_menu, (void*)i, 0);
		}
	}

	return 0;
}

static void
myth_menu_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	int which = (int)key;

	switch (which) {
	case 0:
		busy_start();
		mythtv_state = MYTHTV_STATE_PROGRAMS;
		if (mythtv_update(mythtv_browser) == 0) {
			mvpw_show(mythtv_browser);

			mvpw_hide(mythtv_menu);
			mvpw_focus(mythtv_browser);

			mythtv_main_menu = 0;
		} else {
			mythtv_state = MYTHTV_STATE_MAIN;
		}
		busy_end();
		break;
	case 1:
		busy_start();
		mythtv_state = MYTHTV_STATE_PENDING;
		if (mythtv_pending(mythtv_browser) == 0) {
			mvpw_show(mythtv_browser);

			mvpw_hide(mythtv_menu);
			mvpw_focus(mythtv_browser);

			mythtv_main_menu = 0;
		} else {
			mythtv_state = MYTHTV_STATE_MAIN;
		}
		busy_end();
		break;
	case 2:
		busy_start();
		mythtv_state = MYTHTV_STATE_LIVETV;
		if (mythtv_livetv_menu() == 0) {
			running_mythtv = 1;
			mvpw_hide(mythtv_menu);

			mythtv_main_menu = 0;
		} else {
			mythtv_state = MYTHTV_STATE_MAIN;
		}
		busy_end();
		break;
	}
}

static int
myth_browser_init(void)
{
	mvpw_image_info_t iid;
	mvpw_widget_info_t wid, wid2, info;
	char file[128];
	int x, y, w, h;

	splash_update("Creating MythTV browser");

	snprintf(file, sizeof(file), "%s/mythtv_logo_rotate.png", imagedir);
	if (mvpw_get_image_info(file, &iid) < 0)
		return -1;
	mythtv_logo = mvpw_create_image(NULL, 50, 25, iid.width, iid.height,
				       0, 0, 0);
	mvpw_set_image(mythtv_logo, file);

	mythtv_menu = mvpw_create_menu(NULL, 50+iid.width, 25,
				       si.cols-130-iid.width, si.rows-190,
				       myth_main_attr.bg,
				       myth_main_attr.border,
				       myth_main_attr.border_size);
	mvpw_set_menu_attr(mythtv_menu, &myth_main_attr);

	myth_menu_item_attr.select = myth_menu_select_callback;
	myth_menu_item_attr.fg = myth_main_attr.fg;
	myth_menu_item_attr.bg = myth_main_attr.bg;

	mvpw_add_menu_item(mythtv_menu, "Watch Recordings",
			   (void*)0, &myth_menu_item_attr);
	mvpw_add_menu_item(mythtv_menu, "Upcoming Recordings",
			   (void*)1, &myth_menu_item_attr);
	mvpw_add_menu_item(mythtv_menu, "Live TV",
			   (void*)2, &myth_menu_item_attr);

	mvpw_set_key(mythtv_menu, mythtv_menu_callback);

	mythtv_browser = mvpw_create_menu(NULL, 50, 30,
					  si.cols-130-iid.width, si.rows-190,
					  mythtv_attr.bg, mythtv_attr.border,
					  mythtv_attr.border_size);

	mvpw_attach(mythtv_logo, mythtv_browser, MVPW_DIR_RIGHT);

	mvpw_set_key(mythtv_browser, mythtv_key_callback);

	mvpw_set_menu_attr(mythtv_browser, &mythtv_attr);

	h = mvpw_font_height(description_attr.font) +
		(2 * description_attr.margin);

	mythtv_channel = mvpw_create_text(NULL, 0, 0, 350, h,
					  description_attr.bg,
					  description_attr.border,
					  description_attr.border_size);
	mythtv_date = mvpw_create_text(NULL, 0, 0, 350, h,
				       description_attr.bg,
				       description_attr.border,
				       description_attr.border_size);
	mythtv_description = mvpw_create_text(NULL, 0, 0, 350, h*3,
					      description_attr.bg,
					      description_attr.border,
					      description_attr.border_size);
	mythtv_record = mvpw_create_text(NULL, 0, 0, 350, h,
					 description_attr.bg,
					 description_attr.border,
					 description_attr.border_size);

	mvpw_set_text_attr(mythtv_channel, &description_attr);
	mvpw_set_text_attr(mythtv_date, &description_attr);
	mvpw_set_text_attr(mythtv_description, &description_attr);
	mvpw_set_text_attr(mythtv_record, &description_attr);

	mvpw_get_widget_info(mythtv_logo, &wid);
	mvpw_get_widget_info(mythtv_browser, &wid2);
	mvpw_moveto(mythtv_channel, wid.x, wid2.y+wid2.h);
	mvpw_get_widget_info(mythtv_channel, &wid2);
	mvpw_moveto(mythtv_date, wid.x, wid2.y+wid2.h);
	mvpw_get_widget_info(mythtv_date, &wid2);
	mvpw_moveto(mythtv_description, wid.x, wid2.y+wid2.h);

	mvpw_attach(mythtv_channel, mythtv_record, MVPW_DIR_RIGHT);

	/*
	 * MythTV menu info
	 */
	mvpw_get_widget_info(mythtv_channel, &info);
	shows_widget = mvpw_create_text(NULL, info.x, info.y,
					300, h, description_attr.bg,
					description_attr.border,
					description_attr.border_size);
	episodes_widget = mvpw_create_text(NULL, 50, 80,
					   300, h, description_attr.bg,
					   description_attr.border,
					   description_attr.border_size);
	freespace_widget = mvpw_create_text(NULL, 50, 80,
					    300, h, description_attr.bg,
					    description_attr.border,
					    description_attr.border_size);
	mvpw_set_text_attr(shows_widget, &description_attr);
	mvpw_set_text_attr(episodes_widget, &description_attr);
	mvpw_set_text_attr(freespace_widget, &description_attr);

	mvpw_attach(shows_widget, episodes_widget, MVPW_DIR_DOWN);
	mvpw_attach(episodes_widget, freespace_widget, MVPW_DIR_DOWN);

	/*
	 * mythtv popup menu
	 */
	w = 400;
	h = 5 * (mvpw_font_height(mythtv_popup_attr.font) + 8);
	x = (si.cols - w) / 2;
	y = (si.rows - h) / 2;

	mythtv_popup = mvpw_create_menu(NULL, x, y, w, h,
					mythtv_popup_attr.bg,
					mythtv_popup_attr.border,
					mythtv_popup_attr.border_size);

	mvpw_set_menu_attr(mythtv_popup, &mythtv_popup_attr);

	mvpw_set_menu_title(mythtv_popup, "Recording Menu");

	mvpw_set_key(mythtv_popup, mythtv_popup_key_callback);

	/*
	 * mythtv show info
	 */
	w = si.cols - 100;
	h = si.rows - 40;
	x = (si.cols - w) / 2;
	y = (si.rows - h) / 2;
	mythtv_info = mvpw_create_text(NULL, x, y, w, h,
				       mythtv_info_attr.bg,
				       mythtv_info_attr.border,
				       mythtv_info_attr.border_size);
	mvpw_set_key(mythtv_info, mythtv_info_key_callback);

	mvpw_set_text_attr(mythtv_info, &mythtv_info_attr);

	mvpw_raise(mythtv_browser);
	mvpw_raise(mythtv_menu);
	mvpw_raise(mythtv_popup);
	mvpw_raise(mythtv_info);

	return 0;
}

static int
replaytv_browser_init(void)
{
	splash_update("Creating ReplayTV browser");
	replay_gui_init();
	return 0;
}

static void
main_select_callback(mvp_widget_t *widget, char *item, void *key)
{
	int k = (int)key;

	switch (k) {
	case MM_EXIT:
#ifndef MVPMC_HOST
		/*
		 * Do an orderly shutdown, if possible.
		 */
		system("/sbin/reboot");
		sleep(1);
		reboot(LINUX_REBOOT_CMD_RESTART);
#endif
		exit(0);
		break;
	case MM_FILESYSTEM:
		mvpw_hide(main_menu);
		mvpw_hide(mvpmc_logo);
		mvpw_hide(fb_image);

		fb_update(file_browser);

		mvpw_show(file_browser);
		mvpw_focus(file_browser);
		break;
	case MM_SETTINGS:
		mvpw_hide(main_menu);
		mvpw_hide(setup_image);

		settings_attr.hilite_bg = MVPW_BLUE;
		mvpw_set_menu_attr(settings, &settings_attr);

		settings_attr.hilite_bg = MVPW_DARKGREY;
		mvpw_set_menu_attr(sub_settings, &settings_attr);

		mvpw_show(settings);
		mvpw_show(sub_settings);
		mvpw_focus(settings);
		break;
	case MM_MYTHTV:
		mvpw_hide(main_menu);
		mvpw_hide(mvpmc_logo);
		mvpw_hide(mythtv_image);

		mvpw_show(mythtv_logo);
		mvpw_show(mythtv_menu);
		mvpw_focus(mythtv_menu);

		mythtv_main_menu = 1;
		break;
	case MM_REPLAYTV:
		mvpw_hide(main_menu);
		mvpw_hide(mvpmc_logo);
		mvpw_hide(replaytv_image);
		mvpw_hide(fb_image);

		replaytv_device_update();
		replaytv_show_device_menu();
		break;
	case MM_THEMES:
		mvpw_hide(main_menu);
		mvpw_hide(setup_image);

		mvpw_show(themes_menu);
		mvpw_focus(themes_menu);
		break;
	case MM_ABOUT:
		mvpw_show(about);
		mvpw_focus(about);
		break;
	}
}

static void
main_hilite_callback(mvp_widget_t *widget, char *item, void *key, int hilite)
{
	int k = (int)key;

	if (!init_done)
		return;

	if (hilite) {
		switch (k) {
		case MM_SETTINGS:
			mvpw_show(setup_image);
			break;
		case MM_THEMES:
			mvpw_show(setup_image);
			break;
		case MM_FILESYSTEM:
			mvpw_show(fb_image);
			break;
		case MM_MYTHTV:
			mvpw_show(mythtv_image);
			break;
		case MM_REPLAYTV:
			mvpw_show(replaytv_image);
			break;
		case MM_ABOUT:
			mvpw_show(about_image);
			break;
		case MM_EXIT:
			mvpw_show(exit_image);
			break;
		}
	} else {
		switch (k) {
		case MM_SETTINGS:
			mvpw_hide(setup_image);
			break;
		case MM_THEMES:
			mvpw_hide(setup_image);
			break;
		case MM_FILESYSTEM:
			mvpw_hide(fb_image);
			break;
		case MM_MYTHTV:
			mvpw_hide(mythtv_image);
			break;
		case MM_REPLAYTV:
			mvpw_hide(replaytv_image);
			break;
		case MM_ABOUT:
			mvpw_hide(about_image);
			break;
		case MM_EXIT:
			mvpw_hide(exit_image);
			break;
		}
	}
}

int
main_menu_init(char *server, char *replaytv)
{
	mvpw_image_info_t iid;
	mvpw_widget_info_t wid;
	char file[128];
	int w;
	int i, theme_count = 0;

	for (i=0; i<THEME_MAX; i++) {
		if (theme_list[i].path != NULL) {
			theme_count++;
		}
	}

	splash_update("Creating setup image");

	snprintf(file, sizeof(file), "%s/wrench.png", imagedir);
	if (mvpw_get_image_info(file, &iid) < 0)
		return -1;
	setup_image = mvpw_create_image(NULL, 50, 25, iid.width, iid.height,
					MVPW_BLACK, 0, 0);
	mvpw_set_image(setup_image, file);

	splash_update("Creating file browser image");

	snprintf(file, sizeof(file), "%s/video_folder.png", imagedir);
	if (mvpw_get_image_info(file, &iid) < 0)
		return -1;
	fb_image = mvpw_create_image(NULL, 50, 25,
				     iid.width, iid.height, MVPW_BLACK, 0, 0);
	mvpw_set_image(fb_image, file);

	splash_update("Creating MythTV image");

	snprintf(file, sizeof(file), "%s/tv2.png", imagedir);
	if (mvpw_get_image_info(file, &iid) < 0)
		return -1;
	mythtv_image = mvpw_create_image(NULL, 50, 25, iid.width, iid.height,
					 MVPW_BLACK, 0, 0);
	mvpw_set_image(mythtv_image, file);

	splash_update("Creating ReplayTV image");

	snprintf(file, sizeof(file), "%s/replaytv1.png", imagedir);
	if (mvpw_get_image_info(file, &iid) < 0)
		return -1;
	replaytv_image = mvpw_create_image(NULL, 50, 25, iid.width, iid.height,
					   MVPW_BLACK, 0, 0);
	mvpw_set_image(replaytv_image, file);

	splash_update("Creating about image");

	snprintf(file, sizeof(file), "%s/unknown.png", imagedir);
	if (mvpw_get_image_info(file, &iid) < 0)
		return -1;
	about_image = mvpw_create_image(NULL, 50, 25, iid.width, iid.height,
					MVPW_BLACK, 0, 0);
	mvpw_set_image(about_image, file);

	splash_update("Creating exit image");

	snprintf(file, sizeof(file), "%s/stop.png", imagedir);
	if (mvpw_get_image_info(file, &iid) < 0)
		return -1;
	exit_image = mvpw_create_image(NULL, 50, 25, iid.width, iid.height,
				       MVPW_BLACK, 0, 0);
	mvpw_set_image(exit_image, file);

	splash_update("Creating mvpmc logo");

	snprintf(file, sizeof(file), "%s/mvpmc_logo.png", imagedir);
	if (mvpw_get_image_info(file, &iid) < 0)
		return -1;
	mvpmc_logo = mvpw_create_image(NULL, 50, 25, iid.width, iid.height,
				       MVPW_BLACK, 0, 0);
	mvpw_set_image(mvpmc_logo, file);

	splash_update("Creating main menu");

	w = (iid.width < 300) ? 300 : iid.width;
	main_menu = mvpw_create_menu(NULL, 50, 50, w, si.rows-150,
				     attr.bg, attr.border, attr.border_size);

	mvpw_attach(mvpmc_logo, main_menu, MVPW_DIR_DOWN);
	mvpw_attach(main_menu, setup_image, MVPW_DIR_RIGHT);

	mvpw_get_widget_info(setup_image, &wid);
	mvpw_moveto(fb_image, wid.x, wid.y);
	mvpw_moveto(mythtv_image, wid.x, wid.y);
	mvpw_moveto(replaytv_image, wid.x, wid.y);
	mvpw_moveto(about_image, wid.x, wid.y);
	mvpw_moveto(exit_image, wid.x, wid.y);

	mvpw_set_menu_attr(main_menu, &attr);

	item_attr.select = main_select_callback;
	item_attr.hilite = main_hilite_callback;

	item_attr.fg = attr.fg;
	item_attr.bg = attr.bg;

	if (server)
		mvpw_add_menu_item(main_menu, "MythTV",
				   (void*)MM_MYTHTV, &item_attr);
	if (replaytv)
		mvpw_add_menu_item(main_menu, "ReplayTV",
				   (void*)MM_REPLAYTV, &item_attr);
	mvpw_add_menu_item(main_menu, "Filesystem",
			   (void*)MM_FILESYSTEM, &item_attr);
	mvpw_add_menu_item(main_menu, "Settings",
			   (void*)MM_SETTINGS, &item_attr);
	if (theme_count > 1)
		mvpw_add_menu_item(main_menu, "Themes",
				   (void*)MM_THEMES, &item_attr);
	mvpw_add_menu_item(main_menu, "About",
			   (void*)MM_ABOUT, &item_attr);
#ifdef MVPMC_HOST
	mvpw_add_menu_item(main_menu, "Exit",
			   (void*)MM_EXIT, &item_attr);
#else
	mvpw_add_menu_item(main_menu, "Reboot",
			   (void*)MM_EXIT, &item_attr);
#endif

	mvpw_set_key(main_menu, main_menu_callback);

	return 0;
}

static int
about_init(void)
{
	int h, w, x, y;
	char text[256];
	char buf[] = 
		"http://mvpmc.sourceforge.net/\n\n"
		"Audio: mp3, ogg, wav, ac3\n"
		"Video: mpeg1, mpeg2\n"
		"Images: bmp, gif, png, jpeg\n"
		"Servers: MythTV, ReplayTV, NFS, CIFS\n";

	splash_update("Creating about dialog");

	h = (mvpw_font_height(about_attr.font) +
	     (2 * 2)) * 9;
	w = 500;

	x = (si.cols - w) / 2;
	y = (si.rows - h) / 2;

	if (version[0] == '\0') {
		snprintf(text, sizeof(text),
			 "MediaMVP Media Center\n%s", buf);
	} else {
		snprintf(text, sizeof(text),
			 "MediaMVP Media Center\nVersion %s\n%s",
			 version, buf);
	}

	about = mvpw_create_dialog(NULL, x, y, w, h,
				   about_attr.bg,
				   about_attr.border, about_attr.border_size);

	mvpw_set_dialog_attr(about, &about_attr);

	mvpw_set_dialog_title(about, "About");
	mvpw_set_dialog_text(about, text);

	mvpw_set_key(about, warn_key_callback);

	return 0;
}

static int
image_init(void)
{
	splash_update("Creating image viewer");

	iw = mvpw_create_image(NULL, 0, 0, si.cols, si.rows, 0, 0, 0);
	mvpw_set_key(iw, iw_key_callback);

	return 0;
}

static int
osd_init(void)
{
	mvp_widget_t *widget, *contain, *progress;
	int h, w, x, y;

	splash_update("Creating OSD");

	h = mvpw_font_height(display_attr.font) +
		(2 * display_attr.margin);
	w = mvpw_font_width(display_attr.font, " 000% ");

	/*
	 * State widgets for pause, mute, fast forward, zoom
	 */
	mute_widget = mvpw_create_text(NULL, 50, 25, 75, h,
				       display_attr.bg,
				       display_attr.border,
				       display_attr.border_size);
	mvpw_set_text_attr(mute_widget, &display_attr);
	mvpw_set_text_str(mute_widget, "MUTE");

	pause_widget = mvpw_create_text(NULL, 50, 25, 75, h,
					display_attr.bg,
					display_attr.border,
					display_attr.border_size);
	mvpw_set_text_attr(pause_widget, &display_attr);
	mvpw_set_text_str(pause_widget, "PAUSE");

	ffwd_widget = mvpw_create_text(NULL, 50, 25, 75, h,
				       display_attr.bg,
				       display_attr.border,
				       display_attr.border_size);
	mvpw_set_text_attr(ffwd_widget, &display_attr);
	mvpw_set_text_str(ffwd_widget, "FFWD");

	zoom_widget = mvpw_create_text(NULL, 50, 25, 75, h,
				       display_attr.bg,
				       display_attr.border,
				       display_attr.border_size);
	mvpw_set_text_attr(zoom_widget, &display_attr);
	mvpw_set_text_str(zoom_widget, "ZOOM");

	clock_widget = mvpw_create_text(NULL, 50, 25, 150, h,
					display_attr.bg,
					display_attr.border,
					display_attr.border_size);
	mvpw_set_text_attr(clock_widget, &display_attr);
	mvpw_set_text_str(clock_widget, "");

	mvpw_attach(mute_widget, pause_widget, MVPW_DIR_RIGHT);
	mvpw_attach(pause_widget, ffwd_widget, MVPW_DIR_RIGHT);
	mvpw_attach(ffwd_widget, zoom_widget, MVPW_DIR_RIGHT);
	mvpw_attach(zoom_widget, clock_widget, MVPW_DIR_RIGHT);

	/*
	 * OSD widgets
	 */
	contain = mvpw_create_container(NULL, 50, 80,
					300, h, display_attr.bg,
					display_attr.border,
					display_attr.border_size);
	progress = contain;
	widget = mvpw_create_text(contain, 0, 0, w, h, display_attr.bg,
				  display_attr.border,
				  display_attr.border_size);
	mvpw_set_text_attr(widget, &display_attr);
	mvpw_set_text_str(widget, "0%");
	mvpw_show(widget);
	offset_widget = widget;
	widget = mvpw_create_graph(contain, w, 0, 300-w, h/2,
				   0x80000000, 0, 0);
	mvpw_set_graph_attr(widget, &offset_graph_attr);
	mvpw_show(widget);
	offset_bar = widget;
	add_osd_widget(contain, OSD_PROGRESS, 1, NULL);

	mvpw_attach(mute_widget, contain, MVPW_DIR_DOWN);
	mvpw_set_text_attr(mute_widget, &display_attr);
	mvpw_show(widget);

	time_widget = mvpw_create_text(NULL, 0, 0, 200, h,
				       display_attr.bg,
				       display_attr.border,
				       display_attr.border_size);
	mvpw_set_text_attr(time_widget, &display_attr);
	mvpw_set_text_str(time_widget, "");
	mvpw_attach(contain, time_widget, MVPW_DIR_DOWN);
	add_osd_widget(time_widget, OSD_TIMECODE, 1, NULL);

	bps_widget = mvpw_create_text(NULL, 0, 0, 100, h,
				      display_attr.bg,
				       display_attr.border,
				       display_attr.border_size);
	mvpw_set_text_attr(bps_widget, &display_attr);
	mvpw_set_text_str(bps_widget, "");
	mvpw_attach(time_widget, bps_widget, MVPW_DIR_RIGHT);
	add_osd_widget(bps_widget, OSD_BITRATE, 1, NULL);

	/*
	 * myth OSD
	 */
	x = si.cols - 475;
	y = si.rows - 125;
	h = mvpw_font_height(mythtv_program_attr.font) +
		(mvpw_font_height(mythtv_description_attr.font) * 3) +
		(2 * display_attr.margin);
	contain = mvpw_create_container(NULL, x, y,
					400, h,
					mythtv_description_attr.bg,
					mythtv_description_attr.border,
					mythtv_description_attr.border_size);
	mythtv_program_widget = contain;
	h = mvpw_font_height(mythtv_program_attr.font) +
		(2 * display_attr.margin);
	widget = mvpw_create_text(contain, 0, 0, 400, h,
				  mythtv_program_attr.bg,
				  mythtv_program_attr.border,
				  mythtv_program_attr.border_size);
	mvpw_set_text_attr(widget, &mythtv_program_attr);
	mvpw_set_text_str(widget, "");
	mvpw_show(widget);
	mythtv_osd_program = widget;
	h = (mvpw_font_height(mythtv_description_attr.font) * 3) +
		(2 * display_attr.margin);
	widget = mvpw_create_text(contain, 0, 0, 400, h,
				  mythtv_description_attr.bg,
				  mythtv_description_attr.border,
				  mythtv_description_attr.border_size);
	mvpw_set_text_attr(widget, &mythtv_description_attr);
	mvpw_set_text_str(widget, "");
	mvpw_show(widget);
	mythtv_osd_description = widget;
	mvpw_attach(mythtv_osd_program, mythtv_osd_description, MVPW_DIR_DOWN);

	/*
	 * file browser OSD
	 */
	h = mvpw_font_height(description_attr.font) +
		(2 * display_attr.margin);
	fb_program_widget = mvpw_create_text(NULL, x, y, 400, h*3,
					     description_attr.bg,
					     description_attr.border,
					     description_attr.border_size);
	mvpw_set_text_attr(fb_program_widget, &mythtv_description_attr);
	mvpw_set_text_str(fb_program_widget, "");

	/*
	 * ReplayTV OSD
	 * Comon stuff: progress meter, demux, clock
	 */
	x = si.cols - 475;
	y = si.rows - 125;
	contain = mvpw_create_container(NULL, x, y,
					300, h*2, 0x80000000, 0, 0);
	mvpw_attach(progress, contain, MVPW_DIR_RIGHT);
	widget = mvpw_create_graph(contain, 0, 0, 300, h,
				   0x80000000, 0, 0);
	mvpw_set_graph_attr(widget, &demux_video_graph_attr);
	mvpw_show(widget);
	demux_video = widget;
	widget = mvpw_create_graph(contain, 0, 0, 300, h,
				   0x80000000, 0, 0);
	mvpw_set_graph_attr(widget, &demux_audio_graph_attr);
	mvpw_show(widget);
	demux_audio = widget;
	mvpw_attach(demux_video, demux_audio, MVPW_DIR_DOWN);
	add_osd_widget(contain, OSD_DEMUX, 0, NULL);

	add_osd_widget(clock_widget, OSD_CLOCK, 0, NULL);

	return 0;
}

int
mw_init(char *server, char *replaytv)
{
	int h, w, x, y;
	char buf[128];

	mvpw_init();
	root = mvpw_get_root();
	mvpw_set_key(root, root_callback);
	mvpw_set_bg(root, MVPW_BLACK);

	mvpw_get_screen_info(&si);

	printf("screen is %d x %d\n", si.cols, si.rows);

	if (version[0] != '\0')
		snprintf(buf, sizeof(buf),
			 "MediaMVP Media Center\nVersion %s\n%s",
			 version, compile_time);
	else
		snprintf(buf, sizeof(buf), "MediaMVP Media Center\n%s",
			 compile_time);

	h = (mvpw_font_height(splash_attr.font) +
	     (2 * splash_attr.margin)) * 3;
	w = mvpw_font_width(splash_attr.font, buf) + 8;
	w = 400;

	x = (si.cols - w) / 2;
	y = (si.rows - h) / 2;

	splash_title = mvpw_create_text(NULL, x, y, w, h,
					splash_attr.bg,
					splash_attr.border,
					splash_attr.border_size);
	mvpw_set_text_attr(splash_title, &splash_attr);
	y += h;
	h *= 2;
	h = (mvpw_font_height(splash_attr.font) + (2 * splash_attr.margin));
	splash = mvpw_create_text(NULL, x, y, w, h, splash_attr.bg,
				  splash_attr.border, splash_attr.border_size);
	mvpw_set_text_attr(splash, &splash_attr);

	w = si.cols - 300;
	x = (si.cols - w) / 2;
	y += (h*2);

	splash_graph = mvpw_create_graph(NULL, x, y, w, h,
					 MVPW_BLACK, MVPW_LIGHTGREY, 2);
	mvpw_set_graph_attr(splash_graph, &splash_graph_attr);

	mvpw_set_graph_current(splash_graph, 0);

	mvpw_set_text_str(splash_title, buf);

	mvpw_show(splash);
	mvpw_show(splash_graph);
	mvpw_show(splash_title);
	mvpw_expose(splash_graph);
	mvpw_event_flush();

	return 0;
}

int
popup_init(void)
{
	int x, y, w, h;

	h = 8 * (mvpw_font_height(popup_attr.font) + 8);
	w = mvpw_font_width(popup_attr.font, "On Screen Display") * 1.5;
	x = (si.cols - w) / 2;
	y = (si.rows - h) / 2;

	popup_menu = mvpw_create_menu(NULL, x, y, w, h,
				      popup_attr.bg,
				      popup_attr.border,
				      popup_attr.border_size);

	mvpw_set_menu_attr(popup_menu, &popup_attr);

	mvpw_set_menu_title(popup_menu, "Settings");

	popup_item_attr.select = popup_select_callback;
	popup_item_attr.fg = popup_attr.fg;
	popup_item_attr.bg = popup_attr.bg;

	mvpw_add_menu_item(popup_menu, "Audio Streams",
			   (void*)MENU_AUDIO_STREAM, &popup_item_attr);
	mvpw_add_menu_item(popup_menu, "Video Streams",
			   (void*)MENU_VIDEO_STREAM, &popup_item_attr);
	mvpw_add_menu_item(popup_menu, "Subtitles",
			   (void*)MENU_SUBTITLES, &popup_item_attr);
	mvpw_add_menu_item(popup_menu, "On Screen Display",
			   (void*)MENU_OSD, &popup_item_attr);
	mvpw_add_menu_item(popup_menu, "Brightness",
			   (void*)MENU_BRIGHT, &popup_item_attr);

	mvpw_set_key(popup_menu, popup_key_callback);

	/*
	 * audio stream
	 */
	popup_attr.checkboxes = 1;
	audio_stream_menu = mvpw_create_menu(NULL, x, y, w, h,
					     popup_attr.bg,
					     popup_attr.border,
					     popup_attr.border_size);
	mvpw_set_menu_attr(audio_stream_menu, &popup_attr);
	mvpw_set_menu_title(audio_stream_menu, "Audio Streams");
	mvpw_set_key(audio_stream_menu, popup_key_callback);

	/*
	 * video menu
	 */
	video_stream_menu = mvpw_create_menu(NULL, x, y, w, h,
					     popup_attr.bg,
					     popup_attr.border,
					     popup_attr.border_size);
	mvpw_set_menu_attr(video_stream_menu, &popup_attr);
	mvpw_set_menu_title(video_stream_menu, "Video Streams");
	mvpw_set_key(video_stream_menu, popup_key_callback);

	/*
	 * subtitle menu
	 */
	subtitle_stream_menu = mvpw_create_menu(NULL, x, y, w, h,
					     popup_attr.bg,
					     popup_attr.border,
					     popup_attr.border_size);
	mvpw_set_menu_attr(subtitle_stream_menu, &popup_attr);
	mvpw_set_menu_title(subtitle_stream_menu, "Subtitles");
	mvpw_set_key(subtitle_stream_menu, popup_key_callback);

	/*
	 * osd menu
	 */
	osd_menu = mvpw_create_menu(NULL, x, y, w, h,
				    popup_attr.bg,
				    popup_attr.border,
				    popup_attr.border_size);
	mvpw_set_menu_attr(osd_menu, &popup_attr);
	mvpw_set_menu_title(osd_menu, "OSD Settings");
	mvpw_set_key(osd_menu, popup_key_callback);

	/*
	 * osd sub-menu
	 */
	popup_item_attr.select = osd_select_callback;
	if (popup_attr.checkbox_fg)
		popup_item_attr.checkbox_fg = popup_attr.checkbox_fg;
	mvpw_add_menu_item(osd_menu, "Bitrate", (void*)OSD_BITRATE,
			   &popup_item_attr);
	mvpw_add_menu_item(osd_menu, "Clock", (void*)OSD_CLOCK,
			   &popup_item_attr);
	mvpw_add_menu_item(osd_menu, "Demux Info", (void*)OSD_DEMUX,
			   &popup_item_attr);
	mvpw_add_menu_item(osd_menu, "Progress Meter", (void*)OSD_PROGRESS,
			   &popup_item_attr);
	mvpw_add_menu_item(osd_menu, "Program Info", (void*)OSD_PROGRAM,
			   &popup_item_attr);
	mvpw_add_menu_item(osd_menu, "Timecode", (void*)OSD_TIMECODE,
			   &popup_item_attr);

	mvpw_check_menu_item(osd_menu, (void*)OSD_BITRATE, 1);
	mvpw_check_menu_item(osd_menu, (void*)OSD_CLOCK, 0);
	mvpw_check_menu_item(osd_menu, (void*)OSD_DEMUX, 0);
	mvpw_check_menu_item(osd_menu, (void*)OSD_PROGRESS, 1);
	mvpw_check_menu_item(osd_menu, (void*)OSD_PROGRAM, 1);
	mvpw_check_menu_item(osd_menu, (void*)OSD_TIMECODE, 1);

	/*
	 * Brightness menu
	 */
	bright_menu = mvpw_create_menu(NULL, x, y, w, h,
				       popup_attr.bg,
				       popup_attr.border,
				       popup_attr.border_size);
	mvpw_set_menu_attr(bright_menu, &popup_attr);
	mvpw_set_menu_title(bright_menu, "Brightness");
	mvpw_set_key(bright_menu, popup_key_callback);

	popup_item_attr.select = bright_select_callback;
	mvpw_add_menu_item(bright_menu, "-3", (void*)-3, &popup_item_attr);
	mvpw_add_menu_item(bright_menu, "-2", (void*)-2, &popup_item_attr);
	mvpw_add_menu_item(bright_menu, "-1", (void*)-1, &popup_item_attr);
	mvpw_add_menu_item(bright_menu, "0", (void*)0, &popup_item_attr);
	mvpw_add_menu_item(bright_menu, "+1", (void*)1, &popup_item_attr);
	mvpw_add_menu_item(bright_menu, "+2", (void*)2, &popup_item_attr);
	mvpw_add_menu_item(bright_menu, "+3", (void*)3, &popup_item_attr);

	mvpw_check_menu_item(bright_menu, (void*)0, 1);

	return 0;
}

static void
screensaver_timer(mvp_widget_t *widget)
{
	mvpw_widget_info_t info;
	int x, y;

	mvpw_set_timer(screensaver, screensaver_timer, 1000);

	mvpw_get_widget_info(mvpmc_logo, &info);

	x = rand() % (si.cols - info.w);
	y = rand() % (si.rows - info.h);

	mvpw_moveto(mvpmc_logo, x, y);
}

static void
screensaver_event(mvp_widget_t *widget, int activate)
{
	static int visible = 0;

	if (activate) {
		visible = mvpw_visible(mvpmc_logo);
		mvpw_unattach(mvpmc_logo, MVPW_DIR_DOWN);
		mvpw_reparent(mvpmc_logo, screensaver);
		mvpw_show(mvpmc_logo);
		mvpw_show(screensaver);
		mvpw_focus(screensaver);
		screensaver_timer(widget);
	} else {
		mvpw_set_timer(screensaver, NULL, 0);
		mvpw_hide(screensaver);
		mvpw_hide(mvpmc_logo);
		mvpw_reparent(mvpmc_logo, NULL);
		mvpw_lower(mvpmc_logo);
		mvpw_attach(main_menu, mvpmc_logo, MVPW_DIR_UP);
		if (visible)
			mvpw_show(mvpmc_logo);
	}
}

void
screensaver_enable(void)
{
	printf("screensaver enable\n");

	screensaver_enabled = 1;

	mvpw_set_screensaver(screensaver, screensaver_timeout,
			     screensaver_event);
}

void
screensaver_disable(void)
{
	printf("screensaver disable\n");

	screensaver_enabled = 0;

	mvpw_set_screensaver(NULL, 0, NULL);
}

static int
screensaver_init(void)
{
	splash_update("Creating screensaver");

	screensaver = mvpw_create_container(NULL, 0, 0,
					    si.cols, si.rows,
					    MVPW_BLACK, 0, 0);

	screensaver_enable();

	return 0;
}

void
gui_error(char *msg)
{
	char *key = "\n\nPress any key to continue.";
	char *buf;

	fprintf(stderr, "%s\n", msg);

	if ((buf=alloca(strlen(msg) + strlen(key) + 1)) == NULL)
		buf = msg;
	else
		sprintf(buf, "%s%s", msg, key);

	mvpw_set_dialog_text(warn_widget, buf);
	mvpw_show(warn_widget);

	mvpw_event_flush();
}

void
gui_error_clear(void)
{
	mvpw_hide(warn_widget);
}

void
warn_init(void)
{
	int h, w, x, y;
	char file[256];

	snprintf(file, sizeof(file), "%s/warning.png", imagedir);

	h = (mvpw_font_height(warn_attr.font) +
	     (2 * 2)) * 6;
	w = 400;

	x = (si.cols - w) / 2;
	y = (si.rows - h) / 2;

	warn_widget = mvpw_create_dialog(NULL, x, y, w, h,
					 warn_attr.bg,
					 warn_attr.border,
					 warn_attr.border_size);

	warn_attr.image = file;

	mvpw_set_dialog_attr(warn_widget, &warn_attr);

	mvpw_set_dialog_title(warn_widget, "Warning");

	mvpw_set_key(warn_widget, warn_key_callback);
}

static void*
busy_loop(void *arg)
{
	int off = 0;
	int delta = 1;
	int count = 0;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);

	while (1) {
		pthread_cond_wait(&busy_cond, &mutex);
		count = 0;
		off = 0;
		delta = 1;
		while (busy) {
			/*
			 * Do not show the widget for the first 1/4 second
			 */
			if (++count > 1) {
				off += delta;
				if ((off >= busy_graph_attr.max) ||
				    (off <= 0))
					delta *= -1;
				mvpw_set_graph_current(busy_graph, off);
				mvpw_show(busy_widget);
				mvpw_event_flush();
			}
			usleep(1000*250);  /* 0.25 seconds */
		}
	}

	return NULL;
}

void
busy_start(void)
{
	busy = 1;
	pthread_cond_signal(&busy_cond);
}

void
busy_end(void)
{
	busy = 0;
	mvpw_hide(busy_widget);
}

void
busy_init(void)
{
	int h, w, x, y;
	mvp_widget_t *text, *graph;

	h = (mvpw_font_height(busy_text_attr.font) +
	     (2 * 2)) * 2;
	w = 200;

	x = (si.cols - w) / 2;
	y = (si.rows - h) / 2;

	busy_widget = mvpw_create_container(NULL, x, y, w, h,
					    busy_text_attr.bg,
					    busy_text_attr.border,
					    busy_text_attr.border_size);

	text = mvpw_create_text(busy_widget, 0, 0, w, h/2,
				busy_text_attr.bg, 0, 0);
	graph = mvpw_create_graph(busy_widget, 0, 0, w, h/2,
				  busy_graph_attr.bg, 0, 0);

	mvpw_set_graph_attr(graph, &busy_graph_attr);
	mvpw_set_text_attr(text, &busy_text_attr);
	mvpw_set_text_str(text, "Please wait...");

	mvpw_attach(text, graph, MVPW_DIR_DOWN);

	mvpw_show(text);
	mvpw_show(graph);

	busy_graph = graph;

	pthread_create(&busy_thread, &thread_attr_small, busy_loop, NULL);
}

int
gui_init(char *server, char *replaytv)
{
	char buf[128];
	demux_attr_t *attr;

	snprintf(buf, sizeof(buf), "Initializing GUI");
	mvpw_set_text_str(splash, buf);
	mvpw_expose(splash);
	mvpw_event_flush();

	attr = demux_get_attr(handle);
	demux_video_graph_attr.max = attr->video.bufsz;
	demux_audio_graph_attr.max = attr->audio.bufsz;

	printf("Demux size video: %d  audio: %d\n",
	       attr->video.bufsz, attr->audio.bufsz);

	if (main_menu_init(server, replaytv) < 0)
		return -1;
	if (myth_browser_init() < 0)
		return -1;
	file_browser_init();
	settings_init();
	themes_init();
	about_init();
	image_init();
	osd_init();
	replaytv_browser_init(); // must come after osd_init
	popup_init();
	playlist_init();
	busy_init();
	warn_init();
	screensaver_init();

	mvpw_destroy(splash);
	mvpw_destroy(splash_graph);
	mvpw_destroy(splash_title);

	init_done = 1;

	if (server)
		mvpw_show(mythtv_image);
	else if (replaytv)
		mvpw_show(replaytv_image);
	else
		mvpw_show(fb_image);
	mvpw_show(mvpmc_logo);
	mvpw_show(main_menu);
	mvpw_lower(root);

	mvpw_focus(main_menu);

	return 0;
}
