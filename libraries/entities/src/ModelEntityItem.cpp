//
//  ModelEntityItem.cpp
//  libraries/entities/src
//
//  Created by Brad Hefta-Gaub on 12/4/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <ByteCountCoding.h>
#include <GLMHelpers.h>

#include "EntityTree.h"
#include "EntityTreeElement.h"
#include "ModelEntityItem.h"


EntityItem* ModelEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    //qDebug() << "ModelEntityItem::factory(const EntityItemID& entityItemID, const EntityItemProperties& properties)...";
    return new  ModelEntityItem(entityID, properties);
}

// our non-pure virtual subclass for now...
ModelEntityItem::ModelEntityItem(const EntityItemID& entityItemID, const EntityItemProperties& properties) :
        EntityItem(entityItemID, properties) 
{ 
    //qDebug() << "ModelEntityItem::ModelEntityItem(const EntityItemID& entityItemID, const EntityItemProperties& properties)...";
    _type = EntityTypes::Model;     

    //qDebug() << "ModelEntityItem::ModelEntityItem() properties.getModelURL()=" << properties.getModelURL();
    //qDebug() << "ModelEntityItem::ModelEntityItem() calling setProperties()";
    setProperties(properties);
    //qDebug() << "ModelEntityItem::ModelEntityItem() getModelURL()=" << getModelURL();
    
    _animationFrameIndex = 0.0f;
}

EntityItemProperties ModelEntityItem::getProperties() const {
    //qDebug() << "ModelEntityItem::getProperties()... <<<<<<<<<<<<<<<<  <<<<<<<<<<<<<<<<<<<<<<<<<";

    EntityItemProperties properties = EntityItem::getProperties(); // get the properties from our base class

    properties._color = getXColor();
    properties._modelURL = getModelURL();
    properties._animationURL = getAnimationURL();
    properties._animationIsPlaying = getAnimationIsPlaying();
    properties._animationFrameIndex = getAnimationFrameIndex();
    properties._animationFPS = getAnimationFPS();
    properties._sittingPoints = getSittingPoints(); // sitting support
    properties._colorChanged = false;
    properties._modelURLChanged = false;
    properties._animationURLChanged = false;
    properties._animationIsPlayingChanged = false;
    properties._animationFrameIndexChanged = false;
    properties._animationFPSChanged = false;
    properties._glowLevel = getGlowLevel();
    properties._glowLevelChanged = false;

    //qDebug() << "ModelEntityItem::getProperties() getModelURL()=" << getModelURL();

    return properties;
}

bool ModelEntityItem::setProperties(const EntityItemProperties& properties, bool forceCopy) {
    //qDebug() << "ModelEntityItem::setProperties()...";
    bool somethingChanged = false;
    
    somethingChanged = EntityItem::setProperties(properties, forceCopy); // set the properties in our base class

    if (properties._colorChanged || forceCopy) {
        setColor(properties._color);
        somethingChanged = true;
    }

    if (properties._modelURLChanged || forceCopy) {
        setModelURL(properties._modelURL);
        somethingChanged = true;
    }

    if (properties._animationURLChanged || forceCopy) {
        setAnimationURL(properties._animationURL);
        somethingChanged = true;
    }

    if (properties._animationIsPlayingChanged || forceCopy) {
        setAnimationIsPlaying(properties._animationIsPlaying);
        somethingChanged = true;
    }

    if (properties._animationFrameIndexChanged || forceCopy) {
        setAnimationFrameIndex(properties._animationFrameIndex);
        somethingChanged = true;
    }
    
    if (properties._animationFPSChanged || forceCopy) {
        setAnimationFPS(properties._animationFPS);
        somethingChanged = true;
    }
    
    if (properties._glowLevelChanged || forceCopy) {
        setGlowLevel(properties._glowLevel);
        somethingChanged = true;
    }

    if (somethingChanged) {
        bool wantDebug = false;
        if (wantDebug) {
            uint64_t now = usecTimestampNow();
            int elapsed = now - _lastEdited;
            qDebug() << "ModelEntityItem::setProperties() AFTER update... edited AGO=" << elapsed <<
                    "now=" << now << " _lastEdited=" << _lastEdited;
        }
        setLastEdited(properties._lastEdited);
    }
    
    return somethingChanged;
}



int ModelEntityItem::readEntityDataFromBuffer(const unsigned char* data, int bytesLeftToRead, ReadBitstreamToTreeParams& args) {
    if (args.bitstreamVersion < VERSION_ENTITIES_SUPPORT_SPLIT_MTU) {
        return oldVersionReadEntityDataFromBuffer(data, bytesLeftToRead, args);
    }
    
    // let our base class do most of the work... it will call us back for our porition...
    return EntityItem::readEntityDataFromBuffer(data, bytesLeftToRead, args);
}

