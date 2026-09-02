#include "include/ui/widget/MobileTinaHomeWidget.hpp"

#include <QDateTime>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <cmath>

namespace {
constexpr int kAutomaticPage = 0;
constexpr int kManualPage = 1;

QToolButton *artButton(const QString &resource, int size, QWidget *parent) {
    auto *button = new QToolButton(parent);
    button->setAutoRaise(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedSize(size, size);
    button->setIcon(QIcon(resource));
    button->setIconSize(QSize(size, size));
    button->setStyleSheet("QToolButton { border: 0; background: transparent; padding: 0; }"
                          "QToolButton:pressed { padding: 5px; }");
    return button;
}
}

MobileTinaHomeWidget::MobileTinaHomeWidget(QWidget *manualContent, QWidget *parent) : QWidget(parent) {
    setObjectName("mobileTinaHome");
    setLayoutDirection(Qt::LeftToRight);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 8, 16, 10);
    root->setSpacing(10);

    auto *modeSurface = new QFrame(this);
    modeSurface->setObjectName("mobileTinaModeSurface");
    auto *modeLayout = new QHBoxLayout(modeSurface);
    modeLayout->setContentsMargins(4, 4, 4, 4);
    modeLayout->setSpacing(4);

    manualModeButton_ = new QPushButton(QString::fromUtf8("دستی"), modeSurface);
    automaticModeButton_ = new QPushButton(QString::fromUtf8("خودکار"), modeSurface);
    for (auto *button : {manualModeButton_, automaticModeButton_}) {
        button->setMinimumHeight(48);
        button->setCursor(Qt::PointingHandCursor);
        QFont font = button->font();
        font.setBold(true);
        font.setPointSize(11);
        button->setFont(font);
        modeLayout->addWidget(button, 1);
    }
    root->addWidget(modeSurface);

    pages_ = new QStackedWidget(this);
    root->addWidget(pages_, 1);

    auto *automaticPage = new QWidget(pages_);
    auto *automaticLayout = new QVBoxLayout(automaticPage);
    automaticLayout->setContentsMargins(24, 28, 24, 18);
    automaticLayout->setAlignment(Qt::AlignHCenter);
    automaticLayout->addStretch(1);

    automaticConnectButton_ = artButton(":/mobiletina/white.png", 236, automaticPage);
    automaticConnectButton_->setToolTip(QString::fromUtf8("اتصال هوشمند"));
    automaticLayout->addWidget(automaticConnectButton_, 0, Qt::AlignHCenter);

    automaticStatus_ = new QLabel(QString::fromUtf8("قطع"), automaticPage);
    QFont statusFont = automaticStatus_->font();
    statusFont.setBold(true);
    statusFont.setPointSize(15);
    automaticStatus_->setFont(statusFont);
    automaticStatus_->setAlignment(Qt::AlignCenter);
    automaticLayout->addWidget(automaticStatus_);

    automaticPing_ = new QLabel(automaticPage);
    automaticPing_->setAlignment(Qt::AlignCenter);
    automaticPing_->setCursor(Qt::PointingHandCursor);
    automaticPing_->setStyleSheet("QLabel { color: #1976d2; font-weight: 700; padding: 5px 18px; }");
    automaticLayout->addWidget(automaticPing_);

    automaticServer_ = new QLabel(automaticPage);
    automaticServer_->setAlignment(Qt::AlignCenter);
    automaticServer_->setWordWrap(true);
    QFont serverFont = automaticServer_->font();
    serverFont.setBold(true);
    automaticServer_->setFont(serverFont);
    automaticLayout->addWidget(automaticServer_);

