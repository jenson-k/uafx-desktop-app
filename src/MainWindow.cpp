#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDebug>
#include <QFrame>

MainWindow::MainWindow() {
    setWindowTitle("UAFX MIDI Controller");
    resize(640, 320);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Top Bar
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(new QLabel("Device:"));
    
    portCombo = new QComboBox();
    hLayout->addWidget(portCombo);

    QPushButton *refreshBtn = new QPushButton("Refresh");
    hLayout->addWidget(refreshBtn);

    hLayout->addWidget(new QLabel("Channel:"));
    channelSpin = new QSpinBox();
    channelSpin->setRange(1, 16);
    channelSpin->setValue(1);
    hLayout->addWidget(channelSpin);

    mainLayout->addLayout(hLayout);

    // Grid
    grid = new QGridLayout();
    mainLayout->addLayout(grid);

    // Connections
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(portCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onPortSelected);

    loadConfig();
    loadState();
    refreshPorts();
}

MainWindow::~MainWindow() {
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveCurrentDeviceState();
    saveState();
    QMainWindow::closeEvent(event);
}

void MainWindow::loadConfig() {
    QFile file("controls.json");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Config Error", "Failed to load controls.json");
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    devicesConfig = root["device"].toArray();
}

void MainWindow::loadState() {
    QFile file("state.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        allDeviceStates = doc.object();
    }
}

void MainWindow::saveState() {
    QFile file("state.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(allDeviceStates);
        file.write(doc.toJson());
    }
}

void MainWindow::saveCurrentDeviceState() {
    if (currentDeviceName.isEmpty()) return;

    QJsonObject deviceState;
    for (auto *ctrl : activeControls) {
        if (!ctrl->id.isEmpty()) {
            deviceState[ctrl->id] = ctrl->getValue();
        }
    }
    
    if (!deviceState.isEmpty()) {
        allDeviceStates[currentDeviceName] = deviceState;
    }
}

void MainWindow::refreshPorts() {
    portCombo->blockSignals(true);
    portCombo->clear();

    std::vector<std::string> ports = midi.listPorts();
    bool foundAny = false;

    for (size_t i = 0; i < ports.size(); ++i) {
        QString pName = QString::fromStdString(ports[i]);
        // Filter based on config
        for (const auto &devRef : devicesConfig) {
            QJsonObject dev = devRef.toObject();
            if (pName.contains(dev["name"].toString())) {
                portCombo->addItem(pName, static_cast<int>(i));
                foundAny = true;
                break;
            }
        }
    }

    if (!foundAny) {
        portCombo->addItem("(no matching devices)", -1);
    }

    portCombo->blockSignals(false);

    if (portCombo->count() > 0 && foundAny) {
        portCombo->setCurrentIndex(0);
        onPortSelected(0);
    }
}

void MainWindow::onPortSelected(int index) {
    if (index < 0) return;
    int portIdx = portCombo->itemData(index).toInt();
    if (portIdx < 0) return;

    saveCurrentDeviceState();
    saveState();

    midi.openPort(portIdx);

    QString portName = portCombo->currentText();
    for (const auto &devRef : devicesConfig) {
        QJsonObject dev = devRef.toObject();
        if (portName.contains(dev["name"].toString())) {
            currentDeviceName = dev["name"].toString();
            buildControls(dev["layout"].toArray());
            break;
        }
    }
}

void MainWindow::buildControls(const QJsonArray &layout) {
    // Clear existing
    QLayoutItem *child;
    while ((child = grid->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    activeControls.clear();

    QMap<int, int> colCounters; // row -> next col

    for (const auto &rowRef : layout) {
        QJsonObject rowObj = rowRef.toObject();
        int row = rowObj["row"].toInt(0);
        QJsonArray items = rowObj["items"].toArray();

        int displayRow = row * 2;
        if (row > 0 && !colCounters.contains(row)) {
            QFrame *line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            grid->addWidget(line, displayRow - 1, 0, 1, 20);
        }

        int col = colCounters.value(row, 0);
        for (const auto &itemRef : items) {
            createAndAddControl(itemRef.toObject(), displayRow, col);
            col++;
        }
        colCounters[row] = col;
    }
}

void MainWindow::createAndAddControl(const QJsonObject &item, int row, int col) {
    QString type = item["type"].toString();
    QString name = item["name"].toString();
    QString id = item["id"].toString();
    int cc = item["cc"].toInt();

    QWidget *widget = nullptr;
    ControlInterface *ctrlInterface = nullptr;

    if (type == "volume" || type == "knob") {
        auto *w = new VolumeKnob(id, name, cc);
        connect(w, &VolumeKnob::controlChanged, this, &MainWindow::onControlChanged);
        widget = w;
        ctrlInterface = w;
    } else if (type == "toggle" || type == "toggle_knob") {
        std::vector<QString> texts;
        QJsonArray arr = item["texts"].toArray();
        for (const auto &v : arr) texts.push_back(v.toString());
        
        auto *w = new ToggleKnob(id, name, texts, cc);
        connect(w, &ToggleKnob::controlChanged, this, &MainWindow::onControlChanged);
        widget = w;
        ctrlInterface = w;
    } else if (type == "button" || type == "toggle_button") {
        bool initOff = item["initial_off"].toBool(true);
        auto *w = new ToggleButtonControl(id, name, initOff, cc);
        connect(w, &ToggleButtonControl::controlChanged, this, &MainWindow::onControlChanged);
        widget = w;
        ctrlInterface = w;
    }

    if (widget && ctrlInterface) {
        grid->addWidget(widget, row, col, Qt::AlignTop);
        activeControls.push_back(ctrlInterface);

        // Apply saved state
        if (!currentDeviceName.isEmpty() && allDeviceStates.contains(currentDeviceName)) {
            QJsonObject devState = allDeviceStates[currentDeviceName].toObject();
            if (devState.contains(id)) {
                ctrlInterface->setValue(devState[id].toInt());
            }
        }
    }
}

void MainWindow::onControlChanged(ControlInterface* sender, int value) {
    midi.sendCC(channelSpin->value() - 1, sender->cc, value);
}