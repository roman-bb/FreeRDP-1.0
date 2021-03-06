/**
 * FreeRDP: A Remote Desktop Protocol Client
 * RDP Client Info
 *
 * Copyright 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
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

#include "info.h"

#define INFO_TYPE_LOGON			0x00000000
#define INFO_TYPE_LOGON_LONG		0x00000001
#define INFO_TYPE_LOGON_PLAIN_NOTIFY	0x00000002
#define INFO_TYPE_LOGON_EXTENDED_INF	0x00000003

uint8 INFO_TYPE_LOGON_STRINGS[][32] =
{
	"Logon Info V1",
	"Logon Info V2",
	"Logon Plain Notify",
	"Logon Extended Info"
};

/**
 * Write SYSTEM_TIME structure (TS_SYSTEMTIME).\n
 * @msdn{cc240478}
 * @param s stream
 * @param system_time system time structure
 */

void rdp_write_system_time(STREAM* s, SYSTEM_TIME* system_time)
{
	stream_write_uint16(s, system_time->wYear); /* wYear, must be set to 0 */
	stream_write_uint16(s, system_time->wMonth); /* wMonth */
	stream_write_uint16(s, system_time->wDayOfWeek); /* wDayOfWeek */
	stream_write_uint16(s, system_time->wDay); /* wDay */
	stream_write_uint16(s, system_time->wHour); /* wHour */
	stream_write_uint16(s, system_time->wMinute); /* wMinute */
	stream_write_uint16(s, system_time->wSecond); /* wSecond */
	stream_write_uint16(s, system_time->wMilliseconds); /* wMilliseconds */
}

/**
 * Get client time zone information.\n
 * @param s stream
 * @param settings settings
 */

void rdp_get_client_time_zone(STREAM* s, rdpSettings* settings)
{
	time_t t;
	struct tm* local_time;
	TIME_ZONE_INFORMATION* clientTimeZone;

	time(&t);
	local_time = localtime(&t);
	clientTimeZone = &settings->client_time_zone;

#if defined(sun)
	if(local_time->tm_isdst > 0)
		clientTimeZone->bias = (uint32) (altzone / 3600);
	else
		clientTimeZone->bias = (uint32) (timezone / 3600);
#elif defined(HAVE_TM_GMTOFF)
	if(local_time->tm_gmtoff >= 0)
		clientTimeZone->bias = (uint32) (local_time->tm_gmtoff / 60);
	else
		clientTimeZone->bias = (uint32) ((-1 * local_time->tm_gmtoff) / 60 + 720);
#else
	clientTimeZone->bias = 0;
#endif

	if(local_time->tm_isdst > 0)
	{
		clientTimeZone->standardBias = clientTimeZone->bias - 60;
		clientTimeZone->daylightBias = clientTimeZone->bias;
	}
	else
	{
		clientTimeZone->standardBias = clientTimeZone->bias;
		clientTimeZone->daylightBias = clientTimeZone->bias + 60;
	}

	strftime(clientTimeZone->standardName, 32, "%Z, Standard Time", local_time);
	clientTimeZone->standardName[31] = 0;
	strftime(clientTimeZone->daylightName, 32, "%Z, Summer Time", local_time);
	clientTimeZone->daylightName[31] = 0;
}

/**
 * Write client time zone information (TS_TIME_ZONE_INFORMATION).\n
 * @msdn{cc240477}
 * @param s stream
 * @param settings settings
 */

