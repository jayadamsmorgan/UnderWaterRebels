package com.germanberdnikov.rov;

import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.g2d.Batch;
import com.badlogic.gdx.math.Circle;
import com.badlogic.gdx.math.Vector2;
import com.badlogic.gdx.scenes.scene2d.Actor;
import com.badlogic.gdx.scenes.scene2d.InputEvent;
import com.badlogic.gdx.scenes.scene2d.InputListener;
import com.badlogic.gdx.scenes.scene2d.ui.Widget;
import com.badlogic.gdx.scenes.scene2d.utils.ChangeListener;
import com.badlogic.gdx.scenes.scene2d.utils.Drawable;
import com.badlogic.gdx.utils.Pools;

/**
 * This is copy of LibGDX's Touchpad class, modified for support to set knob position
 */

class CustomTouchPad extends Widget {
    private com.badlogic.gdx.scenes.scene2d.ui.Touchpad.TouchpadStyle style;
    boolean touched;
    boolean resetOnTouchUp = true;
    private float deadzoneRadius;
    private final Circle knobBounds = new Circle(0, 0, 0);
    private final Circle touchBounds = new Circle(0, 0, 0);
    private final Circle deadzoneBounds = new Circle(0, 0, 0);
    private final Vector2 knobPosition = new Vector2();
    private final Vector2 knobPercent = new Vector2();

    CustomTouchPad(float deadzoneRadius, com.badlogic.gdx.scenes.scene2d.ui.Touchpad.TouchpadStyle style) {
        if (deadzoneRadius < 0) throw new IllegalArgumentException("deadzoneRadius must be > 0");
        this.deadzoneRadius = deadzoneRadius;

        knobPosition.set(getWidth() / 2f, getHeight() / 2f);

        setStyle(style);
        setSize(getPrefWidth(), getPrefHeight());

        addListener(new InputListener() {
            @Override
            public boolean touchDown(InputEvent event, float x, float y, int pointer, int button) {
                if (touched) return false;
                touched = true;
                calculatePositionAndValue(x, y, false);
                return true;
            }

            @Override
            public void touchDragged(InputEvent event, float x, float y, int pointer) {
                calculatePositionAndValue(x, y, false);
            }

            @Override
            public void touchUp(InputEvent event, float x, float y, int pointer, int button) {
                touched = false;
                calculatePositionAndValue(x, y, resetOnTouchUp);
            }
        });
    }

    void calculatePositionAndValue(float x, float y, boolean isTouchUp) {
        float oldPositionX = knobPosition.x;
        float oldPositionY = knobPosition.y;
        float oldPercentX = knobPercent.x;
        float oldPercentY = knobPercent.y;
        float centerX = knobBounds.x;
        float centerY = knobBounds.y;
        knobPosition.set(centerX, centerY);
        knobPercent.set(0f, 0f);
        if (!isTouchUp) {
            if (!deadzoneBounds.contains(x, y)) {
                knobPercent.set((x - centerX) / knobBounds.radius, (y - centerY) / knobBounds.radius);
                float length = knobPercent.len();
                if (length > 1) knobPercent.scl(1 / length);
                if (knobBounds.contains(x, y)) {
                    knobPosition.set(x, y);
                } else {
                    knobPosition.set(knobPercent).nor().scl(knobBounds.radius).add(knobBounds.x, knobBounds.y);
                }
            }
        }
        if (oldPercentX != knobPercent.x || oldPercentY != knobPercent.y) {
            ChangeListener.ChangeEvent changeEvent = Pools.obtain(ChangeListener.ChangeEvent.class);
            if (fire(changeEvent)) {
                knobPercent.set(oldPercentX, oldPercentY);
                knobPosition.set(oldPositionX, oldPositionY);
            }
            Pools.free(changeEvent);
        }
    }

    public void setStyle(com.badlogic.gdx.scenes.scene2d.ui.Touchpad.TouchpadStyle style) {
        if (style == null) throw new IllegalArgumentException("style cannot be null");
        this.style = style;
        invalidateHierarchy();
    }

    public com.badlogic.gdx.scenes.scene2d.ui.Touchpad.TouchpadStyle getStyle() {
        return style;
    }

    @Override
    public Actor hit(float x, float y, boolean touchable) {
        return touchBounds.contains(x, y) ? this : null;
    }

    @Override
    public void layout() {
        // Recalc pad and deadzone bounds
        float halfWidth = getWidth() / 2;
        float halfHeight = getHeight() / 2;
        float radius = Math.min(halfWidth, halfHeight);
        touchBounds.set(halfWidth, halfHeight, radius);
        if (style.knob != null)
            radius -= Math.max(style.knob.getMinWidth(), style.knob.getMinHeight()) / 2;
        knobBounds.set(halfWidth, halfHeight, radius);
        deadzoneBounds.set(halfWidth, halfHeight, deadzoneRadius);
        // Recalc pad values and knob position
        knobPosition.set(halfWidth, halfHeight);
        knobPercent.set(0, 0);
    }

    @Override
    public void draw(Batch batch, float parentAlpha) {
        validate();

        Color c = getColor();
        batch.setColor(c.r, c.g, c.b, c.a * parentAlpha);

        float x = getX();
        float y = getY();
        float w = getWidth();
        float h = getHeight();

        final Drawable bg = style.background;
        if (bg != null) bg.draw(batch, x, y, w, h);

        final Drawable knob = style.knob;
        if (knob != null) {
            x += knobPosition.x - knob.getMinWidth() / 2f;
            y += knobPosition.y - knob.getMinHeight() / 2f;
            knob.draw(batch, x, y, knob.getMinWidth(), knob.getMinHeight());
        }
    }

    @Override
    public float getPrefWidth() {
        return style.background != null ? style.background.getMinWidth() : 0;
    }

    @Override
    public float getPrefHeight() {
        return style.background != null ? style.background.getMinHeight() : 0;
    }

    public boolean isTouched() {
        return touched;
    }

    public float getKnobPercentX() {
        return knobPercent.x;
    }

    public float getKnobPercentY() {
        return knobPercent.y;
    }
}
