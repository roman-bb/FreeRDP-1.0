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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freerdp/constants.h>
#include <freerdp/types.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/svc_plugin.h>
#include <freerdp/rail.h>

#include "rail_core.h"
#include "rail_channel_orders.h"

/*
// Initialization stage in UI for RAIL:
// 1) create a sequence of rail_notify_client_sysparam_update()
//    calls with all current client system parameters
//
// 2) if Language Bar capability enabled - call updating Language Bar PDU
//
// 3) prepare and call rail_client_execute(exe_or_file, working_dir, args)
//
*/

//------------------------------------------------------------------------------

/*
Flow of init stage over channel;

   Client notify UI about session start
   and go to RAIL_ESTABLISHING state.

   Client wait for Server Handshake PDU
   	   	   	   	   	   	   	   	   	   	   	   	   Server send Handshake request
   Client check Handshake response.
   If NOT OK - exit with specified reason
   Client send Handshake response
   	   	   	   	   	   	   	   	   	   	   	   	   Server send Server System
   	   	   	   	   	   	   	   	   	   	   	   	   Parameters Update (in paralel)
   Client send Client Information
   Client send Client System Parameters Update
   Client send Client Execute
   Server send Server Execute Result
   Client check Server Execute Result. If NOT OK - exit with specified reason

   Client notify UI about success session establishing and go to
   RAIL_ESTABLISHED state.
*/

