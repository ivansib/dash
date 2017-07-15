// Copyright (c) 2015 The Sibcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QResource>
#include <QCryptographicHash>
#include <QTextStream>
#include <QFile>
#include <QTemporaryFile>

#include "offermodel.h"
#include "util.h"

const int MAX_GOODS_URLS = 2;

const QString GOODS_URLS[MAX_GOODS_URLS] = {
		"http://sibcoin.net/goods/",
		"http://chervonec.com/goods/"
};

const QString goods_data = "sibcoin.rcc";
const QString goods_md5 = "sibcoin.md5";


OfferModel::OfferModel(COfferDB *offerdb, QObject *parent):
    QAbstractTableModel(parent),
    offerDB(offerdb)
{
}

OfferModel::~OfferModel()
{
    //delete net_manager;
}

int OfferModel::rowCount(const QModelIndex & /*parent*/) const
{
   return 2;
}

int OfferModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant OfferModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
    	// ask COfferDB to get data and return value for cell
        return QString("Row%1, Column%2")
                   .arg(index.row() + 1)
                   .arg(index.column() +1);
    }
    return QVariant();
}

QVariant OfferModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("first");
            case 1:
                return QString("second");
            case 2:
                return QString("third");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags OfferModel::flags(const QModelIndex & /*index*/) const
{
    //return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled ;
}
