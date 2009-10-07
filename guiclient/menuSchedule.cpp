/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"
#include "inputManager.h"

#include "plannedOrder.h"
#include "createPlannedOrdersByItem.h"
#include "createPlannedOrdersByPlannerCode.h"
#include "deletePlannedOrder.h"
#include "deletePlannedOrdersByPlannerCode.h"
#include "firmPlannedOrdersByPlannerCode.h"
#include "releasePlannedOrdersByPlannerCode.h"

#include "dspPlannedOrdersByItem.h"
#include "dspPlannedOrdersByPlannerCode.h"
#include "dspMRPDetail.h"
#include "dspRunningAvailability.h"
#include "dspTimePhasedAvailability.h"
#include "dspExpediteExceptionsByPlannerCode.h"
#include "dspReorderExceptionsByPlannerCode.h"

#include "plannerCodes.h"

#include "menuSchedule.h"

menuSchedule::menuSchedule(GUIClient *Pparent) :
 QObject(Pparent)
{
  setObjectName("msModule");
  parent = Pparent;
  
  toolBar = new QToolBar(tr("Schedule Tools"));
  toolBar->setObjectName("Schedule Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowMSToolbar"))
    parent->addToolBar(toolBar);

  mainMenu = new QMenu(parent);
  plannedOrdersMenu = new QMenu(parent);
  plannedOrdersMrpMenu = new QMenu(parent);
  bufferMenu = new QMenu(parent);
  bufferRunMenu = new QMenu(parent);
  bufferInvMenu = new QMenu(parent);
  bufferWoMenu = new QMenu(parent);
  reportsMenu = new QMenu(parent);
  reportsPlannedMenu = new QMenu(parent);
  masterInfoMenu = new QMenu(parent);

  mainMenu->setObjectName("menu.sched");
  plannedOrdersMenu->setObjectName("menu.sched.plannedorders");
  plannedOrdersMrpMenu->setObjectName("menu.sched.plannedordersmrp");
  bufferMenu->setObjectName("menu.sched.buffer");
  bufferRunMenu->setObjectName("menu.sched.bufferrun");
  bufferInvMenu->setObjectName("menu.sched.bufferinv");
  bufferWoMenu->setObjectName("menu.sched.bufferwo");
  reportsMenu->setObjectName("menu.sched.reports");
  reportsPlannedMenu->setObjectName("menu.sched.reportsplanned");
  masterInfoMenu->setObjectName("menu.sched.masterinfo");

  actionProperties acts[] = {
  
    // Schedule | Schedule  
    { "menu",	tr("&Scheduling"), (char*)plannedOrdersMenu,	mainMenu,	"true",	NULL, NULL, true	, NULL },
    { "ms.createPlannedOrder", tr("&New Planned Order..."), SLOT(sCreatePlannedOrder()), plannedOrdersMenu, "CreatePlannedOrders", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, plannedOrdersMenu, "true", NULL, NULL, true , NULL },
 
    // Schedule | Schedule | MRP
    { "menu",	tr("Run &MRP"), (char*)plannedOrdersMrpMenu,	plannedOrdersMenu,	"true",	NULL, NULL, true	, NULL },
    { "ms.runMRPByPlannerCode", tr("by &Planner Code..."), SLOT(sCreatePlannedReplenOrdersByPlannerCode()), plannedOrdersMrpMenu, "CreatePlannedOrders", QPixmap(":/images/runMrpByPlannerCode.png"), toolBar, true , tr("Run MRP by Planner Code") },
    { "ms.runMRPByItem", tr("by &Item..."), SLOT(sCreatePlannedReplenOrdersByItem()), plannedOrdersMrpMenu, "CreatePlannedOrders", NULL, NULL, true , NULL },
    
    { "separator", NULL, NULL, plannedOrdersMenu, "true", NULL, NULL, true , NULL },
    { "ms.firmPlannedOrdersByPlannerCode", tr("&Firm Planned Orders..."), SLOT(sFirmPlannedOrdersByPlannerCode()), plannedOrdersMenu, "FirmPlannedOrders", NULL, NULL, true , NULL },
    { "ms.releasePlannedOrdersByPlannerCode", tr("&Release Planned Orders..."), SLOT(sReleasePlannedOrdersByPlannerCode()),  plannedOrdersMenu, "ReleasePlannedOrders", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, plannedOrdersMenu, "true", NULL, NULL, true , NULL },
    { "ms.deletePlannedOrder", tr("&Delete Planned Order..."), SLOT(sDeletePlannedOrder()),plannedOrdersMenu, "DeletePlannedOrders", NULL, NULL, true , NULL },
    { "ms.deletePlannedOrdersByPlannerCode", tr("Delete Planned Order&s..."), SLOT(sDeletePlannedOrdersByPlannerCode()), plannedOrdersMenu, "DeletePlannedOrders", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, mainMenu, "true", NULL, NULL, true , NULL },
  
    // Schedule | Report
    { "menu",	tr("&Reports"), (char*)reportsMenu, mainMenu, "true", NULL, NULL, true , NULL },
  
    // Schedule | Report | Planned Orders
    { "menu",	tr("Planned &Orders"), (char*)reportsPlannedMenu, reportsMenu, "true", NULL, NULL, true , NULL },
    { "ms.dspPlannedOrdersByPlannerCode", tr("by &Planner Code..."), SLOT(sDspPlannedOrdersByPlannerCode()), reportsPlannedMenu, "ViewPlannedOrders", QPixmap(":/images/dspPlannedOrdersByPlannerCode.png"), toolBar, true , tr("Planned Orders by Planner Code") },
    { "ms.dspPlannedOrdersByItem", tr("by &Item..."), SLOT(sDspPlannedOrdersByItem()), reportsPlannedMenu, "ViewPlannedOrders", NULL, NULL, true , NULL },
    
    { "separator", NULL, NULL, reportsMenu, "true", NULL, NULL, true , NULL },
    { "ms.dspRunningAvailability", tr("&Running Availability..."), SLOT(sDspRunningAvailability()), reportsMenu, "ViewInventoryAvailability", NULL, NULL, true , NULL },
    { "ms.dspTimePhasedAvailabiltiy", tr("&Time-Phased Availability..."), SLOT(sDspTimePhasedAvailability()), reportsMenu, "ViewInventoryAvailability", NULL, NULL, true , NULL },
    { "ms.dspMRPDetail", tr("&MRP Detail..."), SLOT(sDspMRPDetail()), reportsMenu, "ViewInventoryAvailability", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, reportsMenu, "true", NULL, NULL, true , NULL },
    { "ms.dspExpediteExceptionsByPlannerCode", tr("E&xpedite Exceptions..."), SLOT(sDspExpediteExceptionsByPlannerCode()), reportsMenu, "ViewInventoryAvailability", NULL, NULL, true , NULL },
    { "ms.dspReorderExceptionsByPlannerCode", tr("Reorder &Exceptions..."), SLOT(sDspReorderExceptionsByPlannerCode()),reportsMenu, "ViewInventoryAvailability", NULL, NULL, true , NULL },

    { "separator", NULL, NULL, mainMenu, "true", NULL, NULL, true , NULL },
    
    //  Master Information
    { "menu",	tr("&Master Information"), (char*)masterInfoMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "ms.plannerCodes", tr("&Planner Codes..."), SLOT(sPlannerCodes()), masterInfoMenu, "MaintainPlannerCodes ViewPlannerCodes", NULL, NULL, true , NULL },
  };
  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(mainMenu, "Schedule");
  QAction * m = parent->menuBar()->addMenu(mainMenu);
  if(m)
    m->setText(tr("Sche&dule"));
}

