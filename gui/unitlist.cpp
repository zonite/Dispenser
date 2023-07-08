#include "unitlist.h"

UnitList::UnitList(QObject *parent)
        : QObject{parent}
{
	mItems.append({ true, QStringLiteral("TEsti2")});
	mItems.append({ true, QStringLiteral("TEsti3")});
	mItems.append({ false, QStringLiteral("TEsti4")});
	mItems.append({ false, QStringLiteral("TEsti5")});
	mItems.append({ true, QStringLiteral("TEsti6")});
}

QVector<UnitItem> UnitList::items() const
{
	return mItems;
}

bool UnitList::setItemAt(int index, const UnitItem &item)
{
	if (index < 0 || index >= mItems.size())
		return false;

	const UnitItem &oldItem = mItems.at(index);
	if (item.done == oldItem.done && item.description == oldItem.description)
		return false;

	mItems[index] = item;

	return true;
}

void UnitList::appendItem()
{
	emit preItemAppended();

	UnitItem item;
	item.done = false;
	mItems.append(item);

	emit postItemAppended();
}

void UnitList::removeCompletedItems()
{
	for (int i = 0; i < mItems.size(); ) {
		if (mItems.at(i).done) {
			emit preItemRemoved(i);

			mItems.removeAt(i);

			emit postItemRemoved();
		} else {
			++i;
		}
	}
}