void rdp_write_client_time_zone(STREAM* s, rdpSettings* settings)
{
	size_t length;
	uint8* standardName;
	uint8* daylightName;
	size_t standardNameLength;
	size_t daylightNameLength;
	TIME_ZONE_INFORMATION* clientTimeZone;

	rdp_get_client_time_zone(s, settings);
	clientTimeZone = &settings->client_time_zone;

	standardName = (uint8*)freerdp_uniconv_out(settings->uniconv, clientTimeZone->standardName, &length);
	standardNameLength = length;

	daylightName = (uint8*)freerdp_uniconv_out(settings->uniconv, clientTimeZone->daylightName, &length);
	daylightNameLength = length;

	if (standardNameLength > 62)
		standardNameLength = 62;

	if (daylightNameLength > 62)
		daylightNameLength = 62;

	stream_write_uint32(s, clientTimeZone->bias); /* Bias */

	/* standardName (64 bytes) */
	stream_write(s, standardName, standardNameLength);
	stream_write_zero(s, 64 - standardNameLength);

	rdp_write_system_time(s, &clientTimeZone->standardDate); /* StandardDate */
	stream_write_uint32(s, clientTimeZone->standardBias); /* StandardBias */

	/* daylightName (64 bytes) */
	stream_write(s, daylightName, daylightNameLength);
	stream_write_zero(s, 64 - daylightNameLength);

	rdp_write_system_time(s, &clientTimeZone->daylightDate); /* DaylightDate */
	stream_write_uint32(s, clientTimeZone->daylightBias); /* DaylightBias */

	xfree(standardName);
	xfree(daylightName);
}

/**
 * Read Server Auto Reconnect Cookie (ARC_SC_PRIVATE_PACKET).\n
 * @msdn{cc240540}
 * @param s stream
 * @param settings settings
 */

void rdp_read_server_auto_reconnect_cookie(STREAM* s, rdpSettings* settings)
{
	ARC_SC_PRIVATE_PACKET* autoReconnectCookie;
	autoReconnectCookie = &settings->server_auto_reconnect_cookie;

	stream_read_uint32(s, autoReconnectCookie->cbLen); /* cbLen (4 bytes) */
	stream_read_uint32(s, autoReconnectCookie->version); /* version (4 bytes) */
	stream_read_uint32(s, autoReconnectCookie->logonId); /* LogonId (4 bytes) */
	stream_read(s, autoReconnectCookie->arcRandomBits, 16); /* arcRandomBits (16 bytes) */
}

/**
 * Write Client Auto Reconnect Cookie (ARC_CS_PRIVATE_PACKET).\n
 * @msdn{cc240541}
 * @param s stream
 * @param settings settings
 */

void rdp_write_client_auto_reconnect_cookie(STREAM* s, rdpSettings* settings)
{
	ARC_CS_PRIVATE_PACKET* autoReconnectCookie;
	autoReconnectCookie = &settings->client_auto_reconnect_cookie;

	stream_write_uint32(s, autoReconnectCookie->cbLen); /* cbLen (4 bytes) */
	stream_write_uint32(s, autoReconnectCookie->version); /* version (4 bytes) */
	stream_write_uint32(s, autoReconnectCookie->logonId); /* LogonId (4 bytes) */
	stream_write(s, autoReconnectCookie->securityVerifier, 16); /* SecurityVerifier */
}

/**
 * Write Extended Info Packet (TS_EXTENDED_INFO_PACKET).\n
 * @msdn{cc240476}
 * @param s stream
 * @param settings settings
 */

void rdp_write_extended_info_packet(STREAM* s, rdpSettings* settings)
{
	size_t length;
	uint16 clientAddressFamily;
	uint8* clientAddress;
	uint16 cbClientAddress;
	uint8* clientDir;
	uint16 cbClientDir;
	uint16 cbAutoReconnectLen;

	clientAddressFamily = settings->ipv6 ? ADDRESS_FAMILY_INET6 : ADDRESS_FAMILY_INET;

	clientAddress = (uint8*)freerdp_uniconv_out(settings->uniconv, settings->ip_address, &length);
	cbClientAddress = length;

	clientDir = (uint8*)freerdp_uniconv_out(settings->uniconv, settings->client_dir, &length);
	cbClientDir = length;

	cbAutoReconnectLen = settings->client_auto_reconnect_cookie.cbLen;

	stream_write_uint16(s, clientAddressFamily); /* clientAddressFamily */

	stream_write_uint16(s, cbClientAddress + 2); /* cbClientAddress */

	if (cbClientAddress > 0)
		stream_write(s, clientAddress, cbClientAddress); /* clientAddress */
	stream_write_uint16(s, 0);

	stream_write_uint16(s, cbClientDir + 2); /* cbClientDir */

	if (cbClientDir > 0)
		stream_write(s, clientDir, cbClientDir); /* clientDir */
	stream_write_uint16(s, 0);

	rdp_write_client_time_zone(s, settings); /* clientTimeZone */

	stream_write_uint32(s, 0); /* clientSessionId, should be set to 0 */
	stream_write_uint32(s, settings->performance_flags); /* performanceFlags */

	stream_write_uint16(s, cbAutoReconnectLen); /* cbAutoReconnectLen */

	if (cbAutoReconnectLen > 0)
		rdp_write_client_auto_reconnect_cookie(s, settings); /* autoReconnectCookie */

	/* reserved1 (2 bytes) */
	/* reserved2 (2 bytes) */

	xfree(clientAddress);
	xfree(clientDir);
}

