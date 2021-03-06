/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPurchaseReqsByItem.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"

#include "dspRunningAvailability.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "salesOrderItem.h"
#include "errorReporter.h"

dspPurchaseReqsByItem::dspPurchaseReqsByItem(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspPurchaseReqsByItem", fl)
{
  setupUi(optionsWidget());
  setNewVisible(true);
  setWindowTitle(tr("Purchase Requests by Item"));
  setListLabel(tr("Purchase Requests"));
  setReportName("PurchaseRequestsByItem");
  setMetaSQLOptions("purchase", "purchaserequests");
  setUseAltId(true);

  list()->addColumn(tr("P/R #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_number");
  list()->addColumn(tr("Sub #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_subnumber");
  list()->addColumn(tr("Status"),       _statusColumn, Qt::AlignCenter, true,  "pr_status" );
  list()->addColumn(tr("Parent Order"), _orderColumn,  Qt::AlignLeft,   true,  "parent"   );
  list()->addColumn(tr("Create Date"),  _dateColumn,   Qt::AlignCenter, true,  "pr_createdate"  );
  list()->addColumn(tr("Due Date"),     _dateColumn,   Qt::AlignCenter, true,  "pr_duedate" );
  list()->addColumn(tr("Qty."),         _qtyColumn,    Qt::AlignRight,  true,  "pr_qtyreq"  );
  list()->addColumn(tr("Netable QOH"),  _qtyColumn,    Qt::AlignRight,  true,  "netableqoh"  );
  list()->addColumn(tr("Reorder Lvl."), _qtyColumn,    Qt::AlignRight,  true,  "itemsite_reorderlevel"  );
  list()->addColumn(tr("Notes"),        -1,            Qt::AlignLeft,   true,  "pr_releasenote"  );

  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (_privileges->check("MaintainPurchaseRequests"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));

  newAction()->setEnabled(false);
  
  connect(omfgThis, SIGNAL(purchaseRequestsUpdated()), this, SLOT(sFillList()));
  connect(_item,    SIGNAL(valid(bool)), newAction(), SLOT(setEnabled(bool)));
}

void dspPurchaseReqsByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPurchaseReqsByItem::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  if (! _item->isValid())
  {
    QMessageBox::information(this, tr("Item Required"),
                             tr("<p>Item is required."));
    _item->setFocus();
    return false;
  }

  _warehouse->appendValue(params);
  params.append("item_id", _item->id());
  params.append("manual", tr("Manual"));
  params.append("other",  tr("Other"));

  return true;
}

void dspPurchaseReqsByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem* pItem, int)
{
  XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem *>(pItem);
  if(0 == item)
    return;

  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit P/R..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests"));

  menuItem = pMenu->addAction(tr("Release P/R..."), this, SLOT(sRelease()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

  menuItem = pMenu->addAction(tr("Delete P/R..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests"));

  if(item->rawValue("parent").toString().indexOf("S/O") == 0)
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("View S/O Item"), this, SLOT(sViewSOItem()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));
  }
}

void dspPurchaseReqsByItem::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPurchaseReqsByItem::sRelease()
{
  XSqlQuery dspRelease;
  dspRelease.prepare("SELECT releasePr(:pr_id) AS _result;");
  
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  if (selected.size() == 1)
  {
    ParameterList params;
    params.append("mode", "releasePr");
    params.append("pr_id", list()->id());
    
    purchaseOrder *newdlg = new purchaseOrder();
    if(newdlg->set(params) == NoError)
      omfgThis->handleNewWindow(newdlg);
    else
      delete newdlg;
  }
  else
  {
    if (QMessageBox::question(this, tr("Release multiple PRs?"),
                              tr("<p>Purchase Requests will be released "
                                 "using default Item Sources and added "
                                 "to unreleased Purchase Orders."
                                 "<p>Do you want to continue?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      for (int i = 0; i < selected.size(); i++)
      {
        dspRelease.bindValue(":pr_id", ((XTreeWidgetItem*)(selected[i]))->id());
        dspRelease.exec();
        if (dspRelease.first())
        {
          if (dspRelease.value("_result").toInt() < 0)
            QMessageBox::information(this, tr("Release Error"),
                                     tr("<p>Purchase Request %1 "
                                        "could not be released.").arg(selected[i]->rawValue("pr_number").toString()),
                                     QMessageBox::Ok|QMessageBox::Default);
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Releasing Purchase Request(s)"),
                                      dspRelease, __FILE__, __LINE__))
        {
          return;
        }
      }
    }
  }

  sFillList();
  omfgThis->sPurchaseRequestsUpdated();
}

void dspPurchaseReqsByItem::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());
  
  purchaseRequest newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspPurchaseReqsByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pr_id", list()->id());
  
  purchaseRequest newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspPurchaseReqsByItem::sDelete()
{
  XSqlQuery dspDelete;
  dspDelete.prepare("SELECT deletePr(:pr_id) AS _result;");
  
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    dspDelete.bindValue(":pr_id", ((XTreeWidgetItem*)(selected[i]))->id());
    dspDelete.exec();
  }

  sFillList();
  omfgThis->sPurchaseRequestsUpdated();
}

void dspPurchaseReqsByItem::sViewSOItem()
{
  XSqlQuery soitem;
  soitem.prepare("SELECT pr_order_id AS so FROM pr WHERE pr_id=:pr_id");
  soitem.bindValue(":pr_id", list()->id());
  soitem.exec();
  if (soitem.first())
  {
    ParameterList params;
    params.append("soitem_id", soitem.value("so"));
    params.append("mode", "view");

    salesOrderItem *newdlg = new salesOrderItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error opening Sales Order item"),
                                      soitem, __FILE__, __LINE__))
  {
    return;
  }
}

