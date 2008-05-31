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

	Breakpad::install(app.applicationDirPath(), "psi-1.0.0");

	recursion(10);

	// return app.exec();
}
