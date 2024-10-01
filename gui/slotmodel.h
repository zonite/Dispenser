#ifndef SLOTMODEL_H
#define SLOTMODEL_H

#include <QAbstractTableModel>
#include <QObject>

#include <unititem.h>

class SlotModel : public QAbstractTableModel
{
	Q_OBJECT
	Q_PROPERTY(UnitItem *unit READ unit WRITE setUnit NOTIFY unitChanged)
public:
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

	virtual QHash<int, QByteArray> roleNames() const override;

	UnitItem *unit() const;
	void setUnit(UnitItem *unit);
	//void resetList();

signals:
	void unitChanged();

private:
	UnitItem *m_pUnit;
};

#endif // SLOTMODEL_H
