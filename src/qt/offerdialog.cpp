// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015 The SibCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <iostream>

#include "walletmodel.h"
#include "offermodel.h"
#include "offerdialog.h"
#include "ui_offerdialog.h"

#include <QFile>
#include <QTextStream>
#include <QResource>
#include <QByteArray>
#include <QTextDocument>


/** Googs&Service page */
OfferDialog::OfferDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OfferDialog)
    //page_name(":/dev/html/goods.html")
{
    ui->setupUi(this);
    LogPrintf("OfferDialog::OfferDialog\n");
}

OfferDialog::~OfferDialog()
{
    // TODO:
    //unsibscribeModelSignals();
    delete ui;
}

void OfferDialog::setModel(WalletModel *model)
{
    this->model = model;
}

void OfferDialog::setOfferModel(OfferModel *offerModel)
{
    this->offerModel = offerModel;
    this->ui->tableView->show();

//    connect(offerModel, SIGNAL(resourceReady(std::string)),
//            this, SLOT(on_resourceReady(std::string)));
    // QTableView offerModel->fetch();
}

//void OfferDialog::on_resourceReady(std::string res_root)
//{
//    QString htmlContent;
//
//    QFile  htmlFile(page_name);
//    if (!htmlFile.open(QIODevice::ReadOnly | QIODevice::Text)){
//        return;
//    }
//
//    QTextStream in(&htmlFile);
//    in.setCodec("UTF-8");
//    htmlContent = in.readAll();
//
//    QTextDocument *document = new QTextDocument();
//    document->setHtml(htmlContent);
//    ui->textBrowser->setDocument(document);
//}

void OfferDialog::on_addButton_clicked()
{
    this->ui->tableView->show();
    LogPrintf("on_addButton_clicked\n");
}

void OfferDialog::on_chgButton_clicked()
{
}

void OfferDialog::on_delButton_clicked()
{
}
