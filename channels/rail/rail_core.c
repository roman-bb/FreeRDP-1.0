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

#define LOG_LEVEL 11
#define LLOG(_level, _args) \
  do { if (_level < LOG_LEVEL) { printf _args ; } } while (0)
#define LLOGLN(_level, _args) \
  do { if (_level < LOG_LEVEL) { printf _args ; printf("\n"); } } while (0)

//------------------------------------------------------------------------------
/*
Flow of init stage over channel;

   Client notify UI about session start and go to RAIL_ESTABLISHING state.

   Client send Handshake request
   Server send Handshake response
   Client check Handshake response. If NOT OK - exit with specified reason

   Server send Server System Parameters Update
   Client send Client Information
   Client send Client System Parameters Update
   Client send Client Execute
   Server send Server Execute Result
   Client check Server Execute Result. If NOT OK - exit with specified reason

   Client notify UI about success session establishing and go to
   RAIL_ESTABLISHED state.
*/

///////////////////////////////////////////////////////////////////////////////
void mock_ui_on_rail_handshake_request_sent(void * ui)
{
	RAIL_SESSION * session = (RAIL_SESSION *)ui;
	LLOGLN(10, ("mock_ui_on_rail_handshake_request_sent: session=0x%p",
			session));
}
//------------------------------------------------------------------------------
void mock_ui_on_rail_handshake_response_receved(void * ui)
{
	RAIL_SESSION * session = (RAIL_SESSION *)ui;
	LLOGLN(10, ("mock_ui_on_rail_handshake_response_receved: session=0x%p",
			session));
}
//------------------------------------------------------------------------------
void mock_ui_on_initial_client_sysparams_update(void * ui)
{

	RAIL_CLIENT_SYSPARAM sysparams[10];
	size_t i = 0;

	RAIL_SESSION * session = (RAIL_SESSION *)ui;
	LLOGLN(10, ("mock_ui_on_rail_handshake_response_receved: session=0x%p",
			session));

	sysparams[0].type = SPI_SETHIGHCONTRAST;
	sysparams[0].value.high_contrast_system_info.color_scheme.length = 0;
	sysparams[0].value.high_contrast_system_info.color_scheme.buffer = NULL;
	sysparams[0].value.high_contrast_system_info.flags = 0x7e;

	sysparams[1].type = RAIL_SPI_TASKBARPOS;
	sysparams[1].value.work_area.left = 0;
	sysparams[1].value.work_area.top = 0;
	sysparams[1].value.work_area.right = 1024;
	sysparams[1].value.work_area.bottom = 29;

	sysparams[2].type = SPI_SETMOUSEBUTTONSWAP;
	sysparams[2].value.left_right_mouse_buttons_swapped = 0;

	sysparams[3].type = SPI_SETKEYBOARDPREF;
	sysparams[3].value.keyboard_for_user_prefered = 0;

	sysparams[4].type = SPI_SETDRAGFULLWINDOWS;
	sysparams[4].value.full_window_drag_enabled = 1;

	sysparams[5].type = SPI_SETKEYBOARDCUES;
	sysparams[5].value.menu_access_key_always_underlined = 0;

	sysparams[6].type = SPI_SETWORKAREA;
	sysparams[6].value.work_area.left = 0;
	sysparams[6].value.work_area.top = 0;
	sysparams[6].value.work_area.right = 1024;
	sysparams[6].value.work_area.bottom = 768;


//	sysparams[5].type = RAIL_SPI_DISPLAYCHANGE;
//	sysparams[5].value.display_resolution.top = 0;
//	sysparams[5].value.display_resolution.bottom = 768;
//	sysparams[5].value.display_resolution.left = 0;
//	sysparams[5].value.display_resolution.right = 1024;

	for (i = 0; i < 7; i++)
	{
		rail_on_ui_client_system_param_updated(session, &sysparams[i]);
	}
}
//------------------------------------------------------------------------------
void mock_ui_on_rail_exec_result_receved(
	void * ui,
	uint16 exec_result,
	uint32 raw_result
	)
{
	RAIL_SESSION * session = (RAIL_SESSION *)ui;
	LLOGLN(10, ("mock_ui_on_rail_exec_result_receved:"
			" session=0x%p"
			" exec_result=0x%X"
			" raw_result=0x%X",
			session, exec_result, raw_result));

}
//------------------------------------------------------------------------------
void mock_ui_on_rail_server_sysparam_received(
	void * ui,
	RAIL_SERVER_SYSPARAM * sysparam
	)
{
	RAIL_SESSION * session = (RAIL_SESSION *)ui;
	LLOGLN(10, ("mock_ui_on_rail_server_sysparam_received: session=0x%p "
			"param_type=%d src_enabled=%d scr_lock_enabled=%d",
			session, sysparam->type, sysparam->value.screen_saver_enabled,
			sysparam->value.screen_saver_lock_enabled));

}
//------------------------------------------------------------------------------
void register_mock_ui_for_session(RAIL_SESSION* session)
{
	RAIL_UI_LISTENER s_ui = {0};
	RAIL_UI_LISTENER *ui = &s_ui;

	ui->ui_listener_object = session;

	ui->ui_on_rail_handshake_request_sent = mock_ui_on_rail_handshake_request_sent;
	ui->ui_on_rail_handshake_response_receved = mock_ui_on_rail_handshake_response_receved;
	ui->ui_on_initial_client_sysparams_update = mock_ui_on_initial_client_sysparams_update;
	ui->ui_on_rail_exec_result_receved = mock_ui_on_rail_exec_result_receved;
	ui->ui_on_rail_server_sysparam_received = mock_ui_on_rail_server_sysparam_received;

	rail_register_ui_listener(session, ui);
}
//////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
void init_rail_string(RAIL_STRING * rail_string, char * string)
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
rail_session_new(
	RAIL_VCHANNEL_SENDER *channel_sender,
	RAIL_UI_LISTENER *ui_listener
	)
{
	RAIL_SESSION * self;

	self = (RAIL_SESSION *) xmalloc(sizeof(RAIL_SESSION));
	if (self != NULL)
	{
		memset(self, 0, sizeof(RAIL_SESSION));
		self->uniconv = freerdp_uniconv_new();
		self->channel_sender = channel_sender;
		self->ui_listener = ui_listener;
		self->number_icon_caches = 2;
		self->number_icon_cache_entries = 10;
	}
	return self;
}
//------------------------------------------------------------------------------
void
rail_session_free(RAIL_SESSION * rail_session)
{
	if (rail_session != NULL)
	{
		xfree(rail_session);
	}
}
//------------------------------------------------------------------------------
void
rail_on_channel_connected(RAIL_SESSION* session)
{
	LLOGLN(10, ("rail_on_channel_connected() called."));
	uint32 build_number = 0x00001db0; // from MS doc protocol examples

	register_mock_ui_for_session(session);

	rail_send_vchannel_handshake_order(session, build_number);
}
//------------------------------------------------------------------------------
void
rail_on_channel_terminated(RAIL_SESSION* session)
{
	LLOGLN(10, ("rail_on_channel_terminated() called."));
}
//------------------------------------------------------------------------------
void
rail_on_ui_client_system_param_updated(
		RAIL_SESSION* session,
		RAIL_CLIENT_SYSPARAM * sysparam
		)
{
	rail_send_vchannel_client_sysparam_update_order(session, sysparam);
}
//------------------------------------------------------------------------------
void rail_send_client_execute(
	RAIL_SESSION* session
	)
{
	RAIL_STRING exe_or_file_;
	RAIL_STRING working_directory_;
	RAIL_STRING arguments_;
	RAIL_UNICODE_STRING exe_or_file;
	RAIL_UNICODE_STRING working_directory;
	RAIL_UNICODE_STRING arguments;
	uint16 flags;

	init_rail_string(&exe_or_file_, session->rail_exe_or_file);
	init_rail_string(&working_directory_, session->rail_working_directory);
	init_rail_string(&arguments_, session->rail_arguments);

	rail_string2unicode_string(session, &exe_or_file_, &exe_or_file);
	rail_string2unicode_string(session, &working_directory_, &working_directory);
	rail_string2unicode_string(session, &arguments_, &arguments);

	flags = RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY | RAIL_EXEC_FLAG_EXPAND_ARGUMENTS;

	rail_send_vchannel_exec_order(session, flags, &exe_or_file,
			&working_directory,	&arguments);

	free_rail_unicode_string(&exe_or_file);
	free_rail_unicode_string(&working_directory);
	free_rail_unicode_string(&arguments);
}
//------------------------------------------------------------------------------
void
rail_handle_server_hadshake(
	RAIL_SESSION* session,
	uint32 build_number
	)
{
	LLOGLN(10, ("rail_handle_server_hadshake: buildNumber=0x%X.", build_number));

	// Step 1. Send Client Information Order
	rail_send_vchannel_client_information_order(session,
		RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE
		);

	// Step 2. Send Client System Parameters Update with all initial parameters
	session->ui_listener->ui_on_initial_client_sysparams_update(
		session->ui_listener->ui_listener_object);

	// Step 3. Send Client Execute
	// FIXME:
	// According to "3.1.1.1 Server State Machine" Client Execute
	// will be processed after Destop Sync processed.
	// So maybe send after receive Destop Sync sequence?
	rail_send_client_execute(session);
}
//------------------------------------------------------------------------------
void
rail_handle_exec_result(
	RAIL_SESSION* session,
	uint16 flags,
	uint16 exec_result,
	uint32 raw_result,
	RAIL_UNICODE_STRING * exe_or_file
	)
{
	LLOGLN(10, ("rail_handle_exec_result: flags=0x%X exec_result=0x%X"
			" raw_result=0x%X",	flags, exec_result, raw_result));

	session->ui_listener->ui_on_rail_exec_result_receved(
			session->ui_listener->ui_listener_object, exec_result, raw_result);

}
//------------------------------------------------------------------------------
void
rail_handle_server_sysparam(
	RAIL_SESSION* session,
	RAIL_SERVER_SYSPARAM * sysparam
	)
{
	LLOGLN(10, ("rail_handle_server_sysparam: type=0x%X scr_enabled=%d"
			" scr_lock_enabled=%d",	sysparam->type,
			sysparam->value.screen_saver_enabled,
			sysparam->value.screen_saver_lock_enabled));

	session->ui_listener->ui_on_rail_server_sysparam_received(
			session->ui_listener->ui_listener_object, sysparam);
}
//------------------------------------------------------------------------------
void
rail_handle_server_movesize(
	RAIL_SESSION* session,
	uint32 window_id,
	uint16 move_size_started,
	uint16 move_size_type,
	uint16 pos_x,
	uint16 pos_y
    )
{
}
//------------------------------------------------------------------------------
void
rail_handle_server_minmax_info(
	RAIL_SESSION* session,
	uint32 window_id,
	uint16 max_width, uint16 max_height,
	uint16 max_pos_x, uint16 max_pos_y,
	uint16 min_track_width, uint16 min_track_height,
	uint16 max_track_width,	uint16 max_track_height
    )
{
}
//------------------------------------------------------------------------------
void
rail_handle_server_langbar_info(
		RAIL_SESSION* session,
		uint32 langbar_status
		)
{
}
//------------------------------------------------------------------------------
void
rail_handle_server_get_app_resp(
		RAIL_SESSION* session,
		uint32 window_id,
		RAIL_UNICODE_STRING * app_id
		)
{
}
//------------------------------------------------------------------------------
