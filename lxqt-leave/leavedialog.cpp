/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2010-2015 LXQt team
 * Authors:
 *   Paulo Lieuthier <paulolieuthier@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "leavedialog.h"

#include <QListWidgetItem>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>

LeaveDialog::LeaveDialog(QWidget* parent) :
    QDialog(parent, Qt::Dialog | Qt::WindowMinMaxButtonsHint),
    //ui(new Ui::LeaveDialog),
    mPower(new LXQt::Power(this)),
    mPowerManager(new LXQt::PowerManager(this)),
    mScreensaver(new LXQt::ScreenSaver(this))
{
    //ui->setupUi(this);
    mLabel = new QLabel(tr("What do you want computer to do?"));
    mLabel->setWordWrap(true);

    mListView = new ListWidget;
    mListView->setModel(new LeaveActionModel(mPower, this)); //TODO: make member, delete mPower
    mListView->setFrameShape(QFrame::NoFrame);
    mListView->setViewMode(QListView::IconMode);
    int sz = 2 * style()->pixelMetric(QStyle::PM_LargeIconSize);
    mListView->setIconSize(QSize(sz, sz));
    mListView->setSelectionMode(QListView::NoSelection);
    mListView->setEditTriggers(QListView::NoEditTriggers);
    mListView->setResizeMode(QListView::Adjust);
    mListView->setTabKeyNavigation(true);
    mListView->setTextElideMode(Qt::ElideNone);
    mListView->setFlow(QListView::LeftToRight);
    mListView->setWrapping(true);
    mListView->setSpacing(7);
    mListView->setUniformItemSizes(true);
    mListView->setItemAlignment(Qt::AlignRight);

    mCancelButton = new QPushButton(tr("Cancel"));

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(mLabel);
    lay->addWidget(mListView);
    lay->addWidget(mCancelButton);

    /* This is a special dialog. We want to make it hard to ignore.
       We make it:
           * Undraggable
           * Frameless
           * Stays on top of all other windows
           * Present in all desktops
    */
    //setWindowFlags((Qt::CustomizeWindowHint | Qt::FramelessWindowHint |
    //                Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint));

    // populate the items
    QListWidgetItem * item = new QListWidgetItem{QIcon::fromTheme(QStringLiteral("system-log-out")), tr("Logout")};
    item->setData(Qt::UserRole, LXQt::Power::PowerLogout);
    if (!mPower->canAction(LXQt::Power::PowerLogout))
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    ui->listWidget->addItem(item);
    item = new QListWidgetItem{QIcon::fromTheme(QStringLiteral("system-shutdown")), tr("Shutdown")};
    item->setData(Qt::UserRole, LXQt::Power::PowerShutdown);
    if (!mPower->canAction(LXQt::Power::PowerShutdown))
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    ui->listWidget->addItem(item);
    item = new QListWidgetItem{QIcon::fromTheme(QStringLiteral("system-suspend")), tr("Suspend")};
    item->setData(Qt::UserRole, LXQt::Power::PowerSuspend);
    if (!mPower->canAction(LXQt::Power::PowerSuspend))
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    ui->listWidget->addItem(item);
    item = new QListWidgetItem{QIcon::fromTheme(QStringLiteral("system-lock-screen")), tr("Lock screen")};
    item->setData(Qt::UserRole, -1);
    ui->listWidget->addItem(item);
    item = new QListWidgetItem{QIcon::fromTheme(QStringLiteral("system-reboot")), tr("Reboot")};
    item->setData(Qt::UserRole, LXQt::Power::PowerReboot);
    if (!mPower->canAction(LXQt::Power::PowerReboot))
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    ui->listWidget->addItem(item);
    item = new QListWidgetItem{QIcon::fromTheme(QStringLiteral("system-suspend-hibernate")), tr("Hibernate")};
    item->setData(Qt::UserRole, LXQt::Power::PowerHibernate);
    if (!mPower->canAction(LXQt::Power::PowerHibernate))
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    ui->listWidget->addItem(item);

    ui->listWidget->setRows(2);
    ui->listWidget->setColumns(3);

    connect(mListView, &QAbstractItemView::activated, this, &LeaveDialog::doAction);
        bool ok = false;
        const int action = index.data(Qt::UserRole).toInt(&ok);
        if (!ok)
        {
            qWarning("Invalid internal logic, no UserRole set!?");
            return;
        }
        close();
        switch (action)
        {
        case LXQt::Power::PowerLogout:
                mPowerManager->logout();
                break;
            case LXQt::Power::PowerShutdown:
                mPowerManager->shutdown();
                break;
            case LXQt::Power::PowerSuspend:
                mPowerManager->suspend();
                break;
            case -1:
                {
                    QEventLoop loop;
                    connect(mScreensaver, &LXQt::ScreenSaver::done, &loop, &QEventLoop::quit);
                    mScreensaver->lockScreen();
                    loop.exec();
                }
                break;
            case LXQt::Power::PowerReboot:
                mPowerManager->reboot();
                break;
            case LXQt::Power::PowerHibernate:
                mPowerManager->hibernate();
                break;
        }
    });
    connect(mCancelButton, &QAbstractButton::clicked, this, [this] { close(); });
}

LeaveDialog::~LeaveDialog()
{
    delete ui;
    //delete ui;
}
