package com.germanberdnikov.rov;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.BitSet;

class UDPThread extends Thread {

    private boolean isRunning;
    private DatagramSocket socket;
    private byte data[];
    private int ip1 = 192, ip2 = 168, ip3 = 1, ip4 = 124;

    UDPThread() {
        isRunning = true;
        data = new byte[7];
    }

    void setRunning(boolean isRunning) {
        this.isRunning = isRunning;
    }

    void setData(int xAxis, int yAxis, int zAxis, int rAxis, int wAxis,
                 int rotCam, int manTight, int botMan, int speedMode,
                 boolean isAutoYaw, boolean isAutoPitch, boolean isAutoDepth) {
        // Values are from -100 to 100, so we can use 1 byte to send every axis
        data[0] = (byte) xAxis;
        data[1] = (byte) yAxis;
        data[2] = (byte) zAxis;
        data[3] = (byte) rAxis;
        data[4] = (byte) wAxis;

        // UNTESTED YET
        BitSet bitSet = new BitSet(1);
        if (rotCam < 0) {
            bitSet.set(1, true);
            data[5] = bitToByte(bitSet, 7);
            bitSet.set(1, false);
            data[5] = bitToByte(bitSet, 6);
        } else if (rotCam > 0) {

        } else {

        }
        if (manTight < 0) {

        } else if (manTight > 0) {

        } else {

        }
        if (botMan < 0) {

        } else if (botMan > 0) {

        } else {

        }
        if (speedMode == 1) {

        } else if (speedMode == 2) {

        } else if (speedMode == 3) {

        }

    }

    private static byte bitToByte(BitSet bits, int offset) {
        byte value = 0;
        for (int i = offset; (i < bits.length() && ((i + offset) < 8)); ++i) {
            value += bits.get(i) ? (1 << i) : 0;
        }
        return value;
    }

    @Override
    public void run() {
        try {
            int serverPort = 8001;
            socket = new DatagramSocket(serverPort);
            InetAddress address = InetAddress.getByAddress(new byte[]{(byte) ip1, (byte) ip2, (byte) ip3, (byte) ip4});
            while (true) {
                while (isRunning) {
                    DatagramPacket packet = new DatagramPacket(data, data.length, address, serverPort);
                    socket.send(packet);
                }
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
