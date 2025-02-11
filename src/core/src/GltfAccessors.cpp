#include "cesium/omniverse/GltfAccessors.h"

namespace cesium::omniverse {
PositionsAccessor::PositionsAccessor()
    : _size(0) {}

PositionsAccessor::PositionsAccessor(const CesiumGltf::AccessorView<glm::fvec3>& view)
    : _view(view)
    , _size(static_cast<uint64_t>(view.size())) {}

void PositionsAccessor::fill(const gsl::span<glm::fvec3>& values) const {
    for (uint64_t i = 0; i < _size; i++) {
        values[i] = _view[static_cast<int64_t>(i)];
    }
}

const glm::fvec3& PositionsAccessor::get(uint64_t index) const {
    return _view[static_cast<int64_t>(index)];
}

uint64_t PositionsAccessor::size() const {
    return _size;
}

IndicesAccessor::IndicesAccessor()
    : _size(0) {}

IndicesAccessor::IndicesAccessor(uint64_t size)
    : _size(size) {}

IndicesAccessor::IndicesAccessor(const CesiumGltf::AccessorView<uint8_t>& uint8View)
    : _uint8View(uint8View)
    , _size(static_cast<uint64_t>(uint8View.size())) {}

IndicesAccessor::IndicesAccessor(const CesiumGltf::AccessorView<uint16_t>& uint16View)
    : _uint16View(uint16View)
    , _size(static_cast<uint64_t>(uint16View.size())) {}

IndicesAccessor::IndicesAccessor(const CesiumGltf::AccessorView<uint32_t>& uint32View)
    : _uint32View(uint32View)
    , _size(static_cast<uint64_t>(uint32View.size())) {}

template <typename T> IndicesAccessor IndicesAccessor::FromTriangleStrips(const CesiumGltf::AccessorView<T>& view) {
    auto indices = std::vector<uint32_t>();
    indices.reserve(static_cast<uint64_t>(view.size() - 2) * 3);

    for (auto i = 0; i < view.size() - 2; i++) {
        if (i % 2) {
            indices.push_back(static_cast<uint32_t>(view[i]));
            indices.push_back(static_cast<uint32_t>(view[i + 2]));
            indices.push_back(static_cast<uint32_t>(view[i + 1]));
        } else {
            indices.push_back(static_cast<uint32_t>(view[i]));
            indices.push_back(static_cast<uint32_t>(view[i + 1]));
            indices.push_back(static_cast<uint32_t>(view[i + 2]));
        }
    }

    auto accessor = IndicesAccessor();
    accessor._size = indices.size();
    accessor._computed = std::move(indices);
    return accessor;
}

// Explicit template instantiation
template IndicesAccessor IndicesAccessor::FromTriangleStrips<uint8_t>(const CesiumGltf::AccessorView<uint8_t>& view);
template IndicesAccessor IndicesAccessor::FromTriangleStrips<uint16_t>(const CesiumGltf::AccessorView<uint16_t>& view);
template IndicesAccessor IndicesAccessor::FromTriangleStrips<uint32_t>(const CesiumGltf::AccessorView<uint32_t>& view);

template <typename T> IndicesAccessor IndicesAccessor::FromTriangleFans(const CesiumGltf::AccessorView<T>& view) {
    auto indices = std::vector<uint32_t>();
    indices.reserve(static_cast<uint64_t>(view.size() - 2) * 3);

    for (auto i = 0; i < view.size() - 2; i++) {
        indices.push_back(static_cast<uint32_t>(view[0]));
        indices.push_back(static_cast<uint32_t>(view[i + 1]));
        indices.push_back(static_cast<uint32_t>(view[i + 2]));
    }

    auto accessor = IndicesAccessor();
    accessor._size = indices.size();
    accessor._computed = std::move(indices);
    return accessor;
}

// Explicit template instantiation
template IndicesAccessor IndicesAccessor::FromTriangleFans<uint8_t>(const CesiumGltf::AccessorView<uint8_t>& view);
template IndicesAccessor IndicesAccessor::FromTriangleFans<uint16_t>(const CesiumGltf::AccessorView<uint16_t>& view);
template IndicesAccessor IndicesAccessor::FromTriangleFans<uint32_t>(const CesiumGltf::AccessorView<uint32_t>& view);

void IndicesAccessor::fill(const gsl::span<int>& values) const {
    const auto size = values.size();
    assert(size == _size);

    if (!_computed.empty()) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = static_cast<int>(_computed[i]);
        }
    } else if (_uint8View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = static_cast<int>(_uint8View[static_cast<int64_t>(i)]);
        }
    } else if (_uint16View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = static_cast<int>(_uint16View[static_cast<int64_t>(i)]);
        }
    } else if (_uint32View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = static_cast<int>(_uint32View[static_cast<int64_t>(i)]);
        }
    } else {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = static_cast<int>(i);
        }
    }
}

