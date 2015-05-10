'''
This script will look through a WireShark csv file and see if it can find
unaligned reads.  "Unaligned" is any read operation that doesn't begin and end
on a 16-byte boundary.  You might want to change that value for your tests.

Here are some hints and details:

* You can create a .csv file with WireShark.  With WireShark 1.0.15 you do the
  following:
  1. Capture your data.
  2. Hit the "Stop" button.
  3. Select "File" -> "Export" -> "as CSV ..."
  4. Specify the name of the file and click "OK".

* This script is currently hardcoded to read a file called "trace.csv".  That
  can be changed if necessary.

* Using the "csv" module, it reads the file one line at a time.

* Each line is converted to a list object.  The object consists of:
  [0] - The record number.
  [1] - The timestamp.
  [2] - The src IP Address.
  [3] - The dst IP Address.
  [4] - The protocol.  For SMB2 it will say "SMB2".
  [5] - The length of the frame.  I'm not sure if that's the whole frame of just
        some portion.  It's probably the whole frame.
  [6] - The details of the record that was read.  For an SMB2 Read it will say:
          "Read Request Len:xxxxx Off:yyyyy File: joe.txt"

* We are after the xxxxx and yyyyy portions.  We extract them and check to make
  sure they're multiples of 16 (using "val % 16").
'''

import csv

# Open trace.csv as an input file.
with open('trace.csv', 'r') as csvInFile:
	# Get a csv object.  It knows how to read csv file.
	reader = csv.reader(csvInFile)
	# Loop once for each record in the csv file.
	for line in reader:
		# Index 6 contains the details of the record.
		field = line[6]
		# Check to see if it's a read request.  It's possible this won't
		# be a good test.  It worked for my purposes, but you might need
		# to change it for your purposes.
		readRequestIndex = field.find("Read Request")
		if readRequestIndex != -1:
			# Chop the data before the "Len:" field.
			readLenBeg = field.find("Len:") + 4
			lenStr = field[readLenBeg:]
			# Chop the data after the "Len:" value.
			readLenEnd = lenStr.find(" ")
			lenStr = lenStr[0:readLenEnd]
			# Get the read length.
			len = int(lenStr)

			# Chop the data before the "Off:" field.
			readOffBeg = field.find("Off:") + 4
			offStr = field[readOffBeg:]
			# Chop the data after the "Off:" value.
			readOffEnd = offStr.find(" ")
			offStr = offStr[0:readOffEnd]
			# Get the read offset.
			off = int(offStr)

			# Check to see if the offset or length is unaligned on
			# a crypto boundary.
			offBlockMod = off % 16
			lenBlockMod = len % 16

			print "len = " + str(len) + ".  off = " + str(off) + "."
			if offBlockMod > 0:
				print "offBlockMod = " + str(offBlockMod) + "."
				exit(1)

			if lenBlockMod > 0:
				print "lenBlockMod = " + str(lenBlockMod) + "."
				exit(1)

