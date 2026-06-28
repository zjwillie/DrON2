#include "DronValue.hpp"

bool DronValue::valid()  const {
	return node_ != nullptr;
}

bool DronValue::isMap() const {
	return valid() && node_->isMap();
}

bool DronValue::isList() const {
	return valid() && node_->isList();
}

bool DronValue::contains(std::string_view key) const {
	return valid() && isMap() && node_->as<DronMap>().contains(std::string(key));
}

DronValue DronValue::operator[](std::string_view key) const {
	if (!valid() || !isMap()) {
		return DronValue{ nullptr, path_ + "." + std::string(key) };
	}
	const DronMap& map = node_->as<DronMap>();
	auto it = map.find(std::string(key));

	if (it == map.end()) {
		return DronValue{ nullptr, path_ + "." + std::string(key) };
	}

	return DronValue{ &it->second, path_ + "." + std::string(key) };
}

DronValue DronValue::operator[](std::size_t integer) const {
	if (!valid() || !isList()) {
		return DronValue{ nullptr, path_ + "[" + std::to_string(integer) + "]" };
	}
	
	const DronList& list = node_->as<DronList>();
	if (integer >= list.size()) {
		return DronValue{ nullptr, path_ + "[" + std::to_string(integer) + "]" };
	}

	return DronValue{ &list[integer], path_ + "[" + std::to_string(integer) + "]"};
}

DronMapView DronValue::items() const {
	if (!valid() || !isMap()) return DronMapView{ nullptr, path_ };
	return DronMapView{ &node_->as<DronMap>(), path_ };
}

DronListView DronValue::elements() const {
	if (!valid() || !isList()) return DronListView{ nullptr, path_ };
	return DronListView{ &node_->as<DronList>(), path_ };
}

DronValue DronListIterator::operator*() const {
	return DronValue{ &(*it), path + "[" + std::to_string(index) + "]" };
}

std::pair<const std::string&, DronValue> DronMapIterator::operator*() const {
	return { it->first, DronValue{ &it->second, path + "." + it->first } };
}