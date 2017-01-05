#include "cmbaseaudiosink.h"

CMBaseAudioSink::CMBaseAudioSink(QObject *parent)
    : QObject(parent)
    , m_state(QAudio::StoppedState)
{

}

bool CMBaseAudioSink::isValid()
{
    return true;
}

void CMBaseAudioSink::setAudioSource(CMBaseAudioSource *source)
{
    m_source=source;
}

QAudio::State CMBaseAudioSink::state() const
{
    return m_state;
}

void CMBaseAudioSink::setState(QAudio::State state)
{
    if (m_state==state)
        return;
    m_state=state;
    emit stateChanged(m_state);
}
