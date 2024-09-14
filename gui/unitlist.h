#ifndef UNITLIST_H
#define UNITLIST_H

#include <QObject>
#include <QVector>

#include <slotitem.h>
/*
struct UnitItem
{
	bool done;
	QString description;
};
*/

class UnitList : public QObject
{
	Q_OBJECT
public:
	explicit UnitList(QObject *parent = nullptr);

	QVector<UnitItem> items() const;

	bool setItemAt(int index, const UnitItem &item);


signals:
	void preItemAppended();
	void postItemAppended();

	void preItemRemoved(int index);
	void postItemRemoved();

public slots:
	void appendItem();
	void removeCompletedItems();

private:
	QVector<UnitItem> mItems;


};

#endif // UNITLIST_H
