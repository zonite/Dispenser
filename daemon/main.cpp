#include <QDaemonApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>

#include <dispenser.h>

#include "kernelclient.h"

int main(int argc, char *argv[])
{
    QDaemonApplication a(argc, argv);
    QDaemonApplication::setApplicationName(DAEMON_NAME);
    QDaemonApplication::setApplicationVersion(DAEMON_VER);
    QDaemonApplication::setOrganizationDomain("dispenser.daemon");

    /*
    QCommandLineParser parser;
    parser.setApplicationDescription("Food Dispenser Daemon");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption fgOption(QStringList() << "f" << "foreground",
	    QCoreApplication::translate("main", "Do not daemonize."));
    parser.addOption(fgOption);

    // Process the actual command line arguments given by the user
    parser.process(a);

    bool fg = parser.isSet(fgOption);
    */

    KernelClient kernel(&a);


    QObject::connect(&a, &QDaemonApplication::daemonized, &kernel, &KernelClient::start);
    QObject::connect(&a, &QDaemonApplication::aboutToQuit, &kernel, &KernelClient::stop);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
	const QString baseName = "daemon_" + QLocale(locale).name();
	if (translator.load(":/i18n/" + baseName)) {
		a.installTranslator(&translator);
	    break;
	}
    }

    return QDaemonApplication::exec();
//    return a.exec();
}
