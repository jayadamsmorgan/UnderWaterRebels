package com.germanberdnikov.rov;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.SocketException;

class UDPThread extends Thread {

    boolean isRunning;
    DatagramSocket socket;
    int serverPort = 8001;
    byte data[];

    UDPThread() {
        isRunning = true;

    }

    public void setRunning(boolean isRunning) {
        this.isRunning = isRunning;
    }

    public void setData(char xAxis, char yAxis, char zAxis, char rAxis, char wAxis,
                        char rotCam, char manTight, char botMan, char manRot,
                        char speedMode,
                        boolean isAutoYaw, boolean isAutoPitch, boolean isAutoDepth) {
        
    }

    private void getBytes() {

    }

    @Override
    public void run() {
        try {
            socket = new DatagramSocket(serverPort);
            while (isRunning) {
                getBytes();
            }
        } catch (SocketException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (socket != null) {
                socket.close();
            }
        }
    }
}
