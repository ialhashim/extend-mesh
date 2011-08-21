--====================================================
--
--   Convert to simple curve
--
--====================================================
try destroyDialog simpleCurver catch()

rollout simpleCurver "Convert curve"
(
	group "Options"
	(
		spinner np "Num Points:" range:[0,1000,30] type:#integer fieldwidth:30 align:#left
		spinner scaled "Scale:" range:[0.0,1000.0,1.0] type:#float fieldwidth:30 align:#left
		button convertButton "Convert"
	)
	
	group "Export"
	(
		button saveButton "Convert & Save.."
	)
	
	group "Import"
	(
		button loadButton "Load.."
	)

	fn convertShape isSaveToFile =
	(
		numPnts = np.value
		
		-- get selected curve
		curve = selection[1] 
		if curve == undefined do break()
		
		points = #() 

		if isSaveToFile do
		(
			fileName = getSaveFileName caption:"Save Curve" types:"Curve file (*.curve)|*.curve|All Files (*.*)|*.*|"
			fs = createFile fileName
		)
		
		-- get the samples
		for i = 1 to numPnts do
		(
			t = i as float / numPnts
			
			p = lengthInterp curve t

			factor = 1.0 * scaled.value
			
			p[1] *= factor
			p[2] *= factor
			p[3] *= factor
			
			append points p
		)

		-- create curve
		ss = SplineShape pos:[0,0,0]
		addNewSpline ss

		-- add points
		for i = 1 to points.count do
		(
			v = points[i]

			addKnot ss 1 #corner #line [ v[1] as float, v[2] as float, v[3] as float]
			
			if isSaveToFile do
			(
				values = points[i]
				format "% % %\n" values[1] values[2] values[3] to: fs
			)
		)

		updateShape ss
		ss
		
		if isSaveToFile do
		(
			close fs
		)
	)
	
	fn loadShape = 
	(
		fileName = getOpenFileName caption:"Open Curve" types:"Curve file (*.curve)|*.curve|All Files (*.*)|*.*|"
		fs = openFile fileName
		points = #() 

		-- get the points
		while not eof fs do
		(
			inputData = readLine fs 
			p = filterString inputData " "
			append points p
		)
	
		ss = SplineShape pos:[0,0,0]
		addNewSpline ss

		for i = 1 to points.count do
		(
			v = points[i]
			addKnot ss 1 #corner #line [ v[1] as float, v[2] as float, v[3] as float ]
		)

		updateShape ss
		ss

		close fs
	)
	
	on convertButton pressed do 
	(
		convertShape(false)
	)
	
	on saveButton pressed do 
	(
		convertShape(true)
	)
	
	on loadButton pressed do 
	(
		loadShape()
	)
)

createDialog simpleCurver


