SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
SET SCENE=-primitives scenes\blobs_cylinder.txt
SET COMPOSITE=-composite composites\blobs_cyl_blend_sub.txt
SET COMPMATERIAL=-compositeMat composites\composite_color_blended3.txt
SET BOXES=-compositeBoxHalfSize 5.5
frepup-simple.exe %SHADERS% %SCENE% %COMPOSITE% %COMPMATERIAL% %BOXES% 