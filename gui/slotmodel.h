#ifndef SLOTMODEL_H
#define SLOTMODEL_H

//#include <array>
//#include <QHash>
#include <QAbstractTableModel>
#include <QObject>
#include <QtQml/qqml.h>

#include <unititem.h>
#include "unitlist.h"

/*
class SlotModelWorker : public QObject
{
	Q_OBJECT

public:

public slots:
	void doReEncode();
	void doSend();

signals:
	void done(int result);

private:

};
*/

class SlotModel : public QAbstractTableModel
{
	Q_OBJECT
	QML_ELEMENT
	Q_PROPERTY(UnitItem *unit READ unit WRITE setUnit NOTIFY unitChanged)
	//Q_PROPERTY(UnitList *parentUnitList READ unitList WRITE setUnitList NOTIFY unitListChanged)
	Q_PROPERTY(int num READ num WRITE setNum NOTIFY numChanged)

	Q_PROPERTY(int charging READ getCharging NOTIFY chargingChanged)
	Q_PROPERTY(int door READ getDoor NOTIFY doorChanged)
	Q_PROPERTY(int night READ getNight NOTIFY nightChanged)
	Q_PROPERTY(int light READ getLight NOTIFY lightChanged)


	Q_ENUMS(Roles)
public:
	enum Roles {
		CellRole = Qt::UserRole,
		StateRole,
		AlarmRole
		//LightRole,
		//DoorRole,
		//NightRole,
		//ChargingRole,
		//ConnRole
	};

	QHash<int, QByteArray> roleNames() const override {
		return {
			{ Qt::DisplayRole, "display" },
			{ Qt::ToolTipRole, "tooltip" },
			{ CellRole, "value" },
			{ StateRole, "state" },
			{ AlarmRole, "alarm" }
			//{ LightRole, "light" },
			//{ DoorRole, "door" },
			//{ NightRole, "night" },
			//{ ChargingRole, "charging" },
			//{ ConnRole, "conn" },
		};
	}

	explicit SlotModel(QObject *parent = nullptr);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	// Editable: Unit cannot set its slots!
	//bool setData(const QModelIndex &index, const QVariant &value,
	//             int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	//virtual QHash<int, QByteArray> roleNames() const override;

	UnitList *unitList() const { return m_pUnitList; }
	void setUnitList(UnitList *list) { m_pUnitList = list; }

	UnitItem *unit() const;
	void setUnit(UnitItem *unit);
	//void resetList();
	int num() const { return m_iIndex;  }
	void setNum(int i ) { m_iIndex = i; emit numChanged(); }


	//Q_INVOKABLE bool getCharging();

	Q_INVOKABLE void nextStep();
	Q_INVOKABLE bool loadFile(const QString &filename);
	Q_INVOKABLE void loadPatter(const QString &plainText);
	Q_INVOKABLE void clear();

	bool getCharging() const;
	bool getDoor() const;
	bool getNight() const;
	bool getLight() const;

public slots:
	void newCol(ColItem *col);
	void newSlot(SlotItem *slot);
	void slotDataChanged(SlotItem *slot);

	void chargingUnit();
	void doorUnit();
	void nightUnit();
	void lightUnit();

signals:
	void unitListChanged();
	void unitChanged();
	void numChanged();
	void chargingChanged();
	void doorChanged();
	void nightChanged();
	void lightChanged();

private:
	UnitList *m_pUnitList;
	UnitItem *m_pUnit;
	int m_iIndex = 0;
};

#endif // SLOTMODEL_H
