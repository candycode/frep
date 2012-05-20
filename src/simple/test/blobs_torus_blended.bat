REM NOT WORKING!!!!!
SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
SET SCENE=-primitives scenes\blobs_funtorus.txt
SET COMPOSITE=-composite composites\blobs_cyl_blend_union.txt
SET COMPMATERIAL=-compositeMat composites\composite_color_blended3.txt
SET BOXES=-compositeBoxHalfSize 7 -primitiveBoxHalfSize 1.5
frepup-simple.exe %SHADERS% %SCENE% %COMPOSITE% %COMPMATERIAL%  %BOXES% 