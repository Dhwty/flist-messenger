#include "flist_jsonhelper.h"

FJsonHelper::FJsonHelper(QObject* parent) : QObject{parent} {}

QJsonDocument FJsonHelper::generateJsonNodesFromMap(QMap<QString, QString> input) {
    QJsonDocument result;
    QJsonObject rv;

    auto cend = input.cend();
    for (auto cit = input.cbegin(); cit != cend; ++cit) {
        rv.insert(cit.key(), cit.value());
    }

    result.setObject(rv);

    return result;
}

QJsonDocument FJsonHelper::generateJsonNodeWithKeyValue(QString key, QString value) {
    QJsonDocument result;
    QJsonObject rv;

    rv.insert(key, value);

    result.setObject(rv);

    return result;
}
