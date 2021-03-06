//
//  RenderablePolyLineEntityItem.cpp
//  libraries/entities-renderer/src/
//
//  Created by Eric Levin on 8/10/15
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <glm/gtx/quaternion.hpp>

#include <GeometryCache.h>
#include <TextureCache.h>
#include <PathUtils.h>
#include <DeferredLightingEffect.h>
#include <PerfStat.h>

#include "RenderablePolyLineEntityItem.h"

#include "paintStroke_vert.h"
#include "paintStroke_frag.h"


EntityItemPointer RenderablePolyLineEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    EntityItemPointer entity{ new RenderablePolyLineEntityItem(entityID) };
    entity->setProperties(properties);
    return entity;
}

RenderablePolyLineEntityItem::RenderablePolyLineEntityItem(const EntityItemID& entityItemID) :
    PolyLineEntityItem(entityItemID) {
    _numVertices = 0;
    _vertices = QVector<glm::vec3>(0.0f);
}

gpu::PipelinePointer RenderablePolyLineEntityItem::_pipeline;
gpu::Stream::FormatPointer RenderablePolyLineEntityItem::_format;
int32_t RenderablePolyLineEntityItem::PAINTSTROKE_GPU_SLOT;

void RenderablePolyLineEntityItem::createPipeline() {
    static const int NORMAL_OFFSET = 12;
    static const int COLOR_OFFSET = 24;
    static const int TEXTURE_OFFSET = 28;

    _format.reset(new gpu::Stream::Format());
    _format->setAttribute(gpu::Stream::POSITION, 0, gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ), 0);
    _format->setAttribute(gpu::Stream::NORMAL, 0, gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ), NORMAL_OFFSET);
    _format->setAttribute(gpu::Stream::COLOR, 0, gpu::Element::COLOR_RGBA_32, COLOR_OFFSET);
    _format->setAttribute(gpu::Stream::TEXCOORD, 0, gpu::Element(gpu::VEC2, gpu::FLOAT, gpu::UV), TEXTURE_OFFSET);

    auto VS = gpu::Shader::createVertex(std::string(paintStroke_vert));
    auto PS = gpu::Shader::createPixel(std::string(paintStroke_frag));
    gpu::ShaderPointer program = gpu::Shader::createProgram(VS, PS);

    gpu::Shader::BindingSet slotBindings;
    PAINTSTROKE_GPU_SLOT = 0;
    slotBindings.insert(gpu::Shader::Binding(std::string("paintStrokeTextureBinding"), PAINTSTROKE_GPU_SLOT));
    gpu::Shader::makeProgram(*program, slotBindings);

    gpu::StatePointer state = gpu::StatePointer(new gpu::State());
    state->setDepthTest(true, true, gpu::LESS_EQUAL);
    state->setBlendFunction(true,
        gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA,
        gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);
    _pipeline = gpu::Pipeline::create(program, state);
}

void RenderablePolyLineEntityItem::updateGeometry() {
    _numVertices = 0;
    _verticesBuffer.reset(new gpu::Buffer());
    int vertexIndex = 0;
    vec2 uv;
    float tailStart = 0.0f;
    float tailEnd = 0.25f;
    float tailLength = tailEnd - tailStart;

    float headStart = 0.76f;
    float headEnd = 1.0f;
    float headLength = headEnd - headStart;
    float uCoord, vCoord;

    int numTailStrips = 5;
    int numHeadStrips = 10;
    int startHeadIndex = _vertices.size() / 2 - numHeadStrips;
    for (int i = 0; i < _vertices.size() / 2; i++) {
        uCoord = 0.26f;
        vCoord = 0.0f;
        //tail
        if (i < numTailStrips) {
            uCoord = float(i) / numTailStrips * tailLength + tailStart;
        }

        //head
        if (i > startHeadIndex) {
            uCoord = float((i + 1) - startHeadIndex) / numHeadStrips * headLength + headStart;
        }

        uv = vec2(uCoord, vCoord);

        _verticesBuffer->append(sizeof(glm::vec3), (const gpu::Byte*)&_vertices.at(vertexIndex));
        _verticesBuffer->append(sizeof(glm::vec3), (const gpu::Byte*)&_normals.at(i));
        _verticesBuffer->append(sizeof(int), (gpu::Byte*)&_color);
        _verticesBuffer->append(sizeof(glm::vec2), (gpu::Byte*)&uv);
        vertexIndex++;

        uv.y = 1.0f;
        _verticesBuffer->append(sizeof(glm::vec3), (const gpu::Byte*)&_vertices.at(vertexIndex));
        _verticesBuffer->append(sizeof(glm::vec3), (const gpu::Byte*)&_normals.at(i));
        _verticesBuffer->append(sizeof(int), (gpu::Byte*)_color);
        _verticesBuffer->append(sizeof(glm::vec2), (const gpu::Byte*)&uv);
        vertexIndex++;

        _numVertices += 2;
    }
    _pointsChanged = false;
    _normalsChanged = false;
    _strokeWidthsChanged = false;
}