uint32_t IndicesAccessor::get(uint64_t index) const {
    if (!_computed.empty()) {
        return _computed[index];
    } else if (_uint8View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        return static_cast<uint32_t>(_uint8View[static_cast<int64_t>(index)]);
    } else if (_uint16View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        return static_cast<uint32_t>(_uint16View[static_cast<int64_t>(index)]);
    } else if (_uint32View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        return static_cast<uint32_t>(_uint32View[static_cast<int64_t>(index)]);
    } else {
        return static_cast<uint32_t>(index);
    }
}

uint64_t IndicesAccessor::size() const {
    return _size;
}

NormalsAccessor::NormalsAccessor()
    : _size(0) {}

NormalsAccessor::NormalsAccessor(const CesiumGltf::AccessorView<glm::fvec3>& view)
    : _view(view)
    , _size(static_cast<uint64_t>(view.size())) {}

NormalsAccessor NormalsAccessor::GenerateSmooth(const PositionsAccessor& positions, const IndicesAccessor& indices) {
    auto normals = std::vector<glm::fvec3>(positions.size(), glm::fvec3(0.0f));

    for (uint64_t i = 0; i < indices.size(); i += 3) {
        auto idx0 = static_cast<uint64_t>(indices.get(i));
        auto idx1 = static_cast<uint64_t>(indices.get(i + 1));
        auto idx2 = static_cast<uint64_t>(indices.get(i + 2));

        const auto& p0 = positions.get(idx0);
        const auto& p1 = positions.get(idx1);
        const auto& p2 = positions.get(idx2);
        auto n = glm::normalize(glm::cross(p1 - p0, p2 - p0));

        normals[idx0] += n;
        normals[idx1] += n;
        normals[idx2] += n;
    }

    for (auto& n : normals) {
        n = glm::normalize(n);
    }

    auto accessor = NormalsAccessor();
    accessor._computed = std::move(normals);
    accessor._size = indices.size();
    return accessor;
}

void NormalsAccessor::fill(const gsl::span<glm::fvec3>& values) const {
    const auto size = values.size();
    assert(size == _size);

    if (!_computed.empty()) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = _computed[i];
        }
    } else {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = _view[static_cast<int64_t>(i)];
        }
    }
}

uint64_t NormalsAccessor::size() const {
    return _size;
}

TexcoordsAccessor::TexcoordsAccessor()
    : _size(0) {}

TexcoordsAccessor::TexcoordsAccessor(const CesiumGltf::AccessorView<glm::fvec2>& view, bool flipVertical)
    : _view(view)
    , _flipVertical(flipVertical)
    , _size(static_cast<uint64_t>(view.size())) {}

void TexcoordsAccessor::fill(const gsl::span<glm::fvec2>& values) const {
    const auto size = values.size();
    assert(size == _size);

    for (uint64_t i = 0; i < size; i++) {
        values[i] = _view[static_cast<int64_t>(i)];
    }

    if (_flipVertical) {
        for (uint64_t i = 0; i < size; i++) {
            values[i][1] = 1.0f - values[i][1];
        }
    }
}

uint64_t TexcoordsAccessor::size() const {
    return _size;
}

VertexColorsAccessor::VertexColorsAccessor()
    : _size(0) {}

VertexColorsAccessor::VertexColorsAccessor(const CesiumGltf::AccessorView<glm::u8vec3>& uint8Vec3View)
    : _uint8Vec3View(uint8Vec3View)
    , _size(static_cast<uint64_t>(uint8Vec3View.size())) {}

VertexColorsAccessor::VertexColorsAccessor(const CesiumGltf::AccessorView<glm::u8vec4>& uint8Vec4View)
    : _uint8Vec4View(uint8Vec4View)
    , _size(static_cast<uint64_t>(uint8Vec4View.size())) {}

VertexColorsAccessor::VertexColorsAccessor(const CesiumGltf::AccessorView<glm::u16vec3>& uint16Vec3View)
    : _uint16Vec3View(uint16Vec3View)
    , _size(static_cast<uint64_t>(uint16Vec3View.size())) {}

VertexColorsAccessor::VertexColorsAccessor(const CesiumGltf::AccessorView<glm::u16vec4>& uint16Vec4View)
    : _uint16Vec4View(uint16Vec4View)
    , _size(static_cast<uint64_t>(uint16Vec4View.size())) {}

