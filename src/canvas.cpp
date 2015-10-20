/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    canvas.cpp
        main implementation file
        implements platform independend API functionality
*/

#include "canvas.h"
#include "canvasimpl.h"


using namespace k_canvas;
using namespace impl;
using namespace c_util;
using namespace c_geometry;


/*
 -------------------------------------------------------------------------------
 kStroke object implementation
 -------------------------------------------------------------------------------
*/

kStroke::kStroke(kStrokeStyle style, kScalar dashoffset, const kScalar *strokes, size_t count)
{
    p_data.p_style = style;
    p_data.p_startcap = kCapStyle::Flat;
    p_data.p_endcap = kCapStyle::Flat;
    p_data.p_dashcap = kCapStyle::Flat;
    p_data.p_dashoffset = dashoffset;
    p_data.p_join = kLineJoin::Miter;
    p_data.p_count = 0;
}

kStroke::kStroke(
    kStrokeStyle style,
    kLineJoin join,
    kCapStyle startcap,
    kCapStyle endcap,
    kCapStyle dashcap,
    kScalar dashoffset,
    const kScalar *strokes, size_t count
)
{
    p_data.p_style = style;
    p_data.p_startcap = startcap;
    p_data.p_endcap = endcap;
    p_data.p_dashcap = dashcap;
    p_data.p_dashoffset = dashoffset;
    p_data.p_join = join;
    p_data.p_count = 0;
}

kStroke::~kStroke()
{}


/*
 -------------------------------------------------------------------------------
 kGradient object implementation
 -------------------------------------------------------------------------------
*/

kGradient::kGradient(const kColor &start, const kColor &end, kExtendType extend) :
    p_impl(CanvasFactory::CreateGradient())
{
    kGradientStop stops[2] = {
        kGradientStop(start, 0),
        kGradientStop(end, 1)
    };
    p_impl->Initialize(stops, 2, extend);
}

kGradient::kGradient(const kGradientStop *stops, size_t count, kExtendType extend) :
    p_impl(CanvasFactory::CreateGradient())
{
    p_impl->Initialize(stops, count, extend);
}

kGradient::~kGradient()
{
    ReleaseResource(p_impl);
}


/*
 -------------------------------------------------------------------------------
 kPen object implementation
 -------------------------------------------------------------------------------
*/

kPen::kPen()
{
    p_data.p_stroke = nullptr;
    p_data.p_brush = nullptr;
}

kPen::kPen(const kColor &color, kScalar width, kStrokeStyle style, const kScalar *strokes, size_t count)
{
    // stroke style and brush created implicitly
    kBrush brush(color);
    kStroke stroke(style, 0, strokes, count);

    p_data.p_width = width;
    p_data.p_stroke = stroke.getResource();
    p_data.p_brush = brush.getResource();
}

kPen::kPen(const kColor &color, kScalar width, const kStroke *stroke)
{
    kBrush brush(color);

    p_data.p_width = width;
    p_data.p_brush = brush.getResource();
    p_data.p_stroke = stroke->getResource();
}

kPen::kPen(const kBrush &brush, kScalar width, const kStroke *stroke)
{
    p_data.p_width = width;
    p_data.p_brush = brush.getResource();
    p_data.p_stroke = stroke->getResource();
}

kPen::kPen(const kPen &source) :
    kPenBase(source)
{
    if (source.p_data.p_brush) {
        p_data.p_brush->addref();
    }
    if (source.p_data.p_stroke) {
        p_data.p_stroke->addref();
    }
}

kPen::~kPen()
{
    ReleaseResource(p_data.p_brush);
    ReleaseResource(p_data.p_stroke);
}


/*
 -------------------------------------------------------------------------------
 kBrush object implementation
 -------------------------------------------------------------------------------
*/

kBrush::kBrush()
{
    p_data.p_style = kBrushStyle::Clear;
    p_data.p_gradient = nullptr;
    p_data.p_bitmap = nullptr;
}

