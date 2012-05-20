SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
REM-primitives scenes\cone_sphere.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\passthrough.txt
SET COMPMAT=-compositeMat composites\composite_color.txt
SET STEP=-dCompositeStep 0.01
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT% %STEP%  