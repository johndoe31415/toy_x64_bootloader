#!/usr/bin/python3

string = b"Betriebssysteme 21CS1!    \x00"
reg = 55557

values = [ ]
for char in string:
	lsb = reg & 1
	reg >>= 1
	if lsb:
		reg ^= 0x1021
	values.append(char ^ (reg & 0xff))

print(".byte " + (", ".join(str(value) for value in values)))
