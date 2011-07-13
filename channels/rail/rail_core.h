/*
   FreeRDP: A Remote Desktop Protocol client.
   Remote Applications Integrated Locally (RAIL)

   Copyright 2009 Marc-Andre Moreau <marcandre.moreau@gmail.com>
   Copyright 2011 Roman Barabanov <romanbarabanov@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __RAIL_CORE_H
#define	__RAIL_CORE_H


#include <freerdp/types.h>
#include <freerdp/utils/stream.h>
#include <freerdp/utils/unicode.h>

/*
 * RAIL library interface
 * Initialization
 * - create RAIL session according to settings
 *
 * For channels
 * - rail_register_channel_sender(session, sender)
 *   sender object is struct with sender opaque pointer and
 *   with function rail_send_vchannel_data(sender_object, data, length) *
 * - rail_on_channel_connected(session)
 * - rail_on_channel_data_received(session, data, length)
 * - rail_on_channel_terminated(session)
 *
 * For altsec window orders
 * - rail_on_altsec_window_order_received(session, data, length)
 *
 * For UI
 * - rail_register_ui_event_receiver(session, ui_event_receiver)
 * - rail_on_ui_...
 * - rail_on_ui_...
 * etc
 */

/*Non-null terminated UNICODE string with characters in LE byte order*/

typedef struct _RAIL_UNICODE_STRING
{
	uint16  length;
	uint8	*buffer;
} RAIL_UNICODE_STRING;

typedef struct _RAIL_STRING
{
	uint16  length;
	uint8	*buffer;
} RAIL_STRING;


typedef struct _RAIL_CACHED_ICON_INFO
{
	uint8 	cache_id;
	uint16 	cache_entry_id;
} RAIL_CACHED_ICON_INFO;

typedef struct _RAIL_ICON_INFO
{
	RAIL_CACHED_ICON_INFO cache_info;

	uint8  bpp;
	uint16 width;
	uint16 height;
	uint16 color_table_size;
	uint16 bits_mask_size;
	uint16 bits_color_size;

	uint8 *color_table;
	uint8 *bits_mask;
	uint8 *bits_color;

} RAIL_ICON_INFO;

typedef struct _RAIL_RECT_16
{
	uint16 left;
	uint16 top;
	uint16 right;
	uint16 bottom;

} RAIL_RECT_16;

typedef struct _RAIL_WINDOW_INFO
{
	RAIL_UNICODE_STRING title_info;

	uint32	owner_window_id;
	uint32	style;
	uint32	extened_style;
	uint8	show_state;

	uint32	client_offset_x;
	uint32	client_offset_y;

	uint32	client_area_width;
	uint32	client_area_height;
	uint8	rp_content;
	uint32	root_parent_handle;

	uint32	window_offset_x;
	uint32	window_offset_y;

	uint32	window_client_delta_x;
	uint32	window_client_delta_y;
	uint32	window_width;
	uint32	window_height;

	uint32			window_rects_number;
	RAIL_RECT_16* 	window_rects;

	uint32	visible_offset_x;
	uint32	visible_offset_y;

	uint32			visibility_rects_number;
	RAIL_RECT_16* 	visibility_rects;

} RAIL_WINDOW_INFO;

typedef struct _RAIL_NOTIFY_ICON_INFOTIP
{
	uint32 timeout;
	uint32 info_flags;

	RAIL_UNICODE_STRING info_tip_text;
	RAIL_UNICODE_STRING title;

} RAIL_NOTIFY_ICON_INFOTIP;

typedef struct _RAIL_NOTIFY_ICON_INFO
{
	uint32 version;
	uint32 state;

	RAIL_UNICODE_STRING 		tool_tip;
	RAIL_NOTIFY_ICON_INFOTIP 	info_tip;
	RAIL_ICON_INFO 				icon;
	RAIL_CACHED_ICON_INFO   	cached_icon;

} RAIL_NOTIFY_ICON_INFO;

typedef struct _RAIL_HIGHCONTRAST
{
	uint32 flags;
	RAIL_UNICODE_STRING color_scheme;

} RAIL_HIGHCONTRAST;

typedef struct _RAIL_CLIENT_SYSPARAM
{
	uint32 type;

	union
	{
		uint8 full_window_drag_enabled;
		uint8 menu_access_key_always_underlined;
		uint8 keyboard_for_user_prefered;
		uint8 left_right_mouse_buttons_swapped;
		RAIL_RECT_16 work_area;
		RAIL_RECT_16 display_resolution;
		RAIL_RECT_16 taskbar_size; // TODO: Maybe this is taskbar position?
		RAIL_HIGHCONTRAST high_contrast_system_info;
	} value;
} RAIL_CLIENT_SYSPARAM;

