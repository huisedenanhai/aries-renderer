#include "SoA.h"

namespace ars {
uint64_t IndexPool::alloc() {
    if (!_free_index.has_value()) {
        auto index = static_cast<uint64_t>(_indices.size());
        _indices.emplace_back();
        return index;
    }

    auto index = _free_index.value();
    auto &slot = _indices[index];
    auto next = slot.get_free();

    if (next == index) {
        _free_index = std::nullopt;
    } else {
        _free_index = next;
    }

    slot.set_alloc(0);

    return index;
}

void IndexPool::free(uint64_t index) {
    assert(index < _indices.size());

    auto &slot = _indices[index];

    assert(!slot.is_free());

    if (_free_index.has_value()) {
        slot.set_free(_free_index.value());
    } else {
        slot.set_free(index);
    }

    _free_index = index;
}

void IndexPool::set_value(uint64_t index, uint64_t value) {
    _indices[index].set_alloc(value);
}

uint64_t IndexPool::get_value(uint64_t index) {
    return _indices[index].get_alloc().value();
}

std::optional<uint64_t> IndexPool::IndexSlot::get_alloc() {
    if (!is_free()) {
        return _value;
    }

    return std::nullopt;
}

std::optional<uint64_t> IndexPool::IndexSlot::get_free() const {
    if (is_free()) {
        return _value & (~FREE_BIT);
    }

    return std::nullopt;
}

void IndexPool::IndexSlot::set_alloc(uint64_t v) {
    assert(!(v & FREE_BIT));
    _value = v & (~FREE_BIT);
}

void IndexPool::IndexSlot::set_free(uint64_t v) {
    _value = v | FREE_BIT;
}

bool IndexPool::IndexSlot::is_free() const {
    return FREE_BIT & _value;
}
} // namespace ars