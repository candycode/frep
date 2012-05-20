SET SHADERS= -vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
SET SCENE=-primitives scenes\cone_sphere.txt
SET COMPOSITE=-composite composites\subtract.txt
SET COMPMATERIAL=-compositeMat composites\composite_color_blended.txt
SET STEPS=-dCompositeStep 0.002 -dPrimitiveStesp 0.1
SET BOXSIZE=-compositeBoxHalfSize 1.5
frepup-simple.exe %SHADERS% %SCENE% %COMPOSITE% %COMPMATERIAL% %STEPS% %BOXSIZE%  