typedef struct _RAIL_SERVER_SYSPARAM
{
	uint32 type;

	union
	{
		uint8 screen_saver_enabled;
		uint8 screen_saver_lock_enabled;
	} value;
} RAIL_SERVER_SYSPARAM;



typedef struct _RAIL_VCHANNEL_SENDER
{
	void* sender_object;
	void  (*send_rail_vchannel_data)(void* sender_object, void* data, size_t length);

} RAIL_VCHANNEL_SENDER;

typedef struct _RAIL_UI_LISTENER
{
	void* ui_listener_object;

	// Notify UI about channel initialization for beginning timeout timer.
	void (*ui_on_rail_handshake_request_sent)(void * ui);
	void (*ui_on_rail_handshake_response_receved)(void * ui);

    // On this event UI must create a sequence of
	// rail_on_ui_sysparam_update() calls with all current client system parameters
	void (*ui_on_initial_client_sysparams_update)(void * ui);


	void (*ui_on_rail_exec_result_receved)(void * ui, uint16 exec_result,
		uint32 raw_result);

	void (*ui_on_rail_server_sysparam_received)(void * ui,
			RAIL_SERVER_SYSPARAM * sysparam);


} RAIL_UI_LISTENER;

typedef struct _RAIL_SESSION
{
	RAIL_VCHANNEL_SENDER* channel_sender;
	RAIL_UI_LISTENER    * ui_listener;

	UNICONV* uniconv;

    char rail_exe_or_file[64];
    char rail_working_directory[256];
    char rail_arguments[256];

	size_t number_icon_caches;
	size_t number_icon_cache_entries;

} RAIL_SESSION;

RAIL_SESSION *
rail_session_new(
	RAIL_VCHANNEL_SENDER *channel_sender,
	RAIL_UI_LISTENER *ui_listener
	);

void
rail_session_free(RAIL_SESSION * rail_session);

/*For processing Capacities*/
void
rail_get_rail_capset(
		RAIL_SESSION * rail_session,
		uint32 * rail_support_level
		);

void
rail_process_rail_capset(
		RAIL_SESSION * rail_session,
		uint32 rail_support_level
		);

void
rail_get_window_capset(
		RAIL_SESSION * rail_session,
		uint32 * window_support_level,
		uint8  * number_icon_caches,
		uint16 * number_icon_cache_entries
		);

void
rail_process_window_capset(
		RAIL_SESSION * rail_session,
		uint32 window_support_level,
		uint8  number_icon_caches,
		uint16 number_icon_cache_entries
		);


/* For processing Windowing Alternate Secondary Drawing Order */
void
rail_on_altsec_window_order_received(
		RAIL_SESSION * rail_session,
		void* data,
		size_t length
		);

void
rail_on_channel_connected(RAIL_SESSION* rail_session);

void
rail_on_channel_terminated(RAIL_SESSION* rail_session);

void
rail_on_channel_data_received(
	RAIL_SESSION * rail_session,
	void*  data,
	size_t length
	);

void
rail_on_ui_client_system_param_updated(
		RAIL_SESSION* rail_session,
		RAIL_CLIENT_SYSPARAM * sysparam
		);


// RAIL Core Handlers
void
rail_handle_server_hadshake(
	RAIL_SESSION* session,
	uint32 build_number
	);

void
rail_handle_exec_result(
	RAIL_SESSION* session,
	uint16 flags,
	uint16 exec_result,
	uint32 raw_result,
	RAIL_UNICODE_STRING * exe_or_file
	);

void
rail_handle_server_sysparam(
	RAIL_SESSION* session,
	RAIL_SERVER_SYSPARAM * sysparam
	);

void
rail_handle_server_movesize(
	RAIL_SESSION* session,
	uint32 window_id,
	uint16 move_size_started,
	uint16 move_size_type,
	uint16 pos_x,
	uint16 pos_y
    );

void
rail_handle_server_minmax_info(
	RAIL_SESSION* session,
	uint32 window_id,
	uint16 max_width, uint16 max_height,
	uint16 max_pos_x, uint16 max_pos_y,
	uint16 min_track_width, uint16 min_track_height,
	uint16 max_track_width,	uint16 max_track_height
    );

void
rail_handle_server_langbar_info(
		RAIL_SESSION* session,
		uint32 langbar_status
		);

void
rail_handle_server_get_app_resp(
		RAIL_SESSION* session,
		uint32 window_id,
		RAIL_UNICODE_STRING * app_id
		);

// RAIL library internal functions

void
in_rail_unicode_string(STREAM s, RAIL_UNICODE_STRING * string);

void
free_rail_unicode_string(RAIL_UNICODE_STRING * string);


#endif	// __RAIL_CORE_H