/**
 * Write Info Packet (TS_INFO_PACKET).\n
 * @msdn{cc240475}
 * @param s stream
 * @param settings settings
 */

void rdp_write_info_packet(STREAM* s, rdpSettings* settings)
{
	size_t length;
	uint32 flags;
	uint8* domain;
	uint16 cbDomain;
	uint8* userName;
	uint16 cbUserName;
	uint8* password;
	uint16 cbPassword;
	uint8* alternateShell;
	uint16 cbAlternateShell;
	uint8* workingDir;
	uint16 cbWorkingDir;

	flags = INFO_MOUSE |
		INFO_UNICODE |
		INFO_LOGONERRORS |
		INFO_LOGONNOTIFY |
		INFO_MAXIMIZESHELL |
		INFO_ENABLEWINDOWSKEY |
		INFO_DISABLECTRLALTDEL |
		RNS_INFO_AUDIOCAPTURE;

	if (settings->autologon)
		flags |= INFO_AUTOLOGON;

	if (settings->console_audio)
		flags |= INFO_REMOTECONSOLEAUDIO;

	if (settings->compression)
		flags |= INFO_COMPRESSION | PACKET_COMPR_TYPE_64K;

	if (settings->rail_mode_enabled)
	{
		flags |= INFO_RAIL;
	}

	domain = (uint8*)freerdp_uniconv_out(settings->uniconv, settings->domain, &length);
	cbDomain = length;

	userName = (uint8*)freerdp_uniconv_out(settings->uniconv, settings->username, &length);
	cbUserName = length;

	password = (uint8*)freerdp_uniconv_out(settings->uniconv, settings->password, &length);
	cbPassword = length;

	alternateShell = (uint8*)freerdp_uniconv_out(settings->uniconv, settings->shell, &length);
	cbAlternateShell = length;

	workingDir = (uint8*)freerdp_uniconv_out(settings->uniconv, settings->directory, &length);
	cbWorkingDir = length;

	stream_write_uint32(s, 0); /* CodePage */
	stream_write_uint32(s, flags); /* flags */

	stream_write_uint16(s, cbDomain); /* cbDomain */
	stream_write_uint16(s, cbUserName); /* cbUserName */
	stream_write_uint16(s, cbPassword); /* cbPassword */
	stream_write_uint16(s, cbAlternateShell); /* cbAlternateShell */
	stream_write_uint16(s, cbWorkingDir); /* cbWorkingDir */

	if (cbDomain > 0)
		stream_write(s, domain, cbDomain);
	stream_write_uint16(s, 0);

	if (cbUserName > 0)
		stream_write(s, userName, cbUserName);
	stream_write_uint16(s, 0);

	if (cbPassword > 0)
		stream_write(s, password, cbPassword);
	stream_write_uint16(s, 0);

	if (cbAlternateShell > 0)
		stream_write(s, alternateShell, cbAlternateShell);
	stream_write_uint16(s, 0);

	if (cbWorkingDir > 0)
		stream_write(s, workingDir, cbWorkingDir);
	stream_write_uint16(s, 0);

	xfree(domain);
	xfree(userName);
	xfree(password);
	xfree(alternateShell);
	xfree(workingDir);

	if (settings->rdp_version >= 5)
		rdp_write_extended_info_packet(s, settings); /* extraInfo */
}

