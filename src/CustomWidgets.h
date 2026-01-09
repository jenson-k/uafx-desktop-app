#pragma once
#include <QWidget>
#include <QDial>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <vector>

// Base interface for controls to expose ID and CC
class ControlInterface {
public:
    virtual ~ControlInterface() = default;
    QString id;
    int cc = -1;
    virtual void setValue(int val) = 0;
    virtual int getValue() const = 0;
};

class VolumeKnob : public QWidget, public ControlInterface {
    Q_OBJECT
public:
    VolumeKnob(const QString &id, const QString &name, int cc, QWidget *parent = nullptr);
    void setValue(int val) override;
    int getValue() const override;

signals:
    void controlChanged(ControlInterface* sender, int value);

private:
    QDial *dial;
    QLabel *valueLabel;
};

class ToggleKnob : public QWidget, public ControlInterface {
    Q_OBJECT
public:
    ToggleKnob(const QString &id, const QString &name, const std::vector<QString> &texts, int cc, QWidget *parent = nullptr);
    void setValue(int val) override;
    int getValue() const override;

signals:
    void controlChanged(ControlInterface* sender, int value);

private:
    QDial *dial;
    QLabel *valueLabel;
    std::vector<QString> options;
};

class ToggleButtonControl : public QWidget, public ControlInterface {
    Q_OBJECT
public:
    ToggleButtonControl(const QString &id, const QString &name, bool initialOff, int cc, QWidget *parent = nullptr);
    void setValue(int val) override;
    int getValue() const override;

signals:
    void controlChanged(ControlInterface* sender, int value);

private:
    QPushButton *button;
};