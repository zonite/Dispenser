#include <QString>
#include <QDebug>

class QDaemonLog {
public:
	enum EntrySeverity  { NoticeEntry, WarningEntry, ErrorEntry };
	friend void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity);
};

void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity)
{
	Q_UNUSED(severity);

	qDebug() << message;
}
