/*
Debug-Utility-v0_0.ms
"Debug-Utility"
version ##
Written for 3dsmax r8
Written by AUTHOR_NAME
MM.DD.YY

PURPOSE:
- To view settings when debugging options to the standard
*/

-- format:".8g"
-- format:".6f"

--
--             <<< S C R I P T   S T A R T S   H E R E >>> 
--
--
global nifToolsFloater = undefined
global nifdbg_utl_up = undefined
--------------------------
--<< F U N C T I O N S >>
--------------------------
--------------------------
-- FUNCTION (NifDebug)
--
-- This function uses the standard 'Unhide By Name' dialog, PLUS adds options to select objects on
-- unhide; to add objects to current selection on unhide; and to zoom the active viewport
-- or all viewports to the selected objects.
fn toEuler m = (
  local x, y, z
	if ( m[1][3] < 1.0 ) then
	(
		if ( m[1][3] > - 1.0 ) then
		(
			x = atan2 -m[2][3] m[3][3]
			y = asin   m[1][3]
			z = atan2 -m[1][2] m[1][1]
			return eulerAngles x y z
		)
		else
		(
			x = - atan2 -m[2][1] m[2][2]
			y = -pi/2
			z = 0.0
			return eulerAngles x y z
		)
	)
	else
	(
		x = atan2 m[2][1] m[2][2] ;
		y = pi/2
		z = 0.0
		return eulerAngles x y z
	)
)

fn getTransform node = (
  if (node == undefined) then (
    return Matrix3 1
  ) else (
    if (classOf node as string) == "Biped_Object" then (
      local pos = biped.getTransform node #pos
      local rot = biped.getTransform node #rotation
      local scl = biped.getTransform node #scale
      return (scaleMatrix scl) * (Inverse (rot as Matrix3)) * (transMatrix pos)
    ) else (
      return node.transform
    )
  )
)

fn getObjectTransform node = (
  if (node == undefined) then (
    return Matrix3 1
  ) else (
    return node.objecttransform
  )
)

fn nifDebug = 
(
    if (nifToolsFloater != undefined) then
    (
        nifDebugUtil nifToolsFloater.rollouts[1]
    )
    if ( nifdbg_utl_up != null ) then
    (
        nifDebugUtil nifdbg_utl_up
    )
)


fn svNifDbgSelectionChange =
(
	-- Get schematic view index
	-- svIndex = callbacks.notificationParam()
  
)


--------------------------
-- end functions.

