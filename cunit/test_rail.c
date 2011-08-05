/*
   FreeRDP: A Remote Desktop Protocol client.
   RAIL(TS RemoteApp) Virtual Channel Unit Tests

   Copyright 2011 Vic Lee
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
#include <string.h>
#include <stdlib.h>
#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/chanman.h>
#include <freerdp/utils/event.h>
#include <freerdp/utils/hexdump.h>
#include <freerdp/utils/memory.h>
#include <freerdp/rail.h>

#include "test_rail.h"

#define HCF_HIGHCONTRASTON  0x00000001
#define HCF_AVAILABLE       0x00000002
#define HCF_HOTKEYACTIVE    0x00000004
#define HCF_CONFIRMHOTKEY   0x00000008
#define HCF_HOTKEYSOUND     0x00000010
#define HCF_INDICATOR       0x00000020
#define HCF_HOTKEYAVAILABLE 0x00000040


int init_rail_suite(void)
{
	freerdp_chanman_global_init();
	return 0;
}

int clean_rail_suite(void)
{
	freerdp_chanman_global_uninit();
	return 0;
}

int add_rail_suite(void)
{
	add_test_suite(rail);

	add_test_function(rail_plugin);

	return 0;
}

static uint8 server_handshake[] =
{
	0x05, 0x00, 0x08, 0x00, 0xb0, 0x1d, 0x00, 0x00
};

static uint8 client_handshake[] =
{
0x05, 0x00, 0x08, 0x00, 0xb0, 0x1d, 0x00, 0x00
};

static uint8 client_info_pdu[] =
{
0x0b, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00
};

// Flags: TS_RAIL_EXEC_FLAG_EXPAND_ARGUMENTS
// ExeOrFile : ||iexplore
// WorkingDir: f:\windows\system32
// Arguments: www.bing.com

static uint8 client_execute_pdu[] =
{
0x01,0x00,0x5e,0x00,0x08,0x00,0x14,0x00,0x26,0x00,0x18,0x00,0x7c,0x00,
0x7c,0x00,0x69,0x00,0x65,0x00,0x78,0x00,0x70,0x00,0x6c,0x00,0x6f,0x00,
0x72,0x00,0x65,0x00,0x66,0x00,0x3a,0x00,0x5c,0x00,0x77,0x00,0x69,0x00,
0x6e,0x00,0x64,0x00,0x6f,0x00,0x77,0x00,0x73,0x00,0x5c,0x00,0x73,0x00,
0x79,0x00,0x73,0x00,0x74,0x00,0x65,0x00,0x6d,0x00,0x33,0x00,0x32,0x00,
0x77,0x00,0x77,0x00,0x77,0x00,0x2e,0x00,0x62,0x00,0x69,0x00,0x6e,0x00,
0x67,0x00,0x2e,0x00,0x63,0x00,0x6f,0x00,0x6d,0x00
};

static uint8 client_activate_pdu[] =
{
0x02,0x00,
0x09,0x00,
0x8e,0x00,0x07,0x00,
0x01
};


static uint8 server_exec_result_pdu[] =
{
0x80,0x00,0x24,0x00,0x08,0x00,0x03,0x00,0x15,0x00,0x00,0x00,0x00,0x00,
0x14,0x00,0x7c,0x00,0x7c,0x00,0x57,0x00,0x72,0x00,0x6f,0x00,0x6e,0x00,
0x67,0x00,0x41,0x00,0x70,0x00,0x70,0x00
};


static uint8 client_sysparam_highcontrast_pdu[] =
{
0x03,0x00,
0x12,0x00,
0x43,0x00,0x00,0x00, // SPI_SETHIGHCONTRAST
0x7e,0x00,0x00,0x00, // HCF_AVAILABLE | HCF_HOTKEYACTIVE | HCF_CONFIRMHOTKEY
                     // HCF_HOTKEYSOUND | HCF_INDICATOR | HCF_HOTKEYAVAILABLE
0x02,0x00,0x00,0x00, // Minimum length 2
0x00,0x00 // Unicode String
};


static uint8 client_sysparam_taskbarpos_pdu[] =
{
0x03,0x00,
0x10,0x00,
0x00,0xf0,0x00,0x00, // RAIL_SPI_TASKBARPOS
0x00,0x00, // 0
0x9a,0x03, // 0x039a
0x90,0x06, // 0x0690
0xc2,0x03  // 0x03c2
};

static uint8 client_sysparam_mousebuttonswap_pdu[] =
{
0x03,0x00,
0x09,0x00,
0x21,0x00,0x00,0x00, // SPI_SETMOUSEBUTTONSWAP
0x00 // false
};


static uint8 client_sysparam_keyboardpref_pdu[] =
{
0x03,0x00,
0x09,0x00,
0x45,0x00,0x00,0x00, // SPI_SETKEYBOARDPREF
0x00 // false
};


static uint8 client_sysparam_dragfullwindow_pdu[] =
{
0x03,0x00,
0x09,0x00,
0x25,0x00,0x00,0x00, // SPI_SETDRAGFULLWINDOWS
0x01 // true
};


static uint8 client_sysparam_keyboardcues_pdu[] =
{
0x03,0x00,
0x09,0x00,
0x0b,0x10,0x00,0x00, //SPI_SETKEYBOARDCUES
0x00 // false
};

static uint8 client_sysparam_setworkarea_pdu[] =
{
0x03,0x00,
0x10,0x00,
0x2f,0x00,0x00,0x00, //SPI_SETWORKAREA
0x00,0x00, // 0
0x00,0x00, // 0
0x90,0x06, // 0x0690
0x9a,0x03  // 0x039a
};

static uint8 client_syscommand_pdu[] =
{
0x04,0x00,
0x0a,0x00,
0x52,0x00,0x02,0x00,
0x20,0xf0
};

static uint8 client_notify_pdu[] =
{
0x06,0x00,
0x10,0x00,
0xaa,0x01,0x02,0x00,
0x02,0x00,0x00,0x00,
0x04,0x02,0x00,0x00
};

static uint8 client_windowmove_pdu[] =
{
0x08,0x00,
0x10,0x00,
0x20,0x00,0x02,0x00,
0x09,0x03,
0x00,0x01,
0xdb,0x05,
0x88,0x01
};

static uint8 client_system_menu_pdu[] =
{
0x0c,0x00,
0x0c,0x00,
0x22,0x01,0x09,0x00,
0xa4,0xff,
0x4a,0x02
};

static uint8 client_get_app_id_req_pdu[] =
{
0x0E,0x00,0x08,0x00,0x52,0x00,0x02,0x00
};




static uint8 server_sysparam1_pdu[] =
{
0x03,0x00,
0x09,0x00,
0x77,0x00,0x00,0x00,
0x00
};

static uint8 server_sysparam2_pdu[] =
{
0x03,0x00,
0x09,0x00,
0x11,0x00,0x00,0x00,
0x00
};

static uint8 server_localmovesize_start_pdu[] =
{
0x09,0x00,0x10,0x00,0x8e,0x00,0x07,0x00,0x01,0x00,0x09,0x00,0x7e,0x01,
0x0a,0x00
};

static uint8 server_localmovesize_stop_pdu[] =
{
0x09,0x00,0x10,0x00,0x8e,0x00,0x07,0x00,0x00,0x00,0x09,0x00,0xa6,0x00,
0x44,0x00
};

static uint8 server_minmaxinfo_pdu[] =
{
0x0a,0x00,0x18,0x00,0x8e,0x00,0x07,0x00,0x08,0x04,0xd6,0x02,0x00,0x00,
0x00,0x00,0x70,0x00,0x1b,0x00,0x0c,0x04,0x0c,0x03
};

static uint8 server_langbar_pdu[] =
{
0x0D,0x00,0x08,0x00,0x01,0x00,0x00,0x00
};

static uint8 client_app_app_req_pdu[] =
{
0x0E,0x00,0x08,0x00,0x52,0x00,0x02,0x00
};

static uint8 server_app_get_resp_pdu[] =
{
0x0F,0x00,0x08,0x20,0x52,0x00,0x02,0x00,0x6d,0x00,0x69,0x00,0x63,0x00,
0x72,0x00,0x6f,0x00,0x73,0x00,0x6f,0x00,0x66,0x00,0x74,0x00,0x2e,0x00,
0x77,0x00,0x69,0x00,0x6e,0x6f,0x00,0x77,0x00,0x73,0x00,0x2e,0x00,0x6e,
0x00,0x6f,0x00,0x74,0x00,0x65,0x00,0x70,0x00,0x61,0x00,0x64,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00
};

/*

0F.*00.*08.*20




*/


