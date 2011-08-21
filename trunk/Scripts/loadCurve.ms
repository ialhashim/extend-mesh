-- Get filename
fileName = getOpenFileName caption:"Open Curve" types:"Curve file (*.curve)|*.curve|All Files (*.*)|*.*|"
fs = openFile fileName

-- empty array
points = #() 

-- get the points
while not eof fs do
(
	inputData = readLine fs 
	p = filterString inputData " "
	append points p
)

-- create curve
ss = SplineShape pos:[0,0,0]
addNewSpline ss

-- add points
for i = 1 to points.count do
(
	v = points[i]
	addKnot ss 1 #corner #line [ v[1] as float, v[2] as float, v[3] as float ]
)

updateShape ss
ss

-- Close file
close fs
