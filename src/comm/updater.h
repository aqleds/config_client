#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>

class updater : public QObject
{
    Q_OBJECT
public:
    explicit updater(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // UPDATER_H
