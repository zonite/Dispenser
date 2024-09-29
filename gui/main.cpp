//#include "mainwindow.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
//#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <dispenser.h>

#include "manager.h"
#include "unitmodel.h"
#include "unitlist.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setOrganizationName(ORGANIZATION);
	QCoreApplication::setOrganizationDomain(DOMAIN);
	QCoreApplication::setApplicationName(APPNAME);
	QGuiApplication a(argc, argv);
	//QApplication a(argc, argv);

	QTranslator translator;
	const QStringList uiLanguages = QLocale::system().uiLanguages();
	for (const QString &locale : uiLanguages) {
		const QString baseName = "gui_" + QLocale(locale).name();
		if (translator.load(":/i18n/" + baseName)) {
			a.installTranslator(&translator);
			break;
		}
	}

	//qmlRegisterType<Manager>("Dispenser", 1, 0, "Manager");
	qmlRegisterType<UnitModel>("Dispenser", 1, 0, "UnitModel");
	qmlRegisterUncreatableType<UnitList>("Dispenser", 1, 0, "UnitList",
	                                     QStringLiteral("UnitList should not be created in QML"));

	UnitList unitList;

	QQmlApplicationEngine engine;
	engine.addImportPath(":/imports");

	engine.rootContext()->setContextProperty(QStringLiteral("unitList"), &unitList);

	engine.load(QUrl(QLatin1String("qrc:/main.qml")));
	if (engine.rootObjects().isEmpty())
	    return -1;

	//MainWindow w;
	//w.show();
	return a.exec();
}
