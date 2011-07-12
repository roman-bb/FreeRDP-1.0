/**
 * FreeRDP: A Remote Desktop Protocol Client
 * FreeRDP Test UI
 *
 * Copyright 2010 Marc-Andre Moreau <marcandre.moreau@gmail.com>
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
#include <string.h>

#include "connection.h"

#include <freerdp/settings.h>
#include <freerdp/utils/memory.h>

int main(int argc, char* argv[])
{
	rdpRdp* rdp;
	rdpSettings* settings;

	rdp = rdp_new();
	settings = rdp->settings;

	if (argc < 4)
	{
		printf("Usage: freerdp-test <hostname> <username>\n");
		return 0;
	}

	settings->hostname = (uint8*) xmalloc(strlen(argv[1]));
	memcpy(settings->hostname, argv[1], strlen(argv[1]));
	settings->hostname[strlen(argv[1])] = '\0';

	settings->username = (uint8*) xmalloc(strlen(argv[2]));
	memcpy(settings->username, argv[2], strlen(argv[2]));
	settings->username[strlen(argv[2])] = '\0';

	settings->password = (uint8*) xmalloc(strlen(argv[3]));
	memcpy(settings->password, argv[3], strlen(argv[3]));
	settings->password[strlen(argv[3])] = '\0';

	printf("hostname: %s username: %s password: %s\n",
			settings->hostname, settings->username, settings->password);

	rdp_client_connect(rdp);

	return 0;
}