kBrush::kBrush(const kColor &color)
{
    p_data.p_style = kBrushStyle::Solid;
    p_data.p_color = color;
    p_data.p_gradient = nullptr;
    p_data.p_bitmap = nullptr;
}

kBrush::kBrush(const kPoint &start, const kPoint &end, const kGradient &gradient)
{
    p_data.p_style = kBrushStyle::LinearGradient;
    p_data.p_start = start;
    p_data.p_end = end;
    p_data.p_gradient = gradient.p_impl;
    p_data.p_gradient->addref();
    p_data.p_bitmap = nullptr;
}

kBrush::kBrush(const kPoint &center, const kPoint &offset, const kSize &radius, const kGradient &gradient)
{
    p_data.p_style = kBrushStyle::RadialGradient;
    p_data.p_start = center;
    p_data.p_end = offset;
    p_data.p_radius = radius;
    p_data.p_gradient = gradient.p_impl;
    p_data.p_gradient->addref();
    p_data.p_bitmap = nullptr;
}

kBrush::kBrush(kExtendType xextend, kExtendType yextend, const kBitmap *bitmap)
{
    p_data.p_style = kBrushStyle::Bitmap;
    p_data.p_gradient = nullptr;
    p_data.p_xextend = xextend;
    p_data.p_yextend = yextend;
    p_data.p_bitmap = bitmap->p_impl;
    p_data.p_bitmap->addref();
}

kBrush::kBrush(const kBrush &source) :
    kBrushBase(source)
{
    if (source.p_data.p_gradient) {
        p_data.p_gradient->addref();
    }
    if (source.p_data.p_bitmap) {
        p_data.p_bitmap->addref();
    }
}

kBrush::~kBrush()
{
    ReleaseResource(p_data.p_gradient);
    ReleaseResource(p_data.p_bitmap);
}


/*
 -------------------------------------------------------------------------------
 kFont object implementation
 -------------------------------------------------------------------------------
*/

kFont::kFont()
{
    // TODO: implement default font
}

kFont::kFont(const char *facename, kScalar size, uint32_t style)
{
    strncpy(p_data.p_facename, facename, MAX_FONT_FACE_LENGTH);
    p_data.p_facename[MAX_FONT_FACE_LENGTH - 1] = 0;
    p_data.p_size = size;
    p_data.p_style = style;
}

//kFont::kFont(const kFont &source) :
//    kFontBase(source)
//{}

kFont::~kFont()
{}


// helper routine for converting arc to set of bezier segments

static const size_t MAX_ARC_POINTS = 16;

static void ArcToBezierPoints(const kRect &rect, kScalar start, kScalar end, kPoint *points, size_t &count)
{
    typedef vec2<kScalar> vec;
    typedef mat2x2<kScalar> mat;


    kScalar radius = rect.width() * kScalar(0.5);
    kScalar k = rect.height() / rect.width();
    kPoint center = rect.getCenter();

    vec rk = vec(kScalar(1.0), k);

    kScalar angle = end - start;
    kScalar sgn = sign(angle);
    angle = abs(angle);

    mat m;
    m.rotate(start);

    vec cur = m * vec(0, -radius);

    count = 0;
    points[count++] = cur.topoint<vec::type>() + center;

    while (angle > 0) {
        kScalar seg_angle = umin(kScalar(89.9), angle);
        kScalar k = kScalar(4.0) / kScalar(3.0) * tan(kScalar(0.25) * radians(seg_angle) * sgn);
        vec cp;

        cp = cur * rk + vec(-cur.y, cur.x) * rk * k;
        points[count++] = cp.topoint<vec::type>() + center;

        m.rotate(-seg_angle * sgn);
        cur = m * cur;
        cp = cur * rk + vec(cur.y, -cur.x) * rk * k;
        points[count++] = cp.topoint<vec::type>() + center;

        points[count++] = (cur * rk).topoint<vec::type>() + center;

        if ((count + 3) >= MAX_ARC_POINTS) {
            break;
        }

        angle -= seg_angle;
    }
}


