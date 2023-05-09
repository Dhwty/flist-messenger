#ifndef FLIST_JSONHELPER_H
#define FLIST_JSONHELPER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

class FJsonHelper : public QObject {
        Q_OBJECT
    public:
        explicit FJsonHelper(QObject *parent = nullptr);
        QJsonDocument generateJsonNodesFromMap(QMap<QString, QString> input);
        QJsonDocument generateJsonNodeWithKeyValue(QString key, QString value);

    signals:
};

#endif // FLIST_JSONHELPER_H
