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

    @Override
    public void create() {
        batch = new SpriteBatch();
        mainStage = new Stage();

        zSlider = createNewSlider(zSliderBackground, zSliderKnob,
                -100f, 100f, 1f, true);

        speedSlider = createNewSlider(speedSliderBackground, speedSliderKnob,
                0f, 3f, 1f, false);

        rSlider = createNewSlider(rSliderBackground, rSliderKnob,
                -100f, 100f, 1f, false);

        xyTouchPad = createNewTouchPad(xyTouchPadBackground, xyTouchPadKnob, 5f);

        rMainManipSlider = createNewSlider(rMainManipSliderBackground, rMainManipSliderKnob,
                -1f, 1f, 1f, false);

        camSlider = createNewSlider(camSliderBackground, camSliderKnob,
                -1f, 1f, 1f, true);

        tBotManipSlider = createNewSlider(tBotManipSliderBackground, tBotManipSliderKnob,
                -1f, 1f, 1f, true);

        tightMainManipButton = createNewButton(tightMainManipButtonUp, tightMainManipButtonDown);
        untightMainManipButton = createNewButton(untightMainManipButtonUp, untightMainManipButtonDown);
        autoPitchButton = createNewButton(autoPitchButtonUp, autoPitchButtonDown, autoPitchButtonChecked);
        autoYawButton = createNewButton(autoYawButtonUp, autoYawButtonDown, autoYawButtonChecked);
        autoDepthButton = createNewButton(autoDepthButtonUp, autoDepthButtonDown, autoDepthButtonChecked);
        multiplexorButton = createNewButton(multiplexorButtonFirst, multiplexorButtonSecond, multiplexorButtonPressed);
        ledButton = createNewButton(ledButtonUp, ledButtonDown, ledButtonChecked);

        mainStage.addActor(zSlider);
        mainStage.addActor(rSlider);
        mainStage.addActor(speedSlider);
        mainStage.addActor(xyTouchPad);
        mainStage.addActor(rMainManipSlider);
        mainStage.addActor(camSlider);
        mainStage.addActor(tBotManipSlider);
        mainStage.addActor(tightMainManipButton);
        mainStage.addActor(untightMainManipButton);
        mainStage.addActor(autoDepthButton);
        mainStage.addActor(autoPitchButton);
        mainStage.addActor(autoYawButton);
        mainStage.addActor(multiplexorButton);
        mainStage.addActor(ledButton);

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

    private Button createNewButton(String upPath, String downPath, String checkedPath) {
        Image buttonUpImage = new Image(new Texture(Gdx.files.internal(upPath)));
        Image buttonDownImage = new Image(new Texture(Gdx.files.internal(downPath)));
        Image buttonCheckedImage = new Image(new Texture(Gdx.files.internal(checkedPath)));
        Drawable buttonUp = buttonUpImage.getDrawable();
        Drawable buttonDown = buttonDownImage.getDrawable();
        Drawable buttonChecked = buttonCheckedImage.getDrawable();
        return new Button(buttonUp, buttonDown, buttonChecked);
    }

    private Button createNewButton(String upPath, String downPath) {
        Image buttonUpImage = new Image(new Texture(Gdx.files.internal(upPath)));
        Image buttonDownImage = new Image(new Texture(Gdx.files.internal(downPath)));
        Drawable buttonUp = buttonUpImage.getDrawable();
        Drawable buttonDown = buttonDownImage.getDrawable();
        return new Button(buttonUp, buttonDown);
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
        mainManipRotateDirection = (int) rMainManipSlider.getValue();
        cameraRotateDirection = (int) camSlider.getValue();
        if (tightMainManipButton.isPressed())
            mainManipTightDirection = 1;
        else if (untightMainManipButton.isPressed())
            mainManipTightDirection = -1;
        else mainManipTightDirection = 0;
        bottomManipDirection = (int) tBotManipSlider.getValue();
        speedMode = (int) speedSlider.getValue();
        if (multiplexorButton.isChecked())
            muxChannel = 1;
        else muxChannel = 0;
        isAutoYaw = autoYawButton.isChecked();
        isAutoPitch = autoPitchButton.isChecked();
        isAutoDepth = autoDepthButton.isChecked();
        isLED = ledButton.isChecked();
    }

    private void setZeroPositions() {
        if (!zSlider.isDragging()) zSlider.setValue(0f);
        if (!rSlider.isDragging()) rSlider.setValue(0f);
        if (!xyTouchPad.isTouched()) xyTouchPad.calculatePositionAndValue(0, 0, true);
        if (!rMainManipSlider.isDragging()) rMainManipSlider.setValue(0f);
        if (!camSlider.isDragging()) camSlider.setValue(0f);
        if (!tBotManipSlider.isDragging()) tBotManipSlider.setValue(0f);
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

    // Paths:
    private String zSliderBackground = "ui/sliders/zSlider/zSliderBackground.png";
    private String zSliderKnob = "ui/sliders/zSlider/zSliderKnob.png";
    private String speedSliderBackground = "ui/sliders/speedSlider/speedSliderBackground.png";
    private String speedSliderKnob = "ui/sliders/speedSlider/speedSliderKnob.png";
    private String rSliderBackground = "ui/sliders/rSlider/rSliderBackground.png";
    private String rSliderKnob = "ui/sliders/rSlider/rSliderKnob.png";
    private String xyTouchPadBackground = "ui/touchPads/xyTouchPad/xyTouchPadBackground.png";
    private String xyTouchPadKnob = "ui/touchPads/xyTouchPad/xyTouchPadKnob.png";
    private String rMainManipSliderBackground = "ui/sliders/rMainManipSlider/rMainManipSliderBackround.png";
    private String rMainManipSliderKnob = "ui/sliders/rMainManipSlider/rMainManipSliderKnob.png";
    private String camSliderBackground = "ui/sliders/camSlider/camSliderBackround.png";
    private String camSliderKnob = "ui/sliders/camSlider/camSliderKnob.png";
    private String tBotManipSliderBackground = "ui/sliders/tBotManipSlider/tBotManipSliderBackground.png";
    private String tBotManipSliderKnob = "ui/sliders/tBotManipSlider/tBotManipSliderKnob.png";
    private String tightMainManipButtonUp = "ui/buttons/tightMainManipButton/tightMainManipButtonUp.png";
    private String tightMainManipButtonDown = "ui/buttons/tightMainManipButton/tightMainManipButtonDown.png";
    private String untightMainManipButtonUp = "ui/buttons/untightMainManipButton/untightMainManipButtonUp.png";
    private String untightMainManipButtonDown = "ui/buttons/untightMainManipButton/untightMainManipButtonDown.png";
    private String autoPitchButtonUp = "ui/buttons/autoPitchButton/autoPitchButtonUp.png";
    private String autoPitchButtonDown = "ui/buttons/autoPitchButton/autoPitchButtonDown.png";
    private String autoPitchButtonChecked = "ui/buttons/autoPitchButton/autoPitchButtonChecked.png";
    private String autoYawButtonUp = "ui/buttons/autoYawButton/autoYawButtonUp.png";
    private String autoYawButtonDown = "ui/buttons/autoYawButton/autoYawButtonDown.png";
    private String autoYawButtonChecked = "ui/buttons/autoYawButton/autoYawButtonChecked.png";
    private String autoDepthButtonUp = "ui/buttons/autoDepthButton/autoDepthButtonUp.png";
    private String autoDepthButtonDown = "ui/buttons/autoDepthButton/autoDepthButtonDown.png";
    private String autoDepthButtonChecked = "ui/buttons/autoDepthButton/autoDepthButtonChecked.png";
    private String multiplexorButtonFirst = "ui/buttons/multiplexorButton/multiplexorButtonFirst.png";
    private String multiplexorButtonPressed = "ui/buttons/multiplexorButton/multiplexorButtonPressed.png";
    private String multiplexorButtonSecond = "ui/buttons/multiplexorButton/multiplexorButtonSecond.png";
    private String ledButtonUp = "ui/buttons/ledButton/ledButtonUp.png";
    private String ledButtonDown = "ui/buttons/ledButton/ledButtonDown.png";
    private String ledButtonChecked = "ui/buttons/ledButton/ledButtonChecked.png";
}