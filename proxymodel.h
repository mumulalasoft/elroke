#ifndef PROXYMODEL_H
#define PROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QMap>

class ProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QMap<int, Qt::Alignment>alignMap;

public:
      enum mode{column,smart};
      enum role{sortRole=Qt::UserRole};

    explicit ProxyModel(QObject* parent = 0);
    explicit ProxyModel(mode, QObject *parent=0);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
  void search(QString);
//  void search(QVariantList);
  void setAlignment(unsigned int, Qt::Alignment);
  void reset();

private slots:
  void clearAlign();

private:
    QString text_search;
    int colom=0;
    mode md;

protected :
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

};

#endif // PROXYMODEL_H
