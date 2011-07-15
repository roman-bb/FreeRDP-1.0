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

#include "test_rail.h"

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

	/*
	add_test_function(rail_unicode);
	add_test_function(rail_rect16);
	add_test_function(cached_icon);

	add_test_function(test_rail_plugin);
	*/

	return 0;
}

static int test_rail_on_client_send_channel_data_to_server(
		rdpInst* inst, int chan_id, char* data, int data_size)
{
	/*
	 * In there we must to check test hex PDU blobs according to receive number.
	 * */
	printf("chan_id %d data_size %d\n", chan_id, data_size);
}


void test_rail_plugin(void)
{
	rdpChanMan* chan_man;
	rdpSettings settings = { 0 };
	rdpInst inst = { 0 };
	FRDP_EVENT* event;
	int i;

	settings.hostname = "testhost";
	inst.settings = &settings;
	inst.rdp_channel_data = test_rail_on_client_send_channel_data_to_server;

	chan_man = freerdp_chanman_new();

	freerdp_chanman_load_plugin(chan_man, &settings, "../channels/rail/rail.so", NULL);
	freerdp_chanman_pre_connect(chan_man, &inst);
	freerdp_chanman_post_connect(chan_man, &inst);

	freerdp_chanman_close(chan_man, &inst);
	freerdp_chanman_free(chan_man);
}
