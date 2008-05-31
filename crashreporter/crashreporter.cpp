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
		, restartButton_(0)
		, quitButton_(0)
	{
		ui_.setupUi(this);

		appName_ = "Application";

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
		reportCrash();
		if (!appPath_.isEmpty()) {
			QProcess::startDetached(appPath_);
		}
		close();
	}

	void quitApp()
	{
		reportCrash();
		close();
	}

	void reportCrash()
	{
		if (!ui_.groupBox->isChecked())
			return;

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
		reportCrash();
		QWidget::closeEvent(e);
	}

private:
	Ui::CrashReporter ui_;
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