    subscriptionCard_ = new QFrame(automaticPage);
    subscriptionCard_->setObjectName("mobileTinaSubscriptionCard");
    subscriptionCard_->setMaximumWidth(620);
    auto *shadow = new QGraphicsDropShadowEffect(subscriptionCard_);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 35));
    subscriptionCard_->setGraphicsEffect(shadow);
    auto *subscriptionLayout = new QVBoxLayout(subscriptionCard_);
    subscriptionLayout->setContentsMargins(18, 15, 18, 14);
    subscriptionLayout->setSpacing(10);
    subscriptionName_ = new QLabel(subscriptionCard_);
    subscriptionName_->setAlignment(Qt::AlignCenter);
    QFont subscriptionFont = subscriptionName_->font();
    subscriptionFont.setBold(true);
    subscriptionName_->setFont(subscriptionFont);
    subscriptionLayout->addWidget(subscriptionName_);
    subscriptionProgress_ = new QProgressBar(subscriptionCard_);
    subscriptionProgress_->setRange(0, 100);
    subscriptionProgress_->setTextVisible(false);
    subscriptionProgress_->setFixedHeight(8);
    subscriptionLayout->addWidget(subscriptionProgress_);
    auto *details = new QHBoxLayout;
    subscriptionDays_ = new QLabel(subscriptionCard_);
    subscriptionUsage_ = new QLabel(subscriptionCard_);
    subscriptionUsage_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    subscriptionUsage_->setStyleSheet("font-weight: 700;");
    details->addWidget(subscriptionDays_, 1);
    details->addWidget(subscriptionUsage_, 1);
    subscriptionLayout->addLayout(details);
    automaticLayout->addSpacing(14);
    automaticLayout->addWidget(subscriptionCard_);
    automaticLayout->addStretch(1);
    pages_->addWidget(automaticPage);

    auto *manualPage = new QWidget(pages_);
    auto *manualLayout = new QVBoxLayout(manualPage);
    manualLayout->setContentsMargins(0, 0, 0, 0);
    manualLayout->setSpacing(8);
    manualContent->setParent(manualPage);
    manualLayout->addWidget(manualContent, 1);
    auto *manualBar = new QFrame(manualPage);
    manualBar->setObjectName("mobileTinaManualBar");
    auto *manualBarLayout = new QHBoxLayout(manualBar);
    manualBarLayout->setContentsMargins(18, 8, 18, 8);
    auto *manualText = new QVBoxLayout;
    manualServer_ = new QLabel(QString::fromUtf8("یک سرور انتخاب کنید"), manualBar);
    manualServer_->setLayoutDirection(Qt::RightToLeft);
    manualServer_->setStyleSheet("font-weight: 700;");
    manualPing_ = new QLabel(QString::fromUtf8("پینگ: نامشخص"), manualBar);
    manualPing_->setStyleSheet("color: #1976d2; font-weight: 700;");
    manualText->addWidget(manualServer_);
    manualText->addWidget(manualPing_);
    manualBarLayout->addLayout(manualText, 1);
    manualConnectButton_ = artButton(":/mobiletina/stop.png", 76, manualBar);
    manualBarLayout->addWidget(manualConnectButton_);
    manualLayout->addWidget(manualBar);
    pages_->addWidget(manualPage);

    setStyleSheet(
        "#mobileTinaHome { background: #ffffff; color: #1c1b1f; }"
        "#mobileTinaModeSurface { background: #efeff2; border-radius: 25px; }"
        "#mobileTinaSubscriptionCard, #mobileTinaManualBar { background: #f8fbff; border: 1px solid #d6e9ff; border-radius: 20px; }"
        "QProgressBar { border: 0; border-radius: 4px; background: #d6e9ff; }"
        "QProgressBar::chunk { border-radius: 4px; background: #1976d2; }"
    );

    connect(manualModeButton_, &QPushButton::clicked, this, [this] { selectMode(kManualPage); });
    connect(automaticModeButton_, &QPushButton::clicked, this, [this] { selectMode(kAutomaticPage); });
    connect(automaticConnectButton_, &QToolButton::clicked, this, &MobileTinaHomeWidget::automaticConnectRequested);
    connect(manualConnectButton_, &QToolButton::clicked, this, &MobileTinaHomeWidget::manualConnectRequested);
    connect(automaticPing_, &QLabel::linkActivated, this, &MobileTinaHomeWidget::pingRequested);
    connect(manualPing_, &QLabel::linkActivated, this, &MobileTinaHomeWidget::pingRequested);
    selectAutomaticMode();
    clearSubscription();
}

void MobileTinaHomeWidget::selectMode(int index) {
    pages_->setCurrentIndex(index);
    styleModeButtons();
}

void MobileTinaHomeWidget::selectAutomaticMode() {
    selectMode(kAutomaticPage);
}

void MobileTinaHomeWidget::styleModeButtons() {
    const auto apply = [](QPushButton *button, bool selected) {
        button->setStyleSheet(selected
            ? "QPushButton { background: #000; color: #fff; border: 0; border-radius: 21px; }"
            : "QPushButton { background: transparent; color: #49454f; border: 0; border-radius: 21px; }"
              "QPushButton:hover { background: #e2e2e7; }");
    };
    apply(automaticModeButton_, pages_->currentIndex() == kAutomaticPage);
    apply(manualModeButton_, pages_->currentIndex() == kManualPage);
}

