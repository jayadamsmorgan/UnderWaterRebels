import Tkinter as Tk
import commands as com
import netifaces as ni


class GUI:
    def print_text(self, output_text):
        output = Tk.Label(self.frame, text=output_text, font="Arial 12")
        output.pack()

    def __init__(self):
        self.target_ip = "192.168.1.128"
        self.target_port = 69

        self.frame = Tk.Frame(root, width=300, height=500)
        self.prepare_btn = Tk.Button(self.frame, text="Prepare binary", command=self.prepare_binary)
        self.send_btn = Tk.Button(self.frame, text="Send binary", command=self.send_binary)

        self.prepare_btn.pack()
        self.send_btn.pack()
        self.frame.pack()

        self.is_prepared = 0
        self.local_ip = ""
        self.binary = ""
        self.hex_bin = ""

    def prepare_binary(self):
        is_found = 0
        self.print_text("Searching for compiled .hex file...")
        output = com.getoutput("ls ~/Documents/CompiledArduinoSketches")
        output_array = output.splitlines()
        for i in output_array:
            i.strip()
        for x in range(0, len(output_array)):
            if output_array[x] == "ROVSoftware.ino.hex":
                self.print_text("ROVSoftware Found!")
                is_found = 1
                self.hex_bin = output_array[x]
                break
        if is_found != 1:
            self.print_text("Hex file NOT found!")
            return
        self.print_text("Converting hex to binary...")
        com.getoutput("rm /Users/germanberdnikov/Documents/CompiledArduinoSketches/rov.bin")
        com.getoutput("/Applications/Work/Programming/Arduino.app/Contents/Java/hardware/tools/avr/bin/avr-objcopy "
                      "-I ihex ~/Documents/CompiledArduinoSketches/ROVSoftware.ino.hex -O binary "
                      "~/Documents/CompiledArduinoSketches/rov.bin")
        output = com.getoutput("ls ~/Documents/CompiledArduinoSketches")
        output_array = output.splitlines()
        for i in output_array:
            i.strip()
        for x in range(0, len(output_array)):
            if output_array[x] == "rov.bin":
                self.print_text("OK!")
                self.print_text("Now reboot your ROV and send bin in ~2 sec after rebooting")
                is_found = 2
                self.binary = output_array[x]
                break
        if is_found != 2:
            self.print_text("Error ;(")
            return
        self.is_prepared = 1

    def send_binary(self):
        if self.is_prepared != 1:
            self.print_text("Prepare binary first!")
            return
        self.print_text("Sending binary...")
        try:
            self.local_ip = ni.ifaddresses('en0')[2][0]['addr']
        except:
            self.print_text("Check your connection to UnderwaterRebels' Wi-Fi")
            return
        # TODO checking of my static ip
        self.print_text("Your IP address is: " + self.local_ip)
        self.print_text("Target IP address is: " + self.target_ip)
        output = com.getoutput("bash ~/Documents/UnderwaterRebels/ROVProgrammingSoftware/tftpscript.sh")
        self.print_text(output)
        # TODO checking of output


root = Tk.Tk()
root.title("ROV Programming Software")
obj = GUI()
root.mainloop()
