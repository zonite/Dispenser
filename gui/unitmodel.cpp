#include "unitmodel.h"

#include "unitlist.h"

UnitModel::UnitModel(QObject *parent)
        : QAbstractListModel(parent)
        , mList(nullptr)
{
}

int UnitModel::rowCount(const QModelIndex &parent) const
{
	// For list models only the root node (an invalid parent) should return the list's size. For all
	// other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
	if (parent.isValid() || !mList)
		return 0;

	return mList->items().size();
}

QVariant UnitModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || !mList)
		return QVariant();

	const UnitItem item = mList->items().at(index.row());

	switch (role) {
	        case DoneRole:
		        return QVariant(item.done);
	        case DescriptionRole:
		        return QVariant(item.description);
	}

	return QVariant();
}

bool UnitModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!mList)
		return false;

	UnitItem item = mList->items().at(index.row());

	switch (role) {
	case DoneRole:
		item.done = value.toBool();
		break;
	case DescriptionRole:
		item.description = value.toString();
		break;
	}

	if (mList->setItemAt(index.row(), item)) {
		emit dataChanged(index, index, QVector<int>() << role);
		return true;
	}

	return false;
}

Qt::ItemFlags UnitModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return Qt::ItemIsEditable;
}

QHash<int, QByteArray> UnitModel::roleNames() const
{
	QHash<int, QByteArray> names;
	names[DoneRole] = "done";
	names[DescriptionRole] = "description";
	return names;
}

UnitList *UnitModel::list() const
{
	return mList;
}

void UnitModel::setList(UnitList *list)
{
	if (mList == list)
		return;

	beginResetModel();

	if (mList)
		mList->disconnect(this);

	mList = list;

	if (mList) {
		connect(mList, &UnitList::preItemAppended, this, [=]() {
			const int index = mList->items().size();
			beginInsertRows(QModelIndex(), index, index);
		});
		connect(mList, &UnitList::postItemAppended, this, [=]() {
			endInsertRows();
		});
		connect(mList, &UnitList::preItemRemoved, this, [=](int index) {
			beginRemoveRows(QModelIndex(), index, index);
		});
		connect(mList, &UnitList::postItemRemoved, this, [=]() {
			endRemoveRows();
		});
	}

	endResetModel();

	emit listChanged();
}

/*
void UnitModel::resetList()
{
	setList({}); // TODO: Adapt to use your actual default value
}
*/