int ModelEntityItem::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead, 
                                                ReadBitstreamToTreeParams& args,
                                                EntityPropertyFlags& propertyFlags, bool overwriteLocalData) {
    
    int bytesRead = 0;
    const unsigned char* dataAt = data;

    // PROP_COLOR
    if (propertyFlags.getHasProperty(PROP_COLOR)) {
        rgbColor color;
        if (overwriteLocalData) {
            memcpy(_color, dataAt, sizeof(_color));
        }
        dataAt += sizeof(color);
        bytesRead += sizeof(color);
    }

    // PROP_MODEL_URL
    if (propertyFlags.getHasProperty(PROP_MODEL_URL)) {
        // TODO: fix to new format...
        uint16_t modelURLLength;
        memcpy(&modelURLLength, dataAt, sizeof(modelURLLength));
        dataAt += sizeof(modelURLLength);
        bytesRead += sizeof(modelURLLength);
        QString modelURLString((const char*)dataAt);
        dataAt += modelURLLength;
        bytesRead += modelURLLength;
        if (overwriteLocalData) {
            setModelURL(modelURLString);
        }
    }
    
    // PROP_ANIMATION_URL
    if (propertyFlags.getHasProperty(PROP_ANIMATION_URL)) {
        // animationURL
        uint16_t animationURLLength;
        memcpy(&animationURLLength, dataAt, sizeof(animationURLLength));
        dataAt += sizeof(animationURLLength);
        bytesRead += sizeof(animationURLLength);
        QString animationURLString((const char*)dataAt);
        dataAt += animationURLLength;
        bytesRead += animationURLLength;
        if (overwriteLocalData) {
            setAnimationURL(animationURLString);
        }
    }        

    // PROP_ANIMATION_FPS
    if (propertyFlags.getHasProperty(PROP_ANIMATION_FPS)) {
        float animationFPS;
        memcpy(&animationFPS, dataAt, sizeof(animationFPS));
        dataAt += sizeof(animationFPS);
        bytesRead += sizeof(animationFPS);
        if (overwriteLocalData) {
            _animationFPS = animationFPS;
        }
    }

    // PROP_ANIMATION_FRAME_INDEX
    if (propertyFlags.getHasProperty(PROP_ANIMATION_FRAME_INDEX)) {
        float animationFrameIndex;
        memcpy(&animationFrameIndex, dataAt, sizeof(animationFrameIndex));
        dataAt += sizeof(animationFrameIndex);
        bytesRead += sizeof(animationFrameIndex);
        if (overwriteLocalData) {
            _animationFrameIndex = animationFrameIndex;
        }
    }

    // PROP_ANIMATION_PLAYING
    if (propertyFlags.getHasProperty(PROP_ANIMATION_PLAYING)) {
        bool animationIsPlaying;
        memcpy(&animationIsPlaying, dataAt, sizeof(animationIsPlaying));
        dataAt += sizeof(animationIsPlaying);
        bytesRead += sizeof(animationIsPlaying);
        if (overwriteLocalData) {
            _animationIsPlaying = animationIsPlaying;
        }
    }

    return bytesRead;
}

