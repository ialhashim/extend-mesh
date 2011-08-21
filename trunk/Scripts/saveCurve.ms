-- SAVE CURVE =============================
-- Get filename
fileName = getSaveFileName caption:"Save Curve" types:"Curve file (*.curve)|*.curve|All Files (*.*)|*.*|"

-- Create file
fs = createFile fileName

-- empty array
points = #() 

-- specify number of samples on the curve
np = 20

-- get selected curve
curve = selection[1] 
if curve == undefined do break()

-- get the samples
for i = 1 to np do
(
	t = i as float / np
	
	p = lengthInterp curve t
	
	append points p
)

-- write to file
for i = 1 to np do
(
	value = points[i]
	format "% % %\n" value[1] value[2] value[3] to: fs
)

-- Close file
close fs
