package com.germanberdnikov.rov;

import com.badlogic.gdx.ApplicationAdapter;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.scenes.scene2d.Stage;
import com.badlogic.gdx.scenes.scene2d.ui.Image;
import com.badlogic.gdx.scenes.scene2d.ui.Slider;
import com.badlogic.gdx.scenes.scene2d.ui.Touchpad;
import com.badlogic.gdx.scenes.scene2d.utils.Drawable;

class Main extends ApplicationAdapter {
    private SpriteBatch batch;
    private Stage mainStage;

    // UDPThread for udp connection with PC
    private UDPThread udpThread;

    // UI objects:
    private Slider zSlider, rSlider;
    private CustomTouchPad xyTouchPad;

    @Override
    public void create() {
        batch = new SpriteBatch();
        mainStage = new Stage();

        Image zSliderBackgroundImage = new Image(
                new Texture(Gdx.files.internal("ui/sliders/zSlider/zSliderBackground.png")));
        Image zSliderKnobImage = new Image(
                new Texture(Gdx.files.internal("ui/sliders/zSlider/zSliderKnob.png")));
        Drawable zSliderBackground = zSliderBackgroundImage.getDrawable();
        Drawable zSliderKnob = zSliderKnobImage.getDrawable();
        Slider.SliderStyle zSliderStyle = new Slider.SliderStyle(zSliderBackground,
                zSliderKnob);
        zSlider = new Slider(-100f, 100f, 1f, true, zSliderStyle);

        Image rSliderBackgroundImage = new Image(
                new Texture(Gdx.files.internal("ui/sliders/rSlider/rSliderBackground.png")));
        Image rSliderKnobImage = new Image(
                new Texture(Gdx.files.internal("ui/sliders/rSlider/rSliderKnob.png")));
        Drawable rSliderBackground = rSliderBackgroundImage.getDrawable();
        Drawable rSliderKnob = rSliderKnobImage.getDrawable();
        Slider.SliderStyle rSliderStyle = new Slider.SliderStyle(rSliderBackground, rSliderKnob);
        rSlider = new Slider(-100f, 100f, 1f, false, rSliderStyle);

        Image xyTouchPadBackgroundImage = new Image(
                new Texture(Gdx.files.internal("ui/touchPads/xyTouchPad/xyTouchPadBackground.png")));
        Image xyTouchPadKnobImage = new Image(
                new Texture(Gdx.files.internal("ui/touchPads/xyTouchPad/xyTouchPadKnob.png")));
        Drawable xyTouchPadBackground = xyTouchPadBackgroundImage.getDrawable();
        Drawable xyTouchPadKnob = xyTouchPadKnobImage.getDrawable();
        Touchpad.TouchpadStyle xyTouchPadStyle = new Touchpad.TouchpadStyle(xyTouchPadBackground,
                xyTouchPadKnob);
        xyTouchPad = new CustomTouchPad(5f, xyTouchPadStyle);

        mainStage.addActor(zSlider);
        mainStage.addActor(rSlider);
        mainStage.addActor(xyTouchPad);

        setActorsPositions();
        setActorsSize();

        udpThread = new UDPThread();
        udpThread.start();

        Gdx.input.setInputProcessor(mainStage);
    }

    @Override
    public void render() {
        setZeroPositions();
        Gdx.gl.glClearColor(1f / 15f, 1f / 15f, 1f / 15f, 1);
        Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);
        batch.begin();
        mainStage.draw();
        batch.end();
        preparePacket();
    }

    private void preparePacket() {
        udpThread.setData((int) xyTouchPad.getKnobPercentX(), (int) xyTouchPad.getKnobPercentY(),
                (int) zSlider.getValue(), (int) rSlider.getValue(), 0, 0, 0, 0, 0, 0, false, false, false, false);
    }

    private void setZeroPositions() {
        if (!zSlider.isDragging()) {
            zSlider.setValue(0f);
        }
        if (!rSlider.isDragging()) {
            rSlider.setValue(0f);
        }
        if (!xyTouchPad.isTouched()) {
            xyTouchPad.calculatePositionAndValue(0, 0, true);
        }
    }

    private void setActorsSize() {
        zSlider.setSize(60, Gdx.graphics.getHeight());
        rSlider.setSize(Gdx.graphics.getWidth() - zSlider.getWidth() - 30, 120);
        xyTouchPad.setSize(Gdx.graphics.getWidth() / 2.5f, Gdx.graphics.getWidth() / 2.5f);
    }

    private void setActorsPositions() {
        zSlider.setPosition(Gdx.graphics.getWidth() - zSlider.getWidth(), 0);
        xyTouchPad.setPosition(0, 0);
        rSlider.setPosition(xyTouchPad.getWidth() + 20, 0);
    }

    @Override
    public void resume() {
        udpThread.setRunning(true);
    }

    @Override
    public void pause() {
        udpThread.setRunning(false);
    }

    @Override
    public void dispose() {
        batch.dispose();
    }
}