/*
 -------------------------------------------------------------------------------
 kPath object implementation
 -------------------------------------------------------------------------------
*/

kPath::kPath() :
    p_impl(CanvasFactory::CreatePath())
{}

kPath::kPath(const kPath &source, const kTransform &transform) :
    p_impl(CanvasFactory::CreatePath())
{
    p_impl->FromPath(source.p_impl, transform);
}

kPath::~kPath()
{
    ReleaseResource(p_impl);
}

void kPath::MoveTo(const kPoint &point)
{
    p_impl->MoveTo(point);
}

void kPath::LineTo(const kPoint &point)
{
    p_impl->LineTo(point);
}

void kPath::BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3)
{
    p_impl->BezierTo(p1, p2, p3);
}

void kPath::ArcTo(const kRect &rect, kScalar start, kScalar end)
{
    // compute bezier segments' points
    kPoint arcpoints[MAX_ARC_POINTS];
    size_t count;
    ArcToBezierPoints(rect, start, end, arcpoints, count);

    // there's no way to exactly specify arc's starting point coords
    // so line is always added before arc's first segment
    p_impl->LineTo(*arcpoints);

    kPoint *cp = arcpoints + 1;
    --count;

    // add bezier segments to path
    for (size_t seg = 0; seg < (count / 3); ++seg) {
        p_impl->BezierTo(cp[0], cp[1], cp[2]);
        cp += 3;
    }
}

void kPath::PolyLineTo(const kPoint *points, size_t count)
{
    p_impl->PolyLineTo(points, count);
}

void kPath::PolyBezierTo(const kPoint *points, size_t count)
{
    p_impl->PolyBezierTo(points, count);
}

void kPath::Text(const char *text, int count, const kFont *font, kTextOrigin origin)
{
    if (font) {
        font->needResource();
        p_impl->Text(text, count, font, origin);
    }
}

void kPath::Close()
{
    p_impl->Close();
}

void kPath::Clear()
{
    p_impl->Clear();
}

void kPath::Commit()
{
    p_impl->Commit();
}


/*
 -------------------------------------------------------------------------------
 kBitmap object implementation
 -------------------------------------------------------------------------------
*/

kBitmap::kBitmap(size_t width, size_t height, kBitmapFormat format) :
    p_impl(CanvasFactory::CreateBitmap()),
    p_width(width),
    p_height(height),
    p_format(format)
{
    p_impl->Initialize(width, height, format);
}

kBitmap::~kBitmap()
{
    ReleaseResource(p_impl);
}

void kBitmap::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data)
{
    p_impl->Update(updaterect, sourceformat, sourcepitch, data);
}


/*
 -------------------------------------------------------------------------------
 kTextService implementation
 -------------------------------------------------------------------------------
*/

kTextService::kTextService() :
    p_impl(CanvasFactory::CreateCanvas())
{}

kTextService::~kTextService()
{
    delete p_impl;
}

void kTextService::GetFontMetrics(const kFont *font, kFontMetrics *metrics)
{
    if (font) {
        font->needResource();
        p_impl->GetFontMetrics(font, metrics);
    }
}

void kTextService::GetGlyphMetrics(const kFont *font, size_t first, size_t last, kGlyphMetrics *metrics)
{
    if (font) {
        font->needResource();
        p_impl->GetGlyphMetrics(font, first, last, metrics);
    }
}

