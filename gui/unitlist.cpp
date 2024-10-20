#include "unitlist.h"
#include "websocketclient.h"

UnitList::UnitList(QObject *parent)
        : QObject{parent}
{
	UnitItem *default_unit = new UnitItem(this);
	QSettings settings;
	QString server;

	mItems.append(default_unit);

	//default_unit->setDataServer("wss://dispenser128.nykyri.wg:8080/");
	qDebug() << "Connecting to defaul server";
	server = settings.value("DefaultServer", "wss://dispenser128.nykyri.wg:8080/").toString();
	new WebSocketClient(default_unit, server);
	//default_unit->setDataServer(server);

	settings.setValue("DefaultServer", server);
	settings.sync();
}

QVector<UnitItem *> UnitList::items() const
{
	return mItems;
}

bool UnitList::setItemAt(int index, const UnitItem &item)
{
	if (index < 0 || index >= mItems.size())
		return false;

	const UnitItem *oldItem = mItems.at(index);

	if (oldItem == &item)
		return false;

	//if (item.done == oldItem.done && item.description == oldItem.description)
	//	return false;

	mItems[index] = const_cast<UnitItem *>(&item);

	return true;
}

void UnitList::appendItem()
{
	emit preItemAppended();

	//new UnitItem item;
	//item.done = false;
	mItems.append(new UnitItem);

	emit postItemAppended();
}

void UnitList::removeCompletedItems()
{
	for (int i = 0; i < mItems.size(); ) {
		if (mItems.at(i)->getNight()) {
			emit preItemRemoved(i);

			mItems.removeAt(i);

			emit postItemRemoved();
		} else {
			++i;
		}
	}
}
