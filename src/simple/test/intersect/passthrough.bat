SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_external_intersection.frag
REM -optimizedComposite
REM-primitives scenes\cone_sphere.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\intersect\passthrough.txt
SET COMPMAT=-compositeMat composites\composite_color.txt
SET STEP=-dCompositeStep 0.01
SET OPTIMIZATION=-optimizedComposite
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT% %STEP% 