kSize kTextService::TextSize(const char *text, int count, const kFont *font, kSize *bounds, const kTextSizeProperties *properties)
{
    if (font) {
        font->needResource();

        bool multiline = properties && (properties->flags & kTextFlags::Multiline);
        if (multiline) {
            kSize result;

            const unsigned char *t = reinterpret_cast<const unsigned char*>(text);
            if (count = -1) {
                count = int(strlen(text));
            }
            const unsigned char *end = t + count;

            while (t != end) {
                size_t word = 0;
                const unsigned char *w = t;
                while (t != end && *t > 32) {
                    ++word;
                    ++t;
                }

                bool wasbreak = false;
                size_t space = 0;
                while (t != end && *t <= 32) {
                    if (*t == '\n') {
                        wasbreak = true;
                        ++t;
                        break;
                    }
                    ++space;
                    ++t;
                }

                if (word) {


                }
            }

            return result;
        } else {
            return p_impl->TextSize(text, count, font, bounds);
        }
    } else {
        return kSize();
    }
}


/*
 -------------------------------------------------------------------------------
 kCanvas implementation
 -------------------------------------------------------------------------------
*/

bool kCanvas::Initialize(Impl implementation)
{
    // TODO
    return true;
}

bool kCanvas::Shutdown()
{
    if (CanvasFactory::getImpl() == IMPL_NONE) {
        return false;
    }
    CanvasFactory::destroyFactory();
    return true;
}

void kCanvas::needResources(const kPen *pen, const kBrush *brush)
{
    if (pen) {
        pen->needResource();
    }
    if (brush) {
        brush->needResource();
    }
}

void kCanvas::Line(const kPoint &a, const kPoint &b, const kPen *pen)
{
    if (pen) {
        pen->needResource();
        p_impl->Line(a, b, pen);
    }
}

void kCanvas::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPen *pen)
{
    if (pen) {
        pen->needResource();
        p_impl->Bezier(p1, p2, p3, p4, pen);
    }
}

void kCanvas::Arc(const kRect &rect, kScalar start, kScalar end, const kPen *pen)
{
    kPoint points[16];
    size_t count;
    ArcToBezierPoints(rect, start, end, points, count);
    PolyBezier(points, count, pen);
}

void kCanvas::PolyLine(const kPoint *points, size_t count, const kPen *pen)
{
    if (pen) {
        pen->needResource();
        p_impl->PolyLine(points, count, pen);
    }
}

void kCanvas::PolyBezier(const kPoint *points, size_t count, const kPen *pen)
{
    if (pen) {
        pen->needResource();
        p_impl->PolyBezier(points, count, pen);
    }
}

void kCanvas::Rectangle(const kRect &rect, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->Rectangle(rect, pen, brush);
}

void kCanvas::RoundedRectangle(const kRect &rect, const kSize &round, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->RoundedRectangle(rect, round, pen, brush);
}

void kCanvas::Ellipse(const kRect &rect, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->Ellipse(rect, pen, brush);
}

void kCanvas::Polygon(const kPoint *points, size_t count, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->Polygon(points, count, pen, brush);
}

void kCanvas::PolygonBezier(const kPoint *points, size_t count, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->PolygonBezier(points, count, pen, brush);
}

void kCanvas::DrawPath(const kPath *path, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->DrawPath(path->p_impl, pen, brush);
}

void kCanvas::DrawPath(const kPath *path, const kPen *pen, const kBrush *brush, const kPoint &offset)
{
    kTransform tfm;
    tfm.translate(offset.x, offset.y);
    DrawPath(path, pen, brush, tfm);
}

void kCanvas::DrawPath(const kPath *path, const kPen *pen, const kBrush *brush, const kTransform &transform)
{
    needResources(pen, brush);
    p_impl->DrawPath(path->p_impl, pen, brush, transform);
}

void kCanvas::DrawBitmap(const kBitmap *bitmap, const kPoint &origin, kScalar sourcealpha)
{
    kSize sz(kScalar(bitmap->width()), kScalar(bitmap->height()));
    p_impl->DrawBitmap(bitmap->p_impl, origin, sz, kPoint(), sz, sourcealpha);
}

