#!/usr/bin/env python
# Usage: gcovr PATH --xml | gcovr_xml_2_compiler_like_ewi.py LEVEL PREPATH
# LEVEL: 0 = error
#        1 = error and warning (default)
#        2 = error, warning and info
# by Djones A. Boni

import sys
import xml.etree.ElementTree as ET

# Default input file is STDIN
Input = sys.stdin

# Defaults
Level = 1
Prepath = ''

# Commandline arguments
if len(sys.argv) == 1:
	pass
elif len(sys.argv) == 2:
	Level = int(sys.argv[1])
elif len(sys.argv) == 3:
	Level = int(sys.argv[1])
	Prepath = sys.argv[2]+'/'
else:
	print("Invalid argument")
	sys.exit(-1)

tree = ET.parse(Input)
root = tree.getroot()

NumberOfLines = 0
NumberOfBranches = 0

for XmlCoverage in root.iter('coverage'):

	# Each file is in a different <class></class>
	for TestedFile in XmlCoverage.iter('class'):

		# Get file name
		Tested_FileName = TestedFile.attrib['filename']
		File_LineCoverage = float(TestedFile.attrib['line-rate']) * 100
		File_BranchCoverage = float(TestedFile.attrib['branch-rate']) * 100

		# Open file to read its lines
		try:
			if Tested_FileName[0] == '/':
				# Absolute path
				fp = open(Tested_FileName, 'r')
			else:
				# Relative path
				fp = open(Prepath + Tested_FileName, 'r')
		except IOError:
			print("%s: error: Could not open file" % (Tested_FileName) )
			continue
		File_Lines = fp.readlines()
		fp.close()

		# Print file name, line rate and branch rate
		print("File: %-50s Line: %3.0f%% Branch: %3.0f%%" %
				(Tested_FileName,
				File_LineCoverage, File_BranchCoverage)
				)

		# List of lines with statements
		# Sort the list
		Lines = []
		for Line in TestedFile.iter('line'):
			l = Line.attrib
			l['hits'] = int(l['hits'])
			l['number'] = int(l['number'])
			l['branch'] = True if l['branch'] == 'true' else False
			Lines.append(l)

			NumberOfLines += 1
			if l['branch']:
				NumberOfBranches += 1

		Lines.sort(key=lambda x: x['number'])

		# Print uncovered statements and branches
		for Line in Lines:

			Line_Hits = Line['hits']

			if (Level >= 2 and Line_Hits == 0):

				# Remove new line
				File_Line = (
						File_Lines[Line['number']-1]
						.rstrip()
						.replace('\t', ' ')
						)

				print("%s:%d: info: Uncovered statement: %s" % (
						Tested_FileName,
						Line['number'],
						File_Line)
						)

			elif ((Level >= 1 and Line_Hits > 0) and
					Line['branch'] == True):

				# Get the uncovered and total branches of this line
				# Coverage = [ uncovered, total ]
				Coverage = Line['condition-coverage']
				Coverage = Coverage.split()[1].strip('()').split('/')
				Coverage = [ int(Coverage[0]), int(Coverage[1]) ]
				Coverage = [ Coverage[1]-Coverage[0], Coverage[1] ]

				if Coverage[0] != 0:

					# Remove new line
					File_Line = (
							File_Lines[Line['number']-1]
							.rstrip()
							.replace('\t', ' ')
							)

					print("%s:%d: warning: Uncovered branch (%d/%d): %s" % (
							Tested_FileName,
							Line['number'],
							Coverage[0], Coverage[1],
							File_Line)
							)

	Total_LineCoverage = float(XmlCoverage.attrib['line-rate']) * 100
	Total_BranchCoverage = float(XmlCoverage.attrib['branch-rate']) * 100

	print("           %-0s %10s %10s" %
			('', 'Lines', 'Branches') )
	print("Total:     %-0s %10d %10d" %
			('', NumberOfLines, NumberOfBranches) )
	print("Uncovered: %-0s %9.2f%% %9.2f%%" %
			('', Total_LineCoverage, Total_BranchCoverage) )












