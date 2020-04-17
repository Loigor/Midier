#pragma once

#include "../layers/layers.h"

namespace midier
{

struct Sequencer
{
    enum class Assist : char
    {
        No,

        // numbers represeting the # of subdivisions to round
        _12 = 12,
        _8 = 8,
        _6 = 6,
        _4 = 4,
        _3 = 3,
    };

    enum class Bar : char
    {
        None = -1,
        Same = 0,

        // bar index
    };

    enum class Run : char
    {
        Sync,
        Async,
    };

    // this class is made to hide the underlying `Layer` from the client
    // so he or she will not call `Layer` methods directly but will call
    // `Sequencer` methods only
    class Handle
    {
        friend class Sequencer;
        Layer * _layer = nullptr;
    };

    // creation
    Sequencer(ILayers layers, unsigned char bpm = 60);

    // queries
    bool recording() const;
    bool looping() const;

    // start and stop layers
    Handle start(Degree degree);
    void stop(Handle handle);

    // revoke the last recorded layer
    // doing nothing if wandering
    void revoke();

    // state changes
    void record(); // toggle between record/playback/overlay modes
    void wander(); // go back to wandering

    // click the next subdivision in a bar and run the logic:
    //   1) manage state changes
    //   2) reset the beat if reached the end of the recorded loop
    //   3) click all layers
    //
    // this can be done either synchronously or asynchronously
    //
    // synchronous calls are blocking and wait for enough time to pass before
    // actually clicking the next subdivision in order for the bar to take
    // the correct amount of time according to `bpm`
    //
    // asynchronous calls are non-blocking and return `Bar::Same` if it's too
    // soon to actually click
    //
    // returns an indicator of any changes in the record loop bar or its index
    // if we are currently inside a record loop
    //
    Bar click(Run run);

    // run synchronously for a certain time duration
    // this method is blocking and returns after the time duration has fully passed
    void run(const Time::Duration & duration);

    // synchronously play a scale degree for a certain time duration
    // the scale degree is stopped at the end of the duration
    void play(Degree degree, const Time::Duration & duration);

    // exposed members
    Assist assist = Assist::No;
    ILayers layers;
    unsigned char bpm;

private:
    enum class State : char
    {
        Wander,
        Prerecord,
        Record,
        Playback,
        Overlay,
    };

    unsigned long long _clicked = -1; // timestamp of previous click

    struct {
        Time when; // when we started to record
        char bars; // # of recorded bars
    } _record;

    Time _started; // first note ever played
    State _state = State::Wander;
    State _previous = _state;
};

} // midier