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
#include <freerdp/rail.h>


/*Non-null terminated UNICODE string with characters in LE byte order*/

typedef struct _RAIL_STRING
{
	uint16  length;
	uint8	*buffer;
} RAIL_STRING;


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

typedef struct _RAIL_VCHANNEL_DATA_SENDER
{
	void* data_sender_object;
	void  (*send_rail_vchannel_data)(void* sender_object, void* data, size_t length);

} RAIL_VCHANNEL_DATA_SENDER;

typedef struct _RAIL_VCHANNEL_EVENT_SENDER
{
	void * event_sender_object;
	void (*send_rail_vchannel_event)(void * ui_event_sender_object);
}
RAIL_VCHANNEL_EVENT_SENDER;

typedef struct _RAIL_SESSION
{
	RAIL_VCHANNEL_DATA_SENDER* data_sender;
	RAIL_VCHANNEL_EVENT_SENDER* event_sender;

	UNICONV * uniconv;

} RAIL_SESSION;

RAIL_SESSION *
rail_core_session_new(
	RAIL_VCHANNEL_DATA_SENDER *data_sender,
	RAIL_VCHANNEL_EVENT_SENDER *event_sender
	);

void
rail_core_session_free(RAIL_SESSION * rail_session);

// RAIL Core Handlers for events from channel plugin interface

void
rail_core_on_channel_connected(RAIL_SESSION* rail_session);

void
rail_core_on_channel_terminated(RAIL_SESSION* rail_session);

void
rail_core_on_channel_data_received(
	RAIL_SESSION * rail_session,
	void*  data,
	size_t length
	);

// RAIL Core Handlers for events from UI

void
rail_core_handle_ui_update_client_system_param(
	RAIL_SESSION* rail_session,
	RAIL_CLIENT_SYSPARAM * sysparam
	);

void
rail_core_handle_ui_client_execute(
	RAIL_SESSION* session,
	boolean exec_or_file_is_file_path,
	const char* exe_or_file,
	const char* working_directory,
	const char* arguments
	);



// RAIL Core Handlers for events from channel orders reader
void
rail_core_handle_server_hadshake(
	RAIL_SESSION* session,
	uint32 build_number
	);

void
rail_core_handle_exec_result(
	RAIL_SESSION* session,
	uint16 flags,
	uint16 exec_result,
	uint32 raw_result,
	RAIL_UNICODE_STRING * exe_or_file
	);

void
rail_core_handle_server_sysparam(
	RAIL_SESSION* session,
	RAIL_SERVER_SYSPARAM * sysparam
	);

void
rail_core_handle_server_movesize(
	RAIL_SESSION* session,
	uint32 window_id,
	uint16 move_size_started,
	uint16 move_size_type,
	uint16 pos_x,
	uint16 pos_y
    );

void
rail_core_handle_server_minmax_info(
	RAIL_SESSION* session,
	uint32 window_id,
	uint16 max_width, uint16 max_height,
	uint16 max_pos_x, uint16 max_pos_y,
	uint16 min_track_width, uint16 min_track_height,
	uint16 max_track_width,	uint16 max_track_height
    );

void
rail_core_handle_server_langbar_info(
		RAIL_SESSION* session,
		uint32 langbar_status
		);

void
rail_core_handle_server_get_app_resp(
		RAIL_SESSION* session,
		uint32 window_id,
		RAIL_UNICODE_STRING * app_id
		);

#endif	// __RAIL_CORE_H