void menuSchedule::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  QAction * m = 0;
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == QString("menu"))
    {
      m = acts[i].menu->addMenu((QMenu*)(acts[i].slot));
      if(m)
        m->setText(acts[i].actionTitle);
    }
    else if (acts[i].actionName == QString("separator"))
    {
      acts[i].menu->addSeparator();
    }
    else if ((acts[i].toolBar != NULL) && (!acts[i].toolTip.isEmpty()))
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  (acts[i].pixmap),
					  acts[i].toolBar,
                      acts[i].toolTip) );
    }
    else if (acts[i].toolBar != NULL)
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  (acts[i].pixmap),
					  acts[i].toolBar,
                      acts[i].actionTitle) );
    }
    else
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv ) );
    }
  }
}

void menuSchedule::sCreatePlannedOrder()
{
  ParameterList params;
  params.append("mode", "new");

  plannedOrder newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuSchedule::sCreatePlannedReplenOrdersByItem()
{
  createPlannedOrdersByItem(parent, "", TRUE).exec();
}

void menuSchedule::sCreatePlannedReplenOrdersByPlannerCode()
{
  createPlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void menuSchedule::sDeletePlannedOrder()
{
  deletePlannedOrder(parent, "", TRUE).exec();
}

void menuSchedule::sDeletePlannedOrdersByPlannerCode()
{
  deletePlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void menuSchedule::sFirmPlannedOrdersByPlannerCode()
{
  firmPlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void menuSchedule::sReleasePlannedOrdersByPlannerCode()
{
  releasePlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void menuSchedule::sDspPlannedOrdersByItem()
{
  omfgThis->handleNewWindow(new dspPlannedOrdersByItem());
}

void menuSchedule::sDspPlannedOrdersByPlannerCode()
{
  omfgThis->handleNewWindow(new dspPlannedOrdersByPlannerCode());
}

void menuSchedule::sDspTimePhasedAvailability()
{
  omfgThis->handleNewWindow(new dspTimePhasedAvailability());
}

void menuSchedule::sDspRunningAvailability()
{
  omfgThis->handleNewWindow(new dspRunningAvailability());
}

void menuSchedule::sDspMRPDetail()
{
  omfgThis->handleNewWindow(new dspMRPDetail());
}

void menuSchedule::sDspExpediteExceptionsByPlannerCode()
{
  omfgThis->handleNewWindow(new dspExpediteExceptionsByPlannerCode());
}

void menuSchedule::sDspReorderExceptionsByPlannerCode()
{
  omfgThis->handleNewWindow(new dspReorderExceptionsByPlannerCode());
}

void menuSchedule::sPlannerCodes()
{
  omfgThis->handleNewWindow(new plannerCodes());
}

