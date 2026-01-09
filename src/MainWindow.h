#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include "MidiController.h"
#include "CustomWidgets.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void refreshPorts();
    void onPortSelected(int index);
    void onControlChanged(ControlInterface* sender, int value);

private:
    void loadConfig();
    void loadState();
    void saveState();
    void saveCurrentDeviceState();
    void buildControls(const QJsonArray &layout);
    void createAndAddControl(const QJsonObject &item, int row, int col);

    MidiController midi;
    
    QComboBox *portCombo;
    QSpinBox *channelSpin;
    QGridLayout *grid;
    QWidget *centralWidget;

    // Config data
    QJsonArray devicesConfig;
    QString currentDeviceName;

    // State data: DeviceName -> { ControlID -> Value }
    QJsonObject allDeviceStates;

    // Keep track of active widgets to save state
    std::vector<ControlInterface*> activeControls;
};