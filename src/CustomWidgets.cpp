#include "CustomWidgets.h"
#include <QVBoxLayout>
#include <QGridLayout>

// --- VolumeKnob ---
VolumeKnob::VolumeKnob(const QString &id, const QString &name, int cc, QWidget *parent)
    : QWidget(parent) {
    this->id = id;
    this->cc = cc;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(name));

    dial = new QDial();
    dial->setRange(0, 127);
    dial->setNotchesVisible(true);

    valueLabel = new QLabel("0.0");
    valueLabel->setAlignment(Qt::AlignHCenter);

    // Overlay layout for name inside dial (simplified for C++)
    layout->addWidget(dial);
    layout->addWidget(valueLabel);
    setLayout(layout);

    connect(dial, &QDial::valueChanged, this, [this](int val) {
        double mapped = val * 10.0 / 127.0;
        valueLabel->setText(QString::number(mapped, 'f', 1));
        emit controlChanged(this, val);
    });
}

void VolumeKnob::setValue(int val) {
    dial->setValue(val);
}

int VolumeKnob::getValue() const {
    return dial->value();
}

// --- ToggleKnob ---
ToggleKnob::ToggleKnob(const QString &id, const QString &name, const std::vector<QString> &texts, int cc, QWidget *parent)
    : QWidget(parent), options(texts) {
    this->id = id;
    this->cc = cc;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(name));

    dial = new QDial();
    dial->setRange(0, static_cast<int>(texts.size()) - 1);
    dial->setNotchesVisible(true);

    valueLabel = new QLabel(texts.empty() ? "" : texts[0]);
    valueLabel->setAlignment(Qt::AlignHCenter);

    layout->addWidget(dial);
    layout->addWidget(valueLabel);
    setLayout(layout);

    connect(dial, &QDial::valueChanged, this, [this](int val) {
        if (val >= 0 && val < static_cast<int>(options.size())) {
            valueLabel->setText(options[val]);
        }
        emit controlChanged(this, val);
    });
}

void ToggleKnob::setValue(int val) {
    dial->setValue(val);
}

int ToggleKnob::getValue() const {
    return dial->value();
}

// --- ToggleButtonControl ---
ToggleButtonControl::ToggleButtonControl(const QString &id, const QString &name, bool initialOff, int cc, QWidget *parent)
    : QWidget(parent) {
    this->id = id;
    this->cc = cc;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    
    QLabel *lbl = new QLabel(name);
    lbl->setAlignment(Qt::AlignHCenter);
    layout->addWidget(lbl);

    button = new QPushButton(initialOff ? "OFF" : "ON");
    button->setCheckable(true);
    button->setChecked(!initialOff);
    button->setFixedHeight(28);
    button->setFixedWidth(40);

    layout->addWidget(button, 0, Qt::AlignHCenter);
    setLayout(layout);

    connect(button, &QPushButton::toggled, this, [this](bool checked) {
        button->setText(checked ? "ON" : "OFF");
        emit controlChanged(this, checked ? 1 : 0);
    });
}

void ToggleButtonControl::setValue(int val) {
    bool checked = (val != 0);
    button->setChecked(checked);
    button->setText(checked ? "ON" : "OFF");
}

int ToggleButtonControl::getValue() const {
    return button->isChecked() ? 1 : 0;
}