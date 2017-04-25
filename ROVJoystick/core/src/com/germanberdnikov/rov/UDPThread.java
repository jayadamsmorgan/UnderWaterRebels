package com.germanberdnikov.rov;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

class UDPThread extends Thread {

    private boolean isRunning;
    private DatagramSocket socket;
    private byte data[];
    private int ip1 = 192, ip2 = 168, ip3 = 1, ip4 = 124;
    private int packet_size = 7;

    UDPThread() {
        isRunning = true;
        data = new byte[packet_size];
    }

    void setRunning(boolean isRunning) {
        this.isRunning = isRunning;
    }

    void setData(int xAxis, int yAxis, int zAxis, int rAxis, int wAxis,
                 int rotCam, int manTight, int botMan, int speedMode, int muxChannel,
                 boolean isAutoYaw, boolean isAutoPitch, boolean isAutoDepth, boolean isLED) {
        // Values are from -100 to 100, so we can use 1 byte to send every axis
        data[0] = (byte) xAxis;
        data[1] = (byte) yAxis;
        data[2] = (byte) zAxis;
        data[3] = (byte) rAxis;
        data[4] = (byte) wAxis;

        if (rotCam < 0) {
            data[5] |= (byte) 0b00000001;
        } else if (rotCam > 0) {
            data[5] |= (byte) 0b00000010;
        }
        if (manTight < 0) {
            data[5] |= (byte) 0b00000100;
        } else if (manTight > 0) {
            data[5] |= (byte) 0b00001000;
        }
        if (botMan < 0) {
            data[5] |= (byte) 0b00010000;
        } else if (botMan > 0) {
            data[5] |= (byte) 0b00100000;
        }
        if (muxChannel == 0) {
            data[5] |= (byte) 0b01000000;
        } else if (muxChannel == 1) {
            data[5] |= (byte) 0b10000000;
        }

        if (speedMode == 3) {
            data[6] |= (byte) 0b00000001;
        } else if (speedMode == 2) {
            data[6] |= (byte) 0b00000010;
        } else if (speedMode == 1) {
            data[6] |= (byte) 0b00000100;
        }
        if (isAutoYaw) {
            data[6] |= (byte) 0b00001000;
        }
        if (isAutoPitch) {
            data[6] |= (byte) 0b00010000;
        }
        if (isAutoDepth) {
            data[6] |= (byte) 0b00100000;
        }
        if (isLED) {
            data[6] |= (byte) 0b01000000;
        }
    }

    @Override
    public void run() {
        try {
            int serverPort = 8001;
            socket = new DatagramSocket(serverPort);
            InetAddress address = InetAddress.getByAddress(new byte[]{(byte) ip1, (byte) ip2, (byte) ip3, (byte) ip4});
            while (true) {
                while (isRunning) {
                    DatagramPacket packet = new DatagramPacket(data, packet_size, address, serverPort);
                    socket.send(packet);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (socket != null) {
                socket.close();
            }
        }
    }
}
