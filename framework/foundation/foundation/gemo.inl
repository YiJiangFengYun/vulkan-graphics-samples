namespace fd
{
    template <typename VecType>
    Ray<VecType>::Ray()
    {

    }

    template <typename VecType>
    Ray<VecType>::Ray(ValueType origin, ValueType direction)
        : m_origin(origin)
        , m_direction(direction)
        , m_isUpdateCache(FD_TRUE)
    {
#ifdef DEBUG
        VecValueType distSqr = glm::dot(direction, direction);
        if (distSqr == 0)
            throw std::invalid_argument("Invalid direction, direction length is 0.");
#endif // DEBUG
    }

    template <typename VecType>
    Ray<VecType>::Ray(const Ray<ValueType>& target)
        : m_origin(target.m_origin)
        , m_direction(target.m_direction)
        , m_isUpdateCache(target.m_isUpdateCache)
        , m_invDir(target.m_invDir)
        , m_signs(target.m_signs)
    {
    }

    template <typename VecType>
    Ray<VecType>& Ray<VecType>::operator =(const Ray<ValueType>& target)
    {
        m_origin = target.m_origin;
        m_direction = target.m_direction;
        m_isUpdateCache = target.m_isUpdateCache;
        m_invDir = target.m_invDir;
        m_signs = target.m_signs;
        return *this;
    }

    template <typename VecType>
    typename Ray<VecType>::ValueType Ray<VecType>::getOrigin() const
    {
        return m_origin;
    }

    template <typename VecType>
    void Ray<VecType>::setOrigin(ValueType origin)
    {
        m_origin = origin;
        m_isUpdateCache = true;
    }

    template <typename VecType>
    typename Ray<VecType>::ValueType Ray<VecType>::getDirection() const
    {
        return m_direction;
    }

    template <typename VecType>
    void Ray<VecType>::setDirection(ValueType direction)
    {
        m_direction = direction;
        m_isUpdateCache = true;
    }

    template <typename VecType>
    typename Ray<VecType>::ValueType Ray<VecType>::getInvDir()
    {
        _updateCache();
        return m_invDir;
    }

    template <typename VecType>
    typename Ray<VecType>::ValueType Ray<VecType>::getSigns()
    {
        _updateCache();
        return m_signs;
    }

    template <typename VecType>
    typename Ray<VecType>::ValueType Ray<VecType>::getPoint(VecValueType distance) const
    {
        ValueType direction = m_direction;
        glm::normalize(direction);
        direction *= distance;
        direction += m_origin;
        return direction;
    }

    template <typename VecType>
    void Ray<VecType>::_updateCache()
    {
        if (m_isUpdateCache)
        {
            m_isUpdateCache = FD_FALSE;
            const VecValueType Epsilon = std::numeric_limits<VecValueType>::epsilon();
            LengthType length = ValueType::length();
            for (LengthType i = 0; i < length; ++i)
            {
                if(glm::abs(m_direction[i]) < Epsilon)
                {
                    m_signs[i] = 0;
                    m_invDir[i] = 0;
                    continue;
                }
                m_invDir[i] = 1 / m_direction[i];
                //negative is 0 and positive is 1, it is better to be used as array index.
                m_signs[i] = m_direction[i] < 0 ? 
                    static_cast<typename Ray<VecType>::VecValueType>(-1) :
                    static_cast<typename Ray<VecType>::VecValueType>(1);
            }
        }
    }

    template <typename VecType>
    Bounds<VecType>::Bounds()
        : m_min(0u)
        , m_max(0u)
    {
        _updateSize();
    }

    template <typename VecType>
    Bounds<VecType>::Bounds(ValueType min, ValueType max)
        : m_min(min)
        , m_max(max)
    {
        _updateSize();
    }

    template <typename VecType>
    Bounds<VecType>::Bounds(const Bounds<ValueType>& target)
        : m_min(target.m_min)
        , m_max(target.m_max)
        , m_size(target.m_size)
    {
    }

    template <typename VecType>
    Bounds<VecType>::~Bounds()
    {

    }

    template <typename VecType>
    Bounds<VecType>& Bounds<VecType>::operator =(const Bounds<ValueType>& target)
    {
        m_min = target.m_min;
        m_max = target.m_max;
        m_size = target.m_size;
        return *this;
    }

    template <typename VecType>
    typename Bounds<VecType>::ValueType Bounds<VecType>::getMin() const { return m_min; }

    template <typename VecType>
    typename Bounds<VecType>::ValueType Bounds<VecType>::getMax() const { return m_max; }

    template <typename VecType>
    void Bounds<VecType>::setMinMax(ValueType min, ValueType max)
    {
        m_min = min;
        m_max = max;
        _updateSize();
    }

    template <typename VecType>
    typename Bounds<VecType>::ValueType Bounds<VecType>::getSize() { return m_size; }

    template <typename VecType>
    void Bounds<VecType>::setSize(ValueType size)
    {
#ifdef DEBUG
        LengthType length = ValueType::length();
        for (LengthType i = 0; i < length; ++i)
        {
            if (size[i] < 0)throw std::invalid_argument("Invalid size of Bounds");
        }
#endif // DEBUG
        m_size = size;
        _updateMax();
    }

    template <typename VecType>
    typename Bounds<VecType>::ValueType Bounds<VecType>::getClosestPoint(ValueType point) const
    {
        LengthType length = ValueType::length();
        ValueType result;
        for (LengthType i = 0; i < length; ++i)
        {
            if (point[i] < m_min[i]) result[i] = m_min[i];
            else if (point[i] < m_max[i]) result[i] = point[i];
            else result[i] = m_max[i];
        }
        return result;
    }

    template <typename VecType>
    Bool32 Bounds<VecType>::isContains(ValueType point) const
    {
        LengthType length = ValueType::length();
        for (LengthType i = 0; i < length; ++i)
        {
            if (point[i] < m_min[i] || point[i] > m_max[i])
                return false;
        }
        return true;
    }

    template <typename VecType>
    void Bounds<VecType>::expand(VecValueType amount)
    {
        VecValueType halt = amount / 2;
        LengthType length = ValueType::length();
        for (LengthType i = 0; i < length; ++i)
        {
            m_min[i] -= halt;
            m_max[i] += halt;
            m_size[i] += amount;
        }
    }

    template <typename VecType>
    typename Bounds<VecType>::VecValueType Bounds<VecType>::intersectRay(Ray<ValueType> ray, Bool32 isOnlyForward) const
    {

        ////reference:
        //// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
        ValueType origin = ray.getOrigin();
        ValueType dir = ray.getDirection();
        ValueType invDir = ray.getInvDir();
        ValueType signs = ray.getSigns();

        ValueType minMax[2] = { m_min, m_max };

        const LengthType length = ValueType::length();
        const VecValueType Epsilon = std::numeric_limits<VecValueType>::epsilon();
        LengthType i = 0;
        VecValueType min = std::numeric_limits<VecValueType>::lowest();
        VecValueType max = std::numeric_limits<VecValueType>::max();
        VecValueType tMin;
        VecValueType tMax;
        for (LengthType i = 0; i < length; ++i)
        {
            if (signs[i] == 0) {
                if (origin[i] < minMax[0][i] || origin[i] > minMax[1][i]) return -1;
                else continue;
            }

            uint32_t index = (static_cast<uint32_t>(signs[i]) + 1) / 2;
            tMin = (minMax[static_cast<size_t>(1 - index)][i] - origin[i]) * invDir[i];
            tMax = (minMax[static_cast<size_t>(index)][i] - origin[i]) * invDir[i];

            if (isOnlyForward && tMax < 0) return -1; //ray is only positive direction. if tMax < 0, tMax and tMin < 0.

            if (min > tMax || tMin > max)
                return -1;

            if (tMin > min)
                min = tMin;
            if (tMax < max)
                max = tMax;
        }

        //calculate distance.
        VecValueType d;
        if (tMin < 0)
        {
            d = tMax;
        }
        else
        {
            d = tMin;
        }

        return d * glm::length(dir);

    }

    template <typename VecType>
    Bool32 Bounds<VecType>::isIntersects(Bounds<ValueType> bounds) const
    {
        LengthType length = ValueType::length();
        for (LengthType i = 0; i < length; ++i)
        {
            if (m_min[i] > bounds.m_max[i] || m_max[i] < bounds.m_min[i])
                return false;
        }
        return true;
    }

    template <typename VecType>
    Bool32 Bounds<VecType>::intersects(Bounds<ValueType> bounds, Bounds<ValueType> *intersection) const
    {
        LengthType length = ValueType::length();
        Bool32 isInter = FD_TRUE;
        ValueType min(0.0f);
        ValueType max(0.0f);
        for (LengthType i = 0; i < length; ++i)
        {
            if (m_min[i] > bounds.m_max[i] || m_max[i] < bounds.m_min[i])
            {
                isInter = FD_FALSE;
                break;
            }
            else
            {
                if (intersection != nullptr)
                {
                    min[i] = glm::max(m_min[i], bounds.m_min[i]);
                    max[i] = glm::min(m_max[i], bounds.m_max[i]);
                }
            }
        }

        if (isInter && intersection != nullptr)
        {
            intersection->setMinMax(min, max);
        }


        return isInter;
    }

    template <typename VecType>
    typename Bounds<VecType>::VecValueType Bounds<VecType>::getSqrDistance(ValueType point) const
    {
        ValueType closestPoint = getClosestPoint(point);
        closestPoint -= point;
        return glm::dot(closestPoint, closestPoint);
    }

    template <typename VecType>
    void Bounds<VecType>::_updateSize()
    {
        LengthType length = ValueType::length();
        for (LengthType i = 0; i < length; ++i)
        {
            m_size[i] = m_max[i] - m_min[i];
        }
#ifdef DEBUG
        for (LengthType i = 0; i < length; ++i)
        {
            if (m_size[i] < 0)
                throw std::invalid_argument("Invalid size of Bounds");
        }
#endif // DEBUG
    }

    template <typename VecType>
    void Bounds<VecType>::_updateMax()
    {
        LengthType length = ValueType::length();
        for (LengthType i = 0; i < length; ++i)
        {
            m_max[i] = m_min[i] + m_size[i];
        }
    }
}