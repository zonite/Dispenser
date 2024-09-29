#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>

#include <unititem.h>
#include <colitem.h>
#include <slotitem.h>

class Simulator : public QObject
{
	Q_OBJECT
public:
	enum phase {
		RELEASE = 0,
		OPENING,
		OPEN,
		CLOSING,
		CLOSED
	};

	explicit Simulator(UnitItem *unit);

signals:

private slots:
	void open();
	void close();

private:
	UnitItem *m_pUnit = nullptr;
	QTimer m_cTimer;
	enum phase m_iPhase = RELEASE;
	SlotItem *m_pSlot = nullptr;
};

#endif // SIMULATOR_H