int ModelEntityItem::oldVersionReadEntityDataFromBuffer(const unsigned char* data, int bytesLeftToRead, ReadBitstreamToTreeParams& args) {

    int bytesRead = 0;
    if (bytesLeftToRead >= expectedBytes()) {
        int clockSkew = args.sourceNode ? args.sourceNode->getClockSkewUsec() : 0;

        const unsigned char* dataAt = data;

        // id
        // this old bitstream format had 32bit IDs. They are obsolete and need to be replaced with our new UUID
        // format. We can simply read and ignore the old ID since they should not be repeated. This code should only
        // run on loading from an old file.
        quint32 oldID;
        memcpy(&oldID, dataAt, sizeof(oldID));
        dataAt += sizeof(oldID);
        bytesRead += sizeof(oldID);
        _id = QUuid::createUuid();
        //qDebug() << "ModelEntityItem::oldVersionReadEntityDataFromBuffer()... oldID=" << oldID << "new _id=" << _id;

        // _lastUpdated
        memcpy(&_lastUpdated, dataAt, sizeof(_lastUpdated));
        dataAt += sizeof(_lastUpdated);
        bytesRead += sizeof(_lastUpdated);
        _lastUpdated -= clockSkew;

        // _lastEdited
        memcpy(&_lastEdited, dataAt, sizeof(_lastEdited));
        dataAt += sizeof(_lastEdited);
        bytesRead += sizeof(_lastEdited);
        _lastEdited -= clockSkew;

        // radius
        memcpy(&_radius, dataAt, sizeof(_radius));
        dataAt += sizeof(_radius);
        bytesRead += sizeof(_radius);

        // position
        memcpy(&_position, dataAt, sizeof(_position));
        dataAt += sizeof(_position);
        bytesRead += sizeof(_position);

        // color
        memcpy(&_color, dataAt, sizeof(_color));
        dataAt += sizeof(_color);
        bytesRead += sizeof(_color);

        // TODO: how to handle this? Presumable, this would only ever be true if the model file was saved with
        // a model being in a shouldBeDeleted state. Which seems unlikely. But if it happens, maybe we should delete the entity after loading?
        // shouldBeDeleted
        bool shouldBeDeleted = false;
        memcpy(&shouldBeDeleted, dataAt, sizeof(shouldBeDeleted));
        dataAt += sizeof(shouldBeDeleted);
        bytesRead += sizeof(shouldBeDeleted);
        if (shouldBeDeleted) {
            qDebug() << "UNEXPECTED - read shouldBeDeleted=TRUE from an old format file";
        }

        // modelURL
        uint16_t modelURLLength;
        memcpy(&modelURLLength, dataAt, sizeof(modelURLLength));
        dataAt += sizeof(modelURLLength);
        bytesRead += sizeof(modelURLLength);
        QString modelURLString((const char*)dataAt);
        setModelURL(modelURLString);
        dataAt += modelURLLength;
        bytesRead += modelURLLength;

        // rotation
        int bytes = unpackOrientationQuatFromBytes(dataAt, _rotation);
        dataAt += bytes;
        bytesRead += bytes;

        if (args.bitstreamVersion >= VERSION_ENTITIES_HAVE_ANIMATION) {
            // animationURL
            uint16_t animationURLLength;
            memcpy(&animationURLLength, dataAt, sizeof(animationURLLength));
            dataAt += sizeof(animationURLLength);
            bytesRead += sizeof(animationURLLength);
            QString animationURLString((const char*)dataAt);
            setAnimationURL(animationURLString);
            dataAt += animationURLLength;
            bytesRead += animationURLLength;

            // animationIsPlaying
            memcpy(&_animationIsPlaying, dataAt, sizeof(_animationIsPlaying));
            dataAt += sizeof(_animationIsPlaying);
            bytesRead += sizeof(_animationIsPlaying);

            // animationFrameIndex
            memcpy(&_animationFrameIndex, dataAt, sizeof(_animationFrameIndex));
            dataAt += sizeof(_animationFrameIndex);
            bytesRead += sizeof(_animationFrameIndex);

            // animationFPS
            memcpy(&_animationFPS, dataAt, sizeof(_animationFPS));
            dataAt += sizeof(_animationFPS);
            bytesRead += sizeof(_animationFPS);
        }
    }
    return bytesRead;
}


// TODO: eventually only include properties changed since the params.lastViewFrustumSent time
EntityPropertyFlags ModelEntityItem::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties = EntityItem::getEntityProperties(params);

    requestedProperties += PROP_MODEL_URL;
    requestedProperties += PROP_ANIMATION_URL;
    requestedProperties += PROP_ANIMATION_FPS;
    requestedProperties += PROP_ANIMATION_FRAME_INDEX;
    requestedProperties += PROP_ANIMATION_PLAYING;
    
    return requestedProperties;
}