/**
 * Send Client Info PDU (CLIENT_INFO_PDU).\n
 * @msdn{cc240474}
 * @param rdp RDP module
 */

void rdp_send_client_info(rdpRdp* rdp)
{
	STREAM* s;

	s = rdp_send_stream_init(rdp);

	rdp_write_security_header(s, SEC_INFO_PKT);
	rdp_write_info_packet(s, rdp->settings);

	rdp_send(rdp, s, MCS_GLOBAL_CHANNEL_ID);
}

void rdp_recv_logon_info_v1(rdpRdp* rdp, STREAM* s)
{
	uint32 cbDomain;
	uint32 cbUserName;

	stream_read_uint32(s, cbDomain); /* cbDomain (4 bytes) */
	stream_seek(s, 52); /* domain (52 bytes) */
	stream_read_uint32(s, cbUserName); /* cbUserName (4 bytes) */
	stream_seek(s, 512); /* userName (512 bytes) */
	stream_seek_uint32(s); /* sessionId (4 bytes) */
}

void rdp_recv_logon_info_v2(rdpRdp* rdp, STREAM* s)
{
	uint32 cbDomain;
	uint32 cbUserName;

	stream_seek_uint16(s); /* version (2 bytes) */
	stream_seek_uint32(s); /* size (4 bytes) */
	stream_read_uint32(s, cbDomain); /* cbDomain (4 bytes) */
	stream_read_uint32(s, cbUserName); /* cbUserName (4 bytes) */
	stream_seek(s, 558); /* pad */
	stream_seek(s, cbDomain); /* domain */
	stream_seek(s, cbUserName); /* userName */
}

void rdp_recv_logon_plain_notify(rdpRdp* rdp, STREAM* s)
{
	stream_seek(s, 576); /* pad */
}

void rdp_recv_logon_error_info(rdpRdp* rdp, STREAM* s)
{
	uint32 errorNotificationType;
	uint32 errorNotificationData;

	stream_read_uint32(s, errorNotificationType); /* errorNotificationType (4 bytes) */
	stream_read_uint32(s, errorNotificationData); /* errorNotificationData (4 bytes) */
}

void rdp_recv_logon_info_extended(rdpRdp* rdp, STREAM* s)
{
	uint32 cbFieldData;
	uint32 fieldsPresent;

	stream_read_uint32(s, fieldsPresent); /* fieldsPresent (4 bytes) */

	/* logonFields */

	if (fieldsPresent & LOGON_EX_AUTORECONNECTCOOKIE)
	{
		stream_read_uint32(s, cbFieldData); /* cbFieldData (4 bytes) */
		rdp_read_server_auto_reconnect_cookie(s, rdp->settings);
	}

	if (fieldsPresent & LOGON_EX_LOGONERRORS)
	{
		stream_read_uint32(s, cbFieldData); /* cbFieldData (4 bytes) */
		rdp_recv_logon_error_info(rdp, s);
	}

	stream_seek(s, 570); /* pad */
}

void rdp_recv_save_session_info(rdpRdp* rdp, STREAM* s)
{
	uint32 infoType;

	stream_read_uint32(s, infoType); /* infoType (4 bytes) */

	printf("%s\n", INFO_TYPE_LOGON_STRINGS[infoType]);

	switch (infoType)
	{
		case INFO_TYPE_LOGON:
			rdp_recv_logon_info_v1(rdp, s);
			break;

		case INFO_TYPE_LOGON_LONG:
			rdp_recv_logon_info_v2(rdp, s);
			break;

		case INFO_TYPE_LOGON_PLAIN_NOTIFY:
			rdp_recv_logon_plain_notify(rdp, s);
			break;

		case INFO_TYPE_LOGON_EXTENDED_INF:
			rdp_recv_logon_info_extended(rdp, s);
			break;

		default:
			break;
	}
}

