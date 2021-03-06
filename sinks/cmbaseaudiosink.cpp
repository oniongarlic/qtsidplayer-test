#include "cmbaseaudiosink.h"

#include <QDebug>

CMBaseAudioSink::CMBaseAudioSink(QObject *parent)
    : QObject(parent)
    , m_state(QAudio::StoppedState)
    , m_valid(true)
    , m_channels(2)
    , m_rate(48000)
{

}

bool CMBaseAudioSink::isValid()
{
    return m_valid;
}

void CMBaseAudioSink::setAudioSource(CMBaseAudioSource *source)
{
    m_source=source;
}

QAudio::State CMBaseAudioSink::state() const
{
    return m_state;
}

uint CMBaseAudioSink::channels() const
{
    return m_channels;
}

uint CMBaseAudioSink::rate() const
{
    return m_rate;
}

void CMBaseAudioSink::setRate(uint rate)
{
    if (m_rate == rate)
        return;

    m_rate = rate;

    qDebug() << "NewRate" << rate;

    emit rateChanged(m_rate);
}

void CMBaseAudioSink::setState(QAudio::State state)
{
    qDebug() << "CMBaseAudioSink::state = " << state;

    if (m_state==state)
        return;

    m_state=state;
    emit stateChanged(m_state);
}
