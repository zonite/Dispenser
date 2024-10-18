#ifndef SLOTMODEL_H
#define SLOTMODEL_H

//#include <array>
//#include <QHash>
#include <QAbstractTableModel>
#include <QObject>
#include <QtQml/qqml.h>

#include <unititem.h>
#include "unitlist.h"

class SlotModel : public QAbstractTableModel
{
	Q_OBJECT
	QML_ELEMENT
	Q_PROPERTY(UnitItem *unit READ unit WRITE setUnit NOTIFY unitChanged)
	//Q_PROPERTY(UnitList *parentUnitList READ unitList WRITE setUnitList NOTIFY unitListChanged)
	Q_PROPERTY(int num READ num WRITE setNum NOTIFY numChanged)

	Q_ENUMS(Roles)
public:
	enum Roles {
		CellRole = Qt::UserRole,
		StateRole

	};

	QHash<int, QByteArray> roleNames() const override {
		return {
			{ Qt::DisplayRole, "display" },
			{ Qt::ToolTipRole, "tooltip" },
			{ CellRole, "value" },
			{ StateRole, "state" }
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


	Q_INVOKABLE void nextStep();
	Q_INVOKABLE bool loadFile(const QString &filename);
	Q_INVOKABLE void loadPatter(const QString &plainText);
	Q_INVOKABLE void clear();

public slots:
	void newCol(ColItem *col);
	void newSlot(SlotItem *slot);
	void slotDataChanged(SlotItem *slot);

signals:
	void unitListChanged();
	void unitChanged();
	void numChanged();

private:
	UnitList *m_pUnitList;
	UnitItem *m_pUnit;
	int m_iIndex = 0;
};

#endif // SLOTMODEL_H