void kCanvas::DrawBitmap(const kBitmap *bitmap, const kPoint &origin, const kPoint &source, const kSize &size, kScalar sourcealpha)
{
    p_impl->DrawBitmap(bitmap->p_impl, origin, size, source, size, sourcealpha);
}

void kCanvas::DrawBitmap(const kBitmap *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha)
{
    p_impl->DrawBitmap(bitmap->p_impl, origin, destsize, source, sourcesize, sourcealpha);
}

void kCanvas::DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin)
{
    if (brush) {
        brush->needResource();
        kSize sz(kScalar(mask->width()), kScalar(mask->height()));
        p_impl->DrawMask(mask->p_impl, brush, origin, sz, kPoint(), sz);
    }
}

void kCanvas::DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin, const kPoint &source, const kSize &size)
{
    if (brush) {
        brush->needResource();
        p_impl->DrawMask(mask->p_impl, brush, origin, size, source, size);
    }
}

void kCanvas::DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{
    if (brush) {
        brush->needResource();
        p_impl->DrawMask(mask->p_impl, brush, origin, destsize, source, sourcesize);
    }
}

void kCanvas::Text(const kPoint &p, const char *text, int count, const kFont *font, const kBrush *brush, kTextOrigin origin)
{
    if (brush && font) {
        if (brush) {
            brush->needResource();
        }
        if (font) {
            font->needResource();
        }
        p_impl->Text(p, text, count, font, brush, origin);
    }
}

void kCanvas::Text(const kRect &rect, const char *text, int count, const kFont *font, const kBrush *brush, const kTextOutProperties *properties)
{
    if (brush && font) {
        if (brush) {
            brush->needResource();
        }
        if (font) {
            font->needResource();
        }

        p_impl->BeginClippedDrawingByRect(rect);

        if (properties) {
            // if properties defined, output multiline text with word wrapping
            kFontMetrics fm;
            p_impl->GetFontMetrics(font, &fm);

            const unsigned char *t = reinterpret_cast<const unsigned char*>(text);
            if (count = -1) {
                count = int(strlen(text));
            }
            const unsigned char *end = t + count;

            kScalar spacewidth = p_impl->TextSize(" ", 1, font, nullptr).width;
            kScalar ellipseswidth = 0;
            if (properties->flags & kTextFlags::Ellipses) {
                ellipseswidth = p_impl->TextSize("...", 3, font, nullptr).width;
            }

            kPoint cp = rect.getLeftTop();
            kScalar lastspace = 0;
            while (t != end) {
                // find word
                size_t word = 0;
                const unsigned char *w = t;
                while (t != end && *t > 32) {
                    ++word;
                    ++t;
                }

                // find spaces and breaks
                bool wasbreak = false;
                size_t space = 0;
                while (t != end && *t <= 32) {
                    if ((properties->flags & kTextFlags::Multiline) && !(properties->flags & kTextFlags::IgnoreLineBreaks) && *t == '\n') {
                        wasbreak = true;
                        ++t;
                        break;
                    }
                    ++space;
                    ++t;
                }

                kScalar currentspace = 0;
                if (space) {
                    currentspace = properties->flags & kTextFlags::MergeSpaces ? spacewidth : space * spacewidth;
                }

                // if word found - render it with cp adjustment and line breaking if
                // needed
                if (word) {
                    kSize wordbounds;
                    kScalar wordwidth = p_impl->TextSize(
                        reinterpret_cast<const char*>(w), int(word), font, &wordbounds
                    ).width;

                    if (properties->flags & kTextFlags::Ellipses) {
                        // word treated as bound-crossing if it's not last and
                        // there's not enough space to insert ellipses
                        bool wordcrossedbounds = (cp.x + lastspace + wordwidth + currentspace) > (rect.right - ellipseswidth);

                        // check if current word needs to be cut by ellipses
                        if (!(properties->flags & kTextFlags::Multiline) && wordcrossedbounds) {
                            cp.x += lastspace;
                            kScalar x = cp.x;
                            size_t n = 0;
                            for (; n < word; ++n) {
                                kScalar width = p_impl->TextSize(reinterpret_cast<const char*>(w + n), 1, font, nullptr).width;
                                if ((x + width) > (rect.right - ellipseswidth)) {
                                    break;
                                } else {
                                    x += width;
                                }
                            }

                            p_impl->Text(cp, reinterpret_cast<const char*>(w), int(n), font, brush, kTextOrigin::Top);
                            p_impl->Text(kPoint(x, cp.y), "...", 3, font, brush, kTextOrigin::Top);

                            // rest part of the text ignored, because it will be clipped
                            break;
                        }
                    }

                    bool wordcrossedbounds = (cp.x + lastspace + wordbounds.width) > rect.right;
                    if ((properties->flags & kTextFlags::Multiline) && wordcrossedbounds) {
                        cp.x = rect.left;
                        cp.y += fm.height + properties->interval;
                    } else {
                        cp.x += lastspace;
                    }

                    p_impl->Text(
                        cp, reinterpret_cast<const char*>(w), int(word), font, brush,
                        kTextOrigin::Top
                    );
                    cp.x += wordwidth;
                }

                lastspace = currentspace;

                if (wasbreak) {
                    cp.x = rect.left;
                    cp.y += fm.height + properties->interval;
                }
            }
        } else {
            // if properties not given, use default text out as single line clipped to bounds
            // line breaks in text ignored
            p_impl->Text(rect.getLeftTop(), text, count, font, brush, kTextOrigin::Top);
        }

        p_impl->EndClippedDrawing();
    }
}

