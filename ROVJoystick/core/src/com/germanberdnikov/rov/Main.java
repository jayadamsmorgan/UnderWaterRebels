package com.germanberdnikov.rov;

import com.badlogic.gdx.ApplicationAdapter;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.scenes.scene2d.Stage;
import com.badlogic.gdx.scenes.scene2d.ui.Button;
import com.badlogic.gdx.scenes.scene2d.ui.Image;
import com.badlogic.gdx.scenes.scene2d.ui.Slider;
import com.badlogic.gdx.scenes.scene2d.ui.Touchpad;
import com.badlogic.gdx.scenes.scene2d.utils.Drawable;

class Main extends ApplicationAdapter {
    // UDPThread for udp connection with PC
    private UDPThread udpThread;

    // UI objects:
    private SpriteBatch batch;
    private Stage mainStage;
    private Slider zSlider, rSlider, rMainManipSlider, camSlider, tBotManipSlider, speedSlider;
    private CustomTouchPad xyTouchPad;
    private Button tightMainManipButton, untightMainManipButton,
            autoPitchButton, autoYawButton, autoDepthButton,
            multiplexorButton, ledButton;

    // JS Data:
    private int xAxis, yAxis, zAxis, rzAxis;
    private boolean isAutoYaw, isAutoPitch, isAutoDepth, isLED;
    private int speedMode, muxChannel;
    private int cameraRotateDirection, mainManipTightDirection, mainManipRotateDirection,
            bottomManipDirection;

    // Paths:
    private String zSliderBackground = "ui/sliders/zSlider/zSliderBackground.png";
    private String zSliderKnob = "ui/sliders/zSlider/zSliderKnob.png";
    private String speedSliderBackground = "ui/sliders/speedSlider/speedSliderBackground.png";
    private String speedSliderKnob = "ui/sliders/speedSlider/speedSliderKnob.png";
    private String rSliderBackground = "ui/sliders/rSlider/rSliderBackground.png";
    private String rSliderKnob = "ui/sliders/rSlider/rSliderKnob.png";
    private String xyTouchPadBackground = "ui/touchPads/xyTouchPad/xyTouchPadBackground.png";
    private String xyTouchPadKnob = "ui/touchPads/xyTouchPad/xyTouchPadKnob.png";

    @Override
    public void create() {
        batch = new SpriteBatch();
        mainStage = new Stage();

        zSlider = createNewSlider(zSliderBackground, zSliderKnob,
                -100f, 100f, 1f, true);

        speedSlider = createNewSlider(speedSliderBackground, speedSliderKnob,
                0f, 3f, 1f,false);

        rSlider = createNewSlider(rSliderBackground, rSliderKnob,
                -100f, 100f, 1f,false);

        xyTouchPad = createNewTouchPad(xyTouchPadBackground, xyTouchPadKnob, 5f);

        mainStage.addActor(zSlider);
        mainStage.addActor(rSlider);
        mainStage.addActor(speedSlider);
        mainStage.addActor(xyTouchPad);

        setActorsSize();
        setActorsPositions();

        udpThread = new UDPThread();
        udpThread.start();

        Gdx.input.setInputProcessor(mainStage);
    }

    private Slider createNewSlider(String backgroundPath, String knobPath,
                                   float minValue, float maxValue, float stepSize,
                                   boolean isVertical) {
        Image backgroundImage = new Image(new Texture(Gdx.files.internal(backgroundPath)));
        Image knobImage = new Image(new Texture(Gdx.files.internal(knobPath)));
        Drawable sliderBackground = backgroundImage.getDrawable();
        Drawable sliderKnob = knobImage.getDrawable();
        Slider.SliderStyle sliderStyle = new Slider.SliderStyle(sliderBackground, sliderKnob);
        return new Slider(minValue, maxValue, stepSize, isVertical, sliderStyle);
    }

    private CustomTouchPad createNewTouchPad(String backgroundPath, String knobPath,
                                             float deadzoneRadius) {
        Image touchPadBackgroundImage = new Image(
                new Texture(Gdx.files.internal(backgroundPath)));
        Image touchPadKnobImage = new Image(new Texture(Gdx.files.internal(knobPath)));
        Drawable touchPadBackground = touchPadBackgroundImage.getDrawable();
        Drawable touchPadKnob = touchPadKnobImage.getDrawable();
        Touchpad.TouchpadStyle touchPadStyle = new Touchpad.TouchpadStyle(touchPadBackground,
                touchPadKnob);
        return new CustomTouchPad(deadzoneRadius, touchPadStyle);
    }

    @Override
    public void render() {
        setZeroPositions();
        Gdx.gl.glClearColor(1f / 15f, 1f / 15f, 1f / 15f, 1);
        Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);
        batch.begin();
        mainStage.draw();
        batch.end();
        prepareData();
        preparePacket();
    }

    private void preparePacket() {
        udpThread.setData(xAxis, yAxis, zAxis, rzAxis, mainManipRotateDirection,
                cameraRotateDirection, mainManipTightDirection, bottomManipDirection,
                speedMode, muxChannel, isAutoYaw, isAutoPitch, isAutoDepth, isLED);
    }

    private void prepareData() {
        xAxis = (int) xyTouchPad.getKnobPercentX();
        yAxis = (int) xyTouchPad.getKnobPercentY();
        zAxis = (int) zSlider.getValue();
        rzAxis = (int) rSlider.getValue();
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
        zSlider.setSize(60,
                Gdx.graphics.getHeight());
        rSlider.setSize(Gdx.graphics.getWidth() - zSlider.getWidth() - 30,
                120);
        xyTouchPad.setSize(Gdx.graphics.getWidth() / 2.5f,
                Gdx.graphics.getWidth() / 2.5f);
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