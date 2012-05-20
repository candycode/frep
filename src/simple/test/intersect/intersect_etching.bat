SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_external_intersection.frag
REM -optimizedComposite
REM-primitives scenes\cone_sphere.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\intersect\intersect_displace.txt
SET COMPMAT=-compositeMat composites\displace_color.txt
SET STEP=-dCompositeStep 0.01
SET OPTIMIZATION=-optimizedComposite
SET STENCIL_MASK=-compositeTexture %2
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT% %STEP% %STENCIL_MASK%