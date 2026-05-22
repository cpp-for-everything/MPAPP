// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Custom View that renders one of {rect, ellipse, line}
// using the configured fill / stroke / strokeWidth / opacity. Pushed
// through JNI setters by the C++ shape_view_handler; each setter calls
// invalidate() so the next vsync re-renders.
//
// kind ints match the C++ shape_kind enum: 0=rect, 1=ellipse,
// 2=line, 3=polygon, 4=path. polygon/path render as the bounding rect
// in v1 (matches the Windows + Linux v1 fallback).

package io.mpapp;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.view.View;

public final class MppShapeView extends View {
    private int kind = 0;
    private String data = "";
    private int fillColor = 0;      // 0 = unset
    private int strokeColor = 0;
    private float strokeWidth = 1f;
    private float opacity = 1f;

    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);

    public MppShapeView(Context ctx) {
        super(ctx);
    }

    public void setShapeKind(int k)         { kind = k; invalidate(); }
    public void setShapeData(String d)      { data = d != null ? d : ""; invalidate(); }
    public void setFillColor(int c)         { fillColor = c; invalidate(); }
    public void setStrokeColor(int c)       { strokeColor = c; invalidate(); }
    public void setStrokeWidth(float w)     { strokeWidth = w; invalidate(); }
    public void setShapeOpacity(float a)    { opacity = a; invalidate(); }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int w = getWidth();
        int h = getHeight();
        if (w == 0 || h == 0) return;
        float off = strokeWidth * 0.5f;
        float rw  = w - strokeWidth;
        float rh  = h - strokeWidth;

        int alpha = (int)(opacity * 255f);
        if (alpha < 0) alpha = 0; if (alpha > 255) alpha = 255;

        if (kind == 1) {
            // ellipse
            paintFillThenStroke(canvas, off, off, off + rw, off + rh, true, alpha);
        } else if (kind == 2) {
            // line — parse "M x1 y1 L x2 y2"
            float[] pts = parseLine(data, w, h);
            if (strokeColor != 0) {
                paint.setStyle(Paint.Style.STROKE);
                paint.setColor(strokeColor);
                paint.setAlpha(alpha);
                paint.setStrokeWidth(strokeWidth);
                canvas.drawLine(pts[0], pts[1], pts[2], pts[3], paint);
            }
        } else {
            // rectangle / polygon / path (the latter two render as bounding rect)
            paintFillThenStroke(canvas, off, off, off + rw, off + rh, false, alpha);
        }
    }

    private void paintFillThenStroke(Canvas c, float left, float top,
                                     float right, float bottom,
                                     boolean ellipse, int alpha) {
        if (fillColor != 0) {
            paint.setStyle(Paint.Style.FILL);
            paint.setColor(fillColor);
            paint.setAlpha(alpha);
            if (ellipse) {
                c.drawOval(left, top, right, bottom, paint);
            } else {
                c.drawRect(left, top, right, bottom, paint);
            }
        }
        if (strokeColor != 0) {
            paint.setStyle(Paint.Style.STROKE);
            paint.setColor(strokeColor);
            paint.setAlpha(alpha);
            paint.setStrokeWidth(strokeWidth);
            if (ellipse) {
                c.drawOval(left, top, right, bottom, paint);
            } else {
                c.drawRect(left, top, right, bottom, paint);
            }
        }
    }

    private static float[] parseLine(String d, int defaultW, int defaultH) {
        float[] out = new float[] { 0f, 0f, (float)defaultW, (float)defaultH };
        if (d == null || d.isEmpty()) return out;
        int n = 0;
        int i = 0;
        int len = d.length();
        while (i < len && n < 4) {
            // skip non-numeric
            while (i < len) {
                char c = d.charAt(i);
                if (c == '-' || c == '+' || c == '.' || (c >= '0' && c <= '9')) break;
                i++;
            }
            if (i >= len) break;
            int start = i;
            if (d.charAt(i) == '-' || d.charAt(i) == '+') i++;
            while (i < len) {
                char c = d.charAt(i);
                if (!((c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+')) break;
                i++;
            }
            try {
                out[n++] = Float.parseFloat(d.substring(start, i));
            } catch (NumberFormatException nfe) {
                break;
            }
        }
        return out;
    }
}