void ModelEntityItem::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params, 
                                EntityTreeElementExtraEncodeData* modelTreeElementExtraEncodeData,
                                EntityPropertyFlags& requestedProperties,
                                EntityPropertyFlags& propertyFlags,
                                EntityPropertyFlags& propertiesDidntFit,
                                int& propertyCount, OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;

    // PROP_COLOR
    if (requestedProperties.getHasProperty(PROP_COLOR)) {
        LevelDetails propertyLevel = packetData->startLevel();
        successPropertyFits = packetData->appendColor(getColor());
        if (successPropertyFits) {
            propertyFlags |= PROP_COLOR;
            propertiesDidntFit -= PROP_COLOR;
            propertyCount++;
            packetData->endLevel(propertyLevel);
        } else {
            packetData->discardLevel(propertyLevel);
            appendState = OctreeElement::PARTIAL;
        }
    } else {
        propertiesDidntFit -= PROP_COLOR;
    }

    // PROP_MODEL_URL
    if (requestedProperties.getHasProperty(PROP_MODEL_URL)) {
        LevelDetails propertyLevel = packetData->startLevel();
        successPropertyFits = packetData->appendValue(getModelURL());
        if (successPropertyFits) {
            propertyFlags |= PROP_MODEL_URL;
            propertiesDidntFit -= PROP_MODEL_URL;
            propertyCount++;
            packetData->endLevel(propertyLevel);
        } else {
            packetData->discardLevel(propertyLevel);
            appendState = OctreeElement::PARTIAL;
        }
    } else {
        propertiesDidntFit -= PROP_MODEL_URL;
    }

    //qDebug() << "ModelEntityItem::appendEntityData()... modelURL=" << getModelURL();

    // PROP_ANIMATION_URL
    if (requestedProperties.getHasProperty(PROP_ANIMATION_URL)) {
        LevelDetails propertyLevel = packetData->startLevel();
        successPropertyFits = packetData->appendValue(getAnimationURL());
        if (successPropertyFits) {
            propertyFlags |= PROP_ANIMATION_URL;
            propertiesDidntFit -= PROP_ANIMATION_URL;
            propertyCount++;
            packetData->endLevel(propertyLevel);
        } else {
            packetData->discardLevel(propertyLevel);
            appendState = OctreeElement::PARTIAL;
        }
    } else {
        propertiesDidntFit -= PROP_ANIMATION_URL;
    }

    // PROP_ANIMATION_FPS
    if (requestedProperties.getHasProperty(PROP_ANIMATION_FPS)) {
        LevelDetails propertyLevel = packetData->startLevel();
        successPropertyFits = packetData->appendValue(getAnimationFPS());
        if (successPropertyFits) {
            propertyFlags |= PROP_ANIMATION_FPS;
            propertiesDidntFit -= PROP_ANIMATION_FPS;
            propertyCount++;
            packetData->endLevel(propertyLevel);
        } else {
            packetData->discardLevel(propertyLevel);
            appendState = OctreeElement::PARTIAL;
        }
    } else {
        propertiesDidntFit -= PROP_ANIMATION_FPS;
    }

    // PROP_ANIMATION_FRAME_INDEX
    if (requestedProperties.getHasProperty(PROP_ANIMATION_FRAME_INDEX)) {
        LevelDetails propertyLevel = packetData->startLevel();
        successPropertyFits = packetData->appendValue(getAnimationFrameIndex());
        if (successPropertyFits) {
            propertyFlags |= PROP_ANIMATION_FRAME_INDEX;
            propertiesDidntFit -= PROP_ANIMATION_FRAME_INDEX;
            propertyCount++;
            packetData->endLevel(propertyLevel);
        } else {
            packetData->discardLevel(propertyLevel);
            appendState = OctreeElement::PARTIAL;
        }
    } else {
        propertiesDidntFit -= PROP_ANIMATION_FRAME_INDEX;
    }

    // PROP_ANIMATION_PLAYING
    if (requestedProperties.getHasProperty(PROP_ANIMATION_PLAYING)) {
        LevelDetails propertyLevel = packetData->startLevel();
        successPropertyFits = packetData->appendValue(getAnimationIsPlaying());
        if (successPropertyFits) {
            propertyFlags |= PROP_ANIMATION_PLAYING;
            propertiesDidntFit -= PROP_ANIMATION_PLAYING;
            propertyCount++;
            packetData->endLevel(propertyLevel);
        } else {
            packetData->discardLevel(propertyLevel);
            appendState = OctreeElement::PARTIAL;
        }
    } else {
        propertiesDidntFit -= PROP_ANIMATION_PLAYING;
    }

}


QMap<QString, AnimationPointer> ModelEntityItem::_loadedAnimations; // TODO: improve cleanup by leveraging the AnimationPointer(s)
AnimationCache ModelEntityItem::_animationCache;

// This class/instance will cleanup the animations once unloaded.
class EntityAnimationsBookkeeper {
public:
    ~EntityAnimationsBookkeeper() {
        ModelEntityItem::cleanupLoadedAnimations();
    }
};

EntityAnimationsBookkeeper modelAnimationsBookkeeperInstance;

void ModelEntityItem::cleanupLoadedAnimations() {
    foreach(AnimationPointer animation, _loadedAnimations) {
        animation.clear();
    }
    _loadedAnimations.clear();
}

