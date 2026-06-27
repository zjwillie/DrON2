#pragma once

#include <iostream>

#include "DronNode.hpp"

class DronValue;

struct DronMapIterator {
    DronMap::const_iterator it;
    std::string path;

    std::pair<const std::string&, DronValue> operator*() const;

    DronMapIterator& operator++() {
        ++it;
        return *this;
    }

    bool operator==(const DronMapIterator& other) const { return it == other.it; }
    bool operator!=(const DronMapIterator& other) const { return it != other.it; }
};

struct DronMapView {
    const DronMap* map = nullptr;
    std::string path;

    DronMapIterator begin() const {
        static const DronMap empty{};
        const DronMap& target = map ? *map : empty;
        return DronMapIterator{ target.begin(), path };
    }

    DronMapIterator end() const {
        static const DronMap empty{};
        const DronMap& target = map ? *map : empty;
        return DronMapIterator{ target.end(), path };
    }
};

struct DronListIterator {
    DronList::const_iterator it;
    std::string path;
    size_t index = 0;

    DronValue operator*() const;

    DronListIterator& operator++() {
        ++it;
        ++index;
        return *this;
    }

    bool operator==(const DronListIterator& other) const {
        return it == other.it;
    }

    bool operator!=(const DronListIterator& other) const {
        return it != other.it;
    }
};

struct DronListView {
    const DronList* list = nullptr;
    std::string path;

    DronListIterator begin() const {
        static const DronList empty{};
        const DronList& target = list ? *list : empty;
        return DronListIterator{ target.begin(), path, 0 };
    }

    DronListIterator end() const {
        static const DronList empty{};
        const DronList& target = list ? *list : empty;
        return DronListIterator{ target.end(), path, 0 };
    }
};

class DronValue {
public:
    DronValue(const DronNode* node, std::string path)
        : node_(node), path_(std::move(path)) {
    }

    DronValue operator[](std::string_view key) const;
    DronValue operator[](std::size_t i) const;

    template<class T> T as(T fallback) const;

    bool valid()  const;
    bool isMap()  const;
    bool isList() const;
    template<class T> bool is() const;
    bool contains(std::string_view key) const;

    DronMapView  items()    const;
    DronListView elements() const;

private:
    const DronNode* node_ = nullptr;
    std::string path_;
};

template<class T>
bool DronValue::is() const {
    return valid() && node_->is<T>();
}

template<class T>
T DronValue::as(T fallback) const {
    if (!valid() || !node_->is<T>()) {
        std::cout << "[DrON] couldn't resolve \"" << path_ << "\" - using fallback\n";
        return fallback;
    }
    return node_->as<T>();
}