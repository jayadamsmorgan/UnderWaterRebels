package com.germanberdnikov.rov;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

class UDP {

    private String host;
    private int port;

    UDP(String host, int port) {
        this.host = host;
        this.port = port;
    }

    void sendMessage(byte[] data) {
        try {
            InetAddress address = InetAddress.getByName(host);
            DatagramPacket pack = new DatagramPacket(data, data.length, address, port);
            DatagramSocket ds = new DatagramSocket();
            ds.send(pack);
            ds.close();
        } catch (IOException e) {
            System.err.println(e.getMessage());
        }

    }

}
