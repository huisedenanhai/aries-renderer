#include "SoA.h"

namespace ars {
uint64_t IndexPool::alloc() {
    return 0;
}

void IndexPool::free(uint64_t index) {}

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