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
#include "listwidget.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>

class LeaveActionModel : public QAbstractListModel
{
private:
    enum Row
    {
        Logout = 0,
        Shutdown,
        Suspend,
        LockScreen,
        Reboot,
        Hibernate,
        NRows
    };

    struct Item
    {
        QIcon icon;
        QString title;
        LXQt::Power::Action action;
    };
    Item items[NRows];

    LXQt::Power *mPower = nullptr;


public:
    LeaveActionModel(LXQt::Power *power, QObject *parent) :
        QAbstractListModel(parent),
        mPower(power)
    {
        // Populate the items
        items[Logout]     = {QIcon::fromTheme(QStringLiteral("system-log-out")),
                             tr("Logout"),
                             LXQt::Power::PowerLogout};
        items[Shutdown]   = {QIcon::fromTheme(QStringLiteral("system-shutdown")),
                             tr("Shutdown"),
                             LXQt::Power::PowerShutdown};
        items[Suspend]    = {QIcon::fromTheme(QStringLiteral("system-suspend")),
                             tr("Suspend"),
                             LXQt::Power::PowerSuspend};
        items[LockScreen] = {QIcon::fromTheme(QStringLiteral("system-lock-screen")),
                             tr("Lock screen"),
                             LXQt::Power::Action(-1)};
        items[Reboot]     = {QIcon::fromTheme(QStringLiteral("system-reboot")),
                             tr("Reboot"),
                             LXQt::Power::PowerReboot};
        items[Hibernate]  = {QIcon::fromTheme(QStringLiteral("system-suspend-hibernate")),
                             tr("Hibernate"),
                             LXQt::Power::PowerHibernate};
    }

    int rowCount(const QModelIndex &p) const override
    {
        return p.isValid() ? 0 : NRows;
    }

    QVariant data(const QModelIndex &idx, int role) const override
    {
        if(idx.row() >= NRows)
            return QVariant();

        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return items[idx.row()].title;
        case Qt::DecorationRole:
            return items[idx.row()].icon;
        case Qt::UserRole:
            return int(items[idx.row()].action);
        default:
            break;
        }

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &idx) const override
    {
        if(idx.row() >= NRows)
            return Qt::NoItemFlags;

        LXQt::Power::Action action = items[idx.row()].action;
        bool canAction = true;
        if(action != -1)
            canAction = mPower->canAction(action);

        Qt::ItemFlags f = Qt::ItemIsSelectable;
        if(canAction)
            f.setFlag(Qt::ItemIsEnabled);
        return f;
    }
};



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

    //ui->listWidget->setRows(2);
    //ui->listWidget->setColumns(3);

    connect(mListView, &QAbstractItemView::activated, this, &LeaveDialog::doAction);
    connect(mCancelButton, &QAbstractButton::clicked, this, [this] { close(); });
}

LeaveDialog::~LeaveDialog()
{
    //delete ui;
}

void LeaveDialog::doAction(const QModelIndex &idx)
{
    bool ok = false;
    const int action = idx.data(Qt::UserRole).toInt(&ok);
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
}