void MobileTinaHomeWidget::setConnectionState(ConnectionState state, const QString &serverName, int latencyMs) {
    QString art = ":/mobiletina/white.png";
    QString status = QString::fromUtf8("قطع");
    switch (state) {
        case ConnectionState::Testing:
            art = ":/mobiletina/yellow.png";
            status = QString::fromUtf8("در حال یافتن بهترین سرور…");
            break;
        case ConnectionState::Connecting:
            art = ":/mobiletina/yellow.png";
            status = QString::fromUtf8("در حال اتصال…");
            break;
        case ConnectionState::Connected:
            art = ":/mobiletina/blue.png";
            status = QString::fromUtf8("متصل");
            break;
        case ConnectionState::Failed:
            art = ":/mobiletina/red.png";
            status = QString::fromUtf8("اتصال ناموفق بود");
            break;
        case ConnectionState::Expired:
            art = ":/mobiletina/red.png";
            status = QString::fromUtf8("اشتراک شما به پایان رسید");
            break;
        case ConnectionState::Disconnected:
            break;
    }
    automaticConnectButton_->setIcon(QIcon(art));
    automaticConnectButton_->setIconSize(automaticConnectButton_->size());
    automaticStatus_->setText(status);
    automaticServer_->setText(serverName);
    automaticServer_->setVisible(!serverName.isEmpty() && state != ConnectionState::Disconnected);
    const QString ping = latencyMs > 0
        ? QString::fromUtf8("<a href=\"ping\">پینگ: %1 میلی‌ثانیه</a>").arg(latencyMs)
        : QString::fromUtf8("<a href=\"ping\">برای تست پینگ کلیک کنید</a>");
    automaticPing_->setText(ping);
    automaticPing_->setVisible(state == ConnectionState::Connected || state == ConnectionState::Connecting);

    manualServer_->setText(serverName.isEmpty() ? QString::fromUtf8("یک سرور انتخاب کنید") : serverName);
    manualPing_->setText(latencyMs > 0
        ? QString::fromUtf8("<a href=\"ping\">پینگ: %1 میلی‌ثانیه</a>").arg(latencyMs)
        : QString::fromUtf8("<a href=\"ping\">پینگ: نامشخص</a>"));
    manualConnectButton_->setIcon(QIcon(state == ConnectionState::Connected
        ? ":/mobiletina/fab.png" : ":/mobiletina/stop.png"));
    manualConnectButton_->setIconSize(manualConnectButton_->size());
}

QString MobileTinaHomeWidget::readableBytes(qint64 bytes) {
    static const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double value = qMax<qint64>(0, bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < 4) {
        value /= 1024.0;
        ++unit;
    }
    return QString::number(value, 'f', unit == 0 ? 0 : 1) + " " + units[unit];
}

void MobileTinaHomeWidget::setSubscription(const QString &name, qint64 usedBytes, qint64 totalBytes,
                                            qint64 expireEpochSeconds) {
    subscriptionCard_->show();
    subscriptionName_->setText(name);
    if (totalBytes > 0) {
        const int progress = qBound(0, static_cast<int>(std::round(100.0 * usedBytes / totalBytes)), 100);
        subscriptionProgress_->setValue(progress);
        subscriptionProgress_->show();
        subscriptionUsage_->setText(QString::fromUtf8("%1 از %2").arg(readableBytes(usedBytes), readableBytes(totalBytes)));
        subscriptionUsage_->show();
    } else {
        subscriptionProgress_->hide();
        subscriptionUsage_->hide();
    }
    if (expireEpochSeconds > 0) {
        const qint64 seconds = qMax<qint64>(0, expireEpochSeconds - QDateTime::currentSecsSinceEpoch());
        const qint64 days = static_cast<qint64>(std::ceil(seconds / 86400.0));
        subscriptionDays_->setText(QString::fromUtf8("%1 روز باقی‌مانده").arg(days));
        subscriptionDays_->show();
    } else {
        subscriptionDays_->hide();
    }
}

void MobileTinaHomeWidget::clearSubscription() {
    subscriptionCard_->hide();
}
