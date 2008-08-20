/*
 * mailmsg_windows.cpp - send e-mails programmaticaly. On Windows.
 * Copyright (C) 2008  Michail Pishchagin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// based on code taken from MailMsg.cpp by Michael Carruth
// http://code.google.com/p/crashrpt/

#include <windows.h>
#include <mapi.h>

#include "mailmsg_windows.h"

#include <QStringList>
#include <QLibrary>
#include <QSettings>

LPSTR qStringToLPSTR(const QString& qString)
{
	// return qString.toLocal8Bit().data();
	return qString.toLatin1().data();
}

QString mapi32path()
{
	// // try to determine default Simple MAPI handler
	// // first, look for current user's default client
	// registry::tstring default_client =  registry::const_key(HKEY_CURRENT_USER, "Software\\Clients\\Mail")[""].reg_sz("");
	//
	// if (default_client.empty()) {
	// 	// then look for machine-wide settings
	// 	default_client =  registry::const_key(HKEY_LOCAL_MACHINE, "Software\\Clients\\Mail")[""].reg_sz("");
	// }
	//
	// if (!default_client.empty()) {
	// 	registry::const_key regClient(HKEY_LOCAL_MACHINE, registry::tstring("Software\\Clients\\Mail\\" + default_client).c_str());
	// 	registry::tstring s = regClient["DLLPath"].reg_sz("");
	//
	// 	if (s.empty())
	// 		s = regClient["DLLPathEx"].reg_sz("");
	//
	// 	if (!s.empty())
	// 		dllpath = s;
	// }

	return "mapi32";
}

void MailMsg::sendEmail(const QString& to, const QString& subject, const QString& message, const QStringList& attachments)
{
	qWarning("sending off email...");
	QLibrary mapi32(mapi32path());
	if (!mapi32.load()) {
		qWarning("MailMsg: failed to load mapi32.dll");
		return;
	}

	LPMAPISENDMAIL mapiSendMail = 0;
	mapiSendMail = (LPMAPISENDMAIL)mapi32.resolve("MAPISendMail");
	if (!mapiSendMail) {
		qWarning("MailMsg: failed to resolve MAPISendMail");
		return;
	}

	MapiRecipDesc* pRecipients = NULL;
	MapiFileDesc*  pAttachments = NULL;
	MapiMessage    msg;

	pRecipients = new MapiRecipDesc[1];
	pRecipients[0].ulReserved   = 0;
	pRecipients[0].ulRecipClass = MAPI_TO;
	pRecipients[0].lpszAddress  = qStringToLPSTR(to);
	pRecipients[0].lpszName     = qStringToLPSTR(to);
	pRecipients[0].ulEIDSize    = 0;
	pRecipients[0].lpEntryID    = NULL;

	if (attachments.size())
		pAttachments = new MapiFileDesc[attachments.size()];
	for (int i = 0; i < attachments.size(); ++i) {
		pAttachments[i].ulReserved   = 0;
		pAttachments[i].flFlags      = 0;
		pAttachments[i].nPosition    = 0xFFFFFFFF;
		pAttachments[i].lpszPathName = qStringToLPSTR(attachments[i]);
		pAttachments[i].lpszFileName = qStringToLPSTR(attachments[i]);
		pAttachments[i].lpFileType   = NULL;
	}

	msg.ulReserved         = 0;
	msg.lpszSubject        = qStringToLPSTR(subject);
	msg.lpszNoteText       = qStringToLPSTR(message);
	msg.lpszMessageType    = NULL;
	msg.lpszDateReceived   = NULL;
	msg.lpszConversationID = NULL;
	msg.flFlags            = 0;
	msg.lpOriginator       = NULL;
	msg.nRecipCount        = 1;
	msg.lpRecips           = pRecipients;
	msg.nFileCount         = attachments.size();
	msg.lpFiles            = pAttachments;

	int status = mapiSendMail(0, 0, &msg, MAPI_DIALOG | MAPI_LOGON_UI | MAPI_NEW_SESSION, 0);
	Q_UNUSED(status);

	if (pRecipients)
		delete[] pRecipients;

	if (pAttachments)
		delete[] pAttachments;
}
