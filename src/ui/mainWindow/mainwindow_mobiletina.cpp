#include "include/ui/mainwindow.h"

#include "include/database/GroupsRepo.h"
#include "include/database/ProfilesRepo.h"
#include "include/ui/mainWindow/TestRunner.h"
#include "include/ui/widget/MobileTinaHomeWidget.hpp"

#include <QMessageBox>
#include <QPointer>
#include <QRegularExpression>
#include <QToolButton>
#include <QDateTime>

#include <limits>

namespace {
qint64 subscriptionValue(const QString &info, const QString &key) {
    const QRegularExpression re(QStringLiteral("(?:^|[;\\s])%1=([0-9]+)").arg(QRegularExpression::escape(key)));
    const auto match = re.match(info);
    return match.hasMatch() ? match.captured(1).toLongLong() : 0;
}
}

void MainWindow::setup_mobiletina_shell() {
    ui->verticalLayout_3->removeWidget(ui->splitter);
    mobileTinaHome = new MobileTinaHomeWidget(ui->splitter, ui->centralwidget);
    ui->verticalLayout_3->insertWidget(1, mobileTinaHome, 1);

    ui->toolButton_startstop->hide();
    ui->checkBox_VPN->hide();
    ui->system_dns->hide();
    ui->checkBox_SystemProxy->hide();
    ui->data_view->hide();
    ui->label_running->hide();
    ui->label_inbound->hide();
    ui->label_speed->hide();

    ui->toolButton_program->setText(QString::fromUtf8("افزودن"));
    ui->toolButton_preferences->setText(QString::fromUtf8("تنظیمات"));
    ui->toolButton_testing->setText(QString::fromUtf8("اشتراک‌ها"));
    ui->toolButton_routing->setText(QString::fromUtf8("مسیریابی"));
    ui->toolButton_tools->setText(QString::fromUtf8("ابزارها"));

    connect(mobileTinaHome, &MobileTinaHomeWidget::automaticConnectRequested,
            this, &MainWindow::mobiletina_smart_connect);
    connect(mobileTinaHome, &MobileTinaHomeWidget::manualConnectRequested, this, [this] {
        m_mobileTinaSmartFailed = false;
        if (running != nullptr) profile_stop(false, false, true);
        else profile_start();
    });
    connect(mobileTinaHome, &MobileTinaHomeWidget::pingRequested,
            this, &MainWindow::mobiletina_ping_selected);
}

void MainWindow::mobiletina_smart_connect() {
    if (running != nullptr) {
        m_mobileTinaSmartTesting = false;
        m_mobileTinaSmartFailed = false;
        testRunner->stop();
        profile_stop(false, false, true);
        return;
    }
    if (m_mobileTinaSmartTesting) {
        m_mobileTinaSmartTesting = false;
        testRunner->stop();
        refresh_mobiletina_ui();
        return;
    }

    const auto group = Configs::dataManager->groupsRepo->CurrentGroup();
    if (group == nullptr || group->Profiles().isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8("MobileTina"),
                                 QString::fromUtf8("ابتدا یک اشتراک یا سرور اضافه کنید."));
        return;
    }

    QList<int> candidates;
    for (int id : group->Profiles()) {
        const auto profile = Configs::dataManager->profilesRepo->GetProfile(id);
        if (profile != nullptr && profile->type != "autoselector") candidates << id;
    }
    if (candidates.isEmpty()) return;

    m_mobileTinaSmartTesting = true;
    m_mobileTinaSmartFailed = false;
    refresh_mobiletina_ui();

    QPointer<MainWindow> guard(this);
    testRunner->runUrlTests(candidates, [guard, candidates] {
        if (guard == nullptr) return;
        runOnUiThread([guard, candidates] {
            if (guard == nullptr || !guard->m_mobileTinaSmartTesting) return;

            int bestId = -1;
            int bestLatency = std::numeric_limits<int>::max();
            for (int id : candidates) {
                const auto profile = Configs::dataManager->profilesRepo->GetProfile(id);
                if (profile != nullptr && profile->latency > 0 && profile->latency < bestLatency) {
                    bestLatency = profile->latency;
                    bestId = id;
                }
            }

            guard->m_mobileTinaSmartTesting = false;
            if (bestId < 0) {
                guard->m_mobileTinaSmartFailed = true;
                guard->refresh_mobiletina_ui();
                QMessageBox::warning(guard, QString::fromUtf8("MobileTina"),
                                     QString::fromUtf8("هیچ سرور فعالی پیدا نشد."));
                return;
            }

            if (const auto current = Configs::dataManager->groupsRepo->CurrentGroup(); current != nullptr) {
                current->test_sort_by = Configs::testBy::latency;
                current->SortProfiles({GroupSortMethod::ByTestResult, false});
                Configs::dataManager->groupsRepo->Save(current);
            }
            guard->refresh_proxy_list({}, true);
            guard->profile_start(bestId);
        }, true);
    });
}

void MainWindow::mobiletina_ping_selected() {
    const int id = running != nullptr ? running->id : get_profile_to_start();
    if (id < 0 || testRunner->isRunning()) return;
    QPointer<MainWindow> guard(this);
    testRunner->runUrlTests({id}, [guard] {
        if (guard == nullptr) return;
        runOnUiThread([guard] {
            if (guard != nullptr) guard->refresh_mobiletina_ui();
        }, true);
    });
}

void MainWindow::refresh_mobiletina_ui() {
    if (mobileTinaHome == nullptr) return;

    std::shared_ptr<Configs::Profile> selected = running;
    if (selected == nullptr) {
        const int id = get_profile_to_start();
        if (id >= 0) selected = Configs::dataManager->profilesRepo->GetProfile(id);
    }
    const QString serverName = selected != nullptr ? selected->outbound->DisplayName() : QString();
    const int latency = selected != nullptr ? selected->latency : 0;

    auto state = MobileTinaHomeWidget::ConnectionState::Disconnected;
    if (m_mobileTinaSmartTesting) state = MobileTinaHomeWidget::ConnectionState::Testing;
    else if (m_profileConnecting) state = MobileTinaHomeWidget::ConnectionState::Connecting;
    else if (running != nullptr) state = MobileTinaHomeWidget::ConnectionState::Connected;
    else if (m_mobileTinaSmartFailed) state = MobileTinaHomeWidget::ConnectionState::Failed;

    const auto group = Configs::dataManager->groupsRepo->CurrentGroup();
    if (group != nullptr) {
        const qint64 total = subscriptionValue(group->info, "total");
        const qint64 used = subscriptionValue(group->info, "upload") + subscriptionValue(group->info, "download");
        const qint64 expire = subscriptionValue(group->info, "expire");
        if (total > 0 || expire > 0) {
            mobileTinaHome->setSubscription(group->name, used, total, expire);
            if (expire > 0 && expire <= QDateTime::currentSecsSinceEpoch()) {
                state = MobileTinaHomeWidget::ConnectionState::Expired;
            }
        } else {
            mobileTinaHome->clearSubscription();
        }
    } else {
        mobileTinaHome->clearSubscription();
    }
    mobileTinaHome->setConnectionState(state, serverName, latency);
}
