/*
 * breakpad.cpp - google-breakpad wrapper for Qt applications
 * Copyright (C) 2008  Michail Pishchagin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "breakpad.h"

#include <qglobal.h>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

#if defined(Q_WS_MAC)
#include "client/mac/handler/exception_handler.h"
#else if defined(Q_WS_WIN)
#include "client/windows/handler/exception_handler.h"
#include "mailmsg/mailmsg_windows.h"
#endif

using google_breakpad::ExceptionHandler;

namespace Breakpad {

static QString app_version;

#if defined(Q_WS_MAC)
bool MDCallback(const char* _dump_dir,
                const char* _minidump_id,
                void *context, bool success)
#else if defined(Q_WS_WIN)
bool MDCallback(const wchar_t* _dump_dir,
                const wchar_t* _minidump_id,
                void* context,
                EXCEPTION_POINTERS* exinfo,
                MDRawAssertionInfo* assertion,
                bool success)
#endif
{
#if defined(Q_WS_MAC)
	QString dump_dir = QString::fromUtf8(_dump_dir);
	QString minidump_id = QString::fromUtf8(_minidump_id);
#else if defined(Q_WS_WIN)
	Q_UNUSED(assertion);
	Q_UNUSED(exinfo);
	Q_UNUSED(context);
	QString dump_dir = QString::fromWCharArray(_dump_dir);
	QString minidump_id = QString::fromWCharArray(_minidump_id);
#endif

	QString minidumpPath = QString("%1/%2.dmp").arg(dump_dir).arg(minidump_id);
	if (!success) {
		qWarning("Error writing minidump: %s", qPrintable(minidumpPath));
		return true;
	}

#if defined(Q_WS_MAC)
	QString os = "darwin";
#else if defined(Q_WS_WIN)
	QString os = "windows";
#endif

	QFile minidumpFile(minidumpPath);
	QString newName = QString("%1/%2_%3_%4.dmp")
	                  .arg(dump_dir)
	                  .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hhmmss"))
	                  .arg(app_version)
	                  .arg(os);
	if (minidumpFile.rename(newName)) {
		minidumpPath = newName;
	}

	QFileInfo fi(minidumpPath);
	qWarning("Minidump: %s", qPrintable(fi.absoluteFilePath()));

#if defined(Q_WS_WIN)
	QStringList attachments;
	attachments << fi.absoluteFilePath();
	MailMsg::sendEmail("mblsha@domain.org",
	                   "Crash report",
	                   "",
	                   attachments);
#endif

	return true;
}

static ExceptionHandler* handler = 0;

void install(const QString& minidumpPath, const QString& version)
{
	char* p = getenv("DISABLE_BREAKPAD");
	if (p || handler) {
		return;
	}

	app_version = version;

	handler = new ExceptionHandler(
#if defined(Q_WS_MAC)
	    minidumpPath.toUtf8().data(),
#else if defined(Q_WS_WIN)
	    minidumpPath.toStdWString(),
#endif
	    0, MDCallback, 0, true);
}

} // namespace Breakpad