#define EMULATE_SERVER_SEND_CHANNEL_DATA(inst, byte_array) \
	emulate_server_send_channel_data(inst, byte_array, RAIL_ARRAY_SIZE(byte_array))

typedef struct
{
	rdpChanMan* chan_man;
	freerdp*    instance;
	int         th_count;
	int         th_to_finish;

} thread_param;


static void on_free_rail_ui_event(
	FRDP_EVENT* event
	)
{
	assert(event->event_type == FRDP_EVENT_TYPE_RAIL_UI_2_VCHANNEL);

	RAIL_UI_EVENT* rail_event = (RAIL_UI_EVENT*)event->user_data;

	xfree(rail_event);
}
//-----------------------------------------------------------------------------
static int init_ui_event(
	RAIL_UI_EVENT* event,
	uint32 event_id
	)
{
	memset(event, 0, sizeof(RAIL_UI_EVENT));
	event->event_id = event_id;
}
//-----------------------------------------------------------------------------
static int send_ui_event2plugin(
	rdpChanMan* chan_man,
	RAIL_UI_EVENT* event
	)
{
	RAIL_UI_EVENT* payload = NULL;
	FRDP_EVENT* out_event = NULL;

	payload = xnew(RAIL_UI_EVENT);
	memset(payload, 0, sizeof(RAIL_UI_EVENT));
	memcpy(payload, event, sizeof(RAIL_UI_EVENT));

	out_event = freerdp_event_new(FRDP_EVENT_TYPE_RAIL_UI_2_VCHANNEL,
			on_free_rail_ui_event,
			payload);

	freerdp_chanman_send_event(chan_man, "rail", out_event);
}
//-----------------------------------------------------------------------------
static void emulate_server_send_channel_data(
	freerdp* instance,
	void* data,
	size_t size
	)
{
	freerdp_chanman_data(instance, 0, (char*)data, size,
			CHANNEL_FLAG_FIRST | CHANNEL_FLAG_LAST, size);
}
//-----------------------------------------------------------------------------
static int emulate_client_send_channel_data(
	freerdp* freerdp, int channelId, uint8* data, int size
	)
{
	DEBUG_RAIL("Client send to server:");
	freerdp_hexdump(data, size);

	// add to global dumps list
}
//-----------------------------------------------------------------------------
void debug_event(FRDP_EVENT* event)
{
	uint32 rail_event_id = 0;
	if (event->event_type == FRDP_EVENT_TYPE_RAIL_VCHANNEL_2_UI)
	{
		RAIL_VCHANNEL_EVENT* rail_event = (RAIL_VCHANNEL_EVENT*)event->user_data;
		DEBUG_RAIL("RAIL event 0x%X.", rail_event->event_id);
	}
}
//-----------------------------------------------------------------------------
static void process_events_and_channel_data_from_plugin(thread_param* param)
{
	FRDP_EVENT* event;

	param->th_count++;
	while (param->th_to_finish == 0)
	{
		freerdp_chanman_check_fds(param->chan_man, param->instance);
		event = freerdp_chanman_pop_event(param->chan_man);
		if (event)
		{
			DEBUG_RAIL("UI receive %d event.", event->event_type);
			debug_event(event);

			// add to global event list
			freerdp_event_free(event);
		}

		usleep(1000);
	}
	param->th_count--;
}
//-----------------------------------------------------------------------------
void* thread_func(void* param)
{
	thread_param* th_param = (thread_param*)param;
	process_events_and_channel_data_from_plugin(th_param);
	pthread_detach(pthread_self());
}
//-----------------------------------------------------------------------------
void test_rail_plugin(void)
{
	thread_param param;
	pthread_t thread;

	rdpChanMan* chan_man;
	rdpSettings settings = { 0 };
	freerdp s_inst = { 0 };
	freerdp* inst = &s_inst;
	int i;

	RAIL_UI_EVENT event;

	settings.hostname = "testhost";
	inst->settings = &settings;
	inst->SendChannelData = emulate_client_send_channel_data;

	chan_man = freerdp_chanman_new();

	freerdp_chanman_load_plugin(chan_man, &settings, "../channels/rail/rail.so", NULL);
	freerdp_chanman_pre_connect(chan_man, inst);
	freerdp_chanman_post_connect(chan_man, inst);

	param.chan_man = chan_man;
	param.instance = inst;
	param.th_count = 0;
	param.th_to_finish = 0;

	pthread_create(&thread, 0, thread_func, &param);


	// 1. Emulate server handshake binary
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_handshake);
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_exec_result_pdu);
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_sysparam1_pdu);
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_sysparam2_pdu);
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_localmovesize_start_pdu);
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_localmovesize_stop_pdu);
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_minmaxinfo_pdu);
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_langbar_pdu);
	EMULATE_SERVER_SEND_CHANNEL_DATA(inst, server_app_get_resp_pdu);

	// 2. Send UI events

	init_ui_event(&event, RAIL_UI_EVENT_UPDATE_CLIENT_SYSPARAM);
	event.param.sysparam_info.param = SPI_SETHIGHCONTRAST;
	event.param.sysparam_info.value.high_contrast_system_info.flags = 0x7e;
	event.param.sysparam_info.value.high_contrast_system_info.color_scheme = "";
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_UPDATE_CLIENT_SYSPARAM);
	event.param.sysparam_info.param = SPI_SETWORKAREA;
	event.param.sysparam_info.value.work_area.left = 0;
	event.param.sysparam_info.value.work_area.top = 0;
	event.param.sysparam_info.value.work_area.right = 0x0690;
	event.param.sysparam_info.value.work_area.bottom = 0x039a;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_UPDATE_CLIENT_SYSPARAM);
	event.param.sysparam_info.param = RAIL_SPI_TASKBARPOS;
	event.param.sysparam_info.value.taskbar_size.left = 0;
	event.param.sysparam_info.value.taskbar_size.top = 0x039a;
	event.param.sysparam_info.value.taskbar_size.right = 0x0690;
	event.param.sysparam_info.value.taskbar_size.bottom = 0x03c2;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_UPDATE_CLIENT_SYSPARAM);
	event.param.sysparam_info.param = SPI_SETMOUSEBUTTONSWAP;
	event.param.sysparam_info.value.left_right_mouse_buttons_swapped = False;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_UPDATE_CLIENT_SYSPARAM);
	event.param.sysparam_info.param = SPI_SETKEYBOARDPREF;
	event.param.sysparam_info.value.keyboard_for_user_prefered = False;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_UPDATE_CLIENT_SYSPARAM);
	event.param.sysparam_info.param = SPI_SETDRAGFULLWINDOWS;
	event.param.sysparam_info.value.full_window_drag_enabled = True;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_UPDATE_CLIENT_SYSPARAM);
	event.param.sysparam_info.param = SPI_SETKEYBOARDCUES;
	event.param.sysparam_info.value.full_window_drag_enabled = False;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_EXECUTE_REMOTE_APP);
	event.param.execute_info.exe_or_file = "||iexplore";
	event.param.execute_info.working_directory = "f:\\windows\\system32";
	event.param.execute_info.arguments = "www.bing.com";
	event.param.execute_info.exec_or_file_is_file_path = False;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_ACTIVATE);
	event.param.activate_info.window_id = 0x0007008e;
	event.param.activate_info.enabled = True;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_SYS_COMMAND);
	event.param.syscommand_info.window_id = 0x00020052;
	event.param.syscommand_info.syscommand = 0x0f20;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_NOTIFY);
	event.param.notify_info.window_id = 0x000201aa;
	event.param.notify_info.notify_icon_id = 0x02;
	event.param.notify_info.message = 0x0204;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_WINDOW_MOVE);
	event.param.window_move_info.window_id = 0x00020020;
	event.param.window_move_info.new_position.left = 0x0309;
	event.param.window_move_info.new_position.top = 0x0100;
	event.param.window_move_info.new_position.right = 0x05db;
	event.param.window_move_info.new_position.bottom = 0x0188;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_SYSTEM_MENU);
	event.param.system_menu_info.window_id = 0x00090122;
	event.param.system_menu_info.left = 0xffa4; // TODO: WTF?
	event.param.system_menu_info.top = 0x024a;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_LANGBAR_INFO);
	event.param.langbar_info.status = 0x00000001;
	send_ui_event2plugin(chan_man, &event);

	init_ui_event(&event, RAIL_UI_EVENT_GET_APP_ID);
	event.param.get_app_id_info.window_id = 0x00020052;
	send_ui_event2plugin(chan_man, &event);

	// Waiting for possible events or data
	sleep(5);

	// Finishing thread and wait for it
	param.th_to_finish = 1;
	while (param.th_count > 0)
	{
		usleep(1000);
	}

	// We need to collected all events and data dumps and then to.
	// create CU_ASSERT series here!


	freerdp_chanman_close(chan_man, inst);
	freerdp_chanman_free(chan_man);
}


