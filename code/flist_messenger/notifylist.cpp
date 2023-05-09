#include "notifylist.h"

NotifyStringList::NotifyStringList() : contents() {
    _notifier = new NotifyListNotifier();
}

NotifyStringList::NotifyStringList(const QStringList &other) : contents(other) {
    _notifier = new NotifyListNotifier();
}

NotifyStringList::~NotifyStringList() {
    delete _notifier;
}

void NotifyStringList::append(const QString &value) {
    _notifier->notifyBeforeAdd(contents.count(), contents.count());
    contents.append(value);
    _notifier->notifyAdded(contents.count() - 1, contents.count() - 1);
}

int NotifyStringList::removeAll(const QString &value) {
    int count = 0;
    for (int i = contents.count() - 1; i >= 0; i--) {
        if (contents.at(i) == value) {
            _notifier->notifyBeforeRemove(i, i);
            contents.removeAt(i);
            _notifier->notifyRemoved(i, i);
            count++;
        }
    }
    return count;
}

bool NotifyStringList::contains(const QString &value) const {
    return contents.contains(value);
}

bool NotifyStringList::contains(const QString &value, Qt::CaseSensitivity sensitivity) const {
    return contents.contains(value, sensitivity);
}

int NotifyStringList::count() const {
    return contents.count();
}

void NotifyStringList::clear() {
    int last = contents.count() - 1;
    _notifier->notifyBeforeRemove(0, last);
    contents.clear();
    _notifier->notifyRemoved(0, last);
}

QString &NotifyStringList::operator[](const int &index) {
    return contents[index];
}

NotifyListNotifier *NotifyStringList::notifier() const {
    return _notifier;
}

NotifyListNotifier::NotifyListNotifier(QObject *parent) : QObject(parent) {}

void NotifyListNotifier::notifyAdded(const int firstIndex, const int lastIndex) {
    emit added(firstIndex, lastIndex);
}

void NotifyListNotifier::notifyRemoved(const int firstIndex, const int lastIndex) {
    emit removed(firstIndex, lastIndex);
}

void NotifyListNotifier::notifyBeforeAdd(const int firstIndex, const int lastIndex) {
    emit beforeAdd(firstIndex, lastIndex);
}

void NotifyListNotifier::notifyBeforeRemove(const int firstIndex, const int lastIndex) {
    emit beforeRemove(firstIndex, lastIndex);
}
