package com.germanberdnikov.rov;

import com.badlogic.gdx.ApplicationAdapter;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;

class Main extends ApplicationAdapter {
    private SpriteBatch batch;
    private UDPThread udpThread;

    @Override
    public void create() {
        batch = new SpriteBatch();
        udpThread = new UDPThread();
        udpThread.start();
    }

    @Override
    public void render() {
        Gdx.gl.glClearColor(1f / 15f, 1f / 15f, 1f / 15f, 1);
        Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);
        batch.begin();
        batch.end();
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