void kCanvas::BeginClippedDrawing(const kBitmap *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{
    p_impl->BeginClippedDrawingByMask(mask->p_impl, transform, xextend, yextend);
}

void kCanvas::BeginClippedDrawing(const kPath *clip, const kTransform &transform)
{
    p_impl->BeginClippedDrawingByPath(clip->p_impl, transform);
}

void kCanvas::BeginClippedDrawing(const kRect &clip)
{
    p_impl->BeginClippedDrawingByRect(clip);
}

void kCanvas::EndClippedDrawing()
{
    p_impl->EndClippedDrawing();
}

void kCanvas::SetTransform(const kTransform &transform)
{
    kTransform t =
        p_transform_stack.size() > 1 ?
        p_transform_stack[p_transform_stack.size() - 1] * transform :
        transform;

    if (p_transform_stack.size()) {
        p_transform_stack.back() = t;
    }
    p_impl->SetTransform(t);
}

void kCanvas::PushTransform(const kTransform &transform)
{
    kTransform t =
        p_transform_stack.size() ?
        p_transform_stack.back() * transform :
        transform;

    p_transform_stack.push_back(t);

    p_impl->SetTransform(t);
}

void kCanvas::PopTransform()
{
    if (p_transform_stack.size()) {
        p_transform_stack.pop_back();
        p_impl->SetTransform(
            p_transform_stack.size() ?
                p_transform_stack.back() :
                kTransform()
        );
    }
}


/*
 -------------------------------------------------------------------------------
 kBitmapCanvas implementation
 -------------------------------------------------------------------------------
*/

kBitmapCanvas::kBitmapCanvas(const kBitmap *target, const kRectInt *rect) :
    kCanvas()
{
    p_impl->BindToBitmap(target->p_impl, rect);
}

kBitmapCanvas::~kBitmapCanvas()
{
    p_impl->Unbind();
}


/*
 -------------------------------------------------------------------------------
 kContextCanvas implementation
 -------------------------------------------------------------------------------
*/

kContextCanvas::kContextCanvas(kContext context, const kRectInt *rect) :
    kCanvas()
{
    p_impl->BindToContext(context, rect);
}

kContextCanvas::~kContextCanvas()
{
    p_impl->Unbind();
}
