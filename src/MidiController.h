#pragma once
#include <string>
#include <vector>
#include <memory>

// RtMidi 헤더 경로가 시스템마다 다를 수 있음
#include <rtmidi/RtMidi.h>

class MidiController {
public:
    MidiController();
    ~MidiController();

    std::vector<std::string> listPorts();
    void openPort(unsigned int index);
    void sendCC(int channel, int cc, int value);

private:
    std::unique_ptr<RtMidiOut> midiOut;
};