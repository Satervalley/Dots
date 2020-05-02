#pragma once

struct SVec2
{
    float x{ 0.f }, y{ 0.f };

    SVec2& operator =(const SVec2& v)
    {
        x = v.x;
        y = v.y;
        return *this;
    }

    float Distance(const SVec2& v) const
    {
        float dx = v.x - x, dy = v.y - y;

        return ::sqrtf(dx * dx + dy * dy);
    }


    float DistanceP2(const SVec2& v) const
    {
        float dx = v.x - x, dy = v.y - y;

        return (dx * dx + dy * dy);
    }

    float dx(const SVec2& v) const
    {
        return v.x - x;
    }

    float dy(const SVec2& v) const
    {
        return v.y - y;
    }

    void IncX(float dx)
    {
        x += dx;
    }

    void IncY(float dy)
    {
        y += dy;
    }

    void Inc(float dx, float dy)
    {
        x += dx;
        y += dy;
    }

    float Length(void) const
    {
        return ::sqrtf(x * x + y * y);
    }

    void Scale(float r)
    {
        float fl = Length();
        if (fl > 0.f)
        {
            fl = r / fl;
            x *= fl;
            y *= fl;
        }
    }
};


struct SRectF
{
    float l, t, r, b;

    SRectF& operator =(const CRect& rc)
    {
        l = (float)rc.left;
        t = (float)rc.top;
        r = (float)rc.right;
        b = (float)rc.bottom;
        return *this;
    }

    float Width(void) const
    {
        return r - l;
    }

    float Height(void) const
    {
        return b - t;
    }
};

