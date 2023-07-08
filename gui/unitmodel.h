#ifndef UNITMODEL_H
#define UNITMODEL_H

#include <QAbstractListModel>

class UnitList;

class UnitModel : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(UnitList *list READ list WRITE setList NOTIFY listChanged)
	//Q_PROPERTY(UnitList *list READ list WRITE setList RESET resetList NOTIFY listChanged)

public:
	explicit UnitModel(QObject *parent = nullptr);

	enum {
		DoneRole = Qt::UserRole,
		DescriptionRole
	};

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	// Editable:
	bool setData(const QModelIndex &index, const QVariant &value,
	             int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	virtual QHash<int, QByteArray> roleNames() const override;

	UnitList *list() const;
	void setList(UnitList *list);
	//void resetList();

signals:
	void listChanged();

private:
	UnitList *mList;
};

#endif // UNITMODEL_H
