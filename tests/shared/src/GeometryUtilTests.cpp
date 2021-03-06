//
//  GeometryUtilTests.cpp
//  tests/physics/src
//
//  Created by Andrew Meadows on 2015.07.27
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <iostream>

#include "GeometryUtilTests.h"

#include <GeometryUtil.h>
#include <NumericalConstants.h>
#include <StreamUtils.h>

#include <../QTestExtensions.h>


QTEST_MAIN(GeometryUtilTests)


void GeometryUtilTests::testLocalRayRectangleIntersection() {
    glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis(0.0f, 0.0f, 1.0f);

    // create a rectangle in the local frame on the XY plane with normal along -zAxis
    // (this is the assumed local orientation of the rectangle in findRayRectangleIntersection())
    glm::vec2 rectDimensions(3.0f, 5.0f);
    glm::vec3 rectCenter(0.0f, 0.0f, 0.0f);
    glm::quat rectRotation = glm::quat(); // identity

    { // verify hit
        glm::vec3 rayStart(1.0f, 2.0f, 3.0f);
        float delta = 0.1f;
        glm::vec3 rayEnd = rectCenter + rectRotation * ((0.5f * rectDimensions.x - delta) * xAxis);
        glm::vec3 rayHitDirection = glm::normalize(rayEnd - rayStart);
        float expectedDistance = glm::length(rayEnd - rayStart);

        float distance = FLT_MAX;
        bool hit = findRayRectangleIntersection(rayStart, rayHitDirection, rectRotation, rectCenter, rectDimensions, distance);
        QCOMPARE(hit, true);
        QCOMPARE_WITH_ABS_ERROR(distance, expectedDistance, EPSILON);
    }

    { // verify miss
        glm::vec3 rayStart(1.0f, 2.0f, 3.0f);
        float delta = 0.1f;
        glm::vec3 rayEnd = rectCenter + rectRotation * ((0.5f * rectDimensions.y + delta) * yAxis);
        glm::vec3 rayMissDirection = glm::normalize(rayEnd - rayStart);
        float distance = FLT_MAX;
        bool hit = findRayRectangleIntersection(rayStart, rayMissDirection, rectRotation, rectCenter, rectDimensions, distance);
        QCOMPARE(hit, false);
        QCOMPARE(distance, FLT_MAX); // distance should be unchanged
    }

    { // hit with co-planer line
        float yFraction = 0.25f;
        glm::vec3 rayStart = rectCenter + rectRotation * (rectDimensions.x * xAxis + yFraction * rectDimensions.y * yAxis);

        glm::vec3 rayEnd = rectCenter - rectRotation * (rectDimensions.x * xAxis - yFraction * rectDimensions.y * yAxis);
        glm::vec3 rayHitDirection = glm::normalize(rayEnd - rayStart);
        float expectedDistance = rectDimensions.x;

        float distance = FLT_MAX;
        bool hit = findRayRectangleIntersection(rayStart, rayHitDirection, rectRotation, rectCenter, rectDimensions, distance);
        QCOMPARE(hit, true);
        QCOMPARE_WITH_ABS_ERROR(distance, expectedDistance, EPSILON);
    }

    { // miss with co-planer line
        float yFraction = 0.75f;
        glm::vec3 rayStart = rectCenter + rectRotation * (rectDimensions.x * xAxis + (yFraction * rectDimensions.y) * yAxis);

        glm::vec3 rayEnd = rectCenter - rectRotation * (rectDimensions.x * xAxis - (yFraction * rectDimensions.y) * yAxis);
        glm::vec3 rayHitDirection = glm::normalize(rayEnd - rayStart);

        float distance = FLT_MAX;
        bool hit = findRayRectangleIntersection(rayStart, rayHitDirection, rectRotation, rectCenter, rectDimensions, distance);
        QCOMPARE(hit, false);
        QCOMPARE(distance, FLT_MAX); // distance should be unchanged
    }
}

