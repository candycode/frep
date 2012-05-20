SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade.frag
SET SCENE=-primitives %1
REM-primitives scenes\blobs_cylinder.txt
SET COMPOSITE=-composite composites\sum_blend10.txt
SET COMPMATERIAL=-compositeMat composites\composite_color_blended3.txt
SET BOXES=-compositeBoxHalfSize 5.5
SET STEPS=-compositeStep 0.01;
frepup-simple.exe %SHADERS% %SCENE% %COMPOSITE% %COMPMATERIAL%  %BOXES% 