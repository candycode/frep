SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
SET SCENE=-primitives scenes\threeblobs.txt
SET COMPOSITE=-composite composites\blobs_blend_periodic.txt
SET COMPMATERIAL=-compositeMat composites\composite_color_blended3.txt
SET BOXSIZE=-compositeBoxHalfSize 4.5
frepup-simple.exe %SHADERS% %SCENE% %COMPOSITE% %COMPMATERIAL% %BOXSIZE%  