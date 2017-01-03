#include "cmflacaudiosource.h"

#include <QDebug>

CMFlacAudioSource::CMFlacAudioSource(QObject *parent)
    : CMBaseAudioSource(parent)
{
    init();
}

FLAC__StreamDecoderReadStatus CMFlacAudioSource::read_callback(FLAC__byte buffer[], size_t *bytes)
{
    //qDebug() << "FLAC: Read CB " << *bytes;

    if (m_read_buffer.atEnd())
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;

    m_read_buffer.read((char *)buffer, *bytes);

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderWriteStatus CMFlacAudioSource::write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    const FLAC__uint32 total_size = (FLAC__uint32)(m_total_samples * m_channels * (m_bps/8));
    int i,j;

    //qDebug() << "FLAC: Write CB " << frame->header.blocksize;

    m_buffer.resize(frame->header.blocksize*m_channels*2);
    qint16 *b=(qint16 *)m_buffer.data();

    for (i = 0; i < frame->header.blocksize; i++) {
        for (j = 0; j < m_channels; j++) {
            *b++ = (qint16) buffer[j][i];
        }
    }
}

void CMFlacAudioSource::error_callback(FLAC__StreamDecoderErrorStatus status)
{
    qDebug() << "FLAC: Error CB " << FLAC__StreamDecoderErrorStatusString[status];
}

qint64 CMFlacAudioSource::readData(char *data, qint64 maxlen)
{
    bool r;

    if (m_buffer.isEmpty()) {
        generateData(maxlen);
        m_pos=0;
    } else if (m_pos>=m_buffer.size()) {
        m_buffer.clear();
        generateData(maxlen);
        m_pos=0;
    }

    qint64 s=qMin((qint64)m_buffer.size()-m_pos, maxlen);
    memcpy(data, m_buffer.constData()+m_pos, s);
    m_pos+=s;

    return s;
}

bool CMFlacAudioSource::generateData(qint64 maxlen)
{  
    bool r=process_single();

    if (!r)
        qWarning("Failed to process FLAC data");

    return r;
}

bool CMFlacAudioSource::open(QIODevice::OpenMode mode)
{
    bool r=false;

    switch (mode) {
    case QIODevice::ReadOnly:
        if (m_data.isEmpty()) {
            qWarning("FLAC: Not media loaded");
            return false;
        }

        m_read_buffer.setData(m_data);
        m_read_buffer.open(QIODevice::ReadOnly);

        r=process_until_end_of_metadata();

        if (r) {
            setvalid(true);
            QIODevice::open(mode);
            m_pos=0;
        } else {
            qWarning("FLAC: Failed to process flac data");
            setvalid(false);
        }
        break;
    case QIODevice::WriteOnly:
        m_data.clear();
        QIODevice::open(mode);
        r=true;
        break;
    case QIODevice::ReadWrite:
        qWarning("ReadWrite is not supported");
        return false;
        break;
    }

    return r;
}

void CMFlacAudioSource::close()
{
    switch (openMode()) {
    case QIODevice::ReadOnly:
        m_read_buffer.close();
        break;
    case QIODevice::WriteOnly:
        setvalid(!m_data.isEmpty());
        break;
    default:
        break;
    }

    QIODevice::close();
}

bool CMFlacAudioSource::reset()
{
    FLAC::Decoder::Stream::reset();
}

void CMFlacAudioSource::metadata_callback(const FLAC__StreamMetadata *metadata)
{
    qDebug() << "FLAC: Meta CB " << metadata->type;
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        m_total_samples = metadata->data.stream_info.total_samples;
        m_sample_rate = metadata->data.stream_info.sample_rate;
        m_channels = metadata->data.stream_info.channels;
        m_bps = metadata->data.stream_info.bits_per_sample;

        qDebug() << m_total_samples << m_sample_rate << m_channels << m_bps;
    }
}

QStringList CMFlacAudioSource::extensions()
{
    QStringList e;

    e << "*.flac";

    return e;
}