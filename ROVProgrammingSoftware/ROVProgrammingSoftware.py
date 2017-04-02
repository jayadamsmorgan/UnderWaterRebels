import Tkinter as Tk
import commands as com
import netifaces as ni
from tftpy import TftpClient as Tcl


class GUI:
    def __init__(self):
        self.target_ip = "192.168.1.128"
        self.target_port = 69
        self.prepare_btn = Tk.Button(root, text="Prepare binary", command=self.prepare_binary)
        self.send_btn = Tk.Button(root, text="Send binary", command=self.send_binary)
        self.prepare_btn.pack()
        self.send_btn.pack()
        self.is_prepared = 0
        self.local_ip = ""
        self.binary = ""
        self.hex_bin = ""

    def prepare_binary(self):
        is_found = 0
        print "Searching for compiled .hex file..."
        output = com.getoutput("ls ~/Documents/CompiledArduinoSketches")
        output_array = output.splitlines()
        for i in output_array:
            i.strip()
        for x in range(0, len(output_array)):
            if output_array[x] == "ROVSoftware.ino.hex":
                print "ROVSoftware Found!"
                is_found = 1
                self.hex_bin = output_array[x]
                break
        if is_found != 1:
            print "Hex file NOT found!"
            return
        print "Converting hex to binary..."
        com.getoutput("avr-objcopy -I ihex ~/Documents/CompiledArduinoSketches/ROVSoftware.ino.hex "
                      "-O binary ~/Documents/CompiledArduinoSketches/rov.bin")

        output = com.getoutput("ls ~/Documents/CompiledArduinoSketches")
        output_array = output.splitlines()
        for i in output_array:
            i.strip()
        for x in range(0, len(output_array)):
            if output_array[x] == "rov.bin":
                print "OK!"
                is_found = 2
                self.binary = output_array[x]
                break
        if is_found != 2:
            print "Error ;("
            return
        self.is_prepared = 1

    def send_binary(self):
        if self.is_prepared != 1:
            print "Prepare binary first!"
            return
        print "Sending binary..."
        try:
            self.local_ip = ni.ifaddresses('en0')[2][0]['addr']
        except:
            print "Check your connection to UnderwaterRebels' Wi-Fi"
            return
        print "Your IP address is: " + self.local_ip
        print "Target IP address is: " + self.target_ip
        try:
            client = Tcl(self.target_ip, self.target_port)
            client.upload(input="~/Documents/CompiledArduinoSketches/rov.bin", filename="")
        except:
            print "An error occurred... ;("

root = Tk.Tk()
root.title("ROV Programming Software")
obj = GUI()
root.mainloop()
