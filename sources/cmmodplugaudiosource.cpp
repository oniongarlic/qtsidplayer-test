#include <QDebug>

#include "cmmodplugaudiosource.h"

CMModPlugAudioSource::CMModPlugAudioSource(QObject *parent) :
    CMBaseAudioSource(parent)
{
    ModPlug_GetSettings(&settings);
    settings.mFlags = MODPLUG_ENABLE_OVERSAMPLING;
    settings.mResamplingMode = MODPLUG_RESAMPLE_FIR; /* RESAMP */
    settings.mChannels = m_channels;
    settings.mBits = 16;
    settings.mFrequency = m_rate;
    /* insert more setting changes here */
    ModPlug_SetSettings(&settings);

    m_modplug=NULL;
}

bool CMModPlugAudioSource::generateData(qint64 maxlen)
{
    int r;

    qint64 length = maxlen;
    m_buffer.resize(length);

    r=ModPlug_Read(m_modplug, m_buffer.data(), length);

    if (r==0) {
        // XXX: We loop for now
        emit eot();
        ModPlug_Seek(m_modplug, 0);
    } else {
        // m_pos+=(float)r/(float)(m_rate*2*2);
    }

    return r>0 ? true : false;
}

qint64 CMModPlugAudioSource::writeData(const char *data, qint64 len)
{
    m_tune.append(data, len);
    qDebug() << "Wrote: " << len;
    return len;
}

bool CMModPlugAudioSource::open(QIODevice::OpenMode mode)
{
    bool r=false;

    switch (mode) {
    case QIODevice::ReadOnly:
        if (m_tune.isEmpty())
            return false;
        m_modplug = ModPlug_Load(m_tune.data(), m_tune.size());
        if (m_modplug) {
            QIODevice::open(mode);
            r=true;
            m_pos=0;
            setTracks(1);
            setTrack(1);
            m_meta.clear();
            m_meta.insert("title", ModPlug_GetName(m_modplug));
            m_meta.insert("comment", ModPlug_GetMessage(m_modplug));
            m_meta.insert("channels", ModPlug_NumChannels(m_modplug));
            m_meta.insert("instruments", ModPlug_NumInstruments(m_modplug));
            m_meta.insert("patterns", ModPlug_NumPatterns(m_modplug));
            m_meta.insert("samples", ModPlug_NumSamples(m_modplug));
            m_meta.insert("tracks", m_tracks);

            qDebug() << m_meta;

            emit metaChanged(m_meta);
        } else {
            qWarning("Failed to load mod file, invalid ?");
            r=false;
        }
        break;
    case QIODevice::WriteOnly:
        m_tune.clear();
        r=true;
        QIODevice::open(mode);
        break;
    case QIODevice::ReadWrite:
        qWarning("ReadWrite is not supported");
        r=false;
        break;
    }

    return r;
}

QStringList CMModPlugAudioSource::extensions()
{
    QStringList e;

    e << "*.mod" << "*.xm" << "*.it" << "*.s3m" << "*.stm" << "*.669" << "*.amf" << "*.ams" << "*.dbm" << "*.dmf";
    e << "*.dsm" << "*.far" <<"*.mdl" << "*.med" << "*.mtm" <<  "*.okt" << "*.ptm" << "*.ult" << "*.umx" << "*.mt2" << "*.psm";

    return e;
}

void CMModPlugAudioSource::close()
{
    switch (openMode()) {
    case QIODevice::ReadOnly:

        break;
    case QIODevice::WriteOnly:
        setvalid(!m_tune.isEmpty());
        break;
    default:
        break;
    }

    QIODevice::close();
}

bool CMModPlugAudioSource::reset()
{
    if (!m_modplug)
        return false;

    ModPlug_Seek(m_modplug, 0);
    m_pos=0;

    return true;
}