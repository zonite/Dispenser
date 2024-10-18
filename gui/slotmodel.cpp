#include "slotmodel.h"

#include <QDebug>

SlotModel::SlotModel(QObject *parent)
        : QAbstractTableModel{parent}
{
	qDebug() << "Slot constractor" << parent;
}

int SlotModel::rowCount(const QModelIndex &parent) const
{
	//qDebug() << "rowCount: Parent row" << parent.row() << parent.model() << "Parent col" << parent.column();
	//qDebug() << "Parent is" << static_cast<UnitItem*>(parent.internalPointer());
	//return 3;
	// For list models only the root node (an invalid parent) should return the list's size. For all
	// other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
	if (parent.isValid() || !m_pUnit)
		return 0;

	return m_pUnit->getRows();
}

int SlotModel::columnCount(const QModelIndex &parent) const
{
	//qDebug() << "columnCount: Parent row" << parent.row() << "Parent col" << parent.column();
	//qDebug() << "Parent is" << static_cast<UnitItem*>(parent.internalPointer());
	//return 2;

	//QAbstrackItemModel::rootItem.get();

	if (parent.isValid() || !m_pUnit)
		return 0;

	return m_pUnit->numCols();
}

QVariant SlotModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || !m_pUnit)
		return QVariant();

	const SlotItem *slot = m_pUnit->slot(index.column(), index.row());
	Q_UNUSED(slot);
	assert(slot);
	qDebug() << QStringLiteral("Slot is %1/%2: state=%3, release=%4, up=%5, down=%6, full=%7")
	            .arg(QString::number(slot->getCol()->getId()))
	            .arg(QString::number(slot->getId()))
	            .arg(slot->getStateStr())
	            .arg(QString::number(slot->getRel()))
	            .arg(QString::number(slot->getUp()))
	            .arg(QString::number(slot->getDown()))
	            .arg(QString::number(slot->getFull()));

	switch (role) {
	case Qt::DisplayRole:
		return QString("displayRole");
	case StateRole:
		return slot->guiState();
	//case CellRole:
		//return QVariant(true);
		/*
	case DoneRole:
		return QVariant(1);
		//return QVariant(item.done);
	case DescriptionRole:
		return QVariant("joo");
		//return QVariant(item.description);
	*/
	}

	return QVariant();
}

QVariant SlotModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (!m_pUnit)
		return QVariant();

	Q_UNUSED(orientation);

	switch (role) {
	case Qt::DisplayRole:
		return QVariant(section);

	}

	return QVariant();
}

/*
bool SlotModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!m_pUnit)
		return false;

	SlotItem *slot = m_pUnit->slot(index.column(), index.row());
	bool joo = item->getDoor();
	Q_UNUSED(joo);
	QString test;

	switch (role) {
	case DoneRole:
		joo = value.toBool();
		//item.done = value.toBool();
		break;
	case DescriptionRole:
		test = value.toString();
		//item.description = value.toString();
		break;
	}

	if (m_pUnit->setSlotAt(index.column(), index.row(), *slot)) {
		emit dataChanged(index, index, QVector<int>() << role);
		return true;
	}

	return false;
}
*/

Qt::ItemFlags SlotModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	//When editable:
	//return Qt::ItemIsEditable;
	return Qt::NoItemFlags;
}

/*
QHash<int, QByteArray> SlotModel::roleNames() const
{
	QHash<int, QByteArray> names;
	//names[DoneRole] = "done";
	//names[DescriptionRole] = "description";
	return names;
}
*/

UnitItem *SlotModel::unit() const
{
	return m_pUnit;
}

void SlotModel::setUnit(UnitItem *unit)
{
	if (m_pUnit == unit)
		return;

	beginResetModel();

	if (m_pUnit)
		m_pUnit->disconnect(this);

	qDebug() << "Set unit!";

	m_pUnit = unit;

	if (m_pUnit) {
		connect(m_pUnit, &UnitItem::preColAppended, this, [=]() {
			const int index = m_pUnit->numCols();
			beginInsertColumns(QModelIndex(), index, index);
		});
		connect(m_pUnit, &UnitItem::postColAppended, this, [=]() {
			endInsertColumns();
		});
		connect(m_pUnit, &UnitItem::preColRemoved, this, [=](int index) {
			beginRemoveColumns(QModelIndex(), index, index);
		});
		connect(m_pUnit, &UnitItem::postColRemoved, this, [=]() {
			endRemoveColumns();
		});
		connect(m_pUnit, &UnitItem::preSlotAppended, this, [=]() {
			const int index = m_pUnit->getRows();
			beginInsertRows(QModelIndex(), index, index);
		});
		connect(m_pUnit, &UnitItem::postSlotAppended, this, [=]() {
			endInsertRows();
		});
		connect(m_pUnit, &UnitItem::preSlotRemoved, this, [=](int index) {
			beginRemoveRows(QModelIndex(), index, index);
		});
		connect(m_pUnit, &UnitItem::postSlotRemoved, this, [=]() {
			endRemoveRows();
		});
		connect(m_pUnit, &UnitItem::newCol, this, &SlotModel::newCol);
	}

	endResetModel();

	emit unitChanged();
}

void SlotModel::nextStep()
{
	return;
}

bool SlotModel::loadFile(const QString &filename)
{
	Q_UNUSED(filename)

	return true;
}

void SlotModel::loadPatter(const QString &plainText)
{
	Q_UNUSED(plainText)

	return;
}

void SlotModel::clear()
{
	return;
}

void SlotModel::newCol(ColItem *col)
{
	connect(col, &ColItem::newSlot, this, &SlotModel::newSlot);
}

void SlotModel::newSlot(SlotItem *slot)
{
	connect(slot, &SlotItem::fullChanged, this, &SlotModel::slotDataChanged);
	connect(slot, &SlotItem::releaseChanged, this, &SlotModel::slotDataChanged);
	connect(slot, &SlotItem::upChanged, this, &SlotModel::slotDataChanged);
	connect(slot, &SlotItem::downChanged, this, &SlotModel::slotDataChanged);
	connect(slot, &SlotItem::stateChanged, this, &SlotModel::slotDataChanged);
}

void SlotModel::slotDataChanged(SlotItem *slot)
{
	//QModelIndex index = createIndex(slot->getCol()->getId(), slot->getId());
	QModelIndex index = createIndex(slot->getId(), slot->getCol()->getId());

	emit dataChanged(index, index);
}
