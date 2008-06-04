#include <QApplication>

#include "breakpad.h"

void recursion(int n)
{
	if (!n) {
		int* a = 0;
		qWarning("A = %d", *a);
	}
	recursion(n - 1);
}

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);

	QMap<QString, QString> p;
	p["name"]         = "Zysch";
	p["version"]      = "1.0.1";
	p["report-url"]   = "http://localhost/cgi-bin/cgi-collector/collector.py";
	p["report-email"] = "mblsha@domain.org";
	Breakpad::install(app.applicationDirPath(), p);

	recursion(10);

	// return app.exec();
}