Animation* ModelEntityItem::getAnimation(const QString& url) {
    AnimationPointer animation;
    
    // if we don't already have this model then create it and initialize it
    if (_loadedAnimations.find(url) == _loadedAnimations.end()) {
        animation = _animationCache.getAnimation(url);
        _loadedAnimations[url] = animation;
    } else {
        animation = _loadedAnimations[url];
    }
    return animation.data();
}

void ModelEntityItem::mapJoints(const QStringList& modelJointNames) {
    // if we don't have animation, or we're already joint mapped then bail early
    if (!hasAnimation() || _jointMappingCompleted) {
        return;
    }

    Animation* myAnimation = getAnimation(_animationURL);
    
    if (!_jointMappingCompleted) {
        QStringList animationJointNames = myAnimation->getJointNames();

        if (modelJointNames.size() > 0 && animationJointNames.size() > 0) {
            _jointMapping.resize(modelJointNames.size());
            for (int i = 0; i < modelJointNames.size(); i++) {
                _jointMapping[i] = animationJointNames.indexOf(modelJointNames[i]);
            }
            _jointMappingCompleted = true;
        }
    }
}

QVector<glm::quat> ModelEntityItem::getAnimationFrame() {
    bool wantDebug = false;
    QVector<glm::quat> frameData;
    if (hasAnimation() && _jointMappingCompleted) {
        Animation* myAnimation = getAnimation(_animationURL);
        QVector<FBXAnimationFrame> frames = myAnimation->getFrames();
        int frameCount = frames.size();

        if (frameCount > 0) {
            int animationFrameIndex = (int)glm::floor(_animationFrameIndex) % frameCount;

            if (animationFrameIndex < 0 || animationFrameIndex > frameCount) {
                if (wantDebug) {
                    qDebug() << "ModelEntityItem::getAnimationFrame()....";
                    qDebug() << "   frame index out of bounds....";
                    qDebug() << "   _animationFrameIndex=" << _animationFrameIndex;
                    qDebug() << "   frameCount=" << frameCount;
                    qDebug() << "   animationFrameIndex=" << animationFrameIndex;
                }
                animationFrameIndex = 0;
            }
            
            //qDebug() << "ModelEntityItem::getAnimationFrame().... _animationFrameIndex=" << _animationFrameIndex << "frameCount=" << frameCount << "animationFrameIndex=" << animationFrameIndex;

            QVector<glm::quat> rotations = frames[animationFrameIndex].rotations;
            frameData.resize(_jointMapping.size());
            for (int j = 0; j < _jointMapping.size(); j++) {
                int rotationIndex = _jointMapping[j];
                if (rotationIndex != -1 && rotationIndex < rotations.size()) {
                    frameData[j] = rotations[rotationIndex];
                }
            }
        }
    }
    return frameData;
}

bool ModelEntityItem::isAnimatingSomething() const { 
    return getAnimationIsPlaying() && 
            getAnimationFPS() != 0.0f &&
            !getAnimationURL().isEmpty();
}

EntityItem::SimuationState ModelEntityItem::getSimulationState() const {
    EntityItem::SimuationState baseClassState = EntityItem::getSimulationState();
    
    // if the base class is static, then consider our animation state, and upgrade to changing if
    // we are animating. If the base class has a higher simulation state than static, then
    // use the base class state.
    if (baseClassState == EntityItem::Static) {
        if (isAnimatingSomething()) {
            return EntityItem::Changing;
        }
    }
    return baseClassState;
}

void ModelEntityItem::update(const quint64& updateTime) {
    //_lastUpdated = updateTime;
    //setShouldBeDeleted(getShouldBeDeleted());

    EntityItem::update(updateTime); // let our base class handle it's updates...

    quint64 now = updateTime; //usecTimestampNow();
    
    //qDebug() << "ModelEntityItem::update() getAnimationIsPlaying()="<< getAnimationIsPlaying();

    // only advance the frame index if we're playing
    if (getAnimationIsPlaying()) {

        float deltaTime = (float)(now - _lastAnimated) / (float)USECS_PER_SECOND;
        
        const bool wantDebugging = false;
        if (wantDebugging) {
            qDebug() << "EntityItem::update() now=" << now;
            qDebug() << "             updateTime=" << updateTime;
            qDebug() << "          _lastAnimated=" << _lastAnimated;
            qDebug() << "              deltaTime=" << deltaTime;
        }
        _lastAnimated = now;
        _animationFrameIndex += deltaTime * _animationFPS;

        if (wantDebugging) {
            qDebug() << "   _animationFrameIndex=" << _animationFrameIndex;
        }

    } else {
        _lastAnimated = now;
    }
}

