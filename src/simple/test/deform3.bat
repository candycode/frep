SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
REM -optimizedComposite
REM-primitives scenes\blobs_cylinder.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\deform3.txt
SET COMPMAT=-compositeMat composites\composite_color.txt
SET STEP=-dCompositeStep 0.005
SET OPTIMIZATION=-optimizedComposite
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT% %STEP% 