void RenderablePolyLineEntityItem::updateVertices() {
    // Calculate the minimum vector size out of normals, points, and stroke widths
    int minVectorSize = _normals.size();
    if (_points.size() < minVectorSize) {
        minVectorSize = _points.size();
    }
    if (_strokeWidths.size() < minVectorSize) {
        minVectorSize = _strokeWidths.size();
    }

    _vertices.clear();
    glm::vec3 v1, v2, tangent, binormal, point;

    int finalIndex = minVectorSize - 1;
    for (int i = 0; i < finalIndex; i++) {
        float width = _strokeWidths.at(i);
        point = _points.at(i);

        tangent = _points.at(i);

        tangent = _points.at(i + 1) - point; 
        glm::vec3 normal = _normals.at(i);
        binormal = glm::normalize(glm::cross(tangent, normal)) * width;

        // Check to make sure binormal is not a NAN. If it is, don't add to vertices vector
        if (binormal.x != binormal.x) {
            continue;
        }
        v1 = point + binormal;
        v2 = point - binormal;
        _vertices << v1 << v2;
    }

    // For last point we can assume binormals are the same since it represents the last two vertices of quad
    point = _points.at(finalIndex);
    v1 = point + binormal;
    v2 = point - binormal;
    _vertices << v1 << v2;


}


void RenderablePolyLineEntityItem::render(RenderArgs* args) {
    QWriteLocker lock(&_quadReadWriteLock);
    if (_points.size() < 2 || _normals.size () < 2 || _strokeWidths.size() < 2) {
        return;
    }

    if (!_pipeline) {
        createPipeline();
    }

    if (!_texture || _texturesChangedFlag) {
        auto textureCache = DependencyManager::get<TextureCache>();
        QString path = _textures.isEmpty() ? PathUtils::resourcesPath() + "images/paintStroke.png" : _textures;
        _texture = textureCache->getTexture(QUrl(path));
        _texturesChangedFlag = false;
    }

    PerformanceTimer perfTimer("RenderablePolyLineEntityItem::render");
    Q_ASSERT(getType() == EntityTypes::PolyLine);

    Q_ASSERT(args->_batch);
    if (_pointsChanged || _strokeWidthsChanged || _normalsChanged) {
        updateVertices();
        updateGeometry();
    }

    gpu::Batch& batch = *args->_batch;
    Transform transform = Transform();
    transform.setTranslation(getPosition());
    transform.setRotation(getRotation());
    batch.setModelTransform(transform);

    batch.setPipeline(_pipeline);
    if (_texture->isLoaded()) {
        batch.setResourceTexture(PAINTSTROKE_GPU_SLOT, _texture->getGPUTexture());
    } else {
        batch.setResourceTexture(PAINTSTROKE_GPU_SLOT, args->_whiteTexture);
    }

    batch.setInputFormat(_format);
    batch.setInputBuffer(0, _verticesBuffer, 0, _format->getChannels().at(0)._stride);

    batch.draw(gpu::TRIANGLE_STRIP, _numVertices, 0);
};