VertexColorsAccessor::VertexColorsAccessor(const CesiumGltf::AccessorView<glm::fvec3>& float32Vec3View)
    : _float32Vec3View(float32Vec3View)
    , _size(static_cast<uint64_t>(float32Vec3View.size())) {}

VertexColorsAccessor::VertexColorsAccessor(const CesiumGltf::AccessorView<glm::fvec4>& float32Vec4View)
    : _float32Vec4View(float32Vec4View)
    , _size(static_cast<uint64_t>(float32Vec4View.size())) {}

void VertexColorsAccessor::fill(const gsl::span<glm::fvec4>& values, uint64_t repeat) const {
    constexpr auto MAX_UINT8 = static_cast<float>(std::numeric_limits<uint8_t>::max());
    constexpr auto MAX_UINT16 = static_cast<float>(std::numeric_limits<uint16_t>::max());

    const auto size = values.size();
    assert(size == _size * repeat);

    if (_uint8Vec3View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = glm::fvec4(
                static_cast<float>(_uint8Vec3View[static_cast<int64_t>(i / repeat)].x) / MAX_UINT8,
                static_cast<float>(_uint8Vec3View[static_cast<int64_t>(i / repeat)].y) / MAX_UINT8,
                static_cast<float>(_uint8Vec3View[static_cast<int64_t>(i / repeat)].z) / MAX_UINT8,
                1.0);
        }
    } else if (_uint8Vec4View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = glm::fvec4(
                static_cast<float>(_uint8Vec4View[static_cast<int64_t>(i / repeat)].x) / MAX_UINT8,
                static_cast<float>(_uint8Vec4View[static_cast<int64_t>(i / repeat)].y) / MAX_UINT8,
                static_cast<float>(_uint8Vec4View[static_cast<int64_t>(i / repeat)].z) / MAX_UINT8,
                static_cast<float>(_uint8Vec4View[static_cast<int64_t>(i / repeat)].w) / MAX_UINT8);
        }
    } else if (_uint16Vec3View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = glm::fvec4(
                static_cast<float>(_uint16Vec3View[static_cast<int64_t>(i / repeat)].x) / MAX_UINT16,
                static_cast<float>(_uint16Vec3View[static_cast<int64_t>(i / repeat)].y) / MAX_UINT16,
                static_cast<float>(_uint16Vec3View[static_cast<int64_t>(i / repeat)].z) / MAX_UINT16,
                1.0);
        }
    } else if (_uint16Vec4View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = glm::fvec4(
                static_cast<float>(_uint16Vec4View[static_cast<int64_t>(i / repeat)].x) / MAX_UINT16,
                static_cast<float>(_uint16Vec4View[static_cast<int64_t>(i / repeat)].y) / MAX_UINT16,
                static_cast<float>(_uint16Vec4View[static_cast<int64_t>(i / repeat)].z) / MAX_UINT16,
                static_cast<float>(_uint16Vec4View[static_cast<int64_t>(i / repeat)].w) / MAX_UINT16);
        }
    } else if (_float32Vec3View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = glm::fvec4(_float32Vec3View[static_cast<int64_t>(i / repeat)], 1.0f);
        }

    } else if (_float32Vec4View.status() == CesiumGltf::AccessorViewStatus::Valid) {
        for (uint64_t i = 0; i < size; i++) {
            values[i] = _float32Vec4View[static_cast<int64_t>(i / repeat)];
        }
    }
}

uint64_t VertexColorsAccessor::size() const {
    return _size;
}

VertexIdsAccessor::VertexIdsAccessor()
    : _size(0) {}

VertexIdsAccessor::VertexIdsAccessor(uint64_t size)
    : _size(size) {}

void VertexIdsAccessor::fill(const gsl::span<float>& values, uint64_t repeat) const {
    const auto size = values.size();
    assert(size == _size * repeat);

    for (uint64_t i = 0; i < size; i++) {
        values[i] = static_cast<float>(i / repeat);
    }
}

uint64_t VertexIdsAccessor::size() const {
    return _size;
}

FaceVertexCountsAccessor::FaceVertexCountsAccessor()
    : _size(0) {}

FaceVertexCountsAccessor::FaceVertexCountsAccessor(uint64_t size)
    : _size(size) {}

void FaceVertexCountsAccessor::fill(const gsl::span<int>& values) const {
    const auto size = values.size();
    assert(size == _size);

    for (uint64_t i = 0; i < size; i++) {
        values[i] = 3;
    }
}

uint64_t FaceVertexCountsAccessor::size() const {
    return _size;
}

} // namespace cesium::omniverse
