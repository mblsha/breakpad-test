/*
 * crashreporter.cpp - simple crash reporter
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

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include <QHttpRequestHeader>
#include <QCloseEvent>

#include "ui_crashreporter.h"

#if defined(Q_WS_WIN)
#include "mailmsg_windows.h"
#endif

class CrashReporter : public QWidget
{
	Q_OBJECT
public:
	CrashReporter()
		: QWidget()
		, http_(0)
		, restartButton_(0)
		, quitButton_(0)
	{
		ui_.setupUi(this);

		appName_ = "Application";

		http_ = new QHttp(this);
		connect(http_, SIGNAL(done(bool)), SLOT(httpDone(bool)));

		ui_.textEdit->setFocus();
	}

	void doShow()
	{
		ui_.label->setText(ui_.label->text().arg(appName_));

		if (!appPath_.isEmpty()) {
			restartButton_ = ui_.buttonBox->addButton(tr("Restart %1").arg(appName_), QDialogButtonBox::AcceptRole);
			connect(restartButton_, SIGNAL(clicked()), SLOT(restartApp()));
		}

		quitButton_ = ui_.buttonBox->addButton(tr("Quit %1").arg(appName_), QDialogButtonBox::RejectRole);
		connect(quitButton_, SIGNAL(clicked()), SLOT(quitApp()));

		show();
	}

	void setAppName(const QString& appName)
	{
		appName_ = appName;
	}

	void setAppVersion(const QString& appVersion)
	{
		appVersion_ = appVersion;
	}

	void setAppPath(const QString& appPath)
	{
		appPath_ = appPath;
	}

	void setMinidump(const QString& minidump)
	{
		minidump_ = minidump;
	}

private slots:
	void restartApp()
	{
		if (!appPath_.isEmpty()) {
			QProcess::startDetached(appPath_);
		}
		close();
	}

	void quitApp()
	{
		close();
	}

	QByteArray makeFormField(const QString& name, const QByteArray& data)
	{
		QByteArray result;
		result.append("------FormBoundary5WRMUdn8jqMNiFOP\r\n");
		result.append("Content-Disposition: form-data; name=\"" + name + "\"\r\n");
		result.append("\r\n");
		result.append(data);
		result.append("\r\n");
		return result;
	}

	bool reportCrash()
	{
		QFileInfo fi(minidump_);
		if (!fi.exists())
			return false;

		QFile minidump(minidump_);
		if (!minidump.open(QIODevice::ReadOnly))
		         return false;

		if (!ui_.groupBox->isChecked() || !http_)
			return false;

		if (restartButton_)
			restartButton_->setEnabled(false);
		if (quitButton_)
			quitButton_->setEnabled(false);

		QHttpRequestHeader header("POST", "/cgi-bin/cgi-collector/collector.py");
		header.setValue("User-Agent", "crashreporter");
		header.setValue("Host", "localhost");
		header.setValue("Accept-Language", "en-us");
		header.setValue("Content-Type", "multipart/form-data; boundary=----FormBoundary5WRMUdn8jqMNiFOP");
		header.setValue("Accept", "*/*");

		QByteArray bytes;
		bytes.append(makeFormField("app-name", appName_.toUtf8()));
		bytes.append(makeFormField("app-version", appVersion_.toUtf8()));
		bytes.append(makeFormField("comments", ui_.textEdit->toPlainText().toUtf8()));
		bytes.append(makeFormField("upload_file_minidump", minidump.readAll()));

		bytes.append("------FormBoundary5WRMUdn8jqMNiFOP--");
		bytes.append("\r\n");

		int contentLength = bytes.length();
		header.setContentLength(contentLength);

		http_->setHost("localhost", 80);
		http_->request(header, bytes);

		return true;
	}

	void httpDone(bool error)
	{
		if (error) {
			qWarning("CrashReporter: ERROR: %s", qPrintable(http_->errorString()));
			sendEmail();
		}
		else {
			QString result(http_->readAll());
			QRegExp rx("CrashID\\=([\\d\\w-]+)");
			if (rx.indexIn(result) != -1) {
				qWarning("CrashReporter: CrashID=%s", qPrintable(rx.capturedTexts().at(1)));
			}
		}

		delete http_;
		http_ = 0;
		close();
	}

	void sendEmail()
	{
#if defined(Q_WS_WIN)
		QStringList attachments;
		QFileInfo fi(minidump_);
		if (fi.exists()) {
			attachments << fi.absoluteFilePath();
		}

		MailMsg::sendEmail("mblsha@domain.org",
		                   "Crash report",
		                   ui_.textEdit->toPlainText(),
		                   attachments);
#endif
	}

protected:
	// reimplemented
	void closeEvent(QCloseEvent* e)
	{
		if (reportCrash()) {
			e->ignore();
			return;
		}
		QWidget::closeEvent(e);
	}

private:
	Ui::CrashReporter ui_;
	QHttp* http_;
	QPushButton* restartButton_;
	QPushButton* quitButton_;
	QString appName_;
	QString appVersion_;
	QString appPath_;
	QString minidump_;
};

static QString optionValue(const QString& optionName, const QString string)
{
	if (string.startsWith(QString("-%1=").arg(optionName))) {
		QStringList list = string.split("=");
		list.removeFirst();
		if (!list.isEmpty()) {
			return list.join("=");
		}
	}

	return QString();
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	CrashReporter crashReporter;

	for (int n = 1; n < argc; ++n) {
		QString str = argv[n];
		if (!optionValue("appName", str).isEmpty()) {
			crashReporter.setAppName(optionValue("appName", str));
		}

		if (!optionValue("appVersion", str).isEmpty()) {
			crashReporter.setAppVersion(optionValue("appVersion", str));
		}

		if (!optionValue("appPath", str).isEmpty()) {
			crashReporter.setAppPath(optionValue("appPath", str));
		}

		if (!optionValue("minidump", str).isEmpty()) {
			crashReporter.setMinidump(optionValue("minidump", str));
		}
	}

	crashReporter.doShow();
	return app.exec();
}

#include "crashreporter.moc"
