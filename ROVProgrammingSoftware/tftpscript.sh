#!bin/bash
cd ~/Documents/CompiledArduinoSketches/
tftp 192.168.1.128 69 << fin
	mode octet
	trace
	verbose
	put rov.bin
	quit
fin