rollout nifTransformTools "Nif Transform Tools" width:220 height:312
(

  -- UI CONTROL ITEMS.
  edittext edt_Name "Name:" readOnly:true
  checkbox chk_local "Show Local Transform"
  checkbox chk_object "Show Object Transform"
  group "Position"
  (
    edittext edt_posx "X:" readOnly:true
    edittext edt_posy "Y:" readOnly:true
    edittext edt_posz "Z:" readOnly:true
  )
  checkbox chk_invrot "Invert Rotation"
  group "Rotation Quat"
  (
    edittext edt_rotw "W:" readOnly:true
    edittext edt_rotx "X:" readOnly:true
    edittext edt_roty "Y:" readOnly:true
    edittext edt_rotz "Z:" readOnly:true
  )
  group "Rotation Euler"
  (
    label labEuler "Order:" align:#left
    dropdownlist ddEuler "" items:#("NifSkope","1 - XYZ","2 - XZY","3 - YZX","4 - YXZ","5 - ZXY","6 - ZYX","7 - XYX","8 - YZY","9 - ZXZ") selection:10 width:90 offset:[40,-20]
    checkbox chk_showrad "Show in Radians"
    edittext edt_rotex "X:" readOnly:true
    edittext edt_rotey "Y:" readOnly:true
    edittext edt_rotez "Z:" readOnly:true
  )
  group "Rotation Matrix"
  (
    checkbox chk_showlh "Show Transpose"
    edittext edt_rotmat height:50 readOnly:true offset:[-10,-5] width:200
  )
  group "Scale"
  (
    edittext edt_sclx "X:" readOnly:true
    edittext edt_scly "Y:" readOnly:true
    edittext edt_sclz "Z:" readOnly:true
  )
  group "Other"
  (
    edittext edt_len "Length:" readOnly:true
  )
  checkbox chk_parity "Parity" enabled:false
  button btnRefresh "Refresh" -- pos:[114,60] width:77 height:17

  on btnRefresh pressed do
  (
    nifTransformTools.nifDebugUpdate()
  )
  
  on chk_local changed theState do 
  (
    nifTransformTools.nifDebugUpdate()
  )
  
  on chk_object changed theState do 
  (
    nifTransformTools.nifDebugUpdate()
  )
  
  on chk_invrot changed theState do 
  (
    nifTransformTools.nifDebugUpdate()
  )
  on chk_showrad changed theState do 
  (
    nifTransformTools.nifDebugUpdate()
  )
  on chk_showlh changed theState do 
  (
    nifTransformTools.nifDebugUpdate()
  )
  
  on ddEuler selected theState do 
  (
    nifTransformTools.nifDebugUpdate()
  )
  
  on nifTransformTools open do
  (
	try ( nifTransformTools.ddEuler.selection = 1 ) catch ()
    try ( nifTransformTools.chk_local.checked = true ) catch ()
    try ( nifTransformTools.chk_invrot.checked = true ) catch ()
    try ( nifTransformTools.chk_showrad.checked = true ) catch ()
    
    
    nifTransformTools.nifDebugUpdate()
    
    callbacks.addScript #selectionSetChanged "nifTransformTools.nifDebugUpdate()" id:#nidbg
	registerTimeCallback nifTransformTools.nifDebugUpdate
    --nifdbg_utl_up = utl_up
   
  )

  on nifTransformTools close do
  (
    --nifdbg_utl_up = undefined
    callbacks.removeScripts #selectionSetChanged id:#nidbg
	unRegisterTimeCallback nifDebug
  )

  fn nifDebugUpdate =
  (
    local utl_up  = nifTransformTools
    try (
      if (utl_up != undefined) then
      (
        if (selection.count == 1 and utl_up.chk_local != undefined and utl_up.chk_object != undefined) then (
          local obj = selection[1]
          local pos, rot, scl, ppos, prot, pscl, len
          local parent = obj.parent
          local parity = false;
    	  
          if (parent != undefined) then (
            local PM = getTransform parent
            local M = getTransform obj
    		len = (length (M.translationpart - PM.translationpart))
            if (utl_up.chk_object.checked) then (
                if (utl_up.chk_local.checked) then (
                  -- local coordinates
                  local LT = (getObjectTransform obj) * (Inverse M)
                  pos = LT.translationpart
                  rot = (Inverse LT.rotationpart)
                  scl = LT.scalepart
                  parity = (LT.determinantsign != 0)
        		) else (
        	 	  local M = getObjectTransform obj
                  pos = M.translationpart
                  rot = (Inverse M.rotationpart)
                  scl = M.scalepart
                  parity = (M.determinantsign != 0)
        		)
            ) else (
                if (utl_up.chk_local.checked) then (
                  -- local coordinates
                  local LT = M * (Inverse PM)
                  pos = LT.translationpart
                  rot = (Inverse LT.rotationpart)
                  scl = LT.scalepart
                  parity = (LT.determinantsign != 0)
        		) else (
        	 	  local M = getTransform obj
                  pos = M.translationpart
                  rot = (Inverse M.rotationpart)
                  scl = M.scalepart
                  parity = (M.determinantsign != 0)
        		)
            )
          ) else (
    		local M = getTransform obj
            pos = M.translationpart
            rot = (Inverse M.rotationpart)
            scl = M.scalepart
            parity = (M.determinantsign != 0)
    		len = length pos
          )
          
          utl_up.chk_parity.checked = parity
          
          utl_up.edt_Name.text = obj.name as string
          utl_up.edt_posx.text = formattedPrint pos.x format:".6f"
          utl_up.edt_posy.text = formattedPrint pos.y format:".6f"
          utl_up.edt_posz.text = formattedPrint pos.z format:".6f"

          local fix = (matrix3 [1,0,0] [0,0,1] [0,1,0] [0,0,0])
          local lh = (rot as Matrix3) * fix
          if ( utl_up.chk_showlh.checked ) then (
             rot = lh
          )
          
          if (utl_up.chk_invrot.checked) then ( 
             rot = inverse rot
          )
          local rotmat = rot as Matrix3
          local qrot = rot as Quat      
          local esel = utl_up.ddEuler.selection
          local erot = undefined
          if (esel == 1) then (
            erot = toEuler rotmat
          ) else (
            erot = quatToEuler qrot order:(esel-1)
          )
          --local erot = rot as EulerAngles
          utl_up.edt_rotx.text = formattedPrint qrot.x  format:".6f"
          utl_up.edt_roty.text = formattedPrint qrot.y  format:".6f"
          utl_up.edt_rotz.text = formattedPrint qrot.z  format:".6f"
          utl_up.edt_rotw.text = formattedPrint qrot.w  format:".6f"

          if (utl_up.chk_showrad.checked) then (
            utl_up.edt_rotex.text = formattedPrint (degToRad erot.x) format:".6f"
            utl_up.edt_rotey.text = formattedPrint (degToRad erot.y) format:".6f"
            utl_up.edt_rotez.text = formattedPrint (degToRad erot.z) format:".6f"
          ) else (
            utl_up.edt_rotex.text = formattedPrint erot.x format:".6f"
            utl_up.edt_rotey.text = formattedPrint erot.y format:".6f"
            utl_up.edt_rotez.text = formattedPrint erot.z format:".6f"
          )
          
          if (utl_up.chk_showlh.checked) then (
            local tmp = formattedPrint lh[1] format:".6f" + "\r\n" + formattedPrint lh[2] format:".6f" + "\r\n" + formattedPrint lh[3] format:".6f" + "\r\n"
            utl_up.edt_rotmat.text = tmp
          ) else (
            local tmp = formattedPrint rotmat[1] format:".6f" + "\r\n" + formattedPrint rotmat[2] format:".6f" + "\r\n" + formattedPrint rotmat[3] format:".6f" + "\r\n"
            --local tmp = (rotmat[1] as string) + "\r\n" + (rotmat[2] as string) + "\r\n" + (rotmat[3] as string) + "\r\n"
            utl_up.edt_rotmat.text = tmp
          )

          utl_up.edt_sclx.text = formattedPrint scl.x format:".6f"
          utl_up.edt_scly.text = formattedPrint scl.y format:".6f"
          utl_up.edt_sclz.text = formattedPrint pos.z format:".6f"
    	  
    	  utl_up.edt_len.text = formattedPrint len format:".6f"
          
        ) else (
          for obj in utl_up.controls do (
    	     try ( 
             if ( (classOf obj as string) == "EditTextControl") then (
                obj.text = "" 
             )
           ) catch ()
          )
        )
      )
    )catch ()
    )
  
)