//------------------------------------------------------------------------------
void init_rail_string(RAIL_STRING * rail_string, const char * string)
{
	rail_string->buffer = (uint8*)string;
	rail_string->length = strlen(string) + 1;
}
//------------------------------------------------------------------------------
void rail_string2unicode_string(
	RAIL_SESSION* session,
	RAIL_STRING* string,
	RAIL_UNICODE_STRING* unicode_string
	)
{
	size_t   result_length = 0;
	char*    result_buffer = NULL;

	result_buffer = freerdp_uniconv_out(session->uniconv, (char*)string->buffer,
			&result_length);

	unicode_string->buffer = (uint8*)result_buffer;
	unicode_string->length = (uint16)result_length;
}
//------------------------------------------------------------------------------
void rail_unicode_string2string(
	RAIL_SESSION* session,
	RAIL_UNICODE_STRING* unicode_string,
	RAIL_STRING* string
	)
{
	char*    result_buffer = NULL;

	result_buffer = freerdp_uniconv_in(session->uniconv, unicode_string->buffer,
			unicode_string->length);

	string->buffer = (uint8*)result_buffer;
	string->length = unicode_string->length;
}
//------------------------------------------------------------------------------
RAIL_SESSION *
rail_core_session_new(
	RAIL_VCHANNEL_DATA_SENDER *data_sender,
	RAIL_VCHANNEL_EVENT_SENDER *event_sender
	)
{
	RAIL_SESSION * self;

	self = (RAIL_SESSION *) xmalloc(sizeof(RAIL_SESSION));
	if (self != NULL)
	{
		memset(self, 0, sizeof(RAIL_SESSION));
		self->data_sender = data_sender;
		self->event_sender = event_sender;
		self->uniconv = freerdp_uniconv_new();
	}
	return self;
}
//------------------------------------------------------------------------------
void
rail_core_session_free(RAIL_SESSION * rail_session)
{
	if (rail_session != NULL)
	{
		freerdp_uniconv_free(rail_session->uniconv);
		xfree(rail_session);
	}
}
//------------------------------------------------------------------------------
void
rail_core_on_channel_connected(RAIL_SESSION* session)
{
	DEBUG_RAIL("rail_on_channel_connected() called.");
}
//------------------------------------------------------------------------------
void
rail_core_on_channel_terminated(RAIL_SESSION* session)
{
	DEBUG_RAIL("rail_on_channel_terminated() called.");
}
//------------------------------------------------------------------------------
void
rail_core_handle_ui_update_client_system_param(
		RAIL_SESSION* session,
		RAIL_CLIENT_SYSPARAM * sysparam
		)
{
	rail_send_vchannel_client_sysparam_update_order(session, sysparam);
}
//------------------------------------------------------------------------------
void
rail_core_ui_handle_client_execute(
	RAIL_SESSION* session,
	boolean exec_or_file_is_file_path,
	const char* rail_exe_or_file,
	const char* rail_working_directory,
	const char* rail_arguments
	)
{
	RAIL_STRING exe_or_file_;
	RAIL_STRING working_directory_;
	RAIL_STRING arguments_;
	RAIL_UNICODE_STRING exe_or_file;
	RAIL_UNICODE_STRING working_directory;
	RAIL_UNICODE_STRING arguments;
	uint16 flags;

	init_rail_string(&exe_or_file_, rail_exe_or_file);
	init_rail_string(&working_directory_, rail_working_directory);
	init_rail_string(&arguments_, rail_arguments);

	rail_string2unicode_string(session, &exe_or_file_, &exe_or_file);
	rail_string2unicode_string(session, &working_directory_, &working_directory);
	rail_string2unicode_string(session, &arguments_, &arguments);

	flags = (RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY |
			RAIL_EXEC_FLAG_EXPAND_ARGUMENTS);

	if (exec_or_file_is_file_path)
	{
		flags |= (RAIL_EXEC_FLAG_TRANSLATE_FILES | RAIL_EXEC_FLAG_FILE);
	}

	rail_send_vchannel_exec_order(session, flags, &exe_or_file,
		&working_directory,	&arguments);

	free_rail_unicode_string(&exe_or_file);
	free_rail_unicode_string(&working_directory);
	free_rail_unicode_string(&arguments);
}
//------------------------------------------------------------------------------
static void
notify_ui_to_start_initialization_stage(
	RAIL_SESSION* session
	)
{
	// TODO: Send event to UI from vchannel about start UI initialization stage.
}
//------------------------------------------------------------------------------
void
rail_core_handle_server_hadshake(
	RAIL_SESSION* session,
	uint32 build_number
	)
{
	uint32 client_build_number = 0x00001db0;

	DEBUG_RAIL("rail_handle_server_hadshake: buildNumber=0x%X.", build_number);

	// Step 1. Send Handshake PDU (2.2.2.2.1)
	// Fixed: MS-RDPERP 1.3.2.1 is not correct!
	rail_send_vchannel_handshake_order(session, client_build_number);

	// Step 2. Send Client Information PDU (2.2.2.2.1)
	rail_send_vchannel_client_information_order(session,
		RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE);

	// Step 3. Notify UI about requirements to
	//         start UI initialization stage.

	//	session->ui_listener.ui_on_initial_client_sysparams_update(
	//		session->ui_listener.ui_listener_object);

	// Step 4. Send Client Execute
	// FIXME:
	// According to "3.1.1.1 Server State Machine" Client Execute
	// will be processed after Destop Sync processed.
	// So maybe send after receive Destop Sync sequence?
	//rail_send_client_execute(session);
}
//------------------------------------------------------------------------------
void
rail_core_handle_exec_result(
	RAIL_SESSION* session,
	uint16 flags,
	uint16 exec_result,
	uint32 raw_result,
	RAIL_UNICODE_STRING * exe_or_file
	)
{
	DEBUG_RAIL("rail_handle_exec_result: flags=0x%X exec_result=0x%X"
			" raw_result=0x%X",	flags, exec_result, raw_result);

	// TODO: Send event to UI from vchannel about exec result
}
//------------------------------------------------------------------------------
void
rail_core_handle_server_sysparam(
	RAIL_SESSION* session,
	RAIL_SERVER_SYSPARAM * sysparam
	)
{
	DEBUG_RAIL("rail_handle_server_sysparam: type=0x%X scr_enabled=%d"
			" scr_lock_enabled=%d",	sysparam->type,
			sysparam->value.screen_saver_enabled,
			sysparam->value.screen_saver_lock_enabled);

	// TODO: Send event to UI from vchannel about server param update
}
//------------------------------------------------------------------------------
void
rail_core_handle_server_movesize(
	RAIL_SESSION* session,
	uint32 window_id,
	uint16 move_size_started,
	uint16 move_size_type,
	uint16 pos_x,
	uint16 pos_y
    )
{
	// TODO: Send event to UI from vchannel about server movesize
}
//------------------------------------------------------------------------------
void
rail_core_handle_server_minmax_info(
	RAIL_SESSION* session,
	uint32 window_id,
	uint16 max_width, uint16 max_height,
	uint16 max_pos_x, uint16 max_pos_y,
	uint16 min_track_width, uint16 min_track_height,
	uint16 max_track_width,	uint16 max_track_height
    )
{
	// TODO: Send event to UI from vchannel about server minmax
}
//------------------------------------------------------------------------------
void
rail_core_handle_server_langbar_info(
		RAIL_SESSION* session,
		uint32 langbar_status
		)
{
	// TODO: Send event to UI from vchannel about server langbar info
}
//------------------------------------------------------------------------------
void
rail_core_handle_server_get_app_resp(
		RAIL_SESSION* session,
		uint32 window_id,
		RAIL_UNICODE_STRING * app_id
		)
{
	// TODO: Send event to UI from vchannel about server get app response
}
//------------------------------------------------------------------------------