void GeometryUtilTests::testWorldRayRectangleIntersection() {
    glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis(0.0f, 0.0f, 1.0f);

    // create a rectangle in the local frame on the XY plane with normal along -zAxis
    // (this is the assumed local orientation of the rectangle in findRayRectangleIntersection())
    // and then rotate and shift everything into the world frame
    glm::vec2 rectDimensions(3.0f, 5.0f);
    glm::vec3 rectCenter(0.7f, 0.5f, -0.3f);
    glm::quat rectRotation = glm::quat(); // identity


    // create points for generating rays that hit the plane and don't
    glm::vec3 rayStart(1.0f, 2.0f, 3.0f);
    float delta = 0.1f;

    { // verify hit
        glm::vec3 rayEnd = rectCenter + rectRotation * ((0.5f * rectDimensions.x - delta) * xAxis);
        glm::vec3 rayHitDirection = glm::normalize(rayEnd - rayStart);
        float expectedDistance = glm::length(rayEnd - rayStart);

        float distance = FLT_MAX;
        bool hit = findRayRectangleIntersection(rayStart, rayHitDirection, rectRotation, rectCenter, rectDimensions, distance);
        QCOMPARE(hit, true);
        QCOMPARE_WITH_ABS_ERROR(distance, expectedDistance, EPSILON);
    }

    { // verify miss
        glm::vec3 rayEnd = rectCenter + rectRotation * ((0.5f * rectDimensions.y + delta) * yAxis);
        glm::vec3 rayMissDirection = glm::normalize(rayEnd - rayStart);
        float distance = FLT_MAX;
        bool hit = findRayRectangleIntersection(rayStart, rayMissDirection, rectRotation, rectCenter, rectDimensions, distance);
        QCOMPARE(hit, false);
        QCOMPARE(distance, FLT_MAX); // distance should be unchanged
    }

    { // hit with co-planer line
        float yFraction = 0.25f;
        rayStart = rectCenter + rectRotation * (rectDimensions.x * xAxis + (yFraction * rectDimensions.y) * yAxis);

        glm::vec3 rayEnd = rectCenter - rectRotation * (rectDimensions.x * xAxis - (yFraction * rectDimensions.y) * yAxis);
        glm::vec3 rayHitDirection = glm::normalize(rayEnd - rayStart);
        float expectedDistance = rectDimensions.x;

        float distance = FLT_MAX;
        bool hit = findRayRectangleIntersection(rayStart, rayHitDirection, rectRotation, rectCenter, rectDimensions, distance);
        QCOMPARE(hit, true);
        QCOMPARE_WITH_ABS_ERROR(distance, expectedDistance, EPSILON);
    }

    { // miss with co-planer line
        float yFraction = 0.75f;
        rayStart = rectCenter + rectRotation * (rectDimensions.x * xAxis + (yFraction * rectDimensions.y) * yAxis);

        glm::vec3 rayEnd = rectCenter - rectRotation * (rectDimensions.x * xAxis - (yFraction * rectDimensions.y) * yAxis);
        glm::vec3 rayHitDirection = glm::normalize(rayEnd - rayStart);

        float distance = FLT_MAX;
        bool hit = findRayRectangleIntersection(rayStart, rayHitDirection, rectRotation, rectCenter, rectDimensions, distance);
        QCOMPARE(hit, false);
        QCOMPARE(distance, FLT_MAX); // distance should be unchanged
    }
}

void GeometryUtilTests::testTwistSwingDecomposition() {
    // for each twist and swing angle pair:
    // (a) compute twist and swing input components
    // (b) compose the total rotation
    // (c) decompose the total rotation
    // (d) compare decomposed values with input components

    glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
    glm::vec3 twistAxis = glm::normalize(glm::vec3(1.0f, 2.0f, 3.0f)); // can be anything but xAxis
    glm::vec3 initialSwingAxis = glm::normalize(glm::cross(xAxis, twistAxis)); // initialSwingAxis must be perp to twistAxis

    const int numTwists = 6;
    const int numSwings = 7;
    const int numSwingAxes = 5;

    const float smallAngle = PI / 100.0f;

    const float maxTwist = PI;
    const float minTwist = -PI;
    const float minSwing = 0.0f;
    const float maxSwing = PI;

    const float deltaTwist = (maxTwist - minTwist - 2.0f * smallAngle) / (float)(numTwists - 1);
    const float deltaSwing = (maxSwing - minSwing - 2.0f * smallAngle) / (float)(numSwings - 1);

    for (float twist = minTwist + smallAngle; twist < maxTwist; twist += deltaTwist) {
        // compute twist component
        glm::quat twistRotation = glm::angleAxis(twist, twistAxis);

        float deltaTheta = TWO_PI / (numSwingAxes - 1);
        for (float theta = 0.0f; theta < TWO_PI; theta += deltaTheta) {
            // compute the swingAxis
            glm::quat thetaRotation = glm::angleAxis(theta, twistAxis);
            glm::vec3 swingAxis = thetaRotation * initialSwingAxis;

            for (float swing = minSwing + smallAngle; swing < maxSwing; swing += deltaSwing) {
                // compute swing component
                glm::quat swingRotation = glm::angleAxis(swing, swingAxis);

                // compose
                glm::quat totalRotation = swingRotation * twistRotation;

                // decompose
                glm::quat measuredTwistRotation;
                glm::quat measuredSwingRotation;
                swingTwistDecomposition(totalRotation, twistAxis, measuredSwingRotation, measuredTwistRotation);
    
                // dot decomposed with components
                float twistDot = fabsf(glm::dot(twistRotation, measuredTwistRotation));
                float swingDot = fabsf(glm::dot(swingRotation, measuredSwingRotation));
    
                // the dot products should be very close to 1.0
                const float MIN_ERROR = 1.0e-6f;
                QCOMPARE_WITH_ABS_ERROR(1.0f, twistDot, MIN_ERROR);
                QCOMPARE_WITH_ABS_ERROR(1.0f, swingDot, MIN_ERROR);
            }
        }
    }
}


