#pragma once

#include "hitable.h"
#include "utils.h"
#include <iostream>
#include <vector>

bool bboxXCompare(const Hitable* a, const Hitable* b)
{
    math::AABBf bboxLeft, bboxRight;
    if (!a->boundingBox(0.f, 0.f, bboxLeft) || !b->boundingBox(0.f, 0.f, bboxRight))
    {
        std::cerr << "No bounding box in BvhNode constructor\n";
    }
    if (bboxLeft.min().x - bboxRight.min().x < 0.f)
        return true;
    else
        return false;
}

bool bboxYCompare(const Hitable* a, const Hitable* b)
{
    math::AABBf bboxLeft, bboxRight;
    if (!a->boundingBox(0.f, 0.f, bboxLeft) || !b->boundingBox(0.f, 0.f, bboxRight))
    {
        std::cerr << "No bounding box in BvhNode constructor\n";
    }
    if (bboxLeft.min().y - bboxRight.min().y < 0.f)
        return true;
    else
        return false;
}
bool bboxZCompare(const Hitable* a, const Hitable* b)
{
    math::AABBf bboxLeft, bboxRight;
    if (!a->boundingBox(0.f, 0.f, bboxLeft) || !b->boundingBox(0.f, 0.f, bboxRight))
    {
        std::cerr << "No bounding box in BvhNode constructor\n";
    }
    if (bboxLeft.min().z - bboxRight.min().z < 0.f)
        return true;
    else
        return false;
}

class BvhNode : public Hitable
{
public:
    BvhNode() {}
    BvhNode(std::vector<Hitable*>& list, float tMin, float tMax)
    {
        left  = nullptr;
        right = nullptr;

        int axis = int(3.f * rand01());
        if (axis == 0)
        {
            std::sort(list.begin(), list.end(), bboxXCompare);
        }
        else if (axis == 1)
        {
            std::sort(list.begin(), list.end(), bboxYCompare);
        }
        else
        {
            std::sort(list.begin(), list.end(), bboxZCompare);
        }

        if (list.size() == 1)
        {
            left = right = list.front();
        }
        else if (list.size() == 2)
        {
            left  = list.front();
            right = list.back();
        }
        else
        {
            std::vector<Hitable*> subLeft(list.begin(), list.begin() + list.size() / 2);
            std::vector<Hitable*> subRight(list.begin() + list.size() / 2, list.end());
            left  = new BvhNode(subLeft, tMin, tMax);
            right = new BvhNode(subRight, tMin, tMax);
        }
        math::AABBf bboxLeft, bboxRight;
        if (!left->boundingBox(tMin, tMax, bboxLeft) || !right->boundingBox(tMin, tMax, bboxRight))
        {
            std::cerr << "No bounding box in BvhNode constructor\n";
        }
        bbox = surroundingBbox(bboxLeft, bboxRight);
    }

    void release()
    {
        if (dynamic_cast<BvhNode*>(left) != nullptr)
        {
            delete left;
            left = nullptr;
        }
        if (dynamic_cast<BvhNode*>(right) != nullptr)
        {
            delete right;
            right = nullptr;
        }
    }

    bool hit(const math::Rayf& r, float tMin, float tMax, HitData& hitData) const override
    {
        if (bbox.hit(r, tMin, tMax))
        {
            HitData leftHitData, rightHitData;
            bool    hitLeft  = left->hit(r, tMin, tMax, leftHitData);
            bool    hitRight = right->hit(r, tMin, tMax, rightHitData);
            if (hitLeft && hitRight)
            {
                if (leftHitData.t < rightHitData.t)
                    hitData = leftHitData;
                else
                    hitData = rightHitData;
                return true;
            }
            else if (hitLeft)
            {
                hitData = leftHitData;
                return true;
            }
            else if (hitRight)
            {
                hitData = rightHitData;
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }
    bool boundingBox(float /*tMin*/, float /*tMax*/, math::AABBf& bbox) const override
    {
        bbox = this->bbox;
        return true;
    }

public:
    Hitable*    left;
    Hitable*    right;
    math::AABBf bbox;
};