function nifDebug_openTools = 
(

	if (nifToolsFloater == undefined) or (not nifToolsFloater.open) then
	(
		nifToolsFloater = newRolloutFloater "NifTools Toolbox" 225 745
        -- cui.RegisterDialogBar nifToolsFloater style:#(#cui_dock_left, #cui_dock_right, #cui_floatable, #cui_handles, #cui_max_sized) minSize:[230,-1] maxSize:[230,-1]
		addRollout nifTransformTools nifToolsFloater rolledUp:false
	)
)


Utility utl_up "NifDebug Tools" silentErrors:true
(
  -- VARIABLES.
  -- None

  -- ROLLOUT EVENTS.

  -- UTILITY EVENTS.
  -- Notice, these are outside the Rollout clause, but still inside the Utility clause.
  on utl_up open do
  (
    -- callbacks.addScript #selectionSetChanged "nifDebug()" id:#nidbg
	-- registerTimeCallback nifDebug
	-- try ( utl_up.ddEuler.selection = 1 ) catch ()
    -- nifdbg_utl_up = utl_up
    -- nifDebug()
    
    nifDebug_openTools()
  )

  on utl_up close do
  (
    -- nifdbg_utl_up = undefined
    -- callbacks.removeScripts #selectionSetChanged id:#nidbg
	-- unRegisterTimeCallback nifDebug
  )
  
 
)--end utl_up.

-- Note: A Utility is 180 pixels wide. Sometimes this limits the UI design.
--------------------------
-- end script.