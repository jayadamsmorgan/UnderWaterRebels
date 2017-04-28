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
    private Touchpad xyTouchPad;

    @Override
    public void create() {
        batch = new SpriteBatch();
        mainStage = new Stage();

        // TODO: add texture for zSlider in assets/ui/sliders/zSlider
        Image zSliderBackgroundImage = new Image(
                new Texture(Gdx.files.internal("ui/sliders/zSlider/zSliderBackground.jpg")));
        Image zSliderKnobImage = new Image(new Texture(Gdx.files.internal("")));
        Drawable zSliderBackground = zSliderBackgroundImage.getDrawable();
        Drawable zSliderKnob = zSliderKnobImage.getDrawable();
        Slider.SliderStyle zSliderStyle = new Slider.SliderStyle(zSliderBackground,
                zSliderKnob);
        zSlider = new Slider(-100f, 100f, 1f, true, zSliderStyle);

        // TODO: add texture for rSlider in assets/ui/sliders/rSlider
        Image rSliderBackgroundImage = new Image(
                new Texture(Gdx.files.internal("ui/sliders/rSlider/rSliderBackground.jpg")));
        Image rSliderKnobImage = new Image(
                new Texture(Gdx.files.internal("ui/sliders/rSlider/rSliderKnob.jpg")));
        Drawable rSliderBackground = rSliderBackgroundImage.getDrawable();
        Drawable rSliderKnob = rSliderKnobImage.getDrawable();
        Slider.SliderStyle rSliderStyle = new Slider.SliderStyle(rSliderBackground, rSliderKnob);
        rSlider = new Slider(-100f, 100f, 1f, false, rSliderStyle);

        // TODO: add texture for xyTouchPad in assets/ui/touchPads/xyTouchPad
        Image xyTouchPadBackgroundImage = new Image(
                new Texture(Gdx.files.internal("ui/touchPads/xyTouchPad/xyTouchPadBackground.jpg")));
        Image xyTouchPadKnobImage = new Image(
                new Texture(Gdx.files.internal("ui/touchPads/xyTouchPad/xyTouchPadKnob.jpg")));
        Drawable xyTouchPadBackground = xyTouchPadBackgroundImage.getDrawable();
        Drawable xyTouchPadKnob = xyTouchPadKnobImage.getDrawable();
        Touchpad.TouchpadStyle xyTouchPadStyle = new Touchpad.TouchpadStyle(xyTouchPadBackground,
                xyTouchPadKnob);
        xyTouchPad = new Touchpad(5f, xyTouchPadStyle);

        setActorsPositions();

        mainStage.addActor(zSlider);
        mainStage.addActor(rSlider);
        mainStage.addActor(xyTouchPad);

        udpThread = new UDPThread();
        udpThread.start();

        Gdx.input.setInputProcessor(mainStage);
    }

    @Override
    public void render() {
        Gdx.gl.glClearColor(1f / 15f, 1f / 15f, 1f / 15f, 1);
        Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);
        batch.begin();
        mainStage.draw();
        batch.end();
    }

    private void setActorsPositions() {
        // TODO: set positions of all actors
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

