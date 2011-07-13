/**
 * FreeRDP: A Remote Desktop Protocol client.
 * RAIL Virtual Channel
 *
 * Copyright 2010-2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 * Copyright 2011 Vic Lee
 * Copyright 2011 Roman Bararanov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freerdp/constants.h>
#include <freerdp/types.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/svc_plugin.h>

#include "rail_core.h"

typedef struct rail_plugin railPlugin;
struct rail_plugin
{
	rdpSvcPlugin plugin;

	RAIL_SESSION * session;
};

static void rail_plugin_process_connect(rdpSvcPlugin* plugin)
{
	printf("rail_plugin_process_connect\n");
}

static void 
rail_plugin_send_vchannel_data(
	void* rail_plugin_object,
	void* data,
	size_t length
	)
{
	railPlugin* plugin = (railPlugin*)rail_plugin_object;
	STREAM* s = NULL;

	s = stream_new(length);
	stream_write(s, data, length);
	svc_plugin_send((rdpSvcPlugin*)plugin, s);
}

static void rail_plugin_process_receive(rdpSvcPlugin* plugin, STREAM* data_in)
{
	STREAM* data_out;

	printf("rail_plugin_process_receive: size %d\n", stream_get_size(data_in));
	stream_free(data_in);

	data_out = stream_new(8);
	stream_write(data_out, "senddata", 8);
	svc_plugin_send(plugin, data_out);
}

static void rail_plugin_process_event(rdpSvcPlugin* plugin, FRDP_EVENT* event)
{
	printf("rail_plugin_process_event: event_type %d\n", event->event_type);
	freerdp_event_free(event);

	event = freerdp_event_new(FRDP_EVENT_TYPE_DEBUG, NULL, NULL);
	svc_plugin_send_event(plugin, event);
}

static void rail_plugin_process_terminate(rdpSvcPlugin* plugin)
{
	printf("rail_plugin_process_terminate\n");
	xfree(plugin);
}

int VirtualChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints)
{
	railPlugin* rail;

	rail = (railPlugin*)xmalloc(sizeof(railPlugin));
	memset(rail, 0, sizeof(railPlugin));

	rail->plugin.channel_def.options = CHANNEL_OPTION_INITIALIZED |
		CHANNEL_OPTION_ENCRYPT_RDP | CHANNEL_OPTION_COMPRESS_RDP |
		CHANNEL_OPTION_SHOW_PROTOCOL;
	strcpy(rail->plugin.channel_def.name, "rail");

	rail->plugin.connect_callback = rail_plugin_process_connect;
	rail->plugin.receive_callback = rail_plugin_process_receive;
	rail->plugin.event_callback = rail_plugin_process_event;
	rail->plugin.terminate_callback = rail_plugin_process_terminate;

	svc_plugin_init((rdpSvcPlugin*)rail, pEntryPoints);

	return 1;
}
