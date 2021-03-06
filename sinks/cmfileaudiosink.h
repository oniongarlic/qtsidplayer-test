#ifndef CMFILEAUDIOSINK_H
#define CMFILEAUDIOSINK_H

#include "cmbaseaudiosink.h"

#include <QFile>
#include <QTimer>

class CMFileAudioSink : public CMBaseAudioSink
{
    Q_OBJECT
    Q_PROPERTY(QString file READ file WRITE setFile NOTIFY fileChanged)
public:
    CMFileAudioSink(QObject *parent = 0);

    Q_INVOKABLE bool play();
    Q_INVOKABLE bool stop();
    Q_INVOKABLE bool pause();
    //Q_INVOKABLE bool resume();

    Q_INVOKABLE bool setFile(const QString file);

    QString file() const
    {
        return m_file.fileName();
    }

signals:
    void fileChanged(QString file);

protected slots:
    void readTicker();

protected:
    virtual bool afterOpen();
    virtual bool aboutToClose();

    QFile m_file;

private:
    QTimer m_timer;    
    uint m_readSize;
};

#endif // CMFILEAUDIOSINK_H
