#ifndef QDAEMONLOG_H
#define QDAEMONLOG_H

#include <QString>

class QDaemonLog
{
public:
	QDaemonLog();
public:
	enum EntrySeverity  { NoticeEntry, WarningEntry, ErrorEntry };
	friend void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity);
};

#endif // QDAEMONLOG_H
