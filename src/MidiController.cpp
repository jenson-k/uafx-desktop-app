#include "MidiController.h"
#include <iostream>

MidiController::MidiController() {
    try {
        midiOut = std::make_unique<RtMidiOut>();
    } catch (RtMidiError &error) {
        error.printMessage();
    }
}

MidiController::~MidiController() {
    if (midiOut && midiOut->isPortOpen()) {
        midiOut->closePort();
    }
}

std::vector<std::string> MidiController::listPorts() {
    std::vector<std::string> ports;
    if (!midiOut) return ports;

    unsigned int nPorts = midiOut->getPortCount();
    for (unsigned int i = 0; i < nPorts; i++) {
        try {
            ports.push_back(midiOut->getPortName(i));
        } catch (RtMidiError &error) {
            error.printMessage();
        }
    }
    return ports;
}

void MidiController::openPort(unsigned int index) {
    if (!midiOut) return;
    midiOut->closePort();
    midiOut->openPort(index);
}

void MidiController::sendCC(int channel, int cc, int value) {
    if (!midiOut || !midiOut->isPortOpen()) return;

    std::vector<unsigned char> message;
    int ch = std::max(0, std::min(15, channel));
    message.push_back(0xB0 | ch);
    message.push_back(cc & 0x7F);
    message.push_back(value & 0x7F);

    try {
        midiOut->sendMessage(&message);
    } catch (RtMidiError &error) {
        // error.printMessage();
    }
}