// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015 The SibCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef OFFERDIALOG_H
#define	OFFERDIALOG_H

#include "walletmodel.h"
#include "offermodel.h"

#include <QDialog>
#include <QObject>

namespace Ui {
    class OfferDialog;
}

/** Dialog to show the HTML page with sales points that accept sibcoins */
class OfferDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OfferDialog(QWidget *parent=0);
    ~OfferDialog();

    void setModel(WalletModel *model);
    void setOfferModel(OfferModel *offerModel);
    
private:
    Ui::OfferDialog *ui;
//    QString header;
//    QString coreOptions;
//    QString uiOptions;
    
    WalletModel *model;
    OfferModel *offerModel;
    
//    const QString page_name;
    
public Q_SLOTS:
    void on_addButton_clicked();
    void on_chgButton_clicked();
    void on_delButton_clicked();
};


#endif	/* GOODSDIALOG_H */

