#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;
class QPushButton;
class QStackedWidget;
class QToolButton;

class MobileTinaHomeWidget final : public QWidget {
    Q_OBJECT

public:
    enum class ConnectionState { Disconnected, Testing, Connecting, Connected, Failed, Expired };

    explicit MobileTinaHomeWidget(QWidget *manualContent, QWidget *parent = nullptr);

    void setConnectionState(ConnectionState state, const QString &serverName, int latencyMs);
    void setSubscription(const QString &name, qint64 usedBytes, qint64 totalBytes, qint64 expireEpochSeconds);
    void clearSubscription();
    void selectAutomaticMode();

signals:
    void automaticConnectRequested();
    void manualConnectRequested();
    void pingRequested();

private:
    void selectMode(int index);
    void styleModeButtons();
    static QString readableBytes(qint64 bytes);

    QStackedWidget *pages_ = nullptr;
    QPushButton *manualModeButton_ = nullptr;
    QPushButton *automaticModeButton_ = nullptr;
    QToolButton *automaticConnectButton_ = nullptr;
    QToolButton *manualConnectButton_ = nullptr;
    QLabel *automaticStatus_ = nullptr;
    QLabel *automaticPing_ = nullptr;
    QLabel *automaticServer_ = nullptr;
    QLabel *manualServer_ = nullptr;
    QLabel *manualPing_ = nullptr;
    QWidget *subscriptionCard_ = nullptr;
    QLabel *subscriptionName_ = nullptr;
    QLabel *subscriptionUsage_ = nullptr;
    QLabel *subscriptionDays_ = nullptr;
    QProgressBar *subscriptionProgress_ = nullptr;
};
